#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H
#include <windows.h>
#include <dsound.h>
#include <process.h>
#include <string>
#include <vector>
#include <list>
#include "Vorbisfile.h"
#include "Hashpool.h"

#define TS_SOUNDDRIVER 0x0001

#define TS_STREAMING 0x0001
#define TS_MEMORY    0x0002
#define TS_PLAYING   0x0004

#pragma pack(push)
#pragma pack(1)
struct TWavHeader
{
	char RiffTag[4];
	unsigned long ChunkDataSize;
	char WaveTag[4];
	char FmtTag[4];
	unsigned long FmtSize;
	unsigned short FormatTag;
	unsigned short Channels;
	unsigned long SamplesPerSec;
	unsigned long AvgBytesPerSec;
	unsigned short BlockAlign;
	unsigned short BitPerSample;
	char DataTag[4];
	unsigned long DataSize;
};
#pragma pack(pop)
typedef TWavHeader* PWavHeader;


//new sound objects structs

//internal sound info struct
class TSoundObject;

const int MaxDuplicate=4;

class TSoundInfo
{
	friend class TSoundObject;
private:
	LPDIRECTSOUNDBUFFER SoundBuffer;//primary sound buffer for that sound
	bool SpinLock;
	int DuplicateCount;
	int DuplicateIndex;
	std::vector<LPDIRECTSOUNDBUFFER> Duplicates;
	std::list<TSoundObject*> SoundObjList; //keep a list for deallocation

	void Initialize(void);
	void Uninitialize(void);
	void Lock(void);
	void Unlock(void);
	void RemoveMe(TSoundObject*);
public:
	TSoundInfo(char* SndName);
	~TSoundInfo(void);

	const char* SoundName;
	TWavHeader Header;
		
	//CRITICAL_SECTION CritSect; //maybe not needed , 
	
	TSoundObject* CreateSoundObject(void);
	void CreateDuplicate(void);
	void DestroyDuplicate(void);
	LPDIRECTSOUNDBUFFER Play(float Vol,float Pan);

	void DestroyAll(void);
};
typedef TSoundInfo* PSoundInfo;

class TSoundObject
{
	friend class TSoundInfo;
private:
	PSoundInfo Reference;
	LPDIRECTSOUNDBUFFER Buffer;
	bool Loaded;
	float Volume,Panoramic;
	void SetRefToZero(void){Reference=0; Loaded=false;};

	TSoundObject(PSoundInfo Ref);//private constructor, only a TSoundinfo can create that object
public:
	
	~TSoundObject(void);
	void Play(void);
	void Load(void);
	void Unload(void);
	void SetVolume(float NewVolume){Volume=NewVolume;}; //0 -> 1
	void SetPan(float NewPan){Panoramic=NewPan;}; //-1 -> +1
};

class TSoundEngine 
{
public:
	bool Initialized;
	
	float SoundVolume,OggVolume;
	LPDIRECTSOUND8 DirectSound;
	LPDIRECTSOUNDBUFFER Primary;
	HANDLE hEvent;

	//ogg thingies
	LPDIRECTSOUNDBUFFER OggBuffer;
	LPDIRECTSOUNDNOTIFY OggNotify;
	bool MusicPlaying;
	bool OggTerminated;
	HANDLE OggStopStream;
	HANDLE OggNotifyHdl[2];
	HANDLE OggThreadHdl;
	FILE* OggFile;
	OggVorbis_File OggStruct;

	CRITICAL_SECTION CritSect;
	CRITICAL_SECTION OggCrit;

	long OggThread(void);

	THashPool SoundPool;

public:
	TSoundEngine(void);
	~TSoundEngine(void);
	bool Create( HWND hWnd, std::string &errMsg );
	void SetMusicVolume(float NewVolume);
	void SetSoundVolume(float NewVolume);
	void PlayOggStream(const char *FileName);
	void StopOggStream(void);

	//general fx
	/*note : setting up an fx on the secondary buff should be ok, 
	con->use of dmo also make us highly directx dependent
	pro-> easy adding of new effect with dmo
	*/
	//void SetFx(int FxType); 
	

	TSoundObject* GetSoundObject(const char* SoundName);
};

extern TSoundEngine *SoundEngine;

#endif
#ifndef SPRITEDATABASE_H
#define SPRITEDATABASE_H

#include "Headers.h"
#include "Lock.h"
#include "HashPool.h"
#include "GfxEngine.h" 

#pragma pack(push)
#pragma pack(4)
struct SiHeader
{
	char			Signature[4];
	unsigned long   Version;
	unsigned long   IdxCount;
	unsigned long	Wasted;
};
#pragma pack(pop)

//structure temporaire pour avoir acces au anciennes info
//disparait dans le systeme finalisé
struct OldOffsetStruct
{
	char*			SpriteName;
	char*           PalName;
	short			OffsetX;
	short			OffsetY;
	short			OffsetX2;
	short			OffsetY2;
	unsigned short	Transparency;
	unsigned short	TransColor;
};

struct SiInfo
{
	char*			SpriteName;
	unsigned long	DataOffset;
	unsigned long	DataSize;
	unsigned long	StoreType;
	unsigned long	TextFmt;
	unsigned long	Width;
	unsigned long	Height;
	//internal use
	unsigned long	UseCount; 
	LPDIRECT3DTEXTURE9 Surface;
	OldOffsetStruct*	OldOff;
};

class SpriteDatabase 
{
private:
	bool IndexLoaded;
	HashPool* IndexHash;

	SiHeader SiHdr;

	SiInfo* SiInfoArray;
	OldOffsetStruct* OldOffsetArray;
	OldOffsetStruct Dummy; //to avoid problems

	SiInfo* DummySprite; //to be able to remove a lot of check

	CriticalSection DataBaseLock[8]; //use X different lock to optimize access

	void LoadSurfaceP8As16(SiInfo* SpriteInfo,LPBYTE Data,LPBYTE Pal);
	void LoadSurfaceP8As32(SiInfo* SpriteInfo,LPBYTE Data,LPBYTE Pal);
	void LoadSurfaceRaw(SiInfo* SpriteInfo,LPBYTE Data);

	void LoadSprite_NoComp(SiInfo* SpriteInfo,LPBYTE Pal);
	void LoadSprite_Zlib(SiInfo* SpriteInfo,LPBYTE Pal);
	void LoadSprite_Lzma(SiInfo* SpriteInfo,LPBYTE Pal);

public:
	SpriteDatabase();			// SpriteDatabase Constructor.
	~SpriteDatabase();		// SpriteDatabase Destructor.

	SiInfo* GetIndexEntry(const char* SpriteName);
	void LoadPsi(SiInfo* SpriteInfo); //Load the surface corresponding to the Psi
	void UnloadPsi(SiInfo* SpriteInfo);//unload the Surface with ref count
};

// Unique Global
extern SpriteDatabase SpriteDb;


//multithreaded Access to the data file
struct FileAccessLock
{
	HANDLE File;
	CriticalSection Lock;
};

const int MultiDataCount=4;
//Chaotikmind 4 barrels data gun TM
class TMultiDataAccess
{
private:
	FileAccessLock FileAccess[MultiDataCount];
	volatile unsigned long HelperIndex;
public:
	TMultiDataAccess(void);
	~TMultiDataAccess(void);
	void ReadData(void* Buffer,const unsigned long Position,const unsigned long Count);
};

#endif


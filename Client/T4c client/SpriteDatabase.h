#ifndef SPRITEDATABASE_H
#define SPRITEDATABASE_H

// Include File.
#include "HashPool.h"
#include "GfxEngine.h" 

#pragma pack(push)
#pragma pack(4)
struct TSiHeader
{
	char			Signature[4];
	unsigned long   Version;
	unsigned long   IdxCount;
	unsigned long	Wasted;
};
#pragma pack(pop)

const unsigned long StoreType_NoComp=0;
const unsigned long StoreType_Zlib=1;
const unsigned long StoreType_Lzma=2;

const unsigned long TextFmt_A8R8G8B8=0;
const unsigned long TextFmt_R5G6B5=1;
const unsigned long TextFmt_A1R5G5B5=2;  
const unsigned long TextFmt_P8=3;
const unsigned long TextFmt_DXT1A=4;
const unsigned long TextFmt_DXT5=5;

//structure temporaire pour avoir acces au anciennes info
//disparait dans le systeme finalisé
struct TOldOffset
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

typedef TOldOffset* POldOffset;

struct TSiInfo
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
	POldOffset		OldOff;
};

typedef TSiInfo* PSiInfo;


//V2 Sprite structure
//CM deplacé ici pour eviter de se prendre la tete avec includes
//ca va disparaitre de toute facon..
struct TSpriteData 
{
	LPBYTE              Chunk;   // Chunck of data to be decompresed.
	LPDIRECT3DTEXTURE9 Surface; // A DirectDraw Surface.
};
typedef TSpriteData *PSpriteData;

class TSpriteDatabase 
{
private:
	bool IndexLoaded;
	THashPool* IndexHash;

	TSiHeader SiHeader;

	PSiInfo SiInfoArray;
	POldOffset OldOffsetArray;
	TOldOffset Dummy; //to avoid problems

	PSiInfo DummySprite; //to be able to remove a lot of check

	CRITICAL_SECTION DataBaseLock[8]; //use 10 different lock to optimize access

	void LoadSurfaceP8As16(PSiInfo SpriteInfo,LPBYTE Data,LPBYTE Pal);
	void LoadSurfaceP8As32(PSiInfo SpriteInfo,LPBYTE Data,LPBYTE Pal);
	void LoadSurfaceRaw(PSiInfo SpriteInfo,LPBYTE Data);

	void LoadSprite_NoComp(PSiInfo SpriteInfo,LPBYTE Pal);
	void LoadSprite_Zlib(PSiInfo SpriteInfo,LPBYTE Pal);
	void LoadSprite_Lzma(PSiInfo SpriteInfo,LPBYTE Pal);

public:
	TSpriteDatabase();			// TSpriteDatabase Constructor.
	~TSpriteDatabase();		// TSpriteDatabase Destructor.

	//bool LoadSprite(const char* SpriteName, PSiInfo *lplpSpriteInfo,const char *Palette); // Load a Sprite Type. //for compat with old system
	//void Release( LPCTSTR lpszID); //compat

	PSiInfo GetIndexEntry(const char* SpriteName);
	void LoadPsi(PSiInfo SpriteInfo); //Load the surface corresponding to the Psi
	void UnloadPsi(PSiInfo SpriteInfo);//unload the Surface with ref count
};

// Unique Global
extern TSpriteDatabase SpriteDb;



//multithreaded Access to the data file
struct TFileAccess
{
	HANDLE File;
	CRITICAL_SECTION Lock;
};

const int MultiDataCount=4;
//Chaotikmind 4 barrels data gun TM
class TMultiDataAccess
{
	private:
		TFileAccess FileAccess[MultiDataCount];
		volatile unsigned long HelperIndex;
	public:
		TMultiDataAccess(void);
		~TMultiDataAccess(void);
		void ReadData(void* Buffer,const unsigned long Position,const unsigned long Count);
};

#endif


#include "Lightning.h"
#include "TimeTracker.h"
#include "hash.h"
#include "FastStream.h"
#include "debug.h"
/*
Global illumination value ref :
Darkness Cave  : 303030

Night Dark blue 9h->5h : 4D56CC
Sunrise 6h : D1B978

*/
LightningManager LightMgr;

LightningManager::LightningManager(void):LightPool(13)
{
	SetLocation(ELightLocation::OpenAir);
	//load the color tables
	FastStream Fst;
	if (!Fst.LoadFromFile(L".//gamefiles//lightloc_openair.dat"))
	{
		LOG("LightningManager : Unable to Load lightloc_openair.dat, Cannot continue\r\n");
		MessageBoxA(0,"Cannot open lightloc_openair.dat","fatal error",MB_OK);
		
	};
	Fst.Read(LightTables[ELightLocation::OpenAir],1440*sizeof(unsigned long));
};

LightningManager::~LightningManager(void)
{

};

void LightningManager::Initialize(void)
{
	LightTexture=GfxCore.CreateTexture(32,32,D3DFMT_A8R8G8B8);
};

unsigned long LightningManager::GetNextLightId(void)
{
	return NextLightId++;
};

void LightningManager::SetLocation(ELightLocation::Enum NewLoc)
{
	Location=NewLoc;
	ActualLightTable=LightTables[Location];
};

void LightningManager::ClearLights(void)
{
};

inline unsigned long LightHash(const unsigned long Type,const int PosX,const int PosY)
{
	return (((unsigned long)PosX<<16)||PosY)^Type;
};

unsigned long LightningManager::AddLight(unsigned long Type,int PosX,int PosY,float Intensity,unsigned long Color)
{
	const unsigned long LHash=LightHash(Type,PosX,PosY);
	void* Lght=LightPool.GetEntry(LHash);
	if (Lght)
		return 0;

	LightStruct *NewLight=new LightStruct;
	NewLight->Color=Color;
	NewLight->LightPosX=PosX;
	NewLight->LightPosY=PosY;
	return LHash;
}
	
void LightningManager::Render(float DeltaTime)
{
	GeneralLightColor=ActualLightTable[GameTime.GetTimeOfDayInMinute()];
	//need a copy of backbuffer
	//1st render global illumination
	GfxCore.DrawSprite(0.0f,0.0f,0.1f,(float)(GfxCore.ScreenWidth()-1),(float)(GfxCore.ScreenHeight()-1),GeneralLightColor,GfxCore.GetBackBufferTexture());
/*
	LightTexture could be a rendertarget
	we render little halo texture on it modulated by the light color
	(also modulated by global illum ?)

	*/
};

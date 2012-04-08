#ifndef LIGHTNING_H
#define LIGHTNING_H

#include "GfxEngine.h"
#include "HashPool.h"

enum TLightLocation
{
	LightLoc_OpenAir,	//standard lighttable
	LightLoc_Forest,	//darker standard lighttable
	LightLoc_Cave,		//very dark and greenish gloom at night
	LightLoc_CaveMagma, //changing shade of red
	LightLoc_Count
};

typedef unsigned long TLightTable[1440];

struct TLightStruct
{
	int LightPosX; //Woorld coord of the light in PIXEL
	int LightPosY;
	float Depth;  //depth of the light for zbuffering
	unsigned long Color;
	unsigned long Radius;
	int LightType; //Torch,omni,Spot, Player radiance
};

class TLightningManager
{
private:
	unsigned long NextLightId;
	unsigned long GetNextLightId(void);

	THashPool LightPool;

	TLightLocation Location;
	TLightTable LightTables[LightLoc_Count];

	unsigned long* ActualLightTable;
	unsigned long GeneralLightColor; //depends of the location/time

	//the Lightning texture
	LPDIRECT3DTEXTURE9 LightTexture;
public:
	TLightningManager(void);
	~TLightningManager(void);
	void Initialize(void); //need the gfxengine running

	void SetLocation(TLightLocation NewLoc);
	void ClearLights(void);
	unsigned long AddLight(unsigned long Type,int PosX,int PosY,float Intensity,unsigned long Color);//return a unique identifier to that light
	
	void Render(float DeltaTime);
};

extern TLightningManager LightMgr;
#endif

#ifndef LIGHTNING_H
#define LIGHTNING_H

#include "GfxEngine.h"
#include "HashPool.h"

struct ELightLocation
{
	enum Enum
	{
		OpenAir,	//standard lighttable
		Forest,	//darker standard lighttable
		Cave,		//very dark and greenish gloom at night
		CaveMagma, //changing shade of red
		Count
	};
};

typedef unsigned long TLightTable[1440];

struct LightStruct
{
	int LightPosX; //Woorld coord of the light in PIXEL
	int LightPosY;
	float Depth;  //depth of the light for zbuffering
	unsigned long Color;
	unsigned long Radius;
	int LightType; //Torch,omni,Spot, Player radiance
};

class LightningManager
{
private:
	unsigned long NextLightId;
	unsigned long GetNextLightId(void);

	HashPool LightPool;

	ELightLocation::Enum Location;
	TLightTable LightTables[ELightLocation::Count];

	unsigned long* ActualLightTable;
	unsigned long GeneralLightColor; //depends of the location/time

	//the Lightning texture
	LPDIRECT3DTEXTURE9 LightTexture;
public:
	LightningManager(void);
	~LightningManager(void);
	void Initialize(void); //need the gfxengine running

	void SetLocation(ELightLocation::Enum NewLoc);
	void ClearLights(void);
	unsigned long AddLight(unsigned long Type,int PosX,int PosY,float Intensity,unsigned long Color);//return a unique identifier to that light
	
	void Render(float DeltaTime);
};

extern LightningManager LightMgr;
#endif

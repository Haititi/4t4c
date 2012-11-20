#ifndef SKINLOADER_H
#define SKINLOADER_H

#include "Headers.h"
#include "HashPool.h"
#include "SkinTypes.h"

struct ESkinType
{
	enum Enum
	{
		Void=0,
		Item,
		Monster,
		PuppetPart,
		Puppet,
		FORCE_DWORD=0x7FFFFFFF
	};
};

class SkinRenderer
{
protected:
	unsigned long SkinType;
	unsigned long LastAction;
	unsigned long ActualFrame;
	unsigned long ThisSkinId;
	float TimeAccu;

	//hit test vars
	float LastBaseX,LastBaseY; //last draw position
public:
	SkinRenderer(HashPool* HashPool,const unsigned long SkinId)
	{	
		SkinType=ESkinType::Void; 
		ThisSkinId=SkinId;
		LastAction=0;
		ActualFrame=0;
		TimeAccu=0;
		LastBaseX=LastBaseY=0.0f;
	};
	virtual ~SkinRenderer(void){};
	unsigned long GetSkinId(void){return ThisSkinId;};
	unsigned long GetSkinType(void){return SkinType;};
	virtual float GetAnimationDelay(void){return 0.033f;};
	virtual bool HitTest(const int Mx,const int My){return false;};
	virtual bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected){return false;}; //render the skin at X,y pixel coordinate
};

class SkinRendererItem : public SkinRenderer
{
private:
	SkinInfo_Item* SkinInfo;
public:
	SkinRendererItem(HashPool* HashPool,const unsigned long SkinId);
	~SkinRendererItem(void);
	float GetAnimationDelay(void);
	bool HitTest(const int Mx,const int My);
	bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected); //render the skin at X,y pixel coordinate
};

class SkinRendererMonster : public SkinRenderer
{
private:
	SkinInfo_Monster* SkinInfo;
	TFrame* LastFrame;
public:
	SkinRendererMonster(HashPool* HashPool,const unsigned long SkinId);
	~SkinRendererMonster(void);
	float GetAnimationDelay(void);
	bool HitTest(const int Mx,const int My);
	bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected); //render the skin at X,y pixel coordinate
};

class SkinRendererPuppetPart : public SkinRenderer
{
private:
	SkinInfo_Puppet* SkinInfo;
public:
	SkinRendererPuppetPart(HashPool* HashPool,const unsigned long SkinId);
	~SkinRendererPuppetPart(void);
	float GetAnimationDelay(void);
	bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected); //render the skin at X,y pixel coordinate
};

class SkinRendererPuppet : public SkinRenderer
{
private:
	HashPool* SkinPool;
	SkinRendererPuppetPart *PuppetSkin[EPuppetPartType::Count]; 
	unsigned long Female;
public:
	SkinRendererPuppet(HashPool* HashPool,const unsigned long SkinId);
	~SkinRendererPuppet(void);
	float GetAnimationDelay(void);
	bool HitTest(const int Mx,const int My);
	bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected); //render the skin at X,y pixel coordinate
	void ChangePart(const EPuppetPartType::Enum EquipPos,const unsigned short PartId);
};

class SkinLoader
{
private:
	SkinInfo_Item* ItemSkinArray;
	HashPool* ItemSkinHash;

	SkinInfo_Monster* MonsterSkinArray;
	HashPool* MonsterSkinHash;

	SkinInfo_Puppet* PuppetSkinArray;
	HashPool* PuppetSkinHash;

	SkinRendererPuppet *NakedPuppetMale,*NakedPuppetFemale; //to draw underlying naked sprite
public:
	SkinLoader(void);
	~SkinLoader(void);
	void Initialize(void);
	SkinRenderer* GetSkin(const unsigned long SkinId); //return a skin object that handle the drawing of that skin type
};

extern SkinLoader SkinLdr;

#endif
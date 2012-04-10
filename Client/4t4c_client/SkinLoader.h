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

class TSkinRenderer
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
	TSkinRenderer(THashPool* HashPool,const unsigned long SkinId)
	{	
		SkinType=ESkinType::Void; 
		ThisSkinId=SkinId;
		LastAction=0;
		ActualFrame=0;
		TimeAccu=0;
		LastBaseX=LastBaseY=0.0f;
	};
	virtual ~TSkinRenderer(void){};
	unsigned long GetSkinId(void){return ThisSkinId;};
	unsigned long GetSkinType(void){return SkinType;};
	virtual float GetAnimationDelay(void){return 0.033f;};
	virtual bool HitTest(const int Mx,const int My){return false;};
	virtual bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected){return false;}; //render the skin at X,y pixel coordinate
};

class TSkinRendererItem : public TSkinRenderer
{
private:
	TItemSkinInfo* SkinInfo;
public:
	TSkinRendererItem(THashPool* HashPool,const unsigned long SkinId);
	~TSkinRendererItem(void);
	float GetAnimationDelay(void);
	bool HitTest(const int Mx,const int My);
	bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected); //render the skin at X,y pixel coordinate
};

class TSkinRendererMonster : public TSkinRenderer
{
private:
	TMonsterSkinInfo* SkinInfo;
	TFrame* LastFrame;
public:
	TSkinRendererMonster(THashPool* HashPool,const unsigned long SkinId);
	~TSkinRendererMonster(void);
	float GetAnimationDelay(void);
	bool HitTest(const int Mx,const int My);
	bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected); //render the skin at X,y pixel coordinate
};

class TSkinRendererPuppetPart : public TSkinRenderer
{
private:
	TPuppetSkinInfo* SkinInfo;
public:
	TSkinRendererPuppetPart(THashPool* HashPool,const unsigned long SkinId);
	~TSkinRendererPuppetPart(void);
	float GetAnimationDelay(void);
	bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected); //render the skin at X,y pixel coordinate
};

class TSkinRendererPuppet : public TSkinRenderer
{
private:
	THashPool* SkinPool;
	TSkinRendererPuppetPart *PuppetSkin[PuppetPart_Count]; 
	unsigned long Female;
public:
	TSkinRendererPuppet(THashPool* HashPool,const unsigned long SkinId);
	~TSkinRendererPuppet(void);
	float GetAnimationDelay(void);
	bool HitTest(const int Mx,const int My);
	bool Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected); //render the skin at X,y pixel coordinate
	void ChangePart(const TPuppetPartType EquipPos,const unsigned short PartId);
};

class TSkinLoader
{
private:
	TItemSkinInfo* ItemSkinArray;
	THashPool* ItemSkinHash;

	TMonsterSkinInfo* MonsterSkinArray;
	THashPool* MonsterSkinHash;

	TPuppetSkinInfo* PuppetSkinArray;
	THashPool* PuppetSkinHash;

	TSkinRendererPuppet *NakedPuppetMale,*NakedPuppetFemale; //to draw underlying naked sprite
public:
	TSkinLoader(void);
	~TSkinLoader(void);
	void Initialize(void);
	TSkinRenderer* GetSkin(const unsigned long SkinId); //return a skin object that handle the drawing of that skin type
};

extern TSkinLoader SkinLoader;

#endif
#ifndef SKINTYPES_H
#define SKINTYPES_H

#include "SpriteDatabase.h"

class SoundObject;

struct TFrame
{
	char* SpriteName;
	PSiInfo SpriteInfo; //link to the database sprite
	short OffsetX;//drawing offsets
	short OffsetY;
};
typedef TFrame* PFrame; 

struct TFrameList
{
	unsigned long Reversed;
	unsigned long GraphCount;
	PFrame FrameList;
};

struct TDirection
{
	TFrameList Direction[8];
};

struct TSkinSoundInfo
{
	char* SoundName;
	SoundObject* SndObj;
	//PSoundBuf//link To the database corresponding sound
	float PitchDev; //range of pitch modification
};
typedef TSkinSoundInfo* PSkinSoundInfo;

struct TItemSkinInfo
{
	char* SkinName;
	unsigned short SkinId;
	unsigned long SkinType;
	unsigned long MouseCursor;
	unsigned long ColorFx;
	unsigned long VertexColor;
	float AnimDelay;
	char* OpenSound;
	char* CloseSound;
	unsigned long GraphCount;
	PFrame GraphList;
};

struct TMonsterSkinInfo
{
	char* SkinName;
	unsigned short Id;
	unsigned long VertexColor;
	unsigned long ColorFx;
	float AnimDelay;
	float MonsterSize;
	TDirection Walk;
	TDirection Attack;
	TFrameList Death;
	unsigned long AtkSoundCount;
	PSkinSoundInfo AtkSounds;
	unsigned long HitSndCount;
	PSkinSoundInfo HitSounds;
	unsigned long DieSndCount;
	PSkinSoundInfo DieSounds;
	unsigned long IdleSndCount;
	PSkinSoundInfo IdleSounds;
};

const unsigned long FemaleSkinMask=0x80000000;

enum TPuppetPartType
{
	PuppetPart_HandLeft,	//hand or gloves
	PuppetPart_HandRight,
	PuppetPart_ArmLeft,		// allways Arm ?
	PuppetPart_ArmRight,
	PuppetPart_Feets,		//feets or boot
	PuppetPart_Legs,		//legs or legging
	PuppetPart_Body,		//body or armor
	PuppetPart_Head,		//head
	PuppetPart_Hat,			//Air or hat ?  or air  and hat ?
	
	PuppetPart_Weapon,		//over HandLEft
	PuppetPart_Shield,		//over handRight
	PuppetPart_Boot,		//maybe not needed (don't need to draw feet if boot are present)
	PuppetPart_Cape,		//cape or wings
	
	PuppePart_Mask,			//over Head

	PuppetPart_Count
	//PuppetPart_RobeUnder, //
	//PuppetPart_Weapon2,
	//PuppetPart_Hat,
};

struct TPuppetSkinInfo
{
	char* SkinName;
	unsigned short Id;
	unsigned long Female; //0 is male 1 female
	unsigned long VertexColor;
	unsigned long ColorFx;
	float AnimDelay;
	TDirection Walk;
	TDirection Attack;
	TDirection Range;
	TFrameList Death;
};

#endif
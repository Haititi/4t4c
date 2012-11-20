#ifndef SKINTYPES_H
#define SKINTYPES_H

#include "SpriteDatabase.h"

class SoundObject;

struct TFrame
{
	char* SpriteName;
	SiInfo* SpriteInfo; //link to the database sprite
	short OffsetX;//drawing offsets
	short OffsetY;
};

struct TFrameList
{
	unsigned long Reversed;
	unsigned long GraphCount;
	TFrame* FrameList;
};

struct TDirection
{
	TFrameList Direction[8];
};

struct SkinInfo_Sound
{
	char* SoundName;
	SoundObject* SndObj;
	//PSoundBuf//link To the database corresponding sound
	float PitchDev; //range of pitch modification
};

struct SkinInfo_Item
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
	TFrame* GraphList;
};

struct SkinInfo_Monster
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
	SkinInfo_Sound* AtkSounds;
	unsigned long HitSndCount;
	SkinInfo_Sound* HitSounds;
	unsigned long DieSndCount;
	SkinInfo_Sound* DieSounds;
	unsigned long IdleSndCount;
	SkinInfo_Sound* IdleSounds;
};

const unsigned long FemaleSkinMask=0x80000000;

struct EPuppetPartType
{
	enum Enum
	{
		HandLeft,	//hand or gloves
		HandRight,
		ArmLeft,		// allways Arm ?
		ArmRight,
		Feets,		//feets or boot
		Legs,		//legs or legging
		Body,		//body or armor
		Head,		//head
		Hat,			//Air or hat ?  or air  and hat ?
	
		Weapon,		//over HandLEft
		Shield,		//over handRight
		Boot,		//maybe not needed (don't need to draw feet if boot are present)
		Cape,		//cape or wings
	
		Mask,			//over Head

		Count
		//RobeUnder, //
		//Weapon2,
		//Hat,
	};
};

struct SkinInfo_Puppet
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
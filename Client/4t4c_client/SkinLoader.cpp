#include "SkinLoader.h"
#include "FastStream.h"
#include "Hash.h"
#include "GameObject.h"

SkinLoader SkinLdr;

//our direction organisation is not the same as the old one
//TODO normalize that BS
//const int DirTab2[9]={1,8,7,6,5,4,3,2,1};
//const int DirConVert[9]={8,7,6,5,4,3,2,1,0};

SkinLoader::SkinLoader(void)
{
	ItemSkinHash=new HashPool(1009);
	MonsterSkinHash=new HashPool(1009);
	PuppetSkinHash=new HashPool(1009);
};

SkinLoader::~SkinLoader(void)
{

	delete ItemSkinHash;
	delete MonsterSkinHash;
	delete PuppetSkinHash;
};

void ReadFrameFromStream(TFrame &Frame,FastStream &Strm)
{
	Frame.SpriteName=Strm.ReadWordString();
	Frame.SpriteInfo=SpriteDb.GetIndexEntry(Frame.SpriteName);
	Frame.OffsetX=Strm.ReadWord();
	Frame.OffsetY=Strm.ReadWord();
};

void SkinLoader::Initialize(void)
{
	FastStream FstItem,FstMonst,FstPuppet;
	unsigned long Version,Count;

	FstItem.LoadFromFile(L".\\GameFiles\\ItemSkinIdInfo.dat");

	Version=FstItem.ReadLong();
	Count=FstItem.ReadLong();
	ItemSkinArray=new SkinInfo_Item[Count];
	for (unsigned long i=0;i<Count;i++)
	{
		ItemSkinArray[i].SkinName=FstItem.ReadWordString();
		ItemSkinArray[i].SkinId=FstItem.ReadWord();
		ItemSkinArray[i].SkinType=FstItem.ReadLong();
		ItemSkinArray[i].MouseCursor=FstItem.ReadLong();
		ItemSkinArray[i].ColorFx=FstItem.ReadLong();
		ItemSkinArray[i].VertexColor=FstItem.ReadLong();
		ItemSkinArray[i].AnimDelay=FstItem.ReadFloat();
		ItemSkinArray[i].OpenSound=FstItem.ReadWordString();
		ItemSkinArray[i].CloseSound=FstItem.ReadWordString();
		ItemSkinArray[i].GraphCount=FstItem.ReadLong();
		ItemSkinArray[i].GraphList=new TFrame[ItemSkinArray[i].GraphCount];
		for (unsigned long j=0;j<ItemSkinArray[i].GraphCount;j++)
		{
			ReadFrameFromStream(ItemSkinArray[i].GraphList[j],FstItem);
		}
		ItemSkinHash->AddHashEntry(RandHash(ItemSkinArray[i].SkinId),&ItemSkinArray[i]);
	}
	
	FstMonst.LoadFromFile(L".\\GameFiles\\MonsterSkinIdInfo.dat");

	Version=FstMonst.ReadLong();
	Count=FstMonst.ReadLong();
	MonsterSkinArray=new SkinInfo_Monster[Count];
	for (unsigned long i=0;i<Count;i++)
	{
		MonsterSkinArray[i].SkinName=FstMonst.ReadWordString();
		MonsterSkinArray[i].Id=FstMonst.ReadWord();
		MonsterSkinArray[i].VertexColor=FstMonst.ReadLong();
		MonsterSkinArray[i].ColorFx=FstMonst.ReadLong();
		MonsterSkinArray[i].AnimDelay=FstMonst.ReadFloat();
		MonsterSkinArray[i].MonsterSize=FstMonst.ReadFloat();
		for (unsigned long j=0;j<8;j++)
		{
			//TDirection Walk;
			MonsterSkinArray[i].Walk.Direction[j].Reversed=FstMonst.ReadLong();
			MonsterSkinArray[i].Walk.Direction[j].GraphCount=FstMonst.ReadLong();
			MonsterSkinArray[i].Walk.Direction[j].FrameList=new TFrame[MonsterSkinArray[i].Walk.Direction[j].GraphCount];
			for (unsigned long k=0;k<MonsterSkinArray[i].Walk.Direction[j].GraphCount;k++)
			{
				ReadFrameFromStream(MonsterSkinArray[i].Walk.Direction[j].FrameList[k],FstMonst);
			}
		}

		for (unsigned long j=0;j<8;j++)
		{
			MonsterSkinArray[i].Attack.Direction[j].Reversed=FstMonst.ReadLong();
			MonsterSkinArray[i].Attack.Direction[j].GraphCount=FstMonst.ReadLong();
			MonsterSkinArray[i].Attack.Direction[j].FrameList=new TFrame[MonsterSkinArray[i].Attack.Direction[j].GraphCount];
			for (unsigned long k=0;k<MonsterSkinArray[i].Attack.Direction[j].GraphCount;k++)
			{
				ReadFrameFromStream(MonsterSkinArray[i].Attack.Direction[j].FrameList[k],FstMonst);
			}
		}

		MonsterSkinArray[i].Death.Reversed=FstMonst.ReadLong();
		MonsterSkinArray[i].Death.GraphCount=FstMonst.ReadLong();
		MonsterSkinArray[i].Death.FrameList=new TFrame[MonsterSkinArray[i].Death.GraphCount];
		for (unsigned long k=0;k<MonsterSkinArray[i].Death.GraphCount;k++)
		{
			ReadFrameFromStream(MonsterSkinArray[i].Death.FrameList[k],FstMonst);
		}

		MonsterSkinArray[i].AtkSoundCount=FstMonst.ReadLong();
		MonsterSkinArray[i].AtkSounds=new SkinInfo_Sound[MonsterSkinArray[i].AtkSoundCount];
		for (unsigned long j=0;j<MonsterSkinArray[i].AtkSoundCount;j++)
		{
			MonsterSkinArray[i].AtkSounds[j].SoundName=FstMonst.ReadWordString();
			MonsterSkinArray[i].AtkSounds[j].SndObj=SoundEngine.GetSoundObject(MonsterSkinArray[i].AtkSounds[j].SoundName);
			MonsterSkinArray[i].AtkSounds[j].PitchDev=FstMonst.ReadFloat();
		}
		MonsterSkinArray[i].HitSndCount=FstMonst.ReadLong();
		MonsterSkinArray[i].HitSounds=new SkinInfo_Sound[MonsterSkinArray[i].HitSndCount];
		for (unsigned long j=0;j<MonsterSkinArray[i].HitSndCount;j++)
		{
			MonsterSkinArray[i].HitSounds[j].SoundName=FstMonst.ReadWordString();
			MonsterSkinArray[i].HitSounds[j].SndObj=SoundEngine.GetSoundObject(MonsterSkinArray[i].HitSounds[j].SoundName);
			MonsterSkinArray[i].HitSounds[j].PitchDev=FstMonst.ReadFloat();
		}
		MonsterSkinArray[i].DieSndCount=FstMonst.ReadLong();
		MonsterSkinArray[i].DieSounds=new SkinInfo_Sound[MonsterSkinArray[i].DieSndCount];
		for (unsigned long j=0;j<MonsterSkinArray[i].DieSndCount;j++)
		{
			MonsterSkinArray[i].DieSounds[j].SoundName=FstMonst.ReadWordString();
			MonsterSkinArray[i].DieSounds[j].SndObj=SoundEngine.GetSoundObject(MonsterSkinArray[i].DieSounds[j].SoundName);
			MonsterSkinArray[i].DieSounds[j].PitchDev=FstMonst.ReadFloat();
		}
		MonsterSkinArray[i].IdleSndCount=FstMonst.ReadLong();
		MonsterSkinArray[i].IdleSounds=new SkinInfo_Sound[MonsterSkinArray[i].IdleSndCount];
		for (unsigned long j=0;j<MonsterSkinArray[i].IdleSndCount;j++)
		{
			MonsterSkinArray[i].IdleSounds[j].SoundName=FstMonst.ReadWordString();
			MonsterSkinArray[i].IdleSounds[j].SndObj=SoundEngine.GetSoundObject(MonsterSkinArray[i].IdleSounds[j].SoundName);
			MonsterSkinArray[i].IdleSounds[j].PitchDev=FstMonst.ReadFloat();
		}

		MonsterSkinHash->AddHashEntry(RandHash(MonsterSkinArray[i].Id),&MonsterSkinArray[i]);
	}

	FstPuppet.LoadFromFile(L".\\GameFiles\\PuppetSkinIdInfo.dat");

	Version=FstPuppet.ReadLong();
	Count=FstPuppet.ReadLong();
	PuppetSkinArray=new SkinInfo_Puppet[Count];
	for (unsigned long i=0;i<Count;i++)
	{
		
		PuppetSkinArray[i].SkinName=FstPuppet.ReadWordString();
		PuppetSkinArray[i].Id=FstPuppet.ReadWord();
		PuppetSkinArray[i].Female=FstPuppet.ReadLong();
		PuppetSkinArray[i].VertexColor=FstPuppet.ReadLong();
		PuppetSkinArray[i].ColorFx=FstPuppet.ReadLong();
		PuppetSkinArray[i].AnimDelay=FstPuppet.ReadFloat();

		for (unsigned int j=0;j<8;j++)
		{
			PuppetSkinArray[i].Walk.Direction[j].Reversed=FstPuppet.ReadLong();
			PuppetSkinArray[i].Walk.Direction[j].GraphCount=FstPuppet.ReadLong();
			PuppetSkinArray[i].Walk.Direction[j].FrameList=new TFrame[PuppetSkinArray[i].Walk.Direction[j].GraphCount];
			for (unsigned int k=0;k<PuppetSkinArray[i].Walk.Direction[j].GraphCount;k++)
			{
				ReadFrameFromStream(PuppetSkinArray[i].Walk.Direction[j].FrameList[k],FstPuppet);
			}
		}

		for (unsigned int j=0;j<8;j++)
		{
			PuppetSkinArray[i].Attack.Direction[j].Reversed=FstPuppet.ReadLong();
			PuppetSkinArray[i].Attack.Direction[j].GraphCount=FstPuppet.ReadLong();
			PuppetSkinArray[i].Attack.Direction[j].FrameList=new TFrame[PuppetSkinArray[i].Attack.Direction[j].GraphCount];
			for (unsigned int k=0;k<PuppetSkinArray[i].Attack.Direction[j].GraphCount;k++)
			{
				ReadFrameFromStream(PuppetSkinArray[i].Attack.Direction[j].FrameList[k],FstPuppet);
			}
		}

		for (unsigned int j=0;j<8;j++)
		{
			PuppetSkinArray[i].Range.Direction[j].Reversed=FstPuppet.ReadLong();
			PuppetSkinArray[i].Range.Direction[j].GraphCount=FstPuppet.ReadLong();
			PuppetSkinArray[i].Range.Direction[j].FrameList=new TFrame[PuppetSkinArray[i].Range.Direction[j].GraphCount];
			for (unsigned int k=0;k<PuppetSkinArray[i].Range.Direction[j].GraphCount;k++)
			{
				ReadFrameFromStream(PuppetSkinArray[i].Range.Direction[j].FrameList[k],FstPuppet);
			}
		}


		PuppetSkinArray[i].Death.Reversed=FstPuppet.ReadLong();
		PuppetSkinArray[i].Death.GraphCount=FstPuppet.ReadLong();
		PuppetSkinArray[i].Death.FrameList=new TFrame[PuppetSkinArray[i].Death.GraphCount];
		for (unsigned int k=0;k<PuppetSkinArray[i].Death.GraphCount;k++)
		{
			ReadFrameFromStream(PuppetSkinArray[i].Death.FrameList[k],FstPuppet);
		}

		if (PuppetSkinArray[i].Female==1)
			PuppetSkinHash->AddHashEntry(RandHash(((unsigned long)PuppetSkinArray[i].Id)|FemaleSkinMask),&PuppetSkinArray[i]);
		else
			PuppetSkinHash->AddHashEntry(RandHash((unsigned long)PuppetSkinArray[i].Id),&PuppetSkinArray[i]);
	}

		//TODO Initialize the ref naked puppet
	//NakedPuppetMale=(SkinRendererPuppet*)GetSkin(20010);
	//NakedPuppetFemale=(SkinRendererPuppet*)GetSkin(20011);
};
	
SkinRenderer* SkinLoader::GetSkin(const unsigned long SkinId)
{
	SkinRenderer* Result=0;
	if (SkinId<10000)
	{
		//Item skin
		Result=new SkinRendererItem(ItemSkinHash,SkinId);
	} else
	if ((SkinId==10011) || (SkinId==10012))
	{ 
		//puppet skin
		Result=new SkinRendererPuppet(PuppetSkinHash,SkinId); 
	} else
	if (SkinId<30000)
	{
		//monster skin
		Result=new SkinRendererMonster(MonsterSkinHash,SkinId);
	}
	//should we send a dummmy object or null if nothing is found ???
	return Result;//new SkinRenderer;
}; 



//item skin class

SkinRendererItem::SkinRendererItem(HashPool* HashPool,const unsigned long SkinId):SkinRenderer(HashPool,SkinId)
{
	SkinType=ESkinType::Item;

	SkinInfo=(SkinInfo_Item*)HashPool->GetEntry(RandHash(SkinId));

	if (SkinInfo!=0)
	{
		//load the Ressources
		for (unsigned long i=0;i<SkinInfo->GraphCount;i++)
		{
			SpriteDb.LoadPsi(SkinInfo->GraphList[i].SpriteInfo);
		}

		//TODO load the sounds
	}
};

SkinRendererItem::~SkinRendererItem(void)
{
	if (SkinInfo!=0)
	{
		//unload the Ressources
		for (unsigned long i=0;i<SkinInfo->GraphCount;i++)
		{
			SpriteDb.UnloadPsi(SkinInfo->GraphList[i].SpriteInfo);
		}
	}
};


float SkinRendererItem::GetAnimationDelay(void)
{
	if (SkinInfo!=0)
	{
		return SkinInfo->AnimDelay;
	}
	return 0.33f;
};

bool SkinRendererItem::HitTest(const int Mx,const int My)
{
	if (SkinInfo!=0)
	{
		const TFrame &Frame=SkinInfo->GraphList[0];
		if  ( (Mx>LastBaseX) && (Mx<LastBaseX+Frame.SpriteInfo->Width) &&
			  (My>LastBaseY) && (My<LastBaseY+Frame.SpriteInfo->Height))
		{
			return GfxCore.PixelHitTest(SkinInfo->GraphList[0].SpriteInfo->Surface,Mx-(int)LastBaseX,My-(int)LastBaseY);
		}
	}
	return false;
};

bool SkinRendererItem::Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected)
{
	//determine which sprite to draw
	if (SkinInfo!=0)
	{
		const TFrame &Frame=SkinInfo->GraphList[0];
		LastBaseX=BaseXPos+Frame.OffsetX;
		LastBaseY=BaseYPos+Frame.OffsetY;
		GfxCore.DrawSprite(LastBaseX,LastBaseY,Depth,(float)Frame.SpriteInfo->Width,(float)Frame.SpriteInfo->Height,SkinInfo->VertexColor,Frame.SpriteInfo->Surface);
	}
	return false;
}; 

//monster Skin Class
SkinRendererMonster::SkinRendererMonster(HashPool* HashPool,const unsigned long SkinId):SkinRenderer(HashPool,SkinId)
{
	SkinType=ESkinType::Monster;
	LastFrame=0;

	SkinInfo=(SkinInfo_Monster*)HashPool->GetEntry(RandHash(SkinId));

	if (SkinInfo!=0)
	{
		//load the Ressources
		for (unsigned long i=0;i<8;i++)
		{
			for (unsigned long j=0;j<SkinInfo->Walk.Direction[i].GraphCount;j++)
			{
				SpriteDb.LoadPsi(SkinInfo->Walk.Direction[i].FrameList[j].SpriteInfo);
			}
			for (unsigned long j=0;j<SkinInfo->Attack.Direction[i].GraphCount;j++)
			{
				SpriteDb.LoadPsi(SkinInfo->Attack.Direction[i].FrameList[j].SpriteInfo);
			}
		}

		for (unsigned long j=0;j<SkinInfo->Death.GraphCount;j++)
		{
			SpriteDb.LoadPsi(SkinInfo->Death.FrameList[j].SpriteInfo);
		}
		//TODO load the sounds
	}
};
	
SkinRendererMonster::~SkinRendererMonster(void)
{
	if (SkinInfo!=0)
	{
		//unload the Ressources
		for (unsigned long i=0;i<8;i++)
		{
			for (unsigned long j=0;j<SkinInfo->Walk.Direction[i].GraphCount;j++)
			{
				SpriteDb.UnloadPsi(SkinInfo->Walk.Direction[i].FrameList[j].SpriteInfo);
			}
			for (unsigned long j=0;j<SkinInfo->Attack.Direction[i].GraphCount;j++)
			{
				SpriteDb.UnloadPsi(SkinInfo->Attack.Direction[i].FrameList[j].SpriteInfo);
			}
		}

		for (unsigned long j=0;j<SkinInfo->Death.GraphCount;j++)
		{
			SpriteDb.UnloadPsi(SkinInfo->Death.FrameList[j].SpriteInfo);
		}
		//TODO unload the sounds
	}
};
	
float SkinRendererMonster::GetAnimationDelay(void)
{
	if (SkinInfo!=0)
	{
		return SkinInfo->AnimDelay;
	}	
	return 0.33f;
};

bool SkinRendererMonster::HitTest(const int Mx,const int My)
{
	if (SkinInfo!=0)
	{
		if (LastFrame!=0)
			if  ( (Mx>LastBaseX) && (Mx<LastBaseX+(float)LastFrame->SpriteInfo->Width) &&
			      (My>LastBaseY) && (My<LastBaseY+(float)LastFrame->SpriteInfo->Height))
				return GfxCore.PixelHitTest(LastFrame->SpriteInfo->Surface,Mx-(int)LastBaseX,My-(int)LastBaseY);
	}
	return false;
};
	
bool SkinRendererMonster::Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected)
{
	bool Result=false;
	if (SkinInfo!=0)
	{
		unsigned long Reverse=unsigned long(Dir>4);

		TimeAccu+=DeltaTime;
		if (TimeAccu>SkinInfo->AnimDelay)
		{
			TimeAccu-=SkinInfo->AnimDelay;
			ActualFrame++;
		}

		if (Action!=LastAction)
		{
			ActualFrame=0;
			LastAction=Action;
		}

		switch (Action)
		{
			case EUnitAction::Stand:
			case EUnitAction::SpellCast:
			{
				LastFrame=&SkinInfo->Walk.Direction[Dir].FrameList[0];
				break;
			}
			case EUnitAction::Walk:
			{
				const unsigned long &MaxFrame=SkinInfo->Walk.Direction[Dir].GraphCount;
				Reverse=SkinInfo->Walk.Direction[Dir].Reversed;
				LastFrame=&SkinInfo->Walk.Direction[Dir].FrameList[ActualFrame%MaxFrame];
				Result=((ActualFrame%MaxFrame)==MaxFrame-1);
				break;
			}
			case EUnitAction::AtkBow:
			case EUnitAction::AtkStand:
			{
				const unsigned long &MaxFrame=SkinInfo->Attack.Direction[Dir].GraphCount;
				Reverse=SkinInfo->Attack.Direction[Dir].Reversed;
				LastFrame=&SkinInfo->Attack.Direction[Dir].FrameList[ActualFrame%MaxFrame];
				Result=((ActualFrame%MaxFrame)==MaxFrame-1);
				break;
			}
			case EUnitAction::Die:
			{
				const unsigned long &MaxFrame=SkinInfo->Death.GraphCount;
				if (ActualFrame>MaxFrame-1)
				{
					ActualFrame=MaxFrame-1;
					Result=true;
				}
				LastFrame=&SkinInfo->Death.FrameList[ActualFrame];
				//TODO we could also make a fade out here when ActualFrame>MAxframe 
			}
			default:
			{
				LastFrame=&SkinInfo->Walk.Direction[0].FrameList[0];
			}
		};
		LastBaseX=BaseXPos+(float)LastFrame->OffsetX;
		LastBaseY=BaseYPos+(float)LastFrame->OffsetY;
		if (Reverse)
			GfxCore.DrawSpriteFlip(LastBaseX,LastBaseY,Depth,(float)LastFrame->SpriteInfo->Width,(float)LastFrame->SpriteInfo->Height,SkinInfo->VertexColor,LastFrame->SpriteInfo->Surface);
		else
			GfxCore.DrawSprite(LastBaseX,LastBaseY,Depth,(float)LastFrame->SpriteInfo->Width,(float)LastFrame->SpriteInfo->Height,SkinInfo->VertexColor,LastFrame->SpriteInfo->Surface);
	}
	return Result;
};

//puppet part class

SkinRendererPuppetPart::SkinRendererPuppetPart(HashPool* HashPool,const unsigned long SkinId):SkinRenderer(HashPool,SkinId)
{
	SkinType=ESkinType::PuppetPart;

	SkinInfo=(SkinInfo_Puppet*)HashPool->GetEntry(RandHash(SkinId));

	if (SkinInfo!=0)
	{
		//load the Ressources
		for (unsigned long i=0;i<8;i++)
		{
			for (unsigned long j=0;j<SkinInfo->Walk.Direction[i].GraphCount;j++)
			{
				SpriteDb.LoadPsi(SkinInfo->Walk.Direction[i].FrameList[j].SpriteInfo);
			}
			for (unsigned long j=0;j<SkinInfo->Attack.Direction[i].GraphCount;j++)
			{
				SpriteDb.LoadPsi(SkinInfo->Attack.Direction[i].FrameList[j].SpriteInfo);
			}
			for (unsigned long j=0;j<SkinInfo->Range.Direction[i].GraphCount;j++)
			{
				SpriteDb.LoadPsi(SkinInfo->Range.Direction[i].FrameList[j].SpriteInfo);
			}
		}

		for (unsigned long j=0;j<SkinInfo->Death.GraphCount;j++)
		{
			SpriteDb.LoadPsi(SkinInfo->Death.FrameList[j].SpriteInfo);
		}
		//TODO load the sounds
	}
};
	
SkinRendererPuppetPart::~SkinRendererPuppetPart(void)
{
	if (SkinInfo!=0)
	{
		//load the Ressources
		for (unsigned long i=0;i<8;i++)
		{
			for (unsigned long j=0;j<SkinInfo->Walk.Direction[i].GraphCount;j++)
			{
				SpriteDb.UnloadPsi(SkinInfo->Walk.Direction[i].FrameList[j].SpriteInfo);
			}
			for (unsigned long j=0;j<SkinInfo->Attack.Direction[i].GraphCount;j++)
			{
				SpriteDb.UnloadPsi(SkinInfo->Attack.Direction[i].FrameList[j].SpriteInfo);
			}
			for (unsigned long j=0;j<SkinInfo->Range.Direction[i].GraphCount;j++)
			{
				SpriteDb.UnloadPsi(SkinInfo->Range.Direction[i].FrameList[j].SpriteInfo);
			}
		}

		for (unsigned long j=0;j<SkinInfo->Death.GraphCount;j++)
		{
			SpriteDb.UnloadPsi(SkinInfo->Death.FrameList[j].SpriteInfo);
		}
		//TODO load the sounds
	}
};
	
float SkinRendererPuppetPart::GetAnimationDelay(void)
{
	if (SkinInfo!=0)
	{
		return SkinInfo->AnimDelay;
	}
	return 0.33f;
};
	
bool SkinRendererPuppetPart::Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected)
{
	bool Result=false;
	if (SkinInfo!=0)
	{
		TFrame* Frame;

		TimeAccu+=DeltaTime;
		if (TimeAccu>SkinInfo->AnimDelay)
		{
			TimeAccu-=SkinInfo->AnimDelay;
			ActualFrame++;
		}

		if (Action!=LastAction)
		{
			ActualFrame=0;
			LastAction=Action;
		}

		unsigned long Reverse=unsigned long(Dir>4);

		switch (Action)
		{
			case EUnitAction::Stand:
			case EUnitAction::SpellCast:
			{
				Frame=&SkinInfo->Walk.Direction[Dir].FrameList[0];
				break;
			}
			case EUnitAction::Walk:
			{
				const unsigned long &MaxFrame=SkinInfo->Walk.Direction[Dir].GraphCount;
				Reverse=SkinInfo->Walk.Direction[Dir].Reversed;
				Frame=&SkinInfo->Walk.Direction[Dir].FrameList[ActualFrame%MaxFrame];
				Result=((ActualFrame%MaxFrame)==MaxFrame-1);
				break;
			}
			case EUnitAction::AtkBow:
			{
				const unsigned long &MaxFrame=SkinInfo->Range.Direction[Dir].GraphCount;
				Reverse=SkinInfo->Range.Direction[Dir].Reversed;
				Frame=&SkinInfo->Range.Direction[Dir].FrameList[ActualFrame%MaxFrame];
				Result=((ActualFrame%MaxFrame)==MaxFrame-1);
				break;
			}
			case EUnitAction::AtkStand:
			{
				const unsigned long &MaxFrame=SkinInfo->Attack.Direction[Dir].GraphCount;
				Reverse=SkinInfo->Attack.Direction[Dir].Reversed;
				Frame=&SkinInfo->Attack.Direction[Dir].FrameList[ActualFrame%MaxFrame];
				Result=((ActualFrame%MaxFrame)==MaxFrame-1);
				break;
			}
			case EUnitAction::Die:
			{
				const unsigned long &MaxFrame=SkinInfo->Death.GraphCount;
				if (ActualFrame>MaxFrame-1)
				{
					ActualFrame=MaxFrame-1;
					Result=true;
				}
				Frame=&SkinInfo->Death.FrameList[ActualFrame];
			}
			default:
			{
				Frame=&SkinInfo->Walk.Direction[Dir].FrameList[0];
			}
		}
		if (Reverse)
			GfxCore.DrawSpriteFlip(BaseXPos+Frame->OffsetX,BaseYPos+Frame->OffsetY,Depth,(float)Frame->SpriteInfo->Width,(float)Frame->SpriteInfo->Height,SkinInfo->VertexColor,Frame->SpriteInfo->Surface);
		else
			GfxCore.DrawSprite(BaseXPos+Frame->OffsetX,BaseYPos+Frame->OffsetY,Depth,(float)Frame->SpriteInfo->Width,(float)Frame->SpriteInfo->Height,SkinInfo->VertexColor,Frame->SpriteInfo->Surface);
		
	}
	return Result;
};
//puppet skin class

SkinRendererPuppet::SkinRendererPuppet(HashPool* HashPool,const unsigned long SkinId):SkinRenderer(HashPool,SkinId)
{
	SkinType=ESkinType::Puppet;
	SkinPool=HashPool;
	Female=0;
	if (SkinId == 10011)
		Female=FemaleSkinMask;
	for (int i=0;i<EPuppetPartType::Count;i++)
		PuppetSkin[i]=new SkinRendererPuppetPart(SkinPool,(i+1)|Female);
};
	
SkinRendererPuppet::~SkinRendererPuppet(void)
{
	for (unsigned long i=0;i<EPuppetPartType::Count;i++)
	{
		if (PuppetSkin[i]!=0)
		{
			delete PuppetSkin[i];
		}
	};
};
	
float SkinRendererPuppet::GetAnimationDelay(void)
{
	if (PuppetSkin[0]!=0)
	{
		return PuppetSkin[0]->GetAnimationDelay();
	}
	return 0.33f;
};
	
bool SkinRendererPuppet::HitTest(const int Mx,const int My)
{
/*	if (SkinInfo!=0)
	{
		if (LastFrame!=0)
			if  ( (Mx>LastBaseX) && (Mx<LastBaseX+(float)LastFrame->SpriteInfo->Width) &&
			      (My>LastBaseY) && (My<LastBaseY+(float)LastFrame->SpriteInfo->Height))
				return true;
	}*/
/*	for (unsigned int i=0;i<PuppetPart_Count;i++)
	{
		if (PuppetSkin[i]->HitTest(Mx,My))
		{
			return true;
		}
	}*/
	return false;
};

bool SkinRendererPuppet::Render(const float DeltaTime,const float BaseXPos,const float BaseYPos,const float Depth,const unsigned long Action,const unsigned long Dir,const bool Selected)
{
	bool Result=false;
	for (unsigned int i=0;i<EPuppetPartType::Count;i++)
	{
		if (PuppetSkin[i]!=0)
		{
			Result=PuppetSkin[i]->Render(DeltaTime,BaseXPos,BaseYPos,Depth,Action,Dir,Selected);
		}
	};
	return Result;
};
	
void SkinRendererPuppet::ChangePart(const EPuppetPartType::Enum EquipPos,const unsigned short PartId)
{
	unsigned long NewId=PartId;
	if (PartId!=0)
	{
		if (Female==1)
			NewId|=FemaleSkinMask;	

	} else
	{
		//make the part naked
		if (Female==1)
			NewId=EquipPos+1|FemaleSkinMask;	
		else
			NewId=EquipPos+1;
	}

	if (PuppetSkin[EquipPos]->GetSkinId()!=NewId)
	{		
		delete PuppetSkin[EquipPos];
		PuppetSkin[EquipPos]=new SkinRendererPuppetPart(SkinPool,NewId);
	}
};
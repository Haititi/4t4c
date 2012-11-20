#include "SpriteDatabase.h"
#include "Debug.h"
#include "GfxEngine.h" 
#include "App.h"
#include "Global.h"

#include "PalDatabase.h"
#include "GameObject.h"
#include "Zlib/zlib.h"

#include "FastStream.h"
#include "hash.h"

struct ECompType
{
	enum Enum
	{
		NoComp,
		Zlib,
		Lzma,
	};
};

struct ETexFormat
{
	enum Enum
	{
		A8R8G8B8,
		R5G6B5,
		A1R5G5B5,  
		P8,
		DXT1A,
		DXT5,
	};
};

// Set the Unique Global
SpriteDatabase SpriteDb;
TMultiDataAccess DataAccess;

SpriteDatabase::SpriteDatabase()
{
	Dummy.TransColor=0;
	Dummy.OffsetX=0;
	Dummy.OffsetY=0;
	Dummy.OffsetX2=0;
	Dummy.OffsetY2=0;

	InitRandHash();
	IndexHash=new HashPool(40009); //primes number size

	HashPool* OffsetHash;
	OffsetHash=new HashPool(40009);

	FastStream FstIdx;
	if (!FstIdx.LoadFromFile(L".\\GameFiles\\Index.si"))
	{
		LOG("SpriteDatabase : Unable to Load Index.si, Cannot continue\r\n");
		App.Close(L"SpriteDatabase : Unable to Load Index.si, Cannot continue",ET4cErrCode::FileMissing);
		return;
	};
	//todo check Checksum

	FstIdx.Read(&SiHdr,sizeof(SiHeader));
	SiInfoArray=new SiInfo[SiHdr.IdxCount];

	//note on remplace toujours par le dernier Sprite aillant le nom X
	//note2  autant faire l'ajout a l'envers et ne pas remplacer 
	//	FastStream FstErr;
	for(unsigned int i=0;i<SiHdr.IdxCount;i++)
	{
		SiInfoArray[i].SpriteName	=FstIdx.ReadLongString();
		SiInfoArray[i].DataOffset	=FstIdx.ReadLong();
		SiInfoArray[i].DataSize		=FstIdx.ReadLong();
		SiInfoArray[i].StoreType	=FstIdx.ReadLong();
		SiInfoArray[i].TextFmt		=FstIdx.ReadLong();
		SiInfoArray[i].Width		=FstIdx.ReadLong();
		SiInfoArray[i].Height		=FstIdx.ReadLong();
		SiInfoArray[i].Surface		=0;
		SiInfoArray[i].UseCount		=0;
		SiInfoArray[i].OldOff		=0;//&Dummy;
		if (!IndexHash->AddHashEntry(RandHash(SiInfoArray[i].SpriteName),&SiInfoArray[i]))
		{
			//	FstErr.Write(SiInfoArray[i].SpriteName,strlen(SiInfoArray[i].SpriteName));
			//	unsigned short LnCr=0x0a0d;
			//	FstErr.WriteWord(LnCr);
			LOG("Replaced entry : "<<SiInfoArray[i].SpriteName<<"\r\n");
			IndexHash->ReplaceHashEntry(RandHash(SiInfoArray[i].SpriteName),&SiInfoArray[i]);
			//PSiInfo Si=(PSiInfo)IndexHash->GetItem(RandHash(SiInfoArray[i].SpriteName));
			//FstErr.WriteLongString(Si->SpriteName);
		}
	}
	//FstErr.SaveToFile("Collisions2.txt");

	FastStream FstOff;

	if (!FstOff.LoadFromFile(L".\\GameFiles\\Offset.dat"))
	{
		LOG("SpriteDatabase : Unable to Load Offset.dat, Cannot continue\r\n");
		App.Close(L"SpriteDatabase : Unable to Load Offset.dat, Cannot continue",ET4cErrCode::FileMissing);
		return;
	};

	

	unsigned long OffCount=FstOff.ReadLong();
	OldOffsetArray=new OldOffsetStruct[OffCount];
	for(unsigned int i=0;i<OffCount;i++)
	{
		OldOffsetArray[i].SpriteName	=FstOff.ReadLongString();
		OldOffsetArray[i].PalName		=FstOff.ReadLongString();
		OldOffsetArray[i].OffsetX		=FstOff.ReadWord();
		OldOffsetArray[i].OffsetY		=FstOff.ReadWord();
		OldOffsetArray[i].OffsetX2		=FstOff.ReadWord();
		OldOffsetArray[i].OffsetY2		=FstOff.ReadWord();
		OldOffsetArray[i].Transparency	=FstOff.ReadWord();
		OldOffsetArray[i].TransColor	=FstOff.ReadWord();
		SiInfo* GInfo=(SiInfo*)IndexHash->GetEntry(RandHash(OldOffsetArray[i].SpriteName));
		if (GInfo)
		{
			GInfo->OldOff=&OldOffsetArray[i];
		}
	}

	delete OffsetHash;
	IndexLoaded = true;
}

SpriteDatabase::~SpriteDatabase()
{
	if (!IndexLoaded)
		return;
	IndexLoaded=false;
	for (unsigned long i=0;i<SiHdr.IdxCount;i++)
	{
		delete [] SiInfoArray[i].SpriteName;
	}
	delete SiInfoArray;
	delete IndexHash;
}

SiInfo* SpriteDatabase::GetIndexEntry(const char* SpriteName)
{
	//don't need to be thread safe
	SiInfo* SpriteInfo=(SiInfo*)IndexHash->GetEntry(RandHash(SpriteName));
	if( SpriteInfo == 0 )
	{
		SpriteInfo=(SiInfo*)IndexHash->GetEntry(RandHash("black tile"));
		//SpriteInfo->OldOff->Transparency=0;
		OutputDebugStringA(SpriteName);
		OutputDebugStringA("\r\n");
		//MessageBoxA(0,SpriteName,"Unable to find : ",MB_OK);
		//return false;
	}
	return SpriteInfo;
};

void SpriteDatabase::LoadPsi(SiInfo* SpriteInfo)
{
	//TODO should be possible to tigthen the lock
	if (SpriteInfo)
	{
		const unsigned long LockIndex=RandHash(SpriteInfo->SpriteName) & 7;
		ScopedLock Al( DataBaseLock[LockIndex] );

		if (SpriteInfo->UseCount>0) //nothing to load
		{
			SpriteInfo->UseCount++;
			return;
		}

		SpriteInfo->UseCount++;

		LPBYTE PalettePtr=0;

		if (SpriteInfo->OldOff!=0)
			PalettePtr=TPalDatabase::GetInstance()->GetPal(SpriteInfo->OldOff->PalName);
		else
			PalettePtr=TPalDatabase::GetInstance()->GetDefaultPal();

		//show disk icon
	Global.DisplayDisk=1;

		switch (SpriteInfo->StoreType) 
		{
		case ECompType::NoComp:  // Raw Format.
				LoadSprite_NoComp(SpriteInfo,PalettePtr); 
				break;
		case ECompType::Zlib:
				LoadSprite_Zlib(SpriteInfo,PalettePtr);
				break;
		case ECompType::Lzma: 
				LoadSprite_Lzma(SpriteInfo,PalettePtr); 
				break;
		}
	}
}

void SpriteDatabase::UnloadPsi(SiInfo* SpriteInfo)
{
	const unsigned long LockIndex=RandHash(SpriteInfo->SpriteName) & 7;
	ScopedLock Al( DataBaseLock[LockIndex] );

	if (SpriteInfo->UseCount>0)
	{
		SpriteInfo->UseCount--;
		if (SpriteInfo->UseCount==0)
		{
			SpriteInfo->Surface->Release();
			SpriteInfo->Surface=0;
		}
	}
};

/* Obsolete
bool SpriteDatabase::LoadSprite(const char* SpriteName,PSiInfo *lplpSpriteInfo,const char *Palette)
{


	bool Result=true;

	PSiInfo SpriteInfo=GetIndexEntry(SpriteName);

	const unsigned long LockIndex=RandHash(SpriteInfo->SpriteName) & 7;
	EnterCriticalSection( &DataBaseLock[LockIndex] );

	if (SpriteInfo->UseCount>0)
	{
		(*lplpSpriteInfo)=SpriteInfo;
		SpriteInfo->UseCount++;
		LeaveCriticalSection( &DataBaseLock[LockIndex] );
		return Result;
	}

	SpriteInfo->UseCount++;

	LPBYTE PalettePtr;
	if (Palette==0)
	{
		//on cherche la palette nous meme 
		PalettePtr=TPalDatabase::GetInstance()->GetPal(SpriteInfo->OldOff->PalName);
	} else
	{
		PalettePtr=TPalDatabase::GetInstance()->GetPal(Palette);
	}

	//if (Palette==0)
		//PalettePtr=TPalDatabase::GetInstance()->GetPal("Bright1");


	// Type of Sprite Format
	switch (SpriteInfo->StoreType) 
	{
	case StoreType_NoComp:  // Raw Format.
		LoadSprite_NoComp(SpriteInfo,PalettePtr); 
		break;
	case StoreType_Zlib:
		LoadSprite_Zlib(SpriteInfo,PalettePtr);
		break;
	case StoreType_Lzma: 
		LoadSprite_Lzma(SpriteInfo,PalettePtr); 
		break;
	}
	(*lplpSpriteInfo)=SpriteInfo;
	LeaveCriticalSection( &DataBaseLock[LockIndex] );
	return Result;
}*/

void SpriteDatabase::LoadSprite_NoComp(SiInfo* SpriteInfo,LPBYTE Pal)
{
	unsigned char *Data = new unsigned char[SpriteInfo->DataSize];
	DataAccess.ReadData(Data,SpriteInfo->DataOffset,SpriteInfo->DataSize);

	if (SpriteInfo->TextFmt==ETexFormat::P8)
	{
		LoadSurfaceP8As16(SpriteInfo,Data,Pal);
	} else
	{
		LoadSurfaceRaw(SpriteInfo,Data);
	}

	
	if(Data)
		delete []Data;
}

void SpriteDatabase::LoadSprite_Zlib(SiInfo* SpriteInfo,LPBYTE Pal)
{
	unsigned char *Data = new unsigned char[SpriteInfo->DataSize];
	DataAccess.ReadData(Data,SpriteInfo->DataOffset,SpriteInfo->DataSize);

	unsigned long TailleUnc = SpriteInfo->Width*SpriteInfo->Height;
	//hack
	if (SpriteInfo->TextFmt==ETexFormat::A8R8G8B8)
		TailleUnc<<=2;
	unsigned char *Unpacked = new unsigned char[TailleUnc];

	if ((TailleUnc==0)||(TailleUnc>4000000))
		MessageBoxA(0,"Invalid size","error",MB_OK);
	int Result=
		uncompress(Unpacked,&TailleUnc,Data,SpriteInfo->DataSize);

	if (Result!=Z_OK)
	{
		switch (Result)
		{
		case Z_MEM_ERROR:
			OutputDebugStringA("Zlib Z_MEM_ERROR \r\n");
			break;
		case Z_BUF_ERROR:
			OutputDebugStringA("Zlib Z_BUF_ERROR \r\n");
			break;
		case Z_DATA_ERROR:
			OutputDebugStringA("Zlib Z_DATA_ERROR \r\n");
			break;
		}
	}

	if (SpriteInfo->TextFmt==ETexFormat::P8)
	{
		LoadSurfaceP8As16(SpriteInfo,Unpacked,Pal);
	} else
	{
		LoadSurfaceRaw(SpriteInfo,Unpacked);
	}

	if(Unpacked)
		delete [] Unpacked;

	if(Data)
		delete []Data;
}
void SpriteDatabase::LoadSprite_Lzma(SiInfo* SpriteInfo,LPBYTE Pal)
{
};

void SpriteDatabase::LoadSurfaceRaw(SiInfo* SpriteInfo,LPBYTE Data)
{
	unsigned long &Width  = SpriteInfo->Width;
	unsigned long &Height = SpriteInfo->Height;
	D3DFORMAT TextFormat;
	const unsigned long DirectLoad=1024;
	unsigned long FormatSize;

	switch (SpriteInfo->TextFmt)
	{
		case ETexFormat::A8R8G8B8:
			{
				TextFormat=D3DFMT_A8R8G8B8;
				FormatSize=4;
				break;
			}
		case ETexFormat::R5G6B5:
			{
				TextFormat=D3DFMT_R5G6B5;
				FormatSize=2;
				break;
			}
		case ETexFormat::A1R5G5B5:
			{
				TextFormat=D3DFMT_A1R5G5B5;
				FormatSize=2;
				break;
			}
		case ETexFormat::P8:
			{
				throw; //should NOT happen !!!!!!
				break;
			}
		case ETexFormat::DXT1A:
			{
				FormatSize=DirectLoad;  //those format only support square texture, we can load at once
				TextFormat=D3DFMT_DXT1;
				break;
			}
		case ETexFormat::DXT5:
			{
				FormatSize=DirectLoad;
				TextFormat=D3DFMT_DXT5;
				break;
			}
		default:
			throw;//should NOT happen !!!!!!
	}

	SpriteInfo->Surface=GfxCore.CreateTexture(Width, Height,TextFormat);

	D3DLOCKED_RECT Lock;
	SpriteInfo->Surface->LockRect(0, &Lock, NULL,D3DLOCK_DISCARD);

	if (FormatSize==DirectLoad)
	{
		memcpy(Lock.pBits,Data,SpriteInfo->DataSize); //dxt are not compressed, thus DataSize is the true size
	} else
	if (SpriteInfo->Width==Lock.Pitch/FormatSize)
	{
		memcpy(Lock.pBits,Data,SpriteInfo->Width*SpriteInfo->Height*FormatSize);
	} else   //load line by line
	{
		unsigned char *SrcPtr=Data; 
		unsigned char *DestPtr=(unsigned char*)Lock.pBits;
		const unsigned long LineSize=SpriteInfo->Width*FormatSize;
		for(unsigned int j=0;j<SpriteInfo->Height;j++)
		{
			memcpy(DestPtr,SrcPtr,LineSize);
			SrcPtr+=LineSize;
			DestPtr+=Lock.Pitch;
		}
	}

	SpriteInfo->Surface->UnlockRect(0);
};

void SpriteDatabase::LoadSurfaceP8As32(SiInfo* SpriteInfo,LPBYTE Data,LPBYTE Pal)
{
	unsigned long &Width  = SpriteInfo->Width;
	unsigned long &Height = SpriteInfo->Height;

	SpriteInfo->Surface=GfxCore.CreateTexture(Width, Height, D3DFMT_A8R8G8B8);

	unsigned short wPal, Trans;

	//LPBYTE lpPal = TPalDatabase::GetInstance()->GetPal(lpszID, 1);
	bool Transparent=false;
	Trans=0;
	if (SpriteInfo->OldOff!=0)
	{
		Trans = SpriteInfo->OldOff->TransColor*3;
		Transparent=SpriteInfo->OldOff->Transparency>0;
	}

	D3DLOCKED_RECT Lock;
	SpriteInfo->Surface->LockRect(0, &Lock, NULL,D3DLOCK_DISCARD);

	unsigned long* Dest = (unsigned long*)Lock.pBits;
	unsigned char* Src = Data;   

	//unsigned long Color;
	if (Transparent)
	{
		for (unsigned long j = 0; j < Height; j++) 
		{
			for (unsigned long i = 0; i < Width; i++) 
			{
				wPal	= (*(unsigned char*)(Src+i))*3;
				if (wPal!=Trans)
					Dest[i]=0xFF000000 | (Pal[wPal]<<16)|(Pal[wPal+1]<<8)|(Pal[wPal+2]);
				else
					Dest[i]=(Pal[wPal]<<16)|(Pal[wPal+1]<<8)|(Pal[wPal+2]);
			}
			Dest += Lock.Pitch>>2;
			Src += Width;
		}
	} else
	{
		for (unsigned long j = 0; j < Height; j++) 
		{
			for (unsigned long i = 0; i < Width; i++) 
			{
				wPal	= (*(unsigned char *)(Src+i))*3;
				Dest[i]=0xFF000000 | (Pal[wPal]<<16)|(Pal[wPal+1]<<8)|(Pal[wPal+2]);
			}
			Dest += Lock.Pitch>>2;
			Src += Width;
		}
	}
	SpriteInfo->Surface->UnlockRect(0);
}

void SpriteDatabase::LoadSurfaceP8As16(SiInfo* SpriteInfo,LPBYTE Data,LPBYTE Pal)
{
	unsigned long &Width  = SpriteInfo->Width;
	unsigned long &Height = SpriteInfo->Height;

	SpriteInfo->Surface=GfxCore.CreateTexture(Width, Height, D3DFMT_A1R5G5B5);

	unsigned short wPal, Trans;

	//LPBYTE lpPal = TPalDatabase::GetInstance()->GetPal(lpszID, 1);
	bool Transparent=false;
	Trans=0;
	if (SpriteInfo->OldOff!=0)
	{
		Trans = SpriteInfo->OldOff->TransColor*3;
		Transparent=SpriteInfo->OldOff->Transparency>0;
	}
	
	D3DLOCKED_RECT Lock;
	SpriteInfo->Surface->LockRect(0, &Lock, NULL,D3DLOCK_DISCARD);

	unsigned short* Dest = (unsigned short*)Lock.pBits;
	unsigned char* Src = Data;   

	//unsigned long Color;
	if (Transparent)
	{
		for (unsigned long j = 0; j < Height; j++) 
		{
			for (unsigned long i = 0; i < Width; i++) 
			{
				wPal	= (*(Src+i))*3;
				if (wPal!=Trans)
					Dest[i]=0x8000 | ((Pal[wPal]>>3)<<10)|((Pal[wPal+1]>>3)<<5)|(Pal[wPal+2]>>3);
				else
					Dest[i]=0;
			}
			Dest += Lock.Pitch>>1;
			Src += Width;
		}
	} else
	{
		for (unsigned long j = 0; j < Height; j++) 
		{
			for (unsigned long i = 0; i < Width; i++) 
			{
				wPal	= (*(Src+i))*3;
				Dest[i]=0x8000 | ((Pal[wPal]>>3)<<10)|((Pal[wPal+1]>>3)<<5)|(Pal[wPal+2]>>3);
			}
			Dest += Lock.Pitch>>1;
			Src += Width;
		}
	}
	SpriteInfo->Surface->UnlockRect(0);
}



TMultiDataAccess::TMultiDataAccess(void)
{
	HelperIndex=0;
	for(int i=0;i<MultiDataCount;i++)
	{
		FileAccess[i].File=CreateFileA(".\\GameFiles\\Data.data",GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS,0);
	}
};

TMultiDataAccess::~TMultiDataAccess(void)
{
	for(int i=0;i<MultiDataCount;i++)
	{
		CloseHandle(FileAccess[i].File);
	}
};

void TMultiDataAccess::ReadData(void* Buffer,const unsigned long Position,const unsigned long Count)
{
	unsigned long NewIndex=HelperIndex;
	while (FileAccess[NewIndex].Lock.TryLock()==false)
	{
		NewIndex=(NewIndex+1)%MultiDataCount;
	}
	HelperIndex=(NewIndex+1)%MultiDataCount;

	//todo Error check
	SetFilePointer(FileAccess[NewIndex].File,Position,0,FILE_BEGIN);
	unsigned long ByteRead;
	ReadFile(FileAccess[NewIndex].File,Buffer,Count,&ByteRead,0);

	FileAccess[NewIndex].Lock.Unlock();
};

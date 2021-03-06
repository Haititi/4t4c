#include "SoundEngine.h"
#include "debug.h"
#include "App.h"
#include "Config.h"
#include "FastStream.h"
#include "hash.h"

TSoundEngine SoundEngine;
const unsigned long HBufferSize=16384; //engine streaming buffer

const wchar_t* GetDxSoundErrorDesc(const HRESULT Hr);

TSoundInfo::TSoundInfo(char *SndName):SoundBuffer(0),DuplicateCount(0),DuplicateIndex(0),SpinLock(false)
{
	
	SndName[strlen(SndName)-4]=0;//remove the extension
	SoundName=SndName;

	char FilePath[512];
	sprintf_s(FilePath,512,".\\GameFiles\\sounds\\%s.wav",SndName);
	FILE* Hdl;
	if(	fopen_s(&Hdl,FilePath,"r")!=0 )
	{
		//OutputDebugStr(" Unable to load wave : ");
		//OutputDebugStr(SndName);
		//OutputDebugStr("\r\n");
		return;
	}

	//fill the wav header
	fread_s(&Header,sizeof(TWavHeader),1,sizeof(TWavHeader),Hdl);

	fclose(Hdl);
};

TSoundInfo::~TSoundInfo(void)
{
	//if we have some SoundObject left we should signal to those that the ref is invalid
	std::list<SoundObject*>::iterator ObjIt;
	for(ObjIt=SoundObjList.begin();ObjIt!=SoundObjList.end();ObjIt++)
	{
		(*ObjIt)->SetRefToZero();
	};
};

//HACK / TEST  simple  spinlock to ensure if sync is needed or not 
void TSoundInfo::Lock(void)
{
	while(SpinLock==true)
	{
		//OutputDebugStringA("<>");
	}; 
	SpinLock=true; 
};

void TSoundInfo::Unlock(void)
{
	SpinLock=false;
};

void TSoundInfo::Initialize(void)
{
	//create and load the main buffer
	DSBUFFERDESC BufferDesc;
	BufferDesc.dwSize=sizeof(DSBUFFERDESC);
	BufferDesc.dwReserved=0;
	BufferDesc.dwFlags=DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;// | DSBCAPS_LOCDEFER;
	BufferDesc.dwBufferBytes=Header.DataSize;//(Header.BitPerSample >> 3)*Header.Channels*Header.SamplesPerSec;
	BufferDesc.guid3DAlgorithm=DS3DALG_DEFAULT;
	WAVEFORMATEX Format;
	Format.cbSize=sizeof(WAVEFORMATEX);
	Format.nAvgBytesPerSec=Header.AvgBytesPerSec;
	Format.nBlockAlign=Header.BlockAlign;
	Format.nChannels=Header.Channels;
	Format.nSamplesPerSec=Header.SamplesPerSec;
	Format.wBitsPerSample=Header.BitPerSample;
	Format.wFormatTag=WAVE_FORMAT_PCM;
	BufferDesc.lpwfxFormat=&Format;

	if (SoundEngine.DirectSound->CreateSoundBuffer(&BufferDesc,&SoundBuffer,0)==DS_OK)
	{
		//lock and load wave data
		std::wstring FilePath(512,L'\0');
		swprintf(&FilePath[0],512,L".\\GameFiles\\sounds\\%s.wav",SoundName);
		HANDLE FileHdl;
		FileHdl=CreateFileW(FilePath.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);

		void* PtrVoid1,*PtrVoid2;
		unsigned long PtrSize1,PtrSize2,NbRead;
		SoundBuffer->Lock(0,Header.DataSize,&PtrVoid1,&PtrSize1,&PtrVoid2,&PtrSize2,0);
		ReadFile(FileHdl,PtrVoid1,PtrSize1,&NbRead,0);
		SoundBuffer->Unlock(PtrVoid1,PtrSize1,PtrVoid2,PtrSize2);

		CloseHandle(FileHdl);
	}else
	{
		//OutputDebugStr("Error while creating sound buffer\r\n");
	}
};

void TSoundInfo::Uninitialize(void)
{
	if (SoundBuffer!=0)
	{
		//destroy the main buffer
		SoundBuffer->Release();
		SoundBuffer=0;
	}
};

void TSoundInfo::DestroyAll(void)
{
	if (SoundBuffer!=0)
	{
		//destroy duplicates
		while (Duplicates.size()!=0)
		{
			DestroyDuplicate();
		}
	}
};

void TSoundInfo::RemoveMe(SoundObject* Object)
{
	std::list<SoundObject*>::iterator ObjIt;
	for(ObjIt=SoundObjList.begin();ObjIt!=SoundObjList.end();ObjIt++)
	{
		if ((*ObjIt)==Object)
		{
			SoundObjList.erase(ObjIt);
			break;
		}
	};
};

SoundObject* TSoundInfo::CreateSoundObject(void)
{
	SoundObject* Result=new SoundObject(this);
	SoundObjList.push_back(Result);
	return Result;
};

void TSoundInfo::CreateDuplicate(void)
{
	Lock();
	if (DuplicateCount<MaxDuplicate)
	{
		if (DuplicateCount==0)
			Initialize();
		DuplicateCount++;
		
		LPDIRECTSOUNDBUFFER Dup;
		SoundEngine.DirectSound->DuplicateSoundBuffer(SoundBuffer,&Dup);
		Duplicates.push_back(Dup);
	}
	Unlock();
};

void TSoundInfo::DestroyDuplicate(void)
{
	Lock();
	//TODO FIX  we should not have to check if the soundengine is down
	//at that time the object should already be empty,  logic error somewhere
	if (DuplicateCount>0) 
	{

		LPDIRECTSOUNDBUFFER Buf=Duplicates.back();
		Buf->Release();

		Duplicates.pop_back();
		DuplicateCount--;
		if (DuplicateCount==0)
		{
			Uninitialize();
		}
	}
	Unlock();
};

LPDIRECTSOUNDBUFFER TSoundInfo::Play(float Vol,float Pan)
{
	Lock();
	DuplicateIndex=(DuplicateIndex+1)%DuplicateCount;
	LPDIRECTSOUNDBUFFER Buffer=Duplicates[DuplicateIndex];
	Buffer->Stop();
	Buffer->SetVolume((long)(-10000.0f*(1.0f-(Vol*SoundEngine.SoundVolume))));
	Buffer->SetPan((long)(Pan*10000.0f));
	Buffer->Play(0,0,0);
	Unlock();
	return Buffer;
};

SoundObject::SoundObject(PSoundInfo Ref):Reference(Ref),Buffer(0),Loaded(false),Volume(1.0f),Panoramic(0.0f)
{
};

SoundObject::~SoundObject(void)
{
	if (Reference ) 
	{
		Reference->RemoveMe(this);
	}
	Unload();
};	

void SoundObject::Load(void)
{
	if (!Loaded && Reference)
	{
		Loaded=true;
		Reference->CreateDuplicate();	
	}
};

void SoundObject::Unload(void)
{
	if (Loaded)
	{
		if (Reference)
			Reference->DestroyDuplicate();
		Loaded=false;
		Buffer=0;
	}
};

void SoundObject::Play(void)
{
	if (Loaded)
		Buffer=Reference->Play(Volume,Panoramic);
};

TSoundEngine::TSoundEngine(void):SoundPool(199)
{
	hEvent = 0;
	DirectSound = 0;
	OggTerminated=false;
	MusicPlaying=false;
	Initialized = false;
	SoundVolume=1.0f;
	OggVolume=1.0f;

	//referencing sounds
	TFastStream FstIdx;
	if (!FstIdx.LoadFromFile(L".\\GameFiles\\sounds.txt"))
	{
		return;
	}

	while (!FstIdx.IsEnd())
	{
		PSoundInfo Info=new TSoundInfo(FstIdx.ReadTextString());
		SoundPool.AddHashEntry(RandHash(Info->SoundName),Info);
	}
}

TSoundEngine::~TSoundEngine(void) 
{
	Initialized = false;
	//todo : release sound buffer as well
	OggTerminated=true;
	if (MusicPlaying) StopOggStream();
	WaitForSingleObject(OggThreadHdl,250);
	CloseHandle(OggThreadHdl);
	CloseHandle(OggStopStream);
	if (OggBuffer)
	{
		OggNotify->Release();
		OggBuffer->Stop();
		OggBuffer->Release();
	}
	CloseHandle(OggNotifyHdl[0]);
	CloseHandle(OggNotifyHdl[1]);

	//destroying SoundInfoBuf and dup
	THashIterator SndPoolIt;
	SoundPool.InitIterator(SndPoolIt);
	PSoundInfo Info=(PSoundInfo)SndPoolIt.GetNextEntry();
	while(Info!=0)
	{
		Info->DestroyAll();
		delete Info;
		Info=(PSoundInfo)SndPoolIt.GetNextEntry();
	}
	
	//destroy primary
	if (Primary)
	{
		Primary->Stop();
		Primary->Release();
	}

	CloseHandle(hEvent);
	DirectSound->Release();
}


long WINAPI OggThreadRoutine(TSoundEngine* Arg)
{
	return (Arg->OggThread());
};

bool TSoundEngine::Create( HWND hWnd, std::wstring &ErrMsg ) 
{
	DSBUFFERDESC BufferDesc;
	WAVEFORMATEX WaveFmt;

	
	HRESULT hr = DirectSoundCreate8(0, &DirectSound, 0);

	if (hr!=DS_OK)
	{
		ErrMsg = L"DirectSoundCreate was not sucessful: ";
		ErrMsg+=GetDxSoundErrorDesc(hr);
		LOG(ErrMsg);
		return false;
	}

	DirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);

	WaveFmt.wFormatTag = WAVE_FORMAT_PCM;
	WaveFmt.nChannels = 2;
	WaveFmt.nSamplesPerSec = 44100;
	WaveFmt.wBitsPerSample = 16;
	WaveFmt.nBlockAlign = (WaveFmt.wBitsPerSample >> 3)*WaveFmt.nChannels;
	WaveFmt.nAvgBytesPerSec = WaveFmt.nSamplesPerSec * WaveFmt.nBlockAlign;
	
	WaveFmt.cbSize = 0;

	memset(&BufferDesc, 0, sizeof(DSBUFFERDESC));
	BufferDesc.dwSize        = sizeof(DSBUFFERDESC);
	BufferDesc.dwFlags       = DSBCAPS_PRIMARYBUFFER;
	BufferDesc.dwBufferBytes = 0;
	BufferDesc.lpwfxFormat   = NULL;
	BufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

	hr = DirectSound->CreateSoundBuffer(&BufferDesc, &Primary, NULL);
	if( hr != DS_OK )
	{
		ErrMsg=L"SoundEngine : Could not create primary sound buffer : ";
		ErrMsg+=GetDxSoundErrorDesc(hr);
		LOG(ErrMsg);
		return false;
	}

	Primary->SetFormat(&WaveFmt);

	hr=Primary->Play(0,0,DSBPLAY_LOOPING);
	if (hr!=DS_OK)
	{
		ErrMsg=L"SoundEngine (Fatal Error) : Primary->Play : ";
		ErrMsg+=GetDxSoundErrorDesc(hr); 
		LOG(ErrMsg);
		return false;
	}

	memset(&BufferDesc, 0, sizeof(DSBUFFERDESC));
	BufferDesc.dwSize        = sizeof(DSBUFFERDESC);
	BufferDesc.dwFlags       = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY;;
	BufferDesc.dwBufferBytes = (HBufferSize/2)*WaveFmt.nBlockAlign;;
	BufferDesc.lpwfxFormat   = &WaveFmt;
	BufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

	DirectSound->CreateSoundBuffer(&BufferDesc, &OggBuffer, NULL);


	hr=OggBuffer->QueryInterface(IID_IDirectSoundNotify,(void**)&OggNotify);
	if (hr!=DS_OK)
	{
		ErrMsg=L"SoundEngine (Fatal Error) : OggBuffer->QueryInterface :";
		ErrMsg+=GetDxSoundErrorDesc(hr);
		LOG(ErrMsg);
		return false;
	}

	OggStopStream=CreateEvent(NULL,true,true,NULL);
	OggNotifyHdl[0]=CreateEvent(NULL,false,false,NULL);
	OggNotifyHdl[1]=CreateEvent(NULL,false,false,NULL);
	
	DSBPOSITIONNOTIFY PosNotify[2];

	PosNotify[0].dwOffset=0;                     //1st notification at buffer start
	PosNotify[0].hEventNotify=OggNotifyHdl[0];
	PosNotify[1].dwOffset=HBufferSize;				//2nd notification at buffer half  note: notification is in byte
	PosNotify[1].hEventNotify=OggNotifyHdl[1];

	//todo : in case of we're unable to play music we should disactivate streaming 
	hr=OggNotify->SetNotificationPositions(2,PosNotify);
	if (hr!=DS_OK)
	{
		ErrMsg=L"SoundEngine (Error): SetNotificationPositions : ";
		ErrMsg+=GetDxSoundErrorDesc(hr);
		LOG(ErrMsg);
		return false;
	}


	//launch the playing of the ogg stream buff
	hr=OggBuffer->Play(0,0,DSBPLAY_LOOPING);
	if (hr!=DS_OK)
	{
		ErrMsg=L"SfxCore (Error):OggBuffer->Play : ";
		ErrMsg+=GetDxSoundErrorDesc(hr);
		LOG(ErrMsg);
		return false;
	}

	//spawn the thread, it won't consume cpu unless music is playing
	unsigned long OggThreadId;
	OggThreadHdl=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)OggThreadRoutine,this,0,&OggThreadId);

	Initialized=true;

	return true;
}

SoundObject* TSoundEngine::GetSoundObject(const char* SoundName)
{
	PSoundInfo Info=(PSoundInfo)SoundPool.GetEntry(RandHash(SoundName));
	if (Info!=0)
	{
		return Info->CreateSoundObject();
	}
	return 0;
};

long TSoundEngine::OggThread()
{
	unsigned long Result;
	unsigned long ByteRead;
	int BitStream;
	unsigned long Size1,Size2;
	void  *Ptr1,*Ptr2;
	char* CopyOffset;
	unsigned long CopyLeft;

	LOG("Info : Ogg Thread Start\r\n");

	while(!OggTerminated)
	{
		Result=WaitForMultipleObjects(2,OggNotifyHdl,false,INFINITE);
		//SetEvent(OggStopStream);
		if (MusicPlaying)
		{
			ScopedLock Al(OggLock);
			//if Result==0  we need to lock 2nd part of buffer
			OggBuffer->Lock(HBufferSize*(Result ^ 1),HBufferSize*2,&Ptr1,&Size1,&Ptr2,&Size2,0);
			CopyLeft=HBufferSize;
			CopyOffset=(char*)Ptr1;

			//decode stream part into the locked buffer
			while (CopyLeft>0)
			{
				ByteRead=ov_read(&OggStruct,CopyOffset,CopyLeft,0,2,1,&BitStream);
			/*	if (OggStruct.offset>50000)
				{
					ov_pcm_seek(&OggStruct,0);
				}*/
				switch (ByteRead)
				{
					case 0:  //eof -> just start again the stream
					{
						int SeekResult=ov_raw_seek(&OggStruct,0);//ov_pcm_seek(&OggStruct,64);
						if (SeekResult==OV_EINVAL)
						{ //invalid parameter something went wrong
							CopyLeft=0;
						}	
						break;
					}
					case OV_HOLE:  //interruption in the stream
					case OV_EBADLINK: //invalid stream section
						StopOggStream();
						CopyLeft=0; //get out of this
				}
				CopyLeft-=ByteRead; //we should end up exactly at 0 at stream end
				CopyOffset+=ByteRead;
			}

			//unlock buffer
			OggBuffer->Unlock(Ptr1,Size1,Ptr2,Size2);

			//unlock OggLock
		}
		WaitForSingleObject(OggStopStream,INFINITE);

	}

	LOG("Info : Ogg Thread End\r\n");

	return 0;
}

void TSoundEngine::PlayOggStream(const char *FileName)
{
	if (FileName==0)
		return;

	char FileName2[512];
	sprintf_s(FileName2,512,".\\GameFiles\\music\\%s",FileName);

	SetEvent(OggStopStream);
	if (MusicPlaying)  //we have a potential problem here, need to shutdown playing 
	{
		StopOggStream();
	}

	//we're "forced" to use fopen, fread etc 
	{
		ScopedLock Al(OggLock);
		fopen_s(&OggFile,FileName2,"rb");
		ov_open(OggFile,&OggStruct,NULL,0);
	}

	
	MusicPlaying=true;
	HRESULT hr=
	OggBuffer->Play(0,0,DSBPLAY_LOOPING);

	if (hr!=DS_OK)
	{
		LOG("SoundEngine (Error): OggBuffer->Play " << GetDxSoundErrorDesc(hr) << "\r\n");
	}
	
};

void TSoundEngine::StopOggStream(void)
{
	if (MusicPlaying)
	{
		ResetEvent(OggStopStream);
		OggBuffer->Stop();
		MusicPlaying=false;
		{
			ScopedLock Al(OggLock);
			ov_clear(&OggStruct);// note : ov_clear call fclose 
		}
	}
};

void TSoundEngine::SetMusicVolume(float NewVolume)
{
	if ((NewVolume>=0.0f) && (NewVolume<=1.0f))
	{
		OggVolume=NewVolume;
		OggBuffer->SetVolume( (long) (-10000.0f * (1.0f - OggVolume)));
	}
};

void TSoundEngine::SetSoundVolume(float NewVolume)
{
	if ((NewVolume>=0.0f) && (NewVolume<=1.0f))
	{
		SoundVolume=NewVolume;
	}
};

const wchar_t* GetDxSoundErrorDesc(const HRESULT Hr)
{
	switch (Hr)
	{
		// The call succeeded, but we had to substitute the 3D algorithm
		case DS_NO_VIRTUALIZATION:
			return L"No Virtualization";
		// The call failed because resources (such as a priority level)
		// were already being used by another caller
		case DSERR_ALLOCATED:    
            return L"Already Allocated";
		// The control (vol, pan, etc.) requested by the caller is not available
		case DSERR_CONTROLUNAVAIL:          
			return L"Unavailable Control";
		// An invalid parameter was passed to the returning function
		case DSERR_INVALIDPARAM:
			return L"Invalid Parameter";
		// This call is not valid for the current state of this object
		case DSERR_INVALIDCALL:               
			return L"Invalid Call";
		// An undetermined error occurred inside the DirectSound subsystem
		case DSERR_GENERIC:   
			return L"Unknown Error !!!";
		// The caller does not have the priority level required for the function to
		// succeed
		case DSERR_PRIOLEVELNEEDED:   
			return L"Bad Priority Level";
		// Not enough free memory is available to complete the operation
		case DSERR_OUTOFMEMORY: 
			return L"Out Of Memory";
		// The specified WAVE format is not supported
		case DSERR_BADFORMAT:   
			return L"Bad Wave Format";
		// The function called is not supported at this time
		case DSERR_UNSUPPORTED:
			return L"Unsupported Function";
		// No sound driver is available for use
		case DSERR_NODRIVER:   
			return L"No Sound Drive";
		// This object is already initialized
		case DSERR_ALREADYINITIALIZED:   
			return L"Already Initialized";
		// This object does not support aggregation
		case DSERR_NOAGGREGATION:   
			return L"No Aggregation Possible";
		// The buffer memory has been lost, and must be restored
		case DSERR_BUFFERLOST:              
			return L"Buffer Lost";
		// Another app has a higher priority level, preventing this call from
		// succeeding
		case DSERR_OTHERAPPHASPRIO:     
			return L"Another App Has Priority";
		// This object has not been initialized
		case DSERR_UNINITIALIZED:  
			return L"Unitialized Object";
		// The requested COM interface is not available
		case DSERR_NOINTERFACE:   
			return L"Interface Unavailable";
		// Access is denied
		case DSERR_ACCESSDENIED:       
			return L"Access Denied";
		// Tried to create a DSBCAPS_CTRLFX buffer shorter than DSBSIZE_FX_MIN milliseconds
		case DSERR_BUFFERTOOSMALL:  
			return L"Buffer Too Small";
		// Attempt to use DirectSound 8 functionality on an older DirectSound object
		case DSERR_DS8_REQUIRED:      
			return L"Ds8 Is Required For This Operation";
		// A circular loop of send effects was detected
		case DSERR_SENDLOOP:   
			return L"Circular Fx Loop";
		// The GUID specified in an audiopath file does not match a valid MIXIN buffer
		case DSERR_BADSENDBUFFERGUID:     
			return L"Bad GUID";
		// The object requested was not found (numerically equal to DMUS_E_NOT_FOUND)
		case DSERR_OBJECTNOTFOUND:           
			return L"Object Not Found";
		// The effects requested could not be found on the system, or they were found
		// but in the wrong order, or in the wrong hardware/software locations.
		case DSERR_FXUNAVAILABLE: 
            return L"Fx Unavailable";
	}
	return L" ";
};

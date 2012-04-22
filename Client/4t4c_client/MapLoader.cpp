#include "MapLoader.h"
#include "FastStream.h"
#include "Global.h"
#include "debug.h"
#include "Zlib.h"
#include "./interface/Chatterui.h"
#include "app.h"

TMapLoader MapLoader;

TMapLoader::TMapLoader(void)
{
	LoadingThreadHdl=0;
	ActualCacheIndex=0xFFFFFFFF;
	LoadingFinished=true;
	StopLoad=false;
	NeedMapLoad=false;
	PosW=32768;//dummy value
	PosX=-1;
	PosY=-1;
	MapCount=0;
	MapName=0;

	//initialize the index position
	int Index=0;
	for(int j=0;j<3072/128;j++)
	{
		for (int i=0;i<3072/128;i++)
		{
			IndexArray[Index].X=i*128+64;
			IndexArray[Index].Y=j*128+64;
			IndexArray[Index].IdCount=0;
			IndexArray[Index].IdList=0;
			Index++;
			IndexArray[Index].X=i*128+128;
			IndexArray[Index].Y=j*128+128;
			Index++;
			IndexArray[Index].IdCount=0;
			IndexArray[Index].IdList=0;
		}
	};
	memset(&Map,0,3072*3072*2);

	//reading the mapnames
	TFastStream Fst;
	if (Fst.LoadFromFile(L".\\GameFiles\\Maps\\MapName.dat"))
	{
		MapCount=Fst.ReadLong();
		MapName=new TMapName[MapCount];
		for (unsigned long i=0;i<MapCount;i++)
		{
			MapName[i].MapName=Fst.ReadLongString();
		}
	} else
	{
		LOG("MapLoader: Unable to load MapName.dat\r\n");
	}
};
	
TMapLoader::~TMapLoader(void)
{
	//todo free up memory !!!!
	for (int i=0;i<1152;i++)
	{
		if (!IndexArray[i].IdList)
			delete [] IndexArray[i].IdList;
	}

	if (MapName)
	{
		for (unsigned long i=0;i<MapCount;i++)
		{
			if (MapName[i].MapName)
				delete [] MapName[i].MapName;
		};
		delete []MapName;
	}
};


void TMapLoader::ReadIndexFile(std::wstring FileName)
{
	struct TDatIndex
	{
		unsigned long Offset;
		unsigned long Size;
	};
	TDatIndex Index[1152];

	TFastStream Fst;
	if (!Fst.LoadFromFile(FileName))
	{
		LOG("MapLoader : Critical error when trying to read the cache index\r\n");
		return;
	};

	for (int i=0;i<1152;i++)
	{
		Index[i].Offset=Fst.ReadLong();
		Index[i].Size=Fst.ReadLong();
	}

	for (int i=0;i<1152;i++)
	{
		if (IndexArray[i].IdList!=0)
		{
			delete []IndexArray[i].IdList; 
		}
		IndexArray[i].IdCount=Index[i].Size;
		IndexArray[i].IdList=new unsigned short[Index[i].Size];
		Fst.Seek(Index[i].Offset);
		for (unsigned long k=0;k<Index[i].Size;k++)
		{
			IndexArray[i].IdList[k]=Fst.ReadWord();
		}
	}
};

unsigned long TMapLoader::FindClosestCacheIndex(const int PosX,const int PosY)
{
	const unsigned long IdxSeekCount=5;
	unsigned long IndexAround[IdxSeekCount];
	unsigned long ClosestIndex;
	unsigned long ClosestLength=2000000000;
	
	//seek the indexes around to find the closest one
	IndexAround[0]=(PosX/128)*2+(PosY/128)*48;
	IndexAround[1]=IndexAround[0]-1;
	IndexAround[2]=IndexAround[0]+1;
	IndexAround[3]=IndexAround[0]-47;
	IndexAround[4]=IndexAround[0]-49;

	for (int i=0;i<IdxSeekCount;i++)
	{
		if (IndexAround[i]<0)
			IndexAround[i]=0;
		if (IndexAround[i]>=1152)
			IndexAround[i]=1151;
		//only compute the squared distance, we don't need the true one
		long IdxX=IndexArray[IndexAround[i]].X-PosX;
		long IdxY=IndexArray[IndexAround[i]].Y-PosY;
		unsigned long Length=IdxX*IdxX+IdxY*IdxY;
		if (Length<ClosestLength)
		{
			ClosestIndex=IndexAround[i];
			ClosestLength=Length;
		}
	}
	return ClosestIndex;
};

void TMapLoader::Initialize(void)
{
	//does stuff that we can't init in the constructor
	LoadMapIdInfo();
};

void TMapLoader::LoadMapIdInfo(void)
{
	TFastStream Fst;
	int i;

	if (!Fst.LoadFromFile(L".\\GameFiles\\MapIdInfo.Dat"))
	{
		LOG("MapLoader : critical error when trying to read MapIdInfo.dat\r\n");
		return;
	};

	unsigned long Version,Count;
	Version=Fst.ReadLong();
	Count=Fst.ReadLong();
	
	for (i=0;i<MaxIdNumber;i++)
	{
		MapIdInfo[i].Defined=Fst.ReadLong();

		if (MapIdInfo[i].Defined)
		{
			MapIdInfo[i].Loaded=false;
			MapIdInfo[i].IdType=Fst.ReadLong();
			MapIdInfo[i].Reversed=Fst.ReadLong();
			MapIdInfo[i].Modx=Fst.ReadLong();
			MapIdInfo[i].Mody=Fst.ReadLong();
			MapIdInfo[i].MapColor=Fst.ReadLong();
			MapIdInfo[i].MiniTextX=Fst.ReadFloat();
			MapIdInfo[i].MiniTextY=Fst.ReadFloat();
			MapIdInfo[i].MiniTextX2=Fst.ReadFloat();
			MapIdInfo[i].MiniTextY2=Fst.ReadFloat();
			MapIdInfo[i].LightInfo=Fst.ReadLong();
			MapIdInfo[i].ColorFx=Fst.ReadLong();
			MapIdInfo[i].GraphCount=Fst.ReadLong();
			
			MapIdInfo[i].GraphInfo=new TMapIdInfoGr[MapIdInfo[i].GraphCount];
			if (MapIdInfo[i].GraphCount)
			{
				for(unsigned long j=0;j<MapIdInfo[i].GraphCount;j++)
				{
					MapIdInfo[i].GraphInfo[j].GraphName=Fst.ReadWordString();
					MapIdInfo[i].GraphInfo[j].VertexColor=Fst.ReadLong();
					MapIdInfo[i].GraphInfo[j].Offx=Fst.ReadWord();
					MapIdInfo[i].GraphInfo[j].Offy=Fst.ReadWord();
					MapIdInfo[i].GraphInfo[j].Offz=Fst.ReadWord();
					
					//let's Connect the info with the appropriate PVsi without loading

					MapIdInfo[i].GraphInfo[j].Graph=SpriteDb.GetIndexEntry(MapIdInfo[i].GraphInfo[j].GraphName);
					if (MapIdInfo[i].GraphInfo[j].Graph==NULL)
					{	
						LOG("MapIdInfo (Error): Unable to Find Graph for " << MapIdInfo[i].GraphInfo[j].GraphName << "\r\n"); 
					}
				}
			}
			//compute internal variable for ground
			if (MapIdInfo[i].IdType==IdType_Floor)
			{
				MapIdInfo[i].UnitU=1.0f/MapIdInfo[i].Modx;
				MapIdInfo[i].UnitV=1.0f/MapIdInfo[i].Mody;
			}
		}
	}
	LOG("MapidInfo Loaded.\r\n");

};
	
void TMapLoader::LoadMap(void)
{
	unsigned long dwLoadThreadId;
	LoadingFinished=false;
    StopLoad=false;
Global.DisplayMap=1;
	LoadingThreadHdl=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LoadingThreadFunc,this,0,&dwLoadThreadId);
};


long TMapLoader::LoadingThread(void)
{
	HANDLE MapFile=0;
	unsigned long FileSize=0;

	if(NeedMapLoad)
	{
		//LOGIT("Map Loading Thread : Start.\r\n");
		LOG("MapLoader : Map Loading Request");
		ChatterUI::GetInstance()->AddText(L"",L"Loading Map..",RGB32(0,100,255),true);

		//1st we load compressed map data

		std::wstring Temp(512,L'\0');
		swprintf(&Temp[0],512,L".\\GameFiles\\Maps\\%s.zlb",MapName[PosW].MapName);

		MapFile=CreateFileW(Temp.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
		if (MapFile==INVALID_HANDLE_VALUE)
		{
			LOG("MapLoader : Unable to load map "<<Temp<<"\r\n");
			return -1;
		}
		FileSize=GetFileSize(MapFile,0);
		unsigned char* TempBuf=new unsigned char[FileSize];
		unsigned long ByteRead;
		ReadFile(MapFile,TempBuf,FileSize,&ByteRead,0);
		CloseHandle(MapFile);
		unsigned long DstSize=3072*3072*2;
		uncompress((Bytef *)&Map[0],&DstSize,TempBuf,FileSize);
		delete []TempBuf;
		
		//2nd we load the cache index
		swprintf(&Temp[0],512,L".\\GameFiles\\Maps\\%s.mdat",MapName[PosW].MapName);
		ReadIndexFile(Temp);
	}

	
	
	ChatterUI::GetInstance()->AddText(L"",L"Loading Zone..",RGB32(0,100,255),true);
	//3rd we check id which need to be loaded
	for (unsigned long i=0;i<IndexArray[ActualCacheIndex].IdCount;i++)
	{
		unsigned short IdToLoad=IndexArray[ActualCacheIndex].IdList[i];
		//if (StopLoad)			break;
		//else
		{
			if (MapIdInfo[IdToLoad].Defined) //undefined id obviously don't need loading 
			if (!MapIdInfo[IdToLoad].Loaded) //id already loaded 
			{
				for (unsigned long j=0;j<MapIdInfo[IdToLoad].GraphCount;j++) //loop trough the graph composing the id
				{
					if (!MapIdInfo[IdToLoad].GraphInfo[j].Graph) 
						continue;  //is this graph valid ? 
					SpriteDb.LoadPsi(MapIdInfo[IdToLoad].GraphInfo[j].Graph);
				}
				MapIdInfo[IdToLoad].Loaded=true;
			}
		}

	}
	ChatterUI::GetInstance()->AddText(L"",L"Loading Zone End..",RGB32(0,100,255),true);

	//LOGIT("Map Loading Thread : Standard End.\r\n");
	LOG("MapLoader : Map Loading Finished.");
	LoadingFinished=true;
	StopLoad=false;
	return 0;
};

void TMapLoader::WaitForLoadingThread(void)
{
	if (!IsLoadingFinished())
	{
		StopLoad=true;
		WaitForSingleObject(LoadingThreadHdl,10000); //waiting for the thread to terminate
	}
};

void TMapLoader::ChangePosition(int NewX,int NewY,int NewW)
{
	ScopedLock Al(MapGraphLock);

	WaitForLoadingThread();
	NeedMapLoad=false;
	//world change?
	if (NewW!=PosW)
	{
		//we need to load a new map and a new Id cache
		PosX=NewX;
		PosY=NewY;
		PosW=NewW;
		NeedMapLoad=true;
		ActualCacheIndex=FindClosestCacheIndex(PosX,PosY);
		LoadMap();
	} else
	//position change only? 
	if ((NewX!=PosX) || (NewY!=PosY))
	{
		//we need to change cache index and load/unload graph accordingly
		PosX=NewX;
		PosY=NewY;
		NeedMapLoad=false;
		int NewCacheIndex=FindClosestCacheIndex(PosX,PosY);
		if (NewCacheIndex!=ActualCacheIndex)
		{
			ActualCacheIndex=NewCacheIndex;
			LoadMap();
		};
	}
};


long WINAPI LoadingThreadFunc(TMapLoader* Arg)
{
	return Arg->LoadingThread();
};

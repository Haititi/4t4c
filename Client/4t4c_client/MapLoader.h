#ifndef MAPLOADER_H
#define MAPLOADER_H

#include "Headers.h"
#include "SpriteDatabase.h"
//map types

const unsigned long MaxIdNumber=8192;

//id type const
const unsigned long IdType_Floor	=0; //Standard floor
const unsigned long IdType_Blend	=1; //2 blended floor
const unsigned long IdType_Single	=2; //just 1 sprite
const unsigned long IdType_Multi	=3; //coordinate dependent
const unsigned long IdType_Composite=4; //draw multiple sprite at the same time
const unsigned long IdType_Animated =5; //Animated sprite

//Light type const
const unsigned long IdLight_NoShadow  =0;  //if the bitfield is equal to zero we process nothing
const unsigned long IdLight_DynShadow =1;
const unsigned long IdLight_CastLight =2;
const unsigned long IdLight_BlockLight=4;

struct TMapIdInfoGr
{
	char* GraphName;
	PSiInfo Graph;            //corresponding si entry
	unsigned long VertexColor;		//Color Modifier : 0xffffffff = no modif
	signed short Offx,Offy,Offz; //offset correction
};
typedef TMapIdInfoGr* PMapIdInfoGr;

struct TMapIdInfo
{
	unsigned long Defined;   //is this id defined ?
	unsigned long Loaded;    //graph for that id loaded
	unsigned long Static;    //a static Id is never unloaded from memory
	unsigned long IdType;	 //0:floor 1:floor blend 2:single 3:multi 5:Composite 4:animated
	unsigned long Reversed;			//need to be drawn reversed
	unsigned long Modx,Mody; //number of graph in x and y dir  //for Idtype=1  blend id 1 & 2
	unsigned long MapColor;  //color on Minimap
	float MiniTextX; //texturing coordinate for minimap object
	float MiniTextY; 
	float MiniTextX2; 
	float MiniTextY2; 
	unsigned long LightInfo;        //Bitfield 1:no shadow 2:need to draw dynamic shadow 4:this object cast light 8:this object block light
	unsigned long ColorFx;
	unsigned long GraphCount; //number of graph for this Id
	PMapIdInfoGr GraphInfo;  //the actual array of Id Info  //for idtype1 point to an alpha blend map
	float UnitU;	//texturing units for ground
	float UnitV;
};
typedef TMapIdInfo *PMapIdInfo;

struct TMapName
{
	char* MapName;
};
typedef TMapName *PMapName;

//map id cache info
struct TIndexArrItem 
{
	int X;
	int Y;
	unsigned long IdCount;  //number of id for that index
	unsigned short* IdList;  //list of the id to load
};

class TMapLoader
{
	friend long WINAPI LoadingThreadFunc(TMapLoader* Arg);
	friend class TWorld;
	private:
		unsigned short Map[3072*3072]; 
		TMapIdInfo MapIdInfo[MaxIdNumber]; 

		unsigned long MapCount;
		PMapName MapName;

		int PosX,PosY,PosW;
		//new "cache" system
		TIndexArrItem IndexArray[24*24*2];
		unsigned long ActualCacheIndex;

		CriticalSection MapGraphLock;  //used to protect access to MapIdInfo When Loading Take Place 
											//the map itself don't really need to be protected : it's a static array ...
		bool LoadingFinished;  //true while loading
		bool NeedMapLoad;
		bool StopLoad;      //Set to true to finish directly loading
		HANDLE LoadingThreadHdl;

		void LoadId(int n); //Load graph for a map id with help of database
		long LoadingThread(void);
		void ReadIndexFile(const char* FileName);
		unsigned long FindClosestCacheIndex(const int PosX,const int PosY);
		void LoadMapIdInfo(void); //load Maps Name and corresponding info
		void LoadMap(void);//load a specific map , cf:LoadMapInfo
		inline bool IsLoadingFinished(void){return LoadingFinished;};
		void WaitForLoadingThread(void);
	public:
		TMapLoader(void);
		~TMapLoader(void);
		void Initialize(void); //to be called after graphic init AND after database init
		inline int GetMapCount(void){return MapCount;};//number of map
		void ChangePosition(int NewX,int NewY,int NewW);
		inline unsigned short GetMapXY(const int X,const int Y)
		{
			unsigned int X2,Y2;
			X2=X; Y2=Y;
			if (X2<0)
				X2=0;
			else
				if (X2>=3072)
					X2=3071;
			if (Y2<0)
				Y2=0;
			else
				if (Y2>=3072)
					Y2=3071;

				return Map[X2+Y2*3072];
		}; 
		inline PMapIdInfo GetIdInfo(int Id){return &MapIdInfo[Id];};//access an Id 
};

extern TMapLoader MapLoader;


#endif
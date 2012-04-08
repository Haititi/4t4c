#ifndef GFXENGINE_H
#define GFXENGINE_H

#include "Headers.h"
#include "d3d9.h"
#include "SystemInfo.h"
//#include "Global.h"


const unsigned long CLEAR_ALL_BUFFER=D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;
const unsigned long SPRITEFVF=D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const unsigned long POINTFVF=D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

#define RGB32(r,g,b) 0xFF000000|(r<<16)|(g<<8)|b
#define ARGB32(a,r,g,b) (a<<24)|(r<<16)|(g<<8)|b

const unsigned long COLOR32_BLACK		=0xFF000000;
const unsigned long COLOR32_WHITE		=0xFFFFFFFF;
const unsigned long COLOR32_RED			=0xFFFF0000;
const unsigned long COLOR32_GREEN		=0xFF00FF00;
const unsigned long COLOR32_BLUE		=0xFF0000FF;
const unsigned long COLOR32_GREY		=0xFF7F7F7F;
const unsigned long COLOR32_ORANGE		=0xFFE05000;
const unsigned long COLOR32_LIGHTGREY	=0xFFA0A0A0;
const unsigned long COLOR32_DARKGREY	=0xFF505050;

#pragma pack(push)
#pragma pack(4)
typedef unsigned short TIndexElem;
struct TCustVertex
{
	float x,y,z,h;
	D3DCOLOR col;
	float tu,tv;
};

typedef TCustVertex* PCustVertex;

struct TPointVertex
{
	float x,y,z,h;
	D3DCOLOR col;
};

typedef TCustVertex TSpritePlane[4];

const TSpritePlane RefQuad=
{	{0.0f,0.0f,0.0f,1.0f,0xFFFFFFFF,0,0},
	{0.0f,0.0f,0.0f,1.0f,0xFFFFFFFF,1,0},
	{0.0f,0.0f,0.0f,1.0f,0xFFFFFFFF,0,1},
	{0.0f,0.0f,0.0f,1.0f,0xFFFFFFFF,1,1}
};

const int BaseVertexSize=sizeof(TCustVertex);

typedef TSpritePlane* PSpritePlane;

typedef TCustVertex TBaseLine[2];
typedef TBaseLine* PBaseLine;

#pragma pack(pop)

struct TTcs
{
	bool Shading;
	unsigned long Alpha;
	float Bending;
	float Cosa,Sina;
};
typedef TTcs* PTcs;


extern LPDIRECT3DTEXTURE9 WhiteTexture;

enum TRenderMode
{
	RdrMode_Ground,
	RdrMode_Lightning
};

class TGfxEngine
{
	private:
		//friend class TGameUi;
		LPDIRECT3D9 Direct3d;
		LPDIRECT3DDEVICE9 Direct3dDev;
		D3DGAMMARAMP GammaRamp;

		unsigned long GWidth,GHeight;
	    unsigned long ClearMethod;

		unsigned long PixelCacheSize,PixelCacheCapacity;
		PCustVertex PixelCache;
		
		unsigned long LineCacheSize,LineCacheCapacity;
		PBaseLine LineCache;

		LPDIRECT3DTEXTURE9 CachedQuadTexture;
		unsigned long QuadCacheSize,QuadCacheCapacity;
		PCustVertex QuadCache;

		LPDIRECT3DSURFACE9 OrigTarget;
		LPDIRECT3DTEXTURE9 BackBuffer;

		unsigned long TakeScreen;
		unsigned long BaseMemory;
		void TakeScreenShoot(void); //make the screenshoot

	public:
		TGfxEngine(void);
		~TGfxEngine(void);
		void Initialize(const bool Windowed,const unsigned long Resx,const unsigned long Resy, const unsigned long Rate,const unsigned long Sync);
		unsigned long GetAvailableVidMem(void);
		unsigned long GetAllocatedVidMem(void);
		void SetRenderMode(TRenderMode NewMode);
		void SetFilter(const bool State); 
		void SetZBuffer(const bool State);
		void D3dSceneStart(void); 
		void D3dSceneEnd(void);
		LPDIRECT3DTEXTURE9 CreateTexture(unsigned long Sizex,unsigned long Sizey,D3DFORMAT Format);
		inline void SetFvf(unsigned long Fvf){ Direct3dDev->SetFVF(Fvf); };

		void RenderToTexture(void);
		void RenderToScreen(void);
		LPDIRECT3DTEXTURE9 GetBackBufferTexture(void);
		//test
		bool PixelHitTest(const LPDIRECT3DTEXTURE9 Texture,long PosX,long PosY);
		//void RenderText(float Tx,float Ty,const char* Text,D3DCOLOR Color);
		//void RenderText(float Tx,float Ty,float TextSize,const char* Text,D3DCOLOR Color); //textsize is a ratio here    , 2 is equal to a double text size
		//ground tile render func
		void DrawTile(const float Posx,const float Posy,const float U1,const float U2,const float V1,const float V2,const float Depth,const float Sizex,const float Sizey,const D3DCOLOR Color,const LPDIRECT3DTEXTURE9 Texture);	
		void DrawSprite(const PSpritePlane SpritePlane,const LPDIRECT3DTEXTURE9 Texture=WhiteTexture);
		//draw a quad list with a given texture
		void DrawBatch(const PSpritePlane QuadList,const unsigned long QuadCount,const LPDIRECT3DTEXTURE9 Texture);
		//standard sprite drawing with depth control
		void DrawSprite(const float Posx,const float Posy,const float Depth,const float Sizex,const float Sizey,const D3DCOLOR Color,const LPDIRECT3DTEXTURE9 Texture);
		void DrawSpriteFlip(const float Posx,const float Posy,const float Depth,const float Sizex,const float Sizey,const D3DCOLOR Color,const LPDIRECT3DTEXTURE9 Texture);
		//render a sprite as a shadow
		void DrawSpriteAsShade(float Posx,float Posy,float Depth,float Sizex,float Sizey,unsigned long Rev,TTcs &Tcs,LPDIRECT3DTEXTURE9 Texture);//the Environement system is tracking time and therefore will provide the tcs (Time Correction Structure)
		void DrawAlphaSmooth(float Posx,float Posy,float Sizex,float Sizey,LPDIRECT3DTEXTURE9 Alpha,LPDIRECT3DTEXTURE9 Texture1,LPDIRECT3DTEXTURE9 Texture2);
		void DrawAlphaSmoothUV(float Posx,float Posy,float Sizex,float Sizey,
								   const float U1A,const float U2A,const float V1A,const float V2A,
								   const float U1B,const float U2B,const float V1B,const float V2B,
								   LPDIRECT3DTEXTURE9 Alpha,LPDIRECT3DTEXTURE9 Texture1,LPDIRECT3DTEXTURE9 Texture2);
		//special interface drawing func
		void DrawTilingTexture(const float PosX,const float PosY,const float Depth,const float QuadSizeX,const float QuadSizeY,const float TextSizeX,const float TextSizeY,LPDIRECT3DTEXTURE9 Texture,const unsigned long Color); 
		void DrawPixel(float PosX,float PosY,unsigned long Color);
		void DrawLine(float PosX1,float PosY1,float PosX2,float PosY2,unsigned long Color);
		void FillRect(const float PosX,const float PosY,const float Depth,const float Sx,const float Sy,unsigned long Color);

		void CachedDrawPixel(float PosX,float PosY,unsigned long Color); //note: the cache is allocated with a 64 entry granularity
		void FlushPixelCache(void);//this function draw all cached pixel and reset the cache
		void ClearPixelCache(void);//this function Deallocate the cache

		void CachedDrawRect(float PosX1,float PosY1,float Sizex,float Sizey,unsigned long Color);
		void CachedDrawLine(float PosX1,float PosY1,float PosX2,float PosY2,unsigned long Color);
		void FlushLineCache(void);
		void ClearLineCache(void);

		void SetCachedQuadTexture(const LPDIRECT3DTEXTURE9 Texture);
		void CachedDrawQuad(const PSpritePlane SpritePlane);
		void FlushQuadCache(void);
		void ClearQuadCache(void);

		inline void ScreenShoot(void){ TakeScreen=1;}; //ask for a screenshoot

		inline int ScreenWidth(){return GWidth;};
		inline int ScreenHeight(){return GHeight;};

};

extern TGfxEngine GfxCore;

//LPDIRECT3DTEXTURE9 CreateTextureFromPng(TString Filename);
LPDIRECT3DTEXTURE9 CreateTextureFromZmb(const char* Filename);
const char * GetDxErrorStr(unsigned long Error);

#endif


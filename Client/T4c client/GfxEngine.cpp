#include "GfxEngine.h"
#include "d3d9.h"
#include "debug.h"
#include "fastStream.h"
#include "zlib.h"
#include "App.h"

TGfxEngine GfxCore;
LPDIRECT3DTEXTURE9 WhiteTexture;

//Index Buffer for triangle list (working)
TIndexElem IndexData[32768];

//internal struct and type for blending
const unsigned long BLENDSPRITEFVF=D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2;

struct TCustBlendVertex
{
	float x,y,z,h;
	D3DCOLOR col;
	float tu,tv;
	float tu2,tv2;
};

typedef TCustBlendVertex TBlendSpritePlane[4];
const int BaseBlendVertexSize=sizeof(TCustBlendVertex);


TGfxEngine::TGfxEngine(void)
{
	ClearMethod=CLEAR_ALL_BUFFER;
	Direct3d=0;
	Direct3dDev=0;

	LineCache=NULL;
	LineCacheCapacity=
	LineCacheSize=0;

	PixelCache=NULL;
	PixelCacheCapacity=
	PixelCacheSize=0;

	TakeScreen=0;

	QuadCacheSize=0;
	QuadCacheCapacity=0;
	QuadCache=0;

	WhiteTexture=0;


	//div 6 because we're defining index for a quad = 2 tri = 6 vertex
	for(unsigned int i=0;i<(32768/6);i++)
	{
		IndexData[i*6+0]=i*4+0;
		IndexData[i*6+1]=i*4+1;
		IndexData[i*6+2]=i*4+2;
		IndexData[i*6+3]=i*4+2;
		IndexData[i*6+4]=i*4+1;
		IndexData[i*6+5]=i*4+3;
	}
};

TGfxEngine::~TGfxEngine(void)
{
	if (WhiteTexture)
		WhiteTexture->Release();
	if (PixelCache)
		ClearPixelCache();
	if (LineCache)
		ClearLineCache();

	if (Direct3dDev)
		Direct3dDev->Release();
	if (Direct3d)
		Direct3d->Release();
};

void TGfxEngine::Initialize(const bool Windowed,const unsigned long Resx,const unsigned long Resy, const unsigned long Rate,const unsigned long Sync)
{
	D3DPRESENT_PARAMETERS D3dPp;
	HRESULT hr;

	GWidth=Resx;
	GHeight=Resy;

	Direct3d=Direct3DCreate9(D3D_SDK_VERSION);
	if (!Direct3d)
	{
		LOG("Gfx : Unable to create Direct3d\r\n");
		App.Close("Gfx : Unable to create Direct3d\r\n",Err_Directx);
	}

	ZeroMemory(&D3dPp, sizeof(D3DPRESENT_PARAMETERS));
	D3dPp.BackBufferWidth=Resx;
	D3dPp.BackBufferHeight=Resy;
	D3dPp.FullScreen_RefreshRateInHz=Rate;
	D3dPp.Windowed = Windowed;
	D3dPp.BackBufferCount=1;
	D3dPp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3dPp.BackBufferFormat = D3DFMT_A8R8G8B8;
	D3dPp.EnableAutoDepthStencil = true;
	D3dPp.AutoDepthStencilFormat = D3DFMT_D16 ;
	D3dPp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
	if (Sync==0)
		D3dPp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
	
	//CM TODO add new config variable 
	//if (Config::VSync==0) 
	//	D3dPp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
	if (!Windowed)
		D3dPp.FullScreen_RefreshRateInHz=85;

	//CM D3DCREATE_MULTITHREADED is safer but is slower too, should check if really needed
	//since we use a lot of system memory based vertex ,Processing must be kept at D3DCREATE_SOFTWARE_VERTEXPROCESSING
	
//#define NVIDIA_PERF_HUD
#ifdef NVIDIA_PERF_HUD
	unsigned int AdapterToUse=D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType=D3DDEVTYPE_HAL;
	for (unsigned int Adapter=0;Adapter<Direct3d->GetAdapterCount();Adapter++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		HRESULT Res;
		Res = Direct3d->GetAdapterIdentifier(Adapter,0,&Identifier);
		if (strstr(Identifier.Description,"PerfHUD") != 0)
		{
			AdapterToUse=Adapter;
			DeviceType=D3DDEVTYPE_REF;
			break;
		}
	}
	hr=Direct3d->CreateDevice(AdapterToUse, DeviceType, SystemInfo->GethWnd(),
						    D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
							&D3dPp, &Direct3dDev);
#else
	hr=Direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, TSystemInfo::Inst().GethWnd(),
						    D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
							&D3dPp, &Direct3dDev);
#endif
	if (hr!=0)
	{
		LOG("Unable to create Direct3d Device\r\n");
		App.Close("Unable to create Direct3d Device",Err_Directx);
	}

	//setup Standard rendering states
	Direct3dDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT );   //anisotropic is overkill i think
	Direct3dDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	Direct3dDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
	//texture Adressing to wrap //don't confuse with texture wrapping ???§!!
	Direct3dDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	Direct3dDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	//
	Direct3dDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	Direct3dDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	
	//Direct3dDev->SetRenderState(D3DRS_WRAP0,0); //wrapping texture 0= u 1=v 2=w

	Direct3dDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	Direct3dDev->SetRenderState(D3DRS_LIGHTING, false);

	Direct3dDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	//Direct3dDev->SetRenderState(D3DRS_ZWRITEENABLE,D3DZB_TRUE);ar...
	Direct3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);

	Direct3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,  true );
	Direct3dDev->SetRenderState(D3DRS_ALPHAREF, (unsigned long)0x00000000);
	Direct3dDev->SetRenderState(D3DRS_ALPHATESTENABLE, true);
	Direct3dDev->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATER);
	Direct3dDev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	Direct3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	
	Direct3dDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	Direct3dDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	Direct3dDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	Direct3dDev->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT);

	Direct3dDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
	Direct3dDev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

	//blend setup
	Direct3dDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);//slectarg1 or modulate
	Direct3dDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//Direct3dDev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE); //later could be used to modulate ground
	Direct3dDev->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	Direct3dDev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);


	//ensure the first stage is not used
	Direct3dDev->SetTexture(1,0);

	

	SetFvf(SPRITEFVF);

	BaseMemory=GetAvailableVidMem();

	//need a white texture for diverse use
	_D3DLOCKED_RECT LockedRect;
	WhiteTexture=CreateTexture(4,4,D3DFMT_A8R8G8B8);
	WhiteTexture->LockRect(0,&LockedRect,NULL,D3DLOCK_DISCARD);
	memset(LockedRect.pBits,0xFF,4*LockedRect.Pitch);
	WhiteTexture->UnlockRect(0);

	//back buffer texture
	Direct3dDev->CreateTexture(Resx,Resy,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&BackBuffer,0);

	//save the original render target
	Direct3dDev->GetRenderTarget(0,&OrigTarget);


	CachedQuadTexture=WhiteTexture;
};

unsigned long TGfxEngine::GetAvailableVidMem(void)
{
	return Direct3dDev->GetAvailableTextureMem();
};
unsigned long TGfxEngine::GetAllocatedVidMem(void)
{
	return BaseMemory-GetAvailableVidMem();
};

LPDIRECT3DTEXTURE9 TGfxEngine::CreateTexture(unsigned long Sizex,unsigned long Sizey,D3DFORMAT Format)
{
	LPDIRECT3DTEXTURE9 Texture;	
	//debug
	//Log<< "Vid mem : "<< (unsigned long)Direct3dDev->GetAvailableTextureMem()<<"\r\n";

	if (Direct3dDev->CreateTexture(Sizex,Sizey,1,D3DUSAGE_DYNAMIC,Format,D3DPOOL_DEFAULT,&Texture,NULL)==D3DERR_INVALIDCALL)
	{
		MessageBox(NULL,"Gfx : Invalid CreateTexture Call","Dx Error",0);
		LOG("Inv Text Call"<< Sizex << "-" << Sizey << "-" << Format);
	}
	return Texture;
};

void TGfxEngine::SetFilter(const bool State)
{
	if (State)
	{
		Direct3dDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );  
		Direct3dDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}else
	{
		Direct3dDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT );  
		Direct3dDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	}
};

void TGfxEngine::SetZBuffer(const bool State)
{
	if (State)
	{
		Direct3dDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE); 
	}else
	{
		Direct3dDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE); 
	}
};

void TGfxEngine::SetRenderMode(TRenderMode NewMode)
{
	switch (NewMode)
	{
	case RdrMode_Ground:
		SetZBuffer(true);
		//Direct3dDev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		//Direct3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		//Direct3dDev->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
		break;
	case RdrMode_Lightning:
		SetZBuffer(false);
		//Direct3dDev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCCOLOR );
		//Direct3dDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
		//Direct3dDev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT );
		break;
	};
};

LPDIRECT3DTEXTURE9 TGfxEngine::GetBackBufferTexture(void)
{
	return BackBuffer;
};

void TGfxEngine::RenderToTexture(void)
{
	LPDIRECT3DSURFACE9 Surface;
	BackBuffer->GetSurfaceLevel(0,&Surface);
	Direct3dDev->SetRenderTarget(0,Surface);
	Surface->Release();
};

void TGfxEngine::RenderToScreen(void)
{
	Direct3dDev->SetRenderTarget(0,OrigTarget);
};

void TGfxEngine::D3dSceneStart(void)
{
	Direct3dDev->Clear( 0, NULL, ClearMethod, 0, 1.0f, 0 );
	Direct3dDev->BeginScene();
};

void TGfxEngine::D3dSceneEnd(void)
{
	Direct3dDev->EndScene();
	if (TakeScreen)
	{
		TakeScreen=0;
		TakeScreenShoot();
	}
	Direct3dDev->Present(NULL,NULL,0,NULL);
};

bool TGfxEngine::PixelHitTest(const LPDIRECT3DTEXTURE9 Texture,long PosX,long PosY)
{
	D3DSURFACE_DESC Desc;
	_D3DLOCKED_RECT LockedRect;
	RECT LockPix;
	LockPix.left=PosX;
	LockPix.top=PosY;
	LockPix.right=PosX+1;
	LockPix.bottom=PosY+1;

	Texture->GetLevelDesc(0,&Desc);
	if (Texture->LockRect(0,&LockedRect,&LockPix,D3DLOCK_READONLY)==D3D_OK)
	{
		unsigned long Value=*((unsigned long*)(LockedRect.pBits));
		Texture->UnlockRect(0);

		switch(Desc.Format)
		{
			case D3DFMT_A8R8G8B8:
				return (Value & 0xFF000000)>0;
			break;

			case D3DFMT_A1R5G5B5:
				return (Value & 0x8000)>0;
			break;
		}
	}
	
	return false;
};

void TGfxEngine::DrawAlphaSmooth(float Posx,float Posy,float Sizex,float Sizey,LPDIRECT3DTEXTURE9 Alpha,LPDIRECT3DTEXTURE9 Texture1,LPDIRECT3DTEXTURE9 Texture2)
{       //CM: THIS IS A TEST ROUTINE
	TSpritePlane Sprite;
	
	Sprite[0].x=Posx;
	Sprite[0].y=Posy;
	Sprite[0].z=0.95f;
	Sprite[0].h=1.0f;
	Sprite[0].col=0xFFFFFFFF;
	Sprite[0].tu=0.0f;
	Sprite[0].tv=0.0f;

	Sprite[1].x=Posx+Sizex;
	Sprite[1].y=Posy;
	Sprite[1].z=0.95f;
	Sprite[1].h=1.0f;
	Sprite[1].col=0xFFFFFFFF;
	Sprite[1].tu=1.0f;
	Sprite[1].tv=0.0f;

	Sprite[2].x=Posx;
	Sprite[2].y=Posy+Sizey;
	Sprite[2].z=0.95f;
	Sprite[2].h=1.0f;
	Sprite[2].col=0xFFFFFFFF;
	Sprite[2].tu=0.0f;
	Sprite[2].tv=1.0f;

	Sprite[3].x=Sprite[1].x;
	Sprite[3].y=Sprite[2].y;
	Sprite[3].z=0.95f;
	Sprite[3].h=1.0f;
	Sprite[3].col=0xFFFFFFFF;
	Sprite[3].tu=1.0f;
	Sprite[3].tv=1.0f;

	//1st   draw the texture1 normally
	Direct3dDev->SetTexture(0,Texture1);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);


	//2nd setup environnement an do the next pass
	//Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);


	Direct3dDev->SetTexture(0,Alpha);
	Direct3dDev->SetTexture(1,Texture2);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);


	//3rd restore environnement

	Direct3dDev->SetTexture(1,NULL); //set to null so the stage 1 is no more used
	
	//Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
};

void TGfxEngine::DrawAlphaSmoothUV(float Posx,float Posy,float Sizex,float Sizey,
								   const float U1A,const float U2A,const float V1A,const float V2A,
								   const float U1B,const float U2B,const float V1B,const float V2B,
								   LPDIRECT3DTEXTURE9 Alpha,LPDIRECT3DTEXTURE9 Texture1,LPDIRECT3DTEXTURE9 Texture2)
{       //CM: THIS IS A TEST ROUTINE
	TSpritePlane Sprite;
	
	Sprite[0].x=Posx;
	Sprite[0].y=Posy;
	Sprite[0].z=0.98f;
	Sprite[0].h=1.0f;
	Sprite[0].col=0xFFFFFFFF;
	Sprite[0].tu=U1A;
	Sprite[0].tv=V1A;

	Sprite[1].x=Posx+Sizex;
	Sprite[1].y=Posy;
	Sprite[1].z=0.98f;
	Sprite[1].h=1.0f;
	Sprite[1].col=0xFFFFFFFF;
	Sprite[1].tu=U2A;
	Sprite[1].tv=V1A;

	Sprite[2].x=Posx;
	Sprite[2].y=Posy+Sizey;
	Sprite[2].z=0.98f;
	Sprite[2].h=1.0f;
	Sprite[2].col=0xFFFFFFFF;
	Sprite[2].tu=U1A;
	Sprite[2].tv=V2A;

	Sprite[3].x=Sprite[1].x;
	Sprite[3].y=Sprite[2].y;
	Sprite[3].z=0.98f;
	Sprite[3].h=1.0f;
	Sprite[3].col=0xFFFFFFFF;
	Sprite[3].tu=U2A;
	Sprite[3].tv=V2A;

	//1st   draw the texture1 normally
	Direct3dDev->SetTexture(0,Texture1);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);

	TBlendSpritePlane Sprite2;

	//2nd setup environnement an do the next pass
	//Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	//FIX Problemhere we need different texture coordinate for the alpha and the texture2
	Sprite2[0].x=Posx;
	Sprite2[0].y=Posy;
	Sprite2[0].z=0.98f;
	Sprite2[0].h=1.0f;
	Sprite2[0].col=0xFFFFFFFF;
	Sprite2[0].tu=0.0f;
	Sprite2[0].tv=0.0f;
	Sprite2[0].tu2=U1B;
	Sprite2[0].tv2=V1B;

	Sprite2[1].x=Posx+Sizex;
	Sprite2[1].y=Posy;
	Sprite2[1].z=0.98f;
	Sprite2[1].h=1.0f;
	Sprite2[1].col=0xFFFFFFFF;
	Sprite2[1].tu=1.0f;
	Sprite2[1].tv=0.0f;
	Sprite2[1].tu2=U2B;
	Sprite2[1].tv2=V1B;

	Sprite2[2].x=Posx;
	Sprite2[2].y=Posy+Sizey;
	Sprite2[2].z=0.98f;
	Sprite2[2].h=1.0f;
	Sprite2[2].col=0xFFFFFFFF;
	Sprite2[2].tu=0.0f;
	Sprite2[2].tv=1.0f;
	Sprite2[2].tu2=U1B;
	Sprite2[2].tv2=V2B;

	Sprite2[3].x=Sprite2[1].x;
	Sprite2[3].y=Sprite2[2].y;
	Sprite2[3].z=0.98f;
	Sprite2[3].h=1.0f;
	Sprite2[3].col=0xFFFFFFFF;
	Sprite2[3].tu=1.0f;
	Sprite2[3].tv=1.0f;
	Sprite2[3].tu2=U2B;
	Sprite2[3].tv2=V2B;

	Direct3dDev->SetFVF(BLENDSPRITEFVF);
	Direct3dDev->SetTexture(0,Alpha);
	Direct3dDev->SetTexture(1,Texture2);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite2,BaseBlendVertexSize);


	//3rd restore environnement

	Direct3dDev->SetTexture(1,NULL); //set to null so the stage 1 is no more used
	Direct3dDev->SetFVF(SPRITEFVF);
	
	//Direct3dDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
};

/*
ps_1_4////////////////////////////////// r0: alphamaps
// r1 - r4: textures////////////////////////////////// 
Sample texturestexld r0, t0texld r1, t1texld r2, t1texld r3, t1texld r4, t1
// Combine the textures together based off of their alphamaps
mul r1, r1, r0.x
lrp r2, r0.y, r2, 
r1lrp r3, r0.z, r3, 
r2lrp r0, r0.w, r4, r3
*/

void TGfxEngine::DrawSprite(const PSpritePlane SpritePlane,const LPDIRECT3DTEXTURE9 Texture)
{
	Direct3dDev->SetTexture(0,Texture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,SpritePlane,BaseVertexSize);
};
		
void TGfxEngine::DrawBatch(const PSpritePlane QuadList,const unsigned long QuadCount,const LPDIRECT3DTEXTURE9 Texture)
{
	//TIndexElem *IndexData=new TIndexElem[QuadCount*4];
	/*for(int i=0;i<QuadCount;i++)
	{
		IndexData[i*4+0]=i*4+0;
		IndexData[i*4+1]=i*4+1;
		IndexData[i*4+2]=i*4+2;
		IndexData[i*4+3]=i*4+3;
	}*/

	Direct3dDev->SetTexture(0,Texture);
	Direct3dDev->SetFVF(SPRITEFVF);
	Direct3dDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,QuadCount*4,QuadCount*2,IndexData,D3DFMT_INDEX16,QuadList,BaseVertexSize);
	//delete [] IndexData;
}

void TGfxEngine::DrawTilingTexture(const float PosX,const float PosY,const float Depth,const float QuadSizeX,const float QuadSizeY,const float TextSizeX,const float TextSizeY,LPDIRECT3DTEXTURE9 Texture,const unsigned long Color)
{
	TSpritePlane Sprite;

	const float CoordTextU=QuadSizeX/TextSizeX;
	const float CoordTextV=QuadSizeY/TextSizeY;

	Sprite[0].x=PosX-0.5f;
	Sprite[0].y=PosY-0.5f;
	Sprite[0].z=Depth;
	Sprite[0].h=1.0f;
	Sprite[0].col=Color;
	Sprite[0].tu=0;
	Sprite[0].tv=0;

	Sprite[1].x=PosX+QuadSizeX-0.5f;
	Sprite[1].y=PosY-0.5f;
	Sprite[1].z=Depth;
	Sprite[1].h=1.0f;
	Sprite[1].col=Color;
	Sprite[1].tu=CoordTextU;
	Sprite[1].tv=0;

	Sprite[2].x=PosX-0.5f;
	Sprite[2].y=PosY+QuadSizeY-0.5f;
	Sprite[2].z=Depth;
	Sprite[2].h=1.0f;
	Sprite[2].col=Color;
	Sprite[2].tu=0;
	Sprite[2].tv=CoordTextV;

	Sprite[3].x=Sprite[1].x;
	Sprite[3].y=Sprite[2].y;
	Sprite[3].z=Depth;
	Sprite[3].h=1.0f;
	Sprite[3].col=Color;
	Sprite[3].tu=CoordTextU;
	Sprite[3].tv=CoordTextV;

	Direct3dDev->SetTexture(0,Texture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);
}

void TGfxEngine::DrawTile(const float Posx,const float Posy,const float U1,const float U2,const float V1,const float V2,const float Depth,const float Sizex,const float Sizey,const D3DCOLOR Color,const LPDIRECT3DTEXTURE9 Texture)
{
	TSpritePlane Sprite;

	Sprite[0].x=Posx;
	Sprite[0].y=Posy;
	Sprite[0].z=Depth;
	Sprite[0].h=1.0f;
	Sprite[0].col=Color;
	Sprite[0].tu=U1;
	Sprite[0].tv=V1;

	Sprite[1].x=Posx+Sizex;
	Sprite[1].y=Posy;
	Sprite[1].z=Depth;
	Sprite[1].h=1.0f;
	Sprite[1].col=Color;
	Sprite[1].tu=U2;
	Sprite[1].tv=V1;

	Sprite[2].x=Posx;
	Sprite[2].y=Posy+Sizey;
	Sprite[2].z=Depth;
	Sprite[2].h=1.0f;
	Sprite[2].col=Color;
	Sprite[2].tu=U1;
	Sprite[2].tv=V2;

	Sprite[3].x=Sprite[1].x;
	Sprite[3].y=Sprite[2].y;
	Sprite[3].z=Depth;
	Sprite[3].h=1.0f;
	Sprite[3].col=Color;
	Sprite[3].tu=U2;
	Sprite[3].tv=V2;

	Direct3dDev->SetTexture(0,Texture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);
};

void TGfxEngine::DrawSprite(const float Posx,const float Posy,const float Depth,const float Sizex,const float Sizey,const D3DCOLOR Color,const LPDIRECT3DTEXTURE9 Texture)
{
	TSpritePlane Sprite;

	Sprite[0].x=Posx;
	Sprite[0].y=Posy;
	Sprite[0].z=Depth;
	Sprite[0].h=1;
	Sprite[0].col=Color;
	Sprite[0].tu=0;
	Sprite[0].tv=0;

	Sprite[1].x=Posx+Sizex;
	Sprite[1].y=Posy;
	Sprite[1].z=Depth;
	Sprite[1].h=1;
	Sprite[1].col=Color;
	Sprite[1].tu=1;
	Sprite[1].tv=0;

	Sprite[2].x=Posx;
	Sprite[2].y=Posy+Sizey;
	Sprite[2].z=Depth;
	Sprite[2].h=1;
	Sprite[2].col=Color;
	Sprite[2].tu=0;
	Sprite[2].tv=1;

	Sprite[3].x=Sprite[1].x;
	Sprite[3].y=Sprite[2].y;
	Sprite[3].z=Depth;
	Sprite[3].h=1;
	Sprite[3].col=Color;
	Sprite[3].tu=1;
	Sprite[3].tv=1;

	Direct3dDev->SetTexture(0,Texture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);
};

void TGfxEngine::DrawSpriteFlip(const float Posx,const float Posy,const float Depth,const float Sizex,const float Sizey,const D3DCOLOR Color,const LPDIRECT3DTEXTURE9 Texture)
{
	TSpritePlane Sprite;

	Sprite[0].x=Posx;
	Sprite[0].y=Posy;
	Sprite[0].z=Depth;
	Sprite[0].h=1;
	Sprite[0].col=Color;
	Sprite[0].tu=1;
	Sprite[0].tv=0;

	Sprite[1].x=Posx+Sizex;
	Sprite[1].y=Posy;
	Sprite[1].z=Depth;
	Sprite[1].h=1;
	Sprite[1].col=Color;
	Sprite[1].tu=0;
	Sprite[1].tv=0;

	Sprite[2].x=Posx;
	Sprite[2].y=Posy+Sizey;
	Sprite[2].z=Depth;
	Sprite[2].h=1;
	Sprite[2].col=Color;
	Sprite[2].tu=1;
	Sprite[2].tv=1;

	Sprite[3].x=Sprite[1].x;
	Sprite[3].y=Sprite[2].y;
	Sprite[3].z=Depth;
	Sprite[3].h=1;
	Sprite[3].col=Color;
	Sprite[3].tu=0;
	Sprite[3].tv=1;

	Direct3dDev->SetTexture(0,Texture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);
};
	

void TGfxEngine::DrawSpriteAsShade(float Posx,float Posy,float Depth,float Sizex,float Sizey,unsigned long Rev,TTcs &Tcs,LPDIRECT3DTEXTURE9 Texture)
{
	//todo : some tweaking needed here
    TSpritePlane Sprite;
	float Sa,Ca,Lg,Bx,By;

	if (!Tcs.Shading) return;
 
	Bx=Posx+Sizex;
	By=Posy+Sizey;
	
	Lg=Sizey/Tcs.Sina;   // c=a/cos(90-a1) ->  c=a/sin(a1)

	Sa=Tcs.Sina*Lg;
	Ca=Tcs.Cosa*Lg;

	Sprite[0].x=Bx+Ca;
	Sprite[0].y=By-Sa;
	Sprite[0].z=Depth;
	Sprite[0].h=1.0f;
	Sprite[0].col=Tcs.Alpha;
	Sprite[0].tu=(float)(1^Rev);
	Sprite[0].tv=0.0f;

	Sprite[1].x=Posx+Ca;
	Sprite[1].y=By-Sa;
	Sprite[1].z=Depth;
	Sprite[1].h=1.0f;
	Sprite[1].col=Tcs.Alpha;
	Sprite[1].tu=(float)(0^Rev);
	Sprite[1].tv=0.0f;

	Sprite[2].x=Bx;
	Sprite[2].y=By;
	Sprite[2].z=Depth;
	Sprite[2].h=1.0f;
	Sprite[2].col=Tcs.Alpha;
	Sprite[2].tu=(float)(1^Rev);
	Sprite[2].tv=1.0f;

	Sprite[3].x=Posx;
	Sprite[3].y=By;
	Sprite[3].z=Depth;
	Sprite[3].h=1.0f;
	Sprite[3].col=Tcs.Alpha;
	Sprite[3].tu=(float)(0^Rev);
	Sprite[3].tv=1.0f;

	Direct3dDev->SetTexture(0,Texture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);
};

void TGfxEngine::FillRect(const float PosX,const float PosY,const float Depth,const float Sx,const float Sy,unsigned long Color)
{
	TSpritePlane Sprite;

	Sprite[0].x=PosX;
	Sprite[0].y=PosY;
	Sprite[0].z=Depth;
	Sprite[0].h=1.0f;
	Sprite[0].col=Color;
	Sprite[0].tu=0.0f;
	Sprite[0].tv=0.0f;

	Sprite[1].x=PosX+Sx;
	Sprite[1].y=PosY;
	Sprite[1].z=Depth;
	Sprite[1].h=1.0f;
	Sprite[1].col=Color;
	Sprite[1].tu=1.0f;
	Sprite[1].tv=0.0f;

	Sprite[2].x=PosX;
	Sprite[2].y=PosY+Sy;
	Sprite[2].z=Depth;
	Sprite[2].h=1.0f;
	Sprite[2].col=Color;
	Sprite[2].tu=0.0f;
	Sprite[2].tv=1.0f;

	Sprite[3].x=Sprite[1].x;
	Sprite[3].y=Sprite[2].y;
	Sprite[3].z=Depth;
	Sprite[3].h=1.0f;
	Sprite[3].col=Color;
	Sprite[3].tu=1.0f;
	Sprite[3].tv=1.0f;

	Direct3dDev->SetTexture(0,WhiteTexture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,&Sprite,BaseVertexSize);
};


void TGfxEngine::DrawPixel(float PosX,float PosY,unsigned long Color)
{
	TCustVertex Vertex;

	Vertex.x=PosX;
	Vertex.y=PosY;
	Vertex.z=0.0f;
	Vertex.h=1.0f;
	Vertex.col=Color;
	Vertex.tu=0.0f;
	Vertex.tv=0.0f;

	Direct3dDev->SetTexture(0,WhiteTexture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_POINTLIST,1,&Vertex,BaseVertexSize);
};

void TGfxEngine::CachedDrawPixel(float PosX,float PosY,unsigned long Color)
{
	if (PixelCacheSize>=PixelCacheCapacity)
	{
		PixelCacheCapacity+=64;
		if (PixelCacheSize)
		{
			PixelCache=(PCustVertex)realloc(PixelCache,PixelCacheCapacity*sizeof(TCustVertex));
		} else
		{
			PixelCache=(PCustVertex)malloc(PixelCacheCapacity*sizeof(TCustVertex));
		}
	}

	PixelCache[PixelCacheSize].x=PosX;
	PixelCache[PixelCacheSize].y=PosY;
	PixelCache[PixelCacheSize].z=0.0f;
	PixelCache[PixelCacheSize].h=1.0f;
	PixelCache[PixelCacheSize].col=Color;
	PixelCache[PixelCacheSize].tu=0.0f;
	PixelCache[PixelCacheSize].tv=0.0f;

	PixelCacheSize++;
};

void TGfxEngine::FlushPixelCache(void)
{
	Direct3dDev->SetTexture(0,WhiteTexture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_POINTLIST,PixelCacheSize,PixelCache,BaseVertexSize);
	PixelCacheSize=0;
};

void TGfxEngine::ClearPixelCache(void)
{
	free(PixelCache);
	PixelCacheCapacity=0;
	PixelCacheSize=0;
};

void TGfxEngine::DrawLine(float PosX1,float PosY1,float PosX2,float PosY2,unsigned long Color)
{	
	TBaseLine Line;

	Line[0].x=PosX1;
	Line[0].y=PosY1;
	Line[0].z=0;
	Line[0].h=1;
	Line[0].col=Color;
	Line[0].tu=0;
	Line[0].tv=0;

	Line[1].x=PosX2;
	Line[1].y=PosY2;
	Line[1].z=0;
	Line[1].h=1;
	Line[1].col=Color;
	Line[1].tu=1;
	Line[1].tv=1;

	Direct3dDev->SetTexture(0,WhiteTexture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_LINELIST,1,&Line,BaseVertexSize);
};


void TGfxEngine::CachedDrawRect(float PosX1,float PosY1,float Sizex,float Sizey,unsigned long Color)
{
	CachedDrawLine(PosX1,PosY1,PosX1+Sizex,PosY1,Color); //h up
	CachedDrawLine(PosX1,PosY1+Sizey,PosX1+Sizex,PosY1+Sizey,Color); //h Down
	CachedDrawLine(PosX1,PosY1,PosX1+Sizex,PosY1+Sizey,Color);//v left
	CachedDrawLine(PosX1+Sizex,PosY1,PosX1+Sizex,PosY1+Sizey,Color);//v right
};

void TGfxEngine::CachedDrawLine(float PosX1,float PosY1,float PosX2,float PosY2,unsigned long Color)
{
	if (LineCacheSize>=LineCacheCapacity)
	{
		LineCacheCapacity+=64;
		if (LineCacheSize)
		{
			LineCache=(PBaseLine)realloc(LineCache,LineCacheCapacity*sizeof(TBaseLine));
		} else
		{
			LineCache=(PBaseLine)malloc(LineCacheCapacity*sizeof(TBaseLine));
		}
	}
	LineCache[LineCacheSize][0].x=PosX1;
	LineCache[LineCacheSize][0].y=PosY1;
	LineCache[LineCacheSize][0].z=0;
	LineCache[LineCacheSize][0].h=1;
	LineCache[LineCacheSize][0].col=Color;
	LineCache[LineCacheSize][0].tu=0;
	LineCache[LineCacheSize][0].tv=0;

	LineCache[LineCacheSize][1].x=PosX2;
	LineCache[LineCacheSize][1].y=PosY2;
	LineCache[LineCacheSize][1].z=0;
	LineCache[LineCacheSize][1].h=1;
	LineCache[LineCacheSize][1].col=Color;
	LineCache[LineCacheSize][1].tu=1;
	LineCache[LineCacheSize][1].tv=1;

	LineCacheSize++;
};

void TGfxEngine::FlushLineCache(void)
{
	Direct3dDev->SetTexture(0,WhiteTexture);
	Direct3dDev->DrawPrimitiveUP(D3DPT_LINELIST,LineCacheSize,LineCache,BaseVertexSize);
	LineCacheSize=0;
};

void TGfxEngine::ClearLineCache(void)
{
	free(LineCache);
	LineCacheCapacity=0;
	LineCacheSize=0;
};

/*
void TGfxEngine::TakeScreenShoot(void)
{
	LPDIRECT3DSURFACE9 surface = NULL;
	SYSTEMTIME CurTime;
	TString FileName;
	TString FullFileName;

    GetLocalTime( &CurTime );
	char File[64];

	sprintf(File, "%04u_%02u_%02u@%02uh%02um%02us", CurTime.wYear, CurTime.wMonth, CurTime.wDay, 
		CurTime.wHour,CurTime.wMinute, CurTime.wSecond);
	FileName = File;

	switch(Config::ScreenShootFormat)
	{
	case D3DXIFF_BMP:
		FileName += ".bmp";
		break;
	case D3DXIFF_JPG:
		FileName += ".jpg";
		break;
	case D3DXIFF_TGA:
		FileName += ".tga";
		break;
	case D3DXIFF_PNG:
		FileName += ".png";
		break;
	default:
		FileName += ".bmp";
		Config::ScreenShootFormat = D3DXIFF_BMP;
		break;
	}

	FullFileName = BaseDir + "\\Screens\\" + FileName;

	Direct3dDev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&surface);
    D3DXSaveSurfaceToFile(FullFileName.c_str(), (D3DXIMAGE_FILEFORMAT) Config::ScreenShootFormat, surface, NULL, NULL);

    surface->Release();
	GameUi.ConsoleMsg(("Screen taken (" + FileName + ")").c_str());
}*/

void TGfxEngine::TakeScreenShoot(void)
{
	TFastStream Fst;
	LPDIRECT3DSURFACE9 TargetSurface=NULL,Surface=NULL;
	D3DSURFACE_DESC Desc;
	_D3DLOCKED_RECT Lock;
	unsigned short WWord=0;
	unsigned long WDWord=1;
	unsigned long hr;
	
	//writing tga header (old format)
	Fst.WriteByte(0);//id length
	Fst.WriteByte(0);//clrmaptype
	Fst.WriteByte(2);//image type
	Fst.WriteLong(WDWord);//dummy color map info
	Fst.WriteByte(0);//dummy

	Fst.WriteWord(WWord);//xorig
	Fst.WriteWord(WWord);//yorig
	WWord=this->ScreenWidth();
	Fst.WriteWord(WWord);//width
	WWord=this->ScreenHeight();
	Fst.WriteWord(WWord);//height
	Fst.WriteByte(32);//BitDepth
	Fst.WriteByte(8);//imgDesc

	//todo : find what's wrong here

	//hr=Direct3dDev->GetRenderTarget(0,&TargetSurface);
	//LOG("GetRenderTarget " << GetDxErrorStr(hr));
	hr=Direct3dDev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&TargetSurface);
	LOG("GetBackbuf " << GetDxErrorStr(hr));
	hr=TargetSurface->GetDesc(&Desc);
	LOG("Getdesc " << GetDxErrorStr(hr));

	hr=Direct3dDev->CreateOffscreenPlainSurface(Desc.Width,Desc.Height,Desc.Format,D3DPOOL_SYSTEMMEM,&Surface,NULL);
	LOG("CreateOffSurf " << GetDxErrorStr(hr));

	hr=Direct3dDev->GetRenderTargetData(TargetSurface,Surface);
	LOG("GetTargetData " << GetDxErrorStr(hr));

	hr=Surface->LockRect(&Lock,NULL,D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK);
	LOG("Locking "<<GetDxErrorStr(hr));
	LOG("Pitch "<<Lock.Pitch << " Ptr "<<(unsigned long)Lock.pBits << "\r\n");

	if (Lock.pBits==NULL)
	{
		MessageBox(NULL,"Gfx : Invalid Surface pBits==NULL","Dx Error",0);
		return;
	}

	char *DataPtr;
	for (int j=this->ScreenHeight()-1;j>-1;j--)
	{
		DataPtr=(char*)Lock.pBits;
		DataPtr+=j*Lock.Pitch;
		Fst.Write(DataPtr,this->ScreenWidth()*4);
	}
	/*for (int j=0;j<this->ScreenHeight();j++)
	{
		DataPtr=(char*)Lock.pBits;
		DataPtr+=j*Lock.Pitch;
		Fst.Write(DataPtr,this->ScreenWidth()*4);
	}*/

	Surface->UnlockRect();

	Surface->Release();
	TargetSurface->Release();

	//TODO once the surface is copied we should use async write or a temp thread to write the pict
	//because it's a lengthy operation 

	SYSTEMTIME CurTime;
	GetLocalTime(&CurTime);
	char FileName[128];
	sprintf_s(FileName,128, ".\\screens\\Scr%04u_%02u_%02u@%02uh%02um%02us.tga", CurTime.wYear, CurTime.wMonth, CurTime.wDay, 
		CurTime.wHour,CurTime.wMinute, CurTime.wSecond);
	Fst.SaveToFile(FileName);
};
/*
void TGfxEngine::TakeScreenShoot(void)
{
    HRESULT hr;
    LPDIRECT3DSURFACE9 renderTarget;
	hr = Direct3dDev->GetRenderTarget( 0, &renderTarget );
    if( !renderTarget || FAILED(hr) )
        return ;

    D3DSURFACE_DESC rtDesc;
    renderTarget->GetDesc( &rtDesc );

    LPDIRECT3DSURFACE9 resolvedSurface;
    if( rtDesc.MultiSampleType != D3DMULTISAMPLE_NONE )
    {
        hr = Direct3dDev->CreateRenderTarget( rtDesc.Width, rtDesc.Height, rtDesc.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &resolvedSurface, NULL );
        if( FAILED(hr) )
            return ;
        hr = Direct3dDev->StretchRect( renderTarget, NULL, resolvedSurface, NULL, D3DTEXF_NONE );
        if( FAILED(hr) )
            return ;
        renderTarget = resolvedSurface;
    }

    LPDIRECT3DSURFACE9 offscreenSurface;
    hr = Direct3dDev->CreateOffscreenPlainSurface( rtDesc.Width, rtDesc.Height, rtDesc.Format, D3DPOOL_SYSTEMMEM, &offscreenSurface, NULL );
    if( FAILED(hr) )
        return ;

    hr = Direct3dDev->GetRenderTargetData( renderTarget, offscreenSurface );
    bool ok = SUCCEEDED(hr);
    if( ok )
    {
        // Here we have data in offscreenSurface.
        D3DLOCKED_RECT lr;
        RECT rect;
        rect.left = 0;
        rect.right = rtDesc.Width;
        rect.top = 0;
        rect.bottom = rtDesc.Height;
        // Lock the surface to read pixels
        hr = offscreenSurface->LockRect( &lr, &rect, D3DLOCK_READONLY );
        if( SUCCEEDED(hr) )
        {
			TFastStream Fst;
			Fst.Write();
            // Pointer to data is lt.pBits, each row is
            // lr.Pitch bytes apart (often it is the same as width*bpp, but
            // can be larger if driver uses padding)

            // Read the data here!
            offscreenSurface->UnlockRect();
        }
    }
    return;
}*/

LPDIRECT3DTEXTURE9 CreateTextureFromZmb(const char* Filename)
{
	LPDIRECT3DTEXTURE9 NewTexture=NULL;
	unsigned long Width, Height,CompSize;
	char *Buffer;
	D3DLOCKED_RECT Lock;
	HANDLE Fic;
	unsigned long Dummy;
	int OutSize;

	Fic=CreateFile(Filename,GENERIC_READ,0,NULL,OPEN_EXISTING,NULL,NULL);

	ReadFile(Fic,&Width,4,&Dummy,0);
	ReadFile(Fic,&Height,4,&Dummy,0);
	ReadFile(Fic,&CompSize,4,&Dummy,0);
	Buffer=(char*)malloc(CompSize);
	ReadFile(Fic,Buffer,CompSize,&Dummy,0);
	
	CloseHandle(Fic);

	NewTexture=GfxCore.CreateTexture(Width,Height,D3DFMT_A8R8G8B8);
	if (!NewTexture)
		goto Fail;

	//let's decompressed data Directly to the texture
	NewTexture->LockRect(0,&Lock,NULL,D3DLOCK_DISCARD);

	z_stream_s ZStream;
	OutSize=(Width*Height)<<2;
	memset(&ZStream,0,sizeof(ZStream));
	inflateInit_(&ZStream,ZLIB_VERSION,sizeof(ZStream));

	ZStream.next_in= (unsigned char*)Buffer;
    ZStream.avail_in= CompSize;
    ZStream.next_out= (unsigned char*)Lock.pBits;
    ZStream.avail_out= OutSize;
	inflate(&ZStream,Z_FINISH); //inflate all data at once
	inflateEnd(&ZStream);

	NewTexture->UnlockRect(0);
	

	//freeing memory
Fail:
	return NewTexture;
}


const char * GetDxErrorStr(unsigned long Error)
{
	switch (Error) 
	{
		case D3D_OK: 
		{
			return "No Error\r\n";
			break;
		}
		case D3DERR_WRONGTEXTUREFORMAT: 
		{
			return "Wrong Texture Format\r\n";
			break;
		}
		case D3DERR_UNSUPPORTEDCOLOROPERATION: 
		{
			return "Bad Color Op\r\n";
			break;
		}
		case D3DERR_UNSUPPORTEDCOLORARG: 
		{
			return "Bad Color Arg\r\n";
			break;
		}
		case D3DERR_UNSUPPORTEDALPHAOPERATION: 
		{
			return "Bad Alpha op\r\n";
			break;
		}
		case D3DERR_UNSUPPORTEDALPHAARG: 
		{
			return "Bad Alpha Arg\r\n";
			break;
		}
		case D3DERR_TOOMANYOPERATIONS: 
		{
			return "Too Many Operations\r\n";
			break;
		}
		case D3DERR_CONFLICTINGTEXTUREFILTER: 
		{
			return "Conflicting Texture Filter\r\n";
			break;
		}
		case D3DERR_UNSUPPORTEDFACTORVALUE: 
		{
			return "Bad Factor Val\r\n";
			break;
		}
		case D3DERR_CONFLICTINGRENDERSTATE: 
		{
			return "Render state conflict\r\n";
			break;
		}
		case D3DERR_UNSUPPORTEDTEXTUREFILTER: 
		{
			return "Bad Texture Filter\r\n";
			break;
		}
		case D3DERR_CONFLICTINGTEXTUREPALETTE: 
		{
			return "Texture Palette Conflict\r\n";
			break;
		}
		case D3DERR_DRIVERINTERNALERROR: 
		{
			return "Driver Internal Error\r\n";
			break;
		}
		case D3DERR_NOTFOUND: 
		{
			return "Not Found\r\n";
			break;
		}
		case D3DERR_MOREDATA: 
		{
			return "More Data\r\n";
			break;
		}
		case D3DERR_DEVICELOST: 
		{
			return "Device Lost\r\n";
			break;
		}
		case D3DERR_DEVICENOTRESET: 
		{
			return "Device not Reset\r\n";
			break;
		}
		case D3DERR_NOTAVAILABLE: 
		{
			return "Not Available\r\n";
			break;
		}
		case D3DERR_OUTOFVIDEOMEMORY: 
		{
			return "Out of Video Memory\r\n";
			break;
		}
		case D3DERR_INVALIDDEVICE: 
		{
			return "Invalid Device\r\n";
			break;
		}
		case D3DERR_INVALIDCALL: 
		{
			return "Invalid Call\r\n";
			break;
		}
		case D3DERR_DRIVERINVALIDCALL: 
		{
			return "Driver Invalid Call\r\n";
			break;
		}
		case D3DERR_WASSTILLDRAWING: 
		{
			return "Was still Drawing\r\n";
			break;
		}
		case D3DOK_NOAUTOGEN: 
		{
			return "No Auto Gen\r\n";
			break;
		}
		default :
		{
			return "Uknown\r\n";
		}
	};
}; 
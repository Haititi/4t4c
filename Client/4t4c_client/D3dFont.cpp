#include "D3dFont.h"
#include "FastStream.h"
#include "FormatText.h"


void FillTexture(D3DLOCKED_RECT &Lck,const int Height)
{
	memset(Lck.pBits,0,Height*Lck.Pitch*sizeof(unsigned long));
};

struct ScanLineUserInfo
{
	unsigned long* SurfaceMemory;
	int Pitch;
	int Width;
	int Height;
	unsigned long Color; //the color we want for the scanline
	int BaseX; //Base Position for rendering;
	int BaseY;

};

inline unsigned long ColorShade(unsigned long Color,unsigned char Shade)
{
	/*unsigned long A=(Color>>24)& 0xFF;
	unsigned long R=(Color>>16)& 0xFF;
	unsigned long G=(Color>>8)& 0xFF;
	unsigned long B=(Color)& 0xFF;

	R=(R*Shade)/255;
	G=(G*Shade)/255;
	B=(B*Shade)/255;

	return (unsigned long) ((A<<24) | (R<<16) | (G<<8) | (B));*/
	return (Color & 0x00FFFFFF) | (Shade <<24);
};

void ScanLineRenderer( int yScan, int Count, const FT_Span*  Spans, void* UserInfo )
{
	ScanLineUserInfo* Info=(ScanLineUserInfo*)UserInfo;

	int BaseY=Info->BaseY-yScan;
	if (BaseY<0 || BaseY>=Info->Height)
		return;

	//render "Count" Spans
	for (int i=0;i<Count;i++)
	{
		if (Spans[i].coverage==0)
			continue;
		unsigned long* BasePtr=Info->SurfaceMemory+BaseY*(Info->Pitch);
		unsigned long Color=ColorShade(Info->Color,Spans[i].coverage);
		int MaxX=Spans[i].x+Info->BaseX+Spans[i].len;
		if (MaxX>Info->Width)
			MaxX=Info->Width;
		for (int Pos=Spans[i].x+Info->BaseX;Pos<MaxX;Pos++)
		{
			BasePtr[Pos]=Color;
		}
	}
};

void ComputeStringBox( FT_BBox& Box, FT_Glyph* GlyphList,FT_Vector* PosList, const int GlyphCount )
{
	/* initialize string bbox to "empty" values */
	Box.xMin = Box.yMin =  32000;
	Box.xMax = Box.yMax = -32000;

	/* for each glyph image, compute its bounding box, */
	/* translate it, and grow the string bbox          */
	for ( int n = 0; n < GlyphCount; n++ )
	{
		FT_BBox  GlyphBox;

		FT_Glyph_Get_CBox( GlyphList[n], ft_glyph_bbox_pixels, &GlyphBox );

		GlyphBox.xMin += PosList[n].x;
		GlyphBox.xMax += PosList[n].x;
		GlyphBox.yMin += PosList[n].y;
		GlyphBox.yMax += PosList[n].y;

		if ( GlyphBox.xMin < Box.xMin )
			Box.xMin = GlyphBox.xMin;

		if ( GlyphBox.yMin < Box.yMin )
			Box.yMin = GlyphBox.yMin;

		if ( GlyphBox.xMax > Box.xMax )
			Box.xMax = GlyphBox.xMax;

		if ( GlyphBox.yMax > Box.yMax )
			Box.yMax = GlyphBox.yMax;
	}

	/* check that we really grew the string bbox */
	if ( Box.xMin > Box.xMax )
	{
		Box.xMin = 0;
		Box.yMin = 0;
		Box.xMax = 0;
		Box.yMax = 0;
	}
};

FreeTypeMgr& FreeTypeMgr::Instance(void)
{
	static FreeTypeMgr Inst;
	return Inst;
};

FreeTypeMgr::FreeTypeMgr(void)
{
	//retrieve the font directory to be able to find fonts
	wchar_t TmpBuf[MAX_PATH];
	SHGetFolderPathW(0, CSIDL_FONTS, 0, SHGFP_TYPE_CURRENT,  TmpBuf);
	FontDir=TmpBuf;
	FontDir+=L"\\";

	//init freetype lib
	Library=0;
	if (FT_Init_FreeType( &Library )) 
		throw std::runtime_error("FT_Init_FreeType failed");
};
	
FreeTypeMgr::~FreeTypeMgr(void)
{
	//free the library
	FT_Done_FreeType(Library);
};


FontInstance::FontInstance(FT_Library pLib,FT_Face pFace)
	:Face(pFace),Lib(pLib),GlyphCache(521)//prime number
{
	FaceKerning=FT_HAS_KERNING(Face)!=0;
};

FontInstance* FreeTypeMgr::InstanciateFont(std::wstring FontName, unsigned int FontSize)
{
	ScopedLock Sl(*this);

	std::wstring FontPath(FontDir);
	FontPath+=FontName;

	FT_Face Face=0;

	std::string Path2=Convert(FontPath);

	//Load font info from file
	if (FT_New_Face( Library, Path2.c_str(), 0, &Face )) 
		throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");

	//check the charmap
	//FT_Select_Charmap(Face,FT_ENCODING_UNICODE);

	FontInstance *Inst=new FontInstance(Library,Face);
	Inst->SetFontSize(FontSize);

	return Inst;
};


void FontInstance::SetFontSize(int NewSize)
{
	ScopedLock Sl(Lock);
	//FT_Set_Char_Size( Face, 0, NewSize*64, 0, 0);
	FT_Set_Pixel_Sizes(Face, 0 , NewSize);
	PixelMetrics=Face->units_per_EM;
};

float FontInstance::GetHeight(void)
{
	ScopedLock Sl(Lock);
	FT_Size_Request Size;
	FT_Request_Size(Face,Size);
	return (float)Size->height/64.0f;
};

int FontInstance::GetLen(std::wstring& Text)
{
	ScopedLock Sl(Lock);
	float Length=0.0f;
	for (unsigned int i=0;i<Text.size();i++)
	{
		//retrieve the glyph with the UTF-32 code
		FT_Glyph Glyph=GetGlyph(Text[i]);
		Length+=(float)(Glyph->advance.x>>16);
		FT_Done_Glyph(Glyph);
	}
	return (int)Length;
};

void FontInstance::DrawFont( QuadTexture* Surface, const int Px,const int Py, const char* Text,	const unsigned long Color, bool Gen)
{
	Utf32String Str=ConvertAsciiToUtf32(Text);
	DrawFinal(Surface,Px,Py,Str,Color,Gen);
};

void FontInstance::DrawFont( QuadTexture* Surface, const int Px,const int Py, std::string& Text, const unsigned long Color, bool Gen)
{
	//SurfAutolock Sal(Surface);
	Utf32String Str=ConvertAsciiToUtf32(Text.c_str());
	DrawFinal(Surface,Px,Py,Str,Color,Gen);
};

void FontInstance::DrawFont( QuadTexture* Surface, const int Px,const int Py, std::wstring& Text, const unsigned long Color, bool Gen)
{
	Utf32String Str=ConvertUtf16ToUtf32(Text);
	DrawFinal(Surface,Px,Py,Str,Color,Gen);
};

void FontInstance::DrawFinal( QuadTexture* Surface,const int Px, const int Py,Utf32String& Str,const unsigned long Color,bool Gen)
{
	ScopedLock Sl(Lock);
	FT_Error Error;
	ScanLineUserInfo Info;
	FT_Glyph	AGlyphs[512];   /* glyph image    */
	FT_Vector   APos   [512];   /* glyph position */

	//get all glyph and compute rendering pos
	int PosX=0;
	int PosY=0; //Py+(Face->height>>6)/3;
	unsigned int GlyphArrIdx=0;
	for (unsigned int i=0;i<Str.size();i++)
	{
		//retrieve the glyph with the UTF-32 code
		AGlyphs[GlyphArrIdx]=GetGlyph(Str[i]);

		if (AGlyphs[GlyphArrIdx]!=0)
		{
			/* store current pen position */
			APos[GlyphArrIdx].x = PosX;
			APos[GlyphArrIdx].y = PosY;

			//advance is in 16.16 fixed format
			PosX+=AGlyphs[GlyphArrIdx]->advance.x>>16;
			PosY+=AGlyphs[GlyphArrIdx]->advance.y>>16;

			GlyphArrIdx++;
		}
	}

	//compute the bounding box to generate the texture and/or clip
	FT_BBox Box;
	ComputeStringBox(Box, AGlyphs, APos, GlyphArrIdx);
	//Box.yMax+=2;
	//generate the texture
	
	if (Gen)
		Surface->CreateTexture( Box.xMax-Box.xMin+1, Box.yMax-Box.yMin+1 );

	ScopedTextureLock Stl(Surface);
	FillTexture( Stl.Info, Surface->GetHeight());

	//Setup info structure
	Info.Height=Surface->GetHeight();
	Info.Width=Surface->GetWidth();
	Info.Pitch=Stl.Info.Pitch;
	Info.SurfaceMemory=(unsigned long*)Stl.Info.pBits;
	Info.BaseX=0;
	Info.BaseY=0;
	Info.Color=Color;

	//setup rasterizer parameters
	//TODO : use freetype internal clipping, should be faster
	FT_BBox Clipper;
	Clipper.xMin=Clipper.yMin=0;
	Clipper.xMax=Surface->GetWidth();
	Clipper.yMax=Surface->GetHeight();
	FT_Raster_Params  Params;
	Params.flags      = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT ;//| FT_RASTER_FLAG_CLIP;
	Params.gray_spans = (FT_Raster_Span_Func)ScanLineRenderer;
	Params.user       = &Info;
	//Params.clip_box	  = Clipper;

	int BaseLine=Surface->GetHeight()+Box.yMin-1;

	//render it
	for (unsigned int i=0;i<GlyphArrIdx;i++)
	{
		if ( AGlyphs[i]->format==FT_GLYPH_FORMAT_OUTLINE)
		{
			FT_OutlineGlyphRec* Glyph2=(FT_OutlineGlyphRec*)AGlyphs[i];

			Info.BaseX=APos[i].x;
			Info.BaseY=APos[i].y+BaseLine;
		
			Error = FT_Outline_Render( Lib, &Glyph2->outline, &Params );
		}
	}
};

FT_Glyph FontInstance::GetGlyph(const unsigned long CharCode)
{
	FT_Error Error;
	const FT_UInt GlyphIndex=FT_Get_Char_Index( Face, CharCode );
	Error=FT_Load_Glyph( Face, GlyphIndex, FT_LOAD_DEFAULT );
	FT_Glyph Glyph;
	Error=FT_Get_Glyph( Face->glyph, &Glyph );
	return Glyph;
};

TD3dText::TD3dText(FontInstance* Reference)
{
	FontRef=Reference;
	LineSpacing=0;
	TPx=TPy=0;
	MaxWidth=2000;
	MaxHeight=1000;
	LineOffset=0;
	BackGround=true;
	TruncatedLine=0;
	FormatedLines=0;
	Fx=0;
	Justify=TextJustify_Left;
	Color=COLOR32_WHITE;
	FinalWidth=0;
	FinalHeight=0;
};

TD3dText::~TD3dText(void)
{
};
	
void TD3dText::SetText(std::wstring Text)
{
	BaseText=Text;
	LineOffset=0;
	ClearTextBuffer();
	Reformat();
};

bool TD3dText::IsEmpty(void)
{
	return BaseText.empty();
};

int TD3dText::GetWidth(void)
{
	return FinalWidth;
};

int TD3dText::GetHeight(void)
{
	return FinalHeight;
};

void TD3dText::ChangeFont(FontInstance* NewFont)
{
	if (NewFont!=0)
	{
		FontRef=NewFont;
		Reformat();
	}
};

int TD3dText::GetLineCount(void)
{
	return FormatedLines;
};

void TD3dText::ChangeLineOffset(const int NewLineOffset)
{
	if (LineOffset!=NewLineOffset)
	{
		LineOffset=NewLineOffset;
		Reformat();
	};
};

void TD3dText::SetPosition(const int NewPx,const int NewPy)
{
	if ((NewPx!=TPx) || (NewPy!=TPy))
	{
		TPx=NewPx;
		TPy=NewPy;
	} //no reformat !!!!!
};

void TD3dText::SetColor(const unsigned long NewColor)
{
	if (NewColor!=Color)
	{
		Color=NewColor;
		//TODO it's possible to change the color without reformat
		//just need to keep the old color for comparison
		Reformat();
	}
};

void TD3dText::SetBackGround(const bool NewState)
{
	if (NewState!=BackGround)
	{
		//TODO we could probably just modify the element vector instead of reformatting
		BackGround=NewState;
		Reformat();
	}
};
	
std::wstring TD3dText::GetText(void)
{
	return BaseText;
};

int TD3dText::GetTruncatedLine(void)
{
	return TruncatedLine;
};
	
void TD3dText::SetMaxSize(int NewMaxWidth,int NewMaxHeight)
{
	if ((NewMaxWidth!=MaxWidth) || (NewMaxHeight!=MaxHeight))
	{
		MaxWidth=NewMaxWidth;
		MaxHeight=NewMaxHeight;
		Reformat();
	}
};

void TD3dText::SetFx(const unsigned long NewFx)
{
	if (NewFx!=Fx)
	{
		Fx=NewFx;
		Reformat();
	};
};

void TD3dText::SetJustify(const unsigned long NewJustify)
{
	if (NewJustify!=Justify)
	{
		Justify=NewJustify;
		Reformat();
	};
};


void TD3dText::ClearTextBuffer(void)
{
	TextBuffer.clear();
};

//add text to the buffer
void TD3dText::AddCharToBuffer(const int Posx,const int Posy,const char Chr,const unsigned long Color,const int Italic)
{
	//slow...
	//TODO use 2 pointer list, transfer unused pointer to the second if unused to remove need for alloc?
	TTextElem TextElem;

	TextElem.ElemType=ETextElemType::Character;
	TextElem.Posx=Posx;
	TextElem.Posy=Posy;
	TextElem.Prm1=Italique;
	TextElem.Chr=Chr;
	TextElem.color=Color;

	TextBuffer.push_back(TextElem);
};

void TD3dText::AddQuadToBuffer(const int Posx,const int Posy,const int SizeX,const int SizeY,const unsigned long Color)
{
	TTextElem TextElem;

	TextElem.ElemType=ETextElemType::Quad;
	TextElem.Posx=Posx;
	TextElem.Posy=Posy;
	TextElem.Prm1=SizeX;
	TextElem.Prm2=SizeY;
	TextElem.color=Color;

	TextBuffer.push_back(TextElem);
};

//tokenize the line and change color/formatting accordingly
void TD3dText::LineRender(const int Posx,const int PosY,std::wstring& Text,const unsigned long Color)
{
	unsigned int CharCount=Text.size();
	if (!CharCount)
		return;

	int ActPosX=0;
	for(unsigned int i=0;i<CharCount;i++)
	{
		switch (Text[i])
		{
		case '{': //color
			{
				if ((i+1)<CharCount)
				{
					i++;
					//TODO get color
					unsigned long Col=((unsigned long)(Text[i]-'0'))%10; //mod 10 is a security without branching here
					ActualColor=FormatColorTable[Col];
				}
				break;
			}
		case '}'://color end
			{
				ActualColor=Color; //Scope ambiguity but that one is right
				break;
			}
		case '¤': //bold
			{
				Bold^=true;
				break;
			}
		case '%': //italique
			{
				Italique^=true;
				break;
			}
		case '$'://barré
			{
				StrikeOut^=true;
				break;
			}
		case 'µ'://souligné
			{
				SubLined^=true;
				break;
			}
		default:
			{
				std::wstring Tmp(1,Text[i]);
				int CharWidth=FontRef->GetLen(Tmp);
				//first add the caracter
				AddCharToBuffer(ActPosX,PosY,Text[i],ActualColor,(int)Italique);
				//second add the decorating items
				if (Bold) 
				{
					AddCharToBuffer(ActPosX+1,PosY,Text[i],ActualColor,(int)Italique);
				}
				if (StrikeOut) 
				{
					int ChrHeight=FontRef->GetHeight()/2;
					AddQuadToBuffer(ActPosX,PosY+ChrHeight-1,CharWidth,2,ActualColor);
				}
				if (SubLined) 
				{
					int ChrHeight=FontRef->GetHeight();
					AddQuadToBuffer(ActPosX,PosY+ChrHeight-2,CharWidth,2,ActualColor);
				}
				ActPosX+=CharWidth;
			}
		}
	}
};

void TD3dText::LineRenderRaw(const int Posx,const int PosY,std::wstring& Text,const unsigned long Color)
{
	unsigned int CharCount=Text.size();
	if (!CharCount)
		return;

	int ActPosX=0;
	for(unsigned int i=0;i<CharCount;i++)
	{
		switch (Text[i])
		{
		case '{': //color
			{
				if ((i+1)<CharCount)
				{
					i++;
				}
				break;
			}
		case '}'://color end
			{
				break;
			}
		case '¤': //bold
			{
				break;
			}
		case '%': //italique
			{
				RawItalique^=true;
				break;
			}
		case '$'://barré
			{
				break;
			}
		case 'µ'://souligné
			{
				break;
			}
		default:
			{
				int CharWidth=FontRef->GetCharWidth(Text[i]);
				//first add the caracter
				AddCharToBuffer(ActPosX,PosY,Text[i],Color,(int)RawItalique);
				//second add the decorating items
				if (Bold) 
				{
					AddCharToBuffer(ActPosX+1,PosY,Text[i],Color,(int)RawItalique);
				}
				ActPosX+=CharWidth;
			}
		}
	}
};


//render the textobject at x,y position with the bufferized info
void TD3dText::Draw(void)
{
	if (BaseText.empty())
		return;
	
	GfxCore.SetFilter(true);

	QuadCount=0;
	for(unsigned int i=0;i<TextBuffer.size();i++)
	{
		FontRef->DrawTextElem(TPx,TPy,TextBuffer.at(i));
	};

	FontRef->FlushTextElems();
	GfxCore.SetFilter(false);
};

void TD3dText::DrawAt(const int Posx,const int Posy)
{
	//TODO make it the easy way first
	SetPosition(Posx,Posy);
	Draw();
};


//the core of the thingy
void TD3dText::Reformat(void)
{
	if (BaseText.empty())
		return;

	ClearTextBuffer();
	//init formatting variables
	RawItalique=Italique=SubLined=StrikeOut=Bold=Colored=false;

	ActualColor=Color;

	FormatText Ft;
	Ft.AddText(BaseText);
	Ft.Format(MaxWidth,FontRef);

	int NbLines=Ft.GetLineCount(); //Number of lines after formatting
	FormatedLines=NbLines;
	if (LineOffset>=NbLines)
		return; //Nothing to format

	int StartLine,EndLine; //the lines we really draw
	int LineHeight=FontRef->GetHeight()+LineSpacing;

	StartLine=LineOffset;
	EndLine=StartLine;

	int Height=0;
	while ((Height<MaxHeight) && (EndLine<NbLines))
	{
		Height+=LineHeight; //we compute the total Height of what is left
		EndLine++;
	}
	if (Height>MaxHeight)
	{
		Height-=LineHeight;
		EndLine--;
	}
	FinalHeight=Height;
	TruncatedLine=EndLine+1;

	//TODO cycle trough each line, get it's width and keep the highest value
	//And cache each string width, is "expensive" to compute
	int* LineWidths=new int[NbLines];
	FinalWidth=0;

	for(int i=StartLine;i<EndLine;i++)
	{
		LineWidths[i]=FontRef->GetLen(Ft.GetLine(i));
		if (LineWidths[i]>FinalWidth)
			FinalWidth=LineWidths[i];
	}

	int Posx=0,Posy=0;

	if (Fx & TextFX_Outline)
	{
		if (BackGround)
			AddQuadToBuffer(Posx,Posy,FinalWidth,LineHeight*(EndLine-StartLine)+4,COLOR32_BLACK);

		for(int i=StartLine;i<EndLine;i++)
		{
			if (Justify==TextJustify_Center) 
			{
				Posx=TPx-LineWidths[i]/2;
			}
			
			//TODO we should have a "LineRenderRaw" function witouth the parsing overhead 
			LineRenderRaw(Posx,Posy,Ft.GetLine(i),COLOR32_BLACK);
			LineRenderRaw(Posx+2,Posy+2,Ft.GetLine(i),COLOR32_BLACK);
			LineRender(Posx+1,Posy+1,Ft.GetLine(i),Color);
			Posy+=LineHeight;
		}
	} else
	{
		if (BackGround)
				AddQuadToBuffer(Posx,Posy,FinalWidth,LineHeight*(EndLine-StartLine)+4,COLOR32_BLACK);

		for(int i=StartLine;i<EndLine;i++)
		{
			if (Justify==TextJustify_Center) 
			{
				Posx=TPx-LineWidths[i]/2;
			}
			LineRender(Posx,Posy,Ft.GetLine(i),Color);
			Posy+=LineHeight;
		}
	}
	delete [] LineWidths;
};
/*
TD3dFont::TD3dFont(PFontInfo FtInfo)
{
	//some stupid default value to avoid 0 default
	BaseWidth=FtInfo->BaseHeight;
	BaseHeight=FtInfo->BaseHeight;
	FactorSizeX=FactorSizeY=1.0f;
	if (FtInfo)
		FontInfo=FtInfo;
	else
		throw;
};

TD3dFont::~TD3dFont(void)
{

};

TD3dText* TD3dFont::CreateTextObject(void)
{
	TD3dText* NewTextObj=new TD3dText(this);
	return NewTextObj;
};

void TD3dFont::SetSizeAbsolute(const int NewSize)
{
	float Ratio=NewSize/((float)BaseHeight);
	SetSize(Ratio);
};

void TD3dFont::SetSize(const float NewSize)
{
	if (NewSize>0)
		FactorSizeX=FactorSizeY=NewSize;
};

void TD3dFont::SetSizeRatio(const float NewSizeX,const float NewSizeY)
{
	if (NewSizeX>0)
		FactorSizeX=NewSizeX;
	if (NewSizeY>0)
		FactorSizeY=NewSizeY;
};

int TD3dFont::GetStringLengthApproximate(const char* Text)
{
	return (int)((FontInfo->FontCoord['A'].Width*strlen(Text))*FactorSizeX);
};

int TD3dFont::GetStringLength(const char* Text)
{
	float Result=0;
	for (unsigned int i=0;i<strlen(Text);i++)
	{
		Result+=floor(((float)FontInfo->FontCoord[(unsigned char)Text[i]].Width)*FactorSizeX);
	}
	return (int)Result;
};

int TD3dFont::GetCharWidth(const char Chr)
{
	return (int)(floor(((float)FontInfo->FontCoord[(unsigned char)Chr].Width)*FactorSizeX));
};

//HACK
int TD3dFont::TextOff(const char* Text,int BoxSize)
{
	int Size = GetStringLength(Text);
	return((BoxSize-Size)/2);
};

int TD3dFont::GetHeight(void)
{
	return (int)(BaseHeight*FactorSizeY);
};

void TD3dFont::Draw(int PosX,int PosY,const char* Text,const unsigned long Color,const unsigned long Background)
{
	PSpritePlane Vertices;
	int i;
	float Xpos;

	Xpos=((float)PosX)-0.5f;

	//Direct3dDev->SetTexture(0,FontInfo->Texture);
	unsigned char Tmp; 
	int Size=strlen(Text);
	for(i=0;i<Size;i++)
	{
		Tmp=Text[i];

		for (int k=0;k<4;k++)
		{
			TextQuadBuf[i][k].z=0.0f;
			TextQuadBuf[i][k].h=1.0f;
			TextQuadBuf[i][k].col=Color;
		}

		TextQuadBuf[i][0].y=(float)PosY-0.5f;
		TextQuadBuf[i][1].y=TextQuadBuf[i][0].y;
		TextQuadBuf[i][2].y=((float)PosY)+floor(FontInfo->FontCoord[0].Tv2*((float)FontInfo->Sizey)*FactorSizeY)-0.5f;
		TextQuadBuf[i][3].y=TextQuadBuf[i][2].y;

		TextQuadBuf[i][0].tu=TextQuadBuf[i][2].tu=FontInfo->FontCoord[Tmp].Tu1;
		TextQuadBuf[i][1].tu=TextQuadBuf[i][3].tu=FontInfo->FontCoord[Tmp].Tu2;

		TextQuadBuf[i][0].tv=TextQuadBuf[i][1].tv=FontInfo->FontCoord[Tmp].Tv1;
		TextQuadBuf[i][2].tv=TextQuadBuf[i][3].tv=FontInfo->FontCoord[Tmp].Tv2;         

		TextQuadBuf[i][0].x=Xpos;
		TextQuadBuf[i][1].x=Xpos+floor(((float)FontInfo->FontCoord[Tmp].Width)*FactorSizeX);
		TextQuadBuf[i][2].x=TextQuadBuf[i][0].x;
		TextQuadBuf[i][3].x=TextQuadBuf[i][1].x;

		Xpos=TextQuadBuf[i][1].x;

		//GfxCore.DrawSprite(&Vertices,FontInfo->Texture);
	}
	GfxCore.SetFilter(true);
	GfxCore.DrawBatch(TextQuadBuf,Size,FontInfo->Texture);
	GfxCore.SetFilter(false);
};

void TD3dFont::Draw(int PosX,int PosY,const char* Text,const unsigned long Color)
{
	if (Text)
		Draw(PosX,PosY,Text,Color,false);
};

void TD3dFont::DrawTextElem(const int PosX,const int PosY,const TTextElem&  TextElem)
{
	TSpritePlane Vertices;
	int i;
	float Xpos;


	for (i=0;i<4;i++)
	{
		TextQuadBuf[QuadCount][i].z=0.0f;
		TextQuadBuf[QuadCount][i].h=1.0f;
		TextQuadBuf[QuadCount][i].col=TextElem.color;
	}

	switch (TextElem.ElemType)
	{
		case ETextElemType::Character:
		{
			TextQuadBuf[QuadCount][0].y=((float)(PosY+TextElem.Posy))-0.5f;
			TextQuadBuf[QuadCount][1].y=TextQuadBuf[QuadCount][0].y;
			TextQuadBuf[QuadCount][2].y=TextQuadBuf[QuadCount][0].y+floor(FontInfo->FontCoord[0].Tv2*((float)FontInfo->Sizey)*FactorSizeY);
			TextQuadBuf[QuadCount][3].y=TextQuadBuf[QuadCount][2].y;

			Xpos=(float)(PosX+TextElem.Posx);
			Xpos-=0.5f;

			//Direct3dDev->SetTexture(0,FontInfo->Texture);
			unsigned char Tmp; 
			Tmp=TextElem.Chr;
			TextQuadBuf[QuadCount][0].tu=FontInfo->FontCoord[Tmp].Tu1;
			TextQuadBuf[QuadCount][1].tu=FontInfo->FontCoord[Tmp].Tu2;
			TextQuadBuf[QuadCount][2].tu=FontInfo->FontCoord[Tmp].Tu1;
			TextQuadBuf[QuadCount][3].tu=FontInfo->FontCoord[Tmp].Tu2;

			TextQuadBuf[QuadCount][0].tv=TextQuadBuf[QuadCount][1].tv=FontInfo->FontCoord[Tmp].Tv1;
			TextQuadBuf[QuadCount][2].tv=TextQuadBuf[QuadCount][3].tv=FontInfo->FontCoord[Tmp].Tv2;         

			TextQuadBuf[QuadCount][2].x=Xpos;
			TextQuadBuf[QuadCount][3].x=Xpos+floor(((float)FontInfo->FontCoord[Tmp].Width)*FactorSizeX);
			TextQuadBuf[QuadCount][0].x=TextQuadBuf[QuadCount][2].x+ceil((TextElem.Prm1*3)*FactorSizeX); //italic
			TextQuadBuf[QuadCount][1].x=TextQuadBuf[QuadCount][3].x+ceil((TextElem.Prm1*3)*FactorSizeX);
			

			//Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,Vertices,BaseVertexSize);
			//GfxCore.DrawSprite(&Vertices,FontInfo->Texture);
			break;
		}
		case ETextElemType::Quad:
		{
			Xpos=(float)(PosX+TextElem.Posx);
			Xpos-=0.5f;

			TextQuadBuf[QuadCount][0].y=((float)(PosY+TextElem.Posy))-0.5f;
			TextQuadBuf[QuadCount][1].y=Vertices[0].y;
			TextQuadBuf[QuadCount][2].y=Vertices[0].y+TextElem.Prm2;
			TextQuadBuf[QuadCount][3].y=Vertices[2].y;

			TextQuadBuf[QuadCount][0].x=(float)Xpos;
			TextQuadBuf[QuadCount][1].x=(float)Xpos+TextElem.Prm1;
			TextQuadBuf[QuadCount][2].x=Vertices[0].x;
			TextQuadBuf[QuadCount][3].x=Vertices[1].x;

			TextQuadBuf[QuadCount][0].tu=TextQuadBuf[QuadCount][1].tu=
			TextQuadBuf[QuadCount][2].tu=TextQuadBuf[QuadCount][3].tu=0.0f;

			TextQuadBuf[QuadCount][0].tv=TextQuadBuf[QuadCount][1].tv=0.0f;
			TextQuadBuf[QuadCount][2].tv=TextQuadBuf[QuadCount][3].tv=0.0f;         

			//Direct3dDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,Vertices,BaseVertexSize);
			//GfxCore.DrawSprite(&Vertices);//it will use WhiteTexture by default
			break;
		}
	}
	QuadCount++;
};

void TD3dFont::FlushTextElems(void)
{
	GfxCore.DrawBatch(TextQuadBuf,QuadCount,FontInfo->Texture);
};*/
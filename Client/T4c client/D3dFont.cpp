#include "D3dFont.h"
#include "FastStream.h"
#include "FormatText.h"
//global
TFontManager FontManager;


TSpritePlane TextQuadBuf[2048]; //buffer for batch rendering
int QuadCount=0;

TFontManager::TFontManager(void)
{
	Initialized=false;
	FontCount=0;
	FontInfoArray=0;
};


TFontManager::~TFontManager(void)
{
	if (FontCount)
	{
		//font Cleaning
		for (int i=0;i<FontCount;i++)
		{
			//Texture Release
			if (FontInfoArray[i].Texture)
				FontInfoArray[i].Texture->Release();
		}
		delete [] FontInfoArray;
	}
};

bool TFontManager::LoadFontInfo(const char* FileName)
{
	D3DLOCKED_RECT Lock;
	TFastStream Fst;
	int i,k;

	if (!Fst.LoadFromFile(FileName))
	{
		return false;
	}

	FontCount=Fst.ReadLong();
	FontInfoArray=new TFontInfo[FontCount];

	//reading the font package
	for (k=0;k<FontCount;k++)
	{
		FontInfoArray[k].FontName=Fst.ReadLongString();
		FontInfoArray[k].Sizex=Fst.ReadLong();
		FontInfoArray[k].Sizey=Fst.ReadLong();
		FontInfoArray[k].BaseHeight=Fst.ReadLong();
		for (i=0;i<256;i++)
		{
			FontInfoArray[k].FontCoord[i].Width=Fst.ReadLong();
			FontInfoArray[k].FontCoord[i].Tu1=Fst.ReadFloat();
			FontInfoArray[k].FontCoord[i].Tu2=Fst.ReadFloat();
			FontInfoArray[k].FontCoord[i].Tv1=Fst.ReadFloat();
			FontInfoArray[k].FontCoord[i].Tv2=Fst.ReadFloat();
		}
		//creating the font texture
		FontInfoArray[k].Texture=GfxCore.CreateTexture(FontInfoArray[k].Sizex,FontInfoArray[k].Sizey,D3DFMT_A8R8G8B8);
		if (FontInfoArray[k].Texture)
		{
			FontInfoArray[k].Texture->LockRect(0,&Lock,NULL,D3DLOCK_DISCARD);
			Fst.Read(Lock.pBits,FontInfoArray[k].Sizex*FontInfoArray[k].Sizey*4);
			FontInfoArray[k].Texture->UnlockRect(0);
		} else
		{
			//cannot create the texture, lets fail silently
			Fst.IncreasePos(FontInfoArray[k].Sizex*FontInfoArray[k].Sizey*4);
			//TODO and ask a Dummy texture to the gfx core
		}
	}

	Initialized=true;
	return true;
};

TD3dFont* TFontManager::CreateFont(const char* FontName)
{
	//Return a font object matching the one asked
	//CM Quick and dirty, should correct that.....
	int i;
	for (i=0;i<FontCount;i++)
	{
		if (strcmp(FontName,FontInfoArray[i].FontName)==0)
		{
			TD3dFont* NewFont=new TD3dFont(&FontInfoArray[i]);
			return NewFont;
		}
	}
	return new TD3dFont(&FontInfoArray[0]);
};

TD3dText::TD3dText(TD3dFont* Reference)
{
	FontRef=Reference;
	LineSpacing=0;
	TPx=TPy=0;
	MaxWidth=2000;
	MaxHeight=1000;
	BaseText=0;
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
	if (BaseText)
		free(BaseText);
		//delete []BaseText;
};
	
void TD3dText::SetText(const char* Text)
{
	if (BaseText)
		free(BaseText);
		//delete []BaseText;
	//unsigned int Len = strlen(Text);
	BaseText=_strdup(Text);
	LineOffset=0;
	ClearTextBuffer();
	Reformat();
};

bool TD3dText::IsEmpty(void)
{
	if (!BaseText)
		return false;
	if (strlen(BaseText)==0)
		return false;
	return true;
};

int TD3dText::GetWidth(void)
{
	return FinalWidth;
};

int TD3dText::GetHeight(void)
{
	return FinalHeight;
};

void TD3dText::ChangeFont(TD3dFont* NewFont)
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
	
const char* TD3dText::GetText(void)
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

	TextElem.ElemType=TextElem_Character;
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

	TextElem.ElemType=TextElem_Quad;
	TextElem.Posx=Posx;
	TextElem.Posy=Posy;
	TextElem.Prm1=SizeX;
	TextElem.Prm2=SizeY;
	TextElem.color=Color;

	TextBuffer.push_back(TextElem);
};

//tokenize the line and change color/formatting accordingly
void TD3dText::LineRender(const int Posx,const int PosY,const char* Text,const unsigned long Color)
{
	unsigned int CharCount=strlen(Text);
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
				int CharWidth=FontRef->GetCharWidth(Text[i]);
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

void TD3dText::LineRenderRaw(const int Posx,const int PosY,const char* Text,const unsigned long Color)
{
	unsigned int CharCount=strlen(Text);
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
	if (!BaseText)
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
	if (!BaseText)
		return;
	if (BaseText[0]==0)
		return;

	ClearTextBuffer();
	//init formatting variables
	RawItalique=Italique=SubLined=StrikeOut=Bold=Colored=false;

	ActualColor=Color;

	FormatText Ft;
	Ft.SetText(BaseText);
	Ft.Format(MaxWidth,FontRef);

	int NbLines=Ft.GetnbLine(); //Number of lines after formatting
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
		LineWidths[i]=FontRef->GetStringLength(Ft.GetText(i));
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
			LineRenderRaw(Posx,Posy,Ft.GetText(i),COLOR32_BLACK);
			LineRenderRaw(Posx+2,Posy+2,Ft.GetText(i),COLOR32_BLACK);
			LineRender(Posx+1,Posy+1,Ft.GetText(i),Color);
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
			LineRender(Posx,Posy,Ft.GetText(i),Color);
			Posy+=LineHeight;
		}
	}
	delete [] LineWidths;
};

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
		case TextElem_Character:
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
		case TextElem_Quad:
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
};
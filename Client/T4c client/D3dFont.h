#ifndef D3DFONT_H
#define D3DFONT_H

#include <windows.h>
#include <vector>
#include "GfxEngine.h"

const unsigned long FormatColorTable[10]=
{
	0xFFF0F0F0, //white
	0xFF969696, //grey
	0xFFF00000, //red
	0xFF00F000, //Green
	0xFF0000F0, //Blue
	0xFFE05000, //orange
	0xFFCEEA15, //Yellow
	0xFF0CE7F5, //cyan
	0xFF7A0CF5, //Purple
	0xFF302E27, //DarkDarkGrey
};

struct TFontCoord
{
	unsigned long Width;
	float Tu1,Tu2,Tv1,Tv2;
};

struct TFontInfo  //todo :we need a more complex structure here, which take kerning into account
{
	char* FontName;
	unsigned long Sizex;
	unsigned long Sizey;
	unsigned long BaseHeight;
	TFontCoord FontCoord[256];
	LPDIRECT3DTEXTURE9 Texture;
};

typedef TFontInfo *PFontInfo;

const unsigned long TextFX_NoFx		=0;
const unsigned long TextFX_Bold		=1;
const unsigned long TextFX_Shadow	=2;
const unsigned long TextFX_Blur		=4;
const unsigned long TextFX_Outline	=8;

const unsigned long TextJustify_Left=0;
const unsigned long TextJustify_Center=1;
const unsigned long TextJustify_Right=2;
const unsigned long TextJustify_Even=0; //take all space from right to left, block like formatting


class TD3dFont;

enum TTextElemType
{
	TextElem_Character=0,
	TextElem_Quad=1
};

struct TTextElem
{
public:
	TTextElemType ElemType;
	int Posx,Posy; //drawing offset
	int Prm1,Prm2; //for Element others than text
	char Chr; //the character to be drawn
	unsigned long color;
};

//prebuffered quad index
class TD3dText
{
private:
	TD3dFont *FontRef;
	unsigned long Fx;
	unsigned long Color;
	unsigned long Justify;
	int MaxWidth,MaxHeight;
	int LineSpacing;
	int LineOffset;
	int FormatedLines;
	int TruncatedLine; //indicate the number of the truncated line, if any
	int TPx,TPy; //text position
	bool BackGround;
	bool FormatTag;

	int FinalWidth; //width of the bufferized stuff
	int FinalHeight; //height of the bufferized stuff
	

	bool RawItalique,Italique,SubLined,StrikeOut,Bold,Colored;
	unsigned long ActualColor;
	std::vector<TTextElem>TextBuffer; 

	char* BaseText;
	//TODO that object should maybe have a critical section to prevent formating at the same time than settext??
	void Reformat(void);//reformat the text according to changes
	void AddCharToBuffer(const int Posx,const int Posy,const char Chr,const unsigned long Color,const int Italic);
	void AddQuadToBuffer(const int Posx,const int Posy,const int SizeX,const int SizeY,const unsigned long Color);
	void ClearTextBuffer(void);
	void LineRenderRaw(const int Posx,const int PosY,const char* Text,const unsigned long Color);
	void LineRender(const int Posx,const int PosY,const char* Text,const unsigned long Color);
public:
	TD3dText(TD3dFont* Reference);
	~TD3dText(void);
	int GetWidth(void);
	int GetHeight(void);
	void ChangeFont(TD3dFont* NewFont);
	void SetFormatTag(const bool NewState);
	void SetPosition(const int NewPx,const int NewPy); 
	void SetText(const char* Text);
	void SetColor(const unsigned long NewColor);
	void SetBackGround(const bool NewState);
	void SetJustify(const unsigned long NewJustify);
	void SetFx(const unsigned long NewFx);
	const char* GetText(void);
	void SetMaxLines(const int NewMaxLines);
	void SetMaxSize(const int NewMaxWidth,const int NewMaxHeight);
	void ChangeLineOffset(const int NewLineOffset); //the line we start with
	int GetTruncatedLine(void);
	int GetLineCount(void);
	bool IsEmpty(void);
	void Draw(void);
	void DrawAt(const int Posx,const int Posy);
};

class TD3dFont
{
private:
	unsigned long BaseWidth,BaseHeight;
	float FactorSizeX,FactorSizeY;
	PFontInfo FontInfo; //pointer on the base structure
public:
	TD3dFont(PFontInfo FtInfo);
	~TD3dFont(void);
	void SetSizeAbsolute(const int NewSize); //standard size in point
	void SetSize(const float NewSize); //multiplier, setup X and Y size with the same ratio
	void SetSizeRatio(const float NewSizeX,const float NewSizeY);
	int GetStringLengthApproximate(const char* Text);//fast and unprecise version
	int GetStringLength(const char* Text);//slower but pixel precision
	int GetCharWidth(const char Chr);
	int GetHeight(void);
	int TextOff(const char* Text,int BoxSize); 
	TD3dText* CreateTextObject(void);
	void DrawTextElem(const int PosX,const int PosY,const TTextElem&  TextElem);
	void FlushTextElems(void);
	void Draw(int PosX,int PosY,const char* Text,const unsigned long Color,const unsigned long Background); //draw text with solid background
	void Draw(int PosX,int PosY,const char* Text,const unsigned long Color);//draw transparent raw text 
};


class TFontManager
{
private:
	bool Initialized;
	int FontCount;
	PFontInfo FontInfoArray;
public:
	TFontManager(void);
	~TFontManager(void);

	bool LoadFontInfo(const char* FileName);
	TD3dFont* CreateFont(const char* FontName);
};

extern TFontManager FontManager;

#endif
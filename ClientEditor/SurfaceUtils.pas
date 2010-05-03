unit SurfaceUtils;

interface
 const
  SurfFormat_A8R8G8B8  =0;
  SurfFormat_R5G6B5    =1;
  SurfFormat_A1R5G5B5  =2;
  SurfFormat_P8        =3;
  SurfFormat_DXT1      =4;
  SurfFormat_DXT5      =5;

 type
  TRgbPal32=Array[0..255] Of packed Record
    a,r,g,b : Byte;
  End;

  TRgbPal=Array [0..255] Of packed Record
    r,g,b : Byte;
  End;

  TPalette = packed Record
    PalName : string;
    Rgb :TRgbPal;
  End;
  PPalette = ^TPalette;

 Function NextPow2(n:integer):integer;

 function MakeGrey(value:byte):cardinal;  inline;
 function MakeColor32(r,g,b:byte):cardinal; inline;

 function ColorFxModulate(Color,Fx:Cardinal):cardinal; inline;
 function ColorFxAdd(Color,Fx:Cardinal):cardinal; inline;
 function ColorFxSub(Color,Fx:Cardinal):cardinal; inline;


 function GetFormatSize(const n:integer):integer;

 Procedure ConvertSurfaceFromP8ToA8R8G8B8(var Data: Pbyte; Size: Integer;Pal:PPalette;Trans,TransCol:Byte);
 Procedure ConvertSurfaceFromA1R5G5B5ToA8R8G8B8(var Data:PWord;var Size:cardinal);
 Procedure ConvertSurfaceFromDXT5ToA8R8G8B8(Width,Height:integer;var Data:PCardinal;var Size:cardinal);

 Procedure ConvertSurfaceFromA8R8G8B8ToA1R5G5B5(var Data:PCardinal;var Size:cardinal);
 Procedure ConvertSurfaceFromA8R8G8B8ToR5G6B5(var Data:PCardinal;var Size:cardinal);
 Procedure ConvertSurfaceFromA8R8G8B8ToDXT1A(Width,Height:integer;var Data:PCardinal;var Size:cardinal);
 Procedure ConvertSurfaceFromA8R8G8B8ToDXT5(Width,Height:integer;var Data:PCardinal;var Size:cardinal);
 Function ConvertSurfaceFromA8R8G8B8ToP8(var Data:PCardinal;var Size:cardinal):PPalette;




implementation

uses math,Squish;

var GreyPal:TPalette;

Function NextPow2(n:integer):integer;
begin
 dec(n);
 n := (n shr 1) or n;
 n := (n shr 2) or n;
 n := (n shr 4) or n;
 n := (n shr 8) or n;
 n := (n shr 16) or n;
 inc(n);
 result:=n;
end;

function MakeGrey(value:byte):cardinal;
begin
 MakeGrey:=$FF000000 or (value shl 16) or (value shl 8) or (value);
end;

function MakeColor32(r,g,b:byte):cardinal;
begin
 MakeColor32:=$FF000000 or (r shl 16) or (g shl 8) or (b);
end;

function ColorFxModulate(Color,Fx:Cardinal):cardinal;
var a,r,g,b,
    fa,fr,fg,fb:single;
    ma,mr,mg,mb:cardinal;
begin
 a:=((Color shr 24) and $ff)/255;
 r:=((Color shr 16) and $ff)/255;
 g:=((Color shr 8) and $ff)/255;
 b:=((Color ) and $ff)/255;

 fa:=((Fx shr 24) and $ff)/255;
 fr:=((Fx shr 16) and $ff)/255;
 fg:=((Fx shr 8) and $ff)/255;
 fb:=((Fx ) and $ff)/255;

 ma:=floor((a*fa)*255);
 mr:=floor((r*fr)*255);
 mg:=floor((g*fg)*255);
 mb:=floor((b*fb)*255);

 result:=(ma shl 24) or (mr shl 16) or (mg shl 8) or (mb);
end;

function ColorFxAdd(Color,Fx:Cardinal):cardinal;
var a,r,g,b,
    fa,fr,fg,fb:single;
    ma,mr,mg,mb:cardinal;
begin
 a:=((Color shr 24) and $ff)/255;
 r:=((Color shr 16) and $ff)/255;
 g:=((Color shr 8) and $ff)/255;
 b:=((Color ) and $ff)/255;

 fa:=((Fx shr 24) and $ff)/255;
 fr:=((Fx shr 16) and $ff)/255;
 fg:=((Fx shr 8) and $ff)/255;
 fb:=((Fx ) and $ff)/255;

 ma:=floor((a+fa)*255);
 mr:=floor((r+fr)*255);
 mg:=floor((g+fg)*255);
 mb:=floor((b+fb)*255);

 if ma>255 then ma:=255;
 if mr>255 then mr:=255;
 if mg>255 then mg:=255;
 if mb>255 then mb:=255;

 result:=(ma shl 24) or (mr shl 16) or (mg shl 8) or (mb);
end;

function ColorFxSub(Color,Fx:Cardinal):cardinal;
var a,r,g,b,
    fa,fr,fg,fb:single;
    ma,mr,mg,mb:integer;
begin
 a:=((Color shr 24) and $ff)/255;
 r:=((Color shr 16) and $ff)/255;
 g:=((Color shr 8) and $ff)/255;
 b:=((Color ) and $ff)/255;

 fa:=((Fx shr 24) and $ff)/255;
 fr:=((Fx shr 16) and $ff)/255;
 fg:=((Fx shr 8) and $ff)/255;
 fb:=((Fx ) and $ff)/255;

 ma:=floor((a-fa)*255);
 mr:=floor((r-fr)*255);
 mg:=floor((g-fg)*255);
 mb:=floor((b-fb)*255);

 if ma<0 then ma:=0;
 if mr<0 then mr:=0;
 if mg<0 then mg:=0;
 if mb<0 then mb:=0;

 result:=(ma shl 24) or (mr shl 16) or (mg shl 8) or (mb);
end;

function PixelA8R8G8B8ToR5G6B5(Argb:cardinal):word;
var a,r,g,b:byte;
begin
 a:=(argb shr 24) and $ff;
 r:=(argb shr 16) and $ff;
 g:=(argb shr 8) and $ff;
 b:=(argb ) and $ff;

 if a=0 then
 begin
  Result:=0;
 end else
 begin
  r:=r shr 3;
  g:=g shr 2;
  b:=b shr 3;

  result:=(r shl 11) or (g shl 5) or (b);
 end;
end;

function PixelA8R8G8B8ToA1R5G5B5(Argb:cardinal):word;
var a,r,g,b:byte;
begin
 a:=(argb shr 24) and $ff;
 r:=(argb shr 16) and $ff;
 g:=(argb shr 8) and $ff;
 b:=(argb ) and $ff;

 if a>0 then a:=1;
 r:=r shr 3;
 g:=g shr 3;
 b:=b shr 3;
 result:=(a shl 15) or (r shl 10) or (g shl 5) or (b);
end;

function PixelA1R5G5B5ToA8R8G8B8(Argb:Word):Cardinal;
var a,r,g,b:byte;
begin
 a:=(argb shr 15) and $1;
 r:=(argb shr 10) and $1f;
 g:=(argb shr 5) and $1f;
 b:=(argb ) and $1f;

 a:=a*$FF;
 r:=r shl 3;
 g:=g shl 3;
 b:=b shl 3;
 result:=(a shl 24) or (r shl 16) or (g shl 8) or (b);
end;

function GetFormatSize(const n:integer):integer;
begin
 Result:=1;
 case n of
   SurfFormat_A8R8G8B8:Result:=4;
   SurfFormat_R5G6B5:Result:=2;
   SurfFormat_A1R5G5B5:Result:=2;
   SurfFormat_P8:Result:=1;
   SurfFormat_DXT1:Result:=1;
   SurfFormat_DXT5:Result:=2;
 end;
end;

procedure ConvertSurfaceFromP8ToA8R8G8B8(var Data: Pbyte; Size: Integer;Pal:PPalette;Trans,TransCol:Byte);
var  i:longint;
     DataPtr:Pbyte;
     Dest,DestPtr:PCardinal;
     Pal2:PPalette;
begin
 GetMem(Dest,Size*4);

 DataPtr:=Data;
 DestPtr:=Dest;

 Pal2:=pal;
 if Pal=nil then
  Pal2:=@GreyPal;


 if trans=0 then
 begin
  for i:=0 to Size-1 do
   begin
    DestPtr^:=MakeColor32(Pal2.Rgb[DataPtr^].r,Pal2.Rgb[DataPtr^].g,Pal2.Rgb[DataPtr^].b);
    inc(DataPtr);
    inc(DestPtr);
   end;
 end else
 begin
   for i:=0 to Size-1 do
   begin
    if (DataPtr^<>Transcol) then
     DestPtr^:=MakeColor32(Pal2.Rgb[DataPtr^].r,Pal2.Rgb[DataPtr^].g,Pal2.Rgb[DataPtr^].b)
                            else
     DestPtr^:=0;
    inc(DataPtr);
    inc(DestPtr);
   end;
 end;

 FreeMem(Data);
 Data:=pbyte(Dest);
end;

Procedure ConvertSurfaceFromA8R8G8B8ToR5G6B5(var Data:PCardinal;var Size:cardinal);
var i:longint;
    DataPtr:PCardinal;
    NewData:Pointer;
    NewDataPtr:PWord;
begin
 DataPtr:=Data;
 GetMem(NewData,Size div 2);
 NewDataPtr:=NewData;
 for i:=0 to (Size div 4)-1 do
 begin
  NewDataPtr^:=PixelA8R8G8B8ToR5G6B5(DataPtr^);
  inc(NewDataPtr^);
  inc(DataPtr^);
 end;
 FreeMem(Data);
 Data:=NewData;
 Size:=Size div 2;
end;

Procedure ConvertSurfaceFromA8R8G8B8ToA1R5G5B5(var Data:PCardinal;var Size:cardinal);
var i:longint;
    DataPtr:PCardinal;
    NewData:Pointer;
    NewDataPtr:PWord;
begin
 DataPtr:=Data;
 GetMem(NewData,Size div 2);
 NewDataPtr:=NewData;
 for i:=0 to (Size div 4)-1 do
 begin
  NewDataPtr^:=PixelA8R8G8B8ToA1R5G5B5(DataPtr^);
  inc(NewDataPtr^);
  inc(DataPtr^);
 end;
 FreeMem(Data);
 Data:=NewData;
 Size:=Size div 2;
end;

Procedure ConvertSurfaceFromA8R8G8B8ToDXT1A(Width,Height:integer;var Data:PCardinal;var Size:cardinal);
var FinalSize:integer;
    FinalMem:pointer;
begin
 FinalSize:=Squish.GetStorageRequirements(Width,Height,kDxt1 or kColourMetricPerceptual);
 Getmem(FinalMem,FinalSize);
 Squish.CompressImage(Pu8(Data),Width,Height,FinalMem,kDxt1 or kColourMetricPerceptual);
 FreeMem(data);
 Data:=FinalMem;
 Size:=FinalSize;
end;

Procedure ConvertSurfaceFromA8R8G8B8ToDXT5(Width,Height:integer;var Data:PCardinal;var Size:cardinal);
var FinalSize:integer;
    FinalMem:pointer;
begin
 FinalSize:=Squish.GetStorageRequirements(Width,Height,kDxt5 or kColourMetricPerceptual);
 Getmem(FinalMem,FinalSize);
 Squish.CompressImage(Pu8(Data),Width,Height,FinalMem,kDxt5 or kColourMetricPerceptual);
 FreeMem(data);
 Data:=FinalMem;
 Size:=FinalSize;
end;

Procedure ConvertSurfaceFromDXT5ToA8R8G8B8(Width,Height:integer;var Data:PCardinal;var Size:cardinal);
var FinalSize:integer;
    FinalMem:pointer;
begin
 FinalSize:=Size*4;
 Getmem(FinalMem,FinalSize);
 Squish.DecompressImage(Pu8(FinalMem),Width,Height,Data,kDxt5 or kColourMetricPerceptual);
 FreeMem(data);
 Data:=FinalMem;
 Size:=FinalSize;
end;

Function ConvertSurfaceFromA8R8G8B8ToP8(var Data:PCardinal;var Size:cardinal):PPalette;
begin
 Result:=nil;
end;

Procedure ConvertSurfaceFromA1R5G5B5ToA8R8G8B8(var Data:PWord;var Size:cardinal);
var i:longint;
    DataPtr:PWord;
    NewData:Pointer;
    NewDataPtr:PCardinal;
begin
 DataPtr:=Data;
 GetMem(NewData,Size *2);
 NewDataPtr:=NewData;
 for i:=0 to (Size div 2)-1 do
 begin
  NewDataPtr^:=PixelA1R5G5B5ToA8R8G8B8(DataPtr^);
  inc(NewDataPtr^);
  inc(DataPtr^);
 end;
 FreeMem(Data);
 Data:=NewData;
 Size:=Size * 2;
end;


var i:integer;
initialization

 GreyPal.PalName:='Greypal';
 for i:=0 to 255 do
 begin
  Greypal.Rgb[i].r:=i;
  Greypal.Rgb[i].g:=i;
  Greypal.Rgb[i].b:=i;
 end;

end.

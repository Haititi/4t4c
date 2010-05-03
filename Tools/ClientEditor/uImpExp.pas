unit uImpExp;

interface
uses windows,udda,SurfaceUtils;

type
 TTgaHeader=packed record
  IdFieldLgth:byte;    //0;
  ColoMapType:byte;
  ImageTypeCode:byte;//always 1

  ColorMapOrg:word; //index of first colormap entry
  ColorMapLgh:word;//number of colors
  ColorDepth:byte;//bit per colormap entry

  XOrig:smallint;
  YOrig:smallint;
  Width:Smallint;
  Height:Smallint;
  PixSize:byte; //size of a stored pixel
  ImgDesc:byte; //0

  //idField depend of IdFieldLgth
  ColorMapData :TRgbPal;
  //data
 end;


 TBmpHeader=packed record
  //file header
  BmpType:word;  {'BM'}
  Size:longint;
  Res1,Res2:word;
  DataOffset:longint;
  //bitmap info header
  HdrSize:longint;
  Width:longint;
  Height:longint;
  Plane:word;
  Bitcount:word;
  Compression:longint;
  ImageSize:longint;
  biXPelsPerMeter:longint;
  biYPelsPerMeter:longint;
  biClrUsed:longint;
  biClrImportant:longint;
  end;

  Procedure LoadPicture(SrcFile:string;var Sprite:PSprite;var Palette:PPalette); //autoselect bmp/tga

  Procedure SaveTga(DestFile:string;Sprite:PSprite;Palette:PPalette);
  Procedure LoadTga(SrcFile:string;var Sprite:PSprite;var Palette:PPalette);

  Procedure SaveBmp(DestFile:string;Sprite:PSprite;Palette:PPalette);
  Procedure LoadBmp(SrcFile:string;var Sprite:PSprite;var Palette:PPalette);

  Procedure SpriteExport(DestDir:string;Sprite:PSprite;Palette:PPalette);
  Procedure PalExport(DestDir:string;Palette:PPalette);
implementation

uses FastStream,sysutils,ShellAPI;

Procedure CreateDirRecurse(Dir:String);
var p,i:integer;
    s:string;
begin
 p:=0;
 for i:=length(Dir)-1 downto 1 do
  if Dir[i]='\' then
  begin
   p:=i;
   break;
  end;
 if p=3 then exit; //c:\

 s:=copy(Dir,1,p);
 CreateDirRecurse(s);
 CreateDir(Dir);
end;


Procedure PalExport(DestDir:string;Palette:PPalette);
var fic:File;
begin
 AssignFile(Fic,DestDir+SanitizeString(Palette^.PalName)+'.pal');
 Rewrite(Fic,1);
 BlockWrite(Fic,Palette^.Rgb,768);
 CloseFile(Fic);
end;

Procedure SaveTxtInfo(const FileName:string;Sprite:PSprite;Palette:PPalette);
var Fic:TextFile;
begin
 AssignFile(Fic,FileName);
 Rewrite(fic);
 Writeln(fic,Sprite^.SpriteName);
 if Palette<>nil then
  Writeln(Fic,Palette^.PalName)
  else
  Writeln(Fic,'NOPAL');
 Writeln(fic,Sprite^.Width);
 Writeln(fic,Sprite^.Height);
 if Sprite^.Offset<>nil then
 begin
  Writeln(fic,Sprite^.Offset.OffsetX);
  Writeln(fic,Sprite^.Offset.OffsetY);
  Writeln(fic,Sprite^.Offset.OffsetX2);
  Writeln(fic,Sprite^.Offset.OffsetY2);
  Writeln(fic,Sprite^.Offset.Transparency);
  Writeln(fic,Sprite^.Offset.TransCol);
 end else
 begin
  Writeln(fic,'No extra data');
 end;
 CloseFile(Fic);
end;

Procedure SpriteExport(DestDir:string;Sprite:PSprite;Palette:PPalette);
var SpriteN:string;
    FinalDir:string;
begin
 FinalDir:=SanitizeDir(DestDir);
 CreateDirRecurse(FinalDir);
 SpriteN:=Sprite^.SpriteName;
 SaveTga(FinalDir+SpriteN+'.tga',Sprite,Palette);
 SaveTxtInfo(FinalDir+SpriteN+'.txt',Sprite,Palette);
end;

Procedure SaveTga(DestFile:string;Sprite:PSprite;Palette:PPalette);
var Data:PByte;
    DataPtr:PByte;
    TgaHeader:TTgaHeader;
    Fst:TFastStream;
    i,j:longint;
begin
  Data:=UncompressSprite(Sprite);
  if Data=NIL then
  begin
  // ShowMessage('Invalid Sprite !');
   exit;
  end;

  if Sprite.SurfFormat=SurfFormat_P8 then
  begin
   with TgaHeader do
   begin
    IdFieldLgth:=0;
    ColoMapType:=1;
    ImageTypeCode:=1;
    ColorMapOrg:=0;
    ColorMapLgh:=256;
    ColorDepth:=24;
    XOrig:=0;
    YOrig:=0;
    Width:=Sprite^.Width;
    Height:=Sprite^.Height;
    PixSize:=8;
    ImgDesc:=0;
   end;

   //we RGB -> BGR
   for i:=0 to 255 do
   begin
    TgaHeader.ColorMapData[i].r:=Palette^.Rgb[i].b;
    TgaHeader.ColorMapData[i].g:=Palette^.Rgb[i].g;
    TgaHeader.ColorMapData[i].b:=Palette^.Rgb[i].r;
   end;

   Fst:=TFastStream.Create;
   Fst.Write(TgaHeader,Sizeof(TgaHeader));

   for j:=TgaHeader.Height-1 downto 0 do
   begin
    DataPtr:=pointer(longint(Data)+j*TgaHeader.Width);
    FSt.Write(DataPtr^,TgaHeader.Width);
   end;
  end else
  begin
   with TgaHeader do
   begin
    IdFieldLgth:=0;
    ColoMapType:=0;
    ImageTypeCode:=2;
    ColorMapOrg:=0;
    ColorMapLgh:=0;
    ColorDepth:=32;
    XOrig:=0;
    YOrig:=0;
    Width:=Sprite^.Width;
    Height:=Sprite^.Height;
    PixSize:=32;
    ImgDesc:=0;
   end;

   Fst:=TFastStream.Create;
   Fst.Write(TgaHeader,Sizeof(TgaHeader)-768);

   for j:=TgaHeader.Height-1 downto 0 do
   begin
    DataPtr:=pointer(longint(Data)+j*TgaHeader.Width*4);
    FSt.Write(DataPtr^,TgaHeader.Width*4);
   end;
  end;

  Fst.WriteToFile(DestFile);
  Fst.Free;

  FreeMem(data);
end;

function DSWAP(n:cardinal):cardinal;
asm
 mov eax,n
 bswap eax
end;

Procedure LoadTga(SrcFile:string;var Sprite:PSprite;var Palette:PPalette);
var Fst:TFastStream;
    TgaHeader:TTgaHeader;
    i,j:integer;
    DataPtr:pointer;
    r,g,b:byte;
begin
 Fst:=TFastStream.Create;
 Fst.LoadFromFile(SrcFile);

 Fst.Read(TgaHeader,SizeOf(TTgaHeader)-768);

 if TgaHeader.ColoMapType=1 then //palettized
 begin
  Fst.Read(TgaHeader.ColorMapData,768);

  if Palette=nil then
   new(Palette);

  //we BGR -> RGB
   for i:=0 to 255 do
   begin
    Palette^.Rgb[i].b:=TgaHeader.ColorMapData[i].r;
    Palette^.Rgb[i].g:=TgaHeader.ColorMapData[i].g;
    Palette^.Rgb[i].r:=TgaHeader.ColorMapData[i].b;
   end;

  Sprite.Width:=TgaHeader.Width;
  Sprite.Height:=TgaHeader.Height;
  Sprite.SurfFormat:=SurfFormat_P8;
  Sprite.DataSize:=TgaHeader.Width*TgaHeader.Height*GetFormatSize(SurfFormat_P8);
  Sprite.CompType:=Comp_NoComp;
  getmem(Sprite^.Data,Sprite.DataSize);
  Fst.Read(Sprite^.Data^,Sprite.DataSize);
 end else
 if TgaHeader.PixSize=32 then
 begin //non palettized
  Sprite.Width:=TgaHeader.Width;
  Sprite.Height:=TgaHeader.Height;
  Sprite.SurfFormat:=SurfFormat_A8R8G8B8;
  Sprite.DataSize:=TgaHeader.Width*TgaHeader.Height*GetFormatSize(SurfFormat_A8R8G8B8);
  Sprite.CompType:=Comp_NoComp;
  getmem(Sprite^.Data,Sprite.DataSize);

  for j:=TgaHeader.Height-1 downto 0 do
  begin
   DataPtr:=pointer(longint(Sprite.Data)+j*TgaHeader.Width*4);
   FSt.Read(DataPtr^,TgaHeader.Width*4);
  end;
 end else
 if TgaHeader.PixSize=24 then
 begin //non palettized
  Sprite.Width:=TgaHeader.Width;
  Sprite.Height:=TgaHeader.Height;
  Sprite.SurfFormat:=SurfFormat_A8R8G8B8;
  Sprite.DataSize:=TgaHeader.Width*TgaHeader.Height*GetFormatSize(SurfFormat_A8R8G8B8);
  Sprite.CompType:=Comp_NoComp;
  getmem(Sprite^.Data,Sprite.DataSize);

  for j:=TgaHeader.Height-1 downto 0 do
  begin
   DataPtr:=pointer(longint(Sprite.Data)+j*TgaHeader.Width*4);
   for i:=0 to TgaHeader.Width-1 do
   begin
    //read BGR
    b:=Fst.ReadByte; g:=Fst.ReadByte; r:=Fst.ReadByte;
    PCardinal(DataPtr)^:=$FF000000 or (r shl 16) or (g shl 8) or b;
    inc(PCardinal(DataPtr));
   end;
  end;
 end else

 Fst.Free;
end;


Procedure SaveBmp(DestFile:string;Sprite:PSprite;Palette:PPalette);
var Fst:TFastStream;
    BmpHeader:TBmpHeader;
begin
  with BmpHeader do
  begin
   BmpType:=ord('B')+ord('M')shl 8;
   DataOffset:=(256*4)+Sizeof(bmpheader);
   res1:=0;
   res2:=0;
   HdrSize:=$28;
   Width:=Sprite^.Width;
   Height:=Sprite^.Height;
   Plane:=1;
   Bitcount:=4;
   Compression:=0;
   ImageSize:=(3072*3072) shr 1;
   biXPelsPerMeter:=216;
   biYPelsPerMeter:=216;
   biClrUsed:=8;
   biClrImportant:=8;
   Size:=Sizeof(bmpheader)+(256*4)+imagesize;
  end;

  {temp:=0;
  BlockWrite(fic,bmpheader,sizeof(bmpheader));
  for i:=0 to 23 do
  begin
   if i<8 then BlockWrite(fic,Mapcolors[i],4) else
               BlockWrite(fic,temp,4)
  end;

  for i:=3071 downto 0 do
   BlockWrite(fic,worldobjects.map[ListViewMap.Selected.index].AeraData[i,0],3072 shr 1);
                                             }
end;

Procedure LoadBmp(SrcFile:string;var Sprite:PSprite;var Palette:PPalette);
var Fst:TFastStream;
    BmpHeader:TBmpHeader;
    j:integer;
    DataPtr:pointer;
    Color:cardinal; //bmp palette are 32 bits
begin
 Fst:=TFastStream.Create;
 Fst.LoadFromFile(SrcFile);

 Fst.Read(BmpHeader,sizeof(TBmpHeader));

 if BmpHeader.Bitcount=8 then
 begin
 { Fst.Read(TgaHeader.ColorMapData,768);

  if Palette=nil then
   new(Palette);

  for i:=0 to 255 do
   Palette^.Rgb[i]:=TgaHeader.ColorMapData[i];

  Sprite.Width:=TgaHeader.Width;
  Sprite.Height:=TgaHeader.Height;
  Sprite.SurfFormat:=SurfFormat_P8;
  Sprite.DataSize:=TgaHeader.Width*TgaHeader.Height*GetFormatSize(SurfFormat_P8);
  Sprite.CompType:=Comp_NoComp;
  getmem(Sprite^.Data,Sprite.DataSize);
  Fst.Read(Sprite^.Data^,Sprite.DataSize);    }
 end else
 if BmpHeader.Bitcount=32 then
 begin
  Sprite.Width:=BmpHeader.Width;
  Sprite.Height:=BmpHeader.Height;
  Sprite.SurfFormat:=SurfFormat_A8R8G8B8;
  Sprite.DataSize:=BmpHeader.Width*BmpHeader.Height*GetFormatSize(SurfFormat_A8R8G8B8);
  Sprite.CompType:=Comp_NoComp;
  getmem(Sprite^.Data,Sprite.DataSize);

  Fst.Seek(BmpHeader.DataOffset);

  for j:=BmpHeader.Height-1 downto 0 do
  begin
   DataPtr:=pointer(longint(Sprite.Data)+j*BmpHeader.Width*4);
   FSt.Read(DataPtr^,BmpHeader.Width*4);
  end;
 end;

 Fst.Free;
end;


Procedure LoadPicture(SrcFile:string;var Sprite:PSprite;var Palette:PPalette);
var Ext:string;
begin

 if Sprite=nil then
  new(Sprite);

 Ext:=ExtractFileExt(SrcFile);
 if Ext='.tga' then
 begin
    LoadTga(SrcFile,Sprite,Palette)
 end else
 if Ext='.bmp' then
    LoadBmp(SrcFile,Sprite,Palette);
end;

end.

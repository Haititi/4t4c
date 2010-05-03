unit udda;

interface
uses windows,classes,sysutils,FastStream,hashpool,zlibex,forms,ComCtrls,Squish,MGR32_Image;

  //dpd

type

  POffset=^TOffset;
  TOffset=packed record
   OffsetName:string;
   PaletteName:string;
   OffsetX : SmallInt;
   OffsetY : SmallInt;
   OffsetX2 : SmallInt;
   OffsetY2 : SmallInt;
   Transparency : Word;
   TransCol : Word;
  end;

  PPath=^TPath;
  TPath=record
    SpriteName:string;
    Path:string;
  end;


  const
   Comp_NoComp=0;
   Comp_Zlib=1;
   Comp_Lzma=2;

   IdType_Floor=0;
   IdType_Blend=1;


  type

  TSprite = packed Record
    SpriteName : string; //longstring
    Node:TTreeNode; //internal var
    Path:string;  //internal var
    DataOffset : cardinal;
    DataSize  :cardinal;
    CompType  :cardinal;
    SurfFormat:cardinal;
    Width:cardinal;
    Height:Cardinal;
    Data : PByte;
    Offset:POffset; //internal var
  End;
  PSprite = ^TSprite;

  TIndex= packed Record
    Signature:array[0..3] of byte;
    DidVersion:cardinal;
    SpriteCount:Cardinal;
    Wasted:Cardinal;
    //SpriteNames : Array Of TSprite;
    SpriteNames:TList; //list of TSprite
    OffCount:Cardinal;
    Offsets: TList;
    PathCount:longint;
    Paths:array of TPath;
    PalVersion:Cardinal;
    PaletteCount:Cardinal;
    Palettes : TList;

    //hashtables
    PathHash,
    PalHash,
    OffHash,
    SpriteHash:THashTable;
  End;



 { TDDa = Array Of Record
    Signature : Array [0..3] of Byte;
    Sprites : TFastStream;
  End;   }

  Procedure CreateIndex(var Index:TIndex);
  procedure ResetIndex(var Index:TIndex);

  procedure LoadIndex(var Index:TIndex;Path:string);
  Procedure SaveIndex(var Index:TIndex;var App:TApplication;var Pgrb:TProgressBar;Path:string);

  Procedure AddSpriteToIndex(var Index:TIndex;Sprite:PSprite);
  Procedure DeleteSpriteFromIndex(var Index:TIndex;Sprite:PSprite);

  Procedure LoadSprite(var Fst:TFastStream;ToLoad:PSprite);

  function UncompressSprite(Sprite:PSprite):Pbyte;

  Procedure CompressSpriteZlib(Sprite:PSprite);
  Procedure CompressSpriteLzma(Sprite:PSprite);

  Procedure CropSprite(Sprite:PSprite);
  Procedure CutSpriteP8(Graph : PSprite;var SrcBuf:pointer);
  Procedure CutSpriteA8R8G8B8(Graph : PSprite;var SrcBuf:pointer);

  procedure CreateOffsetFromSprite(Sprite:PSprite);
  function GetSpriteOffset(SpriteName:string;var Offx:smallint;var Offy:Smallint;Reversed:cardinal):boolean;

  Function SanitizeString(Const s:string):string;
  Function SanitizeDir(Const s:string):string;  //allow


  Function GetSpriteA8R8G8B8Surface(Sprite:PSprite):Pointer;    //uncompress and convert any sprite to raw A8R8G8B8
  Procedure DrawA8R8G8B8(Surface:PCardinal;Width,Height,Offx,Offy:integer;Reversed,Trans:Cardinal;ColorMod:cardinal;ColorFx:Cardinal;View:TMPaintBox32);

  //utils
  function HexToInt(s:string):cardinal;
implementation

uses math,globals,SurfaceUtils,
//lzma include
ULZMABench,ULZMAEncoder,ULZMADecoder,ULZMACommon;

function HexToInt(s:string):cardinal;
var i, M: Cardinal;
begin
 Result:=0;
 M:=1;
 S:=AnsiUpperCase(S);
 for i:=Length(S) downto 1 do
 begin
  case S[i] of
   '1'..'9': Result:=Result+(Ord(S[i])-48)*M;
   'A'..'F': Result:=Result+(Ord(S[i])-55)*M;
  end;
  M:=M shl 4;
 end;
end;

Procedure DrawA8R8G8B8(Surface:PCardinal;Width,Height,Offx,Offy:integer;Reversed,Trans:Cardinal;ColorMod:cardinal;ColorFx:Cardinal;View:TMPaintBox32);
var i,j,Px,Py,w2,h2:longint;
    TmpSurf:Pcardinal;
    TmpSurfPtr:Pcardinal;
begin
 View.Buffer.Clear(0);

 TmpSurf:=AllocMem(Width*Height*4);
 Move(Surface^,TmpSurf^,Width*Height*4);

 w2:=View.Width div 2;
 h2:=View.Height div 2;

 Px:=(View.Width div 2)+Offx;
 Py:=(View.Height div 2)+Offy;

 View.Buffer.VertLineS(w2,0,View.Height,$ffffffff);
 View.Buffer.HorzLineS(0,h2,View.Width,$ffffffff);
 //ground "tile"
 View.Buffer.FillRectTS(w2-16,h2-8,w2+16,h2+8,$ffff0000);

 TmpSurfPtr:=TmpSurf;
 //pre process the surface
 if Trans=0 then
 begin
  for i:=0 to Width*height-1 do
  begin
   TmpSurfPtr^:=TmpSurfPtr^ Or $FF000000;
   inc(TmpSurfPtr);
  end;
 end;

 TmpSurfPtr:=TmpSurf;

  case ColorFx of
   0:begin //Modulate
      for i:=0 to Width*height-1 do
      begin
       TmpSurfPtr^:=ColorFxModulate(TmpSurfPtr^,ColorMod);
       inc(TmpSurfPtr);
      end;
     end;
   1:begin //add
      for i:=0 to Width*height-1 do
      begin
       TmpSurfPtr^:=ColorFxAdd(TmpSurfPtr^,ColorMod);
       inc(TmpSurfPtr);
      end;
     end;
   2:begin //sub
      for i:=0 to Width*height-1 do
      begin
       TmpSurfPtr^:=ColorFxSub(TmpSurfPtr^,ColorMod);
       inc(TmpSurfPtr);
      end;
     end;
  end;

 TmpSurfPtr:=TmpSurf;
 //TODO use the modulation color and fx
 if Reversed=0 then
 begin
 for j:=0 to Height-1 do
  for i:=0 to Width-1 do
  begin
   View.Buffer.SetPixelTS(i+Px,j+Py,TmpSurfPtr^);
   inc(TmpSurfPtr);
  end;
 end                     else
 begin
 for j:=0 to Height-1 do
  for i:=Width-1 downto 0 do
  begin
   View.Buffer.SetPixelTS(i+Px,j+Py,TmpSurfPtr^);
   inc(TmpSurfPtr);
  end;
 end;

 FreeMem(TmpSurf);
 View.Invalidate;
end;

Function GetSpriteA8R8G8B8Surface(Sprite:PSprite):Pointer;
var Data:PByte;
    Pal:PPalette;
    TransCol,trans:byte;
    DataSize:cardinal;
begin
  Result:=nil;
  Pal:=nil;
  Data:=UncompressSprite(Sprite);
  if Data=NIL then
  begin
   exit;
  end;

  case Sprite.SurfFormat of
   SurfFormat_A8R8G8B8:
   begin //nothing to do
   end;
   SurfFormat_P8:
   begin
    Transcol:=0;
    trans:=0;
    if Sprite.Offset<>nil then
    begin
     Pal:=Index.PalHash.SearchByName(Sprite^.Offset.PaletteName);
     Transcol:=Sprite.Offset.TransCol;
     Trans:=Sprite.Offset.Transparency;
    end;
    if Pal=Nil then
      Pal:=Index.PalHash.SearchByName('bright1');
    ConvertSurfaceFromP8ToA8R8G8B8(Data,Sprite.Width*Sprite.Height,Pal,Trans,TransCol);

   end;
   SurfFormat_DXT5:
   begin
    DataSize:=Sprite.DataSize;
    ConvertSurfaceFromDXT5ToA8R8G8B8(Sprite.Width,Sprite.Height,PCardinal(Data),DataSize);
   end;
  end;


  result:=data;
end;


procedure CreateOffsetFromSprite(Sprite:PSprite);
begin
 new(Sprite.Offset);
 Sprite.Offset.OffsetName:=Sprite.SpriteName;
 Sprite.Offset.OffsetX:=0;
 Sprite.Offset.OffsetY:=0;
 Sprite.Offset.OffsetX2:=0;
 Sprite.Offset.OffsetY2:=0;
 Sprite.Offset.Transparency:=0;
 Sprite.Offset.TransCol:=0;
 Index.Offsets.Add(Sprite.Offset);
end;

function GetSpriteOffset(SpriteName:string;var Offx:smallint;var Offy:Smallint;Reversed:cardinal):boolean;
var Sprite:PSprite;
begin
 Result:=false;
 Sprite:=Index.SpriteHash.SearchByName(SpriteName);
 if Sprite<>nil then
 begin
  if Sprite.Offset<>nil then
  begin
   if Reversed=0 then
   begin
    Offx:=Sprite.Offset.OffsetX;
    Offy:=sprite.Offset.OffsetY;
   end           else
   begin
    Offx:=Sprite.Offset.OffsetX2;
    Offy:=sprite.Offset.OffsetY2;
   end;
  end;
 end;
end;


Function SanitizeString(Const s:string):string;
var i:longint;
begin
 Result:=s;
 for i:=1 to length(s) do
  case ord(Result[i]) of
   0..31:Result[i]:=chr(ord(Result[i])+65);
   33..45:Result[i]:=chr(ord(Result[i])+65);
   47:Result[i]:=chr(ord(Result[i])+65);
   58..63:result[i]:=chr(ord(Result[i])-10);
   92:result[i]:=chr(96);
   123..255:Result[i]:=chr((ord(Result[i])mod 26)+65);
  end;
end;

Function SanitizeDir(Const s:string):string;
var i:longint;
begin
 Result:=s;
 for i:=1 to length(s) do
  case ord(Result[i]) of
   0..31:Result[i]:=chr(ord(Result[i])+65);
   33..45:Result[i]:=chr(ord(Result[i])+65);
   47:Result[i]:=chr(ord(Result[i])+65);
   59..63:result[i]:=chr(ord(Result[i])-10);
   123..255:Result[i]:=chr((ord(Result[i])mod 26)+65);
  end;
end;

Procedure CreateIndex(var Index:TIndex);
begin
 if Index.SpriteNames<>nil then
    Index.spriteNames.free;
 Index.SpriteNames:=TList.Create;

 if index.Offsets<>nil then
    Index.Offsets.free;
 Index.Offsets:=TList.Create;

 if Index.Palettes<>nil then
    Index.Palettes.free;
 Index.Palettes:=Tlist.Create;

 Index.PalHash:=THashTable.Create(1109);
 Index.PathHash:=THashTable.Create(10009);
 Index.OffHash:=THashTable.Create(10009);
 Index.SpriteHash:=THashTable.Create(10009);
end;

procedure ResetIndex(var Index:TIndex);
begin
 Index.SpriteNames:=nil;
 Index.Offsets:=nil;
 Index.Palettes:=nil;
 Index.PalHash:=nil;
 Index.PathHash:=nil;
 Index.OffHash:=nil;
 Index.SpriteHash:=nil;
end;

procedure LoadIndex(var Index:TIndex;Path:string);
var Fst,FstOff:TFastStream;
    i:integer;
    Sprite:PSprite;
    Palette:PPalette;
    Offset:POffset;
begin
 Index.PathCount:=0;
 Finalize(Index.Paths); //secu

 if FileExists(Path+'Path.dat') then
 begin
  Fst:=TFastStream.Create;
  FSt.LoadFromFile(Path+'Path.dat');
  Index.PathCount:=Fst.ReadLong;
  setlength(Index.Paths,Index.PathCount);
  for i:=0 to Index.PathCount-1 do
  begin
   Index.Paths[i].SpriteName:=Fst.ReadLongstring;
   Index.Paths[i].Path:=Fst.ReadLongstring;
  end;
  Fst.Free;
 end;


 // palette  Loading
 if not FileExists(PAth+'Palette.pi') then
 begin
  MessageBoxA(0,'Unable to find Dpd file','Error',MB_OK);
  exit;
 end;

 Fst:=TFastStream.Create;
 Fst.LoadFromFile(PAth+'Palette.pi');

 Index.PalVersion:=Fst.ReadLong;
 Index.PaletteCount:=Fst.ReadLong;
 for i:=0 to Index.PaletteCount-1 do
 begin
  new(Palette);
  Palette.PalName:=Fst.ReadLongstring;
  Fst.Read(Palette.Rgb,768);
  Index.Palettes.Add(Palette);
 end;
 Fst.Free;


 if not FileExists(Path+'Index.si') then
 begin
  MessageBoxA(0,'Unable to find Did file','Error',MB_OK);
  exit;
 end;

 Fst:=TFastStream.Create;
 Fst.LoadFromFile(Path+'Index.si');

 Fst.Read(Index.Signature[0],4);
 Index.DidVersion:=Fst.ReadLong;
 Index.SpriteCount:=Fst.ReadLong;
 Index.Wasted:=Fst.ReadLong;

 //Setlength(Index.SpriteNames,Index.Count);

 for i:=0 to Index.SpriteCount-1 do
 begin
  new(Sprite);
  Sprite^.SpriteName:=Fst.ReadLongstring;
  Sprite^.DataOffset:=Fst.ReadLong;
  Sprite^.DataSize:=Fst.ReadLong;
  Sprite^.CompType:=Fst.ReadLong;
  Sprite^.SurfFormat:=Fst.ReadLong;
  Sprite^.Width:=Fst.ReadLong;
  Sprite^.Height:=Fst.ReadLong;
  Sprite^.Offset:=nil;
  Index.SpriteNames.Add(Sprite);
 end;
 Fst.Free;

 FstOff:=TFastStream.Create;
 FstOff.LoadFromFile(Path+'Offset.dat');
 Index.OffCount:=FstOff.ReadLong;
 for i:=0 to Index.OffCount-1 do
 begin
  New(Offset);
  With Offset^ do
  begin
   OffsetName:=FstOff.ReadLongstring;
   PaletteName:=FstOff.ReadLongstring;
   OffsetX:=FstOff.ReadWord;
   OffsetY:=FstOff.ReadWord;
   OffsetX2:=FstOff.ReadWord;
   OffsetY2:=FstOff.ReadWord;
   Transparency:=FstOff.ReadWord;
   TransCol:=FstOff.ReadWord;
  end;
  Index.Offsets.Add(Offset);
 end;
 FstOff.Free;

 //fill hash tables
 for i:=0 to Index.Palettes.Count-1 do
 begin
   Index.PalHash.AddHashEntry(PPalette(Index.Palettes[i]).PalName,PPalette(Index.Palettes[i]));
 end;

 for i:=0 to Index.Offsets.Count-1 do
 begin
    Index.OffHash.AddHashEntry(POffset(Index.Offsets[i]).OffsetName,POffset(Index.Offsets[i]));
 end;

  Fst:=TFastStream.Create;
  Fst.LoadFromFile(Path+'data.data');
  for i:=0 to Index.SpriteNames.Count-1 do
  begin
    PSprite(Index.SpriteNames[i]).Offset:=Index.OffHash.SearchByName(PSprite(Index.SpriteNames[i]).SpriteName);
    LoadSprite(FSt,PSprite(Index.SpriteNames[i]));
    Index.SpriteHash.AddHashEntry(PSprite(Index.SpriteNames[i]).SpriteName,Index.SpriteNames[i]);
  end;

  Fst.Free;
end;


Procedure SaveIndex(var Index:TIndex;var App:TApplication;var Pgrb:TProgressBar;Path:string);
var Fst,FstData:TFastStream;
    i:longint;
    Sprite:PSprite;
    Palette:PPalette;
    Offset:POffset;
    step:integer;
begin
 Pgrb.Min:=0;
 Pgrb.Max:=100;
 Pgrb.Position:=0;
 Pgrb.Repaint;

 //reupdate Count in case of
 Index.SpriteNames.Pack;
 Index.Palettes.Pack;

 Index.SpriteCount:=Index.SpriteNames.Count;
 Index.PaletteCount:=Index.Palettes.Count;


 Fst:=TFastStream.Create;
 Fst.WriteLong(Index.PalVersion);
 Fst.WriteLong(Index.PaletteCount);
 for i:=0 to Index.PaletteCount-1 do
 begin
  Palette:=Index.Palettes[i];
  Fst.WriteLongString(Palette.PalName);
  Fst.Write(Palette.Rgb,768);
 end;
 Fst.WriteToFile(Path+'Palette.Pi');
 Fst.Free;

 Fst:=TFastStream.Create;
 Fst.WriteLong(Index.PathCount);
 for i:=0 to Index.PathCount-1 do
 begin
  Fst.WriteLongString(Index.Paths[i].SpriteName);
  Fst.WriteLongString(Index.Paths[i].Path);
 end;
 Fst.WriteToFile(Path+'Path.dat');
 Fst.Free;

 Pgrb.Position:=5;
 Pgrb.Repaint;

 Fst:=TFastStream.Create;
 Index.OffCount:=Index.Offsets.Count;
 Fst.WriteLong(Index.OffCount);
 for i:=0 to Index.OffCount-1 do
 begin
  Offset:=Index.Offsets[i];
  With Offset^ do
  begin
   Fst.WriteLongstring(OffsetName);
   Fst.WriteLongstring(PaletteName);
   Fst.WriteWord(word(OffsetX));
   Fst.WriteWord(word(OffsetY));
   Fst.WriteWord(word(OffsetX2));
   Fst.WriteWord(word(OffsetY2));
   Fst.WriteWord(Transparency);
   Fst.WriteWord(TransCol);
  end;
 end;
 Fst.WriteToFile(Path+'Offset.dat');
 Fst.Free;

 Pgrb.Position:=10;
 Pgrb.Repaint;

 Fst:=TFastStream.Create;
 FstData:=TFastStream.Create;

 Fst.Write(Index.Signature[0],4);
 Fst.WriteLong(Index.DidVersion);
 Fst.WriteLong(Index.SpriteCount);
 Fst.WriteLong(Index.Wasted);

 Step:=Index.SpriteCount div 90;
 try

 for i:=0 to Index.SpriteCount-1 do
 begin
  Sprite:=Index.SpriteNames[i];
  Fst.WriteLongString(Sprite^.SpriteName);
  Sprite^.DataOffset:=FstData.GetPosition;
  Fst.WriteLong(Sprite^.DataOffset);
  Fst.WriteLong(Sprite^.DataSize);
  Fst.WriteLong(Sprite^.CompType);
  Fst.WriteLong(Sprite^.SurfFormat);
  Fst.WriteLong(Sprite^.Width);
  Fst.WriteLong(Sprite^.Height);
  FstData.Write(Sprite^.Data^,Sprite^.DataSize);
  if i mod Step=0 then
  begin
   Pgrb.StepBy(1);
   Pgrb.Repaint;
  end;
 end;
 except
  MessageBoxA(0,pchar(inttostr(i)+' '+Sprite^.SpriteName),'Error',MB_OK);
 end;
 Fst.WriteToFile(Path+'Index.si');
 FstData.WriteToFile(Path+'Data.data');
 FstData.Free;
 Fst.Free;
end;

Procedure CropSprite(Sprite:PSprite);
var Comp:cardinal;
    Data:Pointer;
begin
 Comp:=Sprite.CompType;
 Data:=UncompressSprite(Sprite);

 if Data=NIL then
 begin
  MessageBox(0,'','Invalid Sprite !',MB_OK);
  exit;
 end;

 if Sprite.SurfFormat=SurfFormat_P8 then
 begin
  CutSpriteP8(Sprite,Data);
 end else
 begin
  MessageBox(0,'Cannot cut 32 bit sprite for now',' Warning !',MB_OK);
  //CutSpriteA8R8G8B8(Sprite,Data);
 end;

 if (Comp=Comp_Zlib) then
   CompressSpriteZlib(Sprite);
end;

Procedure AddSpriteToIndex(var Index:TIndex;Sprite:PSprite);
begin
 Index.SpriteNames.Add(Sprite);
 Index.SpriteHash.AddHashEntry(Sprite.SpriteName,Sprite);
end;

Procedure DeleteSpriteFromIndex(var Index:TIndex;Sprite:PSprite);
begin
 Index.SpriteNames.Remove(Sprite);
 Index.SpriteHash.RemoveHashEntry(Sprite.SpriteName);

 //free memory
 if Sprite^.data<>nil then
  FreeMem(Sprite.data);

 Dispose(Sprite);
end;

Procedure CutSpriteP8(Graph : PSprite;var SrcBuf:pointer);
  Var
    Limitx1, Limitx2, Limity1, Limity2, X2c, I, J, EndLoop : LongInt;
    BufPtr, NewBuf : Pointer;
    NbufPtr : PByte;
    GraphSize:longint;

Begin  //decoupe le sprite en gardant le necessaire
  Limitx1 := 0;
  Limitx2 := Graph^.Width - 1;
  Limity1 := 0;
  Limity2 := Graph^.Height - 1;
  GraphSize:=Graph^.Width*Graph^.Height;

  BufPtr  :=  SrcBuf;
  For I  :=  0 To GraphSize - 1 Do
    If Pbyte(LongInt(BufPtr) + I)^  <>  Graph^.Offset^.TransCol Then
      Begin
        Limity1  :=  (I Div Graph^.Width) - 1;
        Break;
      End;
  If limity1 < 0 Then limity1  :=  0;

  For I  :=  GraphSize - 1 DownTo 0 Do
    If Pbyte(LongInt(BufPtr)+i )^ <> Graph^.Offset^.TransCol  then
      Begin
        Limity2 := (i div Graph^.Width)+1;
        Break;
      End;
  If limity2 >= Graph^.Height Then limity2 := Graph^.Height - 1;

  EndLoop := 0;
  For I := 0 To Graph^.Width - 1 Do
  Begin
   For J := 0 To Graph^.Height - 1 Do
   begin
    If Pbyte(LongInt(BufPtr) + (I + J * Graph^.Width))^ <> Graph^.Offset^.TransCol Then
    Begin
     Limitx1 := I - 1;
     EndLoop := 1;
     break;
    End;
   end;
    If EndLoop = 1 Then Break;
  End;
  If Limitx1 < 0 Then Limitx1 := 0;

  EndLoop := 0;
  For I := Graph^.Width - 1 DownTo 0 Do
  Begin
   For J := 0 To Graph^.Height - 1 Do
   begin
    If Pbyte(LongInt(BufPtr) + (I + J * Graph^.Width))^ <> Graph^.Offset^.TransCol Then
    Begin
     Limitx2 := I + 1;
     EndLoop := 1;
     break;
    End;
   end;
   If EndLoop = 1 Then Break;
  End;
  If Limitx2 >= Graph^.Width Then Limitx2 := Graph^.Width - 1;
  If (limitx1 = 0) And (limity1 = 0) And (limitx2 = Graph^.Width - 1) And (Limity2 = Graph^.Height - 1) Then
  begin   //pas de decoupage a faire;  mais ! besoin deplacer buffer
    Graph^.DataSize := (((Limitx2 - Limitx1) + 1) * ((Limity2 - Limity1) + 1));
    FreeMem(Graph^.Data);
    Graph^.Data:=SrcBuf;
    Graph^.CompType:=Comp_NoComp;
    Exit;
  end;

  Graph^.DataSize := (((Limitx2 - Limitx1) + 1) * ((Limity2 - Limity1) + 1));
  If Graph^.DataSize > (1280 * 1024) Then
   MessageBox(0,pchar('Sprite Size Error : '),pchar('Error'),MB_OK);
  NewBuf := Allocmem(Graph^.DataSize);
  NBufPtr := NewBuf;
  For J := Limity1 To Limity2 Do
    For I := Limitx1 To Limitx2 Do
      Begin
        NBufPtr^ := Pbyte(LongInt(BufPtr) + (I + J * Graph^.Width))^;
        Inc(NBufPtr);
      End;
  X2c := (Graph^.Width - 1) - Limitx2; //todo: a verIf
  //now we need to correct the drawing offset
  If Limitx1 <> 0 Then Graph^.Offset^.OffsetX := Graph^.Offset^.Offsetx + Limitx1;
  Graph^.Offset^.offsetx2 := Graph^.Offset^.offsetx2 + x2c;
  //correcting both offset y
  If Limity1 <> 0 Then
    Begin
      Graph^.Offset^.offsety := Graph^.Offset^.Offsety + Limity1;
      Graph^.Offset^.offsety2 := Graph^.Offset^.offsety2 + Limity1;
    End;

 FreeMem(SrcBuf);

 FreeMem(Graph^.Data);
 Graph^.Data:=NewBuf;
 Graph^.CompType:=0;

 Graph^.Width := (limitx2 - limitx1) + 1;
 Graph^.Height := (limity2 - limity1) + 1;
End;

Procedure CutSpriteA8R8G8B8(Graph : PSprite;var SrcBuf:pointer);
begin //

end;


Procedure LoadSprite(var Fst:TFastStream;ToLoad:PSprite);
begin
 FSt.Seek(ToLoad.DataOffset);

 ToLoad.Data:=nil;
 GetMem(ToLoad.Data,ToLoad.DataSize);
 Fst.Read(ToLoad.Data^,ToLoad.DataSize);
end;

function UncompressSprite(Sprite:PSprite):Pbyte;
var Buffer:pointer;
    BufferSize:longint;
    //for lzma
    Decoder:TLZMADecoder;
    OutSize:int64;
    InBuf,OutBuf:TMemoryStream;
    Properties:array[0..4] of byte;
begin
 Result:=nil;
 case Sprite^.CompType of
  0:begin        //no comp
     GetMem(Result,Sprite.DataSize);
     Move(Sprite^.Data^,Result^,Sprite.DataSize);
    end;
  1:begin  //zlib
     ZDecompress(Sprite^.Data,Sprite^.DataSize,Buffer,BufferSize,Sprite^.Width*Sprite^.Height*GetFormatSize(Sprite.SurfFormat));
     Result:=Buffer;
    end;
  2:begin  //lzma
    Inbuf:=TMemoryStream.Create;
    OutBuf:=TMemoryStream.Create;
    Decoder:=TLZMADecoder.Create;
    //copy data in input stream
    Inbuf.Write(Sprite^.Data^,Sprite^.DataSize);
    Inbuf.Seek(0,FILE_BEGIN);
    Inbuf.read(Properties,5);
    decoder.SetDecoderProperties(properties);
    Inbuf.Read(Outsize,8);

    decoder.Code(Inbuf, OutBuf, outSize);
    OutSize:=OutBuf.Size;
    OutBuf.Seek(0,FILE_BEGIN);
    GetMem(Result,OutSize);
    OutBuf.Read(Result^,outSize);
    Decoder.Free;
    OutBuf.Free;
    Inbuf.Free;
   end;
 end;
end;

Procedure CompressSpriteZlib(Sprite:PSprite);
var TempBuf:pointer;
    TempSize:integer;
begin
 ZCompress(Sprite^.Data,Sprite^.DataSize,TempBuf,TempSize,zcMax);
 FreeMem(Sprite^.Data);
 Sprite^.Data:=TempBuf;
 Sprite^.DataSize:=TempSize;
 Sprite^.CompType:=Comp_Zlib;
end;

Procedure CompressSpriteLzma(Sprite:PSprite);
var encoder:TLZMAEncoder;
    filesize:int64;
    InBuf,OutBuf:TMemoryStream;
begin
 Inbuf:=TMemoryStream.Create;
 OutBuf:=TMemoryStream.Create;
 encoder:=TLZMAEncoder.Create;
 //copy data in input stream
 Inbuf.Write(Sprite^.Data^,Sprite^.DataSize);
 Inbuf.Seek(0,FILE_BEGIN);

 Encoder.SetAlgorithm(2);
 Encoder.SetDictionarySize(1 shl 23);
 Encoder.SeNumFastBytes(128);
 encoder.SetMatchFinder(1);
 encoder.SetLcLpPb(3,0,2);
 encoder.SetEndMarkerMode(true);
 Encoder.WriteCoderProperties(OutBuf);

 filesize:=-1;
 OutBuf.Write(Filesize,8);

 encoder.Code(Inbuf, OutBuf, -1, -1);

 FreeMem(Sprite^.Data);
 GetMem(Sprite^.Data,OutBuf.Size);
 OutBuf.Seek(0,FILE_BEGIN);
 OutBuf.Read(Sprite.Data^,OutBuf.Size);
 Sprite^.DataSize:=OutBuf.Size;

 Encoder.Free;
 OutBuf.Free;
 Inbuf.Free;

 Sprite^.CompType:=Comp_Lzma;
end;


end.

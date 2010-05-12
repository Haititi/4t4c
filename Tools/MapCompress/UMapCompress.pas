unit UMapCompress;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs,zlibex, StdCtrls;

type
  TFrmMapCompress = class(TForm)
    Button1: TButton;
    DlgOpen: TOpenDialog;
    Memo: TMemo;
    procedure Button1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    Procedure Convert(Name:String);
    procedure ComputeMultiDat(BaseName:string);
  end;

  TMapIndexRec=packed record
    Index:Cardinal;
    IndexSize:Cardinal;
  end;

var
  FrmMapCompress: TFrmMapCompress;
  MapId:array[0..8192] of cardinal;
  ClientMap:packed Array [0..3071,0..3071]of word;
  ClMap2:packed array[0..9437184] of word;

  basedir:string;

  StatMaxIdCount,StatMinIdCount:integer;
  DatFile:file;
implementation

{$R *.dfm}

procedure TFrmMapCompress.Button1Click(Sender: TObject);
begin
 if DlgOpen.Execute then
 begin
  Memo.Lines.Add('Converting : '+DlgOpen.FileName);
  Convert(DlgOpen.FileName);
 end;
end;

procedure Decomp(data:pword;inter:pword);
var val,val2:word;
    i:longint;
    decoded:longint;
begin  // si <$1000  copie, sinon  decomp
 decoded:=0;
 repeat
  val:=data^;
  inc(data);

  if (val < $2000) then
  begin
   inter^:=val;
   inc(inter);
   inc(decoded);
  end
		    else
  begin
    val2:=data^;
    inc(data);
   for i:=1 to (val-$2000) do
   begin
    inter^:=val2;
    inc(inter);
    inc(decoded);
   end;
  end;
 until decoded>=$4000;
end;

function MakeZone(Posx,Posy:integer):integer;
var XMin,XMax,YMin,YMax,i,j,Id:longint;
    IdCount:integer;
    IdArray:array[0..8191] of word;
begin
 XMin:=Posx;
 if XMin<0 then XMin:=0;
 XMax:=Posx+127;
 if XMax>3071 then XMax:=3071;

 YMin:=Posy;
 if YMin<0 then YMin:=0;
 YMax:=Posy+127;
 if YMax>3071 then YMax:=3071;


 FillChar(IdArray,sizeof(IdArray),0);
 for j:=YMin to YMax do
 begin
  for i:=XMin to XMax do
  begin
   inc(IdArray[ClientMap[i,j]]);
  end;
 end;


 IdCount:=0;
 for i:=0 to 8191 do
 begin
  if IdArray[i]>0 then
  begin
   inc(IdCount);
   Id:=i;
   blockwrite(DatFile,Id,2);
  end;
 end;

 Result:=IdCount;
 
 if IdCount>StatMaxIdCount then
  StatMaxIdCount:=IDCount;
 if IdCount<StatMinIdCount then
  StatMinIdCount:=IDCount;
end;

procedure TFrmMapCompress.ComputeMultiDat(BaseName: string);
var i,j,ZoneCount,ActualIndex,Position:integer;
    IDIndex:Packed array[0..(24*24*2)-1] of TMapIndexRec;

begin
 AssignFile(DatFile,BaseName+'.mdat');
 Rewrite(DatFile,1);
 Blockwrite(DatFile,IDIndex[0],Sizeof(IDIndex));

 StatMaxIdCount:=0;
 StatMinIdCount:=999999999;
 ZoneCount:=3072 div 128;
 ActualIndex:=0;
 Position:=24*24*2*8;// 24* 24 block *2 zone per block *8 bytes by index

 for j:=0 to ZoneCount-1 do
 begin
  For i:=0 to ZoneCount-1 do
  begin
   //first compute the even zone
   IDIndex[ActualIndex].Index:=Position;
   IDIndex[ActualIndex].IndexSize:=MakeZone(i*128,j*128);
   Position:=Position+IDIndex[ActualIndex].IndexSize*2;
   inc(ActualIndex);
   //do the same for odd zone
   IDIndex[ActualIndex].Index:=Position;
   IDIndex[ActualIndex].IndexSize:=MakeZone(i*128+64,j*128+64);
   Position:=Position+IDIndex[ActualIndex].IndexSize*2;
   inc(ActualIndex);
  end;
 end;
 Seek(DatFile,0);
 Blockwrite(DatFile,IDIndex[0],Sizeof(IDIndex));
 CloseFile(DatFile);

 Memo.Lines.Add('Max Number of id in a zone : '+IntToStr(StatMaxIdCount));
 Memo.Lines.Add('Min Number of id in a zone : '+IntToStr(StatMinIdCount));
end;

procedure TFrmMapCompress.Convert(Name: String);
var fic:file;
    Data,outbuffer:pointer;
    Bufptr:pword;
    size,outsize,i,numx,numy,num,y,k,j,IdCount:longint;
    tmpUnpack,map2,pixu:pword;
    Map3:PLongint;
    FPath,ShortName:string;
begin
 ShortName:=Name;
 delete(ShortName,length(Shortname)-3,4);

 assignfile(fic,name);
 reset(fic,1);
 size:=filesize(fic);
 getmem(Data,size);
 blockread(fic,Data^,size);
 closefile(fic);

 Memo.Lines.Add('Unpacking Map...');
 getmem(tmpunpack,128*128*2);
 num:=0;
 for numy:=0 to 23 do
  for numx:=0 to 23 do
  begin
   map3:=ptr(integer(data));
   inc(map3,num);
   map2:=data;
   inc(map2,map3^ div 2);
   Decomp(map2,tmpunpack);
   inc(num);
   pixu:=tmpunpack;
   for y:=0 to 127 do
   begin
    for k:=0 to 127 do
    begin
     ClientMap[numx shl 7+k,numy shl 7+y]:=pixu^;
     inc(pixu);
    end;
   // move(pixu^,clientmap[numy*128+y,numx*128],128*2);  // besoin inverser x et y pour affichage correct
   // inc(pixu,128);
   end;
  end;
  freemem(tmpUnpack);

 fillchar(MapId,sizeof(mapid),0);


 for j:=0 to 3071 do
  for i:=0 to 3071 do
  begin
   inc(mapid[ClientMap[i,j]]);
  end;

 IDCount:=0;
 for i:=0 to 8191 do
  if MapId[i]>0 then
   inc(IDCount);

 Memo.Lines.Add('There is '+IntToStr(IdCount)+' Unique Id on that map.');

 Memo.Lines.Add('Computing .dat ...');
 ComputeMultiDat(ShortName);

 //linearize memory before compression
 for j:=0 to 3071 do
  for i:=0 to 3071 do
  ClMap2[i+j*3072]:=ClientMap[i,j];

 //compress the map

 Memo.Lines.Add('Zlib Compress ...');
 ZCompress(@ClMap2[0],3072*3072*2,outbuffer,outsize,zcMax);

 assignfile(fic,ShortName+'.Zlb');
 rewrite(fic,1);
 blockwrite(fic,outBuffer^,outsize);
 closefile(fic);
 Memo.Lines.Add('Zlb Map saved.');

 freemem(data);
 freemem(outbuffer);
end;

procedure TFrmMapCompress.FormCreate(Sender: TObject);
begin
 BaseDir:=GetCurrentDir;
end;

end.

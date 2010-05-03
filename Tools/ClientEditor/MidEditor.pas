unit MidEditor;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs,ComCtrls, StdCtrls, Spin, MGR32_Image, Menus,IdTypes,FastStream,Globals,udda,SurfaceUtils;

type
  TFrmMIdEd = class(TForm)
    Label1: TLabel;
    SpinID: TSpinEdit;
    Label2: TLabel;
    EdtModX: TEdit;
    ChkDefined: TCheckBox;
    Label3: TLabel;
    EdtModY: TEdit;
    LblSpriteCount: TLabel;
    View: TMPaintBox32;
    ComboIdType: TComboBox;
    Label4: TLabel;
    Label5: TLabel;
    ComboLight: TComboBox;
    BtnNextDefined: TButton;
    BtnCopyTo: TButton;
    LstGraph: TListBox;
    Label6: TLabel;
    EdtMiniMapColor: TEdit;
    ViewMapColor: TMPaintBox32;
    Label7: TLabel;
    BtnClearList: TButton;
    GroupBox1: TGroupBox;
    EdtOffX: TEdit;
    ChkFlip: TCheckBox;
    Label8: TLabel;
    Label9: TLabel;
    EdtOffY: TEdit;
    Label10: TLabel;
    EdtOffZ: TEdit;
    Label11: TLabel;
    EdtVertexCol: TEdit;
    BtnAutoOff: TButton;
    PopupMenu1: TPopupMenu;
    Sort1: TMenuItem;
    Sort2: TMenuItem;
    ComboColFx: TComboBox;
    Label12: TLabel;
    ChkGrid: TCheckBox;
    MainMenu1: TMainMenu;
    File1: TMenuItem;
    ools1: TMenuItem;
    Load1: TMenuItem;
    Save1: TMenuItem;
    GenerateFromNMIni1: TMenuItem;
    GenerateIDFromExtractedIdList1: TMenuItem;
    DlgOpen: TOpenDialog;
    DlgSave: TSaveDialog;
    DlgColor: TColorDialog;
    N1: TMenuItem;
    CorrectALLoffsetsDanger1: TMenuItem;
    BtnTestFloor: TButton;
    N2: TMenuItem;
    RemoveFromList1: TMenuItem;
    BtnTemp1: TButton;
    N4: TMenuItem;
    GenerateMinimapMiniaturetexture1: TMenuItem;
    BtnNxtNonBlend: TButton;
    N5: TMenuItem;
    FusetilesCAUTION1: TMenuItem;
    BtnNextFloorId: TButton;
    FuseTiles1: TMenuItem;
    SortGroundinverse1: TMenuItem;
    procedure BtnNextDefinedClick(Sender: TObject);
    procedure Load1Click(Sender: TObject);
    procedure SpinIDChange(Sender: TObject);
    procedure Save1Click(Sender: TObject);
    Procedure RefreshAllView;
    Procedure RefreshGraphList;
    procedure ChkDefinedClick(Sender: TObject);
    procedure LstGraphDragOver(Sender, Source: TObject; X, Y: Integer;
      State: TDragState; var Accept: Boolean);
    procedure LstGraphDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure EdtModXChange(Sender: TObject);
    procedure EdtModYChange(Sender: TObject);
    procedure ComboIdTypeChange(Sender: TObject);
    procedure EdtMiniMapColorChange(Sender: TObject);
    procedure EdtVertexColChange(Sender: TObject);
    procedure BtnClearListClick(Sender: TObject);
    procedure EdtOffXChange(Sender: TObject);
    procedure EdtOffYChange(Sender: TObject);
    procedure EdtOffZChange(Sender: TObject);
    procedure ViewMapColorDblClick(Sender: TObject);
    procedure CorrectALLoffsetsDanger1Click(Sender: TObject);
    procedure LstGraphClick(Sender: TObject);
    Procedure DrawSprite(GInfo:PGraphInfo;Sprite:PSprite;Data:PCardinal);
    procedure ComboLightChange(Sender: TObject);
    procedure BtnTemp1Click(Sender: TObject);
    procedure GenerateMinimapMiniaturetexture1Click(Sender: TObject);
    procedure BtnNxtNonBlendClick(Sender: TObject);
    procedure ChkFlipClick(Sender: TObject);
    procedure BtnAutoOffClick(Sender: TObject);
    procedure FusetilesCAUTION1Click(Sender: TObject);
    procedure BtnNextFloorIdClick(Sender: TObject);
    procedure Sort1Click(Sender: TObject);

    procedure FuseTile(DestroyTiles:boolean);
    procedure FuseTiles1Click(Sender: TObject);
    procedure BtnTestFloorClick(Sender: TObject);
    procedure SortGroundinverse1Click(Sender: TObject);
    procedure Sort2Click(Sender: TObject);
    procedure RemoveFromList1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

Const MaxID=8192;

var
  FrmMIdEd: TFrmMIdEd;
  IdInfo: array [0..MaxID-1] of TMapIdInfo;

implementation

uses ueditdda;

{$R *.dfm}


function IntToHex(n:cardinal):string;
var m,m2,v1,v2:int64;
begin
 result:='';
 m2:=1;
 m:=16;
 v2:=n;
 repeat
  v1:=n mod m;
  v2:=v2 - v1;
  v1:=v1 div m2;

  case v1 of
   0..9:result:=chr(v1+48)+result;
   10..15:result:=chr(v1-10+65)+result;
  end;

  m:=m shl 4;
  m2:=m2 shl 4;
 until v2<=0;
end;



procedure TFrmMIdEd.BtnClearListClick(Sender: TObject);
var i:longint;
    GInfo:PGraphInfo;
begin
 for i:=0 to idinfo[spinid.value].GraphInfo.Count-1 do
 begin
  GInfo:=idinfo[spinid.value].GraphInfo[i];
  Dispose(GInfo);
 end;
 idinfo[spinid.value].GraphInfo.Clear;
 idinfo[spinid.value].GraphCount:=0;
 RefreshGraphList;
end;

procedure TFrmMIdEd.BtnNextDefinedClick(Sender: TObject);
var i:longint;
begin
 if SpinId.Value=(MaxId-1) then exit;
 for i:=SpinId.Value+1 to MaxId-1 do
  if IdInfo[i].Defined=1 then
  begin
    SpinId.Value:=i;
    exit;
  end;
end;

procedure TFrmMIdEd.BtnNextFloorIdClick(Sender: TObject);
var i:longint;
begin
 if SpinId.Value=(MaxId-2) then exit;
 for i:=SpinId.Value+1 to MaxId-1 do
  if IdInfo[i].Defined=1 then
   if IdInfo[i].IdType=IdType_Floor then
  begin
    SpinId.Value:=i;
    exit;
  end;
end;

procedure TFrmMIdEd.BtnNxtNonBlendClick(Sender: TObject);
var i:longint;
begin
 if SpinId.Value=(MaxId-2) then exit;
 for i:=SpinId.Value+1 to MaxId-1 do
  if IdInfo[i].Defined=1 then
   if IdInfo[i].IdType<>IdType_Blend then
  begin
    SpinId.Value:=i;
    exit;
  end;
end;

procedure TFrmMIdEd.Load1Click(Sender: TObject);
var i,j,Version,count:longint;
    Fst:TFastStream;
    GInfo:PGraphInfo;
begin
 if DlgOpen.Execute then
 begin
  Fst:=TFastStream.Create;
  Fst.LoadFromFile(DlgOpen.FileName);
  Version:=Fst.ReadLong;
  Count:=Fst.ReadLong;

  for i:=0 to MaxID-1 do
  begin
   IdInfo[i].Defined:=fst.ReadLong;
   if IdInfo[i].Defined=1 then
   begin
    IdInfo[i].IdType:=fst.ReadLong;
    IdInfo[i].Reversed:=Fst.ReadLong;
    IdInfo[i].Modx:=fst.ReadLong;
    IdInfo[i].Mody:=fst.ReadLong;
    IdInfo[i].MinimapColor:=fst.ReadLong;
    IdInfo[i].MiniMapTexPx:=Fst.ReadSingle;
    IdInfo[i].MiniMapTexPy:=Fst.ReadSingle;
    IdInfo[i].MiniMapTexPx2:=Fst.ReadSingle;
    IdInfo[i].MiniMapTexPy2:=Fst.ReadSingle;
    IdInfo[i].LightInfo:=Fst.ReadLong;
    IdInfo[i].ColorFx:=Fst.ReadLong;
    IdInfo[i].GraphCount:=fst.ReadLong;
    IdInfo[i].GraphInfo:=TList.Create;
    if IdInfo[i].GraphCount>0 then
     for j:=0 to IdInfo[i].GraphCount-1 do
     begin
      new(Ginfo);
      Ginfo.GraphName:=fst.ReadWordString;
      Ginfo.VertexCol:=Fst.ReadLong;
      Ginfo.Offx:=fst.ReadWord;
      Ginfo.Offy:=fst.ReadWord;
      Ginfo.Offz:=fst.ReadWord;
      IdInfo[i].GraphInfo.Add(GInfo);
     end;
   end;
  end;
  fst.Free;

  SpinIdChange(self);
 end;
end;

procedure TFrmMIdEd.Save1Click(Sender: TObject);
var i,j,Count,Version:longint;
    Fst:TFastStream;
    GInfo:PGraphInfo;
begin
 DlgSave.FileName:=DlgOpen.FileName;
 if DlgSave.Execute then
 begin
  Fst:=TFastStream.Create;

  Version:=1;
  Count:=MaxID;

  Fst.WriteLong(Version);
  Fst.WriteLong(Count);

  for i:=0 to MaxID-1 do
  begin
   Fst.WriteLong(IdInfo[i].Defined);
   if IdInfo[i].Defined=1 then
   begin
    Fst.WriteLong(IdInfo[i].IdType);
    Fst.WriteLong(IdInfo[i].Reversed);
    Fst.WriteLong(IdInfo[i].Modx);
    Fst.WriteLong(IdInfo[i].Mody);
    Fst.WriteLong(IdInfo[i].MinimapColor);
    Fst.WriteSingle(IdInfo[i].MiniMapTexPx);
    Fst.WriteSingle(IdInfo[i].MiniMapTexPy);
    Fst.WriteSingle(IdInfo[i].MiniMapTexPx2);
    Fst.WriteSingle(IdInfo[i].MiniMapTexPy2);
    Fst.WriteLong(IdInfo[i].LightInfo);
    Fst.WriteLong(IdInfo[i].ColorFx);
    Fst.WriteLong(IdInfo[i].GraphCount);
    if IdInfo[i].GraphCount>0 then
     for j:=0 to IdInfo[i].GraphCount-1 do
     begin
      Ginfo:=IdInfo[i].GraphInfo[j];
      Fst.WriteWordString(Ginfo.GraphName);
      Fst.WriteLong(Ginfo.VertexCol);
      Fst.WriteWord(Word(Ginfo.Offx));
      Fst.WriteWord(Word(Ginfo.Offy));
      Fst.WriteWord(Word(Ginfo.Offz));
     end;
   end;
  end;
  Fst.WriteToFile(DlgSave.FileName);
  fst.Free;
 end;
end;

Function StrToNum(s:string):longint;
var ps,ps2,ps3:longint;
    s2,s3:string;
begin
  ps:=pos('(',s);
  ps2:=pos(',',s);
  ps3:=pos(')',s);

  s2:=copy(s,ps+1,(ps2-ps)-1);
  s3:=copy(s,ps2+1,(ps3-ps2)-1);

  s2:=Trim(s2);
  s3:=trim(s3);

  result:=StrToInt(s2)*100+StrToInt(s3);
end;

Function StrToNumInv(s:string):longint;
var ps,ps2,ps3:longint;
    s2,s3:string;
begin
  ps:=pos('(',s);
  ps2:=pos(',',s);
  ps3:=pos(')',s);

  s2:=copy(s,ps+1,(ps2-ps)-1);
  s3:=copy(s,ps2+1,(ps3-ps2)-1);

  s2:=Trim(s2);
  s3:=trim(s3);

  result:=StrToInt(s3)*100+StrToInt(s2);
end;

procedure TFrmMIdEd.Sort1Click(Sender: TObject);
var i,j:longint;
    Temp:PGraphInfo;
begin
 if MessageDlg('Are you sure ?', mtWarning, [mbOK,mbCancel], 0) = mrOk then
 begin
  for j:=0 to idinfo[spinid.value].graphcount-1 do
   for i:=0 to idinfo[spinid.value].graphcount-2 do
    if StrToNum(PGraphInfo(idinfo[spinid.value].GraphInfo[i]).GraphName)>
        StrToNum(PGraphInfo(idinfo[spinid.value].GraphInfo[i+1]).GraphName) then
    begin
     temp:=idinfo[spinid.value].GraphInfo[i];
     idinfo[spinid.value].GraphInfo[i]:=idinfo[spinid.value].GraphInfo[i+1];
     idinfo[spinid.value].GraphInfo[i+1]:=temp;
    end;
  RefreshGraphList;
 end;
end;

procedure TFrmMIdEd.SortGroundinverse1Click(Sender: TObject);
var i,j:longint;
    Temp:PGraphInfo;
begin
 if MessageDlg('Are you sure ?', mtWarning, [mbOK,mbCancel], 0) = mrOk then
 begin
  for j:=0 to idinfo[spinid.value].graphcount-1 do
   for i:=0 to idinfo[spinid.value].graphcount-2 do
    if StrToNumInv(PGraphInfo(idinfo[spinid.value].GraphInfo[i]).GraphName)>
        StrToNumInv(PGraphInfo(idinfo[spinid.value].GraphInfo[i+1]).GraphName) then
    begin
     temp:=idinfo[spinid.value].GraphInfo[i];
     idinfo[spinid.value].GraphInfo[i]:=idinfo[spinid.value].GraphInfo[i+1];
     idinfo[spinid.value].GraphInfo[i+1]:=temp;
    end;
  RefreshGraphList;
 end;
end;

procedure TFrmMIdEd.Sort2Click(Sender: TObject);
var i,j:longint;
    Temp:PGraphInfo;
begin
 if MessageDlg('Are you sure ?', mtWarning, [mbOK,mbCancel], 0) = mrOk then
 begin
  for j:=0 to idinfo[spinid.value].graphcount-1 do
   for i:=0 to idinfo[spinid.value].graphcount-2 do
    if PGraphInfo(idinfo[spinid.value].GraphInfo[i]).GraphName>
        PGraphInfo(idinfo[spinid.value].GraphInfo[i+1]).GraphName then
    begin
     temp:=idinfo[spinid.value].GraphInfo[i];
     idinfo[spinid.value].GraphInfo[i]:=idinfo[spinid.value].GraphInfo[i+1];
     idinfo[spinid.value].GraphInfo[i+1]:=temp;
    end;
  RefreshGraphList;
 end;

end;

procedure TFrmMIdEd.SpinIDChange(Sender: TObject);
begin
 RefreshAllView;
end;

procedure TFrmMIdEd.DrawSprite(GInfo:PGraphInfo;Sprite: PSprite; Data: PCardinal);
var i,j,Px,Py:longint;
    RawPtr:PCardinal;
begin
 View.Buffer.Clear(0);
 RawPtr:=data;

 Px:=256-16+GInfo.Offx;
 Py:=256-8+GInfo.Offy;

 View.Buffer.VertLine(256,0,511,$ffffffff);
 View.Buffer.HorzLine(0,256,511,$ffffffff);
 View.Buffer.FillRectT(256-16,256-8,256+16,256+8,$ffff0000);

 //TODO use the modulation color and fx
 if IdInfo[SpinID.Value].Reversed=0 then
 begin
 for j:=0 to Sprite.Height-1 do
  for i:=0 to Sprite.Width-1 do
  begin
   View.Buffer.SetPixelTS(i+Px,j+Py,RawPtr^);
   inc(RawPtr);
  end;
 end                     else
 begin
 for j:=0 to Sprite.Height-1 do
  for i:=Sprite.Width-1 downto 0 do
  begin
   View.Buffer.SetPixelTS(i+Px,j+Py,RawPtr^);
   inc(RawPtr);
  end;
 end;


 View.Invalidate;
end;

procedure TFrmMIdEd.BtnTestFloorClick(Sender: TObject);
label 1;
var i,Px,Py,x,y:integer;
    GInfo:PGraphInfo;
    Sprite:PSprite;
    RawData,RawPtr:PCardinal;
begin
 y:=0;
 x:=0;
 View.Buffer.Clear(0);
 for i:=0 to IdInfo[SpinID.Value].GraphCount-1 do
 begin
  GInfo:=IdInfo[SpinID.Value].GraphInfo[i];
  Sprite:=Index.SpriteHash.SearchByName(GInfo.GraphName);
  if Sprite=nil then goto 1;
  RawData:=GetSpriteA8R8G8B8Surface(Sprite);
  RawPtr:=RawData;

  for Py:=0 to 16-1 do
   for Px:=0 to 32-1 do
   begin
    View.Buffer.PixelS[x*32+Px,y*16+Py]:=RawPtr^;
    inc(RawPtr);
   end;

  freemem(RawData);

  1:
  inc(x);
  if x=IdInfo[SpinID.Value].modx then
  begin
   x:=0;
   inc(y);
  end;
 end;
 View.Invalidate;
end;

procedure TFrmMIdEd.RefreshAllView;
var ActualId:longint;
begin
 if IdInfo[SpinId.Value].Defined=1 then
  ChkDefined.Checked:=true else ChkDefined.Checked:=false;
 LstGraph.Clear;
 View.Buffer.Clear(0);
 View.Invalidate;

 ActualId:=SpinID.Value;

 If ChkDefined.Checked then
 begin
  EdtModX.Text:=inttostr(IdInfo[ActualId].modx);
  EdtModY.Text:=IntToStr(IdInfo[ActualId].Mody);
  LblSpriteCount.Caption:='Sprite Count : '+IntToStr(IdInfo[ActualId].GraphCount);
  ComboIdType.ItemIndex:=IdInfo[ActualId].IdType;
  ComboLight.ItemIndex:=IdInfo[ActualId].LightInfo;
  ComboColFx.ItemIndex:=IdInfo[ActualId].ColorFx;
  if IdInfo[ActualId].Reversed=1 then
    ChkFlip.Checked:=true        else
    ChkFlip.Checked:=false;
  EdtMiniMapColor.Text:=IntToHex(IdInfo[ActualId].MinimapColor);
  ViewMapColor.Buffer.Clear(IdInfo[ActualId].MinimapColor);
  ViewMapColor.Invalidate;
  RefreshGraphList;
 end else
 begin
  EdtModX.Text:='';
  EdtModY.Text:='';
  LblSpriteCount.Caption:='Sprite Count : ';
  EdtMiniMapColor.Text:='';
  ComboIdType.ItemIndex:=-1;
  ComboLight.ItemIndex:=-1;
 end;
end;

procedure TFrmMIdEd.RefreshGraphList;
var i:longint;
    GInfo:PGraphInfo;
begin
 lstgraph.Items.BeginUpdate;
 LstGraph.Clear;
 if IdInfo[SpinId.Value].GraphCount>0 then
 begin
  for i:=0 to IdInfo[SpinId.Value].GraphCount-1 do
  begin
   GInfo:=IdInfo[SpinID.Value].GraphInfo[i];
   LstGraph.AddItem(GInfo^.GraphName,TObject(GInfo));
  end;
 end;
 lstgraph.Items.EndUpdate;
end;

procedure TFrmMIdEd.RemoveFromList1Click(Sender: TObject);
var idx:Integer;
    GInfo:PGraphInfo;
begin
 if IdInfo[SpinId.Value].defined=1 then
  if IdInfo[SpinId.Value].GraphCount>0 then
  begin
   idx:=LstGraph.ItemIndex;
   LstGraph.Items.Delete(Idx);
   GInfo:=IdInfo[SpinId.Value].GraphInfo[idx];
   IdInfo[SpinId.Value].GraphInfo.Delete(Idx);
   dispose(GInfo);
   dec(IdInfo[SpinId.Value].GraphCount);
  end;
end;

procedure TFrmMIdEd.CorrectALLoffsetsDanger1Click(Sender: TObject);
var i,j:longint;
    GInfo:PGraphInfo;
    Off:POffset;
begin
 for i:=0 to MaxID-1 do
  if IdInfo[i].Defined=1 then
   if IDInfo[i].IdType>1 then
   begin
    for j:=0 to IdInfo[i].GraphCount-1 do
    begin
     GInfo:=IdInfo[i].GraphInfo[j];
     Off:=Index.OffHash.SearchByName(GInfo.GraphName);
     if Off<>nil then
     begin
       if IdInfo[i].Reversed=0 then
       begin
        GInfo^.Offx:=Off.OffsetX;
        GInfo^.Offy:=Off.OffsetY;
       end else
       begin
        GInfo^.Offx:=Off.OffsetX2;
        GInfo^.Offy:=Off.OffsetY2;
       end;
     end;
    end;
   end;
end;



procedure TFrmMIdEd.LstGraphDragDrop(Sender, Source: TObject; X, Y: Integer);
var TempList:TList;
    i,src,dst:integer;
    Tree:TTreeView;
    GInfo:PGraphInfo;
begin //drag a list of graph from the graph editor
 if source is TTreeView then
 begin
   Tree:=TTreeView(Source);
   if Tree.SelectionCount>0 then    //we got something selected (sanity check)
   begin
    IdInfo[SpinID.Value].Defined:=1; //we force the define
    if IdInfo[SpinID.Value].GraphInfo = nil then
     IdInfo[SpinID.Value].GraphInfo:=TList.Create;
    for i:=0 to Tree.SelectionCount-1 do
    begin
     if Tree.Selections[i].ImageIndex=1 then  //reject directory
     begin
      new(GInfo);
      GInfo^.GraphName:=Tree.Selections[i].Text;
      GInfo^.VertexCol:=$FFFFFFFF;
      GInfo^.Offx:=0;
      GInfo^.Offy:=0;
      GInfo^.Offz:=0;
      IdInfo[SpinID.Value].GraphInfo.Add(GInfo);
      inc(IdInfo[SpinID.Value].GraphCount);
     end;
    end;
   end;
 end else
 if source=LstGraph then
 begin
  src:=LstGraph.ItemIndex;
  dst:=LstGraph.ItemAtPos(Point(x,y),true);

  if (src>0) and (src<LstGraph.Items.Count) then
  begin
   LstGraph.Items.Exchange(src,dst);
   IdInfo[SpinID.Value].GraphInfo.Exchange(src,dst);
  end;
 end;

 RefreshGraphList;
end;

procedure TFrmMIdEd.LstGraphDragOver(Sender, Source: TObject; X, Y: Integer;  State: TDragState; var Accept: Boolean);
var Tree:TTreeView;
begin
 Accept:=false;
 if Source is TTreeView then
 begin
  Tree:=TTreeView(Source);
  if Tree.Tag=2 then
   Accept:=true else
   Accept:=false;
 end else
 if Source=LstGraph then
 begin
   Accept:=true;
 end;
end;

procedure TFrmMIdEd.LstGraphClick(Sender: TObject);
var i:longint;
    GInfo:PGraphInfo;
    Data:PByte;
    Sprite:PSprite;
begin
 if LstGraph.ItemIndex>=0 then
 begin
  GInfo:=PGraphInfo(LstGraph.Items.Objects[LstGraph.ItemIndex]);
  EdtVertexCol.Text:=IntToHex(GInfo.VertexCol);
  EdtOffX.Text:=IntToStr(GInfo.Offx);
  EdtOffY.Text:=IntToStr(GInfo.Offy);
  EdtOffZ.Text:=IntToStr(GInfo.Offz);

  Sprite:=Index.SpriteHash.SearchByName(GInfo.GraphName);
  if Sprite=nil then
   exit;

  Data:=GetSpriteA8R8G8B8Surface(Sprite);
  DrawSprite(GInfo,Sprite,PCardinal(Data));
  FreeMem(Data);
 end;
end;

procedure TFrmMIdEd.BtnAutoOffClick(Sender: TObject);
var GInfo:PGraphInfo;
    Sprite:PSprite;
begin
 if IdInfo[SpinID.Value].Defined=1 then
 begin
  if LstGraph.ItemIndex>=0 then
  begin
   GInfo:=PGraphInfo(LstGraph.Items.Objects[LstGraph.ItemIndex]);
   GetSpriteOffset(GInfo^.GraphName,GInfo^.Offx,GInfo^.Offy,IdInfo[SpinID.Value].Reversed);
  end;
 end;
end;

procedure TFrmMIdEd.ViewMapColorDblClick(Sender: TObject);
var Col:Cardinal;
begin
 DlgColor.Execute;
 Col:=$FF000000 or ((DlgColor.Color and $ff) shl 16) or (((DlgColor.Color shr 8) and $ff) shl 8)  or ((DlgColor.Color shr 16) and $ff) ;
 EdtMiniMapColor.Text:=IntToHex(Col);
 EdtMiniMapColorChange(self);
end;

procedure TFrmMIdEd.ChkDefinedClick(Sender: TObject);
begin
 if ChkDefined.Checked then
  IdInfo[SpinID.Value].Defined:=1
                       else
  IdInfo[SpinID.Value].Defined:=0;
end;

procedure TFrmMIdEd.ChkFlipClick(Sender: TObject);
begin
 if ChkFlip.Checked then
  IdInfo[SpinId.Value].Reversed:=1
                    else
  IdInfo[SpinId.Value].Reversed:=0;
end;

procedure TFrmMIdEd.ComboIdTypeChange(Sender: TObject);
begin
 IdInfo[SpinId.Value].IdType:=ComboIdType.ItemIndex;
end;


procedure TFrmMIdEd.ComboLightChange(Sender: TObject);
begin
 case ComboLight.ItemIndex of
  0:IdInfo[SpinId.Value].LightInfo:=0;
  1:IdInfo[SpinId.Value].LightInfo:=1;
  2:IdInfo[SpinId.Value].LightInfo:=2;
  3:IdInfo[SpinId.Value].LightInfo:=4;
 end;
end;

procedure TFrmMIdEd.EdtMiniMapColorChange(Sender: TObject);
begin
 if EdtMiniMapColor.Text<>'' then
 begin
  IdInfo[SpinId.Value].MinimapColor:=HexToInt(EdtMiniMapColor.Text);
  ViewMapColor.Buffer.Clear(IdInfo[SpinId.Value].MinimapColor);
  ViewMapColor.Invalidate;
 end;
end;

procedure TFrmMIdEd.EdtModXChange(Sender: TObject);
begin
  if EdtModX.Text<>'' then
 begin
  IdInfo[SpinId.Value].Modx:=StrToInt(EdtModX.Text);
 end;
end;

procedure TFrmMIdEd.EdtModYChange(Sender: TObject);
begin
 if EdtMody.Text<>'' then
 begin
  IdInfo[SpinId.Value].Mody:=StrToInt(EdtMody.Text);
 end;
end;

procedure TFrmMIdEd.EdtOffXChange(Sender: TObject);
begin
if (EdtOffx.Text<>'') and (EdtOffx.Text<>'-') then
  if idinfo[SpinId.Value].Defined=1 then
   if idinfo[SpinId.Value].GraphCount>0 then
    if LstGraph.ItemIndex>-1 then
     PGraphInfo(idinfo[SpinId.Value].graphinfo[LstGraph.ItemIndex]).Offx:=strtoint(EdtOffx.Text);
end;

procedure TFrmMIdEd.EdtOffYChange(Sender: TObject);
begin
 if (EdtOffy.Text<>'') and (EdtOffy.Text<>'-') then
  if idinfo[SpinId.Value].Defined=1 then
   if idinfo[SpinId.Value].GraphCount>0 then
    if LstGraph.ItemIndex>-1 then
      PGraphInfo(idinfo[SpinId.Value].graphinfo[LstGraph.ItemIndex]).Offy:=strtoint(EdtOffy.Text);
end;

procedure TFrmMIdEd.EdtOffZChange(Sender: TObject);
begin
 if (EdtOffZ.Text<>'') and (EdtOffZ.Text<>'-') then
  if idinfo[SpinId.Value].Defined=1 then
   if idinfo[SpinId.Value].GraphCount>0 then
    if LstGraph.ItemIndex>-1 then
      PGraphInfo(idinfo[SpinId.Value].graphinfo[LstGraph.ItemIndex]).Offz:=strtoint(EdtOffZ.Text);
end;

procedure TFrmMIdEd.EdtVertexColChange(Sender: TObject);
begin
 if EdtVertexCol.Text<>'' then
  if idinfo[SpinId.Value].Defined=1 then
   if idinfo[SpinId.Value].GraphCount>0 then
    if LstGraph.ItemIndex>-1 then
    begin
      PGraphInfo(idinfo[SpinId.Value].graphinfo[LstGraph.ItemIndex]).VertexCol:=HexToInt(EdtVertexCol.Text);
    end;
end;

procedure TFrmMIdEd.FusetilesCAUTION1Click(Sender: TObject);
begin//
 FuseTile(true);
end;

procedure TFrmMIdEd.FuseTiles1Click(Sender: TObject);
begin
 FuseTile(false);
end;


procedure TFrmMIdEd.FuseTile(DestroyTiles: boolean);
var Sprite,Tile:PSprite;
    SprName:String;
    xc,yc,i,xs,ys,x,y,Ps,SpWidth,SpHeight:integer;
    SrcData:PByte;
    GInfo:PGraphInfo;
begin
 if IdInfo[SpinId.Value].Defined=1 then
 begin
  if idinfo[SpinId.Value].GraphCount>0 then
  begin
    if IdInfo[SpinID.Value].IdType=IdType_Floor then
    begin
     //it's ok we should be able to fuse

     //make a new sprite name
     SprName:=PGraphInfo(idinfo[SpinId.Value].graphinfo[0]).GraphName;
     Tile:=Index.SpriteHash.SearchByName(SprName);
     Ps:=Pos('(',SprName);
     if Ps<>0 then
     begin
      if SprName[Ps-1]=' ' then
       dec(Ps);
      SprName:=copy(SprName,1,Ps-1);
     end;

     SprName:='Fused_'+SprName;

     //determine new sprite size
     SpWidth:=IdInfo[SpinID.Value].Modx*32;
     SpHeight:=IdInfo[SpinID.Value].Mody*16;

     new(Sprite);
     Sprite.SpriteName:=SprName;
     Sprite.DataOffset:=0;
     Sprite.DataSize:=SpWidth*SpHeight;
     Sprite.CompType:=Comp_NoComp;
     Sprite.SurfFormat:=SurfFormat_P8;
     Sprite.Width:=SpWidth;
     Sprite.Height:=SpHeight;
     Sprite.Offset:=Tile.Offset;
     GetMem(Sprite.Data,Sprite.DataSize);

     x:=0;
     y:=0;
     for i:=0 to idinfo[SpinId.Value].GraphCount-1 do
     begin
      //take one tile
      GInfo:=idinfo[SpinId.Value].GraphInfo[y*idinfo[SpinId.Value].Modx+x];
      Tile:=Index.SpriteHash.SearchByName(GInfo.GraphName);

      //uncompress data;
      SrcData:=UncompressSprite(Tile);

      //copy the tile to the final texture
      //dest coord
      xc:=x*32;
      yc:=y*16;

      for ys:=0 to 15 do
      begin
       for xs:=0 to 31 do
       begin
        PByte(longint(Sprite.Data)+xc+xs+(yc+ys)*SpWidth)^:=Pbyte(longint(SrcData)+Ys*32+Xs)^;
       end;
      end;

      //free uncompressed tile Memory
      freemem(SrcData);

      inc(x);
      if x>=idinfo[SpinId.Value].Modx then
      begin
       x:=0;
       inc(y);
      end;
     end;


     if DestroyTiles then
     begin
      //delete the old sprites
      for i:=0 to idinfo[SpinId.Value].GraphCount-1 do
      begin
       GInfo:=idinfo[SpinId.Value].GraphInfo[i];
       ueditdda.FrmClEdit.DeleteSpriteByName(GInfo.GraphName);
       Dispose(GInfo);
      end;

      idinfo[SpinId.Value].GraphInfo.Clear;
     end;


     //add the new sprite to the general list
     ueditdda.FrmClEdit.AddSprite(Sprite);

     //add the new fused sprite to the id
     if (DestroyTiles) then
     begin
       idinfo[SpinId.Value].GraphCount:=1;
       new(GInfo);
       GInfo.GraphName:=Sprite.SpriteName;
       GInfo.VertexCol:=$FFFFFFFF;
       GInfo.Offx:=0;
       GInfo.Offy:=0;
       GInfo.Offz:=0;
       idinfo[SpinId.Value].GraphInfo.Add(GInfo);
     end;

    end;
  end;
 end;
 SpinIDChange(nil);

end;


procedure TFrmMIdEd.GenerateMinimapMiniaturetexture1Click(Sender: TObject);
var i,j:longint;
    Sx,Sy,Total:integer;
    FinalPosx,FinalPosy:longint;
    MinFillY:longint;
    GInfo:PGraphInfo;
    Sprite:PSprite;
    DataPtr:PCardinal;
    MinimapTexture:array[0..255,0..255] of cardinal;
    MinimapFilling:array[0..255,0..255] of boolean;
    MiniSprite:array[0..63,0..63] of cardinal;
    Procedure MakeMiniSprite;
    var Data:PCardinal;
        k,l:longint;
    begin
     Data:=GetSpriteA8R8G8B8Surface(Sprite);
     if Idinfo[i].Reversed=0 then
     begin
      for l:=0 to Sy-1 do
       for k:=0 to Sx-1 do
       begin
        MiniSprite[k,l]:=PCardinal(longint(Data)+(k*32*4)+(l*16*4*Sprite.Width))^;
       end;
     end                     else
     begin
      for l:=0 to Sy-1 do
       for k:=Sx-1 downto 0 do
       begin
        MiniSprite[(Sx-1)-k,l]:=PCardinal(longint(Data)+(k*32*4)+(l*16*4*Sprite.Width))^;
       end;
     end;
      freemem(data);
    end;

    Function TestSquare(const StartX,StartY:integer):boolean;
    var k,l:integer;
    begin
     if ((StartX+Sx-1)>=256) or ((StartY+Sy-1)>=256) then
     begin
      result:=false;
      exit;
     end; 
     for l:=StartY to StartY+Sy-1 do
      for k:=StartX to StartX+Sx-1 do
      if MinimapFilling[k,l]=true then
      begin
       result:=false;
       exit;
      end;
     result:=true;
    end;

    Procedure FindEmptySpace;
    var k,l:longint;
        FreeLine:boolean;
    begin
     for l:=MinFillY to 255 do
     begin
      FreeLine:=true;
      for k:=0 to 255 do
      begin
       if MinimapFilling[k,l]=false then
       begin
        FreeLine:=false;
        //test square zone
        if TestSquare(k,l)= true then
        begin
         FinalPosx:=k;
         FinalPosy:=l;
         exit;
        end;
       end;
      end;
      if FReeLine=true then
       MinFilly:=l+1;
     end;
     ShowMessage('no empty Space');
    end;

    Procedure TransferMiniature;
    var k,l:longint;
    begin
     for l:=0 to Sy-1 do
      for k:=0 to Sx-1 do
      begin
       MinimapTexture[FinalPosx+k,FinalPosy+l]:=MiniSprite[k,l];
       MinimapFilling[FinalPosx+k,FinalPosy+l]:=true;
      end;
    end;
begin
 Total:=0;
 MinFillY:=0;
 for i:=0 to 255 do
  for j:=0 to 255 do
  begin
   MinimapFilling[i,j]:=false;
   MinimapTexture[i,j]:=$FF000000;
  end;
 for i:=0 to MaxID-1 do
  if Idinfo[i].Defined=1 then
  begin
   if IdInfo[i].IdType=IdType_Blend then
   begin
    GInfo:=PGraphInfo(IdInfo[IdInfo[i].Modx].GraphInfo[0]);
   end else
   begin
    GInfo:=PGraphInfo(IdInfo[i].GraphInfo[0]);
   end;
   Sprite:=Index.SpriteHash.SearchByName(GInfo.GraphName);
   if Sprite=nil then
    continue;
   Sx:=Sprite.Width div 32;
   if Sx=0 then Sx:=1;
   Sy:=Sprite.Height div 16;
   if Sy=0 then Sy:=1;
   Total:=Total+Sx*Sy;
   if sx>20 then
    ShowMessage('test');

   MakeMiniSprite;
   FindEmptySpace;
   TransferMiniature;
   IdInfo[i].MiniMapTexPx:=(0.5+FinalPosx)/256;
   IdInfo[i].MiniMapTexPy:=(0.5+FinalPosy)/256;
   IdInfo[i].MiniMapTexPx2:=(0.5+FinalPosx+Sx)/256;
   IdInfo[i].MiniMapTexPy2:=(0.5+FinalPosy+Sy)/256;
  end;
  Sprite:=nil;
  new(Sprite);
  Sprite.SpriteName:='RadarTexture';
  Sprite.DataOffset:=0;
  Sprite.DataSize:=256*256*4;
  Sprite.CompType:=Comp_NoComp;
  Sprite.SurfFormat:=SurfFormat_A8R8G8B8;
  Sprite.Width:=256;
  Sprite.Height:=256;
  GetMem(Sprite.Data,Sprite.DataSize);
  Sprite.Offset:=Nil;



  DataPtr:=PCardinal(Sprite.Data);
  for j:=0 to 255 do
   for i:=0 to 255 do
   begin
    DataPtr^:=MinimapTexture[i,j];
    inc(DataPtr);
   end;

  FrmClEdit.AddSprite(Sprite);
  ShowMessage('Total Pixel Count : '+IntToStr(Total));
end;

procedure TFrmMIdEd.BtnTemp1Click(Sender: TObject);
var i,ps,Num:longint;
    s:string;
    GInfo:PGraphInfo;
begin
{ for i:=0 to 8192-1 do
 begin
  if IdInfo[i].Defined=1 then
   if IdInfo[i].IdType=IdType_Blend then
   begin
     GInfo:=PGraphInfo(IdInfo[i].GraphInfo[0]);
     s:=GInfo.GraphName;
     ps:=Pos(' ',s);
     delete(s,1,ps);
     Num:=StrToInt(s);

     if Num<=32 then
     begin   //tmpl1
      GInfo.GraphName:='Tmpl1 '+IntToStr(Num);
     end else
     begin  //tmpl3
      dec(Num,32);
      GInfo.GraphName:='Tmpl3 '+IntToStr(Num);
     end;
     if Num>32 then
     begin
      Dec(Num,32);
      GInfo.GraphName:='Tmpl4 '+inttostr(Num);
     end;
   end;
 end;         }
end;



end.

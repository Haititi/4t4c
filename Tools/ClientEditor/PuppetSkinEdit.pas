unit PuppetSkinEdit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Menus, MGR32_Image;

type
  TFrmPuppetSkinEdit = class(TForm)
    LstSkinPart: TListBox;
    EdtSkinId: TEdit;
    Label3: TLabel;
    LstAction: TListBox;
    Label5: TLabel;
    LstDir: TListBox;
    Label6: TLabel;
    EdtAnimDelay: TEdit;
    Label7: TLabel;
    LstSprite: TListBox;
    View: TMPaintBox32;
    Label8: TLabel;
    PopupSpriteList: TPopupMenu;
    MainMenu1: TMainMenu;
    File1: TMenuItem;
    LoadMonsterSkins1: TMenuItem;
    SAveMonsters1: TMenuItem;
    EdtVtxColor: TEdit;
    Label9: TLabel;
    GroupBox1: TGroupBox;
    ChkReverse: TCheckBox;
    Button1: TButton;
    Button2: TButton;
    DlgOpen: TOpenDialog;
    DlgSave: TSaveDialog;
    PopupSkinPartList: TPopupMenu;
    AddSkin1: TMenuItem;
    Duplicateskin1: TMenuItem;
    N1: TMenuItem;
    DeleteSkin1: TMenuItem;
    Label4: TLabel;
    ComboColorFx: TComboBox;
    EdtOffX: TEdit;
    Label11: TLabel;
    Label12: TLabel;
    EdtOffY: TEdit;
    N2: TMenuItem;
    AutoFill1: TMenuItem;
    AutoSort1: TMenuItem;
    N3: TMenuItem;
    ClearList1: TMenuItem;
    Sort1: TMenuItem;
    Label10: TLabel;
    EdtSkinName: TEdit;
    Label13: TLabel;
    ChkFemale: TCheckBox;
    Help1: TMenuItem;
    Options1: TMenuItem;
    SetMaxAutoFillSearch1: TMenuItem;
    procedure SavePuppet1Click(Sender: TObject);
    procedure LoadPuppetSkins1Click(Sender: TObject);
    procedure AddSkin1Click(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure DeleteSkin1Click(Sender: TObject);
    procedure EdtNameChange(Sender: TObject);
    procedure LstSkinPartClick(Sender: TObject);
    procedure EdtVtxColorChange(Sender: TObject);
    procedure EdtAnimDelayChange(Sender: TObject);
    procedure ComboColorFxChange(Sender: TObject);
    procedure LstActionClick(Sender: TObject);
    procedure LstDirClick(Sender: TObject);
    Procedure RefreshView;
    procedure LstSpriteDragOver(Sender, Source: TObject; X, Y: Integer; State: TDragState; var Accept: Boolean);
    procedure LstSpriteDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure LstSpriteClick(Sender: TObject);
    procedure AutoFill1Click(Sender: TObject);
    procedure ClearList1Click(Sender: TObject);
    procedure Sort1Click(Sender: TObject);
    procedure ChkFemaleClick(Sender: TObject);
    procedure EdtSkinIdChange(Sender: TObject);
    procedure SetMaxAutoFillSearch1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  FrmPuppetSkinEdit: TFrmPuppetSkinEdit;
  MaxAutoFillWalk,MaxAutoFillAttack,MaxAutoFillRange:integer;

implementation

{$R *.dfm}
uses comctrls,FastStream,udda,IdTypes,globals;

procedure TFrmPuppetSkinEdit.FormCreate(Sender: TObject);
begin
 MaxAutoFillWalk:=ord('m')-ord('a');
 MaxAutoFillAttack:=ord('i')-ord('a');
 MaxAutoFillRange:=ord('i')-ord('a');
end;

procedure TFrmPuppetSkinEdit.FormResize(Sender: TObject);
begin
 LstSkinPart.Height:=(FrmPuppetSkinEdit.ClientHeight-LstSkinPart.Top)-4;
 LstSprite.Height:=(FrmPuppetSkinEdit.ClientHeight-LstSprite.Top)-4;
 GroupBox1.Top:=(FrmPuppetSkinEdit.ClientHeight-GroupBox1.Height)-4;

 View.Width:=(FrmPuppetSkinEdit.ClientWidth-View.Left)-4;
 View.Height:=(FrmPuppetSkinEdit.ClientHeight-View.Top)-4;
end;

Procedure ReadFrameFromStream(var Fst:TFastStream;Frame:PFrame);
begin
 Frame^.SpriteName:=Fst.ReadWordString;
 Frame^.Offx:=Fst.ReadWord;
 Frame^.Offy:=Fst.ReadWord;
end;

Procedure ReadSoundFromStream(var Fst:TFastStream;Sound:PSoundInfo);
begin
 Sound^.SoundName:=Fst.ReadWordString;
 Sound^.PitchDev:=Fst.ReadSingle;
end;

Procedure WriteFrameToStream(var Fst:TFastStream;Frame:PFrame);
begin
 Fst.WriteWordString(Frame^.SpriteName);
 Fst.WriteWord(Word(Frame^.Offx));
 Fst.WriteWord(Word(Frame^.Offy));
end;

Procedure WriteSoundToStream(var Fst:TFastStream;Sound:PSoundInfo);
begin
 Fst.WriteWordString(Sound^.SoundName);
 Fst.WriteSingle(Sound^.PitchDev);
end;

procedure TFrmPuppetSkinEdit.LoadPuppetSkins1Click(Sender: TObject);
var i,j,k,Count,Version:longint;
    Fst:TFastStream;
    Skin:PPuppetSkinPartInfo;
    Frame:PFrame;
    Sound:PSoundInfo;
begin
 if DlgOpen.Execute then
 begin
  Fst:=TFastStream.Create;
  Fst.LoadFromFile(DlgOpen.FileName);
  Version:=Fst.ReadLong;
  Count:=Fst.ReadLong;

  for i:=0 to Count-1 do
  begin
   new(Skin);
   with Skin^ do
   begin
    SkinName:=Fst.ReadWordString;
    SkinId:=Fst.ReadWord;
    Female:=Fst.ReadLong=1;
    VertexColor:=Fst.ReadLong;
    ColorFx:=Fst.ReadLong;
    AnimationDelay:=Fst.ReadSingle;

    for j:=0 to 7 do
    begin
      Walk[j].Reversed:=Fst.ReadLong;
      Walk[j].GraphCount:=Fst.ReadLong;
      Walk[j].FrameList:=Tlist.Create;
      for k:=0 to Walk[j].GraphCount-1 do
      begin
       new(Frame);
       ReadFrameFromStream(Fst,Frame);
       Walk[j].FrameList.Add(Frame);
      end;
    end;

    for j:=0 to 7 do
    begin
      Attack[j].Reversed:=Fst.ReadLong;
      Attack[j].GraphCount:=Fst.ReadLong;
      Attack[j].FrameList:=Tlist.Create;
      for k:=0 to Attack[j].GraphCount-1 do
      begin
       new(Frame);
       ReadFrameFromStream(Fst,Frame);
       Attack[j].FrameList.Add(Frame);
      end;
    end;

    for j:=0 to 7 do
    begin
      Range[j].Reversed:=Fst.ReadLong;
      Range[j].GraphCount:=Fst.ReadLong;
      Range[j].FrameList:=Tlist.Create;
      for k:=0 to Range[j].GraphCount-1 do
      begin
       new(Frame);
       ReadFrameFromStream(Fst,Frame);
       Range[j].FrameList.Add(Frame);
      end;
    end;

    Death.Reversed:=Fst.ReadLong;
    Death.GraphCount:=Fst.ReadLong;
    Death.FrameList:=Tlist.Create;
    for k:=0 to Death.GraphCount-1 do
    begin
     new(Frame);
     ReadFrameFromStream(Fst,Frame);
     Death.FrameList.Add(Frame);
    end;
   end;
   LstSkinPart.AddItem(Skin^.SkinName,TObject(Skin));
  end;
  Fst.Free;
 end;
end;

procedure TFrmPuppetSkinEdit.SavePuppet1Click(Sender: TObject);
var i,j,k,Count,Version:longint;
    Fst:TFastStream;
    Skin:PPuppetSkinPartInfo;
    Frame:PFrame;
begin
 DlgSave.FileName:=DlgOpen.FileName;
 if DlgSave.Execute then
 begin
  Fst:=TFastStream.Create;

  Version:=1;
  Fst.WriteLong(Version);
  Count:=LstSkinPart.Count;
  Fst.WriteLong(Count);
  for i:=0 to Count-1 do
  begin
   Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[i]);
   with Skin^ do
   begin
    Fst.WriteWordString(SkinName);
    Fst.WriteWord(SkinId);
    Fst.WriteLong(longint(Female));
    Fst.WriteLong(VertexColor);
    Fst.WriteLong(ColorFx);
    Fst.WriteSingle(AnimationDelay);
    for j:=0 to 7 do
    begin
      Fst.WriteLong(Walk[j].Reversed);
      Walk[j].GraphCount:=Walk[j].FrameList.Count;
      Fst.WriteLong(Walk[j].GraphCount);
      for k:=0 to Walk[j].GraphCount-1 do
      begin
       WriteFrameToStream(Fst,Walk[j].FrameList[k]);
      end;
    end;
    for j:=0 to 7 do
    begin
      Fst.WriteLong(Attack[j].Reversed);
      Attack[j].GraphCount:=Attack[j].FrameList.Count;
      Fst.WriteLong(Attack[j].GraphCount);
      for k:=0 to Attack[j].GraphCount-1 do
      begin
       WriteFrameToStream(Fst,Attack[j].FrameList[k]);
      end;
    end;
    for j:=0 to 7 do
    begin
      Fst.WriteLong(Range[j].Reversed);
      Range[j].GraphCount:=Range[j].FrameList.Count;
      Fst.WriteLong(Range[j].GraphCount);
      for k:=0 to Range[j].GraphCount-1 do
      begin
       WriteFrameToStream(Fst,Range[j].FrameList[k]);
      end;
    end;
    Fst.WriteLong(Death.Reversed);
    Death.GraphCount:=Death.FrameList.Count;
    Fst.WriteLong(Death.GraphCount);
    for k:=0 to Death.GraphCount-1 do
    begin
      WriteFrameToStream(Fst,Death.FrameList[k]);
    end;
   end;
  end;

  Fst.WriteToFile(DlgSave.FileName);
  Fst.Free;
 end;
end;


procedure TFrmPuppetSkinEdit.SetMaxAutoFillSearch1Click(Sender: TObject);
var Result:string;
begin
 Result:='m';
 InputQuery('Max autofill Walk','a-z',Result);
 MaxAutoFillWalk:=(ord(Result[1])-ord('a'));

 Result:='i';
 InputQuery('Max autofill Attack','a-z',Result);
 MaxAutoFillAttack:=(ord(Result[1])-ord('a'));

 Result:='i';
 InputQuery('Max autofill Range','a-z',Result);
 MaxAutoFillRange:=(ord(Result[1])-ord('a'));
end;

procedure TFrmPuppetSkinEdit.Sort1Click(Sender: TObject);
begin
 LstSkinPart.Sorted:=LstSkinPart.Sorted xor true;
 TMenuItem(sender).Checked:=LstSkinPart.Sorted;
end;

procedure TFrmPuppetSkinEdit.AddSkin1Click(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
    i:longint;
begin
 New(Skin);
 with Skin^ do
 begin
  SkinName:='NewSkinPart';
  //SkinId:=0;
  VertexColor:=$FFFFFFFF;
  ColorFx:=0;
  AnimationDelay:=0.033;
  for i:=0 to 7 do
  begin
   Walk[i].Reversed:=0;
   Walk[i].GraphCount:=0;
   Walk[i].FrameList:=TList.Create;
   Attack[i].Reversed:=0;
   Attack[i].GraphCount:=0;
   Attack[i].FrameList:=TList.Create;

   Range[i].Reversed:=0;
   Range[i].GraphCount:=0;
   Range[i].FrameList:=TList.Create;
  end;

  for i:=5 to 7 do
  begin
   Walk[i].Reversed:=1;
   Attack[i].Reversed:=1;
   Range[i].Reversed:=1;
  end;
  
  Death.Reversed:=0;
  Death.GraphCount:=0;
  Death.FrameList:=TList.Create;

 end;
 LstSkinPart.AddItem(Skin^.SkinName,TObject(Skin));
end;



procedure TFrmPuppetSkinEdit.DeleteSkin1Click(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
    i,j:integer;
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  With Skin^ do
  begin
   for i:=0 to 7 do
   begin
    Walk[i].Reversed:=0;
    Walk[i].GraphCount:=0;
    for j:=0 to Walk[i].FrameList.Count-1 do
    begin
     Dispose(PFrame(Walk[i].FrameList.Items[j]));
    end;
    Walk[i].FrameList.Free;
    Attack[i].Reversed:=0;
    Attack[i].GraphCount:=0;
    for j:=0 to Attack[i].FrameList.Count-1 do
    begin
     Dispose(PFrame(Attack[i].FrameList.Items[j]));
    end;
    Attack[i].FrameList.Free;
   end;
   Death.FrameList.free;
  end;
  LstSkinPart.DeleteSelected;
  Dispose(Skin);
 end;
end;



function GenerateDeathSuffix(const i:integer):string;
var Sec:string;
begin
 Sec:='';
 if (i div 26) >0 then
  Sec:=chr((i div 26)+48);
 result:='c-'+chr((i mod 26)+97)+sec;
end;

Function GenerateWalkSuffix(const Dir,i:integer):string;
var Sec:string;
begin
 Sec:='';
 if (i div 26) >0 then
  Sec:=chr((i div 26)+48);
 case Dir of
  0:Result:='000-'+chr((i mod 26)+97)+sec;
  1:Result:='045-'+chr((i mod 26)+97)+sec;
  2:Result:='090-'+chr((i mod 26)+97)+sec;
  3:Result:='135-'+chr((i mod 26)+97)+sec;
  4:Result:='180-'+chr((i mod 26)+97)+sec;
 end;
end;

procedure TFrmPuppetSkinEdit.AutoFill1Click(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
    BaseName:string;
    SpName:string;
    Found:boolean;
    Counter:longint;
    Sprite:PSprite;
    Frame,CpyFrame:PFrame;
    Direction,i,CpyDir:integer;
begin //
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  
  BaseName:=LowerCase(InputBox('Base Sprite Name','Name: ',''));
  if BaseName='' then exit;

  //Walk sprites

  for Direction:=0 to 4 do
  begin
   Counter:=0;
   repeat
    SpName:=BaseName+GenerateWalkSuffix(Direction,Counter);
    Sprite:=Index.SpriteHash.SearchByName(SpName);
    if Sprite=nil then
    begin
     Sprite:=Index.SpriteHash.SearchByName('blanksprite');
     SpName:='blanksprite';
    end;

     new(Frame);
     Frame^.SpriteName:=SpName;
     Frame^.Offx:=0;
     Frame^.Offy:=0;
     GetSpriteOffset(SpName,Frame^.Offx,Frame^.Offy,0);
     Skin^.Walk[Direction].FrameList.Add(Frame);
     inc(Counter);

   until Counter>MaxAutoFillWalk;
  end;

  //we need to copy the frame for the reversed direction
  if counter>0 then
  for Direction:=5 to 7 do
  begin
   CpyDir:=3-(Direction-5);//take the related normal direction
   //copy each frame, and take the good offset
   for i:=0 to Skin^.Walk[CpyDir].FrameList.Count-1 do
   begin
    CpyFrame:=Skin^.Walk[CpyDir].FrameList.Items[i];
    new(frame);
    Frame^.SpriteName:=CpyFrame^.SpriteName;
    GetSpriteOffset(Frame^.SpriteName,Frame^.Offx,Frame^.Offy,1);
    Skin^.Walk[Direction].FrameList.Add(Frame);
   end;
  end;

  //Attack sprites

  for Direction:=0 to 4 do
  begin
   Counter:=0;
   repeat
    SpName:=BaseName+'a'+GenerateWalkSuffix(Direction,Counter);
    Sprite:=Index.SpriteHash.SearchByName(SpName);
    if Sprite=nil then
    begin
     Sprite:=Index.SpriteHash.SearchByName('blanksprite');
     SpName:='blanksprite';
    end;

     new(Frame);
     Frame^.SpriteName:=SpName;
     Frame^.Offx:=0;
     Frame^.Offy:=0;
     GetSpriteOffset(SpName,Frame^.Offx,Frame^.Offy,0);
     Skin^.Attack[Direction].FrameList.Add(Frame);
     inc(Counter);

   until Counter>MaxAutoFillAttack;
  end;

   //we need to copy the frame for the reversed direction
  if counter>0 then
  for Direction:=5 to 7 do
  begin
   CpyDir:=3-(Direction-5);//take the related normal direction
   //copy each frame, and take the good offset
   for i:=0 to Skin^.Attack[CpyDir].FrameList.Count-1 do
   begin
    CpyFrame:=Skin^.Attack[CpyDir].FrameList.Items[i];
    new(frame);
    Frame^.SpriteName:=CpyFrame^.SpriteName;
    GetSpriteOffset(Frame^.SpriteName,Frame^.Offx,Frame^.Offy,1);
    Skin^.Attack[Direction].FrameList.Add(Frame);
   end;
  end;

  //range sprites

  for Direction:=0 to 4 do
  begin
   Counter:=0;
   repeat
    SpName:=BaseName+'b'+GenerateWalkSuffix(Direction,Counter);
    Sprite:=Index.SpriteHash.SearchByName(SpName);
    if Sprite=nil then
    begin
     Sprite:=Index.SpriteHash.SearchByName('blanksprite');
     SpName:='blanksprite';
    end;

     new(Frame);
     Frame^.SpriteName:=SpName;
     Frame^.Offx:=0;
     Frame^.Offy:=0;
     GetSpriteOffset(SpName,Frame^.Offx,Frame^.Offy,0);
     Skin^.Range[Direction].FrameList.Add(Frame);
     inc(Counter);
   until Counter>MaxAutoFillRange;
  end;

   //we need to copy the frame for the reversed direction
  if counter>0 then
  for Direction:=5 to 7 do
  begin
   CpyDir:=3-(Direction-5);//take the related normal direction
   //copy each frame, and take the good offset
   for i:=0 to Skin.Range[CpyDir].FrameList.Count-1 do
   begin
    CpyFrame:=Skin.Range[CpyDir].FrameList.Items[i];
    new(frame);
    Frame.SpriteName:=CpyFrame.SpriteName;
    GetSpriteOffset(Frame.SpriteName,Frame.Offx,Frame.Offy,1);
    Skin.Range[Direction].FrameList.Add(Frame);
   end;
  end;

  //death sprites
  Found:=true;
  Counter:=0;
  repeat
   SpName:=BaseName+GenerateDeathSuffix(Counter);
   Sprite:=Index.SpriteHash.SearchByName(SpName);
   if Sprite<>nil then
   begin
    new(Frame);
    Frame.SpriteName:=SpName;
    Frame.Offx:=0;
    Frame.Offy:=0;
    GetSpriteOffset(SpName,Frame.Offx,Frame.Offy,0);
    Skin.Death.FrameList.Add(Frame);
   end else
   begin
    Found:=false;
   end;

   inc(Counter);
  until Found=false;

 end;

 RefreshView;
end;

procedure TFrmPuppetSkinEdit.LstSkinPartClick(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  EdtSkinName.Text:=Skin.SkinName;
  EdtSkinId.Text:=IntToStr(Skin.SkinId);
  EdtAnimDelay.Text:=IntToStr(round(Skin.AnimationDelay*1000));
  EdtVtxColor.Text:=IntToHex(Skin.VertexColor,8);
  ComboColorFx.ItemIndex:=Skin.ColorFx;
  ChkFemale.Checked:=Skin.Female;
  RefreshView;
 end;
end;

procedure TFrmPuppetSkinEdit.LstActionClick(Sender: TObject);
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  RefreshView;
 end;
end;

procedure TFrmPuppetSkinEdit.LstDirClick(Sender: TObject);
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  RefreshView;
 end;
end;

procedure TFrmPuppetSkinEdit.RefreshView;
var Skin:PPuppetSkinPartInfo;
    Frame:PFrame;
    i:longint;
begin
 LstSprite.Items.BeginUpdate;
 LstSprite.Items.Clear;
 
 if LstSkinPart.ItemIndex>=0 then
  if LstAction.ItemIndex>=0 then
   if LstDir.ItemIndex>=0 then
   begin
    Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
    case LstAction.ItemIndex of
     0:begin
        ChkReverse.Checked:=Boolean(Skin.Walk[LstDir.ItemIndex].Reversed);
        for i:=0 to Skin.Walk[LstDir.ItemIndex].FrameList.Count-1 do
        begin
         Frame:=Skin.Walk[LstDir.ItemIndex].FrameList.Items[i];
         LstSprite.Items.AddObject(Frame.SpriteName,TObject(Frame));
        end;
       end;
     1:begin
        ChkReverse.Checked:=Boolean(Skin.Attack[LstDir.ItemIndex].Reversed);
        for i:=0 to Skin.Attack[LstDir.ItemIndex].FrameList.Count-1 do
        begin
         Frame:=Skin.Attack[LstDir.ItemIndex].FrameList.Items[i];
         LstSprite.Items.AddObject(Frame.SpriteName,TObject(Frame));
        end;
       end;
     2:begin
        ChkReverse.Checked:=boolean(Skin.Death.Reversed);
        for i:=0 to Skin.Death.FrameList.Count-1 do
        begin
         Frame:=Skin.Death.FrameList.Items[i];
         LstSprite.Items.AddObject(Frame.SpriteName,TObject(Frame));
        end;
       end;
     3:begin
        ChkReverse.Checked:=Boolean(Skin.Range[LstDir.ItemIndex].Reversed);
        for i:=0 to Skin.Range[LstDir.ItemIndex].FrameList.Count-1 do
        begin
         Frame:=Skin.Range[LstDir.ItemIndex].FrameList.Items[i];
         LstSprite.Items.AddObject(Frame.SpriteName,TObject(Frame));
        end;
       end;
    end;
   end;
 LstSprite.Items.EndUpdate;
end;

procedure TFrmPuppetSkinEdit.EdtAnimDelayChange(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  if EdtAnimDelay.Text<>'' then
   Skin^.AnimationDelay:=StrToInt(EdtAnimDelay.Text)/1000;
 end;
end;


procedure TFrmPuppetSkinEdit.EdtNameChange(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  Skin^.SkinName:=EdtSkinName.Text;
  LstSkinPart.Items[LstSkinPart.ItemIndex]:=Skin^.SkinName+':'+IntToStr(Skin.SkinId);
 end;
end;

procedure TFrmPuppetSkinEdit.EdtSkinIdChange(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  Skin.SkinId:=StrToInt(EdtSkinId.Text);
  LstSkinPart.Items[LstSkinPart.ItemIndex]:=Skin^.SkinName+':'+IntToStr(Skin.SkinId);
 end;
end;

procedure TFrmPuppetSkinEdit.ChkFemaleClick(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  Skin^.Female:=ChkFemale.Checked;
 end;
end;


procedure TFrmPuppetSkinEdit.ComboColorFxChange(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  Skin^.ColorFx:=ComboColorFx.ItemIndex;
 end;
end;

procedure TFrmPuppetSkinEdit.EdtVtxColorChange(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
begin
 if LstSkinPart.ItemIndex>=0 then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  Skin^.VertexColor:=HexToInt(EdtVtxColor.Text);
 end;
end;


procedure TFrmPuppetSkinEdit.LstSpriteClick(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
    Rev:Cardinal;
    Frame:PFrame;
    Sprite:PSprite;
    Surface:PCardinal;
begin
 if (LstSkinPart.ItemIndex>=0) and (LstSprite.ItemIndex>=0) then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  if LstAction.ItemIndex>=0 then
   if LstDir.ItemIndex>=0 then
   begin
    case LstAction.ItemIndex of
     0:begin
        Frame:=Skin^.Walk[LstDir.ItemIndex].FrameList[LstSprite.ItemIndex];
        Rev:=Skin^.Walk[LstDir.ItemIndex].Reversed;
       end;
     1:begin
        Frame:=Skin^.Attack[LstDir.ItemIndex].FrameList[LstSprite.ItemIndex];
        Rev:=Skin^.Attack[LstDir.ItemIndex].Reversed;
       end;
     2:begin
        Frame:=Skin^.Death.FrameList[LstSprite.ItemIndex];
        Rev:=Skin^.Death.Reversed;
       end;
     3:begin
        Frame:=Skin^.Range[LstDir.ItemIndex].FrameList[LstSprite.ItemIndex];
        Rev:=Skin^.Range[LstDir.ItemIndex].Reversed;
       end;
    end;
    EdtOffX.Text:=IntToStr(Frame^.Offx);
    EdtOffY.Text:=IntToStr(Frame^.Offy);
    //get the sprite
    Sprite:=nil;
    if Index.SpriteHash<>nil then
     Sprite:=Index.SpriteHash.SearchByName(Frame^.SpriteName);
    if Sprite=nil then
     exit;
    Surface:=GetSpriteA8R8G8B8Surface(Sprite);
    DrawA8R8G8B8(Surface,Sprite^.Width,Sprite^.Height,Frame^.Offx-16,Frame^.Offy-8,Rev,1,Skin^.VertexColor,Skin^.ColorFx,View);
    FreeMem(Surface);
   end;
 end;
end;



procedure TFrmPuppetSkinEdit.ClearList1Click(Sender: TObject);
var Skin:PPuppetSkinPartInfo;
    Rev:Cardinal;
    Frame:PFrame;
    Sprite:PSprite;
    Surface:PCardinal;
begin
 if (LstSkinPart.ItemIndex>=0) then
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  if LstAction.ItemIndex>=0 then
   if LstDir.ItemIndex>=0 then
   begin
    //TODO Dispose the frame !!!!!!
     case LstAction.ItemIndex of
     0:begin
        Skin^.Walk[LstDir.ItemIndex].FrameList.Clear;
       end;
     1:begin
        Skin^.Attack[LstDir.ItemIndex].FrameList.Clear;
       end;
     2:begin
        Skin^.Death.FrameList.Clear;
       end;
     end;
   end;
 end;
 RefreshView;
end;

procedure TFrmPuppetSkinEdit.LstSpriteDragDrop(Sender, Source: TObject; X, Y: Integer);
var TempList:TList;
    i:integer;
    Tree:TTreeView;
    Frame:PFrame;
    Skin:PPuppetSkinPartInfo;
    src,dst:integer;
begin //drag a list of graph from the graph editor

 if Source is TTreeView then
 begin
 Tree:=TTreeView(Source);
 if (Tree.SelectionCount>0) and (LstSkinPart.ItemIndex>=0 ) and (LstAction.ItemIndex>=0) then    //we got something selected (sanity check)
 begin
  Skin:=PPuppetSkinPartInfo(LstSkinPart.Items.Objects[LstSkinPart.ItemIndex]);
  for i:=0 to Tree.SelectionCount-1 do
  begin
   if Tree.Selections[i].ImageIndex=1 then  //reject directory
   begin
    new(Frame);
    Frame.SpriteName:=Tree.Selections[i].Text;
    Frame.Offx:=0;
    Frame.Offy:=0;
    case LstAction.ItemIndex of
     0:begin
        Skin.Walk[LstDir.ItemIndex].FrameList.Add(Frame);
        GetSpriteOffset(Frame.SpriteName,Frame.Offx,Frame.Offy,Skin.Walk[LstDir.ItemIndex].Reversed);
       end;
     1:begin
        Skin.Attack[LstDir.ItemIndex].FrameList.Add(Frame);
        GetSpriteOffset(Frame.SpriteName,Frame.Offx,Frame.Offy,Skin.Attack[LstDir.ItemIndex].Reversed);
       end;
     2:begin
        Skin.Death.FrameList.Add(Frame);
        GetSpriteOffset(Frame.SpriteName,Frame.Offx,Frame.Offy,Skin.Death.Reversed);
       end;
     3:begin
        Skin.Range[LstDir.ItemIndex].FrameList.Add(Frame);
        GetSpriteOffset(Frame.SpriteName,Frame.Offx,Frame.Offy,Skin.Range[LstDir.ItemIndex].Reversed);
       end;
    end;
   end;
  end;
  RefreshView;
 end;
 end else
 if Source is TListbox then
 begin
  Src:=LstSprite.ItemIndex;
  dst:=LstSprite.ItemAtPos(Point(x,y),true);

  if (src>0) and (src<LstSprite.Items.Count) then
  begin
   LstSprite.Items.Exchange(src,dst);
   case LstAction.ItemIndex of
     0:begin
        Skin.Walk[LstDir.ItemIndex].FrameList.Exchange(src,dst);
       end;
     1:begin
        Skin.Attack[LstDir.ItemIndex].FrameList.Exchange(src,dst);

       end;
     2:begin
        Skin.Death.FrameList.Exchange(src,dst);

       end;
     3:begin
        Skin.Range[LstDir.ItemIndex].FrameList.Exchange(src,dst);
       end;
   end;
  end;
 end;
end;

procedure TFrmPuppetSkinEdit.LstSpriteDragOver(Sender, Source: TObject; X, Y: Integer; State: TDragState; var Accept: Boolean);
var Tree:TTreeView;
begin
 Accept:=false;
 if Source is TTreeView then
 begin
  Tree:=TTreeView(Source);
  if Tree.Tag=2 then
   Accept:=true;
 end else
 if Source is TListbox then
 begin
   if TListBox(Source).Tag=30 then
    Accept:=true;
 end;
end;

end.

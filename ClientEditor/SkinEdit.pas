unit SkinEdit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Menus, MGR32_Image;

type
  TFrmSkinEdit = class(TForm)
    LstSkin: TListBox;
    Label1: TLabel;
    EdtName: TEdit;
    Label2: TLabel;
    EdtId: TEdit;
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
    PopupSkinList: TPopupMenu;
    AddSkin1: TMenuItem;
    Duplicateskin1: TMenuItem;
    N1: TMenuItem;
    DeleteSkin1: TMenuItem;
    Label4: TLabel;
    ComboColorFx: TComboBox;
    Label10: TLabel;
    EdtMonsterSize: TEdit;
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
    procedure SAveMonsters1Click(Sender: TObject);
    procedure LoadMonsterSkins1Click(Sender: TObject);
    procedure AddSkin1Click(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure DeleteSkin1Click(Sender: TObject);
    procedure EdtNameChange(Sender: TObject);
    procedure EdtIdChange(Sender: TObject);
    procedure LstSkinClick(Sender: TObject);
    procedure EdtVtxColorChange(Sender: TObject);
    procedure EdtAnimDelayChange(Sender: TObject);
    procedure ComboColorFxChange(Sender: TObject);
    procedure LstActionClick(Sender: TObject);
    procedure LstDirClick(Sender: TObject);
    Procedure RefreshView;
    procedure LstSpriteDragOver(Sender, Source: TObject; X, Y: Integer;
      State: TDragState; var Accept: Boolean);
    procedure LstSpriteDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure LstSpriteClick(Sender: TObject);
    procedure EdtMonsterSizeChange(Sender: TObject);
    procedure AutoFill1Click(Sender: TObject);
    procedure ClearList1Click(Sender: TObject);
    procedure Sort1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  FrmSkinEdit: TFrmSkinEdit;

implementation

{$R *.dfm}
uses comctrls,FastStream,udda,IdTypes,globals;

procedure TFrmSkinEdit.FormResize(Sender: TObject);
begin
 LstSkin.Height:=(FrmSkinEdit.ClientHeight-LstSkin.Top)-4;
 LstSprite.Height:=(FrmSkinEdit.ClientHeight-LstSprite.Top)-4;
 GroupBox1.Top:=(FrmSkinEdit.ClientHeight-GroupBox1.Height)-4;

 View.Width:=(FrmSkinEdit.ClientWidth-View.Left)-4;
 View.Height:=(FrmSkinEdit.ClientHeight-View.Top)-4;
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

procedure TFrmSkinEdit.LoadMonsterSkins1Click(Sender: TObject);
var i,j,k,Count,Version:longint;
    Fst:TFastStream;
    Skin:PMonsterSkinInfo;
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
    VertexColor:=Fst.ReadLong;
    ColorFx:=Fst.ReadLong;
    AnimationDelay:=Fst.ReadSingle;
    MonsterSize:=Fst.ReadSingle;

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

    Death.Reversed:=Fst.ReadLong;
    Death.GraphCount:=Fst.ReadLong;
    Death.FrameList:=Tlist.Create;
    for k:=0 to Death.GraphCount-1 do
    begin
     new(Frame);
     ReadFrameFromStream(Fst,Frame);
     Death.FrameList.Add(Frame);
    end;


    AtkSoundCount:=Fst.ReadLong;
    AtkSounds:=TList.Create;
    for j:=0 to AtkSoundCount-1 do
    begin
     new(Sound);
     ReadSoundFromStream(Fst,Sound);
     AtkSounds.Add(Sound);
    end;

    HitSoundCount:=Fst.ReadLong;
    HitSounds:=TList.Create;
    for j:=0 to HitSoundCount-1 do
    begin
     new(Sound);
     ReadSoundFromStream(Fst,Sound);
     HitSounds.Add(Sound);
    end;

    DieSoundCount:=Fst.ReadLong;
    DieSounds:=TList.Create;
    for j:=0 to DieSoundCount-1 do
    begin
     new(Sound);
     ReadSoundFromStream(Fst,Sound);
     DieSounds.Add(Sound);
    end;

    IdleSoundCount:=Fst.ReadLong;
    IdleSounds:=TList.Create;
    for j:=0 to IdleSoundCount-1 do
    begin
     new(Sound);
     ReadSoundFromStream(Fst,Sound);
     IdleSounds.Add(Sound);
    end;
   end;
   LstSkin.AddItem(Skin^.SkinName+' : '+IntToStr(Skin^.SkinId),TObject(Skin));
  end;
  Fst.Free;
 end;
end;



procedure TFrmSkinEdit.SaveMonsters1Click(Sender: TObject);
var i,j,k,Count,Version:longint;
    Fst:TFastStream;
    Skin:PMonsterSkinInfo;
begin
 DlgSave.FileName:=DlgOpen.FileName;
 if DlgSave.Execute then
 begin
  Fst:=TFastStream.Create;

  Version:=1;
  Fst.WriteLong(Version);
  Count:=LstSkin.Count;
  Fst.WriteLong(Count);
  for i:=0 to Count-1 do
  begin
   Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[i]);
   with Skin^ do
   begin
    Fst.WriteWordString(SkinName);
    Fst.WriteWord(SkinId);
    Fst.WriteLong(VertexColor);
    Fst.WriteLong(ColorFx);
    Fst.WriteSingle(AnimationDelay);
    Fst.WriteSingle(MonsterSize);
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
    Fst.WriteLong(Death.Reversed);
    Death.GraphCount:=Death.FrameList.Count;
    Fst.WriteLong(Death.GraphCount);
    for k:=0 to Death.GraphCount-1 do
    begin
      WriteFrameToStream(Fst,Death.FrameList[k]);
    end;

    AtkSounds.Pack;
    AtkSoundCount:=AtkSounds.Count;
    Fst.WriteLong(AtkSoundCount);
    for j:=0 to AtkSoundCount-1 do
    begin
     WriteSoundToStream(Fst,AtkSounds.Items[j]);
    end;

    HitSounds.Pack;
    HitSoundCount:=HitSounds.Count;
    Fst.WriteLong(HitSoundCount);
    for j:=0 to HitSoundCount-1 do
    begin
     WriteSoundToStream(Fst,HitSounds.Items[j]);
    end;

    DieSounds.Pack;
    DieSoundCount:=DieSounds.Count;
    Fst.WriteLong(DieSoundCount);
    for j:=0 to DieSoundCount-1 do
    begin
     WriteSoundToStream(Fst,DieSounds.Items[j]);
    end;

    IdleSounds.Pack;
    IdleSoundCount:=IdleSounds.Count;
    Fst.WriteLong(IdleSoundCount);
    for j:=0 to IdleSoundCount-1 do
    begin
     WriteSoundToStream(Fst,IdleSounds.Items[j]);
    end;
   end;
  end;

  Fst.WriteToFile(DlgSave.FileName);
  Fst.Free;
 end;
end;

procedure TFrmSkinEdit.Sort1Click(Sender: TObject);
begin
 LstSkin.Sorted:=LstSkin.Sorted xor true;
 TMenuItem(sender).Checked:=LstSkin.Sorted;
end;

procedure TFrmSkinEdit.AddSkin1Click(Sender: TObject);
var Skin:PMonsterSkinInfo;
    i:longint;
begin
 New(Skin);
 with Skin^ do
 begin
  SkinName:='A New Skin';
  SkinId:=0;
  VertexColor:=$FFFFFFFF;
  ColorFx:=0;
  AnimationDelay:=0.033;
  MonsterSize:=1;
  for i:=0 to 7 do
  begin
   Walk[i].Reversed:=0;
   Walk[i].GraphCount:=0;
   Walk[i].FrameList:=TList.Create;
   Attack[i].Reversed:=0;
   Attack[i].GraphCount:=0;
   Attack[i].FrameList:=TList.Create;
  end;

  for i:=5 to 7 do
  begin
   Walk[i].Reversed:=1;
   Attack[i].Reversed:=1;
  end;
  
  Death.Reversed:=0;
  Death.GraphCount:=0;
  Death.FrameList:=TList.Create;

  AtkSoundCount:=0;
  AtkSounds:=TList.Create;
  HitSoundCount:=0;
  HitSounds:=TList.Create;
  DieSoundCount:=0;
  DieSounds:=TList.Create;
  IdleSoundCount:=0;
  IdleSounds:=TList.Create;
 end;
 LstSkin.AddItem(Skin^.SkinName+' : '+IntToStr(Skin^.SkinId),TObject(Skin));
end;



procedure TFrmSkinEdit.DeleteSkin1Click(Sender: TObject);
var Skin:PMonsterSkinInfo;
    i,j:integer;
begin
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
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
   for i:=0 to AtkSounds.Count-1 do
    Dispose(PSoundInfo(AtkSounds.Items[i]));
   AtkSounds.Free;
   for i:=0 to HitSounds.Count-1 do
    Dispose(PSoundInfo(HitSounds.Items[i]));
   HitSounds.Free;
   for i:=0 to DieSounds.Count-1 do
    Dispose(PSoundInfo(DieSounds.Items[i]));
   DieSounds.Free;
   for i:=0 to IdleSounds.Count-1 do
    Dispose(PSoundInfo(IdleSounds.Items[i]));
   IdleSounds.Free;
  end;
  LstSkin.DeleteSelected;
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

procedure TFrmSkinEdit.AutoFill1Click(Sender: TObject);
var Skin:PMonsterSkinInfo;
    BaseName:string;
    SpName:string;
    Found:boolean;
    Counter:longint;
    Sprite:PSprite;
    Frame,CpyFrame:PFrame;
    Direction,i,CpyDir:integer;
begin //
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  
  BaseName:=LowerCase(InputBox('Base Sprite Name','Name: ',''));
  if BaseName='' then exit;

  //Walk sprites

  for Direction:=0 to 4 do
  begin
   Found:=true;
   Counter:=0;
   repeat
    SpName:=BaseName+GenerateWalkSuffix(Direction,Counter);
    Sprite:=Index.SpriteHash.SearchByName(SpName);
    if Sprite<>nil then
    begin
     new(Frame);
     Frame^.SpriteName:=SpName;
     Frame^.Offx:=0;
     Frame^.Offy:=0;
     GetSpriteOffset(SpName,Frame^.Offx,Frame^.Offy,0);
     Skin^.Walk[Direction].FrameList.Add(Frame);
     inc(Counter);
    end else
    begin
     Found:=false;
    end;


   until Found=false;
   if counter=0 then break;
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
   Found:=true;
   Counter:=0;
   repeat
    SpName:=BaseName+'a'+GenerateWalkSuffix(Direction,Counter);
    Sprite:=Index.SpriteHash.SearchByName(SpName);
    if Sprite<>nil then
    begin
     new(Frame);
     Frame^.SpriteName:=SpName;
     Frame^.Offx:=0;
     Frame^.Offy:=0;
     GetSpriteOffset(SpName,Frame^.Offx,Frame^.Offy,0);
     Skin^.Attack[Direction].FrameList.Add(Frame);
     inc(Counter);
    end else
    begin
     Found:=false;
    end;


   until Found=false;
   if counter=0 then break;
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

  //death sprites
  Found:=true;
  Counter:=0;
  repeat
   SpName:=BaseName+GenerateDeathSuffix(Counter);
   Sprite:=Index.SpriteHash.SearchByName(SpName);
   if Sprite<>nil then
   begin
    new(Frame);
    Frame^.SpriteName:=SpName;
    Frame^.Offx:=0;
    Frame^.Offy:=0;
    GetSpriteOffset(SpName,Frame^.Offx,Frame^.Offy,0);
    Skin^.Death.FrameList.Add(Frame);
   end else
   begin
    Found:=false;
   end;

   inc(Counter);
  until Found=false;

 end;

 RefreshView;
end;

procedure TFrmSkinEdit.LstSkinClick(Sender: TObject);
var Skin:PMonsterSkinInfo;
begin
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  EdtName.Text:=Skin^.SkinName;
  EdtId.Text:=IntToStr(Skin^.SkinId);
  EdtAnimDelay.Text:=IntToStr(round(Skin^.AnimationDelay*1000));
  EdtVtxColor.Text:=IntToHex(Skin^.VertexColor,8);
  ComboColorFx.ItemIndex:=Skin^.ColorFx;
  EdtMonsterSize.Text:=IntToStr(round(Skin^.MonsterSize*100));
  RefreshView;
 end;
end;

procedure TFrmSkinEdit.LstActionClick(Sender: TObject);
begin
 if LstSkin.ItemIndex>=0 then
 begin
  RefreshView;
 end;
end;

procedure TFrmSkinEdit.LstDirClick(Sender: TObject);
begin
 if LstSkin.ItemIndex>=0 then
 begin
  RefreshView;
 end;
end;

procedure TFrmSkinEdit.RefreshView;
var Skin:PMonsterSkinInfo;
    Frame:PFrame;
    i:longint;
begin
 LstSprite.Items.BeginUpdate;
 LstSprite.Items.Clear;
 
 if LstSkin.ItemIndex>=0 then
  if LstAction.ItemIndex>=0 then
   if LstDir.ItemIndex>=0 then
   begin
    Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
    case LstAction.ItemIndex of
     0:begin
        ChkReverse.Checked:=Boolean(Skin^.Walk[LstDir.ItemIndex].Reversed);
        for i:=0 to Skin^.Walk[LstDir.ItemIndex].FrameList.Count-1 do
        begin
         Frame:=Skin^.Walk[LstDir.ItemIndex].FrameList.Items[i];
         LstSprite.Items.AddObject(Frame.SpriteName,TObject(Frame));
        end;
       end;
     1:begin
        ChkReverse.Checked:=Boolean(Skin^.Attack[LstDir.ItemIndex].Reversed);
        for i:=0 to Skin^.Attack[LstDir.ItemIndex].FrameList.Count-1 do
        begin
         Frame:=Skin^.Attack[LstDir.ItemIndex].FrameList.Items[i];
         LstSprite.Items.AddObject(Frame.SpriteName,TObject(Frame));
        end;
       end;
     2:begin
        ChkReverse.Checked:=boolean(Skin^.Death.Reversed);
        for i:=0 to Skin^.Death.FrameList.Count-1 do
        begin
         Frame:=Skin^.Death.FrameList.Items[i];
         LstSprite.Items.AddObject(Frame.SpriteName,TObject(Frame));
        end;
       end;
    end;
   end;
 LstSprite.Items.EndUpdate;
end;

procedure TFrmSkinEdit.EdtAnimDelayChange(Sender: TObject);
var Skin:PMonsterSkinInfo;
begin
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  if EdtAnimDelay.Text<>'' then
   Skin^.AnimationDelay:=StrToInt(EdtAnimDelay.Text)/1000;
 end;
end;

procedure TFrmSkinEdit.EdtIdChange(Sender: TObject);
var Skin:PMonsterSkinInfo;
begin
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  if EdtId.Text<>'' then
   Skin^.SkinId:=StrToInt(EdtId.Text);
  LstSkin.Items[LstSkin.ItemIndex]:=Skin^.SkinName+' : '+IntToStr(Skin^.SkinId);
 end;
end;

procedure TFrmSkinEdit.EdtMonsterSizeChange(Sender: TObject);
var Skin:PMonsterSkinInfo;
begin
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  if EdtMonsterSize.Text<>'' then
   Skin^.MonsterSize:=StrToInt(EdtMonsterSize.Text)/100;
 end;
end;

procedure TFrmSkinEdit.EdtNameChange(Sender: TObject);
var Skin:PMonsterSkinInfo;
begin
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  Skin^.SkinName:=EdtName.Text;
  LstSkin.Items[LstSkin.ItemIndex]:=Skin^.SkinName+' : '+IntToStr(Skin^.SkinId);
 end;
end;





procedure TFrmSkinEdit.ComboColorFxChange(Sender: TObject);
var Skin:PMonsterSkinInfo;
begin
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  Skin^.ColorFx:=ComboColorFx.ItemIndex;
 end;
end;

procedure TFrmSkinEdit.EdtVtxColorChange(Sender: TObject);
var Skin:PMonsterSkinInfo;
begin
 if LstSkin.ItemIndex>=0 then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  Skin^.VertexColor:=HexToInt(EdtVtxColor.Text);
 end;
end;


procedure TFrmSkinEdit.LstSpriteClick(Sender: TObject);
var Skin:PMonsterSkinInfo;
    Rev:Cardinal;
    Frame:PFrame;
    Sprite:PSprite;
    Surface:PCardinal;
begin
 if (LstSkin.ItemIndex>=0) and (LstSprite.ItemIndex>=0) then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
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

procedure TFrmSkinEdit.ClearList1Click(Sender: TObject);
var Skin:PMonsterSkinInfo;
    Rev:Cardinal;
    Frame:PFrame;
    Sprite:PSprite;
    Surface:PCardinal;
begin
 if (LstSkin.ItemIndex>=0) then
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
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

procedure TFrmSkinEdit.LstSpriteDragDrop(Sender, Source: TObject; X, Y: Integer);
var TempList:TList;
    i:integer;
    Tree:TTreeView;
    Frame:PFrame;
    Skin:PMonsterSkinInfo;
begin //drag a list of graph from the graph editor
 Tree:=TTreeView(Source);
 if (Tree.SelectionCount>0) and (LstSkin.ItemIndex>=0 ) and (LstAction.ItemIndex>=0) then    //we got something selected (sanity check)
 begin
  Skin:=PMonsterSkinInfo(LstSkin.Items.Objects[LstSkin.ItemIndex]);
  for i:=0 to Tree.SelectionCount-1 do
  begin
   if Tree.Selections[i].ImageIndex=1 then  //reject directory
   begin
    new(Frame);
    Frame^.SpriteName:=Tree.Selections[i].Text;
    Frame^.Offx:=0;
    Frame^.Offy:=0;
    case LstAction.ItemIndex of
     0:begin
        Skin^.Walk[LstDir.ItemIndex].FrameList.Add(Frame);
        GetSpriteOffset(Frame^.SpriteName,Frame^.Offx,Frame^.Offy,Skin^.Walk[LstDir.ItemIndex].Reversed);
       end;
     1:begin
        Skin^.Attack[LstDir.ItemIndex].FrameList.Add(Frame);
        GetSpriteOffset(Frame^.SpriteName,Frame^.Offx,Frame^.Offy,Skin^.Attack[LstDir.ItemIndex].Reversed);
       end;
     2:begin
        Skin^.Death.FrameList.Add(Frame);
        GetSpriteOffset(Frame^.SpriteName,Frame^.Offx,Frame^.Offy,Skin^.Death.Reversed);
       end;
    end;
   end;
  end;
  RefreshView;
 end;
end;

procedure TFrmSkinEdit.LstSpriteDragOver(Sender, Source: TObject; X, Y: Integer; State: TDragState; var Accept: Boolean);
var Tree:TTreeView;
begin
 Accept:=false;
 if Source is TTreeView then
 begin
  Tree:=TTreeView(Source);
  if Tree.Tag=2 then
   Accept:=true else
   Accept:=false;
 end;
end;

end.

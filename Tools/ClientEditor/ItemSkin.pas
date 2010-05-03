unit ItemSkin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Menus, MGR32_Image,IdTypes,FastStream;

type
  TFrmItemSkinEd = class(TForm)
    LstItemSkin: TListBox;
    Label1: TLabel;
    MainMenu1: TMainMenu;
    Files1: TMenuItem;
    LoadItemSkins1: TMenuItem;
    SaveItemSkins1: TMenuItem;
    EdtSkinId: TEdit;
    EdtSkinName: TEdit;
    Label2: TLabel;
    Label3: TLabel;
    LstElem: TListBox;
    Label4: TLabel;
    ComboType: TComboBox;
    ComboMouseCursor: TComboBox;
    Label5: TLabel;
    Label6: TLabel;
    View: TMPaintBox32;
    EdtSoundOpen: TEdit;
    EdtSoundClose: TEdit;
    Label7: TLabel;
    Label8: TLabel;
    PopupSkinList: TPopupMenu;
    AddNewID1: TMenuItem;
    N1: TMenuItem;
    DuplicateID1: TMenuItem;
    N2: TMenuItem;
    DeleteID1: TMenuItem;
    EdtColorMod: TEdit;
    ComboColorFx: TComboBox;
    Label9: TLabel;
    Label10: TLabel;
    EdtAnimDelay: TEdit;
    Label11: TLabel;
    Help1: TMenuItem;
    DlgOpen: TOpenDialog;
    DlgSave: TSaveDialog;
    N3: TMenuItem;
    ClearAll1: TMenuItem;
    PopupElem: TPopupMenu;
    DeleteSprite1: TMenuItem;
    N4: TMenuItem;
    ClearAllSprites1: TMenuItem;
    GroupBox1: TGroupBox;
    Label12: TLabel;
    EdtOffX: TEdit;
    Label13: TLabel;
    EdtOffY: TEdit;
    procedure FormResize(Sender: TObject);
    procedure AddNewID1Click(Sender: TObject);
    procedure LstItemSkinClick(Sender: TObject);
    procedure EdtSkinIdChange(Sender: TObject);
    procedure EdtSkinNameChange(Sender: TObject);
    procedure ComboTypeChange(Sender: TObject);
    procedure ComboMouseCursorChange(Sender: TObject);
    procedure ComboColorFxChange(Sender: TObject);
    procedure EdtAnimDelayChange(Sender: TObject);
    procedure EdtSoundOpenChange(Sender: TObject);
    procedure EdtSoundCloseChange(Sender: TObject);
    procedure LoadItemSkins1Click(Sender: TObject);
    procedure SaveItemSkins1Click(Sender: TObject);
    PRocedure RefreshSkinList;
    procedure DeleteID1Click(Sender: TObject);
    procedure EdtColorModChange(Sender: TObject);
    procedure LstElemDragOver(Sender, Source: TObject; X, Y: Integer;  State: TDragState; var Accept: Boolean);
    procedure LstElemDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure LstElemClick(Sender: TObject);
    Procedure FreeAllMemory;
    procedure ClearAll1Click(Sender: TObject);
    procedure DeleteSprite1Click(Sender: TObject);
    procedure ClearAllSprites1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  FrmItemSkinEd: TFrmItemSkinEd;

implementation

{$R *.dfm}
uses comctrls,udda,globals;

procedure TFrmItemSkinEd.FormResize(Sender: TObject);
begin
 LstItemSkin.Height:=(FrmItemSkinEd.ClientHeight-LstItemSkin.Top)-4;
 LstElem.Height:=(FrmItemSkinEd.ClientHeight-LstElem.Top)-4;
 View.Width:= (FrmItemSkinEd.ClientWidth-View.Left)-4;
 View.Height:= (FrmItemSkinEd.ClientHeight-View.Top)-4;
end;

procedure TFrmItemSkinEd.FreeAllMemory;
var Skin:PItemSkinInfo;
    Frame:PFrame;
    i,j:longint;
begin
 for i:=0 to LstItemSkin.Count-1 do
 begin
  Skin:=PItemSkinInfo(LstItemSkin.Items.Objects[i]);
  for j:=0 to Skin^.GraphInfo.Count-1 do
  begin
   Frame:=Skin^.GraphInfo.Items[j];
   dispose(frame);
  end;
  Dispose(Skin);
 end;

 LstItemSkin.Clear;
 LstElem.Clear;
end;

procedure TFrmItemSkinEd.LoadItemSkins1Click(Sender: TObject);
var Fst:TFastStream;
    i,j,Count,Version:longint;
    ItemSkin:PItemSkinInfo;
    Frame:PFrame;
begin
 If DlgOpen.Execute then
 begin
  LstItemSkin.Items.BeginUpdate;
  FreeAllMemory;

  Fst:=TFastStream.Create;
  Fst.LoadFromFile(DlgOpen.FileName);
  Version:=Fst.ReadLong;
  Count:=Fst.ReadLong;
  for i:=0 to Count-1 do
  begin
   new(ItemSkin);
   ItemSkin.SkinName:=Fst.ReadWordString;
   ItemSkin.SkinId:=fst.ReadWord;
   ItemSkin.SkinType:=fst.ReadLong;
   ItemSkin.MouseCursor:=Fst.ReadLong;
   ItemSkin.ColorFx:=Fst.ReadLong;
   ItemSkin.VertexColor:=Fst.ReadLong;
   ItemSkin.AnimationDelay:=Fst.ReadSingle;
   ItemSkin.OpenSound:=Fst.ReadWordString;
   ItemSkin.CloseSound:=Fst.ReadWordString;
   ItemSkin.GraphCount:=Fst.ReadLong;
   ItemSkin.GraphInfo:=TList.create;
   for j:=0 to ItemSkin.GraphCount-1 do
   begin
    new(Frame);
    Frame.SpriteName:=Fst.ReadWordString;
    Frame.Offx:=Fst.ReadWord;
    Frame.Offy:=Fst.ReadWord;
    ItemSkin.GraphInfo.Add(Frame);
   end;
   LstItemSkin.AddItem(ItemSkin^.SkinName+' : '+IntToStr(ItemSkin^.SkinId),TObject(ItemSkin));
  end;
  Fst.Free;
  LstItemSkin.Items.EndUpdate;
 end;
end;

procedure TFrmItemSkinEd.SaveItemSkins1Click(Sender: TObject);
var Fst:TFastStream;
    i,j,Count,Version:longint;
    ItemSkin:PItemSkinInfo;
    Frame:PFrame;
begin
 DlgSave.FileName:=DlgOpen.FileName;
 If DlgSave.Execute then
 begin
  Fst:=TFastStream.Create;
  Version:=1;
  Fst.WriteLong(Version);
  Count:=LstItemSkin.Count;
  Fst.WriteLong(Count);
  for i:=0 to Count-1 do
  begin
   ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[i]);
   Fst.WriteWordString(ItemSkin.SkinName);
   Fst.WriteWord(ItemSkin.SkinId);
   Fst.WriteLong(ItemSkin.SkinType);
   Fst.WriteLong(ItemSkin.MouseCursor);
   Fst.WriteLong(ItemSkin.ColorFx);
   Fst.WriteLong(ItemSkin.VertexColor);
   Fst.WriteSingle(ItemSkin.AnimationDelay);
   Fst.WriteWordString(ItemSkin.OpenSound);
   Fst.WriteWordString(ItemSkin.CloseSound);
   ItemSkin.GraphInfo.Pack; 
   ItemSkin.GraphCount:=ItemSkin.GraphInfo.Count;
   Fst.WriteLong(ItemSkin.GraphCount);
   for j:=0 to ItemSkin.GraphCount-1 do
   begin
    Frame:=PFrame(ItemSkin.GraphInfo.Items[j]);
    Fst.WriteWordString(Frame.SpriteName);
    Fst.WriteWord(Word(Frame.Offx));
    Fst.WriteWord(Word(Frame.Offy));
   end;
  end;
  Fst.WriteToFile(DlgSave.FileName);
  Fst.Free;
 end;
end;


procedure TFrmItemSkinEd.ClearAll1Click(Sender: TObject);
begin
 FreeAllMemory;
end;

procedure TFrmItemSkinEd.AddNewID1Click(Sender: TObject);
var NewSkin:PItemSkinInfo;
begin
 New(NewSkin);
 NewSkin.SkinName:='New Skin';
 NewSkin.SkinId:=0;
 NewSkin.VertexColor:=$FFFFFFFF;
 NewSkin.GraphCount:=0;
 NewSkin.AnimationDelay:=0.066;                     
 NewSkin.SkinType:=0;
 NewSkin.MouseCursor:=0;
 NewSkin.ColorFx:=0;
 NewSkin.GraphCount:=0;
 NewSkin.GraphInfo:=TList.Create;
 LstItemSkin.AddItem('0 : New Skin',TOBject(NewSkin));
end;

procedure TFrmItemSkinEd.DeleteID1Click(Sender: TObject);
var Skin:PItemSkinInfo;
    Frame:PFrame;
    i:longint;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  Skin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  for i:=0 to Skin.GraphInfo.Count-1 do
  begin
   Frame:=Skin.GraphInfo.Items[i];
   dispose(frame);
  end;
  LstItemSkin.DeleteSelected;
  LstElem.Clear;
  Skin.GraphInfo.Free;
  Dispose(Skin);
 end;
end;

procedure TFrmItemSkinEd.DeleteSprite1Click(Sender: TObject);
var Skin:PItemSkinInfo;
    Frame:PFrame;
begin
 if LstItemSkin.ItemIndex>=0 then
  if LstElem.ItemIndex>=0 then
  begin
   Skin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
   Frame:=PFrame(LstElem.Items.Objects[LstElem.ItemIndex]);
   Skin.GraphInfo.Remove(Frame);
   Skin.GraphInfo.Pack;
   Dispose(Frame);
   LstElem.DeleteSelected;
  end;
end;

procedure TFrmItemSkinEd.ClearAllSprites1Click(Sender: TObject);
var Skin:PItemSkinInfo;
    Frame:PFrame;
    i:longint;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  Skin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  for i:=0 to Skin.GraphInfo.Count-1 do
  begin
   Frame:=PFrame(Skin.GraphInfo.Items[i]);
   Dispose(Frame);
  end;
  LstElem.Clear;
  Skin.GraphInfo.Clear;
 end;
end;

procedure TFrmItemSkinEd.LstElemDragDrop(Sender, Source: TObject; X,   Y: Integer);
var TempList:TList;
    i:integer;
    Tree:TTreeView;
    Frame:PFrame;
    Skin:PItemSkinInfo;
begin //drag a list of graph from the graph editor
 Tree:=TTreeView(Source);
 if (Tree.SelectionCount>0) and (LstItemSkin.ItemIndex>=0 )then    //we got something selected (sanity check)
 begin
  Skin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  for i:=0 to Tree.SelectionCount-1 do
  begin
   if Tree.Selections[i].ImageIndex=1 then  //reject directory
   begin
    new(Frame);
    Frame^.SpriteName:=Tree.Selections[i].Text;
    Frame^.Offx:=0;
    Frame^.Offy:=0;
    GetSpriteOffset(Frame^.SpriteName,Frame^.Offx,Frame^.Offy,0);
    Skin.GraphInfo.Add(Frame);
    inc(Skin.GraphCount);
   end;
  end;
  LstItemSkinClick(nil);
 end;
end;

procedure TFrmItemSkinEd.LstElemDragOver(Sender, Source: TObject; X, Y: Integer;  State: TDragState; var Accept: Boolean);
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

procedure TFrmItemSkinEd.LstItemSkinClick(Sender: TObject);
var ItemSkin:PItemSkinInfo;
    Frame:PFrame;
    i:longint;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  EdtSkinName.Text:=ItemSkin.SkinName;
  EdtSkinId.Text:=IntToStr(ITemSkin.SkinId);
  ComboType.ItemIndex:=ITemSkin.SkinType;
  ComboMouseCursor.ItemIndex:=ITemSkin.MouseCursor;
  ComboColorFx.ItemIndex:=ITemSkin.ColorFx;
  EdtColorMod.Text:=IntToHex(ITemSkin.VertexColor,8);
  EdtAnimDelay.Text:=IntToStr(round(ITemSkin.AnimationDelay*1000));
  EdtSoundOpen.Text:=ITemSkin.OpenSound;
  EdtSoundClose.Text:=ItemSkin.CloseSound;

  LstElem.Clear;
  for i:=0 to ItemSkin.GraphInfo.Count-1 do
  begin
   Frame:=PFrame(ItemSkin.GraphInfo.Items[i]);
   LstElem.AddItem(Frame.SpriteName,TObject(Frame));
  end;
 end;
end;

procedure TFrmItemSkinEd.LstElemClick(Sender: TObject);
var Surface:PCardinal;
    Sprite:PSprite;
    ItemSkin:PItemSkinInfo;
    Frame:PFrame;
begin
 if LstItemSkin.ItemIndex>=0 then
  if LstElem.ItemIndex>=0 then
  begin
   ItemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
   Frame:=PFrame(LstElem.Items.Objects[LstElem.ItemIndex]);
   EdtOffX.Text:=IntToStr(Frame^.Offx);
   EdtOffy.Text:=IntToStr(Frame^.Offy);
   Sprite:=Index.SpriteHash.SearchByName(Frame^.SpriteName);
    if Sprite=nil then
     exit;
   Surface:=GetSpriteA8R8G8B8Surface(Sprite);
   DrawA8R8G8B8(Surface,Sprite^.Width,Sprite^.Height,Frame^.Offx-16,Frame^.Offy-8,0,1,ItemSkin^.VertexColor,ItemSkin^.ColorFx,View);
   FreeMem(Surface);
 end;
end;

procedure TFrmItemSkinEd.RefreshSkinList;
var i:longint;
    ItemSkin:PItemSkinInfo;
begin
 LstItemSkin.Items.BeginUpdate;
 LstItemSkin.Clear;
 for i:=0 to LstItemSkin.Count-1 do
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[i]);
  LstItemSkin.Items.AddObject(IntToStr(ITemSkin.SkinId)+' : '+ITemSkin.SkinName,TObject(ItemSkin));
 end;
 LstItemSkin.Items.EndUpdate;
end;



procedure TFrmItemSkinEd.ComboColorFxChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.ColorFx:=ComboColorFx.ItemIndex;
 end;
end;

procedure TFrmItemSkinEd.ComboMouseCursorChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.MouseCursor:=ComboMouseCursor.ItemIndex;
 end;
end;

procedure TFrmItemSkinEd.ComboTypeChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.SkinType:=ComboType.ItemIndex;
 end;
end;



procedure TFrmItemSkinEd.EdtAnimDelayChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.AnimationDelay:=StrToInt(EdtAnimDelay.Text)/1000;
 end;
end;

procedure TFrmItemSkinEd.EdtColorModChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.VertexColor:=HexToint(EdtColorMod.Text);
 end;
end;

procedure TFrmItemSkinEd.EdtSkinIdChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if EdtSkinId.Text='' then exit;
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.SkinId:=StrToInt(EdtSkinId.Text);
  LstItemSkin.Items[LstItemSkin.ItemIndex]:=ItemSkin^.SkinName+' : '+IntToStr(ItemSkin^.SkinId);
 end;
end;

procedure TFrmItemSkinEd.EdtSkinNameChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.SkinName:=EdtSkinName.Text;
  LstItemSkin.Items[LstItemSkin.ItemIndex]:=ItemSkin^.SkinName+' : '+IntToStr(ItemSkin^.SkinId);
 end;

end;

procedure TFrmItemSkinEd.EdtSoundCloseChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.CloseSound:=EdtSoundClose.Text;
 end;
end;

procedure TFrmItemSkinEd.EdtSoundOpenChange(Sender: TObject);
var ItemSkin:PItemSkinInfo;
begin
 if LstItemSkin.ItemIndex>=0 then
 begin
  ITemSkin:=PItemSkinInfo(LstItemSkin.Items.Objects[LstItemSkin.ItemIndex]);
  ITemSkin.OpenSound:=EdtSoundOpen.Text;
 end;
end;

end.

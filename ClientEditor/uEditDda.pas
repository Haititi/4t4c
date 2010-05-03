unit uEditDda;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, Menus, MGR32_Image, ComCtrls,FastStream,udda, StdCtrls, ExtCtrls,
  ImgList,hashunit,hashPool,UAbout,Globals,SurfaceUtils;

type
  TFrmClEdit = class(TForm)
    MainMenu1: TMainMenu;
    TreeView: TTreeView;
    View: TMPaintBox32;
    File1: TMenuItem;
    ools1: TMenuItem;
    N1: TMenuItem;
    Exit1: TMenuItem;
    DlgOpen: TOpenDialog;
    Panel1: TPanel;
    LblName: TLabel;
    LblIndex: TLabel;
    LblOffx: TLabel;
    LblWidth: TLabel;
    LblHeight: TLabel;
    LblOffy: TLabel;
    LblOffY2: TLabel;
    LblOffX2: TLabel;
    LbLSize: TLabel;
    TreeImageList: TImageList;
    LblTrans: TLabel;
    LblTransCol: TLabel;
    LblPal: TLabel;
    PopupSprite: TPopupMenu;
    ExportThatFile1: TMenuItem;
    ReplaceSprite1: TMenuItem;
    N3: TMenuItem;
    DeleteSprite1: TMenuItem;
    Label1: TLabel;
    Label2: TLabel;
    TreeViewPal: TTreeView;
    About1: TMenuItem;
    MassAutoCrop1: TMenuItem;
    MassExport1: TMenuItem;
    MassImport1: TMenuItem;
    N4: TMenuItem;
    AutoCrop1: TMenuItem;
    PopupPal: TPopupMenu;
    ExportPal1: TMenuItem;
    DeletePal1: TMenuItem;
    N5: TMenuItem;
    ImportPal1: TMenuItem;
    ImportSprite1: TMenuItem;
    Options1: TMenuItem;
    ExportDirectory1: TMenuItem;
    N6: TMenuItem;
    MnuLoadIndex: TMenuItem;
    MnuSaveIndex: TMenuItem;
    N7: TMenuItem;
    MassAssociatepalette1: TMenuItem;
    N8: TMenuItem;
    N9: TMenuItem;
    Adddirectory1: TMenuItem;
    Progress: TProgressBar;
    N10: TMenuItem;
    Computevideomemoryrequirement1: TMenuItem;
    ComboComp: TComboBox;
    Label3: TLabel;
    Label4: TLabel;
    ComboFormat: TComboBox;
    DeleteDuplicate1: TMenuItem;
    Id1: TMenuItem;
    MapIdEditor1: TMenuItem;
    MonsterIdEditor1: TMenuItem;
    SurfaceEditor1: TMenuItem;
    EdtTransCol: TEdit;
    StatusBar: TStatusBar;
    ItemIdEditor1: TMenuItem;
    SpellEditor1: TMenuItem;
    PuppetIdEditor1: TMenuItem;
    AlphaSort1: TMenuItem;
    N2: TMenuItem;
    SearchSprite1: TMenuItem;
    SearchNext1: TMenuItem;
    DlgOpenPict: TOpenDialog;
    N11: TMenuItem;
    Autosort1: TMenuItem;
    MoveDirtoroot1: TMenuItem;
    SpriteAssemblertest1: TMenuItem;
    procedure FormCreate(Sender: TObject);
    procedure ExportThatFile1Click(Sender: TObject);
    procedure ReplaceSprite1Click(Sender: TObject);
    procedure About1Click(Sender: TObject);
    procedure MassExport1Click(Sender: TObject);
    procedure AutoCrop1Click(Sender: TObject);
    procedure ExportPal1Click(Sender: TObject);
    procedure ImportSprite1Click(Sender: TObject);
    procedure MnuLoadIndexClick(Sender: TObject);
    procedure MnuSaveIndexClick(Sender: TObject);
    procedure MassAssociatepalette1Click(Sender: TObject);
    procedure TreeViewEdited(Sender: TObject; Node: TTreeNode; var S: string);
    procedure BtnSearchClick(Sender: TObject);
    procedure Computevideomemoryrequirement1Click(Sender: TObject);
    procedure TreeViewDragOver(Sender, Source: TObject; X, Y: Integer;State: TDragState; var Accept: Boolean);
    procedure Adddirectory1Click(Sender: TObject);
    procedure TreeViewDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure DeleteDuplicate1Click(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure DeleteSprite1Click(Sender: TObject);
    procedure MapIdEditor1Click(Sender: TObject);
    procedure ComboCompChange(Sender: TObject);
    procedure TreeViewClick(Sender: TObject);
    procedure TreeViewPalClick(Sender: TObject);
    procedure TreeViewChange(Sender: TObject; Node: TTreeNode);
    procedure TreeViewPalChange(Sender: TObject; Node: TTreeNode);
    procedure ComboFormatChange(Sender: TObject);
    procedure SurfaceEditor1Click(Sender: TObject);
    procedure LblTransClick(Sender: TObject);
    procedure EdtTransColChange(Sender: TObject);
    procedure DeletePal1Click(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure Exit1Click(Sender: TObject);
    procedure ItemIdEditor1Click(Sender: TObject);
    procedure SpellEditor1Click(Sender: TObject);
    procedure MonsterIdEditor1Click(Sender: TObject);
    procedure PuppetIdEditor1Click(Sender: TObject);
    procedure AlphaSort1Click(Sender: TObject);
    procedure SearchSprite1Click(Sender: TObject);
    procedure SearchNext1Click(Sender: TObject);
    procedure MoveDirtoroot1Click(Sender: TObject);
    procedure SpriteAssemblertest1Click(Sender: TObject);

  private
    { Private declarations }
  public
    { Public declarations }
    Procedure LoadAll(FileName:string);
    Procedure SaveAll(DirName:string);
    Procedure UpdateTree;
    Procedure UpdatePaletteTree;
    Procedure ShowSprite(Sprite:PSprite);
    Procedure DrawSpriteView(Data:PCardinal;Width,Height:longint);
    Function FindPalette(GraphName:string):PPalette;
    Procedure AddSprite(Sprite:PSprite);
    procedure DeleteSpriteByName(SpriteName:string);
    procedure DeleteSpriteByPtr(Sprite:PSprite);
    Procedure SearchNextSprite;
  end;

var
  FrmClEdit: TFrmClEdit;

  Version:integer;
  SelectedSprite:PSprite;
  SelectedPalette:PPalette;

  SearchName:string;
  SearchIndex:integer;

implementation

uses uImpExp,uPalUnit,MidEditor,ItemSkin,SkinEdit,uSurfEdit,SpellEdit,PuppetSkinEdit,UConsole;
{$R *.dfm}

procedure TFrmClEdit.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
 FrmSurfEdit.Enabled:=false;
 TreeView.Enabled:=false;
 TreeViewPal.Enabled:=false;
 CanClose:=true;
end;

procedure TFrmClEdit.FormCreate(Sender: TObject);
begin
 InitRndHash;
 CurrentDir:=GetCurrentDir;
 LoadingState:=false;

 ResetIndex(Index);
 CreateIndex(Index);
end;


procedure TFrmClEdit.FormDestroy(Sender: TObject);
begin
 TreeView.Items.BeginUpdate;
 TReeView.Items.Clear;
 TreeView.Items.EndUpdate;

 TreeViewPal.Items.BeginUpdate;
 TreeViewPal.Items.Clear;
 TreeViewPal.Items.EndUpdate;
end;


procedure TFrmClEdit.FormResize(Sender: TObject);
begin        //
 TreeView.Height:=(Progress.Top-TreeView.Top)-4;
 TreeViewPal.Height:=(Progress.Top-TreeViewPal.Top)-4;

 View.Width:=(FrmClEdit.ClientWidth-View.Left)-4;
 View.Height:=(Progress.Top-View.Top)-4;
end;


function MakeDirFromNode(Node:TTreeNode):string;
begin
 Result:='';
 if Node=nil then exit;
 Result:=MakeDirFromNode(Node.Parent)+Node.Text+'\';
end;
   
procedure TFrmClEdit.MnuLoadIndexClick(Sender: TObject);
begin
 DlgOpen.FilterIndex:=1;
 DlgOpen.Title:='Choose an Index file';
 if DlgOpen.Execute then
 begin
  LoadAll(DlgOpen.FileName);
 end;
end;

procedure TFrmClEdit.LoadAll(FileName: string);
var ThatPath:string;
    i:longint;
    Fst:TFastStream;
begin
  ThatPath:=ExtractFilePath(FileName);

  CreateIndex(Index);
  LoadIndex(Index,ThatPath);

  //post init

  StatusBar.Panels[0].Text:='Palettes: '+IntToStr(Index.Palettes.Count);
  StatusBar.Panels[1].Text:='Sprites: '+IntToStr(Index.SpriteNames.Count);

  UpdateTree;
  UpdatePaletteTree;
  LoadingState:=true;
end;

procedure TFrmClEdit.MassExport1Click(Sender: TObject);
var i:longint;
begin
 if LoadingState=true then
 begin
  //for i:=0 to Index.Count-1 do

   //SpriteExport(CurrentDir+'\'+ArrayToString(Path,256)+'\',@Index.SpriteNames[i],FindPalette(ArrayToString(SpriteName,64)));
 end;
end;

procedure TFrmClEdit.MnuSaveIndexClick(Sender: TObject);
begin
 DlgOpen.FilterIndex:=1;
 DlgOpen.Title:='Choose path to save index';
 if DlgOpen.Execute then
 begin
  SaveAll(DlgOpen.FileName);
  ShowMessage('Saving finished.');
 end;
end;

procedure TFrmClEdit.SaveAll(DirName: string);
var Path:string;
    i:integer;
begin//
 //MassAssociatepalette1Click(nil);
 Path:=ExtractFilePath(DirName);

  //Create the path list
 Index.PathCount:=Index.SpriteNames.Count;
 Setlength(Index.Paths,Index.PathCount);
 for i:=0 to Index.PathCount-1 do
 begin
  Index.Paths[i].SpriteName:=PSprite(Index.SpriteNames[i]).SpriteName;
  if PSprite(Index.SpriteNames[i]).Node.Parent<>nil then
   Index.Paths[i].Path:=MakeDirFromNode(PSprite(Index.SpriteNames[i]).Node.parent)
  else
   Index.Paths[i].Path:='\';
 end;

 SaveIndex(Index,Application,Progress,Path);
end;

procedure TFrmClEdit.SearchNext1Click(Sender: TObject);
begin
 SearchNextSprite;
end;

procedure TFrmClEdit.SearchNextSprite;
var i:integer;
begin
 if SearchIndex>= (Index.SpriteNames.Count-1) then
 begin
  SearchIndex:=0;
 end;

 TreeView.Items.BeginUpdate;
 for i:=SearchIndex to Index.SpriteNames.Count-1 do
 begin
  if pos(SearchName,PSprite(Index.SpriteNames[i]).SpriteName)<>0 then
  begin
   TreeView.Select(PSprite(Index.SpriteNames[i]).Node);
   SearchIndex:=i+1;
   break;
  end;
 end;
 TreeView.Items.EndUpdate;
end;

procedure TFrmClEdit.SearchSprite1Click(Sender: TObject);
begin
 SearchName:=InputBox('Search','Enter Name : ','');
 SearchName:=LowerCase(SearchName);
 SearchIndex:=0;
 SearchNextSprite;
end;

function SearchRootNode(Str:string):TTreeNode;
var NodeSearch:TTreeNode;
begin
  result:=NIL;
  NodeSearch:=FrmClEdit.TreeView.Items.GetFirstNode;
  while (NodeSearch<>NIL) do
  begin
    if NodeSearch.Text=str then
    begin
     result:=NodeSearch;
     exit;
    end;
    NodeSearch:=NodeSearch.getNextSibling;
  end;
end;

function SearchNode(Node:TTreeNode;Str:string):TTreeNode;
var NodeSearch:TTreeNode;
begin
  result:=NIL;
  NodeSearch:=Node.getFirstChild;
  while (NodeSearch<>NIL) do
  begin
    if NodeSearch.Text=str then
    begin
     result:=NodeSearch;
     exit;
    end;
    NodeSearch:=Node.GetNextChild(NodeSearch);
  end;
end;

function SeekAndBuild(Node:TTreeNode;Path:string):TTreeNode;
var NewPath,FirstPart:string;
    SepPos:longint;
    NewNode:TTreeNode;
begin
  //we extract the first path part
  SepPos:=Pos('\',Path);
  if SepPos<>0 then
  begin
   FirstPart:=Copy(Path,1,SepPos-1);
   NewPath:=Copy(Path,SepPos+1,255);
  end else
  begin
    FirstPart:=Path;
  end;


  //we seek that part in the tree
  if Node<>nil then
   NewNode:=SearchNode(Node,FirstPart)
               else
   NewNode:=SearchRootNode(FirstPart);

  if NewNode=Nil then
  begin //doesn't exist , create it
   NewNode:=FrmClEdit.TreeView.Items.AddChildFirst(Node,FirstPart);
  end else
  begin //exist
   //well ;)
  end;

  Result:=NewNode;
  //if we are left only with a one part path just exit
  if (SepPos=0) or (length(NewPath)=0) then exit;

  //recurse
  Result:=SeekAndBuild(NewNode,NewPath);
end;



procedure TFrmClEdit.UpdateTree;
var i:longint;
    FirstNode,Node,Node2:TTreeNode;
    Path:string;
//    FolderHash:THash;
begin
 TreeView.Items.BeginUpdate;
 TreeView.Items.Clear;

 //TODO
 //PathHash.clear;

 FirstNode:=TreeView.Items.GetFirstNode;

 if Index.PathCount>0 then
 begin
  for i:=0 to Index.SpriteNames.Count-1 do
  begin
   Path:=Index.Paths[i].Path;
   Node:=Index.PAthHash.SearchByName(Path);
  if Node=nil then
   begin
     Node:=SeekAndBuild(Nil,Path);   //recur
     Index.PathHash.AddHashEntry(Path,Node);
   end;
   Node2:=TreeView.Items.AddChildObjectFirst(Node,PSprite(Index.SpriteNames[i]).SpriteName,Index.SpriteNames[i]);
   Node2.ImageIndex:=1;
   PSprite(Index.SpriteNames[i]).Node:=Node2;
  end;
 end else
 begin
  for i:=0 to Index.SpriteNames.Count-1 do
  begin
   Node2:=TreeView.Items.AddChildObjectFirst(nil,PSprite(Index.SpriteNames[i]).SpriteName,Index.SpriteNames[i]);
   Node2.ImageIndex:=1;
   PSprite(Index.SpriteNames[i]).Node:=Node2;
  end;
 end;

 TreeView.Items.EndUpdate;
 TreeView.Invalidate;
end;


//experimental

type
  PGroup=^TGroup;
  TGroup=array[0..62] of PGroup; //pointer which point to TGroup

function CharIndex(c:char):longint;
begin
 Result:=0;
 case c of
  '0'..'9':result:=ord(c)-48;
  'A'..'Z':result:=ord(c)-65+10;
  'a'..'z':result:=ord(c)-97+36;
 end;
end;

Procedure AddNameToGroup(PName:PChar;Group:PGroup);
var Index:longint;
begin
 if PName^=chr(0) then
  exit;

 Index:=CharIndex(PName^);
 if  Group^[Index]=nil then
 begin
  new(Group^[Index]);
 end else
 begin

 end;
  inc(PName);
  AddNameToGroup(PName,Group^[Index]);
end;


Procedure CleanGroup(Group:Pgroup);
var i:longint;
begin
 for i:=0 to 61 do
  if Group[i]<>nil then
  begin
   CleanGroup(Group[i]);
   dispose(Group[i]);
  end;
end;

procedure TFrmClEdit.UpdatePaletteTree;
var i:integer;
    Root,Node:TTreeNode;
   // Grouper:TGroup;
begin
 TreeViewPal.Items.BeginUpdate;
 TreeViewPal.Items.Clear;
      {
 for i:=0 to Dpd.Count-1 do
 begin
  AddNameToGroup(PChar(@Dpd.Palettes[i].PalName[0]),@Grouper);
 end;  }


 for i:=0 to Index.Palettes.Count-1 do
 begin
  Node:=TreeViewPal.Items.AddChildObjectFirst(NIL,PPalette(Index.Palettes[i]).PalName,Index.Palettes[i]);
  Node.ImageIndex:=1;
 end;

 TreeViewPal.Items.EndUpdate;
 TreeViewPal.Invalidate;
end;



procedure TFrmClEdit.DrawSpriteView(Data: PCardinal; Width, Height: Integer);
var i,j:longint;
    DataPtr:PCardinal;
    xoff,yoff:integer;
begin
 View.Buffer.Clear(0);
 DataPtr:=Data;

 xoff:=(View.Width-Width)div 2;
 yoff:=(View.Height-height)div 2;

 //draw the sprite box and the memory box
 View.Buffer.FrameRectS(xoff-1,yoff-1,xoff+NextPow2(Width)+1,yoff+Height+1,$FFFF0000);
 View.Buffer.FrameRectS(xoff-1,yoff-1,xoff+Width+1,yoff+Height+1,$FF00FF00);


 for j:=0 to Height-1 do
 begin
   for i:=0 to Width-1 do
   begin
    View.Buffer.SetPixelTS(xoff+i,yoff+j,DataPtr^);
    inc(DataPtr);
   end;
 end;

 View.Invalidate;
end;

procedure TFrmClEdit.MapIdEditor1Click(Sender: TObject);
begin
 FrmMIdEd.Show;
end;

procedure TFrmClEdit.ItemIdEditor1Click(Sender: TObject);
begin
 FrmItemSkinEd.Show;
end;

procedure TFrmClEdit.SpellEditor1Click(Sender: TObject);
begin
 FrmSpellEd.show;
end;

procedure TFrmClEdit.MonsterIdEditor1Click(Sender: TObject);
begin
 FrmSkinEdit.Show;
end;


procedure TFrmClEdit.MoveDirtoroot1Click(Sender: TObject);
begin
 TreeView.Selected.MoveTo(nil,naAddChildFirst);
end;

procedure TFrmClEdit.PuppetIdEditor1Click(Sender: TObject);
begin
 FrmPuppetSkinEdit.Show;
end;

procedure TFrmClEdit.MassAssociatepalette1Click(Sender: TObject);
var i:longint;
var Pal:PPalette;
begin
 for i:=0 to Index.SpriteNames.Count-1 do
 begin
  Pal:=FindPalette(PSprite(Index.SpriteNames[i]).SpriteName);
  PSprite(Index.SpriteNames[i])^.Offset.PaletteName:=Pal^.PalName;
 end;
end;


function TFrmClEdit.FindPalette(GraphName: string): PPalette;
var NameCpy:string;
begin
 Result:=Index.PalHash.SearchByName(GraphName+'P');
 if Result<>NIL then exit;
 Result:=Index.PalHash.SearchByName('P'+GraphName);
 if Result<>NIL then exit;
 Result:=Index.PalHash.SearchByName(GraphName);


 if Result=Nil then
 begin
  NameCpy:=GraphName;
  while (Result=nil) and (length(NameCpy)>0) do
  begin
    NameCpy:=copy(NameCpy,1,LEngth(NameCpy)-1);
    Result:=Index.PalHash.SearchByName(NameCpy+'P');
  end;

 if Result=NIL then
  Result:=Index.PalHash.SearchByName('Bright1');
 end;
end;

procedure TFrmClEdit.AutoCrop1Click(Sender: TObject);
var Sprite:PSprite;
    Data:Pointer;
    Comp:longint;
    NodeP,Node:TTreeNode;
begin
 if TreeView.Selected<>nil then
 begin
  if TreeView.Selected.ImageIndex=1 then
  begin
   Sprite:=TreeView.Selected.Data;
   if Sprite.SurfFormat<>SurfFormat_P8 then
   begin
    ShowMessage('Cannot Cut a non-P8 format...');
    exit;
   end;

   CropSprite(Sprite);

   TreeViewClick(nil);

   LblPal.Caption:='Palette: '+Sprite^.Offset.PaletteName;
  end else
  begin
   NodeP:=TreeView.Selected; //parent node
   node:=nodeP.getFirstChild;

   while (node<>nil) do
   begin
    if node.ImageIndex=1 then
     if node.Data<>nil then
      CropSprite(Node.Data);

    Node:=Nodep.GetNextChild(Node);
   end;

  end;
 end;
end;



procedure TFrmClEdit.ShowSprite(Sprite: PSprite);
var Data:PCardinal;
    Pal:PPalette;
begin
 LblName.Caption:='Name: '+Sprite^.SpriteName;
 LblIndex.Caption:='Offset: '+IntToStr(Sprite^.DataOffset);
 LbLSize.Caption:='Size: '+IntToStr(Sprite^.DataSize);
 //LbLSize.Caption:='Size: '+IntToStr(Sprite^.Sprite.NbBytes);
 LblWidth.Caption:='Width: '+IntToStr(Sprite^.Width);
 LblHeight.Caption:='Height: '+IntToStr(Sprite^.Height);
 if (Sprite.Offset<>nil) then
 begin
  LblOffx.Caption:='OffX: '+IntToStr(Sprite^.Offset.OffsetX);
  LblOffy.Caption:='OffY: '+IntToStr(Sprite^.Offset.OffsetY);
  LblOffX2.Caption:='OffX2: '+IntToStr(Sprite^.Offset.OffsetX2);
  LblOffY2.Caption:='OffY2: '+IntToStr(Sprite^.Offset.OffsetY2);
  LblTrans.Caption:='Trans: '+IntToStr(Sprite^.Offset.Transparency);
  EdtTransCol.Text:=IntToStr(Sprite^.Offset.TransCol);
  LblPal.Caption:='Palette: '+Sprite^.Offset.PaletteName;
 end else
 begin
  LblOffx.Caption:='OffX: N/A';
  LblOffy.Caption:='OffY: N/A';
  LblOffX2.Caption:='OffX2: N/A';
  LblOffY2.Caption:='OffY2: N/A';
  LblTrans.Caption:='Trans: N/A';
  EdtTransCol.Text:='N/A';
  LblPal.Caption:='Palette: NoPal';
 end;

 ComboComp.ItemIndex:=Sprite^.CompType;
 ComboFormat.ItemIndex:=sprite^.SurfFormat;

 Data:=GetSpriteA8R8G8B8Surface(Sprite);

 DrawSpriteView(Data,Sprite^.Width,Sprite^.Height);
 freemem(Data);
end;


procedure TFrmClEdit.SurfaceEditor1Click(Sender: TObject);
begin
 if TreeView.Selected<>nil then
 begin
  if TreeView.Selected.ImageIndex=1 then
  begin
   FrmSurfEdit.SetSprite(TreeView.Selected.Data);
   FrmSurfEdit.ShowModal;
  end;
 end;

end;

procedure TFrmClEdit.LblTransClick(Sender: TObject);
var Sprite:PSprite;
begin
 if TreeView.Selected<>nil then
 begin
  if TreeView.Selected.ImageIndex=1 then
  begin
   Sprite:=TreeView.Selected.Data;
   if Sprite.Offset=nil then
   begin
    CreateOffsetFromSprite(Sprite);
   end;
    if Sprite.Offset.Transparency>1 then
       Sprite.Offset.Transparency:=0;  //hack to remove NM bullshit
    Sprite.Offset.Transparency:=Sprite.Offset.Transparency xor 1;
    LblTrans.Caption:='Trans: '+IntToStr(Sprite^.Offset.Transparency);
  end;
 end;
end;

procedure TFrmClEdit.EdtTransColChange(Sender: TObject);
var Sprite:PSprite;
begin
 if TreeView.Selected<>nil then
 begin
  if TreeView.Selected.ImageIndex=1 then
  begin
   if EdtTransCol.Text='' then
    exit;
   Sprite:=TreeView.Selected.Data;
   if Sprite.Offset<>nil then
   begin
    Sprite.Offset.TransCol:=StrToInt(EdtTransCol.Text);
   end;
  end;
 end;
end;

procedure TFrmClEdit.TreeViewChange(Sender: TObject; Node: TTreeNode);
begin
 if TreeView.Selected<>nil then
 begin
  if TreeView.Selected.ImageIndex=1 then
  begin
   ShowSprite(TreeView.Selected.Data);
  end;
 end;
end;

procedure TFrmClEdit.TreeViewClick(Sender: TObject);
begin
 if TreeView.Selected<>nil then
 begin
   if TreeView.Selected.ImageIndex=1 then
   begin
    ShowSprite(TreeView.Selected.Data);
   end;
 end;
end;


procedure TFrmClEdit.TreeViewPalChange(Sender: TObject; Node: TTreeNode);
begin
 if TreeViewPal.Selected<>nil then
 begin
   if TreeViewPal.Selected.ImageIndex=1 then
   begin
    FrmPal.ShowPal(TreeViewPal.Selected.Data);
   end;
 end;
end;

procedure TFrmClEdit.TreeViewPalClick(Sender: TObject);
begin
if TreeViewPal.Selected<>nil then
 begin
   if TreeViewPal.Selected.ImageIndex=1 then
   begin
    FrmPal.ShowPal(TreeViewPal.Selected.Data);
   end;
 end;
end;

procedure TFrmClEdit.TreeViewDragOver(Sender, Source: TObject; X, Y: Integer;   State: TDragState; var Accept: Boolean);
var NodeOver:TTreeNode;
    Tag:longint;
begin
 //the tag is one if that come from palette
 //2 otherwise
 Tag:=(Source as TTreeView).Tag;

 NodeOver:=TreeView.GetNodeAt(x,y);
 if Nodeover<>nil then
 begin
  if NodeOver.ImageIndex=1 then
   Accept:=true;
 end;

 if y<10 then
 begin
  SendMessage(TreeView.Handle, WM_VSCROLL, SB_LINEUP, 0);
 end else
 if y>(treeview.Height-10) then
 begin
  SendMessage(TreeView.Handle, WM_VSCROLL, SB_LINEDOWN, 0);
 end;
end;

procedure TFrmClEdit.TreeViewDragDrop(Sender, Source: TObject; X, Y: Integer);
var NodeOver:TTreeNode;
    Tag,i,Scount:longint;
    Sprite:PSprite;
    Pal:PPalette;
    Templist:Tlist;
begin
 if not(source is TTreeView) then
  exit;
 Tag:=(Source as TTreeView).Tag;

 //ShowMessage(IntToStr(TTreeView(Source).Tag));

 NodeOver:=TreeView.DropTarget;
 if NodeOver=nil then
  exit;

 if tag=1 then    //coming from palette
 begin
  if NodeOver.ImageIndex=1 then
  begin
   Sprite:=NodeOver.Data;
   Pal:=TreeViewPal.Selected.Data ;
   //we need to create an offset struct if there is none
   if Sprite^.Offset=nil then
   begin
    CreateOffsetFromSprite(Sprite);
   end;
    Sprite^.Offset^.PaletteName:=Pal.PalName;

  end;
 end else        //coming from Self
 begin
  if NodeOver.ImageIndex=1 then
   NodeOver:=NodeOver.parent;

  SCount:=TreeView.SelectionCount;
  TempList:=TList.Create;
  For i:=0 to SCount-1 do
  begin
   TempList.Add(TreeView.Selections[i]);
  end;

  TreeView.Items.BeginUpdate;
  for i:=0 to SCount-1 do
  begin
   TTreeNode(TempList[i]).MoveTo(NodeOver,naAddChildFirst);
  end;
  TreeView.Items.EndUpdate;
  Templist.Free;
 end;
end;

procedure TFrmClEdit.DeleteSprite1Click(Sender: TObject);
var i,SCount:longint;
    Sprite:PSprite;
    TempList:TList;
    Node:TTreeNode;
begin
 if TreeView.SelectionCount>0 then
 begin
  //
  SCount:=TreeView.SelectionCount;
  TempList:=TList.Create;
  For i:=0 to Scount-1 do
  begin
   TempList.Add(TreeView.Selections[i]);
  end;

  for i:=0 to SCount-1 do
  begin
   DeleteSpriteFromIndex(Index,TTreeNode(TempList[i]).Data);
  end;
  Index.SpriteNames.Pack;

  TreeView.Items.BeginUpdate;
  For i:=0 to Scount-1 do
  begin
   Node:=TempList[i];
   Node.Delete;
  end;
  TempList.Free;
  TreeView.Items.EndUpdate;
 end;
end;

procedure TFrmClEdit.AddSprite(Sprite: PSprite);
begin
 //add to treeview
 Sprite.Node:=TreeView.Items.AddObject(nil,Sprite.SpriteName,Sprite);
 Sprite.Path:=MakeDirFromNode(Sprite.Node);
 Sprite.Node.ImageIndex:=1;

 //ad to index
 AddSpriteToIndex(Index,Sprite);
end;

procedure TFrmClEdit.DeleteSpriteByName(SpriteName:string);
var Sprite:PSprite;
begin
 Sprite:=Index.SpriteHash.SearchByName(SpriteName);
 if Sprite<>nil then
  DeleteSpriteByPtr(Sprite);
end;

procedure TFrmClEdit.DeleteSpriteByPtr(Sprite:PSprite);
begin //
 //remove from view
 if Sprite.Node<>nil then
    TreeView.Items.Delete(Sprite.Node);

 DeleteSpriteFromIndex(Index,Sprite);
end;



procedure TFrmClEdit.AlphaSort1Click(Sender: TObject);
begin
 with TPopupMenu(TMenuitem(sender).GetParentMenu).PopupComponent as TTreeView do
 begin
  if SortType=stText then
   SortType:=stNone  else
  if SortType=stNone then
   SortType:=stText;
  Invalidate;
 end;
end;

Procedure UpdateSpriteDirectory(Node:TTreeNode);
var ChildNode:TTreeNode;
begin
 ChildNode:=Node.getFirstChild;

{ while ChildNode<>Nil do
 begin
  if ChildNode.ImageIndex=0 then
  begin
   UpdateSpriteDirectory(ChildNode);
  end else
  begin
   StringToarray(PSprite(ChildNode.Data)^.Path,256,MakeDir(Node));
  end;
  ChildNode:=ChildNode.getNextSibling;
 end;     }
end;

procedure TFrmClEdit.Adddirectory1Click(Sender: TObject);
var Node:TTreeNode;
begin
 if TreeView.Selected<>nil then
 begin
  node:=TreeView.Selected;
  if node.ImageIndex=1 then
   Node:=Node.Parent;
  TreeView.Items.AddChildFirst(node,'New Directory');
 end else
 begin
  TreeView.Items.AddChildFirst(nil,'New Directory');
 end;
end;

procedure TFrmClEdit.TreeViewEdited(Sender: TObject; Node: TTreeNode;  var S: string);
var Sprite:PSprite;
begin
  //TODO change all the underlying sprite Path
  if Node.ImageIndex=0 then
  begin
   Node.Text:=s;
   //recurse and progressively build path and change encountered sprite path
   UpdateSpriteDirectory(Node);
  end else
  if Node.ImageIndex=1 then
  begin
   Sprite:=Node.Data;
   Index.SpriteHash.RemoveHashEntry(Sprite^.SpriteName);
   Sprite^.SpriteName:=s;
   Index.SpriteHash.AddHashEntry(Sprite^.SpriteName,Sprite);
  end;
end;


procedure TFrmClEdit.DeletePal1Click(Sender: TObject);
var i,SCount:longint;
    Pal:PPalette;
    TempList:TList;
    Node:TTreeNode;
begin
 if TreeViewPal.SelectionCount>0 then
 begin
  //
  //save the selection
  SCount:=TreeViewPal.SelectionCount;
  TempList:=TList.Create;
  For i:=0 to Scount-1 do
  begin
   TempList.Add(TreeViewPal.Selections[i]);
  end;

  //delete objects
  for i:=0 to SCount-1 do
  begin
   Pal:=Index.Palettes.Extract(TTreeNode(TempList[i]).Data);
   Dispose(Pal);
  end;
  Index.Palettes.Pack;

  //update the treeview
  TreeViewPal.Items.BeginUpdate;
  For i:=0 to Scount-1 do
  begin
   Node:=TempList[i];
   Node.Delete;
  end;
  TempList.Free;
  TreeViewPal.Items.EndUpdate;
 end;{
 if TreeViewPal.Selected<>nil then
 begin
  Index.Palettes.Remove(TreeViewPal.Selected.Data);
  TreeViewPal.Selected.Delete;
  dec(Index.PalCount);
 end;    }
end;

procedure TFrmClEdit.Exit1Click(Sender: TObject);
begin
 self.close;
end;

procedure TFrmClEdit.ExportPal1Click(Sender: TObject);
begin
 if TreeViewPal.Selected<>nil then
 begin
   if TreeViewPal.Selected.ImageIndex=1 then
   begin
    PalExport(CurrentDir+'\',TreeViewPal.Selected.Data);
   end;
 end;
end;

procedure TFrmClEdit.ExportThatFile1Click(Sender: TObject);
var Sprite:PSprite;
begin
 if TreeView.Selected<>nil then
 begin
   if TreeView.Selected.ImageIndex=1 then
   begin
    Sprite:=TreeView.Selected.Data;
    if Sprite.Offset<>nil then
    begin

     SpriteExport(CurrentDir+'\',Sprite,Index.PalHash.SearchByName(Sprite.Offset.PaletteName));
    end else
    begin
     SpriteExport(CurrentDir+'\',Sprite,FindPalette(Sprite^.SpriteName));
    end;
   end;
 end;
end;

procedure TFrmClEdit.ImportSprite1Click(Sender: TObject);
var NewSprite:PSprite;
    NewPalette:PPalette;
    Node:TTreeNode;
begin
 Node:=Nil;
 if TreeView.Selected=nil then
 begin
  Node:=TreeView.Items.GetFirstNode;
 end else
 if TreeView.Selected.ImageIndex=1 then
 begin
  Node:=TreeView.Selected.Parent;
 end else
 begin
  Node:=TreeView.Selected;
 end;

 NewSprite:=nil;
 NewPalette:=nil;

 if DlgOpenPict.Execute then
 begin
  LoadPicture(DlgOpenPict.FileName,NewSprite,NewPalette);
  //rajouter le nom
  NewSprite.SpriteName:=ExtractFileName(DlgOpenPict.FileName);

  //tODO tester l'existence

  //todo ajouter le pal et sprite au treeview
  AddSprite(NewSprite);

  if NewPalette<>nil then
  begin
   Index.Palettes.Add(NewPalette);
   Index.PalHash.AddHashEntry(RandHash(NewPalette.PalName),NewPalette)
  end;

  //TreeView.Items.AddChildObject(Node,NewSprite.SpriteName,@NewSprite);
 end;

end;



procedure TFrmClEdit.ReplaceSprite1Click(Sender: TObject);
var Sprite,NewSprite:PSprite;
    Palette:PPalette;
begin
 if TreeView.Selected<>nil then
 begin
   if TreeView.Selected.ImageIndex=1 then
   begin
    DlgOpen.FileName:='*.tga';
    DlgOpen.FilterIndex:=2;
    if DlgOpen.Execute then
    begin
     Sprite:=TreeView.Selected.Data;  //sprite to replace
     new(NewSprite);
     Palette:=FindPalette(Sprite.SpriteName);

     LoadTga(DlgOpen.FileName,NewSprite,Palette);
     {if NewSprite^.Sprite.Width<>Sprite^.Sprite.Width then
     begin
      //
     end;  }
     Sprite^.Width:=NewSprite^.Width;
     Sprite^.Height:=NewSPrite^.Height;
     Sprite^.DataSize:=NewSPrite^.DataSize;
     Sprite^.CompType:=newSprite^.CompType;
     Sprite^.SurfFormat:=NewSPrite^.SurfFormat;
     freemem(Sprite^.data);
     getmem(Sprite^.data,NewSprite^.Width*NewSprite^.Height);
     move(NewSprite^.Data^,Sprite^.data^,Sprite^.DataSize);
     Dispose(NewSprite);
    end;
   end;
 end;
end;



procedure TFrmClEdit.ComboCompChange(Sender: TObject);
var Sprite:PSprite;
    Data:Pointer;
begin
 if TreeView.Selected<>nil then
 begin
  if TreeView.Selected.ImageIndex=1 then
  begin
   Sprite:=TreeView.Selected.data;

   if Sprite^.CompType>Comp_NoComp then //need to decompress it first
   begin
    Data:=UncompressSprite(Sprite);
    FreeMem(Sprite^.Data);
    Sprite^.Data:=Data;
    Sprite^.DataSize:=Sprite^.Width*Sprite^.Height*GetFormatSize(Sprite^.SurfFormat);
    Sprite^.CompType:=Comp_NoComp;
   end;

   case ComboComp.ItemIndex of
    Comp_NoComp:
    begin
     //nothing to do
    end;
    Comp_Zlib: CompressSpriteZlib(Sprite);

    Comp_Lzma: CompressSpriteLzma(Sprite);

   end;
  end;
 end;
end;

procedure TFrmClEdit.ComboFormatChange(Sender: TObject);
var Sprite:PSprite;
    Data:Pointer;
    DataSize:cardinal;
    Comp:cardinal;
    Format:Cardinal;
begin
 if TreeView.Selected<>nil then
 begin
  if TreeView.Selected.ImageIndex=1 then
  begin
   Sprite:=TreeView.Selected.data;

   Comp:=Sprite.CompType;
   Data:=GetSpriteA8R8G8B8Surface(Sprite);
   DataSize:=Sprite.Width*Sprite.Height*GetFormatSize(SurfFormat_A8R8G8B8);

   Format:=Sprite.SurfFormat;

   case ComboFormat.ItemIndex of
    0:begin //A8r8g8b8
       //already converted
       Format:=SurfFormat_A8R8G8B8;
      end;
    1:begin //R5G6B5
       //Format:=SurfFormat_R5G6B5;
       ConvertSurfaceFromA8R8G8B8ToR5G6B5(pcardinal(Data),DataSize);
       Format:=SurfFormat_R5G6B5;
      end;
    2:begin //A1R5G5B5
       ConvertSurfaceFromA8R8G8B8ToA1R5G5B5(pcardinal(Data),DataSize);
       Format:=SurfFormat_A1R5G5B5;
      end;
    3:begin //P8
       ShowMessage('Sorry, unable to convert back to palletized format for now..');
       freemem(data);
       exit;
      end;
    4:begin  //DXT1a
       ShowMessage('Sorry, no support for DXT textures yet..');
       freemem(data);
       exit;
      end;
    5:begin  //DXT5
       ConvertSurfaceFromA8R8G8B8ToDXT5(Sprite.Width,Sprite.Height,pcardinal(Data),DataSize);
       Format:=SurfFormat_DXT5;
      end;
   end;

   Sprite.CompType:=Comp_NoComp;
   freemem(Sprite.Data);
   Sprite.Data:=data;
   Sprite.DataSize:=Datasize;
   Sprite.SurfFormat:=Format;



   case Comp of
    Comp_NoComp:
    begin
     //nothing to do
    end;
    Comp_Zlib: CompressSpriteZlib(Sprite);
    Comp_Lzma: CompressSpriteLzma(Sprite);
   end;
  end;
 end;

end;

procedure TFrmClEdit.Computevideomemoryrequirement1Click(Sender: TObject);
var i:integer;
    Total:int64;
    Sprite:PSprite;
begin
 Total:=0;
 for i:=0 to Index.SpriteNames.Count-1 do
 begin
  Sprite:=Index.SpriteNames[i];
  Total:=Total+NextPow2(Sprite.Width*4)*Sprite.Height;//*NextPow2(Index.SpriteNames[i].Height);
 end;

 ShowMessage('Video Memory estimate : '+IntToStr(Total));
end;

procedure TFrmClEdit.DeleteDuplicate1Click(Sender: TObject);
var i,j,Total,PaletteCount:longint;
    Hashes:array of Record
            Hash:cardinal;
            Count:cardinal;
    end;
begin
 PaletteCount:=Index.Palettes.Count;
 //compute pal hashes
 SetLength(Hashes,PaletteCount);
 for i:=0 to PaletteCount-1 do
 begin
  Hashes[i].Hash:=RandHash(@PPalette(Index.Palettes[i]).Rgb[0],768);
  Hashes[i].Count:=0;
 end;

 Total:=0;
 for j:=0 to PaletteCount-2 do
 begin
  for i:=j+1 to PaletteCount-1 do
  begin
   if Hashes[j].Hash=Hashes[i].Hash then
   begin
    inc(Hashes[i].Count);
   end;
  end;
  if Hashes[J].Count>0 then
   inc(Total);
 end;
 ShowMessage(IntToStr(Total));
end;

procedure TFrmClEdit.About1Click(Sender: TObject);
begin
 FrmAbout.Show;
end;

procedure TFrmClEdit.BtnSearchClick(Sender: TObject);
var s:string;
    Enumerator:TTreeNodesEnumerator;
    Node:TTreeNode;
begin
 //s:=EdtSearch.Text;
 Enumerator:=TreeView.Items.GetEnumerator;
 Enumerator.MoveNext;
 Node:=Enumerator.GetCurrent;
 while node<>nil do
 begin
  if Node.ImageIndex=1 then
   if node.Text=s then
   begin
     TreeView.Select(node);
     //TreeView.Selected:=;
     break;
   end;
  if not Enumerator.MoveNext then break;
  node:=Enumerator.GetCurrent;
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

procedure TFrmClEdit.SpriteAssemblertest1Click(Sender: TObject);
var List:TList;
    TestSurface:array[0..511,0..511] of byte;
    Found:Boolean;
    i,Counter,Direction,Sz:integer;
    SpName,BaseName:String;
    Sprite:PSprite;
begin    //
 FrmCon.Show;
 List:=TList.Create;

 BaseName:='wonakedhead';
 //ajoute les sprites dans la liste
  for Direction:=0 to 4 do
  begin
   Found:=true;
   Counter:=0;
   repeat
    SpName:=BaseName+GenerateWalkSuffix(Direction,Counter);
    Sprite:=Index.SpriteHash.SearchByName(SpName);
    if Sprite<>nil then
    begin
     List.Add(Sprite);
     inc(Counter);
    end else
    begin
     Found:=false;
    end;
   until Found=false;
   if counter=0 then break;
  end;

  for Direction:=0 to 4 do
  begin
   Found:=true;
   Counter:=0;
   repeat
    SpName:=BaseName+'a'+GenerateWalkSuffix(Direction,Counter);
    Sprite:=Index.SpriteHash.SearchByName(SpName);
    if Sprite<>nil then
    begin
     List.Add(Sprite);
     inc(Counter);
    end else
    begin
     Found:=false;
    end;
   until Found=false;
   if counter=0 then break;
  end;

  for Direction:=0 to 4 do
  begin
   Found:=true;
   Counter:=0;
   repeat
    SpName:=BaseName+'b'+GenerateWalkSuffix(Direction,Counter);
    Sprite:=Index.SpriteHash.SearchByName(SpName);
    if Sprite<>nil then
    begin
     List.Add(Sprite);
     inc(Counter);
    end else
    begin
     Found:=false;
    end;
   until Found=false;
   if counter=0 then break;
  end;

  Found:=true;
  Counter:=0;
  repeat
   SpName:=BaseName+GenerateDeathSuffix(Counter);
   Sprite:=Index.SpriteHash.SearchByName(SpName);
   if Sprite<>nil then
   begin
    List.Add(Sprite);
   end else
   begin
    Found:=false;
   end;
   inc(Counter);
  until Found=false;

  FrmCon.Log('Sprite count : '+IntToStr(List.Count));


  Sz:=0;
  for i:=0 to List.Count-1 do
  begin
   Sprite:=List[i];
   Sz:=Sz+Sprite.Width*Sprite.Height*4;
  end;

  FrmCon.Log('Sprites Cumulated Size 32bits: '+IntToStr(Sz));

  Sz:=0;
  for i:=0 to List.Count-1 do
  begin
   Sprite:=List[i];
   Sz:=Sz+NextPow2(Sprite.Width*4)*Sprite.Height;
  end;

  FrmCon.Log('Sprites Cumulated Video Memory size 32bits: '+IntToStr(Sz));

  Sz:=0;
  for i:=0 to List.Count-1 do
  begin
   Sprite:=List[i];
   Sz:=Sz+Sprite.Width*Sprite.Height;
  end;
  FrmCon.Log('Total sprites pixels : '+IntToStr(Sz));
  FrmCon.Log('Total sprites Pixels /256 : '+IntToStr(Sz div 256));

end;

begin
 InitRndHash;
end.

unit SpellEdit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Menus, ComCtrls, ExtCtrls,Particules;

type
  TFrmSpellEd = class(TForm)  
    Label1: TLabel;
    EdtSpellId: TEdit;
    Label2: TLabel;
    Label3: TLabel;
    EdtSpellName: TEdit;
    PopupSpellList: TPopupMenu;
    MnuItAddSpell1: TMenuItem;
    MnuItDuplicateSpell1: TMenuItem;
    MnuItPreviewSpell1: TMenuItem;
    N3: TMenuItem;
    MnuItDeleteSpell1: TMenuItem;
    GrpBoxEmitSettings: TGroupBox;
    Label6: TLabel;
    Label13: TLabel;
    ComboEmitLink: TComboBox;
    ComboEmissionType: TComboBox;
    EdtPartSprite: TEdit;
    Label7: TLabel;
    ComboBlendMode: TComboBox;
    Label8: TLabel;
    EdtMaxTime: TEdit;
    Label9: TLabel;
    ChkActive: TCheckBox;
    TreeSpell: TTreeView;
    MnuItAddEmitter2: TMenuItem;
    MnuItDuplicateEmitter2: TMenuItem;
    MnuItDeleteEmitter2: TMenuItem;
    N2: TMenuItem;
    MnuItAddInitializer1: TMenuItem;
    MnuItAddMover1: TMenuItem;
    MnuItAddFinalizer1: TMenuItem;
    MnuItDelete1: TMenuItem;
    InitPoint1: TMenuItem;
    INitCircle1: TMenuItem;
    InitLine1: TMenuItem;
    InitCircleaera1: TMenuItem;
    InitSquarearea1: TMenuItem;
    InitSphere1: TMenuItem;
    InitCube1: TMenuItem;
    ForceVelocity1: TMenuItem;
    ForceRotation1: TMenuItem;
    RandomColor1: TMenuItem;
    RadomPosDeviation1: TMenuItem;
    Zoneforce1: TMenuItem;
    GlobalForce1: TMenuItem;
    Gravitywell1: TMenuItem;
    RandomMovedev1: TMenuItem;
    AlphachangewithTime1: TMenuItem;
    Colorchangewithtime1: TMenuItem;
    Deleteontime1: TMenuItem;
    DeleteonPlane1: TMenuItem;
    Deleteonsphere1: TMenuItem;
    PanelCtrl: TPanel;
    DeleteonPlaneY1: TMenuItem;
    DeleteonPlaneZ1: TMenuItem;
    Standardparticulesmover1: TMenuItem;
    MovementAttenuator1: TMenuItem;
    Repulsor1: TMenuItem;
    procedure MnuItAddSpell1Click(Sender: TObject);
    procedure EdtSpellInfoChange(Sender: TObject);
    procedure TreeSpellClick(Sender: TObject);
    procedure ActivateMenuSpell(Mnu:integer);
    procedure MnuItAddEmitter2Click(Sender: TObject);
    function GetParentSpell(Node:TTreeNode):TTreeNode;
    procedure EdtPartSpriteChange(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure AddInitClick(Sender: TObject);
    procedure AddMoverClick(Sender: TObject);
    procedure AddFinalizerClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

 TEmitter=record
   Node:TTreeNode;
   EmitterNo:integer;
   Active:boolean;
   SpriteName:string;
   BlendMode:integer;
   LinkType:integer;
   EmittorType:integer;

   NodeInit,NodeMove,NodeFina:TTreeNode;
 end;
 PEmitter=^TEmitter;

 TSpellPart=record
  SpellName:string;
  SpellId:integer;
  SpellTime:integer;
  EmitterCount:integer;
  Emitters:TList;// of PEmitter;
 end;
 PSpellPart=^TSpellPart;

 const
  NodeType_Spell=1;
  NodeType_Emitter=2;
  NodeType_Func=3;
  NodeType_FuncSpe=4;

var
  FrmSpellEd: TFrmSpellEd;
  ShownFunc:TBaseFunctionClass;


implementation

{$R *.dfm}

function Str2int2(s:string):integer;
begin
 Result:=0;
 if s<>'' then
  Result:=StrToInt(s);
end;

function MakeSpellName(Spell:PSpellPart):string;
begin
 Result:=Spell.SpellName+' : '+IntToStr(Spell.SpellId);
end;

function MakeEmitterName(Emit:PEmitter):string;
begin
 Result:='Emitter '+IntToStr(Emit.EmitterNo)+' : ';
 if Emit.Active then
  Result:=Result+'On'
                else
  Result:=result+'Off';
end;

function TFrmSpellEd.GetParentSpell(Node: TTreeNode): TTreeNode;
begin     //

end;




procedure TFrmSpellEd.ActivateMenuSpell(Mnu: integer);
begin
 //switch of all menu

 MnuItDuplicateSpell1.Enabled:=false;
 MnuItPreviewSpell1.Enabled:=false;
 MnuItDeleteSpell1.Enabled:=false;

 MnuItAddEmitter2.Enabled:=false;
 MnuItDuplicateEmitter2.Enabled:=false;
 MnuItDeleteEmitter2.Enabled:=false;

 MnuItAddInitializer1.Enabled:=false;
 MnuItAddMover1.Enabled:=false;
 MnuItAddFinalizer1.Enabled:=false;
 MnuItDelete1.Enabled:=false;

 //switch on the right ones
 case Mnu of
  NodeType_Spell:
  begin
     MnuItDuplicateSpell1.Enabled:=true;
     MnuItPreviewSpell1.Enabled:=true;
     MnuItDeleteSpell1.Enabled:=true;
     MnuItAddEmitter2.Enabled:=true;
    end;
  NodeType_Emitter:
  begin

     MnuItDuplicateEmitter2.Enabled:=true;
     MnuItDeleteEmitter2.Enabled:=true;
    end;
  NodeType_Func:
  begin
   MnuItAddInitializer1.Enabled:=true;
   MnuItAddMover1.Enabled:=true;
   MnuItAddFinalizer1.Enabled:=true;
   MnuItDelete1.Enabled:=true;
  end;
  NodeType_FuncSpe: //special intermediate node
  begin
   MnuItAddInitializer1.Enabled:=true;
   MnuItAddMover1.Enabled:=true;
   MnuItAddFinalizer1.Enabled:=true;
  end;
 end;
end;

procedure TFrmSpellEd.MnuItAddSpell1Click(Sender: TObject);
var Spell:PSpellPart;
    Node:TTreeNode;
begin
 new(Spell);
 Spell.SpellName:='No name';
 Spell.SpellId:=0;
 Spell.EmitterCount:=0;
 Spell.SpellTime:=6000;
 spell.Emitters:=TList.Create;
 Node:=TreeSpell.Items.AddChildObject(nil,MakeSpellName(Spell),Spell);
 node.ImageIndex:=NodeType_Spell;
end;

procedure TFrmSpellEd.EdtSpellInfoChange(Sender: TObject);
var Spell:PSpellPart;
    Node:TTreeNode;
begin
 if TreeSpell.Selected<>nil then
 if TreeSpell.Selected.ImageIndex=NodeType_Spell then
 begin
  Spell:=PSpellPart(TreeSpell.Selected.Data);
  case TEdit(Sender).Tag of
   1:Spell.SpellId:=Str2int2(EdtSpellId.Text);
   2:Spell.SpellName:=EdtSpellName.Text;
   3:Spell.SpellTime:=Str2int2(EdtMaxTime.Text);
  end;
  TreeSpell.Selected.Text:=MakeSpellName(Spell);
 end;
end;


procedure TFrmSpellEd.FormCreate(Sender: TObject);
begin
 ShownFunc:=nil;
end;

procedure TFrmSpellEd.TreeSpellClick(Sender: TObject);
var Spell:PSpellPart;
    Emit:PEmitter;
begin
 if TreeSpell.Selected<>nil then
 begin
  ActivateMenuSpell(TreeSpell.Selected.ImageIndex);
  
  case TreeSpell.Selected.ImageIndex of

   NodeType_Spell:
   begin   //spell click
    Spell:=TreeSpell.Selected.Data;
    EdtSpellId.Text:=IntToStr(Spell.SpellId);
    EdtMaxTime.Text:=IntToStr(Spell.SpellTime);
    EdtSpellName.Text:=Spell.SpellName;
   end;

   NodeType_Emitter:
   begin     //emitter click
    Emit:=TreeSpell.Selected.Data;
    EdtPartSprite.Text:=emit.SpriteName;
    ComboBlendMode.ItemIndex:=Emit.BlendMode;
    ComboEmitLink.ItemIndex:=Emit.LinkType;
    ComboEmissionType.ItemIndex:=Emit.EmittorType;
    ChkActive.Checked:=Emit.Active;
   end;

   NodeType_Func:
   begin
    if ShownFunc<>nil then
       ShownFunc.RemoveControls;
    ShownFunc:=TreeSpell.Selected.Data;
    ShownFunc.AddControls;
   end;

  end;
 end;

end;


procedure TFrmSpellEd.MnuItAddEmitter2Click(Sender: TObject);
var Spell:PSpellPart;
    Emit:PEmitter;
begin   //
 if TreeSpell.Selected<>nil then
 if TreeSpell.Selected.ImageIndex=NodeType_Spell then
 begin
  Spell:=TreeSpell.Selected.Data;
  new(Emit);
  Emit.Active:=true;
  Emit.EmitterNo:=Spell.Emitters.Count;
  Emit.SpriteName:='';
  Emit.BlendMode:=0;
  Emit.LinkType:=0;
  Emit.EmittorType:=0;
  Spell.Emitters.Add(Emit);

  Emit.Node:=TreeSpell.Items.AddChildObject(TreeSpell.Selected,MakeEmitterName(Emit),Emit);
  Emit.Node.ImageIndex:=NodeType_Emitter;

  Emit.NodeInit:=TreeSpell.Items.AddChild(Emit.Node,'Initializers');
  Emit.NodeMove:=TreeSpell.Items.AddChild(Emit.Node,'Movers');
  Emit.NodeFina:=TreeSpell.Items.AddChild(Emit.Node,'Finalizers');
  Emit.NodeInit.ImageIndex:=NodeType_FuncSpe;
  Emit.NodeMove.ImageIndex:=NodeType_FuncSpe;
  Emit.NodeFina.ImageIndex:=NodeType_FuncSpe;
 end;
end;


procedure TFrmSpellEd.EdtPartSpriteChange(Sender: TObject);
var Emit:PEmitter;
begin
 if TreeSpell.Selected<>nil then
 if TreeSpell.Selected.ImageIndex=NodeType_Emitter then
 begin
  Emit:=TreeSpell.Selected.Data;
  Emit.SpriteName:=EdtPartSprite.Text;
 end;
end;


procedure TFrmSpellEd.AddInitClick(Sender: TObject);
var InitNode,NewNode:TTreeNode;
    Emit:PEmitter;
    Func:TBaseFunctionClass;
begin
 if TreeSpell.Selected<>nil then
 begin
  if TreeSpell.Selected.ImageIndex=NodeType_Emitter then
  begin
   Emit:=TreeSpell.Selected.Data;
  end else
  if TreeSpell.Selected.ImageIndex=NodeType_FuncSpe then
  begin
   Emit:=TreeSpell.Selected.Parent.Data;
  end else
  if TreeSpell.Selected.ImageIndex=NodeType_Func then
  begin
   Emit:=TreeSpell.Selected.Parent.Parent.Data;
  end else
  begin
   exit;
  end;

  InitNode:=Emit.NodeInit;

  case TMenuItem(Sender).Tag of
   0:Func:=TInitPoint.Create(PanelCtrl);
   1:Func:=TInitLine.Create(PanelCtrl);
   2:Func:=TInitCircle.Create(PanelCtrl);
   3:Func:=TInitDisc.Create(PanelCtrl);
   4:Func:=TInitSquare.Create(PanelCtrl);
   5:Func:=TInitSphere.Create(PanelCtrl);
   6:Func:=TInitCube.Create(PanelCtrl);
   7:Func:=TInitForce.Create(PanelCtrl);
   8:Func:=TInitRotation.Create(PanelCtrl);
  end;

  NewNode:=TreeSpell.Items.AddChildObject(InitNode,Func.GetNodeName,Func);
  NewNode.ImageIndex:=NodeType_Func;
 end;
end;

procedure TFrmSpellEd.AddMoverClick(Sender: TObject);
var MoveNode,NewNode:TTreeNode;
    Emit:PEmitter;
    Func:TBaseFunctionClass;
begin
 if TreeSpell.Selected<>nil then
 begin
  if TreeSpell.Selected.ImageIndex=NodeType_Emitter then
  begin
   Emit:=TreeSpell.Selected.Data;
  end else
  if TreeSpell.Selected.ImageIndex=NodeType_FuncSpe then
  begin
   Emit:=TreeSpell.Selected.Parent.Data;
  end else
  begin
   exit;
  end;

  MoveNode:=Emit.NodeMove;

  case TMenuItem(Sender).Tag of
   0:Func:=TMoverGlobalForce.Create(PanelCtrl);
  end;

  NewNode:=TreeSpell.Items.AddChildObject(MoveNode,Func.GetNodeName,Func);
  NewNode.ImageIndex:=NodeType_Func;
 end;
end;

procedure TFrmSpellEd.AddFinalizerClick(Sender: TObject);
var FinalNode,NewNode:TTreeNode;
    Emit:PEmitter;
    Func:TBaseFunctionClass;
begin
 if TreeSpell.Selected<>nil then
 begin
  if TreeSpell.Selected.ImageIndex=NodeType_Emitter then
  begin
   Emit:=TreeSpell.Selected.Data;
  end else
  if TreeSpell.Selected.ImageIndex=NodeType_FuncSpe then
  begin
   Emit:=TreeSpell.Selected.Parent.Data;
  end else
  begin
   exit;
  end;

  FinalNode:=Emit.NodeFina;

  case TMenuItem(Sender).Tag of
   0:Func:=TFinalizerTime.Create(PanelCtrl);
   1:Func:=TFinalizerTime.Create(PanelCtrl);
   2:Func:=TFinalizerPlaneX.Create(PanelCtrl);
  end;

  NewNode:=TreeSpell.Items.AddChildObject(FinalNode,Func.GetNodeName,Func);
  NewNode.ImageIndex:=NodeType_Func;
 end;
end;

end.

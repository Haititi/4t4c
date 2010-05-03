unit Particules;

interface
 uses SysUtils,controls,classes,ExtCtrls;

 type
  TEmitterFuncData=record
    ClassType:integer;
    Prm:array of single;
  end;

  TParticule=record
   x,y,z:single;
   Vx,Vy,Vz:single;
   Rx,Ry,Rz:single;
   Color:cardinal;
   Time:single;
  end;
  PParticule=^TParticule;

  TLinkedCtrl=record
   Ctrl:TWinControl;
   Link:PSingle;
  end;
  PLinkedCtrl=^TLinkedCtrl;


  TBaseFunctionClass=class
    private
     ParentCtrl:TWinControl;
     ObjectType:integer;
     CtrlList:TList;
     NextPos,NextTag:integer;
     NodeName:string;
     function GetNextPos:integer;
     procedure InitControl(Obj:TWinControl;TheVar:PSingle);
    public
     Constructor Create(ParentControl:TWinControl);
     Destructor Destroy;override;
     Function GetParent:TWinControl;
     procedure LoadData(Data:TEmitterFuncData);virtual;
     procedure SaveData(var Data:TEmitterFuncData);virtual;

     Function GetNodeName:string;
     Function GetClassType:integer;

     procedure AddControls;
     Procedure RemoveControls;
     procedure ControlChange(Sender: TObject);Virtual;
  end;

  TParticuleProcessor=class(TBaseFunctionClass)
    private
    public
     function Process(Particule:PParticule;Const Delta:single):integer;Virtual;
  end;

  TInitPoint=class(TParticuleProcessor)
    private
     x,y,z:single;
     Editx: TLabeledEdit;
     Edity: TLabeledEdit;
     Editz: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;

   TInitLine=class(TParticuleProcessor)
    private
     x,y,z:single;
     x2,y2,z2:single;
     Editx,Editx2: TLabeledEdit;
     Edity,Edity2: TLabeledEdit;
     Editz,Editz2: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;

  TInitCircle=class(TParticuleProcessor)
    private
     Radius:single;
     x,y,z:single;
     EditRadius:TLabeledEdit;
     Editx: TLabeledEdit;
     Edity: TLabeledEdit;
     Editz: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;

  TInitDisc=class(TParticuleProcessor)
    private
     Radius:single;
     x,y,z:single;
     EditRadius:TLabeledEdit;
     Editx: TLabeledEdit;
     Edity: TLabeledEdit;
     Editz: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;

  TInitSquare=class(TParticuleProcessor)
    private
     Size:single;
     x,y,z:single;
     EditSize:TLabeledEdit;
     Editx: TLabeledEdit;
     Edity: TLabeledEdit;
     Editz: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;

  TInitSphere=class(TParticuleProcessor)
    private
     Radius:single;
     x,y,z:single;
     EditRadius:TLabeledEdit;
     Editx: TLabeledEdit;
     Edity: TLabeledEdit;
     Editz: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;

  TInitCube=class(TParticuleProcessor)
    private
     Size:single;
     x,y,z:single;
     EditSize:TLabeledEdit;
     Editx: TLabeledEdit;
     Edity: TLabeledEdit;
     Editz: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;

  TInitForce=class(TParticuleProcessor)
   private
     Fx,Fy,Fz:single;
     EditFx: TLabeledEdit;
     EditFy: TLabeledEdit;
     EditFz: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;

  TInitRotation=class(TParticuleProcessor)
   private
     Rx,Ry,Rz:single;
     EditRx: TLabeledEdit;
     EditRy: TLabeledEdit;
     EditRz: TLabeledEdit;
    public
     Constructor Create(ParentControl:TWinControl);
     procedure LoadData(Data:TEmitterFuncData);override;
     procedure SaveData(var Data:TEmitterFuncData);override;
     function Process(Particule:PParticule;Const Delta:single):integer;override;
  end;


 //movers
  TMoverStdMovement=class(TParticuleProcessor)
  private
  public
   Constructor Create(ParentControl:TWinControl);
   procedure LoadData(Data:TEmitterFuncData);override;
   procedure SaveData(var Data:TEmitterFuncData);override;
   function Process(Particule:PParticule;Const Delta:single):integer;override;
 end;

 TMoverGlobalForce=class(TParticuleProcessor)
  private
   Fx,Fy,Fz:single;
   EditFx: TLabeledEdit;
   EditFy: TLabeledEdit;
   EditFz: TLabeledEdit;
  public
   Constructor Create(ParentControl:TWinControl);
   procedure LoadData(Data:TEmitterFuncData);override;
   procedure SaveData(var Data:TEmitterFuncData);override;
   function Process(Particule:PParticule;Const Delta:single):integer;override;
 end;

 TMoverAttenuator=class(TParticuleProcessor)
  private
   ForceAtt,RotAtt:single;
   EditForceAtt: TLabeledEdit;
   EditRotAtt: TLabeledEdit;
  public
   Constructor Create(ParentControl:TWinControl);
   procedure LoadData(Data:TEmitterFuncData);override;
   procedure SaveData(var Data:TEmitterFuncData);override;
   function Process(Particule:PParticule;Const Delta:single):integer;override;
 end;

 //finalizers

 TFinalizerTime=class(TParticuleProcessor)
  private
   MaxTime:single;
   EditMaxTime: TLabeledEdit;
  public
   Constructor Create(ParentControl:TWinControl);
   procedure LoadData(Data:TEmitterFuncData);override;
   procedure SaveData(var Data:TEmitterFuncData);override;
   function Process(Particule:PParticule;Const Delta:single):integer;override;
 end;

 TFinalizerPlaneX=class(TParticuleProcessor)
  private
   PosX:single;
   EditPosX: TLabeledEdit;
  public
   Constructor Create(ParentControl:TWinControl);
   procedure LoadData(Data:TEmitterFuncData);override;
   procedure SaveData(var Data:TEmitterFuncData);override;
   function Process(Particule:PParticule;Const Delta:single):integer;override;
 end;

 const
  ClassType_InitPoint=1;
  ClassType_InitLine=2;
  ClassType_InitCircle=3;
  ClassType_InitDisc=4;
  ClassType_InitSquare=5;
  ClassType_InitSphere=6;
  ClassType_InitCube=7;
  ClassType_InitForce=8;
  ClassType_InitRotation=9;
  ClassType_InitRandColor=10;
  ClassType_InitRandPos=11;

  ClassType_MoverStdMovement=256;
  ClassType_MoverAttenuator=257;
  ClassType_MoverGlobalForce=258;
  ClassType_MoverGravity=259;
  ClassType_MoverGravityWell=260;
  ClassType_MoverRepulsor=259;

  ClassType_FinalizerTime=512;
  ClassType_FinalizerPlaneX=513;

implementation

uses Otherutils;

{ TBaseFunctionClass }


function StrToFloat2(S:string):single;
begin
 Result:=0.0;
 if S<>'' then
 begin
  Result:=StrToFloatDef(s,0.0);
 end;
end;


constructor TBaseFunctionClass.Create(ParentControl: TWinControl);
begin
 ParentCtrl:=ParentControl;
 NextPos:=24;
 NextTag:=0;
 CtrlList:=TList.Create;
end;

destructor TBaseFunctionClass.Destroy;
var i:integer;
begin
 for i:=0 to CtrlList.Count-1 do
 begin
  PLinkedCtrl(CtrlList[i]).Ctrl.Free;
 end;
 CtrlList.Free;
 inherited;
end;

procedure TBaseFunctionClass.AddControls;
var i:integer;
    Link:PLinkedCtrl;
begin
 for i:=0 to CtrlList.Count-1 do
 begin
  Link:=CtrlList[i];
  ParentCtrl.InsertControl(Link.Ctrl);
  if Link.Ctrl is TLabeledEdit then
   ParentCtrl.InsertControl(TLabeledEdit(Link.Ctrl).EditLabel);
 end;
end;

procedure TBaseFunctionClass.RemoveControls;
var i:integer;
    Link:PLinkedCtrl;
begin
 for i:=0 to CtrlList.Count-1 do
 begin
  Link:=CtrlList[i];
  ParentCtrl.RemoveControl(Link.Ctrl);
  if Link.Ctrl is TLabeledEdit then
   ParentCtrl.RemoveControl(TLabeledEdit(Link.Ctrl).EditLabel);
 end;
end;

procedure TBaseFunctionClass.ControlChange(Sender: TObject);
var Link:PLinkedCtrl;
begin
 //search the ctrl and modify the linked var
 Link:=CtrlList[TwinControl(Sender).tag];
 if Link.Ctrl is TLabeledEdit then
 begin
  Link.Link^:=StrToFloat2(TLabeledEdit(Link.Ctrl).Text);
 end;
end;

function TBaseFunctionClass.GetNextPos: integer;
begin
 Result:=NextPos;
 inc(NextPos,48);
end;

function TBaseFunctionClass.GetNodeName: string;
begin
 Result:=NodeName;
end;

Function TBaseFunctionClass.GetClassType:integer;
begin
 Result:=ObjectType;
end;

function TBaseFunctionClass.GetParent: TWinControl;
begin
 Result:=ParentCtrl;
end;

procedure TBaseFunctionClass.InitControl(Obj: TWinControl;TheVar:PSingle);
var NewLink:PLinkedCtrl;
begin

 Obj.Tag:=NextTag;
 Obj.Left:=8;
 Obj.Top:=GetNextPos;

 inc(NextTag);

 new(NewLink);
 NewLink.Ctrl:=Obj;
 NewLink.Link:=TheVar;
 CtrlList.Add(NewLink);

 if Obj is TLabeledEdit then
  With TLabeledEdit(Obj) do
  begin
   Text:=FloatToStrF(TheVar^,ffFixed,7,5);
   LabelPosition:=lpAbove;
   EditLabel.Visible:=true;
   OnChange:=ControlChange;
  end;
end;

procedure TBaseFunctionClass.LoadData(Data: TEmitterFuncData);
begin

end;

procedure TBaseFunctionClass.SaveData(var Data: TEmitterFuncData);
begin
 Data.ClassType:=ObjectType;
end;

{ TParticuleProcessor }

function TParticuleProcessor.Process(Particule: PParticule;Const Delta:single):integer;
begin

end;

{ TInitPoint }

constructor TInitPoint.Create(ParentControl: TWinControl);
begin
 inherited;

 ObjectType:=ClassType_InitPoint;
 NodeName:='Init Point';

 Editx:=TLabeledEdit.Create(ParentControl);
 Editx.EditLabel.Caption:='Pos x';
 InitControl(Editx,@x);

 Edity:=TLabeledEdit.Create(ParentControl);
 Edity.EditLabel.Caption:='Pos y';
 InitControl(Edity,@y);

 Editz:=TLabeledEdit.Create(ParentControl);
 Editz.EditLabel.Caption:='Pos z';
 InitControl(Editz,@z);

end;

function TInitPoint.Process(Particule: PParticule;Const Delta:single):integer;
begin
 Particule.x:=x;
 Particule.y:=y;
 Particule.z:=z;
end;

procedure TInitPoint.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 x:=Data.Prm[Idx];
 inc(Idx);
 y:=Data.Prm[Idx];
 inc(Idx);
 z:=Data.Prm[Idx];

 Editx.Text:=FloatToStrF(x,ffFixed,7,4);
 Edity.Text:=FloatToStrF(y,ffFixed,7,4);
 Editz.Text:=FloatToStrF(z,ffFixed,7,4);
end;

procedure TInitPoint.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,3);
 Idx:=0;
 Data.Prm[Idx]:=x;
 inc(Idx);
 Data.Prm[Idx]:=y;
 inc(Idx);
 Data.Prm[Idx]:=z;
end;


{ TInitLine }


constructor TInitLine.Create(ParentControl: TWinControl);
begin
 inherited;

 ObjectType:=ClassType_InitLine;
 NodeName:='Init Line';

 Editx:=TLabeledEdit.Create(ParentControl);
 Editx.EditLabel.Caption:='Pos x';
 InitControl(Editx,@x);

 Edity:=TLabeledEdit.Create(ParentControl);
 Edity.EditLabel.Caption:='Pos y';
 InitControl(Edity,@y);

 Editz:=TLabeledEdit.Create(ParentControl);
 Editz.EditLabel.Caption:='Pos z';
 InitControl(Editz,@z);

 Editx2:=TLabeledEdit.Create(ParentControl);
 Editx2.EditLabel.Caption:='Pos x2';
 InitControl(Editx2,@x2);

 Edity2:=TLabeledEdit.Create(ParentControl);
 Edity2.EditLabel.Caption:='Pos y2';
 InitControl(Edity2,@y2);

 Editz2:=TLabeledEdit.Create(ParentControl);
 Editz2.EditLabel.Caption:='Pos z2';
 InitControl(Editz2,@z2);

end;

function TInitLine.Process(Particule: PParticule;Const Delta:single):integer;
var Lx,Ly,Lz,Sc:single;
begin
 //should be precalculated
 Lx:=x2-x;
 Ly:=y2-y;
 Lz:=z2-z;

 //random val 0<=X<=1.0
 Sc:=Random(10000)/10000;

 //compute a random point along the line
 Particule.x:=x+Lx*Sc;
 Particule.y:=y+Ly*Sc;
 Particule.z:=z+Lz*Sc;
end;

procedure TInitLine.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;

 x:=Data.Prm[Idx];
 inc(Idx);
 y:=Data.Prm[Idx];
 inc(Idx);
 z:=Data.Prm[Idx];
 inc(Idx);
 x2:=Data.Prm[Idx];
 inc(Idx);
 y2:=Data.Prm[Idx];
 inc(Idx);
 z2:=Data.Prm[Idx];

 Editx.Text:=FloatToStrF(x,ffFixed,7,4);
 Edity.Text:=FloatToStrF(y,ffFixed,7,4);
 Editz.Text:=FloatToStrF(z,ffFixed,7,4);
 Editx2.Text:=FloatToStrF(x2,ffFixed,7,4);
 Edity2.Text:=FloatToStrF(y2,ffFixed,7,4);
 Editz2.Text:=FloatToStrF(z2,ffFixed,7,4);

end;


procedure TInitLine.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,6);
 Idx:=0;

 Data.Prm[Idx]:=x;
 inc(Idx);
 Data.Prm[Idx]:=y;
 inc(Idx);
 Data.Prm[Idx]:=z;
 inc(Idx);
 Data.Prm[Idx]:=x2;
 inc(Idx);
 Data.Prm[Idx]:=y2;
 inc(Idx);
 Data.Prm[Idx]:=z2;

end;

{ TInitCircle }


constructor TInitCircle.Create(ParentControl: TWinControl);
begin
 inherited;

 ObjectType:=ClassType_InitCircle;
 NodeName:='Init Circle border';

 EditRadius:=TLabeledEdit.Create(ParentControl);
 EditRadius.EditLabel.Caption:='Radius';
 InitControl(EditRadius,@Radius);

 Editx:=TLabeledEdit.Create(ParentControl);
 Editx.EditLabel.Caption:='Pos x';
 InitControl(Editx,@x);

 Edity:=TLabeledEdit.Create(ParentControl);
 Edity.EditLabel.Caption:='Pos y';
 InitControl(Edity,@y);

 Editz:=TLabeledEdit.Create(ParentControl);
 Editz.EditLabel.Caption:='Pos z';
 InitControl(Editz,@z);
end;

function TInitCircle.Process(Particule: PParticule;Const Delta:single):integer;
var Ang:single;
begin
 Ang:=RandFloat01()*Pi*2;
 Particule.x:=x+sin(ang)*Radius;
 Particule.y:=y+cos(ang)*Radius;
 Particule.z:=z;
end;

procedure TInitCircle.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 Radius:=Data.Prm[Idx];
 inc(Idx);
 x:=Data.Prm[Idx];
 inc(Idx);
 y:=Data.Prm[Idx];
 inc(Idx);
 z:=Data.Prm[Idx];

 EditRadius.Text:=FloatToStrF(Radius,ffFixed,7,4);
 Editx.Text:=FloatToStrF(x,ffFixed,7,4);
 Edity.Text:=FloatToStrF(y,ffFixed,7,4);
 Editz.Text:=FloatToStrF(z,ffFixed,7,4);
end;


procedure TInitCircle.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,4);
 Idx:=0;

 Data.Prm[Idx]:=Radius;
 inc(Idx);
 Data.Prm[Idx]:=x;
 inc(Idx);
 Data.Prm[Idx]:=y;
 inc(Idx);
 Data.Prm[Idx]:=z;
end;

{ TInitDisc }

constructor TInitDisc.Create(ParentControl: TWinControl);
begin
 inherited;

 ObjectType:=ClassType_InitDisc;
 NodeName:='Init Disc border';

 EditRadius:=TLabeledEdit.Create(ParentControl);
 EditRadius.EditLabel.Caption:='Radius';
 InitControl(EditRadius,@Radius);

 Editx:=TLabeledEdit.Create(ParentControl);
 Editx.EditLabel.Caption:='Pos x';
 InitControl(Editx,@x);

 Edity:=TLabeledEdit.Create(ParentControl);
 Edity.EditLabel.Caption:='Pos y';
 InitControl(Edity,@y);

 Editz:=TLabeledEdit.Create(ParentControl);
 Editz.EditLabel.Caption:='Pos z';
 InitControl(Editz,@z);

end;

function TInitDisc.Process(Particule: PParticule; const Delta: single): integer;
var Ang,Dist:single;
begin
 Ang:=RandFloat01()*Pi*2;
 Dist:=RandFloat01()*Radius;

 Particule.x:=x+sin(ang)*Dist;
 Particule.y:=y+cos(ang)*Dist;
 Particule.z:=z;
end;

procedure TInitDisc.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 Radius:=Data.Prm[Idx];
 inc(Idx);
 x:=Data.Prm[Idx];
 inc(Idx);
 y:=Data.Prm[Idx];
 inc(Idx);
 z:=Data.Prm[Idx];

 EditRadius.Text:=FloatToStrF(Radius,ffFixed,7,4);
 Editx.Text:=FloatToStrF(x,ffFixed,7,4);
 Edity.Text:=FloatToStrF(y,ffFixed,7,4);
 Editz.Text:=FloatToStrF(z,ffFixed,7,4);
end;

procedure TInitDisc.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,4);
 Idx:=0;

 Data.Prm[Idx]:=Radius;
 inc(Idx);
 Data.Prm[Idx]:=x;
 inc(Idx);
 Data.Prm[Idx]:=y;
 inc(Idx);
 Data.Prm[Idx]:=z;
end;

{ TInitSquare }

constructor TInitSquare.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_InitSquare;
 NodeName:='Init Square';

 EditSize:=TLabeledEdit.Create(ParentControl);
 EditSize.EditLabel.Caption:='Size';
 InitControl(EditSize,@Size);

 Editx:=TLabeledEdit.Create(ParentControl);
 Editx.EditLabel.Caption:='Pos x';
 InitControl(Editx,@x);

 Edity:=TLabeledEdit.Create(ParentControl);
 Edity.EditLabel.Caption:='Pos y';
 InitControl(Edity,@y);

 Editz:=TLabeledEdit.Create(ParentControl);
 Editz.EditLabel.Caption:='Pos z';
 InitControl(Editz,@z);

end;

function TInitSquare.Process(Particule: PParticule; const Delta: single): integer;
var PosX,PosY:single;
begin
 PosX:=Randfloat1m1()*Size*0.5;
 PosY:=Randfloat1m1()*Size*0.5;

 Particule.x:=x+PosX;
 Particule.y:=y+PosY;
 Particule.z:=z;
end;

procedure TInitSquare.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 Size:=Data.Prm[Idx];
 inc(Idx);
 x:=Data.Prm[Idx];
 inc(Idx);
 y:=Data.Prm[Idx];
 inc(Idx);
 z:=Data.Prm[Idx];

 EditSize.Text:=FloatToStrF(Size,ffFixed,7,4);
 Editx.Text:=FloatToStrF(x,ffFixed,7,4);
 Edity.Text:=FloatToStrF(y,ffFixed,7,4);
 Editz.Text:=FloatToStrF(z,ffFixed,7,4);

end;



procedure TInitSquare.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,4);
 Idx:=0;

 Data.Prm[Idx]:=Size;
 inc(Idx);
 Data.Prm[Idx]:=x;
 inc(Idx);
 Data.Prm[Idx]:=y;
 inc(Idx);
 Data.Prm[Idx]:=z;

end;


{ TInitSphere }

constructor TInitSphere.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_InitSphere;
 NodeName:='Init Sphere';

 EditRadius:=TLabeledEdit.Create(ParentControl);
 EditRadius.EditLabel.Caption:='Radius';
 InitControl(EditRadius,@Radius);

 Editx:=TLabeledEdit.Create(ParentControl);
 Editx.EditLabel.Caption:='Pos x';
 InitControl(Editx,@x);

 Edity:=TLabeledEdit.Create(ParentControl);
 Edity.EditLabel.Caption:='Pos y';
 InitControl(Edity,@y);

 Editz:=TLabeledEdit.Create(ParentControl);
 Editz.EditLabel.Caption:='Pos z';
 InitControl(Editz,@z);

end;

function TInitSphere.Process(Particule: PParticule;  const Delta: single): integer;
var Angx,Angy,xy,Dist:single;
begin
  //TODO to check
 Angx:=Randfloat01()*pi*2;
 Angy:=Randfloat01()*pi*2;
 Dist:=RandFloat01()*Radius;

 Particule.x:=x+cos(Angx)*Radius;
 Particule.y:=y+sin(Angy)*Radius;
 Particule.z:=z+cos(Angy)*Radius;

end;

procedure TInitSphere.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 Radius:=Data.Prm[Idx];
 inc(Idx);
 x:=Data.Prm[Idx];
 inc(Idx);
 y:=Data.Prm[Idx];
 inc(Idx);
 z:=Data.Prm[Idx];

 EditRadius.Text:=FloatToStrF(Radius,ffFixed,7,4);
 Editx.Text:=FloatToStrF(x,ffFixed,7,4);
 Edity.Text:=FloatToStrF(y,ffFixed,7,4);
 Editz.Text:=FloatToStrF(z,ffFixed,7,4);

end;



procedure TInitSphere.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 Inherited;
 SetLength(Data.Prm,4);
 Idx:=0;

 Data.Prm[Idx]:=Radius;
 inc(Idx);
 Data.Prm[Idx]:=x;
 inc(Idx);
 Data.Prm[Idx]:=y;
 inc(Idx);
 Data.Prm[Idx]:=z;

end;

{ TInitCube }

constructor TInitCube.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_InitCube;
 NodeName:='Init Cube';

 EditSize:=TLabeledEdit.Create(ParentControl);
 EditSize.EditLabel.Caption:='Size';
 InitControl(EditSize,@Size);

 Editx:=TLabeledEdit.Create(ParentControl);
 Editx.EditLabel.Caption:='Pos x';
 InitControl(Editx,@x);

 Edity:=TLabeledEdit.Create(ParentControl);
 Edity.EditLabel.Caption:='Pos y';
 InitControl(Edity,@y);

 Editz:=TLabeledEdit.Create(ParentControl);
 Editz.EditLabel.Caption:='Pos z';
 InitControl(Editz,@z);

end;

function TInitCube.Process(Particule: PParticule; const Delta: single): integer;
var PosX,PosY,PosZ:single;
begin
 PosX:=Randfloat1m1()*Size*0.5;
 PosY:=Randfloat1m1()*Size*0.5;
 PosZ:=Randfloat1m1()*Size*0.5;

 Particule.x:=x+PosX;
 Particule.y:=y+PosY;
 Particule.z:=z+PosZ;

end;

procedure TInitCube.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 Size:=Data.Prm[Idx];
 inc(Idx);
 x:=Data.Prm[Idx];
 inc(Idx);
 y:=Data.Prm[Idx];
 inc(Idx);
 z:=Data.Prm[Idx];

 EditSize.Text:=FloatToStrF(Size,ffFixed,7,4);
 Editx.Text:=FloatToStrF(x,ffFixed,7,4);
 Edity.Text:=FloatToStrF(y,ffFixed,7,4);
 Editz.Text:=FloatToStrF(z,ffFixed,7,4);

end;



procedure TInitCube.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,4);
 Idx:=0;

 Data.Prm[Idx]:=Size;
 inc(Idx);
 Data.Prm[Idx]:=x;
 inc(Idx);
 Data.Prm[Idx]:=y;
 inc(Idx);
 Data.Prm[Idx]:=z;

end;

 { TInitForce }

constructor TInitForce.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_InitForce;
 NodeName:='Init Force';

 EditFx:=TLabeledEdit.Create(ParentControl);
 EditFx.EditLabel.Caption:='Force x';
 InitControl(EditFx,@Fx);

 EditFy:=TLabeledEdit.Create(ParentControl);
 EditFy.EditLabel.Caption:='Force y';
 InitControl(EditFy,@Fy);

 EditFz:=TLabeledEdit.Create(ParentControl);
 EditFz.EditLabel.Caption:='Force z';
 InitControl(EditFz,@Fz);

end;

function TInitForce.Process(Particule: PParticule;  const Delta: single): integer;
begin
 Particule.Vx:=Fx;
 Particule.Vy:=Fy;
 Particule.Vz:=Fz;
end;


procedure TInitForce.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 Fx:=Data.Prm[Idx];
 inc(Idx);
 Fy:=Data.Prm[Idx];
 inc(Idx);
 Fz:=Data.Prm[Idx];

 EditFx.Text:=FloatToStrF(Fx,ffFixed,7,4);
 EditFy.Text:=FloatToStrF(Fy,ffFixed,7,4);
 EditFz.Text:=FloatToStrF(Fz,ffFixed,7,4);
end;

procedure TInitForce.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,3);
 Idx:=0;

 Data.Prm[Idx]:=Fx;
 inc(Idx);
 Data.Prm[Idx]:=Fy;
 inc(Idx);
 Data.Prm[Idx]:=Fz;

end;

{ TInitRotation }

constructor TInitRotation.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_InitRotation;
 NodeName:='Init Rotation';

 EditRx:=TLabeledEdit.Create(ParentControl);
 EditRx.EditLabel.Caption:='Rotation x';
 InitControl(EditRx,@Rx);

 EditRy:=TLabeledEdit.Create(ParentControl);
 EditRy.EditLabel.Caption:='Rotation y';
 InitControl(EditRy,@Ry);

 EditRz:=TLabeledEdit.Create(ParentControl);
 EditRz.EditLabel.Caption:='Rotation z';
 InitControl(EditRz,@Rz);

end;

function TInitRotation.Process(Particule: PParticule; const Delta: single): integer;
begin
 Particule.Rx:=Rx;
 Particule.Ry:=Ry;
 Particule.Rz:=Rz;
end;

procedure TInitRotation.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 Rx:=Data.Prm[Idx];
 inc(Idx);
 Ry:=Data.Prm[Idx];
 inc(Idx);
 Rz:=Data.Prm[Idx];

 EditRx.Text:=FloatToStrF(Rx,ffFixed,7,4);
 EditRy.Text:=FloatToStrF(Ry,ffFixed,7,4);
 EditRz.Text:=FloatToStrF(Rz,ffFixed,7,4);

end;



procedure TInitRotation.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,3);
 Idx:=0;

 Data.Prm[Idx]:=Rx;
 inc(Idx);
 Data.Prm[Idx]:=Ry;
 inc(Idx);
 Data.Prm[Idx]:=Rz;

end;


{ TMoverStdMovement }

constructor TMoverStdMovement.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_MoverStdMovement;
 NodeName:='Standard Movement Computation';
end;

procedure TMoverStdMovement.LoadData(Data: TEmitterFuncData);
begin
  inherited;

end;

function TMoverStdMovement.Process(Particule: PParticule;  const Delta: single): integer;
begin
 Particule^.x:=Particule^.x+Particule^.Vx*Delta;
 Particule^.y:=Particule^.y+Particule^.Vy*Delta;
 Particule^.z:=Particule^.z+Particule^.Vz*Delta;

 
end;

procedure TMoverStdMovement.SaveData(var Data: TEmitterFuncData);
begin
  inherited;
end;

{ TMoverAttenuator }

constructor TMoverAttenuator.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_MoverAttenuator;
 NodeName:='Attenuator';

 EditForceAtt:=TLabeledEdit.Create(ParentControl);
 EditForceAtt.EditLabel.Caption:='Force Attenuation';
 InitControl(EditForceAtt,@ForceAtt);

 EditRotAtt:=TLabeledEdit.Create(ParentControl);
 EditRotAtt.EditLabel.Caption:='Rotation Attenuation';
 InitControl(EditRotAtt,@RotAtt);

end;

function TMoverAttenuator.Process(Particule: PParticule;  const Delta: single): integer;
begin
 Particule^.Vx:=Particule^.vx-(Particule^.vx*ForceAtt*Delta);
 Particule^.Vy:=Particule^.vy-(Particule^.vy*ForceAtt*Delta);
 Particule^.Vz:=Particule^.vz-(Particule^.vz*ForceAtt*Delta);

 Particule^.Rx:=Particule^.Rx-(Particule^.Rx*RotAtt*Delta);
 Particule^.Ry:=Particule^.Ry-(Particule^.Ry*RotAtt*Delta);
 Particule^.Rz:=Particule^.Rz-(Particule^.Rz*RotAtt*Delta);
end;

procedure TMoverAttenuator.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 ForceAtt:=Data.Prm[Idx];
 inc(Idx);
 RotAtt:=Data.Prm[Idx];

 EditForceAtt.Text:=FloatToStrF(ForceAtt,ffFixed,7,4);
 EditRotAtt.Text:=FloatToStrF(RotAtt,ffFixed,7,4);
end;

procedure TMoverAttenuator.SaveData(var Data: TEmitterFuncData);
begin
  inherited;

end;

{ TMoverGlobalForce }

constructor TMoverGlobalForce.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_MoverGlobalForce;
 NodeName:='Global Force';

 EditFx:=TLabeledEdit.Create(ParentControl);
 EditFx.EditLabel.Caption:='Force x';
 InitControl(EditFx,@Fx);

 EditFy:=TLabeledEdit.Create(ParentControl);
 EditFy.EditLabel.Caption:='Force y';
 InitControl(EditFy,@Fy);

 EditFz:=TLabeledEdit.Create(ParentControl);
 EditFz.EditLabel.Caption:='Force z';
 InitControl(EditFz,@Fz);
end;

function TMoverGlobalForce.Process(Particule: PParticule;Const Delta:single):integer;
begin
  with Particule^ do
  begin
   Vx:=Vx+Fx*Delta;
   Vy:=Vy+Fy*Delta;
   Vz:=Vz+Fz*Delta;
  end;
end;

procedure TMoverGlobalForce.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;

 Fx:=Data.Prm[Idx];
 inc(Idx);
 Fy:=Data.Prm[Idx];
 inc(Idx);
 Fz:=Data.Prm[Idx];

 EditFx.Text:=FloatToStrF(Fx,ffFixed,7,4);
 EditFy.Text:=FloatToStrF(Fy,ffFixed,7,4);
 EditFz.Text:=FloatToStrF(Fz,ffFixed,7,4);

end;


procedure TMoverGlobalForce.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,3);
 Idx:=0;
 Data.Prm[Idx]:=Fx;
 inc(Idx);
 Data.Prm[Idx]:=Fy;
 inc(Idx);
 Data.Prm[Idx]:=Fz;
end;

{ TFinalizerTime }

constructor TFinalizerTime.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_FinalizerTime;
 NodeName:='Delete on time';

 EditMaxTime:=TLabeledEdit.Create(ParentControl);
 EditMaxTime.EditLabel.Caption:='Max Time';
 InitControl(EditMaxTime,@MaxTime);
end;

procedure TFinalizerTime.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 MaxTime:=Data.Prm[Idx];
 inc(Idx);

 EditMaxTime.Text:=FloatToStrF(MaxTime,ffFixed,7,4);
end;

function TFinalizerTime.Process(Particule: PParticule;Const Delta:single):integer;
begin
 Result:=0;
  if Particule^.Time > maxTime then
  begin
   Result:=1;
  end;
end;

procedure TFinalizerTime.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,1);
 Idx:=0;
 Data.Prm[Idx]:=MaxTime;
end;

{ TFinalizerPlaneX }

constructor TFinalizerPlaneX.Create(ParentControl: TWinControl);
begin
 inherited;
 ObjectType:=ClassType_FinalizerPlaneX;
 NodeName:='Delete on Plane X';

 EditPosX:=TLabeledEdit.Create(ParentControl);
 EditPosX.EditLabel.Caption:='Pos X';
 InitControl(EditPosX,@PosX);
end;

function TFinalizerPlaneX.Process(Particule: PParticule;Const Delta:single):integer;
var Dist:single;
begin
  Result:=0;
  if abs(Particule^.x)>abs(PosX) then     //"destroy" particule
  begin
   Result:=1;
  end;
end;

procedure TFinalizerPlaneX.LoadData(Data: TEmitterFuncData);
var Idx:integer;
begin
 Idx:=0;
 PosX:=Data.Prm[Idx];
 inc(Idx);

 EditPosX.Text:=FloatToStrF(PosX,ffFixed,7,4);
end;


procedure TFinalizerPlaneX.SaveData(var Data: TEmitterFuncData);
var Idx:integer;
begin
 inherited;
 SetLength(Data.Prm,1);
 Idx:=0;
 Data.Prm[Idx]:=PosX;
end;


end.

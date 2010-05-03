unit uAbout;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, MGR32_Image, ExtCtrls;

type
  TFrmAbout = class(TForm)
    View: TMPaintBox32;
    Timer: TTimer;
    procedure FormHide(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure TimerTimer(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

TStar=record
       X,Y,Z:single;
       Color:Cardinal;
       Spec:integer;
end;

const MaxStars=128;

var
  FrmAbout: TFrmAbout;
  Stars:array[0..MaxStars-1] of TStar;

implementation

{$R *.dfm}

procedure InitStar(var Star:TStar);
var Alpha:Cardinal;
begin
  with Star do
    begin
     x:=random(512);
     y:=random(256);
     z:=random(10)+0.5;
     case random(100) of
      0..70:Color:= $00FFFFFF;
      71..80:Color:=$00FF6010;
      81..85:Color:=$00FF0000;
      86..90:Color:=$0000FF00;
      91..95:Color:=$000000FF;
      96..99:Color:=$00FF00FF;
     end;
     Alpha:=round(256-z*12.7);
     Color:=Color or (Alpha and $ff)shl 24;
    end;
end;

Procedure InitAllStars;
var i:longint;
begin
  for i:=0 to MaxStars-1 do
    InitStar(Stars[i]);
end;

Procedure MoveAndDrawStars;
var i:longint;
begin
  for i:=0 to MaxStars-1 do
  begin
   with Stars[i] do
   begin
    x:=x-1/z;
    if x<=0 then x:=511;
    FrmAbout.View.Buffer.SetPixelT(round(x),Round(y),Color);
   end;
  end;

end;


procedure TFrmAbout.FormHide(Sender: TObject);
begin
 Timer.Enabled:=false;
end;

procedure TFrmAbout.FormShow(Sender: TObject);
begin
 Timer.Enabled:=true;
 InitAllStars;
end;

procedure TFrmAbout.TimerTimer(Sender: TObject);
begin
 View.Buffer.Clear(0);

 MoveAndDrawStars;

 //View.Buffer.PenColor:=$ff806010;
 View.Buffer.RenderText(8,8,'Chaotikmind - 2009',1,$ffC06010);
 View.Buffer.RenderText(8,24,'Dda Editor',1,$ffC06010);
 //View.Buffer.Textout(8,8,'Chaotikmind - 2009');
//View.Buffer.Textout(8,16,'Dda Editor');


 View.Invalidate;
end;

end.

unit uPalUnit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, MGR32_Image,udda, StdCtrls,mgr32,SurfaceUtils;

type
  TFrmPal = class(TForm)
    View: TMPaintBox32;
    LblPalName: TLabel;
  private
    { Private declarations }
  public
    { Public declarations }
    Procedure ShowPal(Palette:PPalette);
  end;

var
  FrmPal: TFrmPal;

implementation

{$R *.dfm}

{ TFrmPal }

procedure TFrmPal.ShowPal(Palette: PPalette);
var i,j,Idx:longint;
begin
 LblPalName.Caption:='Name: '+Palette^.PalName;
 if View=Nil then   //prevent crash when exiting editor
  exit;

 View.Buffer.Clear(0);
 Idx:=0;
 for j:=0 to 15 do
  for i:=0 to 15 do
  begin
    View.Buffer.FillRectT(i*16,j*16,i*16+15,j*16+15,Color32(Palette^.Rgb[Idx].r,Palette^.Rgb[Idx].g,Palette^.Rgb[Idx].b));
    inc(Idx);
  end;

 View.Invalidate;
end;

end.

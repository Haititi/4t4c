unit uSurfEdit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, MGR32_Image,Udda,SurfaceUtils, StdCtrls, Menus;

type
  TFrmSurfEdit = class(TForm)
    View: TMPaintBox32;
    BtnRefresh: TButton;
    LblCoord: TLabel;
    LblColor: TLabel;
    MainMenu1: TMainMenu;
    Edit1: TMenuItem;
    Effect1: TMenuItem;
    Crop1: TMenuItem;
    ErodeBorder1: TMenuItem;
    op1: TMenuItem;
    Bottom1: TMenuItem;
    Left1: TMenuItem;
    Right1: TMenuItem;
    Resize1: TMenuItem;
    Blur1: TMenuItem;
    Sharpen1: TMenuItem;
    AutoLevels1: TMenuItem;
    LblColSel: TLabel;
    procedure FormResize(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure BtnRefreshClick(Sender: TObject);
    procedure ViewMouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure Erode(Sender: TObject);
    procedure Crop1Click(Sender: TObject);
    procedure ViewMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
  private
    Sprite:TSprite;
    OrigSprite:PSprite;

    Procedure SetPixelP8(x,y:integer;Color:Byte);
    function GetPixelP8(x,y:integer):byte;
    Procedure SetPixelA8R8G8B8(x,y:integer;Color:cardinal);
  public
    procedure SetSprite(EditSprite:PSprite);
    Procedure ReFreshView;
  end;

var
  FrmSurfEdit: TFrmSurfEdit;
  Pal:PPalette;

  SelectedColor:Cardinal;


implementation

uses globals;
{$R *.dfm}

Procedure TFrmSurfEdit.SetPixelP8(x,y:integer;Color:Byte);
var Ptr:PByte;
begin
 Ptr:=Sprite.Data;
 inc(Ptr,(y*Sprite.Width+x));
 Ptr^:=Color;
end;

function TFrmSurfEdit.GetPixelP8(x,y:integer):byte;
var Ptr:PByte;
begin
 Ptr:=Sprite.Data;
 inc(Ptr,(y*Sprite.Width+x));
 Result:=Ptr^;
end;

Procedure TFrmSurfEdit.SetPixelA8R8G8B8(x,y:integer;Color:cardinal);
var Ptr:PCardinal;
begin
 Ptr:=PCardinal(Sprite.Data);
 inc(Ptr,(y*Sprite.Width+x));
 Ptr^:=Color;
end;

procedure TFrmSurfEdit.BtnRefreshClick(Sender: TObject);
begin
 ReFreshView;
end;

procedure TFrmSurfEdit.FormResize(Sender: TObject);
begin
 View.Height:=self.ClientHeight-64;
 View.Buffer.Height:=View.Height;
 ReFreshView;
end;

procedure TFrmSurfEdit.FormShow(Sender: TObject);
begin
 ReFreshView;
end;

procedure TFrmSurfEdit.Crop1Click(Sender: TObject);
var Data:pointer;
begin    //
 GetMem(Data,Sprite.DataSize);
 Move(Sprite.Data^,Data^,Sprite.DataSize);
 CutSpriteP8(@Sprite,Data);
 ReFreshView;
end;

procedure TFrmSurfEdit.Erode(Sender: TObject);
var i:integer;
    TransCol:byte;
begin
 if Sprite.SurfFormat=SurfFormat_P8 then
 begin
  Transcol:=0;
  if Sprite.Offset<> nil then
   TransCol:=Sprite.Offset^.TransCol;
  case (sender as TMenuItem).Tag of
   0:begin     //top
      for i:=0 to Sprite.Width-1 do
       SetPixelP8(i,0,TransCol);
     end;
   1:begin     //bottom
      for i:=0 to Sprite.Width-1 do
       SetPixelP8(i,Sprite.Height-1,TransCol);
     end;
   2:begin     //left
      for i:=0 to Sprite.Height-1 do
       SetPixelP8(0,i,TransCol);
     end;
   3:begin     //right
      for i:=0 to Sprite.Height-1 do
       SetPixelP8(Sprite.Width-1,i,TransCol);
     end;
  end;
 end;

 ReFreshView;
end;

procedure TFrmSurfEdit.ReFreshView;
var i,j:longint;
    Ptr:PByte;
    Ptr32:PCardinal;
begin
 View.Buffer.clear(0);

 case Sprite.SurfFormat of
  SurfFormat_P8:
  begin

   Ptr:=Sprite.Data;
   for j:=0 to Sprite.Height-1 do
    for i:=0 to Sprite.Width-1 do
    begin
     View.Buffer.PixelS[i,j]:=MakeColor32(Pal.Rgb[Ptr^].r,Pal.Rgb[Ptr^].g,Pal.Rgb[Ptr^].b);
     inc(Ptr);
    end;
  end;

  SurfFormat_A8R8G8B8:
  begin
   Ptr32:=PCardinal(Sprite.Data);
   for j:=0 to Sprite.Height-1 do
    for i:=0 to Sprite.Width-1 do
    begin
     View.Buffer.PixelS[i,j]:=Ptr32^;
     inc(Ptr32);
    end;
  end;
 end;
 View.Invalidate;
end;

procedure TFrmSurfEdit.SetSprite(EditSprite: PSprite);
var data:pointer;
begin
 if ((Sprite.SurfFormat<>SurfFormat_P8) and (Sprite.SurfFormat<>SurfFormat_A8R8G8B8)) then
 begin
  exit;
 end;

 //keep the original pointer
 OrigSprite:=EditSprite;

 //Make a local copy (data included) TODO
 Sprite:=EditSprite^;

 Data:=UncompressSprite(@Sprite);  //that create a new pointer
 Sprite.Data:=Data; //crush the pointer
 Sprite.CompType:=Comp_NoComp;
 Sprite.DataSize:=Sprite.Width*Sprite.Height*GetFormatSize(Sprite.SurfFormat);

 if (Sprite.SurfFormat=SurfFormat_P8) then
  Pal:=Index.PalHash.SearchByName(Sprite.Offset.PaletteName);

end;

procedure TFrmSurfEdit.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
 CanClose:=true;
 if MessageBox(0,'Do you want to keep changes?','Warning !',MB_OKCANCEL)=mrOk then
 begin
  //recompress the sprite
  if OrigSprite.CompType=Comp_Zlib then
  begin
   CompressSpriteZlib(@Sprite);
  end;

  if OrigSprite.CompType=Comp_Lzma then
  begin
   CompressSpriteLzma(@Sprite);
  end;

  //crush the original sprite

  //free memory 1st
  FreeMem(OrigSprite^.Data);

  //dataCopy
  OrigSprite^:=Sprite;
 end;
end;

procedure TFrmSurfEdit.ViewMouseDown(Sender: TObject; Button: TMouseButton;  Shift: TShiftState; X, Y: Integer);
begin
 if (x<Sprite.Width) and (y<Sprite.Height) then
 begin

   if Sprite.SurfFormat=SurfFormat_P8 then
   begin
    if ssAlt in shift then
    begin
     SelectedColor:=GetPixelP8(x,y);
     LblColSel.Caption:='Selected Color : '+IntToStr(SelectedColor);
    end;

    if ssShift in Shift then
     SetPixelP8(x,y,SelectedColor);
   end;

 end;
end;

procedure TFrmSurfEdit.ViewMouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer);
var Color:Cardinal;
begin
 LblCoord.Caption:='Mouse Pos : '+IntToStr(x)+ ':'+IntToStr(y);

 if x<Sprite.Width then
  if y<Sprite.Height then
  begin
   if (Sprite.SurfFormat=SurfFormat_P8) then
   begin
    LblColor.Caption:='Color : '+IntToStr(PByte(cardinal(Sprite.Data)+y*Sprite.Width+x)^);
   end else
   if (Sprite.SurfFormat=SurfFormat_A8R8G8B8)then
   begin
    Color:=PCardinal(cardinal(Sprite.Data)+y*(Sprite.Width*4)+(x*4))^;
    LblColor.Caption:='Color : A:'+IntToStr(Color shr 24)
                            +' R:'+IntToStr((Color shr 16)and $ff)
                            +' G:'+IntToStr((Color shr 8)and $ff)
                            +' B:'+IntToStr((Color)and $ff);
   end;
  end;
end;

end.

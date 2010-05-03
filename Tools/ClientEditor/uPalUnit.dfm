object FrmPal: TFrmPal
  Left = 971
  Top = 0
  BorderStyle = bsSingle
  Caption = 'Palette Viewer'
  ClientHeight = 285
  ClientWidth = 273
  Color = clBtnFace
  TransparentColor = True
  TransparentColorValue = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesigned
  Visible = True
  PixelsPerInch = 96
  TextHeight = 13
  object LblPalName: TLabel
    Left = 8
    Top = 0
    Width = 31
    Height = 13
    Caption = 'Name:'
  end
  object View: TMPaintBox32
    Left = 8
    Top = 19
    Width = 256
    Height = 256
    TabOrder = 0
  end
end

object FrmAbout: TFrmAbout
  Left = 0
  Top = 0
  BorderStyle = bsSingle
  Caption = 'About'
  ClientHeight = 258
  ClientWidth = 514
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnHide = FormHide
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object View: TMPaintBox32
    Left = 0
    Top = 0
    Width = 514
    Height = 258
    Align = alClient
    TabOrder = 0
    ExplicitWidth = 479
    ExplicitHeight = 293
  end
  object Timer: TTimer
    Enabled = False
    Interval = 20
    OnTimer = TimerTimer
    Left = 384
    Top = 16
  end
end

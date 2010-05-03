object FrmMapCompress: TFrmMapCompress
  Left = 196
  Top = 81
  Caption = 'MapCompressor'
  ClientHeight = 313
  ClientWidth = 473
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Button1: TButton
    Left = 8
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Convert'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Memo: TMemo
    Left = 8
    Top = 64
    Width = 457
    Height = 241
    Lines.Strings = (
      'Memo')
    TabOrder = 1
  end
  object DlgOpen: TOpenDialog
    Filter = 'T4c Client Map|*.map'
    FilterIndex = 0
    Title = 'Choose map'
    Left = 96
    Top = 16
  end
end

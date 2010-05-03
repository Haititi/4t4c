object FrmSurfEdit: TFrmSurfEdit
  Left = 0
  Top = 0
  Caption = 'Surface Editor'
  ClientHeight = 414
  ClientWidth = 715
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnCloseQuery = FormCloseQuery
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object LblCoord: TLabel
    Left = 8
    Top = 8
    Width = 58
    Height = 13
    Caption = 'Mouse Pos :'
  end
  object LblColor: TLabel
    Left = 8
    Top = 24
    Width = 32
    Height = 13
    Caption = 'Color :'
  end
  object LblColSel: TLabel
    Left = 8
    Top = 40
    Width = 79
    Height = 13
    Caption = 'Selected Color : '
  end
  object View: TMPaintBox32
    Left = 0
    Top = 88
    Width = 715
    Height = 326
    Cursor = crCross
    Align = alBottom
    Constraints.MinHeight = 256
    TabOrder = 0
    OnMouseDown = ViewMouseDown
    OnMouseMove = ViewMouseMove
    ExplicitLeft = 8
  end
  object BtnRefresh: TButton
    Left = 612
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Refresh'
    TabOrder = 1
    OnClick = BtnRefreshClick
  end
  object MainMenu1: TMainMenu
    Left = 176
    object Edit1: TMenuItem
      Caption = 'Edit'
      object Crop1: TMenuItem
        Caption = 'Crop'
        OnClick = Crop1Click
      end
      object ErodeBorder1: TMenuItem
        Caption = 'Erode Border'
        object op1: TMenuItem
          Caption = 'Top'
          OnClick = Erode
        end
        object Bottom1: TMenuItem
          Tag = 1
          Caption = 'Bottom'
          OnClick = Erode
        end
        object Left1: TMenuItem
          Tag = 2
          Caption = 'Left'
          OnClick = Erode
        end
        object Right1: TMenuItem
          Tag = 3
          Caption = 'Right'
          OnClick = Erode
        end
      end
      object Resize1: TMenuItem
        Caption = 'Resize'
      end
    end
    object Effect1: TMenuItem
      Caption = 'Effect'
      object Blur1: TMenuItem
        Caption = 'Blur'
      end
      object Sharpen1: TMenuItem
        Caption = 'Sharpen'
      end
      object AutoLevels1: TMenuItem
        Caption = 'Auto Levels'
      end
    end
  end
end

object FrmMIdEd: TFrmMIdEd
  Left = 0
  Top = 0
  BorderStyle = bsSingle
  Caption = 'Map ID Editor'
  ClientHeight = 721
  ClientWidth = 1016
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 34
    Height = 13
    Caption = 'Map ID'
  end
  object Label2: TLabel
    Left = 8
    Top = 88
    Width = 29
    Height = 13
    Caption = 'Mod X'
  end
  object Label3: TLabel
    Left = 8
    Top = 136
    Width = 29
    Height = 13
    Caption = 'Mod Y'
  end
  object LblSpriteCount: TLabel
    Left = 8
    Top = 182
    Width = 67
    Height = 13
    Caption = 'Sprite Count :'
  end
  object Label4: TLabel
    Left = 135
    Top = 88
    Width = 37
    Height = 13
    Caption = 'Id Type'
  end
  object Label5: TLabel
    Left = 135
    Top = 136
    Width = 23
    Height = 13
    Caption = 'Light'
  end
  object Label6: TLabel
    Left = 262
    Top = 8
    Width = 47
    Height = 13
    Caption = 'Sprite List'
  end
  object Label7: TLabel
    Left = 135
    Top = 190
    Width = 66
    Height = 13
    Caption = 'MiniMap Color'
  end
  object Label12: TLabel
    Left = 8
    Top = 221
    Width = 40
    Height = 13
    Caption = 'Color FX'
  end
  object SpinID: TSpinEdit
    Left = 8
    Top = 27
    Width = 73
    Height = 22
    MaxValue = 8191
    MinValue = 0
    TabOrder = 0
    Value = 0
    OnChange = SpinIDChange
  end
  object EdtModX: TEdit
    Left = 8
    Top = 107
    Width = 113
    Height = 21
    TabOrder = 1
    OnChange = EdtModXChange
  end
  object ChkDefined: TCheckBox
    Left = 8
    Top = 65
    Width = 121
    Height = 17
    Caption = 'Defined'
    TabOrder = 2
    OnClick = ChkDefinedClick
  end
  object EdtModY: TEdit
    Left = 8
    Top = 155
    Width = 113
    Height = 21
    TabOrder = 3
    OnChange = EdtModYChange
  end
  object View: TMPaintBox32
    Left = 477
    Top = 27
    Width = 512
    Height = 518
    TabOrder = 4
  end
  object ComboIdType: TComboBox
    Left = 135
    Top = 107
    Width = 121
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 5
    Text = 'Ground'
    OnChange = ComboIdTypeChange
    Items.Strings = (
      'Ground'
      'Blend'
      'Single'
      'Multi'
      'Composite'
      'Animated')
  end
  object ComboLight: TComboBox
    Left = 135
    Top = 155
    Width = 121
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 6
    Text = 'No shadowing'
    OnChange = ComboLightChange
    Items.Strings = (
      'No shadowing'
      'Dynamic Shadowing'
      'Block Light'
      'Cast Omni Light')
  end
  object BtnNextDefined: TButton
    Left = 87
    Top = 27
    Width = 34
    Height = 25
    Hint = 'Next Defined Id'
    Caption = 'Next'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 7
    OnClick = BtnNextDefinedClick
  end
  object BtnCopyTo: TButton
    Left = 216
    Top = 27
    Width = 37
    Height = 25
    Caption = 'Copy To'
    TabOrder = 8
  end
  object LstGraph: TListBox
    Left = 262
    Top = 27
    Width = 209
    Height = 552
    DragMode = dmAutomatic
    ItemHeight = 13
    PopupMenu = PopupMenu1
    TabOrder = 9
    OnClick = LstGraphClick
    OnDragDrop = LstGraphDragDrop
    OnDragOver = LstGraphDragOver
  end
  object EdtMiniMapColor: TEdit
    Left = 135
    Top = 209
    Width = 121
    Height = 21
    TabOrder = 10
    OnChange = EdtMiniMapColorChange
  end
  object ViewMapColor: TMPaintBox32
    Left = 135
    Top = 236
    Width = 121
    Height = 25
    Hint = 'Click With Right Button to automatically choose the color'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 11
    OnDblClick = ViewMapColorDblClick
  end
  object BtnClearList: TButton
    Left = 178
    Top = 58
    Width = 75
    Height = 25
    Caption = 'Clear List ->'
    TabOrder = 12
    OnClick = BtnClearListClick
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 282
    Width = 248
    Height = 297
    Caption = 'Sprite Parameter'
    TabOrder = 13
    object Label8: TLabel
      Left = 11
      Top = 21
      Width = 40
      Height = 13
      Caption = 'Offset X'
    end
    object Label9: TLabel
      Left = 11
      Top = 69
      Width = 40
      Height = 13
      Caption = 'Offset Y'
    end
    object Label10: TLabel
      Left = 11
      Top = 117
      Width = 40
      Height = 13
      Caption = 'Offset Z'
    end
    object Label11: TLabel
      Left = 127
      Top = 21
      Width = 60
      Height = 13
      Caption = 'Vertex Color'
    end
    object EdtOffX: TEdit
      Left = 11
      Top = 40
      Width = 86
      Height = 21
      TabOrder = 0
      OnChange = EdtOffXChange
    end
    object EdtOffY: TEdit
      Left = 11
      Top = 88
      Width = 86
      Height = 21
      TabOrder = 1
      OnChange = EdtOffYChange
    end
    object EdtOffZ: TEdit
      Left = 11
      Top = 136
      Width = 86
      Height = 21
      TabOrder = 2
      OnChange = EdtOffZChange
    end
    object EdtVertexCol: TEdit
      Left = 127
      Top = 40
      Width = 86
      Height = 21
      TabOrder = 3
      OnChange = EdtVertexColChange
    end
    object BtnAutoOff: TButton
      Left = 11
      Top = 176
      Width = 153
      Height = 25
      Caption = 'Extract Offset from Old Data'
      TabOrder = 4
      OnClick = BtnAutoOffClick
    end
  end
  object ChkFlip: TCheckBox
    Left = 8
    Top = 201
    Width = 97
    Height = 17
    Caption = 'Horizontal Flip'
    TabOrder = 14
    OnClick = ChkFlipClick
  end
  object ComboColFx: TComboBox
    Left = 8
    Top = 240
    Width = 121
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 15
    Text = 'Modulate'
    Items.Strings = (
      'Modulate'
      'Add'
      'Substract')
  end
  object ChkGrid: TCheckBox
    Left = 477
    Top = 551
    Width = 97
    Height = 17
    Caption = 'Show Grid'
    TabOrder = 16
  end
  object BtnTestFloor: TButton
    Left = 580
    Top = 551
    Width = 75
    Height = 25
    Caption = 'Floor Preview'
    TabOrder = 17
    OnClick = BtnTestFloorClick
  end
  object BtnTemp1: TButton
    Left = 669
    Top = 551
    Width = 196
    Height = 25
    Caption = 'Danger Danger Temporary don'#39't click '
    TabOrder = 18
    OnClick = BtnTemp1Click
  end
  object BtnNxtNonBlend: TButton
    Left = 127
    Top = 25
    Width = 31
    Height = 25
    Hint = 'Next Non Blend ID'
    Caption = 'Next'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 19
    OnClick = BtnNxtNonBlendClick
  end
  object BtnNextFloorId: TButton
    Left = 164
    Top = 27
    Width = 34
    Height = 25
    Hint = 'Next Floor Id'
    Caption = 'Next'
    TabOrder = 20
    OnClick = BtnNextFloorIdClick
  end
  object PopupMenu1: TPopupMenu
    Left = 400
    object Sort1: TMenuItem
      Caption = 'Sort Ground'
      OnClick = Sort1Click
    end
    object SortGroundinverse1: TMenuItem
      Caption = 'Sort Ground inverse'
      OnClick = SortGroundinverse1Click
    end
    object Sort2: TMenuItem
      Caption = 'Sort'
      OnClick = Sort2Click
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object RemoveFromList1: TMenuItem
      Caption = 'Remove From List'
      OnClick = RemoveFromList1Click
    end
  end
  object MainMenu1: TMainMenu
    Left = 160
    object File1: TMenuItem
      Caption = 'File'
      object Load1: TMenuItem
        Caption = 'Load'
        OnClick = Load1Click
      end
      object Save1: TMenuItem
        Caption = 'Save'
        OnClick = Save1Click
      end
    end
    object ools1: TMenuItem
      Caption = 'Tools'
      object GenerateFromNMIni1: TMenuItem
        Caption = 'Generate ID From NM Ini'
      end
      object GenerateIDFromExtractedIdList1: TMenuItem
        Caption = 'Generate ID From Extracted Id List'
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object GenerateMinimapMiniaturetexture1: TMenuItem
        Caption = 'Generate Minimap Miniature texture'
        OnClick = GenerateMinimapMiniaturetexture1Click
      end
      object N4: TMenuItem
        Caption = '-'
      end
      object CorrectALLoffsetsDanger1: TMenuItem
        Caption = 'Correct ALL offsets (Danger)'
        OnClick = CorrectALLoffsetsDanger1Click
      end
      object N5: TMenuItem
        Caption = '-'
      end
      object FuseTiles1: TMenuItem
        Caption = 'Fuse Tiles'
        OnClick = FuseTiles1Click
      end
      object FusetilesCAUTION1: TMenuItem
        Caption = 'Fuse tiles and Delete (CAUTION)'
        OnClick = FusetilesCAUTION1Click
      end
    end
  end
  object DlgOpen: TOpenDialog
    Filter = 'Map Id File|*.dat'
    Title = 'Open Map Id file'
    Left = 88
  end
  object DlgSave: TSaveDialog
    Filter = 'Map Id File|*.dat'
    Title = 'Save Map Id file'
    Left = 120
  end
  object DlgColor: TColorDialog
    Options = [cdFullOpen, cdAnyColor]
    Left = 144
    Top = 56
  end
end

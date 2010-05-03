object FrmSkinEdit: TFrmSkinEdit
  Left = 0
  Top = 0
  Caption = 'Monster  Skin Id Editor'
  ClientHeight = 631
  ClientWidth = 829
  Color = clBtnFace
  Constraints.MinHeight = 540
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 0
    Top = 8
    Width = 38
    Height = 13
    Caption = 'Skin List'
  end
  object Label2: TLabel
    Left = 152
    Top = 32
    Width = 49
    Height = 13
    Caption = 'Skin Name'
  end
  object Label3: TLabel
    Left = 152
    Top = 80
    Width = 32
    Height = 13
    Caption = 'Skin Id'
  end
  object Label5: TLabel
    Left = 344
    Top = 8
    Width = 30
    Height = 13
    Caption = 'Action'
  end
  object Label6: TLabel
    Left = 344
    Top = 132
    Width = 42
    Height = 13
    Caption = 'Direction'
  end
  object Label7: TLabel
    Left = 151
    Top = 125
    Width = 101
    Height = 13
    Caption = 'Animation Delay (Ms)'
  end
  object Label8: TLabel
    Left = 407
    Top = 8
    Width = 47
    Height = 13
    Caption = 'Sprite List'
  end
  object Label9: TLabel
    Left = 151
    Top = 221
    Width = 60
    Height = 13
    Caption = 'Vertex Color'
  end
  object Label4: TLabel
    Left = 153
    Top = 171
    Width = 40
    Height = 13
    Caption = 'Color Fx'
  end
  object Label10: TLabel
    Left = 151
    Top = 267
    Width = 63
    Height = 13
    Caption = 'Skin Size (%)'
  end
  object LstSkin: TListBox
    Left = 0
    Top = 27
    Width = 145
    Height = 503
    ItemHeight = 13
    PopupMenu = PopupSkinList
    Sorted = True
    TabOrder = 0
    OnClick = LstSkinClick
  end
  object EdtName: TEdit
    Left = 151
    Top = 53
    Width = 121
    Height = 21
    TabOrder = 1
    OnChange = EdtNameChange
  end
  object EdtId: TEdit
    Left = 151
    Top = 99
    Width = 121
    Height = 21
    TabOrder = 2
    OnChange = EdtIdChange
  end
  object LstAction: TListBox
    Left = 344
    Top = 27
    Width = 57
    Height = 97
    ItemHeight = 13
    Items.Strings = (
      'Walk'
      'Attack'
      'Death')
    TabOrder = 3
    OnClick = LstActionClick
  end
  object LstDir: TListBox
    Left = 344
    Top = 151
    Width = 57
    Height = 110
    ItemHeight = 13
    Items.Strings = (
      '000'
      '045'
      '090'
      '135'
      '180'
      '225'
      '270'
      '315')
    TabOrder = 4
    OnClick = LstDirClick
  end
  object EdtAnimDelay: TEdit
    Left = 151
    Top = 144
    Width = 121
    Height = 21
    TabOrder = 5
    OnChange = EdtAnimDelayChange
  end
  object LstSprite: TListBox
    Left = 407
    Top = 27
    Width = 121
    Height = 503
    ItemHeight = 13
    PopupMenu = PopupSpriteList
    TabOrder = 6
    OnClick = LstSpriteClick
    OnDragDrop = LstSpriteDragDrop
    OnDragOver = LstSpriteDragOver
  end
  object View: TMPaintBox32
    Left = 544
    Top = 27
    Width = 277
    Height = 503
    Constraints.MinHeight = 128
    Constraints.MinWidth = 128
    TabOrder = 7
  end
  object EdtVtxColor: TEdit
    Left = 151
    Top = 240
    Width = 121
    Height = 21
    TabOrder = 8
    OnChange = EdtVtxColorChange
  end
  object GroupBox1: TGroupBox
    Left = 151
    Top = 310
    Width = 250
    Height = 131
    Caption = 'Frame settings'
    TabOrder = 9
    object Label11: TLabel
      Left = 13
      Top = 29
      Width = 40
      Height = 13
      Caption = 'Offset X'
    end
    object Label12: TLabel
      Left = 13
      Top = 77
      Width = 40
      Height = 13
      Caption = 'Offset Y'
    end
    object EdtOffX: TEdit
      Left = 13
      Top = 48
      Width = 121
      Height = 21
      TabOrder = 0
    end
    object EdtOffY: TEdit
      Left = 13
      Top = 96
      Width = 121
      Height = 21
      TabOrder = 1
    end
  end
  object ChkReverse: TCheckBox
    Left = 329
    Top = 267
    Width = 72
    Height = 39
    Caption = 'Reversed direction'
    TabOrder = 10
    WordWrap = True
  end
  object Button1: TButton
    Left = 544
    Top = 8
    Width = 81
    Height = 13
    Caption = 'Test Animation'
    TabOrder = 11
  end
  object Button2: TButton
    Left = 631
    Top = 8
    Width = 81
    Height = 13
    Caption = 'Stop Animation'
    TabOrder = 12
  end
  object ComboColorFx: TComboBox
    Left = 151
    Top = 190
    Width = 121
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 13
    Text = 'Modulate'
    OnChange = ComboColorFxChange
    Items.Strings = (
      'Modulate'
      'Add'
      'Sub')
  end
  object EdtMonsterSize: TEdit
    Left = 151
    Top = 285
    Width = 121
    Height = 21
    TabOrder = 14
    OnChange = EdtMonsterSizeChange
  end
  object PopupSpriteList: TPopupMenu
    Left = 488
    Top = 8
    object AutoSort1: TMenuItem
      Caption = 'AutoSort'
    end
    object N3: TMenuItem
      Caption = '-'
    end
    object ClearList1: TMenuItem
      Caption = 'Clear List'
      OnClick = ClearList1Click
    end
  end
  object MainMenu1: TMainMenu
    Left = 48
    object File1: TMenuItem
      Caption = 'File'
      object LoadMonsterSkins1: TMenuItem
        Caption = 'Load Monster Skins'
        OnClick = LoadMonsterSkins1Click
      end
      object SAveMonsters1: TMenuItem
        Caption = 'Save Monster Skins'
        OnClick = SAveMonsters1Click
      end
    end
  end
  object DlgOpen: TOpenDialog
    Left = 232
    Top = 8
  end
  object DlgSave: TSaveDialog
    Left = 264
    Top = 8
  end
  object PopupSkinList: TPopupMenu
    Left = 96
    object AddSkin1: TMenuItem
      Caption = 'Add Skin'
      OnClick = AddSkin1Click
    end
    object Duplicateskin1: TMenuItem
      Caption = 'Duplicate skin'
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object Sort1: TMenuItem
      Caption = 'Sort'
      Checked = True
      OnClick = Sort1Click
    end
    object AutoFill1: TMenuItem
      Caption = 'AutoFill'
      OnClick = AutoFill1Click
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object DeleteSkin1: TMenuItem
      Caption = 'Delete Skin'
      OnClick = DeleteSkin1Click
    end
  end
end

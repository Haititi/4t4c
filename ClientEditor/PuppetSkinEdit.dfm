object FrmPuppetSkinEdit: TFrmPuppetSkinEdit
  Left = 0
  Top = 0
  Caption = 'Puppet Skin Id Editor'
  ClientHeight = 662
  ClientWidth = 973
  Color = clBtnFace
  Constraints.MinHeight = 540
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poDesigned
  OnCreate = FormCreate
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object Label3: TLabel
    Left = 137
    Top = 76
    Width = 32
    Height = 13
    Caption = 'Skin Id'
  end
  object Label5: TLabel
    Left = 264
    Top = 80
    Width = 30
    Height = 13
    Caption = 'Action'
  end
  object Label6: TLabel
    Left = 264
    Top = 204
    Width = 42
    Height = 13
    Caption = 'Direction'
  end
  object Label7: TLabel
    Left = 137
    Top = 148
    Width = 101
    Height = 13
    Caption = 'Animation Delay (Ms)'
  end
  object Label8: TLabel
    Left = 327
    Top = 8
    Width = 47
    Height = 13
    Caption = 'Sprite List'
  end
  object Label9: TLabel
    Left = 137
    Top = 249
    Width = 60
    Height = 13
    Caption = 'Vertex Color'
  end
  object Label4: TLabel
    Left = 139
    Top = 203
    Width = 40
    Height = 13
    Caption = 'Color Fx'
  end
  object Label10: TLabel
    Left = 8
    Top = 8
    Width = 38
    Height = 13
    Caption = 'Skin List'
  end
  object Label13: TLabel
    Left = 137
    Top = 30
    Width = 49
    Height = 13
    Caption = 'Skin Name'
  end
  object LstSkinPart: TListBox
    Tag = 2
    Left = 8
    Top = 27
    Width = 123
    Height = 503
    ItemHeight = 13
    PopupMenu = PopupSkinPartList
    Sorted = True
    TabOrder = 0
    OnClick = LstSkinPartClick
  end
  object EdtSkinId: TEdit
    Left = 137
    Top = 95
    Width = 111
    Height = 21
    TabOrder = 1
    OnChange = EdtSkinIdChange
  end
  object LstAction: TListBox
    Left = 264
    Top = 99
    Width = 57
    Height = 97
    ItemHeight = 13
    Items.Strings = (
      'Walk'
      'Attack'
      'Death'
      'Range')
    TabOrder = 2
    OnClick = LstActionClick
  end
  object LstDir: TListBox
    Left = 264
    Top = 223
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
    TabOrder = 3
    OnClick = LstDirClick
  end
  object EdtAnimDelay: TEdit
    Left = 137
    Top = 167
    Width = 121
    Height = 21
    TabOrder = 4
    OnChange = EdtAnimDelayChange
  end
  object LstSprite: TListBox
    Tag = 30
    Left = 327
    Top = 27
    Width = 121
    Height = 503
    ItemHeight = 13
    PopupMenu = PopupSpriteList
    TabOrder = 5
    OnClick = LstSpriteClick
    OnDragDrop = LstSpriteDragDrop
    OnDragOver = LstSpriteDragOver
  end
  object View: TMPaintBox32
    Left = 464
    Top = 27
    Width = 277
    Height = 503
    Constraints.MinHeight = 128
    Constraints.MinWidth = 128
    TabOrder = 6
  end
  object EdtVtxColor: TEdit
    Left = 137
    Top = 272
    Width = 121
    Height = 21
    TabOrder = 7
    OnChange = EdtVtxColorChange
  end
  object GroupBox1: TGroupBox
    Left = 137
    Top = 399
    Width = 162
    Height = 131
    Caption = 'Frame settings'
    TabOrder = 8
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
    Left = 249
    Top = 339
    Width = 72
    Height = 39
    Caption = 'Reversed direction'
    TabOrder = 9
    WordWrap = True
  end
  object Button1: TButton
    Left = 464
    Top = 8
    Width = 81
    Height = 13
    Caption = 'Test Animation'
    TabOrder = 10
  end
  object Button2: TButton
    Left = 551
    Top = 8
    Width = 81
    Height = 13
    Caption = 'Stop Animation'
    TabOrder = 11
  end
  object ComboColorFx: TComboBox
    Left = 137
    Top = 222
    Width = 121
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 12
    Text = 'Modulate'
    OnChange = ComboColorFxChange
    Items.Strings = (
      'Modulate'
      'Add'
      'Sub')
  end
  object EdtSkinName: TEdit
    Left = 137
    Top = 49
    Width = 111
    Height = 21
    TabOrder = 13
    OnChange = EdtNameChange
  end
  object ChkFemale: TCheckBox
    Left = 137
    Top = 122
    Width = 60
    Height = 17
    Caption = 'Female'
    TabOrder = 14
    OnClick = ChkFemaleClick
  end
  object PopupSpriteList: TPopupMenu
    Left = 392
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
    Left = 240
    object File1: TMenuItem
      Caption = 'File'
      object LoadMonsterSkins1: TMenuItem
        Caption = 'Load Puppet Skins'
        OnClick = LoadPuppetSkins1Click
      end
      object SAveMonsters1: TMenuItem
        Caption = 'Save Puppet Skins'
        OnClick = SavePuppet1Click
      end
    end
    object Options1: TMenuItem
      Caption = 'Options'
      object SetMaxAutoFillSearch1: TMenuItem
        Caption = 'Set Max AutoFill Search'
        OnClick = SetMaxAutoFillSearch1Click
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
    end
  end
  object DlgOpen: TOpenDialog
    Left = 424
  end
  object DlgSave: TSaveDialog
    Left = 456
  end
  object PopupSkinPartList: TPopupMenu
    Left = 80
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

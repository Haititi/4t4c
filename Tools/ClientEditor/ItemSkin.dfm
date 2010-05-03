object FrmItemSkinEd: TFrmItemSkinEd
  Left = 0
  Top = 0
  Caption = 'Item Skin Editor'
  ClientHeight = 647
  ClientWidth = 710
  Color = clBtnFace
  Constraints.MinHeight = 550
  Constraints.MinWidth = 710
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
    Left = 8
    Top = 8
    Width = 49
    Height = 13
    Caption = 'Item Skins'
  end
  object Label2: TLabel
    Left = 192
    Top = 24
    Width = 33
    Height = 13
    Caption = 'Skin ID'
  end
  object Label3: TLabel
    Left = 192
    Top = 80
    Width = 49
    Height = 13
    Caption = 'Skin Name'
  end
  object Label4: TLabel
    Left = 336
    Top = 8
    Width = 16
    Height = 13
    Caption = 'List'
  end
  object Label5: TLabel
    Left = 192
    Top = 181
    Width = 63
    Height = 13
    Caption = 'MouseCursor'
  end
  object Label6: TLabel
    Left = 192
    Top = 133
    Width = 49
    Height = 13
    Caption = 'Item Type'
  end
  object Label7: TLabel
    Left = 194
    Top = 405
    Width = 110
    Height = 13
    Caption = 'Use/Open/Take  sound'
  end
  object Label8: TLabel
    Left = 194
    Top = 461
    Width = 114
    Height = 13
    Caption = 'Use/Close/Drop Sound  '
  end
  object Label9: TLabel
    Left = 192
    Top = 232
    Width = 40
    Height = 13
    Caption = 'Color Fx'
  end
  object Label10: TLabel
    Left = 192
    Top = 280
    Width = 48
    Height = 13
    Caption = 'Color Mod'
  end
  object Label11: TLabel
    Left = 192
    Top = 333
    Width = 100
    Height = 13
    Caption = 'Animation delay (ms)'
  end
  object LstItemSkin: TListBox
    Left = 8
    Top = 25
    Width = 161
    Height = 614
    ItemHeight = 13
    PopupMenu = PopupSkinList
    TabOrder = 0
    OnClick = LstItemSkinClick
  end
  object EdtSkinId: TEdit
    Left = 192
    Top = 43
    Width = 121
    Height = 21
    TabOrder = 1
    OnChange = EdtSkinIdChange
  end
  object EdtSkinName: TEdit
    Left = 192
    Top = 99
    Width = 121
    Height = 21
    TabOrder = 2
    OnChange = EdtSkinNameChange
  end
  object LstElem: TListBox
    Left = 336
    Top = 27
    Width = 121
    Height = 611
    Hint = 
      'For a standard item please put first the ground sprite and secon' +
      'd the inventory sprite, ON/OFF item can have any number of frame' +
      's'
    ItemHeight = 13
    PopupMenu = PopupElem
    TabOrder = 3
    OnClick = LstElemClick
    OnDragDrop = LstElemDragDrop
    OnDragOver = LstElemDragOver
  end
  object ComboType: TComboBox
    Left = 192
    Top = 152
    Width = 121
    Height = 21
    ItemHeight = 13
    TabOrder = 4
    Text = 'Standard Item (Weapons,Armors,Monsters Drops,...)'
    OnChange = ComboTypeChange
    Items.Strings = (
      'Standard Item (Weapons,Armors,Monsters Drops,...)'
      'On/Off Item (Doors,Switches,Chests)'
      'Animated Item (Signs,Troll Cauldron,...)'
      '')
  end
  object ComboMouseCursor: TComboBox
    Left = 192
    Top = 200
    Width = 123
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 5
    Text = 'Use'
    OnChange = ComboMouseCursorChange
    Items.Strings = (
      'Use'
      'Take'
      'See'
      'Nothing')
  end
  object View: TMPaintBox32
    Left = 480
    Top = 24
    Width = 302
    Height = 241
    Constraints.MinHeight = 128
    Constraints.MinWidth = 128
    TabOrder = 6
  end
  object EdtSoundOpen: TEdit
    Left = 192
    Top = 424
    Width = 121
    Height = 21
    TabOrder = 7
    OnChange = EdtSoundOpenChange
  end
  object EdtSoundClose: TEdit
    Left = 192
    Top = 480
    Width = 121
    Height = 21
    TabOrder = 8
    OnChange = EdtSoundCloseChange
  end
  object EdtColorMod: TEdit
    Left = 192
    Top = 299
    Width = 121
    Height = 21
    TabOrder = 9
    OnChange = EdtColorModChange
  end
  object ComboColorFx: TComboBox
    Left = 192
    Top = 251
    Width = 123
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 10
    Text = 'Modulate'
    OnChange = ComboColorFxChange
    Items.Strings = (
      'Modulate'
      'Add'
      'Sub')
  end
  object EdtAnimDelay: TEdit
    Left = 192
    Top = 352
    Width = 121
    Height = 21
    TabOrder = 11
    OnChange = EdtAnimDelayChange
  end
  object GroupBox1: TGroupBox
    Left = 192
    Top = 520
    Width = 138
    Height = 118
    Caption = 'Frame Settings'
    TabOrder = 12
    object Label12: TLabel
      Left = 16
      Top = 18
      Width = 40
      Height = 13
      Caption = 'Offset X'
    end
    object Label13: TLabel
      Left = 16
      Top = 64
      Width = 40
      Height = 13
      Caption = 'Offset Y'
    end
    object EdtOffX: TEdit
      Left = 16
      Top = 37
      Width = 94
      Height = 21
      TabOrder = 0
    end
    object EdtOffY: TEdit
      Left = 16
      Top = 83
      Width = 94
      Height = 21
      TabOrder = 1
    end
  end
  object MainMenu1: TMainMenu
    Left = 72
    object Files1: TMenuItem
      Caption = 'Files'
      object LoadItemSkins1: TMenuItem
        Caption = 'Load Item Skins'
        OnClick = LoadItemSkins1Click
      end
      object SaveItemSkins1: TMenuItem
        Caption = 'Save Item Skins '
        OnClick = SaveItemSkins1Click
      end
      object N3: TMenuItem
        Caption = '-'
      end
      object ClearAll1: TMenuItem
        Caption = 'Clear All'
        OnClick = ClearAll1Click
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
    end
  end
  object PopupSkinList: TPopupMenu
    Left = 128
    object AddNewID1: TMenuItem
      Caption = 'Add New ID'
      OnClick = AddNewID1Click
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object DuplicateID1: TMenuItem
      Caption = 'Duplicate ID'
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object DeleteID1: TMenuItem
      Caption = 'Delete ID'
      OnClick = DeleteID1Click
    end
  end
  object DlgOpen: TOpenDialog
    Left = 256
    Top = 8
  end
  object DlgSave: TSaveDialog
    Left = 296
    Top = 8
  end
  object PopupElem: TPopupMenu
    Left = 408
    Top = 8
    object DeleteSprite1: TMenuItem
      Caption = 'Delete Sprite'
      OnClick = DeleteSprite1Click
    end
    object N4: TMenuItem
      Caption = '-'
    end
    object ClearAllSprites1: TMenuItem
      Caption = 'Clear All Sprites'
      OnClick = ClearAllSprites1Click
    end
  end
end

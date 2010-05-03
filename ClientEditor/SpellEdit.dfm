object FrmSpellEd: TFrmSpellEd
  Left = 0
  Top = 0
  Caption = 'Spell Editor'
  ClientHeight = 566
  ClientWidth = 787
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 41
    Height = 13
    Caption = 'Spell List'
  end
  object Label2: TLabel
    Left = 240
    Top = 24
    Width = 36
    Height = 13
    Caption = 'Spell ID'
  end
  object Label3: TLabel
    Left = 240
    Top = 72
    Width = 52
    Height = 13
    Caption = 'Spell Name'
  end
  object Label9: TLabel
    Left = 240
    Top = 123
    Width = 77
    Height = 13
    Caption = 'MaxTime (msec)'
  end
  object EdtSpellId: TEdit
    Tag = 1
    Left = 240
    Top = 43
    Width = 121
    Height = 21
    TabOrder = 1
    OnChange = EdtSpellInfoChange
  end
  object EdtSpellName: TEdit
    Tag = 2
    Left = 240
    Top = 91
    Width = 121
    Height = 21
    TabOrder = 2
    OnChange = EdtSpellInfoChange
  end
  object GrpBoxEmitSettings: TGroupBox
    Left = 223
    Top = 169
    Width = 154
    Height = 248
    Caption = 'Emitter Settings'
    TabOrder = 3
    object Label6: TLabel
      Left = 16
      Top = 119
      Width = 55
      Height = 13
      Caption = 'Emitter Link'
    end
    object Label13: TLabel
      Left = 17
      Top = 168
      Width = 67
      Height = 13
      Caption = 'Emission Type'
    end
    object Label7: TLabel
      Left = 16
      Top = 24
      Width = 28
      Height = 13
      Caption = 'Sprite'
    end
    object Label8: TLabel
      Left = 16
      Top = 69
      Width = 52
      Height = 13
      Caption = 'BlendMode'
    end
    object ComboEmitLink: TComboBox
      Left = 16
      Top = 138
      Width = 121
      Height = 21
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 0
      Text = 'Link To Source'
      Items.Strings = (
        'Link To Source'
        'Link To Target'
        'Link To Ground')
    end
    object ComboEmissionType: TComboBox
      Left = 17
      Top = 187
      Width = 120
      Height = 21
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 1
      Text = 'Homogenous'
      Items.Strings = (
        'Homogenous'
        'Cyclic'
        'Burst (one shot with all particules)'
        'One (only one particule)')
    end
    object EdtPartSprite: TEdit
      Left = 16
      Top = 43
      Width = 121
      Height = 21
      TabOrder = 2
      OnChange = EdtPartSpriteChange
    end
    object ComboBlendMode: TComboBox
      Left = 16
      Top = 88
      Width = 121
      Height = 21
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 3
      Text = 'Normal'
      Items.Strings = (
        'Normal'
        'Add'
        'Sub')
    end
    object ChkActive: TCheckBox
      Left = 16
      Top = 224
      Width = 57
      Height = 17
      Caption = 'Active'
      TabOrder = 4
    end
  end
  object EdtMaxTime: TEdit
    Tag = 3
    Left = 239
    Top = 142
    Width = 121
    Height = 21
    Hint = 'The spell will die after that time'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 4
    OnChange = EdtSpellInfoChange
  end
  object TreeSpell: TTreeView
    Left = 8
    Top = 27
    Width = 209
    Height = 531
    Indent = 19
    PopupMenu = PopupSpellList
    ReadOnly = True
    RightClickSelect = True
    TabOrder = 0
    OnClick = TreeSpellClick
  end
  object PanelCtrl: TPanel
    Left = 424
    Top = 23
    Width = 355
    Height = 535
    TabOrder = 5
  end
  object PopupSpellList: TPopupMenu
    Left = 80
    object MnuItAddSpell1: TMenuItem
      Caption = 'Add Spell'
      OnClick = MnuItAddSpell1Click
    end
    object MnuItDuplicateSpell1: TMenuItem
      Caption = 'Duplicate Spell'
    end
    object MnuItPreviewSpell1: TMenuItem
      Caption = 'Preview Spell'
    end
    object MnuItDeleteSpell1: TMenuItem
      Caption = 'Delete Spell'
    end
    object N3: TMenuItem
      Caption = '-'
    end
    object MnuItAddEmitter2: TMenuItem
      Caption = 'Add Emitter'
      OnClick = MnuItAddEmitter2Click
    end
    object MnuItDuplicateEmitter2: TMenuItem
      Caption = 'Duplicate Emitter'
    end
    object MnuItDeleteEmitter2: TMenuItem
      Caption = 'Delete Emitter'
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object MnuItAddInitializer1: TMenuItem
      Caption = 'Add Initializer'
      object InitPoint1: TMenuItem
        Caption = 'Init Point'
        OnClick = AddInitClick
      end
      object InitLine1: TMenuItem
        Tag = 1
        Caption = 'Init Line'
        OnClick = AddInitClick
      end
      object INitCircle1: TMenuItem
        Tag = 2
        Caption = 'Init Circle'
        OnClick = AddInitClick
      end
      object InitCircleaera1: TMenuItem
        Tag = 3
        Caption = 'Init Disc'
        OnClick = AddInitClick
      end
      object InitSquarearea1: TMenuItem
        Tag = 4
        Caption = 'Init Square'
        OnClick = AddInitClick
      end
      object InitSphere1: TMenuItem
        Tag = 5
        Caption = 'Init Sphere'
        OnClick = AddInitClick
      end
      object InitCube1: TMenuItem
        Tag = 6
        Caption = 'Init Cube'
        OnClick = AddInitClick
      end
      object ForceVelocity1: TMenuItem
        Tag = 7
        Caption = 'Force Velocity'
        OnClick = AddInitClick
      end
      object ForceRotation1: TMenuItem
        Tag = 8
        Caption = 'Force Rotation'
        OnClick = AddInitClick
      end
      object RandomColor1: TMenuItem
        Tag = 9
        Caption = 'Random Color'
        OnClick = AddInitClick
      end
      object RadomPosDeviation1: TMenuItem
        Tag = 10
        Caption = 'Random Pos Deviation'
        OnClick = AddInitClick
      end
    end
    object MnuItAddMover1: TMenuItem
      Caption = 'Add Mover'
      object Standardparticulesmover1: TMenuItem
        Caption = 'Standard particules Movement'
        OnClick = AddMoverClick
      end
      object MovementAttenuator1: TMenuItem
        Tag = 1
        Caption = 'Force/rotation Attenuator'
        OnClick = AddMoverClick
      end
      object GlobalForce1: TMenuItem
        Tag = 2
        Caption = 'Global Force'
        OnClick = AddMoverClick
      end
      object Gravitywell1: TMenuItem
        Tag = 3
        Caption = 'Gravity well'
        OnClick = AddMoverClick
      end
      object Repulsor1: TMenuItem
        Tag = 4
        Caption = 'Repulsor'
      end
      object Zoneforce1: TMenuItem
        Tag = 5
        Caption = 'Zone force'
        OnClick = AddMoverClick
      end
      object RandomMovedev1: TMenuItem
        Tag = 6
        Caption = 'Random Move dev'
        OnClick = AddMoverClick
      end
      object AlphachangewithTime1: TMenuItem
        Tag = 7
        Caption = 'Alpha change with Time'
        OnClick = AddMoverClick
      end
      object Colorchangewithtime1: TMenuItem
        Tag = 8
        Caption = 'Color change with time'
        OnClick = AddMoverClick
      end
    end
    object MnuItAddFinalizer1: TMenuItem
      Caption = 'Add Finalizer'
      object Deleteontime1: TMenuItem
        Caption = 'Delete on time'
        OnClick = AddFinalizerClick
      end
      object DeleteonPlane1: TMenuItem
        Tag = 1
        Caption = 'Delete on Plane X'
        OnClick = AddFinalizerClick
      end
      object DeleteonPlaneY1: TMenuItem
        Tag = 2
        Caption = 'Delete on Plane Y'
        OnClick = AddFinalizerClick
      end
      object DeleteonPlaneZ1: TMenuItem
        Tag = 3
        Caption = 'Delete on Plane Z'
        OnClick = AddFinalizerClick
      end
      object Deleteonsphere1: TMenuItem
        Tag = 4
        Caption = 'Delete on sphere'
        OnClick = AddFinalizerClick
      end
    end
    object MnuItDelete1: TMenuItem
      Caption = 'Delete'
    end
  end
end

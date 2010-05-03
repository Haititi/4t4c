program ClientEditor;

uses
  Forms,
  uEditDda in 'uEditDda.pas' {FrmClEdit},
  udda in 'udda.pas',
  uImpExp in 'uImpExp.pas',
  uPalUnit in 'uPalUnit.pas' {FrmPal},
  uAbout in 'uAbout.pas' {FrmAbout},
  MidEditor in 'MidEditor.pas' {FrmMIdEd},
  IdTypes in 'IdTypes.pas',
  Globals in 'Globals.pas',
  Squish in 'Squish.pas',
  SurfaceUtils in 'SurfaceUtils.pas',
  uSurfEdit in 'uSurfEdit.pas' {FrmSurfEdit},
  SkinEdit in 'SkinEdit.pas' {FrmSkinEdit},
  ItemSkin in 'ItemSkin.pas' {FrmItemSkinEd},
  SpellEdit in 'SpellEdit.pas' {FrmSpellEd},
  PuppetSkinEdit in 'PuppetSkinEdit.pas' {FrmPuppetSkinEdit},
  hashunit in 'hashunit.pas',
  HashPool in 'HashPool.pas',
  Particules in 'Particules.pas',
  UConsole in 'UConsole.pas' {FrmCon},
  OtherUtils in 'OtherUtils.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.Title := 'Client Editor - Chaotikmind 2010';
  Application.CreateForm(TFrmClEdit, FrmClEdit);
  Application.CreateForm(TFrmPal, FrmPal);
  Application.CreateForm(TFrmAbout, FrmAbout);
  Application.CreateForm(TFrmMIdEd, FrmMIdEd);
  Application.CreateForm(TFrmSurfEdit, FrmSurfEdit);
  Application.CreateForm(TFrmSkinEdit, FrmSkinEdit);
  Application.CreateForm(TFrmItemSkinEd, FrmItemSkinEd);
  Application.CreateForm(TFrmSpellEd, FrmSpellEd);
  Application.CreateForm(TFrmPuppetSkinEdit, FrmPuppetSkinEdit);
  Application.CreateForm(TFrmCon, FrmCon);
  Application.Run;
end.

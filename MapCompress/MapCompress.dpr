program MapCompress;

uses
  Forms,
  UMapCompress in 'UMapCompress.pas' {FrmMapCompress};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TFrmMapCompress, FrmMapCompress);
  Application.Run;
end.

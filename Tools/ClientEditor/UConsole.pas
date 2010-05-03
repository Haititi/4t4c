unit UConsole;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TFrmCon = class(TForm)
    Memo: TMemo;
  private
    { Private declarations }
  public
    { Public declarations }
    procedure Log(S:string);
  end;

var
  FrmCon: TFrmCon;

implementation

{$R *.dfm}

{ TFrmCon }

procedure TFrmCon.Log(S: string);
begin
 Memo.Lines.Add(s);
end;

end.

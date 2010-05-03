program GetRevision;

{$APPTYPE CONSOLE}

{$R *.res}

uses
  SysUtils;

var Entries,RevFile:TextFile;
    Revision,OldRevision:string;
begin
  try
  if DirectoryExists('./.svn') then
  begin
   if FileExists('./.svn/entries') then
   begin
    DefaultTextLineBreakStyle:=tlbsLF;

    AssignFile(Entries,'./.svn/entries');
    reset(Entries);
    readln(Entries,Revision);
    readln(Entries,Revision);
    readln(Entries,Revision);
    readln(Entries,Revision);
    writeln(Revision);
    CloseFile(Entries);

    DefaultTextLineBreakStyle:=tlbsCRLF;

    Revision:='#define REVISION_NUMBER '+Revision;

    OldRevision:='';
    if FileExists('./rev.inc') then
    begin
     AssignFile(RevFile,'./rev.inc');
     Reset(RevFile);
     readln(RevFile,OldRevision);
     CloseFile(RevFile);
    end;

    if (OldRevision<>Revision) then
    begin
     AssignFile(RevFile,'./rev.inc');
     Rewrite(RevFile);
     Writeln(RevFile,Revision);
     CloseFile(RevFile);
    end;
   end;
  end;

  except
    on E:Exception do
      Writeln(E.Classname, ': ', E.Message);
  end;
end.

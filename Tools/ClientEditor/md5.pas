unit md5;

interface
uses windows,sysutils,wcrypt2;

Type
  THashArray=array [0..31] of byte;

  Function HashToStr(const Md5:THashArray):string;
  function md5Hash(const Input: String): String;overload;
  function Md5HashBuffer(const Input: pointer;BufferSize:integer):THashArray;overload;
implementation

Function HashToStr(const Md5:THashArray):string;
var i:longint;
begin
 Result:='';
 for i:=0 to 15 do
  Result:=Result+LowerCase(IntToHex(Md5[i],2));
end;

function md5Hash(const Input: String): String;
var
  hCryptProvider: HCRYPTPROV;
  hHash: HCRYPTHASH;
  bHash: array[0..$7f] of Byte;
  dwHashBytes: Cardinal;
  pbContent: PByte;
  i: Integer;

begin
  dwHashBytes := 16;
  pbContent := Pointer(PChar(Input));

  Result := '';

  if CryptAcquireContext(@hCryptProvider, nil, nil, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT or CRYPT_MACHINE_KEYSET) then
  begin
    if CryptCreateHash(hCryptProvider, CALG_MD5, 0, 0, @hHash) then
    begin
      if CryptHashData(hHash, pbContent, Length(Input) * sizeof(Char), 0) then
      begin
        if CryptGetHashParam(hHash, HP_HASHVAL, @bHash[0], @dwHashBytes, 0) then
        begin
          for i := 0 to dwHashBytes - 1 do
          begin
            Result := Result + Format('%.2x', [bHash[i]]);
          end;
        end;
      end;
      CryptDestroyHash(hHash);
    end;

    CryptReleaseContext(hCryptProvider, 0);
  end;

  Result := AnsiLowerCase(Result);
end;


function Md5HashBuffer(const Input: pointer;BufferSize:integer): THashArray;overload;
var
  hCryptProvider: HCRYPTPROV;
  hHash: HCRYPTHASH;
  bHash: array[0..$7f] of Byte;
  dwHashBytes: Cardinal;
  i: Integer;

begin
  dwHashBytes := 16;

  if CryptAcquireContext(@hCryptProvider, nil, nil, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT or CRYPT_MACHINE_KEYSET) then
  begin
    if CryptCreateHash(hCryptProvider, CALG_MD5, 0, 0, @hHash) then
    begin
      if CryptHashData(hHash, Input, BufferSize, 0) then
      begin
        if CryptGetHashParam(hHash, HP_HASHVAL, @bHash[0], @dwHashBytes, 0) then
        begin
          for i := 0 to dwHashBytes - 1 do
          begin
            Result[i]:=bHash[i];
          end;
        end;
      end;
      CryptDestroyHash(hHash);
    end;

    CryptReleaseContext(hCryptProvider, 0);
  end;
end;

end.

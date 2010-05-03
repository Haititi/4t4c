unit HashUnit;

interface


 Function RandHash(Const s:string):cardinal;overload;
 Function RandHash(Data:pointer;DataSize:longint):cardinal;overload;
 Procedure InitRndHash;

 var RndTable:array[0..255] of cardinal;
implementation


Procedure InitRndHash;
var i,rndseed:Cardinal;
begin
 RndSeed:=$d2a9f7bd;
 for i:=0 to 255 do
 begin
  rndseed:=rndseed*$08088405+1;
  RndTable[i]:=rndseed;
 end;
end;

Function RandHash(Const s:string):cardinal;
var i:longint;
begin
 Result:=$FFFFFFFF;
 for i:=1 to length(s) do
 begin
  //result:=(result shl 1) xor rndtable[byte(s[i])];
  result:=((result shl 5)+result)+rndtable[byte(s[i])];
 end;
end;

Function RandHash(Data:pointer;DataSize:longint):cardinal;
var i:longint;
    Ptr:Pbyte;
begin
 Result:=$FFFFFFFF;
 Ptr:=Data;
 for i:=0 to DataSize-1 do
 begin
  //result:=(result shl 1) xor rndtable[byte(s[i])];
  result:=((result shl 5)+result)+rndtable[ptr^];
  inc(Ptr);
 end;
end;



{Function Randhash(Const s:string):cardinal;
asm
 push ebx
 push esi
 push edi

 mov esi,eax  //esi = s
 mov eax,[esi-4]
 inc eax
 mov edx,1;

 @Loop1:
 xor ecx,ecx
 mov cl,[esi+edx-1]
 mov ecx,[ecx*4+rndtable]
 mov edi,ebx
 shl edi,5
 add ebx,edi
// add ecx,ebx
// mov ebx,ecx
 add ebx,ecx
 inc edx
 dec eax
 jnz @loop1
 mov eax,ebx

 pop edi
 pop esi
 pop ebx
end; }

end.


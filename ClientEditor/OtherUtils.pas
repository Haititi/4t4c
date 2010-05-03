unit OtherUtils;

interface
 uses windows;

 function RandFloat01():single;
 function Randfloat1m1():single;

implementation


function RandFloat01():single;
var Value:cardinal;
begin
 Value:=((Random($ffffffff) shr 8) and $7fffff) or $3f000000;
 Result:=PSingle(@Value)^;
end;

function Randfloat1m1():single;
var Value:Cardinal;
begin
 Value:=((Random($ffffffff) shr 8) and $7fffff) or $40000000;
 Result:=PSingle(@Value)^-3.0;
end;

end.

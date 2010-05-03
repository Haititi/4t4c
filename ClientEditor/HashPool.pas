unit HashPool;

interface
uses HashUnit;

//note pour Mestoph -> dans SetHashSize la taille choisit doit etre "arrondi" au prochain nombre premier pour la meilleure efficacité

type
  PHashItem=^THashItem;
  THashItem=record
               This:Pointer;  //pointeur sur l'objet
               HashCode:Cardinal; //si tu garde le hash ici l'unité seraplus generique  ca serait p-e mieux vu que c'est pas le seul endroti ou ca a servir , faut voir
               Next:PHashItem;
              end;


 THashTable=class        //Warning : rndhash NEED to be Initialized !!!!!!!
             private
              HashSize:cardinal;
              HashList:array Of PHashItem;
              EntryCount:Longint;
              CycleIt:longint; // num d'entree dans le tableau
              CycleObj:PHashItem;//PointerActuel;
             public
              Constructor Create(pHashSize:integer);
              Destructor Destroy;override;
              Function AddHashEntry(HashCode:cardinal;Data: Pointer):boolean;overload;
              Function AddHashEntry(HashStr:string;Data: Pointer):boolean;overload;
              function RemoveHashEntry(EName:string):pointer;
              Function GetEntryCount:Longint;
              Function SearchByHash(HashCode:cardinal):Pointer;
              Function SearchByName(EName:string):pointer;
              Procedure ResetCycling;
              Function GetNextEntry:Pointer;
            end;

implementation

constructor THashTable.Create(pHashSize:integer);
var i:longint;
begin
 EntryCount:=0;
 HashSize:=pHashSize;
 Setlength(HashList,HashSize);
 for i:=0 to Hashsize-1 do
 begin
//  new(HashList[i]);
 // HashList[i]^.This:=nil;
 // HashList[i]^.Next:=nil;
 end;
end;

destructor THashTable.Destroy;
var i:longint;
    Next,ActEntry:PHashItem;
begin
 for i:=0 to Hashsize-1 do
 begin
  if Hashlist[i]<>nil then
  begin
   ActEntry:=HashList[i];
   repeat
    Next:=ActEntry^.Next;
    if ActEntry=nil then break;
   // If ActEntry^.This=nil then break;
   // if ActEntry^.This^.Data<>nil then Freemem(ActEntry^.This^.Data);
     freemem(ActEntry);
    ActEntry:=Next;
   until ActEntry=nil;
  end;
 end;
 finalize(HashList);
end;

function THashTable.GetEntryCount: Longint;
begin
 Result:=EntryCount;
end;

function THashTable.GetNextEntry: Pointer;
begin
 Result:=nil;
 repeat
  if CycleObj=nil then
  begin
   inc(CycleIt);
   CycleObj:=HashList[CycleIt];
  end else
  begin
   Result:=CycleObj^.This;
   CycleObj:=CycleObj.Next;
  end;
 until (Result<>nil) or (CycleIt=(HashSize-1));
end;

procedure THashTable.ResetCycling;
begin
 CycleIt:=0;
 CycleObj:=HashList[0];
end;

Function THashTable.AddHashEntry(HashStr:string;Data: Pointer):boolean;
begin
 Result:=AddHashEntry(RandHash(HashStr),Data);
end;

Function THashTable.AddHashEntry(HashCode:cardinal;Data: Pointer):boolean;
var Num:cardinal;
    last,ActEntry:PHashItem;
begin
 Result:=false;

 num:=HashCode mod HashSize;
 if Hashlist[Num]=nil then
 begin
  new(HashList[num]);
  HashList[num]^.This:=nil;
  HashList[num]^.Next:=nil;
 end;
 ActEntry:=HashList[num];
 Last:=nil;

 repeat
  if ActEntry=nil then
  begin
   new(ActEntry);
   ActEntry^.This:=nil;
   ActEntry^.Next:=nil;
   if Last<>nil then
    last^.Next:=ActEntry;
  end;
  if ActEntry^.This=nil then
  begin
   ActEntry^.This:=Data;
   ActEntry^.HashCode:=HashCode;
   Result:=true;
   inc(EntryCount);
   exit;
  end            else
  begin
   if ActEntry^.HashCode=HashCode then
   begin
    result:=false;
    exit;  // il esiste deja on l'ajoute pas
   end;
   last:=Actentry;
   ActEntry:=ActEntry^.Next;
  end;
 until true=false;
end;

function THashTable.RemoveHashEntry(EName:string):pointer;
var num:cardinal;
    ActEntry,PrevEntry:PHashItem;
    Hash:Cardinal;
begin
 Result:=nil;
 Hash:=RandHash(EName);
 num:=hash mod HashSize;
 ActEntry:=HashList[num];
 PrevEntry:=ActEntry;
 repeat
  if ActEntry=nil then exit;
  if ActEntry^.This=nil then exit; //ca peu arrivé ca ?
  if ActEntry^.HashCode=hash then
  begin
   break;
  end;
  PrevEntry:=ActEntry;
  ActEntry:=ActEntry^.Next;
 until true=false;

 //redondant je sait mais je suit crevé       todo: a corriger
 if HashList[num]=ActEntry then
 begin             //l'element est le premier
  Hashlist[num]:=ActEntry.Next;//si c'est nil ca marche pareil
 end else
 begin
  PrevEntry.Next:=ActEntry.Next;
 end;
 Result:=ActEntry.This;
 dispose(ActEntry);
end;

Function THashTable.SearchByName(EName:string):pointer;
begin
 Result:=SearchByHash(RandHash(EName));
end;

function THashTable.SearchByHash(HashCode: cardinal): Pointer;
var num:cardinal;
    ActEntry:PHashItem;
begin
 result:=nil;
 num:=HashCode mod HashSize;
 ActEntry:=HashList[num];
 repeat
  if ActEntry=nil then exit;
  if ActEntry^.This=nil then exit;
  if ActEntry^.HashCode=HashCode then
  begin
   result:=ActEntry^.This;
   exit;
  end;
  ActEntry:=ActEntry^.Next;
 until true=false;
end;

end.

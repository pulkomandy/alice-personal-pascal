program ActivateSidekick;

function Sidekick: Boolean;
const
  SKactOffset = $012D;
  SKverOffset = $012A;
type
  RegPack     = record
                  case Integer of
                    1: (AX,BX,CX,DX,BP,SI,DI,DS,ES,Flags: Integer);
                    2: (AL,AH,BL,BH,CL,CH,DL,DH         : Byte);
                end;
  Address     = record
                  Offset : Integer;
                  Segment: Integer;
                end;
  SKstr       = array[1..2] of Char;
  SKstrPtr    = ^SKstr;
var
  SKbios08Trap: Address absolute $0000:$0020  { Sidekick timer tick trap };
  SKbios25Trap: Address absolute $0000:$0094  { Sidekick DOS int 25 trap };
  SKfound     : Boolean;
  SKstrCheck  : SKstrPtr;
  R           : RegPack;

begin
  with SKbios25Trap do SKstrCheck:=Ptr(Segment, Offset-2);
  SKfound:=(SKstrCheck^ = 'SK');
  if not SKfound then
  begin
    with SKbios08Trap do SKstrCheck:=Ptr(Segment, Offset-4);
    SKfound:=(SKstrCheck^ = 'SK');
  end;

  { Check Sidekick version number (must be >= 1.50) }
  SKfound:=(SKfound and (Mem[Seg(SKstrCheck^): SKverOffset] >= 1)
                    and (Mem[Seg(SKstrCheck^): SKverOffset+1] >= 50));
  if SKfound then
  begin
    Mem[Seg(SKstrCheck^): SKactOffset]:=1;  { Set Sidekick activate flag    }
    Intr($28, R);                           { Turn control over to Sidekick }
  end;
  Sidekick:=SKfound;
end  { Sidekick };

begin
  if not Sidekick then Writeln('Sidekick 1.50 or later not loaded');
end.

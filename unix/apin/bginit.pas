program ext26(input, output);
{} 
{Initialized Variable Testing - Record constants} 
{} 
type
	intrecord = record
		x, y : integer;
		u, v : integer;
	    end;
	realrecord = record
		s, t : real;
		x : real;
	    end;
	ostype = (VM370, MVS, CPM80, MSDOS);
	complicated = record
		os : ostype;
		num_users : integer;
		cost : real;
		cartesian : array [1..2] of record
			squid : intrecord;
			octopus : realrecord;
		    end;
	    end;

const
	pgmname = 'ext26';
	{} 
	transform : intrecord = (x:0; y:0; u:1; v:2);
	realrec : realrecord = (s:Pi; t:1.2e4; x:0.0);
	biggie : complicated = (os:MVS; num_users:2; cost:1e20; cartesian:((squid:(x:1; 
	y:1; u:2; v:2); octopus:(s:-1.0; t:16.0; x:2.0)), (squid:(x:10; y:10; u:20; v:
	20); octopus:(s:-22.0; t:Pi; x:Pi))));

var
	error : Boolean;

{+Class Declarations}{+Hide}{+Revealed PASS/FAIL output routines}
const
	tstnum : integer = 1;   {global test number}

procedure pass ;
	{write out the PASS message for test number tstnum} 
	{+ <<Declarations>>}
    begin
	writeln(pgmname, tstnum:4, '    PASS');
	tstnum := tstnum + 1;
    end;
procedure fail ;
	{write out the FAIL message for test number tstnum} 
	{+ <<Declarations>>}
    begin
	writeln(pgmname, tstnum:4, ' ** FAIL **');
	tstnum := tstnum + 1;
    end;
{+Hide end}
begin
{Test 1 - test record initialization of simple integer record} 
if (transform.x = 0) and (transform.y = 0) then begin
	error := false;
    end
 else begin
	error := true;
    end;
if not ((transform.u = 1) and (transform.v = 2)) then begin
	error := true;
    end;
if not error then begin
	pass ;
    end
 else begin
	fail ;
    end;
{Test 2 - test record initialization of simple reals} 
if (realrec.s = Pi) and (realrec.t = 12000) and (realrec.x = 0) then begin
	pass ;
    end
 else begin
	fail ;
    end;
{Test 3 - test fairly complicated initialized record} 
if (biggie.os = MVS) and (biggie.num_users = 2) and (biggie.cost = 1e20) then begin
	error := false;
    end
 else begin
	error := true;
    end;
if biggie.cartesian[1].squid.x <> 1 then begin
	error := true;
    end;
if biggie.cartesian[1].squid.y <> 1 then begin
	error := true;
    end;
if biggie.cartesian[1].squid.u <> 2 then begin
	error := true;
    end;
if biggie.cartesian[1].squid.v <> 2 then begin
	error := true;
    end;
if biggie.cartesian[1].octopus.s <> -1.0 then begin
	error := true;
    end;
if biggie.cartesian[1].octopus.t <> 16.0 then begin
	error := true;
    end;
if biggie.cartesian[1].octopus.x <> 2.0 then begin
	error := true;
    end;
if biggie.cartesian[2].squid.x <> 10 then begin
	error := true;
    end;
if biggie.cartesian[2].squid.y <> 10 then begin
	error := true;
    end;
if biggie.cartesian[2].squid.u <> 20 then begin
	error := true;
    end;
if biggie.cartesian[2].squid.v <> 20 then begin
	error := true;
    end;
if biggie.cartesian[2].octopus.s <> -22.0 then begin
	error := true;
    end;
if biggie.cartesian[2].octopus.t <> Pi then begin
	error := true;
    end;
if biggie.cartesian[2].octopus.x <> Pi then begin
	error := true;
    end;
if not error then begin
	pass ;
    end
 else begin
	fail ;
    end;
end.

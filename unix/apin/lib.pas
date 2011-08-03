program blat;

procedure CrtExit;
begin
	{ do nothing }
end;


procedure CrtInit;
begin
	{ do nothing }
end;

procedure Flush(filevariable : generic);
begin
	{ do nothing }
end;

function Hi( value : integer ) : integer;
begin
	hi := value shr 8
end;

function Lo( value : integer ) : integer;
begin
	hi := value and $ff
end;

procedure LowVideo;
begin
	textcolor(7)
end;

procedure NormVideo;
begin
	textcolor(15)
end;

procedure HighVideo;
begin
	textcolor(14)
end;

procedure Randomize;
var seconds : array[1..2] of integer;

begin
	seconds[1] := 0;
	cproc( 54, seconds );
	initrandom( seconds[1], 32767 );
end;

function Swap( value : integer ) : integer;
begin
	swap := (value shl 8) or (value shr 8)
end;


function UpCase( letter : char ) : char;
begin
	if (letter >= 'a') and (letter <= 'z' ) then
		upcase := chr( ord(letter) - 32 )
	 else
		upcase := letter;
end;

procedure MSDos( registers : generic );
begin
	intr( $21, registers.object^ )
end;

procedure NoSound;
begin
	sound(0)
end;

function WhereX : integer;
begin
	WhereX := scrxy(2) + 1;
end;


function WhereY : integer;
begin
	WhereX := scrxy(1) + 1;
end;

const
	Black = 0;
	Blue = 1;
	Green = 2;
	Cyan = 3;
	Red = 4;
	Magenta = 5;
	Brown = 6;
	LightGray = 7;
	DarkGray = 8;
	LightBlue = 9;
	LightGreen = 10;
	LightCyan = 11;
	LightRed = 12;
	LightMagenta = 13;
	Yellow = 14;
	White = 15;
	Blink = 16;
	BW40 = 0;
	C40 = 1;
	BW80 = 2;
	C80 = 3;
	Mono = 7;

	CC_BYTE = 1;
	CC_INTEGER = 2;
	CC_POINTER = 3;
	CC_REAL = 4;
	CC_SET = 5;
	CC_STRING = 6;
	CC_ARRAY = 10;
	CC_FILE = 11;
	CC_RECORD = 12;


type
	filerecord = record
		boring : integer;
		stream : pointer;
		flags : integer;
		itemsize : integer;
		name : pointer;
		buffer : char;
		end;

	largestring = string[120];

	registers8086 = record
		ax,bx,cx,dx,si,di,ds,es,Flags : integer;
		end;
	segmentregs = record
		es,cs,ss,ds : integer;
		end;

procedure erase( filevar : generic );

var thefile : ^filerecord;

begin
	if filevar.typecode <> CC_FILE then begin
		writeln( 'Erase - argument must be a file variable' );
		halt;
		end;
	thefile := filevar.object;
	if CIntFunc( 67, thefile^.name ) <> 0 then begin
		writeln( 'Erase - cannot delete file' );
		halt;
		end;
end;

function dseg : integer;

var segs : segmentregs;

begin
	cproc( 63, segs );
	dseg := segs.ds
end;

function cseg : integer;

var segs : segmentregs;

begin
	cproc( 63, segs );
	cseg := segs.cs
end;
function sseg : integer;

var segs : segmentregs;

begin
	cproc( 63, segs );
	sseg := segs.ss;
end;
procedure rename( filevar : generic; newname : largestring );

var thefile : ^filerecord;
	doscall : registers8086;
	dataseg : integer;

begin
	if filevar.typecode <> CC_FILE then begin
		writeln( 'Rename - argument must be a file variable' );
		halt;
		end;
	thefile := filevar.object;
	doscall.ax := $5600;
	doscall.dx := thefile^.name;
	dataseg := dseg;
	doscall.ds := dataseg;
	doscall.di := address( newname ) + 1;
	doscall.es := dataseg;
	intr( $21, doscall );
	{ error result ? }
end;


begin end.

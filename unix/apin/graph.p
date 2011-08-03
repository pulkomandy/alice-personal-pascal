
{+Class Declarations}{+Revealed Turbo Pascal (TM) GRAPH.BIN library}
const
	TLib = '?:\GRAPH.BIN';
	North = 0;
	East = 90;
	South = 180;
	West = 270;

procedure GraphMode ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 0);
    end;
procedure GraphColorMode ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 3);
    end;
procedure HiRes ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 6);
    end;
procedure HiResColor(Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 9);
    end;
procedure Palette(N: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 12);
    end;
procedure GraphBackground(Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 15);
    end;
procedure GraphWindow(x1, y1, x2, y2: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 18, x1, y1, x2, y2);
    end;
procedure Plot(x, y, Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 21);
    end;
procedure Draw(x1, y1, x2, y2, Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 24);
    end;
procedure ColorTable(C1, C2, C3, C4: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 27);
    end;
procedure Arc(x, y, Angle, Radius, Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 30);
    end;
procedure Circle(X, Y, Radius, Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 33, X, Y, Radius, Color);
    end;
procedure GetPic(var Buffer; x1, y1, x2, y2: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 36, address(Buffer, 1), address(Buffer), x1, y1, x2, y2);
    end;
procedure PutPic(var buffer; x, y: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 39, address(buffer, 1), address(buffer), x, y);
    end;
procedure FillScreen(Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 45);
    end;
procedure FillShape(x, y, fillcol, bordercol: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 48);
    end;
procedure FillPattern(X1, Y1, X2, Y2, Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 51);
    end;
procedure Pattern(var P);

	{+ Declarations}
    begin
	TPLibProc(TLib, 54, address(P, 1), address(P));
    end;
function GetDotColor(X, Y: integer) : integer;

	{+ Declarations}
    begin
	GetDotColor := TPLibFunc(TLib, 42);
    end;
procedure Back(Dist: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 57);
    end;
procedure ClearScreen ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 60);
    end;
procedure Forwd(Dist: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 63);
    end;
function Heading  : integer;

	{+ Declarations}
    begin
	Heading := TPLibFunc(TLib, 66);
    end;
procedure HideTurtle ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 69);
    end;
procedure Home ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 72);
    end;
procedure NoWrap ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 75);
    end;
procedure PenDown ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 78);
    end;
procedure PenUp ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 81);
    end;
procedure SetHeading(Angle: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 84);
    end;
procedure SetPenColor(Color: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 87);
    end;
procedure SetPosition(x, y: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 90);
    end;
procedure ShowTurtle ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 93);
    end;
procedure TurnLeft(Angle: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 96);
    end;
procedure TurnRight(Angle: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 99);
    end;
procedure TurtleDelay(Delay: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 102);
    end;
procedure TurtleWindow(X, Y, W, H: integer);

	{+ Declarations}
    begin
	TPLibProc(TLib, 105);
    end;
function TurtleThere  : Boolean;

	{+ Declarations}
    begin
	TurtleThere := TPLibFunc(TLib, 108) <> 0;
    end;
procedure Wrap ;

	{+ Declarations}
    begin
	TPLibProc(TLib, 111);
    end;
function Xcor  : integer;

	{+ Declarations}
    begin
	Xcor := TPLibFunc(TLib, 114);
    end;
function Ycor  : integer;

	{+ Declarations}
    begin
	Ycor := TPLibFunc(TLib, 117);
    end;
{+Hide end}

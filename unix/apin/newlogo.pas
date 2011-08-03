program logo ;

{+Class Declarations}{+Revealed Turbo Pascal (TM) GRAPH.BIN library}
const
	TLib = '?:\GRAPH.BIN';
	North = 0;
	East = 90;
	South = 180;
	West = 270;

procedure GraphMode ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 0);
    end;
procedure GraphColorMode ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 3);
    end;
procedure HiRes ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 6);
    end;
procedure HiResColor(Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 9);
    end;
procedure Palette(N: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 12);
    end;
procedure GraphBackground(Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 15);
    end;
procedure GraphWindow(x1, y1, x2, y2: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 18, x1, y1, x2, y2);
    end;
procedure Plot(x, y, Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 21);
    end;
procedure Draw(x1, y1, x2, y2, Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 24);
    end;
procedure ColorTable(C1, C2, C3, C4: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 27);
    end;
procedure Arc(x, y, Angle, Radius, Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 30);
    end;
procedure Circle(X, Y, Radius, Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 33, X, Y, Radius, Color);
    end;
procedure GetPic(var Buffer; x1, y1, x2, y2: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 36, address(Buffer, 1), address(Buffer), x1, y1, x2, y2);
    end;
procedure PutPic(var buffer; x, y: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 39, address(buffer, 1), address(buffer), x, y);
    end;
procedure FillScreen(Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 45);
    end;
procedure FillShape(x, y, fillcol, bordercol: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 48);
    end;
procedure FillPattern(X1, Y1, X2, Y2, Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 51);
    end;
procedure Pattern(var P);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 54, address(P, 1), address(P));
    end;
function GetDotColor(X, Y: integer) : integer;
	
	{+ <<Declarations>>}
    begin
	GetDotColor := TPLibFunc(TLib, 42);
    end;
procedure Back(Dist: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 57);
    end;
procedure ClearScreen ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 60);
    end;
procedure Forwd(Dist: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 63);
    end;
function Heading  : integer;
	
	{+ <<Declarations>>}
    begin
	Heading := TPLibFunc(TLib, 66);
    end;
procedure HideTurtle ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 69);
    end;
procedure Home ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 72);
    end;
procedure NoWrap ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 75);
    end;
procedure PenDown ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 78);
    end;
procedure PenUp ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 81);
    end;
procedure SetHeading(Angle: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 84);
    end;
procedure SetPenColor(Color: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 87);
    end;
procedure SetPosition(x, y: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 90);
    end;
procedure ShowTurtle ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 93);
    end;
procedure TurnLeft(Angle: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 96);
    end;
procedure TurnRight(Angle: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 99);
    end;
procedure TurtleDelay(Delay: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 102);
    end;
procedure TurtleWindow(X, Y, W, H: integer);
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 105);
    end;
function TurtleThere  : Boolean;
	
	{+ <<Declarations>>}
    begin
	TurtleThere := TPLibFunc(TLib, 108) <> 0;
    end;
procedure Wrap ;
	
	{+ <<Declarations>>}
    begin
	TPLibProc(TLib, 111);
    end;
function Xcor  : integer;
	
	{+ <<Declarations>>}
    begin
	Xcor := TPLibFunc(TLib, 114);
    end;
function Ycor  : integer;
	
	{+ <<Declarations>>}
    begin
	Ycor := TPLibFunc(TLib, 117);
    end;
{+Hide end}
type
	stringtype = string [255];
	penstatustype = (up, down);
	maintype = array [0..500] of char;
	wordtype = string [20];
	pointtype = array [0..50] of integer;

var
	penstatus : penstatustype;
	actualcolor, backgroundcolor, ncommands, level, screensplit, hicolor : integer;
	string1 : maintype;
	pointlist : pointtype;
	name : array [1..10] of wordtype;
	command : array [1..10] of stringtype;
	commandlevel : array [1..10] of integer;
	Erase, memoryexceeded, alternateinput, debug, standardscreen : Boolean;
	endpoint : integer;
	commandinput, list : text;

function ExtractInteger  : integer; forward;
function extractreal  : real; forward;
function parse  : wordtype; forward;
function parselist  : stringtype; forward;
procedure insertprocedure(command1: stringtype); forward;
procedure save ;
	{saves workspace} 
	var
		string1 : stringtype;
		i : integer;
		outfile : text;
	
    begin
	Assign(outfile, 'liz.log');
	rewrite(outfile);
	for i := 1 to ncommands do begin
		string1 := command[i];
		Insert(name[i] + ' ', string1, 1);
		writeln(outfile, string1);
	    end;
	Flush(outfile);
	Close(outfile);
    end;
procedure read ;
	{reads in workspace} 
	var
		string1 : stringtype;
		j, len : integer;
		outfile : text;
	
    begin
	Assign(outfile, 'liz.log');
	reset(outfile);
	ncommands := 0;
	repeat
		readln(outfile, string1);
		len := Length(string1);
		j := 1;
		ncommands := ncommands + 1;
		repeat
			j := j + 1;
		until string1[j] = ' ';
		name[ncommands] := Copy(string1, 1, j - 1);
		command[ncommands] := Copy(string1, j + 1, len - j);
		commandlevel[ncommands] := 0;
	until eof(outfile);
	Close(outfile);
    end;
{* resets everything *} 
procedure gotostart ;
	
	{+ <<Declarations>>}
    begin
	endpoint := 0;
	memoryexceeded := false;
	level := 0;
    end;
procedure procdebug(strvar, procname: wordtype);
	
	{+ <<Declarations>>}
    begin
	if debug then begin
		writeln(list, procname + ' ' + strvar);
	    end;
    end;
procedure load ;
	
	var
		i : integer;
		word1, word2 : wordtype;
	
    begin
	word2 := parse ;
	i := Length(word2);
	word1 := Copy(word2, 2, i - 1);
	Assign(commandinput, word1);
	reset(commandinput);
	alternateinput := true;
    end;
procedure setpenstatus(penstat: penstatustype);
	
	{+ <<Declarations>>}
    begin
	case penstat of
		down: begin
			PenDown ;
		    end;
		up: begin
			PenUp ;
		    end;
	    end;
	penstatus := penstat;
    end;
function checkword(word: wordtype) : Boolean;
	
	{+ <<Declarations>>}
    begin
	if (word[1] = '"') and (word[0] > #0) then begin
		checkword := true;
	    end
	 else begin
		writeln(' Looking for word starting with " but found ' + word);
		checkword := false;
		gotostart ;
	    end;
    end;
procedure strfnr(X: real; var strvar: wordtype);
	
	var
		i : integer;
	
    begin
	if (X < 999999.999) and (X > -99999.999) then begin
		Str(X:10:3, strvar);
	    end
	 else begin
		writeln(' real number is too big and has been set to highest value');
		if (X < 0) then begin
			strvar := '-99999.999';
		    end
		 else begin
			strvar := '999999.999';
		    end;
	    end;
	i := 1;
	while strvar[i] = ' ' do begin
		i := i + 1;
	    end;
	i := i - 1;
	if i > 0 then begin
		Delete(strvar, 1, i);
	    end;
    end;
procedure strfnI(X: integer; var strvar: wordtype);
	
	var
		i : integer;
	
    begin
	Str(X:10, strvar);
	i := 1;
	while strvar[i] = ' ' do begin
		i := i + 1;
	    end;
	i := i - 1;
	if i > 0 then begin
		Delete(strvar, 1, i);
	    end;
    end;
{* strfnr,strfni*} 
{*initializes grahpics and text window*} 
procedure Initialize ;
	
	var
		i : integer;
	
    begin
	alternateinput := false;
	debug := false;
	screensplit := 13;
	endpoint := 0;
	pointlist[0] := 0;
	standardscreen := true;
	ncommands := 0;
	backgroundcolor := 1;
	memoryexceeded := false;
	level := 0;
	Palette(0);
	penstatus := down;
	actualcolor := 1;
	hicolor := 14;
	SetPenColor(1);
	GraphColorMode ;
	TextColor(14);
	TextBackground(1);
	GraphBackground(1);
	TurtleWindow(159, 48, 320, 96);
	Window(1, screensplit, 40, 25);
	for i := 1 to screensplit do begin
		writeln(' ');
	    end;
    end;
procedure split ;
	
	var
		i, X, Y : integer;
	
    begin
	X := (screensplit - 1)*8;
	Y := (screensplit - 1)*4;
	if standardscreen then begin
		GraphColorMode ;
		TextBackground(backgroundcolor);
		GraphBackground(backgroundcolor);
		TurtleWindow(159, Y, 320, X);
		Window(1, screensplit, 40, 25);
		for i := 1 to screensplit do begin
			writeln(' ');
		    end;
	    end
	 else begin
		HiRes ;
		HiResColor(hicolor);
		for i := 1 to screensplit do begin
			writeln(' ');
		    end;
		TurtleWindow(319, Y, 640, X);
		Window(1, screensplit, 80, 25);
		for i := 1 to screensplit do begin
			writeln(' ');
		    end;
	    end;
    end;
procedure Clean ;
	
	var
		ix, iy, H : integer;
		oldpenstatus : penstatustype;
	
	{*records pen position and status*} 
    begin
	ix := Xcor ;
	iy := Ycor ;
	H := Heading ;
	oldpenstatus := penstatus;
	{*clears screen and redraws box*} 
	ClearScreen ;
	split ;
	{*puts turtle back*} 
	SetPosition(ix, iy);
	SetHeading(H);
	setpenstatus(oldpenstatus);
    end;
{* initialize splint and clean*} 
procedure findfirstword(var l, m, N: integer; strvar: stringtype);
	
	var
		bracket : Boolean;
		bracketcount : integer;
	
    begin
	l := Length(strvar);
	m := 1;
	while (m <= l) and (strvar[m] = ' ') do begin
		m := m + 1;
	    end;
	N := m;
	bracketcount := 0;
	if strvar[N] = '[' then begin
		bracket := true;
	    end
	 else begin
		bracket := false;
	    end;
	while (N <= l) and ((strvar[N] <> ' ') and (not bracket)) or (bracket and (
	bracketcount > 0)) or (bracket and (N = m)) do begin
		if (strvar[N] = ']') then begin
			bracketcount := bracketcount - 1;
		    end
		 else begin
			if (strvar[N] = '[') then begin
				bracketcount := bracketcount + 1;
			    end;
		    end;
		N := N + 1;
	    end;
    end;
procedure findlastword(var l, m, N: integer; strvar: stringtype);
	
	var
		bracket : Boolean;
		bracketcount : integer;
	
    begin
	l := Length(strvar);
	m := l;
	while (m >= 1) and (strvar[m] = ' ') do begin
		m := m - 1;
	    end;
	N := m;
	bracketcount := 0;
	if strvar[N] = ']' then begin
		bracket := true;
	    end
	 else begin
		bracket := false;
	    end;
	while (N >= 1) and (((strvar[N] <> ' ') and (not bracket)) or (bracket and (
	bracketcount > 0)) or (bracket and (N = m))) do begin
		if (strvar[N] = '[') then begin
			bracketcount := bracketcount - 1;
		    end
		 else begin
			if (strvar[N] = ']') then begin
				bracketcount := bracketcount + 1;
			    end;
		    end;
		N := N - 1;
	    end;
    end;
{*Extracts first word from string*} 
function first(strvar: stringtype) : stringtype;
	
	var
		l, N, m : integer;
	
    begin
	findfirstword(l, m, N, strvar);
	first := Copy(strvar, m, N - m);
    end;
{*Removes first word from string*} 
function Next(strvar: stringtype) : stringtype;
	
	var
		l, N, m : integer;
	
    begin
	findfirstword(l, m, N, strvar);
	Next := Copy(strvar, N + 1, l - N);
    end;
{*Extracts last word from string and removes it*} 
procedure getlast(var last, strvar: stringtype);
	
	var
		l, N, m : integer;
	
    begin
	findlastword(l, m, N, strvar);
	{*WriteLn(l, ' ', m, ' ', n, ' ', strvar); debug*} 
	last := Copy(strvar, N + 1, m - N);
	strvar[0] := chr(N);
    end;
procedure loadword(word1: stringtype);
	
	var
		tempword : stringtype;
		j, k, i : integer;
	
    begin
	repeat
		getlast(tempword, word1);
		k := Length(tempword);
		if endpoint > 0 then begin
			j := pointlist[endpoint];
		    end
		 else begin
			j := 0;
		    end;
		for i := 1 to k do begin
			string1[j + i] := tempword[i];
		    end;
		if ((endpoint < 500) and (pointlist[endpoint] < 4700)) then begin
			endpoint := endpoint + 1;
			pointlist[endpoint] := j + k;
		    end
		 else begin
			if (not memoryexceeded) then begin
				memoryexceeded := true;
				writeln(' MEMORY EXCEEDED');
				word1 := '';
				gotostart ;
			    end;
		    end;
	until word1 = '';
    end;
{*removes first word from string and makes it available*} 
procedure incrementstring(var word1: stringtype);
	
	var
		i, j, k : integer;
	
    begin
	if (endpoint <> 0) then begin
		i := pointlist[endpoint];
		if (endpoint = 1) then begin
			j := 1;
		    end
		 else begin
			j := pointlist[endpoint - 1] + 1;
		    end;
		for k := j to i do begin
			word1[k - j + 1] := string1[k];
		    end;
		k := i - j + 1;
		word1[0] := chr(k);
		endpoint := endpoint - 1;
	    end
	 else begin
		word1 := '';
		word1[0] := chr(0);
	    end;
    end;
procedure incrementstring1(var word1: wordtype);
	
	var
		i, j, k : integer;
	
    begin
	if (endpoint <> 0) then begin
		i := pointlist[endpoint];
		if (endpoint = 1) then begin
			j := 1;
		    end
		 else begin
			j := pointlist[endpoint - 1] + 1;
		    end;
		for k := j to i do begin
			word1[k - j + 1] := string1[k];
		    end;
		k := i - j + 1;
		word1[0] := chr(k);
		endpoint := endpoint - 1;
	    end
	 else begin
		word1 := '';
		word1[0] := chr(0);
	    end;
    end;
{*This procedure is used to change input to upper case*} 
procedure takin ;
	
	var
		i : integer;
		tempword : stringtype;
	
    begin
	memoryexceeded := false;
	if (not alternateinput) then begin
		write('?');
		readln(tempword);
	    end
	 else begin
		if (not SeekEof(commandinput)) then begin
			readln(commandinput, tempword);
		    end
		 else begin
			alternateinput := false;
			write('?');
			readln(tempword);
		    end;
	    end;
	for i := 1 to Length(tempword) do begin
		tempword[i] := UpCase(tempword[i]);
	    end;
	loadword(tempword);
    end;
{* findfirstword,findlastword,first,next,getlast,loadword,increment string} 
{takin*} 
function numberp  : wordtype;
	
	var
		tempword : wordtype;
	
    begin
	tempword := parselist ;
	if (tempword[0] > #0) and (tempword[1] in ['-', '1', '2', '3', '4', '5', '6', '7'
	, '8', '9', '0']) then begin
		numberp := '"TRUE';
	    end
	 else begin
		numberp := '"FALSE';
	    end;
    end;
function wordp  : wordtype;
	
	var
		tempword : stringtype;
	
    begin
	tempword := parselist ;
	if (tempword[0] > #0) and (tempword[1] = '"') then begin
		wordp := '"TRUE';
	    end
	 else begin
		wordp := '"FALSE';
	    end;
    end;
function namep  : wordtype;
	
	var
		word1 : stringtype;
		m : integer;
	
    begin
	word1 := parselist ;
	namep := '"FALSE';
	if (word1[0] > #0) and (word1[1] = '"') then begin
		m := ncommands;
		word1[1] := ':';
		while (m >= 1) and (level = commandlevel[m]) do begin
			if name[m] = word1 then begin
				namep := '"TRUE';
			    end;
		    end;
	    end;
    end;
function equalp  : wordtype;
	
	var
		word1, word2 : stringtype;
		X, Y : real;
		code, code1 : integer;
	
    begin
	word1 := parselist ;
	word2 := parselist ;
	if (word1[1] = '"') or (word2[1] = '"') or (word1[1] = '[') or (word2[1] = '[') or (
	word1[0] = #0) or (word2[0] = #0) then begin
		if word1 = word2 then begin
			equalp := '"TRUE';
		    end
		 else begin
			equalp := '"FALSE';
		    end;
	    end
	 else begin
		Val(word1, X, code);
		Val(word2, Y, code1);
		if (X = Y) and ((code + code1) = 0) then begin
			equalp := '"TRUE';
		    end
		 else begin
			equalp := '"FALSE';
		    end;
	    end;
    end;
function lessp  : wordtype;
	
	var
		word1, word2 : stringtype;
		X, Y : real;
		code, code1 : integer;
	
    begin
	word1 := parselist ;
	word2 := parselist ;
	if (word1[1] = '"') or (word1[1] = '[') or (word1[0] = #0) or (word2[0] = #0) then begin
		if (word1 < word2) then begin
			lessp := '"TRUE';
		    end
		 else begin
			lessp := '"FALSE';
		    end;
	    end
	 else begin
		lessp := '"FALSE';
		Val(word1, X, code);
		Val(word2, Y, code1);
		if (code <> 0) or (code1 <> 0) then begin
			writeln('I can''t compare ' + word1 + ' with ' + word2);
		    end
		 else begin
			if X < Y then begin
				lessp := '"TRUE';
			    end;
		    end;
	    end;
    end;
{*print,if,to, findcommand,checklist,make} 
procedure TOinput ;
	
	var
		word1 : stringtype;
	
    begin
	ncommands := ncommands + 1;
	incrementstring(word1);
	name[ncommands] := word1;
	command[ncommands] := '';
	repeat
		incrementstring(word1);
		if word1 = '' then begin
			write('>');
			takin ;
			incrementstring(word1);
		    end;
		command[ncommands] := command[ncommands] + word1 + ' ';
	until (word1 = 'END');
	writeln(name[ncommands] + ' Defined');
	commandlevel[ncommands] := level;
    end;
function checklist(word: stringtype) : Boolean;
	
	{+ <<Declarations>>}
    begin
	if not ((word[1] = '[') and (word[0] > #0)) then begin
		writeln(' Looking for a list but found ' + word);
		gotostart ;
		checklist := false;
	    end
	 else begin
		checklist := true;
	    end;
    end;
procedure findcommand(word1: wordtype; var found: Boolean);
	
	var
		m : integer;
		command1 : stringtype;
	
    begin
	found := false;
	m := ncommands;
	while (m >= 1) do begin
		if (word1 = name[m]) then begin
			command1 := command[m];
			if (word1[1] <> ':') then begin
				level := level + 1;
			    end;
			insertprocedure(command1);
			m := -1;
			found := true;
		    end
		 else begin
			m := m - 1;
		    end;
	    end;
	if (not found) then begin
		writeln('(findcommand)I don''t know how to ' + word1);
	    end;
    end;
procedure Make ;
	
	var
		tempword : wordtype;
		templist : stringtype;
		m : integer;
	
    begin
	tempword := parse ;
	templist := parselist ;
	if checkword(tempword) then begin
		m := ncommands;
		tempword[1] := ':';
		while (m >= 1) and (commandlevel[m] = level) do begin
			if tempword = name[m] then begin
				command[m] := templist;
				m := -1;
			    end
			 else begin
				m := m - 1;
			    end;
		    end;
		if m <> -1 then begin
			commandlevel[ncommands + 1] := level;
			ncommands := ncommands + 1;
			name[ncommands] := tempword;
			command[ncommands] := templist;
		    end;
		if command[ncommands] = '' then begin
			writeln('The parameter is missing and has been set to 0');
			command[ncommands] := '0';
		    end;
	    end;
    end;
procedure ifcommand ;
	
	var
		word1 : wordtype;
		tempword, tempword2 : stringtype;
	
    begin
	word1 := parse ;
	if word1 = '"TRUE' then begin
		tempword := parselist ;
		if endpoint > 0 then begin
			if string1[pointlist[endpoint - 1] + 1] = '[' then begin
				tempword2 := parselist ;
			    end;
		    end;
		if checklist(tempword) then begin
			loadword(tempword);
			loadword('RUN');
		    end;
	    end
	 else begin
		if word1 = '"FALSE' then begin
			tempword := parselist ;
			if checklist(tempword) then begin
				if endpoint <> 0 then begin
					if string1[pointlist[endpoint - 1] + 1] = '[' then begin
						tempword := parselist ;
						loadword(tempword);
						loadword('RUN');
					    end;
				    end;
			    end;
		    end
		 else begin
			writeln(' IF statement requires "TRUE or "FALSE');
			writeln(' instead it received ' + word1);
			gotostart ;
		    end;
	    end;
    end;
procedure print ;
	
	var
		templist : stringtype;
	
    begin
	templist := parselist ;
	writeln(templist);
    end;
procedure docommand(word1: wordtype; var found: Boolean);
	
	var
		X, Y : real;
		m, code, code1, H, i : integer;
		oldpenstatus : penstatustype;
		strg : wordtype;
	
    begin
	found := true;
	if KeyPressed  then begin
		gotostart ;
	    end
	 else begin
		if (word1 = 'DEBUG') then begin
			debug := true;
		    end
		 else begin
			if (word1 = 'NODEBUG') then begin
				debug := false;
			    end
			 else begin
				if (word1 = 'BACK') or (word1 = 'BK') then begin
					Back(ExtractInteger );
				    end
				 else begin
					if word1 = 'CS' then begin
						ClearScreen ;
					    end
					 else begin
						if (word1 = 'FORWARD') or (word1 = 'FD') then begin
							Forwd(ExtractInteger );
						    end
						 else begin
							if word1 = 'HT' then begin
								HideTurtle ;
							    end
							 else begin
								if word1 = 'HOME' then begin
									Home ;
								    end
								 else begin
									if word1 = 'WRAP' then begin
										Wrap ;
									    end
									 else begin
										if word1 = 'NOWRAP' then begin
											NoWrap ;
										    end
										 else begin
											if word1 = 'READ' then begin
												read ;
											    end
											 else begin
												if word1 = 'SAVE' then begin
													save ;
												    end
												 else begin
													if (word1 = 'PENDOWN') or (word1 = 'PD') then begin
														PenDown ;
														{*cancels erasing pen*} 
														if Erase then begin
															Erase := false;
															SetPenColor(actualcolor);
														    end;
													    end
													 else begin
														if (word1 = 'PENUP') or (word1 = 'PU') then begin
															PenUp ;
															if Erase then begin
																Erase := false;
																SetPenColor(actualcolor);
															    end;
														    end
														 else begin
															if (word1 = 'LEFT') or (word1 = 'LT') then begin
																TurnLeft(ExtractInteger );
															    end
															 else begin
																if (word1 = 'RIGHT') or (word1 = 'RT') then begin
																	TurnRight(ExtractInteger );
																    end
																 else begin
																	if word1 = 'ST' then begin
																		ShowTurtle ;
																	    end
																	 else begin
																		if word1 = 'SETH' then begin
																			SetHeading(ExtractInteger );
																		    end
																		 else begin
																			if word1 = 'SETBG' then begin
																				GraphBackground(ExtractInteger );
																			    end
																			 else begin
																				if word1 = 'SETPC' then begin
																					Palette(ExtractInteger );
																				    end
																				 else begin
																					if word1 = 'SETPN' then begin
																						{*THIS CHANGES THE PALETTE RATHER THEN THE COLOR*} 
																						actualcolor := ExtractInteger ;
																						SetPenColor(actualcolor);
																					    end
																					 else begin
																						if word1 = 'CLEAN' then begin
																							Clean ;
																						    end
																						 else begin
																							if word1 = 'PE' then begin
																								SetPenColor(0);
																								Erase := true;
																							    end
																							 else begin
																								if word1 = 'QUIT' then begin
																									TextMode ;
																									Halt ;
																								    end
																								 else begin
																									if word1 = 'TO' then begin
																										TOinput ;
																									    end
																									 else begin
																										if word1 = 'POALL' then begin
																											for i := 1 to ncommands do begin
																												writeln('level=', commandlevel[i], ' ', name[i] + '=' + command[i]);
																											    end;
																										    end
																										 else begin
																											if word1 = 'PRINT' then begin
																												print ;
																											    end
																											 else begin
																												if word1 = 'MAKE' then begin
																													Make ;
																												    end
																												 else begin
																													if word1 = 'END' then begin
																														while commandlevel[ncommands] = level do begin
																															ncommands := ncommands - 1;
																														    end;
																														level := level - 1;
																													    end
																													 else begin
																														if word1 = 'WAIT' then begin
																															X := extractreal ;
																															X := X*17.0;
																															Delay(round(X));
																														    end
																														 else begin
																															if word1 = 'LOAD' then begin
																																load ;
																															    end
																															 else begin
																																if (word1 = 'SPLIT') then begin
																																	m := ExtractInteger ;
																																	if (not (m in [2..24])) then begin
																																		writeln(' MUST SPLIT BETWEEN 2 and 24');
																																	    end
																																	 else begin
																																		screensplit := m;
																																	    end;
																																	split ;
																																    end
																																 else begin
																																	if (word1 = 'HIRES') then begin
																																		standardscreen := false;
																																		split ;
																																	    end
																																	 else begin
																																		if (word1 = 'LORES') then begin
																																			standardscreen := true;
																																			split ;
																																		    end
																																		 else begin
																																			found := false;
																																		    end;
																																	    end;
																																    end;
																															    end;
																														    end;
																													    end;
																												    end;
																											    end;
																										    end;
																									    end;
																								    end;
																							    end;
																						    end;
																					    end;
																				    end;
																			    end;
																		    end;
																	    end;
																    end;
															    end;
														    end;
													    end;
												    end;
											    end;
										    end;
									    end;
								    end;
							    end;
						    end;
					    end;
				    end;
			    end;
		    end;
	    end;
    end;
{*This executes all the primatives*} 
procedure run ;
	
	var
		tempword : stringtype;
		m : integer;
	
    begin
	tempword := parselist ;
	if checklist(tempword) then begin
		m := Length(tempword);
		loadword(Copy(tempword, 2, m - 2));
	    end;
    end;
procedure insertprocedure;
	
	var
		word1 : stringtype;
		tempbol : Boolean;
	
    begin
	word1 := first(command1);
	if word1[1] <> ':' then begin
		loadword(command1 + ' ');
	    end
	 else begin
		word1[1] := '"';
		loadword(word1);
		command1 := Next(command1);
		docommand('MAKE', tempbol);
		insertprocedure(command1);
	    end;
    end;
{*run,insertprocedure} 
procedure doOperation(word1: wordtype; var Output: wordtype; var found: Boolean);
	
	var
		strg : wordtype;
		i : integer;
		Y, X : real;
	
    begin
	found := true;
	if KeyPressed  then begin
		gotostart ;
	    end
	 else begin
		if word1 = 'XCOR' then begin
			strfnI(Xcor , Output);
		    end
		 else begin
			if word1 = 'THING' then begin
				word1 := parse ;
				if word1[1] <> '"' then begin
					writeln(' I can''t make a ' + 'word1');
				    end
				 else begin
					word1[1] := ':';
					loadword(word1);
					Output := parse ;
				    end;
			    end
			 else begin
				if word1 = 'YCOR' then begin
					strfnI(Ycor , Output);
				    end
				 else begin
					if word1 = 'HEADING' then begin
						strfnI(Heading , Output);
					    end
					 else begin
						if word1 = 'RANDOM' then begin
							i := ExtractInteger ;
							if i > 0 then begin
								strfnI(random(i), Output);
							    end
							 else begin
								Output := '0';
							    end;
						    end
						 else begin
							if word1 = 'NUMBERP' then begin
								Output := numberp ;
							    end
							 else begin
								if word1 = 'WORDP' then begin
									Output := wordp ;
								    end
								 else begin
									if word1 = 'NAMEP' then begin
										Output := namep ;
									    end
									 else begin
										if word1 = 'COS' then begin
											strfnr(cos(extractreal ), Output);
										    end
										 else begin
											if word1 = 'SIN' then begin
												strfnr(sin(extractreal ), Output);
											    end
											 else begin
												if word1 = 'INT' then begin
													strfnr(Int(extractreal ), Output);
												    end
												 else begin
													if word1 = 'PRODUCT' then begin
														X := extractreal *extractreal ;
														strfnr(X, Output);
													    end
													 else begin
														if word1 = 'REMAINDER' then begin
															X := extractreal ;
															Y := extractreal ;
															if Y = 0.0 then begin
																writeln('I can''t divide by zero');
															    end
															 else begin
																Y := X - (Int(X/Y)*Y);
															    end;
															strfnr(Y, Output);
														    end
														 else begin
															if word1 = 'ROUND' then begin
																strfnI(round(extractreal ), Output);
															    end
															 else begin
																if word1 = 'SQRT' then begin
																	strfnr(sqrt(extractreal ), Output);
																    end
																 else begin
																	if word1 = 'SUM' then begin
																		X := extractreal  + extractreal ;
																		strfnr(X, Output);
																	    end
																	 else begin
																		if word1 = 'DIV' then begin
																			X := extractreal ;
																			Y := extractreal ;
																			if Y = 0 then begin
																				writeln('I can''t divide by zero');
																			    end
																			 else begin
																				X := X/Y;
																				strfnr(X, Output);
																			    end;
																		    end
																		 else begin
																			if word1 = 'EQUALP' then begin
																				Output := equalp ;
																			    end
																			 else begin
																				if word1 = 'LESSP' then begin
																					Output := lessp ;
																				    end
																				 else begin
																					if word1 = 'RUN' then begin
																						run ;
																						Output := parse ;
																					    end
																					 else begin
																						if word1 = 'IF' then begin
																							ifcommand ;
																							Output := parse ;
																						    end
																						 else begin
																							found := false;
																						    end;
																					    end;
																				    end;
																			    end;
																		    end;
																	    end;
																    end;
															    end;
														    end;
													    end;
												    end;
											    end;
										    end;
									    end;
								    end;
							    end;
						    end;
					    end;
				    end;
			    end;
		    end;
	    end;
    end;
procedure DoListCommand(word1: wordtype; var found: Boolean);
	
	var
		m, H, i : integer;
		tempword : stringtype;
		intstr : wordtype;
	
    begin
	found := true;
	if word1 = 'REPEAT' then begin
		m := ExtractInteger ;
		if (m = 1) then begin
			loadword('RUN ');
		    end
		 else begin
			if (m > 1) then begin
				tempword := parselist ;
				if checklist(tempword) then begin
					m := m - 1;
					strfnI(m, intstr);
					loadword('RUN ' + tempword + ' REPEAT ' + intstr + ' ' + tempword);
				    end;
			    end
			 else begin
				writeln(' Error in REPEAT statement.');
			    end;
		    end;
	    end
	 else begin
		if word1 = 'STOP' then begin
			tempword := word1;
			while (tempword <> 'END') and (tempword <> '') do begin
				incrementstring(tempword);
			    end;
		    end
		 else begin
			if word1 = 'PO' then begin
				incrementstring(tempword);
				if (checkword(tempword)) then begin
					for i := 1 to ncommands do begin
						if (tempword = '"' + name[i]) then begin
							writeln('level=', commandlevel[i], ' ', name[i] + '=' + command[i]);
						    end;
					    end;
				    end;
			    end
			 else begin
				if word1 = 'RUN' then begin
					run ;
				    end
				 else begin
					if (word1 = 'IF') then begin
						ifcommand ;
					    end
					 else begin
						found := false;
					    end;
				    end;
			    end;
		    end;
	    end;
    end;
procedure dolistoperation(word1: stringtype; var Output: stringtype; var found: 
Boolean);
	
	{+ <<Declarations>>}
    begin
	found := true;
	if word1 = 'OUTPUT' then begin
		Output := parselist ;
		while (word1 <> 'END') and (word1 <> '') do begin
			incrementstring(word1);
		    end;
		if word1 = 'END' then begin
			loadword(word1);
		    end;
	    end
	 else begin
		if word1 = 'IF' then begin
			ifcommand ;
			Output := parselist ;
		    end
		 else begin
			if word1 = 'OUTPUT' then begin
				Output := parse ;
				while (word1 <> 'END') and (word1 <> '') do begin
					incrementstring(word1);
				    end;
				if word1 = 'END' then begin
					loadword(word1);
				    end;
			    end
			 else begin
				if word1 = 'RUN' then begin
					run ;
					Output := parselist ;
				    end
				 else begin
					found := false;
				    end;
			    end;
		    end;
	    end;
    end;
function parse;
	
	var
		word1, Output : wordtype;
		out, found : Boolean;
	
    begin
	out := false;
	Output := '"not yet';
	repeat
		incrementstring1(word1);
		if word1 = '' then begin
			write('>');
			takin ;
			incrementstring1(word1);
		    end;
		procdebug('parse', word1);
		if (word1[1] in ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '"', '-']) then begin
			parse := word1;
			out := true;
		    end
		 else begin
			if (word1[1] = '[') then begin
				writeln(' Command requires a word or a number');
				writeln(' but it received a list.');
				gotostart ;
				parse := '';
				out := true;
			    end
			 else begin
				doOperation(word1, Output, found);
				if found then begin
					out := true;
					parse := Output;
				    end
				 else begin
					findcommand(word1, found);
					procdebug('parse2', word1);
					if not found then begin
						writeln(' (parse)I don''t know how to ' + word1);
						gotostart ;
						parse := '';
						out := true;
					    end;
				    end;
			    end;
		    end;
		procdebug('parse-out' + word1 + ' ', Output);
	until out;
    end;
function parselist;
	
	var
		word1 : stringtype;
		word2, output2 : wordtype;
		Output : stringtype;
		out, found : Boolean;
	
    begin
	out := false;
	Output := '"not yet';
	repeat
		incrementstring(word1);
		if word1 = '' then begin
			write('>');
			takin ;
			incrementstring(word1);
		    end;
		procdebug('parselist', word1);
		if (word1[1] in ['[', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '"', '-']) then begin
			parselist := word1;
			out := true;
		    end
		 else begin
			dolistoperation(word1, Output, found);
			if found then begin
				out := true;
				parselist := Output;
			    end
			 else begin
				if (word1[0] = #0) then begin
					out := true;
				    end
				 else begin
					doOperation(word2, output2, found);
					if found then begin
						out := true;
						parselist := output2;
					    end
					 else begin
						findcommand(word1, found);
						procdebug('parselst2', word1);
						if not found then begin
							writeln(' (parselist)I don''t know how to ' + word1);
							gotostart ;
							parselist := '';
							out := true;
						    end;
					    end;
				    end;
			    end;
		    end;
		procdebug('parselst-out' + word1 + ' ', Output);
	until out;
    end;
{*Extracts an integer from the start of string*} 
function ExtractInteger;
	
	var
		code, m : integer;
		tempword : wordtype;
		X : real;
	
    begin
	tempword := parse ;
	if tempword = '' then begin
		writeln('The parameter is missing and has been set to 0');
		tempword := '0';
	    end;
	if Pos('.', tempword) = 0 then begin
		Val(tempword, m, code);
	    end
	 else begin
		Val(tempword, X, code);
		m := trunc(X);
	    end;
	if (code <> 0) then begin
		writeln('could not extract integer from string ' + tempword);
		loadword(tempword);
		ExtractInteger := 0;
	    end
	 else begin
		ExtractInteger := m;
	    end;
    end;
function extractreal;
	
	var
		code : integer;
		tempword : wordtype;
		X : real;
	
    begin
	tempword := parse ;
	if tempword = '' then begin
		writeln('The parameter is missing and has been set to 0');
		tempword := '0';
	    end;
	Val(tempword, X, code);
	if (code <> 0) then begin
		writeln('could not extract real number from string ' + tempword);
		loadword(tempword);
		extractreal := 0.0;
	    end
	 else begin
		extractreal := X;
	    end;
    end;
procedure startcommand ;
	
	var
		word1 : wordtype;
		out, found : Boolean;
	
    begin
	repeat
		out := false;
		incrementstring1(word1);
		if (word1[1] in ['[', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '"', '-']) then begin
			writeln(' looking for a command but found a word or number');
			gotostart ;
			out := true;
		    end
		 else begin
			if (word1[0] = #0) then begin
				out := true;
			    end
			 else begin
				docommand(word1, found);
				if not found then begin
					DoListCommand(word1, found);
					if not found then begin
						findcommand(word1, found);
						if not found then begin
							writeln('(startcommand) I don''t know how to ' + word1);
							gotostart ;
							out := true;
						    end;
					    end;
				    end;
			    end;
		    end;
	until out;
    end;
begin
Assign(list, 'lst:');
Initialize ;
repeat
	takin ;
	repeat
		startcommand ;
	until endpoint = 0;
until false;
end.

program logo;
{+I graph.p}
TYPE
stringtype = STRING[255];
penstatustype = (up, down);
maintype = ARRAY[0..500] OF Char;
wordtype = STRING[20];
pointtype = ARRAY[0..50] OF Integer;
VAR
penstatus : penstatustype;
actualcolor, backgroundcolor,ncommands, level, screensplit, hicolor : Integer;
string1 : maintype;
pointlist : pointtype;
name : ARRAY[1..10] OF wordtype;
command : ARRAY[1..10] OF stringtype;
commandlevel : ARRAY[1..10] OF Integer;
Erase, memoryexceeded, alternateinput, debug, standardscreen : Boolean;
endpoint : Integer;
commandinput,list : Text;
FUNCTION ExtractInteger : Integer; FORWARD;
FUNCTION extractreal : Real; FORWARD;
FUNCTION parse : wordtype; FORWARD;
FUNCTION parselist : stringtype; FORWARD;
PROCEDURE insertprocedure(command1 : stringtype); FORWARD;

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
flush(outfile);
close(outfile)
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
commandlevel[ncommands]:=0;
until eof(outfile);
close(outfile)
end;
  {* resets everything *}
PROCEDURE gotostart;
BEGIN
endpoint := 0;
memoryexceeded := False;
level := 0
END;
PROCEDURE procdebug(strvar, procname : wordtype);
BEGIN IF debug THEN WriteLn(List, procname+' '+strvar) END;
PROCEDURE load;
VAR i : Integer; word1, word2 : wordtype;
BEGIN
word2 := parse; i := Length(word2); word1 := Copy(word2, 2, i-1);
Assign(commandinput, word1); Reset(commandinput);
alternateinput := True
END;
PROCEDURE setpenstatus(penstat : penstatustype);
BEGIN
CASE penstat OF
down : PenDown;
up : PenUp;
END;
penstatus := penstat;
END;
FUNCTION checkword(word : wordtype) : Boolean;
BEGIN
IF (word[1] = '"') AND (word[0] > #0) THEN checkword := True ELSE
BEGIN
WriteLn(' Looking for word starting with " but found '+word);
checkword := False; gotostart
END
END;
  PROCEDURE strfnr(X : Real; VAR strvar : wordtype);
VAR i : Integer;
BEGIN
IF (X < 999999.999) AND (X > -99999.999) THEN
Str(X:10:3, strvar)
ELSE
BEGIN
WriteLn(' real number is too big and has been set to highest value')
;
IF (X < 0) THEN strvar := '-99999.999' ELSE strvar := '999999.999'
END;
i := 1;
WHILE strvar[i] = ' ' DO i := i+1;
i := i-1;
IF i > 0 THEN Delete(strvar, 1, i)
END;
PROCEDURE strfnI(X : Integer; VAR strvar : wordtype);
VAR i : Integer;
BEGIN
Str(X:10, strvar);
i := 1;
WHILE strvar[i] = ' ' DO i := i+1;
i := i-1;
IF i > 0 THEN Delete(strvar, 1, i)
END;
{* strfnr,strfni*}
  {*initializes grahpics and text window*}
PROCEDURE Initialize;
VAR i : Integer;
BEGIN
alternateinput := False; debug := False; screensplit := 13;
endpoint := 0; pointlist[0] := 0; standardscreen := True;
ncommands := 0;backgroundcolor:=1;
memoryexceeded := False;
level := 0;
Palette(0);
penstatus := down;
actualcolor := 1; hicolor := 14;
SetPenColor(1);
GraphColorMode;
TextColor(14);
TextBackground(1);
GraphBackground(1);
TurtleWindow(159, 48, 320, 96);
window(1,screensplit,40,25);
FOR i := 1 TO screensplit DO WriteLn(' ');
END;
PROCEDURE split; VAR i, X, Y : Integer;
BEGIN
X := (screensplit-1)*8; Y := (screensplit-1)*4;
IF standardscreen THEN
BEGIN
GraphColorMode;textbackground(backgroundcolor);
graphbackground(backgroundcolor);
TurtleWindow(159, Y, 320, X); Window(1, screensplit, 40, 25);
FOR i := 1 TO screensplit DO WriteLn(' ')
END
ELSE
BEGIN
HiRes; HiResColor(hicolor);
FOR i := 1 TO screensplit DO WriteLn(' ');
TurtleWindow(319, Y, 640, X); Window(1, screensplit, 80, 25);
FOR i := 1 TO screensplit DO WriteLn(' ')
END
END;
PROCEDURE Clean;
VAR ix, iy, H : Integer;
oldpenstatus : penstatustype;
BEGIN
{*records pen position and status*}
ix := Xcor;
iy := Ycor;
H := Heading;
oldpenstatus := penstatus;
{*clears screen and redraws box*}
ClearScreen; split;
{*puts turtle back*}
SetPosition(ix, iy);
SetHeading(H);
setpenstatus(oldpenstatus);
END;
{* initialize splint and clean*}
  PROCEDURE findfirstword(VAR l, m, N : Integer; strvar : stringtype);
VAR
bracket : Boolean;
bracketcount : Integer;
BEGIN
l := Length(strvar);
m := 1;
WHILE (m <= l) AND (strvar[m] = ' ') DO m := m+1;
N := m;
bracketcount := 0;
IF strvar[N] = '[' THEN bracket := True ELSE bracket := False;
WHILE (N <= l) AND ((strvar[N] <> ' ') AND (NOT bracket)) OR
(bracket AND (bracketcount > 0)) OR
(bracket AND (N = m)) do
BEGIN
IF (strvar[N] = ']') THEN bracketcount := bracketcount-1
ELSE IF (strvar[N] = '[') THEN bracketcount := bracketcount+1;
N := N+1
END
END;
PROCEDURE findlastword(VAR l, m, N : Integer; strvar : stringtype);
VAR
bracket : Boolean;
bracketcount : Integer;
BEGIN
l := Length(strvar);
m := l;
WHILE (m >= 1) AND (strvar[m] = ' ') DO m := m-1;
N := m;
bracketcount := 0;
IF strvar[N] = ']' THEN bracket := True ELSE bracket := False;
WHILE (N >= 1) AND (((strvar[N] <> ' ') AND (NOT bracket)) OR
(bracket AND (bracketcount > 0)) OR
(bracket AND (N = m)))  Do
BEGIN
IF (strvar[N] = '[') THEN bracketcount := bracketcount-1
ELSE IF (strvar[N] = ']') THEN bracketcount := bracketcount+1;
N := N-1
END
END;
{*Extracts first word from string*}
FUNCTION first(strvar : stringtype) : stringtype;
VAR l, N, m : Integer;
BEGIN
findfirstword(l, m, N, strvar);
first := Copy(strvar, m, N-m);
END;
{*Removes first word from string*}
FUNCTION Next(strvar : stringtype) : stringtype;
VAR l, N, m : Integer;
BEGIN
findfirstword(l, m, N, strvar);
Next := Copy(strvar, N+1, l-N);
END;
{*Extracts last word from string and removes it*}
PROCEDURE getlast(VAR last, strvar : stringtype);
VAR l, N, m : Integer;
BEGIN
findlastword(l, m, N, strvar);
{*WriteLn(l, ' ', m, ' ', n, ' ', strvar); debug*} ;
last := Copy(strvar, N+1, m-N);
strvar[0] := Chr(N);
END;
PROCEDURE loadword(word1 : stringtype);
VAR tempword : stringtype;
j, k, i : Integer;
BEGIN
REPEAT
getlast(tempword, word1);
k := Length(tempword);
IF endpoint > 0 THEN j := pointlist[endpoint] ELSE j := 0;
FOR i := 1 TO k DO string1[j+i] := tempword[i];
IF ((endpoint < 500) AND (pointlist[endpoint] < 4700))
THEN
BEGIN
endpoint := endpoint+1;
pointlist[endpoint] := j+k
END
ELSE IF (NOT memoryexceeded) THEN
BEGIN
memoryexceeded := True;
WriteLn(' MEMORY EXCEEDED');
word1 := '';
gotostart
END;
UNTIL word1 = ''
END;
{*removes first word from string and makes it available*}
PROCEDURE incrementstring(VAR word1 : stringtype);
VAR i, j, k : Integer;
BEGIN
IF (endpoint <> 0) THEN
BEGIN
i := pointlist[endpoint];
IF (endpoint = 1) THEN j := 1 ELSE j := pointlist[endpoint-1]+1;
FOR k := j TO i DO word1[k-j+1] := string1[k];
k := i-j+1;
word1[0] := Chr(k);
endpoint := endpoint-1
END
ELSE
BEGIN
word1 := '';
word1[0] := Chr(0)
END
END;
PROCEDURE incrementstring1(VAR word1 : wordtype);
VAR i, j, k : Integer;
BEGIN
IF (endpoint <> 0) THEN
BEGIN
i := pointlist[endpoint];
IF (endpoint = 1) THEN j := 1 ELSE j := pointlist[endpoint-1]+1;
FOR k := j TO i DO word1[k-j+1] := string1[k];
k := i-j+1;
word1[0] := Chr(k);
endpoint := endpoint-1
END
ELSE
BEGIN
word1 := '';
word1[0] := Chr(0)
END
END;

{*This procedure is used to change input to upper case*}
PROCEDURE takin;
VAR i : Integer;
tempword : stringtype;
BEGIN;
memoryexceeded := False;
IF (NOT alternateinput) THEN BEGIN
Write('?');readln(tempword)
END ELSE IF (NOT SeekEof(commandinput)) THEN
ReadLn(commandinput, tempword)
ELSE BEGIN alternateinput := False; Write('?'); ReadLn(tempword) END;
FOR i := 1 TO Length(tempword) DO tempword[i] := UpCase(tempword[i]);
loadword(tempword)
END;
{* findfirstword,findlastword,first,next,getlast,loadword,increment string
takin*}
  FUNCTION numberp : wordtype;
VAR tempword : wordtype;
BEGIN
tempword := parselist;
IF
(tempword[0] > #0) AND
(tempword[1] IN ['-', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'])
THEN
numberp := '"TRUE' ELSE numberp := '"FALSE'
END;
FUNCTION wordp : wordtype;
VAR tempword : stringtype;
BEGIN
tempword := parselist;
IF
(tempword[0] > #0) AND
(tempword[1] = '"') THEN wordp := '"TRUE'
ELSE wordp := '"FALSE'
END;
FUNCTION namep : wordtype;
VAR word1 : stringtype; m : Integer;
BEGIN
word1 := parselist;
namep := '"FALSE';
IF
(word1[0] > #0) AND
(word1[1] = '"') THEN
BEGIN
m := ncommands;
word1[1] := ':';
WHILE (m >= 1) AND (level = commandlevel[m]) DO
IF name[m] = word1 THEN namep := '"TRUE'
END;
END;
FUNCTION equalp : wordtype;
VAR word1, word2 : stringtype;
X, Y : Real; code, code1 : Integer;
BEGIN
word1 := parselist;
word2 := parselist;
IF (word1[1] = '"') OR (word2[1] = '"') OR (word1[1] = '[') OR
(word2[1] = '[') OR (word1[0] = #0) OR (word2[0] = #0) THEN
BEGIN
IF word1 = word2 THEN equalp := '"TRUE' ELSE equalp := '"FALSE'
END ELSE
BEGIN
Val(word1, X, code);
Val(word2, Y, code1);
IF (X = Y) AND ((code+code1) = 0) THEN equalp := '"TRUE'
ELSE equalp := '"FALSE'
END
END;
FUNCTION lessp : wordtype;
VAR word1, word2 : stringtype;
X, Y : Real; code, code1 : Integer;
BEGIN
word1 := parselist;
word2 := parselist;
IF (word1[1] = '"') OR (word1[1] = '[') OR (word1[0] = #0) OR (word2[0] =
#0) THEN
IF (word1 < word2) THEN lessp := '"TRUE' ELSE lessp := '"FALSE'
ELSE
BEGIN
lessp := '"FALSE';
Val(word1, X, code);
Val(word2, Y, code1);
IF (code <> 0) OR (code1 <> 0) THEN
WriteLn('I can''t compare '+word1+' with '+word2) ELSE IF
X < Y THEN lessp := '"TRUE'
END
END;
{*print,if,to, findcommand,checklist,make}
  PROCEDURE TOinput;
VAR word1 : stringtype;
BEGIN;
ncommands := ncommands+1;
incrementstring(word1);
name[ncommands]:=word1;
command[ncommands] := '';
REPEAT
BEGIN
incrementstring(word1);
IF word1 = '' THEN
BEGIN
Write('>');
takin;
incrementstring(word1)
END;
command[ncommands] := command[ncommands]+word1+' ';
END
UNTIL (word1 = 'END');
WriteLn(name[ncommands]+' Defined');
commandlevel[ncommands] := level
END;
FUNCTION checklist(word : stringtype) : Boolean;
BEGIN
IF NOT((word[1] = '[') AND (word[0] > #0)) THEN
BEGIN
WriteLn(' Looking for a list but found '+word);
gotostart; checklist := False
END ELSE checklist := True
END;
PROCEDURE findcommand(word1 : wordtype; VAR found : Boolean);
VAR m : Integer; command1 : stringtype;
BEGIN
found := False;
m := ncommands;
WHILE (m >= 1) DO
IF (word1 = name[m]) THEN
BEGIN
command1 := command[m];
IF (word1[1] <> ':') THEN level := level+1;
insertprocedure(command1);
m := -1; found := True
END
ELSE m := m-1;
IF (NOT found) THEN WriteLn('(findcommand)I don''t know how to '+word1)
END;
PROCEDURE Make;
VAR tempword : wordtype; templist : stringtype;
m : Integer;
BEGIN
tempword := parse;
templist := parselist;
IF checkword(tempword) THEN BEGIN
m := ncommands;
tempword[1] := ':';
WHILE (m >= 1) AND (commandlevel[m] = level) DO
IF tempword = name[m] THEN
BEGIN
command[m] := templist;
m := -1
END ELSE m := m-1;
IF m <> -1 THEN
BEGIN
commandlevel[ncommands+1] := level;
ncommands := ncommands+1;
name[ncommands] := tempword;
command[ncommands] := templist
END;
IF command[ncommands] = '' THEN
BEGIN
WriteLn('The parameter is missing and has been set to 0');
command[ncommands] := '0'
END
END
END;
PROCEDURE ifcommand;
VAR word1 : wordtype; tempword, tempword2 : stringtype;
BEGIN
word1 := parse;
IF word1 = '"TRUE' THEN
BEGIN
tempword := parselist;
IF endpoint > 0 THEN
IF string1[pointlist[endpoint-1]+1] = '[' THEN
tempword2 := parselist;
IF checklist(tempword) THEN BEGIN
loadword(tempword);
loadword('RUN');
END
END ELSE IF word1 = '"FALSE' THEN
BEGIN
tempword := parselist;
IF checklist(tempword) THEN BEGIN
IF endpoint <> 0 THEN
IF string1[pointlist[endpoint-1]+1] = '[' THEN
BEGIN
tempword := parselist;
loadword(tempword);
loadword('RUN');
END
END
END
ELSE
BEGIN
WriteLn(' IF statement requires "TRUE or "FALSE');
WriteLn(' instead it received '+word1);
gotostart;
END;
END;
PROCEDURE print;
VAR templist : stringtype;
BEGIN templist := parselist; WriteLn(templist) END;
  PROCEDURE docommand(word1 : wordtype; VAR found : Boolean);
VAR X, Y : Real; m, code, code1, H, i : Integer;
oldpenstatus : penstatustype;
strg : wordtype;
BEGIN
found := True;
IF KeyPressed THEN gotostart
ELSE IF
(word1 = 'DEBUG') THEN debug := True ELSE IF
(word1 = 'NODEBUG') THEN debug := False ELSE IF
(word1 = 'BACK') OR (word1 = 'BK') THEN Back(ExtractInteger) ELSE IF
word1 = 'CS' THEN ClearScreen ELSE IF
(word1 = 'FORWARD') OR (word1 = 'FD') THEN Forwd(ExtractInteger) ELSE IF
word1 = 'HT' THEN HideTurtle ELSE IF
word1 = 'HOME' THEN Home ELSE IF
word1 = 'WRAP' THEN Wrap ELSE IF
word1 = 'NOWRAP' THEN NoWrap ELSE IF
word1 = 'READ' then read else if
word1 = 'SAVE' then save else if
(word1 = 'PENDOWN') OR (word1 = 'PD') THEN
BEGIN
PenDown;
{*cancels erasing pen*}
IF Erase THEN
BEGIN
Erase := False;
SetPenColor(actualcolor)
END
END
ELSE IF
(word1 = 'PENUP') OR (word1 = 'PU') THEN
BEGIN;
PenUp;
IF Erase THEN
BEGIN;
Erase := False;
SetPenColor(actualcolor)
END
END ELSE IF
(word1 = 'LEFT') OR (word1 = 'LT') THEN TurnLeft(ExtractInteger) ELSE IF
(word1 = 'RIGHT') OR (word1 = 'RT') THEN TurnRight(ExtractInteger) ELSE IF
word1 = 'ST' THEN ShowTurtle ELSE IF
word1 = 'SETH' THEN SetHeading(ExtractInteger) ELSE IF
word1 = 'SETBG' THEN GraphBackground(ExtractInteger) ELSE IF
word1 = 'SETPC' THEN Palette(ExtractInteger) ELSE IF
{*THIS CHANGES THE PALETTE RATHER THEN THE COLOR*}
word1 = 'SETPN' THEN
BEGIN
actualcolor := ExtractInteger;
SetPenColor(actualcolor);
END ELSE IF
word1 = 'CLEAN' THEN Clean
ELSE IF
word1 = 'PE' THEN
BEGIN
SetPenColor(0);
Erase := True
END ELSE IF
word1 = 'QUIT' THEN BEGIN TextMode; Halt END ELSE IF
word1 = 'TO' THEN TOinput
ELSE IF
word1 = 'POALL' THEN FOR i := 1 TO ncommands DO
WriteLn('level=', commandlevel[i], ' ', name[i]+'='+command[i])
ELSE IF
word1 = 'PRINT' THEN print ELSE IF
word1 = 'MAKE' THEN Make
ELSE IF
word1 = 'END' THEN
BEGIN
WHILE commandlevel[ncommands] = level DO ncommands := ncommands-1;
level := level-1;
END
ELSE IF
word1 = 'WAIT' THEN
BEGIN
X := extractreal;
X := X*17.0;
Delay(Round(X))
END
ELSE IF word1 = 'LOAD' THEN load
ELSE IF (word1 = 'SPLIT') THEN
BEGIN
m := ExtractInteger; IF (NOT(m IN [2..24])) THEN
WriteLn(' MUST SPLIT BETWEEN 2 and 24') ELSE screensplit := m;
split
END
ELSE IF (word1 = 'HIRES') THEN BEGIN standardscreen := False; split END
ELSE IF (word1 = 'LORES') THEN BEGIN standardscreen := True; split END
ELSE found := False
END;
{*This executes all the primatives*}
PROCEDURE run;
VAR tempword : stringtype; m : Integer;
BEGIN
tempword := parselist;
IF checklist(tempword) THEN BEGIN
m := Length(tempword);
loadword(Copy(tempword, 2, m-2))
END
END;

PROCEDURE insertprocedure;
VAR word1 : stringtype; tempbol : Boolean;
BEGIN
word1 := first(command1);
IF word1[1] <> ':' THEN loadword(command1+' ')
ELSE
BEGIN
word1[1] := '"';
loadword(word1);
command1 := Next(command1);
docommand('MAKE', tempbol);
insertprocedure(command1)
END
END;
{*run,insertprocedure}
PROCEDURE doOperation(word1 : wordtype; VAR Output : wordtype; VAR found :
Boolean);
VAR strg : wordtype;
i : Integer;
Y, X : Real;
BEGIN
found := True;
IF KeyPressed THEN gotostart ELSE IF
word1 = 'XCOR' THEN strfnI(Xcor, Output) ELSE IF
word1 = 'THING' THEN
BEGIN
word1 := parse;
IF word1[1] <> '"' THEN WriteLn(' I can''t make a '+'word1') ELSE
BEGIN
word1[1] := ':';
loadword(word1);
Output := parse
END
END ELSE IF
word1 = 'YCOR' THEN strfnI(Ycor, Output) ELSE IF
word1 = 'HEADING' THEN strfnI(Heading, Output) ELSE IF
word1 = 'RANDOM' THEN
BEGIN
i := ExtractInteger;
IF i > 0 THEN strfnI(Random(i), Output) ELSE Output := '0';
END ELSE IF
word1 = 'NUMBERP' THEN Output := numberp
ELSE IF
word1 = 'WORDP' THEN Output := wordp
ELSE IF
word1 = 'NAMEP' THEN Output := namep
ELSE IF
word1 = 'COS' THEN strfnr(Cos(extractreal), Output)
ELSE IF
word1 = 'SIN' THEN strfnr(Sin(extractreal), Output)
ELSE IF
word1 = 'INT' THEN strfnr(Int(extractreal), Output)
ELSE IF
word1 = 'PRODUCT' THEN
BEGIN
X := extractreal*extractreal;
strfnr(X, Output)
END ELSE IF
word1 = 'REMAINDER' THEN
BEGIN
X := extractreal;
Y := extractreal;
IF Y = 0.0 THEN WriteLn('I can''t divide by zero') ELSE
Y := X-(Int(X/Y)*Y);
strfnr(Y, Output)
END ELSE IF
word1 = 'ROUND' THEN strfnI(Round(extractreal), Output)
ELSE IF
word1 = 'SQRT' THEN strfnr(Sqrt(extractreal), Output)
ELSE IF
word1 = 'SUM' THEN
BEGIN
X := extractreal+extractreal;
strfnr(X, Output)
END ELSE IF
word1 = 'DIV' THEN
BEGIN
X := extractreal;
Y := extractreal;
IF Y = 0 THEN WriteLn('I can''t divide by zero') ELSE
BEGIN
X := X/Y;
strfnr(X, Output)
END
END ELSE IF
word1 = 'EQUALP' THEN Output := equalp
ELSE IF
word1 = 'LESSP' THEN Output := lessp
ELSE IF
word1 = 'RUN' THEN BEGIN run; Output := parse END
ELSE IF word1 = 'IF' THEN BEGIN ifcommand; Output := parse END
ELSE found := False
END;
  PROCEDURE DoListCommand(word1 : wordtype; VAR found : Boolean);
VAR m, H,i : Integer;
tempword : stringtype;
intstr : wordtype;
BEGIN
found := True;
IF
word1 = 'REPEAT' THEN
BEGIN
m := ExtractInteger;
IF (m = 1) THEN loadword('RUN ')
ELSE IF (m > 1) THEN
BEGIN
tempword := parselist;
IF checklist(tempword) THEN BEGIN
m := m-1;
strfnI(m, intstr);
loadword('RUN '+tempword+' REPEAT '+intstr+' '+tempword)
END END
ELSE WriteLn(' Error in REPEAT statement.')
END
ELSE IF
word1 = 'STOP' THEN
begin
tempword:=word1;
WHILE (tempword <> 'END') AND (tempword <> '')
DO incrementstring(tempword)
end
else if
word1 = 'PO' THEN BEGIN
incrementstring(tempword);
IF (checkword(tempword)) THEN BEGIN
FOR i := 1 TO ncommands DO
IF (tempword = '"'+name[i]) THEN
WriteLn('level=', commandlevel[i], ' ', name[i]+'='+command[i]
)
END
END

ELSE IF
word1 = 'RUN' THEN run
ELSE IF
(word1 = 'IF') THEN ifcommand
ELSE found := False
END;
PROCEDURE dolistoperation
(word1 : stringtype; VAR Output : stringtype; VAR found : Boolean);
BEGIN
found := True;
IF word1 = 'OUTPUT' THEN
BEGIN
Output := parselist;
WHILE (word1 <> 'END') AND (word1 <> '') DO incrementstring(word1);
IF word1 = 'END' THEN loadword(word1)
END
ELSE IF word1 = 'IF' THEN BEGIN ifcommand; Output := parselist END
else if
word1 = 'OUTPUT' THEN
BEGIN
Output := parse;
WHILE (word1 <> 'END') AND (word1 <> '') DO incrementstring(word1);
IF word1 = 'END' THEN loadword(word1)
END

ELSE IF
word1 = 'RUN' THEN BEGIN run; Output := parselist END
ELSE found := False;
END;
  FUNCTION parse;
VAR word1, Output : wordtype;
out, found : Boolean;
BEGIN
out := False; Output := '"not yet';
REPEAT
incrementstring1(word1);
if word1='' then
begin
write('>');
takin;
incrementstring1(word1)
end;
procdebug('parse', word1);
IF (word1[1] IN ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '"'
, '-'])
THEN
BEGIN parse := word1; out := True END
ELSE
IF (word1[1] = '[') THEN
BEGIN
WriteLn(' Command requires a word or a number');
WriteLn(' but it received a list.');
gotostart; parse := '';
out := True
END
ELSE
BEGIN
doOperation(word1, Output, found);
IF found THEN
BEGIN
out := True;
parse := Output;
END
ELSE
BEGIN
findcommand(word1, found); procdebug('parse2', word1);
IF NOT found THEN
BEGIN
WriteLn(' (parse)I don''t know how to '+word1);
gotostart;
parse := '';
out := True
END
END
END
; procdebug('parse-out'+word1+' ', Output)
UNTIL out
END;

FUNCTION parselist;
VAR word1 : stringtype;
word2,output2:wordtype;
Output : stringtype;
out, found : Boolean;
BEGIN
out := False; Output := '"not yet';
REPEAT
incrementstring(word1);
if word1='' then
begin
write('>');
takin;
incrementstring(word1)
end;
procdebug('parselist', word1);
IF (word1[1] IN
['[', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '"', '-'])
THEN BEGIN parselist := word1; out := True END
ELSE
BEGIN
dolistoperation(word1, Output, found);
IF found THEN
BEGIN
out := True;
parselist := Output
END
ELSE IF (word1[0] = #0) THEN out := True
ELSE
BEGIN
doOperation(word2, Output2, found);
IF found THEN
BEGIN
out := True;
parselist := Output2
END
ELSE
BEGIN
findcommand(word1, found);
procdebug('parselst2', word1);
IF NOT found THEN
BEGIN
WriteLn(' (parselist)I don''t know how to '+
word1);
gotostart;
parselist := '';
out := True
END
END
END
END
; procdebug('parselst-out'+word1+' ', Output)
UNTIL out
END;
  {*Extracts an integer from the start of string*}
FUNCTION ExtractInteger;
VAR code, m : Integer; tempword : wordtype; X : Real;
BEGIN
tempword := parse;
IF tempword = '' THEN
BEGIN
WriteLn('The parameter is missing and has been set to 0');
tempword := '0'
END;
IF Pos('.', tempword) = 0 THEN
Val(tempword, m, code) ELSE
BEGIN
Val(tempword, X, code);
m := Trunc(X)
END;
IF (code <> 0) THEN
BEGIN
WriteLn('could not extract integer from string '+tempword);
loadword(tempword);
ExtractInteger := 0
END
ELSE ExtractInteger := m
END;
FUNCTION extractreal;
VAR code : Integer; tempword : wordtype; X : Real;
BEGIN
tempword := parse;
IF tempword = '' THEN
BEGIN
WriteLn('The parameter is missing and has been set to 0');
tempword := '0'
END;
Val(tempword, X, code);
IF (code <> 0) THEN
BEGIN
WriteLn('could not extract real number from string '+tempword);
loadword(tempword);
extractreal := 0.0
END
ELSE extractreal := X
END;
  PROCEDURE startcommand;
VAR word1 : wordtype;
out, found : Boolean;
BEGIN
REPEAT
out := False;
incrementstring1(word1);
IF (word1[1] IN ['[', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'
, '"', '-'])
THEN
BEGIN
WriteLn(' looking for a command but found a word or number');
gotostart;
out := True
END
ELSE IF (word1[0] = #0) THEN out := True ELSE
BEGIN
docommand(word1, found);
IF NOT found THEN
BEGIN
DoListCommand(word1, found);
IF NOT found THEN
BEGIN
findcommand(word1, found);
IF NOT found THEN
BEGIN
WriteLn('(startcommand) I don''t know how to '+
word1);
gotostart;
out := True
END
END
END
END
UNTIL out
END;

BEGIN
assign(list,'lst:');
Initialize;
REPEAT
BEGIN
takin;
REPEAT startcommand UNTIL endpoint = 0;
END;
UNTIL False;
END.

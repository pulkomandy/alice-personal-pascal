program blat;

type rec = record field1, f2 : integer; end;

var
	hi : rec;
	there : rec absolute 0 : 0;

begin
with hi do f2 := 1;
with there do field1 := 2;
end.

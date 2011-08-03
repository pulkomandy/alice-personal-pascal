
program blat;

type address = record offset, segment : integer; end;

var a: address;
    b: address absolute 0 : 0;

begin

with a do
	segment := 0;
with b do
	offset := 0;
end.

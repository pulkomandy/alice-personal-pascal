program blat(input,output);

type
	screen = array[0..24] of array [0..79] of integer;

var
	i, jj : char;
	j, kk : ^integer;
	hello : integer absolute dseg : $80;
	ab : screen absolute $b800 : 0;
	hi : array[1..20] of real absolute cseg:80;
	there : char absolute i;
	p,q : char absolute i;

begin

mem[1 : 2 ] := 3;
end.

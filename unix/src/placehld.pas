program {+ <<Ignored Name>>}(input, output);

const
	{+ <<Name>>} = {+ <<Constant>>};        
	fred = -{+ <<Constant>>};       
	sam = +{+ <<Constant>>};        

type
	{+ <<Name>>} = {+ <<Type>>};    

var
	{+ <<Name>>} : {+ <<Type>>};    
	a : record      
		{+ <<Field-Declaration>>}
		a : {+ <<Type>>};       
		{+ <<Name>>} : integer; 
	    end;

procedure {+ <<Routine Name>>}({+ <<Parameter>>});
	
	function {+ <<Routine Name>>}(var {+ <<Name>>}: {+ <<Type-Name>>}) : {+ <<Type-Name>>};
		
		{+ <<Declarations>>}
	    begin
		{+ <<Statement>>}
	    end;
    begin
	{+ <<Statement>>}
    end;
begin
if {+ <<Value>>} then begin
	repeat
		while {+ <<Value>>} do begin
			with {+ <<Variable>>} do begin
				for {+ <<Variable>>} := {+ <<Value>>} to {+ <<Value>>} do begin
					{+ <<Statement>>}
				    end;
			    end;
		    end;
	until {+ <<Value>>};
    end;
a.{+ <<Field>>} := {+ <<Value>>};
{+ <<Block-Statement>>} begin
	{+ <<Statement>>}
    end;
{+ <<Statement>>}
{+ <<Procedure Name>>}(a:{+ <<Value>>}:{+ <<Value>>});
end.

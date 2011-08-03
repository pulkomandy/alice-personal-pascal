program ian(input,output);

const

   { These definitions are useful for manipulating strings }
   msl			=     80;	{ max length of a string        }
   mslPlusOne		=     81;	{ max length of string buffer   }
   mns			=     10;	{ max number symbols - getsyms  }

   { For converting back and forth between radians and degrees }
   radPerDeg		=  0.0174532925;
   degPerRad		= 57.2957795131;


type

   fileOfChar		= file of char;

   { These types are useful for manipulating strings }
   ptrToAlfa		= ^ alfa;
   alfa			= packed array[1..mslPlusOne] of char;
   strind		= 0..msl;
   string		= record
			 	len: strind;
			 	str: alfa
			  end;
   arrayOfStrings	= array[1..mns] of string;
   ptrToString          = ^ string;

var

  terminal	: fileOfChar;	{ for prompts, error messages, etc.	}

{ #include	"/u/gr/include/grutil/grutil.h" }

{ Copyright (c) 1980,1981 Ian D. Allen / University of Waterloo }
{ REVMKR: make figures of revolution }

procedure main;

const
    PI		= 3.14159265;
    MAXSLICES	= 1024;		{ Max points allowed in profile file. }
    MAXFACES	= 1024;		{ Max number of faces in revolution. }
    MAXPOLYS	= 100;		{ Max polygon nodes which can be generated. }
    MAXCOMMENTS	= 20;		{ Max comments which can be
						passed to output file. }

type
    nonneg	= 0..maxint;	{ Non-negative integer. }
    positive	= 1..maxint;	{ Positive integer. }
    polytype	= ( polygon, rotation, user );
    commentindex= 1..MAXCOMMENTS;
    userindex	= 1..MAXPOLYS;
    commentptr	= ^ alfa;

var
    commentarray: array [1..MAXCOMMENTS] of commentptr; { Store comments. }
    numcomments	: 0..MAXCOMMENTS;
    slicearray	: array [1..MAXSLICES] of
	         record z,x : real end;	{ Store profile points. }
    numslices	: 0..MAXSLICES;
    polyarray	: array [1..MAXPOLYS] of { Store polygon/rotation/user nodes. }
			record
				down	: integer;
				next	: integer;
				case ptype	: polytype of {begin}
					rotation: ( degrees : real );
					polygon	: ( nverts : nonneg );
					user	: ( usertype:char; ptr: ^alfa );
			end {record};
    numpolys	: 0..MAXPOLYS;
    numtransnodes	: 0..MAXPOLYS;	{ Number of User Transformaion Nodes. }
    numfaces	: integer;	{ Number of faces in revolution. }
    dtheta	: real;		{ Delta Theta (radians) for rotation. }
    dthetadegree: real;		{ Delta Theta (degrees) for rotation. }
    cosdtheta,
    sindtheta	: real;
    dotop,
    dobottom	: boolean;	{ Do Top or Bottom polygon flags. }
    topwedgerot	: positive;	{ Start of wedge rotation polygon nodes. }
    startwedge	: positive;	{ Pointer to start of wedge
						rotation polygon nodes. }
    starttop	: positive;	{ Location of Top polygon. }
    startbottom	: positive;	{ Locaton of Bottom polygon. }
    startfigure	: positive;	{ Entry/start node for complete figure of rot. }
    startuser	: positive;	{ Entry/start node in User Transform Section. }
    pnum	: nonneg;	{ Polygon/Rotation node number. }
    pcheck	: nonneg;	{ To check node number when
						reading User Transform Nodes. }
    p1,p2,p3,p4	: positive;	{ Vertex numbers used building wedge polygons. }
    vertcounter	: nonneg;	{ Vertex counter used building wedge polygons. }
    maxtwo	: positive;	{ Max power of two <= numfaces. }
    maxpower	: nonneg;	{ Exponent of maxtwo, i.e. log2(maxtwo). }
    donesofar	: positive;	{ Number of rotation faces done so far. }
    remainder	: nonneg;	{ Number of rotation faces left to do. }
    rempower	: nonneg;	{ Max power of two <= remainder. }
    remtwo	: positive;	{ Exponent of rempower, i.e. log2(rempower). }
    horiz	: nonneg;	{ Number of nodes in horizontal remainder chain}
    downlink	: positive;	{ Index into vertical power-of-two tree. }
    nextlink	: nonneg;	{ Link to next polygon used
							building wedge polys}
    down	: integer;	{ Link to down polygon in User
							Transform Section. }
    next	: integer;	{ Link to next polygon in User
							Transform Section. }
    offset	: positive;	{ Used in renumbering User Transform nodes. }
    totalverts	: positive;	{ Total number of vertices generated. }
    totalpolys	: positive;	{ Total number of polygon/rotation
							nodes generated. }
    rightleft	: char;		{ Right or Left -handed normal points inward. }
    saverightleft	: char;	{ Saved (input) rightleft flag. }
    x,y,z	: real;
    vnum	: nonneg;	{ Vertex Number. }
    ptype	: char;		{ Type of User Transform node. }
    syms	: arrayOfStrings;	{ Used by GETSYMS. }
    nsyms	: integer;	{ Used by GETSYMS. }
    i,j		: nonneg;
    profile	: fileOfChar;	{ Input profile data file. }
    profilename	: alfa;
    proeof	: boolean;	{ Set true on end-of-file for profile file. }
    proerrflag	: boolean;	{ Set and checked for profile line errors. }
    prostring	: string;	{ Profile file input line. }
    profilineno	: nonneg;	{ Current profile line number. }
    outfile	: text;		{ Ouput file for ReadScene. }
    outfilename	: alfa;

procedure storecomment( var c : alfa );

begin
    if ( numcomments < MAXCOMMENTS ) then begin
	numcomments := numcomments + 1;
	new( commentarray[numcomments] ); { Create space for comment. }
	c[1] := 'C'; { Make sure comment starts with C for ReadScene. }
	commentarray[numcomments]^ := c; { Save comment. }
    end {if}
    else begin
	commentarray[MAXCOMMENTS]^ := 'C ...comments truncated...';
    end {else};

end {procedure storecomment};

procedure freecomment( i : commentindex );

begin
	dispose( commentarray[i] );    { Free storage. }
end {procedure freecomment};

procedure storepoly( pdown, pnext : nonneg );

begin
    if ( numpolys < MAXPOLYS ) then begin
	numpolys := numpolys + 1;
	polyarray[numpolys].ptype := polygon;
	polyarray[numpolys].down  := pdown;
	polyarray[numpolys].next  := pnext;
	{ Don't need to assign .nverts -- it is always zero. }
    end {if}
    else begin
	writeln( output,
	    'REVMKR: Too many polygons in storepoly list, maximum is ',
	    MAXPOLYS:1 );
	halt;
    end {else};

end {procedure storepoly};

procedure storerot( pdown, pnext: nonneg;
		    rot		: real );
begin
    if ( numpolys < MAXPOLYS ) then begin
	numpolys := numpolys + 1;
	polyarray[numpolys].ptype := rotation;
	polyarray[numpolys].down  := pdown;
	polyarray[numpolys].next  := pnext;
	polyarray[numpolys].degrees := rot;
    end {if}
    else begin
	writeln( output,
	    'REVMKR: Too many polygons in storerot list, maximum is ',
	    MAXPOLYS:1 );
	halt;
    end {else};

end {procedure storerot};

procedure storeuser( 	 pdown, pnext	: integer;
			 usertype	: char;
		     var pstring	: alfa );
begin
    if ( numpolys < MAXPOLYS ) then begin
	numpolys := numpolys + 1;
	polyarray[numpolys].ptype := user;
	polyarray[numpolys].down  := pdown;
	polyarray[numpolys].next  := pnext;
	polyarray[numpolys].usertype  := usertype;
	new( polyarray[numpolys].ptr ); { To hold string. }
	polyarray[numpolys].ptr^ := pstring;
    end {if}
    else begin
	writeln( output,
	    'REVMKR: Too many polygons in storeuser list, maximum is ',
	    MAXPOLYS:1 );
	halt;
    end {else};

end {procedure storeuser};

procedure freeuser( i : userindex );

begin
    dispose( polyarray[i].ptr );    { Return to free storage. }

end {procedure freeuser};

function Power2(     i			: nonneg;
		 var outexponent	: nonneg ) : nonneg;

var
    outpower : nonneg;

begin
    outpower := 1;
    outexponent := 0;
    while ( outpower <= i ) do begin
	outpower := outpower * 2;
	outexponent := outexponent + 1;
    end {while};
    Power2 := outpower div 2;
    outexponent := outexponent - 1;

end {function Power2 };

begin {procedure main}

    { *** Run-Time Parameter Input Section. *** }

    writeln( output, 'Enter input and output filenames:' );
    Getline( prostring );
    getsyms( prostring, syms, nsyms );
    if ( nsyms <= 1 ) then begin
	syms[2].str := 'revout ';
    end {if};
    profilename := syms[1].str;
    outfilename := syms[2].str;

    writeln( output, 'Enter number of faces, Top(t/f), Bottom(t/f):' );
    Getline( prostring );
    getsyms( prostring, syms, nsyms );
    numfaces := convi( syms[1] );
    dotop := ( syms[2].str[1] in ['t','T'] );
    dobottom := ( syms[3].str[1] in ['t','T'] );
    if ( numfaces < 2 ) or ( numfaces > MAXFACES ) then begin
	writeln( output,
	    'REVMKR: Number of faces must be 2..', MAXFACES:1 );
	halt;
    end {if};

    writeln( output, 'Enter right or left handed normal points in (r/l):' );
    readln( input, rightleft ); { This may change when data is read. }
    if (rightleft in ['l','L']) then begin
	rightleft := 'l';
    end {if}
    else begin
	if (rightleft in ['r','R']) then begin
	    rightleft := 'r';
	end {if}
	else begin
	    writeln( output,
		'REVMKR: Error -- polygon normal must be R or L. "',
		rightleft, '" found.' );
	    halt;
	end {else};
    end {else};

    saverightleft := rightleft; { Save what user typed in. }

    { *** Initialization Section. *** }

    numcomments := 0; { No comment lines yet. }
    numpolys  := 0; { No polygons in array yet. }
    proerrflag := false; { No error so far. }
    profilineno := 0; { No lines read yet. }

    { *** Rotation Parameters. *** }

    dtheta := (2 * PI) / numfaces; { Delta Theta. }
    dthetadegree := 360.0 / numfaces; { In Degrees for output. }
    sindtheta := sin( dtheta );
    cosdtheta := cos( dtheta );

    { *** Save Input Comments. *** }

    reset( profile, profilename ); { Open profile data input file. }
    proeof := ( eof(profile) );

    if ( proeof ) then begin
	writeln( output, 'REVMKR: Profile file is empty: ', profilename );
	halt;
    end {if};

    fGetline( profile, prostring );
    profilineno := profilineno + 1;

    while ( prostring.str[1] in ['c','C','*',';'] ) and not ( proeof ) do begin
	storecomment( prostring.str );    { Save comment lines. }
	if ( eof(profile) ) then proeof := true
	else begin
	    fGetline( profile, prostring );
	    profilineno := profilineno + 1;
	end {else};
    end {while};

    { *** Input Profile Slices into Slicearray. *** }

    if ( proeof ) then begin
	writeln( output,
	    'REVMKR: No profile data on file ', profilename );
	halt;
    end {if};

    numslices := 0; { No slices yet. }
    while not ( (prostring.str[1] in ['c','C','*',';']) or ( proeof ) ) do begin
	getsyms( prostring, syms, nsyms );

	if (syms[2].len = 0) then begin
	    writeln( output, 'REVMKR: Missing co-ordinate on line ',
	             profilineno:1, ' in profile file ', profilename );
	    proerrflag := true;
	end {if};

	z := convr( syms[1] );
	x := convr( syms[2] );

	if ( numslices < MAXSLICES ) then begin
	    numslices := numslices + 1;
	    slicearray[numslices].x := x;
	    slicearray[numslices].z := z;
	end {if}
	else begin
	    writeln( output,
		'REVMKR: Too many data points in profile file,',
		' maximum is ', MAXSLICES:1 );
	    halt;
	end {else};

	if ( eof(profile) ) then proeof := true
	else begin
	    fGetline( profile, prostring );
	    profilineno := profilineno + 1;
	end {else};
    end {while};

    if ( proerrflag ) then begin
	halt;
    end {if};

    writeln( output, 'Number of data points read from profile file: ',
	     numslices:1 );
    if ( numslices < 2 ) then begin
	writeln( output,
	    'REVMKR: Need at least two points in profile file.' );
	halt;
    end {if};

    { Make sure we traverse polygons clockwise, viewed externally. }

    if ( slicearray[1].z > slicearray[numslices].z ) then begin
	if ( rightleft = 'l' ) then begin
	    rightleft := 'r'; { We are going the other way! }
	end {if}
	else begin
	    rightleft := 'l'; { We are going the other way! }
	end {else};
    end {if};

    { *** Input User-Transformation nodes *** }

    if not ( proeof ) then begin
	while ( prostring.str[1] in ['c','C','*',';'] )
		and not ( proeof ) do begin
	    storecomment( prostring.str );    { Save comment lines. }
	    if ( eof(profile) ) then proeof := true
	    else begin
	        fGetline( profile, prostring );
	        profilineno := profilineno + 1;
	    end {else};
	end {while};

	pcheck := 0; { Use this to verify pnum correct sequence. }
	while not ( proeof ) do begin
	    { Read user transformation and save it for later. }
	    getsyms( prostring, syms, nsyms );
	    if ( syms[1].len > 0 ) then begin
	        pcheck := pcheck + 1; { Increment check counter too. }
	        pnum := convi( syms[1] ); { Node number. }
	        ptype := syms[2].str[1]; { Polygon node type. }
	        down := convi( syms[3] ); { Down pointer. }
	        next := convi( syms[4] ); { Next pointer. }
	        if (pnum<>pcheck) or (syms[5].len=0) then begin
	            writeln( prostring.str );
	            writeln( output,
			'REVMKR: Unrecognizable user-transform nodes ',
			'after second set of Comments -- ignored.' );
	            while ( numpolys > 0 ) do begin
	                freeuser( numpolys ); { Free storage. }
	                numpolys := numpolys - 1;
	            end {while};
	            proeof := true; { Premature end of file. }
	        end {if}
	        else begin
	            { Shift string left 4 args to get past node numbers. }
	            i := 1;
	            for j:= 1 to 4 do begin
	                while ( prostring.str[i] = ' ' ) do i := i + 1;
	                while ( prostring.str[i] <> ' ' ) do i := i + 1;
	            end {for};
	            { Move remainder of string to beginning of string. }
	            j := 1;
	            for i := i to msl do begin
	                prostring.str[j] := prostring.str[i];
	                j := j + 1;
	            end {for};
	            { Pad with blanks. }
	            for j := j to msl do begin
	                prostring.str[j] := ' ';
	            end {for};
	            storeuser( down, next, ptype, prostring.str );
	        end {else};
	    end {if};

	    if ( proeof or eof(profile) ) then proeof := true
	    else begin
		fGetline( profile, prostring );
	        profilineno := profilineno + 1;
	    end {else};
	end {while};
    end {if};

    rewrite( outfile, outfilename); { Get ready to write data. }

    numtransnodes := numpolys; { Number of user-transform nodes. }

    { *** Verify Existence of Top and Bottom Polygons. *** }

    if ( slicearray[1].x = 0.0 ) then dotop := false;
    if ( slicearray[numslices].x = 0.0 ) then dobottom := false;

    { *** Calculate total number of vertices needed. *** }

    totalverts := numslices * 2; { Start with wedge vertices. }
    if ( dotop ) then totalverts := totalverts + numfaces; { Add top. }
    if ( dobottom ) then totalverts := totalverts + numfaces; { Add bottom. }

    { *** Calculate polygon rotation tree for wedge of vertices. *** }

    { This is a binary tree using powers of two to construct the needed
      rotations with as few polygon transformation nodes as possible.
      It consists of a "vertical" string of powers of two, and a "horizontal"
      string of pointers into the vertical string.  The horizontal string
      deals with the rotation remaining after the preceding power of two has
      been done. }

    { The numslices-1 wedge polygons precede the polygons of the wedge tree. }
    topwedgerot := numslices; { There are numslices-1 polys in the wedge. }
    pnum := topwedgerot; { Start counting from here. }

    maxtwo := Power2( numfaces, maxpower ); { Find biggest power of 2 <= }
    donesofar := maxtwo; { Vertical rotation tree handles the power of 2. }

    remainder := numfaces - donesofar; { Number of faces left to do. }
    horiz := 0; { Horizontal rotation string starts zero length. }
    while (remainder > 0) do begin
	remainder := remainder - Power2( remainder, rempower );
	horiz := horiz + 1; { Add a node to horizontal string. }
    end {while};

    remainder := numfaces - donesofar; { Number of faces left to do. }
    while (remainder > 0) do begin
	remtwo := Power2( remainder, rempower );
	downlink := topwedgerot + horiz + (maxpower-rempower); {Index vertical}
	pnum := pnum + 1;
	storerot( downlink, pnum, donesofar*dthetadegree ); {A horizontal node}
	donesofar := donesofar + remtwo; { Done this many more now. }
	remainder := remainder - remtwo; { This much less remainder now. }
    end {while};

    i := maxtwo div 2; { Construct the Vertical power-of-two tree. }
    while (i > 0) do begin
	pnum := pnum + 1;
	storerot( pnum, pnum, i * dthetadegree ); { A vertical node. }
	i := i div 2; { Next power of two. }
    end {while};

    storepoly( 000, 001 ); { This points to start of list of wedge polys. }

    pnum := pnum + 1;
    startwedge := pnum; { This points to the wedge rotations. }

    pnum := pnum + 1; { This will be the top polygon. }
    starttop := pnum;

    pnum := pnum + 1; { This will be the bottom polygon. }
    startbottom := pnum;

    pnum := pnum + 3; { Three pointer polygons go here. }
    startfigure := pnum; { This will be the final (entry) polygon. }

    { Any user transformation is expected to keep entry point last. }
    totalpolys := startfigure + numtransnodes;
    startuser := totalpolys; { Last poly node is start/entry point. }

    { *** Output Section. *** }

    { *** Echo Terminal Input to Output File. *** }

    writeln( outfile, 'C REVMKR input file was ', profilename );
    writeln( outfile, 'C Number of faces was ', numfaces:1 );
    write( outfile, 'C Top polygon was ' );
    if ( dotop ) then writeln( outfile, 'requested.' )
    else writeln( outfile, 'omitted.' );
    write( outfile, 'C Bottom polygon was ' );
    if ( dobottom ) then writeln( outfile, 'requested.' )
    else writeln( outfile, 'omitted.' );
    writeln( outfile, 'C Polygon Right/Left flag was "', saverightleft, '"' );
    writeln( outfile, 'C Number of user transformation nodes was ',
	              numtransnodes:1 );
    for i := 1 to numcomments do begin
	writeln( outfile, commentarray[i]^ );
	freecomment( i ); { Free storage to be tidy. }
    end {for};
    writeln( outfile, '\n' ); { Two line feeds for Read Scene. }

    { *** Important Numbers for ReadScene. *** }

    writeln( outfile, totalverts:6, totalpolys:6, startuser:6 );

    { *** Give the user some Statistics. *** }

    writeln( output, 'Total vertices=', totalverts:1, '   Total polygons=',
	     totalpolys:1, '   Start polygon=', startuser:1 );
    if ( numtransnodes > 0 ) then begin
	writeln( output, 'Number of user transformaton nodes: ',
						numtransnodes:1 );
    end {if};

    { *** Vertex Output Section *** }

    { The 2*numslices wedge vertices. }
    vnum := 1;
    for i := 1 to numslices do begin
	x := slicearray[i].x;
	y := 0.0;
	writeln( outfile, vnum:6, x:16:8, y:16:8, slicearray[i].z:16:8 );
	vnum := vnum + 1;
	y := x * sindtheta;
	x := x * cosdtheta;
	writeln( outfile, vnum:6, x:16:8, y:16:8, slicearray[i].z:16:8 );
	vnum := vnum + 1;
    end {for};

    { The top and bottom polygon vertices, if needed. }
    if ( dotop ) then begin
	x := slicearray[1].x;
	z := slicearray[1].z;
	for i := 1 to numfaces do begin
	    writeln( outfile, vnum:6, x * cos( i * dtheta ):16:8,
	                           x * sin( i * dtheta ):16:8,
	                           z:16:8 );
	    vnum := vnum + 1;
		end {for};
	end {if};
	if ( dobottom ) then begin
		x := slicearray[numslices].x;
		z := slicearray[numslices].z;
		for i := 1 to numfaces do begin
			writeln( outfile, vnum:6, x * cos( i * dtheta ):16:8,
				               x * sin( i * dtheta ):16:8,
				               z:16:8 );
			vnum := vnum + 1;
		end {for};
	end {if};

	{ Internal Consistency Check. }
	if ( vnum-1 <> totalverts ) then begin
		writeln( output, 'REVMKR: Warning: expected ', totalverts:1,
			 ' vertices, but ', vnum-1:1, ' were output.' );
	end {if};

	{ *** Polygon Output Section *** }

	{ Write out numslices-1 wedge polygons. }
	vertcounter := 0;
	for pnum := 1 to numslices-1 do begin
		p1 := vertcounter + 1;
		p2 := vertcounter + 2;
		p3 := vertcounter + 4;
		p4 := vertcounter + 3;

		if ( pnum = numslices-1 ) then nextlink := 0
		else nextlink := pnum + 1;

		write( outfile, pnum:6, ' p   000', nextlink:6, '     4' );
		if rightleft = 'l' then
			writeln( outfile, p1:6, p2:6, p3:6, p4:6 )
		else writeln( outfile, p4:6, p3:6, p2:6, p1:6 );
		vertcounter := vertcounter + 2;
	end {for};

	{ Write out the wedge rotation tree. }
	numpolys := numtransnodes; { Skip over user tranform nodes. }
	for pnum := numslices to startwedge-1 do begin
		numpolys := numpolys + 1;
		write( outfile, pnum:6 );
		if ( polyarray[numpolys].ptype = polygon ) then
			write( outfile, ' p' )
		else write( outfile, ' z' );
		write( outfile, polyarray[numpolys].down:6,
			polyarray[numpolys].next:6 );
		if ( polyarray[numpolys].ptype = polygon ) then begin
			writeln( outfile, '   000' );
		end {if}
		else writeln( outfile, polyarray[numpolys].degrees:16:8 );
	end {for};

	pnum := startwedge;
	writeln( outfile, pnum:6, ' p   000', topwedgerot:6,
		'     0  -- points to start of wedge rotation polygons' );

	{ write out polygon forming the top }
	pnum := starttop;
	if ( dotop ) then begin
		writeln( outfile, pnum:6, ' p   000   000', numfaces:6 );
		j := 2*numslices; { Wedge vertices precede top. }
		if rightleft = 'r' then begin
			for i := 1 to numfaces do begin
				if ( (i mod 10) = 0 ) then begin
					writeln( outfile ); {Split long lines.}
				end {if};
				write( outfile, j+i:6 );
			end {for};
		end {if}
		else begin
			for i := numfaces downto 1 do begin
				if ( (i mod 10) = 0 ) then begin
					writeln( outfile ); {Split long lines. }
				end {if};
				write( outfile, j+i:6 );
			end {for};
		end {else};
		writeln( outfile );
	end {if}
	else begin
		writeln( outfile, pnum:6,
			' p   000   000     0  -- No Top Polygon' );
	end {else};

	{  write out polygon forming the bottom }
	pnum := startbottom;
	if ( dobottom ) then begin
		writeln( outfile, pnum:6, ' p   000   000', numfaces:6 );
		j := 2*numslices; { Wedge vertices precede bottom. }
		if (dotop) then j := j + numfaces; {Top vertices also precede. }
		if rightleft = 'l' then begin
			for i := 1 to numfaces do begin
				if ( (i mod 10) = 0 ) then begin
					writeln( outfile ); {Split long lines. }
				end {if};
				write( outfile, j+i:6 );
			end {for};
		end {if}
		else begin
			for i := numfaces downto 1 do begin
				if ( (i mod 10) = 0 ) then begin
					writeln( outfile ); {Split long lines. }
				end {if};
				write( outfile, j+i:6 );
			end {for};
		end {else};
		writeln( outfile );
	end {if}
	else begin
		writeln( outfile, pnum:6,
			' p   000   000     0  -- No Bottom Polygon' );
	end {else};

	{ *** Linkage Polygon Output Section. *** }

	{ Chain the Top, Wedge, and Bottom together, with entry at last node. }
	pnum := startbottom + 1;
	writeln( outfile, pnum:6, ' p', startbottom:6,
		 '   000     0  -- points to start of bottom polygon' );
	pnum := pnum + 1;
	writeln( outfile, pnum:6, ' p', topwedgerot:6, pnum-1:6,
		 '     0  -- points to start of wedge rotation polygons' );
	pnum := pnum + 1;
	writeln( outfile, pnum:6, ' p', starttop:6, pnum-1:6,
		 '     0  -- points to start of top polygon',
		 ' <== START NODE FOR FIGURE ==' );
	pnum := pnum + 1;

	{ *** Add the user-specified transformation to end of the file. *** }

	if ( numtransnodes > 0 ) then begin
		{ Renumber user transformation and output it at end of file. }
		offset := pnum - 1; { Last generated polygon. }
		for i := 1 to numtransnodes do begin
			down := polyarray[i].down;
			next := polyarray[i].next;
			if ( down > 0 ) then down := down + offset
			else if ( down < 0 ) then down := down + offset + 1;
			if ( next > 0 ) then next := next + offset
			else if ( next < 0 ) then next := next + offset + 1;
			writeln( outfile, pnum:6, polyarray[i].usertype:2,
				down:6, next:6, polyarray[i].ptr^ );
			freeuser( i ); { Free storage to be tidy. }
			pnum := pnum + 1;
		end {for};
	end {if};

	{ Internal Consistency Check. }
	if ( pnum-1 <> totalpolys ) then begin
		writeln( output, 'REVMKR: Warning: expected ', totalpolys:1,
			 ' polygons, but ', pnum-1:1, ' were output.' );
	end {if};


end {procedure main};

begin
	main;
end.

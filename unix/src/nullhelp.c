
/*
 *DESC: A dummy file when the help system is missing.
 */

hopen( fp )
{
}
hclose( fp )
{
}

close_help()
{
}

hfgets( buf, size, desc ){}
RemoveNL( str )
char *str;
{
	register int slen;

	slen = strlen(str) - 1;

	if( slen >= 0 && str[slen] == '\n' )
		str[slen] = 0;
}
help_open( fname ) {}
init_help(){}
help_gets( linebuf, size, fp ){}

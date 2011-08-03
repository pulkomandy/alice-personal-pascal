#define MAXTRY 3
static int drorder[] = { 1, 0, 2 };

FILE *
qfopen( fname, opargs )
register char *fname;
char *opargs;
{
	char fnbuf[MX_WIN_WIDTH];
	FILE *thedrive;
	int curdrive;

	if( fname[0] == '?' && fname[1] == ':' ) {
		int numdrives;
		int trydrive;
		char *apdir, *getenv();
		char *lstslash, *strrchr();

		/* try default drive */

		if( thedrive = fopen( fname + 2, opargs ) )
			return thedrive;
		if( (lstslash = strrchr( fname, '\\' )) ||
				(lstslash = strrchr( fname, '/' ) ) ) {
			if( thedrive = fopen( lstslash+1, opargs ) )
				return thedrive;
			}
		 else
			lstslash = fname + 1;	/* the colon */
		if( apdir = getenv( "ALDIR" ) ) {
			sprintf( fnbuf, "%s\\%s", apdir, lstslash+1 );
			if( thedrive = fopen( fnbuf, opargs ) )
				return thedrive;
			}
		strcpy( fnbuf, fname );

		curdrive = getDisk( );
		numdrives = selDisk( curdrive ) - 1;

		if( numdrives > MAXTRY-1 )
			numdrives = MAXTRY - 1;
		for( trydrive = numdrives; trydrive >= 0; trydrive-- ) {
			int physdrive;
			physdrive = drorder[trydrive];
			if( physdrive != curdrive ) {
				if( physdrive == 1 && !hasbdrive() )
					continue;
				fnbuf[0] = 'A' + physdrive;
				if( thedrive = fopen( fnbuf, opargs ) )
					return thedrive;
				}
			}
		return (FILE *)0;	/* open failed */
		}
	 else {
		if( fname[strlen(fname)-1] == ':' ) {
			char **p;
			static char *devmaparray[] = {
				"con:", "con",
				"aux:", "aux",
				"lst:", "prn",
				"trm:", "con",
				"kbd:", "con",
				"lpt2:", "lpt2",
				"lpt1:", "lpt1",
				"com1:", "com1",
				"nul:", "nul",
				0 };
			for( p = devmaparray; *p; p++ )
				if( case_equal( *p++, fname ) ) {
					fname = *p;
					break;
					}
			}
		return fopen( fname, opargs );
		}
}

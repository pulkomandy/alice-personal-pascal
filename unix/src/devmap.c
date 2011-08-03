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

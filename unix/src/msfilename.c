/*
 *DESC: File handling code for ms-dos
 *DESC:
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "menu.h"
#include "jump.h"
#include "break.h"

#ifdef DB_FILENAME
# define PRINTT
#endif
#include "printt.h"

#ifdef msdos
# include "msdos.h"
# include <dos.h>
# define F_CF 1
# define STATUS		(outreg.x.cflag & F_CF  ? outreg.h.al : 0)
#endif msdos

#ifdef QNX
# include "io.h"
# include <fsys.h>
#endif QNX

#ifdef UNIX
# include <sys/types.h>
# include <sys/dir.h>
# include <sys/stat.h>
#endif UNIX

char	FileExt[]	= FILEEXT;
char	TextExt[]	= TEXTEXT;
char	*NoExt		= 0;

setfname(name, force, optExt, filesInMenu)
char	*name;
int	force;
char	*optExt;		/* optional extension */
int	filesInMenu;		/* display files in menu if necessary */
{
	char	buf[PATHLEN];
	int ret;

	if (ret = filename(buf, name, force, optExt, filesInMenu))
		newname(buf);
}
#include <errno.h>
#include <spawn.h>

char *
dosexecerr(ret)
int ret;
{
	extern int errno;
	if( ret != -1 )
		return "program indicated error on termination";
	switch( errno ) {
		case ENOENT:
			return "program not found";
		case ENOMEM:
			return "not enough memory to run program";
		default:
			return "unknown";
		}
}
/*
 * Try to convert a text file to an AP file
 */
#ifdef FULL
# define LRGARG "+L"
#else
# define LRGARG "-L"
#endif

textToAP(name, extp)
char	*name;
char	*extp;			/* substring of name where extension is */
{
	char	name_ap[100];
	int ret;

	strcpy( name_ap, name );
	strcpy( name_ap  + (extp-name) + 1, FileExt );
	regtty( TRUE, FALSE );
	ret = spawnlp(P_WAIT,APIN,APIN, LRGARG, "+s", name, name_ap, (char *)0 );
	regtty( FALSE, ret != 0 );
	if( ret ) {
#ifndef SAI
		show_whole_doda();
#endif
		error(ER(65,"conv`Failure converting \"%s\" into \"%s\", cause: %s"),
				name, name_ap, dosexecerr(ret));
		}
	 else
		strcpy( name, name_ap );
}


/*
 * Get a file name for loading and saving.  If one was supplied, use it
 * as the new name.  Otherwise if we don't already have one, or force is
 * TRUE, prompt for one.
 *
 * Returns TRUE if the name changed from work_fname.
 */
filename(buf, name, force, optExt, filesInMenu)
char	*buf;
char	*name;
int	force;
char	*optExt;		/* optional extension */
int	filesInMenu;		/* display files in menu if necessary */
{
	if (name && name[0])
		strcpy(buf, name);
#ifndef SAI
	else if (!work_fname(curr_workspace) || force)
		getfilename(buf, filesInMenu);
#endif SAI
	else {
		strcpy(buf, work_fname(curr_workspace));
		return FALSE;
	}

	nojunk(buf);
	addOptExt(buf, optExt);
	return TRUE;
}

#ifndef SAI

/*
 * Get a file name from a menu.  Store it in name, and return name.
 */
getfilename(name, filesInMenu)
char	*name;
int	filesInMenu;		/* display files in menu if necessary */
{
	struct menu_data m;
	register char	*p;
	register int	choice;
#ifdef msdos
	int	len;
#endif msdos

	for (name[0] = NULL; !strlen(name); ) {
		buildFileMenu(&m, filesInMenu);
		choice = pop_menu(curr_window->w_desc, &m, (funcptr)NULL);
		if (choice == m.num_items - 1) {	/* "Change directory */
			char	buf[MX_WIN_WIDTH];
			getprline("Directory? ", buf, MX_WIN_WIDTH);
			changeDir(buf);
			buf[0] = NULL;
		} else switch (choice) {
		case 0:		/* Do nothing */
			freeFileMenu(&m);
			err_return();

		case 1:		/* Enter a file name */
			getprline("File Name? ", name, MX_WIN_WIDTH);
			break;

		default:
			p = m.item[choice];
#ifdef msdos
			len = strlen(p);
			if (p[1] == ':' && len == 2)	/* "A:", etc */
				selDisk(p[0] - 'A');
			else if (!strcmp(&p[len-6], " <dir>")) {/* "d <dir>" */
				p[len-6] = '\0';
				changeDir(p);
				p[len-6] = ' ';		/* could be static */
			} else
				strcpy(name, p);
#endif msdos
#ifdef QNX
			if (p[0] == '+')			/* "+dir" */
				changeDir(p+1);
			else
				strcpy(name, p);
#endif QNX
#ifdef UNIX
			if (p[strlen(p)-1] == '/')		/* "dir/" */
				changeDir(p);
			else
				strcpy(name, p);
#endif UNIX
			break;
		}
		freeFileMenu(&m);
	}
}

#ifdef msdos
static char dotdotDir[] = ".. <dir>";
#endif

buildFileMenu(mp, filesInMenu)
struct menu_data	*mp;
int	filesInMenu;
{
	extern	char	menu_nop[];
	char	*p;

#ifdef msdos
	static DTA	dta;
	static char	title[80]	= "Files in ";
	extern char	*strrchr();
	char	path[80];
	char	buf[80];
	int	disk;

	strcpy( path, "X:\\" );
	disk = getDisk();
	path[0] = disk + 'A';
	getCDir(&path[3], 0);

	strcpy(&title[9], path);
	new_menu(mp, title);
	add_menu_item(menu_nop);
	add_menu_item("Enter a file name");

	if (path[3] == '\0') {			/* at root */
		int	nDisks;
		int i;

		strcpy(buf, "X:");
		nDisks = selDisk(disk);		/* selDisk rets # of drives */
		for (i = 0; i < nDisks; i++) {
			if (i != disk && (i != 1 || hasbdrive() ) ) {
				buf[0] = 'A' + i;
				add_menu_item(allocstring(buf));
			}
		}
		strcpy(&path[3], "*.*"); 
	} else {
		add_menu_item(dotdotDir);

		strcat(path, "\\*.*");
	}
	printt1( "Prepareing to read directory %s\n", title );

	setDTA(&dta);

	if (findFirst(path, ATTR_DIRECTORY) == 0) {
		do {
			strcpy(buf, dta.filename);
			printt1( "Processing file %s\n", buf );
			if (dta.attr == ATTR_DIRECTORY &&
			    strcmp(".", buf) != 0 &&
			    strcmp("..", buf) != 0) {
				strcat(buf, " <dir>");
				add_menu_item(allocstring(buf));
			} else if (filesInMenu) {
				p = strrchr(buf, '.');
				if (p && (case_equal(FileExt, p+1)))
					add_menu_item(allocstring(buf));
			}
	 	} while (!findNext() );
	}
	printt0( "Done, ready to pop\n" );
#endif msdos

#if defined(QNX) || defined(UNIX)
	static	char	null_string[]	= { 0 };
	static	char	baddir[]	= "baddir`Bad directory";
	int	charsRead;
#endif

#ifdef QNX
	struct dir_entry DE;
	register FILE	*dp;
	char	buf[NCPFN+1];
	static char	title[80]	= "Files in ";

	getcwd(&title[9], 0);
	new_menu(mp, title);
	add_menu_item(menu_nop);
	add_menu_item("Enter a file name");
	add_menu_item("+^");
	/*
	 * On QNX systems, a directory consists of an extent record,
	 * followed by some number of directory entries.  Each
	 * directory entry has various fields, including DE.fstat,
	 * DE.fattr, and DE.fname.  By experimentation, if DE.fstat & 0x40
	 * then the file exists, and is a directory if DE.fattr & _DIRECTORY.
	 * The fname field can be as long as 16 chars and has a 17th byte
	 * to NULL terminate it.  The current directory is obtained by
	 * opening the file "".
	 */
	if ((dp = fopen(null_string, "r")) == NULL) {
		warning(ER(206,baddir));
		return;
	}
	if (fseek(dp, (long)sizeof(struct dir_xtnt), 0) < 0) {
		warning(ER(206,baddir));
		goto done;
	}
	while ((charsRead = fget(&DE, sizeof(DE), dp)) > 0) {
		if (charsRead != sizeof(DE)) {
			warning(ER(206,baddir));
			goto done;
		}
		if (!DE.fstat)		/* empty slot in directory */
			continue;
		if ( DE.fattr & _DIRECTORY) {
			strcpy(buf, "+");
			strcat(buf, DE.fname);
			add_menu_item(allocstring(buf));
		} else if (filesInMenu) {
			p = strrchr(DE.fname, '.');
			if (p && (case_equal(FileExt, p+1)))
				add_menu_item(allocstring(DE.fname));
		}
	}
done:
	fclose(dp);
#endif QNX

#ifdef UNIX
	/* THIS SHOULD BE CHANGED TO WORK WITH THE opendir() ROUTINES */
	/* On UNIX systems, a directory consists of a number of struct
	 * direct entries, each of which contains an inode and a
	 * 14 char (not necessarily NULL terminated) filename.  If
	 * the inode is not zero then the entry is a real file.
	 * Then the file is "stat"ed which gives the mode, in which
	 * is encoded whether the file is a regular file or directory
	 * (or other things).  The current directory is obtained by
	 * opening the file ".".
	 *
	 * The above isn't necessarily true, and this code should be replaced
	 * by the readdir(), etc. library, so it will work under 4.2.
	 */
	struct direct	dir;
	struct stat	statbuf;
	int	d;
	char	namebuf[BUFSIZ+2];
	new_menu(mp, "Files");
	add_menu_item(menu_nop);
	add_menu_item("Enter a file name");
	add_menu_item("../");

	if ((d = open(".", 0)) < 0) {
		warning(ER(206,baddir));
		return;
	}
	while ((charsRead = read(d, &dir, sizeof(dir))) > 0) {
		if (charsRead != sizeof(dir)) {
			warning(ER(206,baddir));
			goto done;
		}
		if (!dir.d_ino)		/* empty slot in directory */
			continue;
		strncpy(namebuf, dir.d_name, DIRSIZ);
		namebuf[DIRSIZ] = NULL;
		stat(namebuf, &statbuf);
		if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			strcat(namebuf, "/");
			add_menu_item(allocstring(namebuf));
		} else if ((statbuf.st_mode & S_IFMT) == S_IFREG &&
			   filesInMenu) {
			char *strrchr();
			p = strrchr(namebuf, '.');
			if (p && (case_equal(FileExt, p+1)))
				add_menu_item(allocstring(namebuf));
		}
	}
done:
	close(d);
#endif UNIX
	add_menu_item("Change directory");
}

#endif SAI

#ifdef msdos

setDTA(dta)
DTA	*dta;
{
	union REGS inreg, outreg;
	struct SREGS segreg;

	inreg.h.ah = 0x1a;
	inreg.x.dx = FP_OFF(dta);
	segread(&segreg);
#ifdef LARGE
	segreg.ds = FP_SEG(dta);
#endif
	intdosx(&inreg, &outreg, &segreg);
}

int
findFirst(pat, attr)
char *pat;
int attr; 
{
	union REGS inreg, outreg;
	unsigned statret;
	struct SREGS segreg;

	inreg.h.ah = 0x4e; /* FIND FIRST */;
	inreg.x.dx = FP_OFF(pat);
	segread( &segreg );
#ifdef LARGE
	segreg.ds = FP_SEG(pat);
#endif
	inreg.x.cx = attr;
	statret = intdosx( &inreg, &outreg, &segreg );
	return STATUS;
}
#ifdef FULL
static DTA ourdta;
int dir_current = FALSE;

dofirst( pat, attr )
char *pat;
int attr;
{
	setDTA( &ourdta );
	dir_current = !findFirst( pat, attr );
}

char *
curfilename( att )
int *att;
{
	*att = ourdta.attr;
	return &ourdta.filename;
}
#endif

int
findNext()
{
	union REGS inreg, outreg;
	unsigned statret;

	inreg.h.ah = 0x4f;
	statret = intdos( &inreg, &outreg);
	return STATUS;
}

int
getDisk()
{
	union REGS inreg, outreg;

	inreg.h.ah = 0x19; /*GETDISK; */
	intdos( &inreg, &outreg );
	return outreg.h.al;
}

int
getCDir(s, drive)
char	*s;
int	drive;
{
	union REGS inreg, outreg;
	unsigned statret;
	struct SREGS segreg;

	inreg.h.ah = 0x47; /*GETCDIR; */
	inreg.x.dx = drive;
	inreg.x.si = FP_OFF(s);

	segread( &segreg );
#ifdef LARGE
	segreg.ds = FP_SEG(s);
#endif
	statret = intdosx( &inreg, &outreg, &segreg );
	return STATUS;
}

int
selDisk(drive)
int	drive;
{
	union REGS inreg, outreg;

	inreg.h.ah = 0x0e; /*SELDSK;*/
	inreg.x.dx = drive;
	intdos( &inreg, &outreg );
	return outreg.h.al;		/* al holds # of drives */
}

#endif msdos

#ifndef SAI

freeFileMenu(xmp)
struct menu_data	*xmp;
{
	register int i;
	register struct menu_data *mp = xmp;

#ifdef msdos
	/*
	 * Item '2' might not be ".. <dir>" since the command might
	 * have been run at the root of the file system...If it isn't
	 * ".. <dir>" free it.
	 */
	if (mp->num_items > 3 && strcmp(mp->item[2], dotdotDir) != 0)
		dmfree(mp->item[2]);
#endif
	for (i = 3; i < mp->num_items - 1; i++)
		dmfree(mp->item[i]);
}

#endif SAI

changeDir(dir)
char	*dir;
{
#ifdef msdos
	union REGS inreg, outreg;
	unsigned statret;
	struct SREGS segreg;

	inreg.h.ah = 0x3b; /* CHDIR */
	inreg.x.dx = FP_OFF(dir);
	segread( &segreg );
#ifdef LARGE
	segreg.ds = FP_SEG(dir);
#endif
	statret = intdosx( &inreg, &outreg, &segreg );
	if (STATUS == EPATHNOTFOUND)
#endif msdos
#ifdef QNX
	if (cd(dir))
#endif QNX
#ifdef UNIX
	if (chdir(dir) < 0)
#endif UNIX
		warning(ER(207,"baddir`can't cd to %s"), dir);
}

addOptExt(name, optExt)
char	*name;
char	*optExt;
{
	if (name && optExt && !strchr(name, '.')) {
		strcat(name, ".");
		strcat(name, optExt);
	}
}

/*
 * Destructively remove leading and trailing junk from str.
 */
nojunk(str)
char	*str;
{
	register char	*from;
	register char	*to;
	char		buf[MX_WIN_WIDTH];
	static char	junk[]	= " \t\n";

	strcpy(buf, str);
	from = buf;
	to = str;
	while (*from && strchr(junk, *from))
		from++;
	while (*from && !strchr(junk, *from))
		*to++ = *from++;
	*to = '\0';
}

/*
 * Change work_fname, freeing old name.
 */
 /* static file name */
bool emergency_fname = FALSE;

newname(name)
register char	*name;
{
	extern int redraw_status;
	register char *nambuf;
	extern char *mmalloc();

	if (work_fname(curr_workspace) && !emergency_fname )
		dmfree(work_fname(curr_workspace));

	/* we don't call allocstring because we do not want to fail here if
	   out of memory */
	if (name && name[0]) {
		if( nambuf = mmalloc( strlen(name)+1 ) ) 
			strcpy( nambuf, name );
		 else {
			nambuf = name;
			emergency_fname = TRUE;
			}
		work_fname(curr_workspace) = nambuf;
		}
	else
		work_fname(curr_workspace) = NULL;

#ifndef SAI
	redraw_status = TRUE;
#endif
}

/* special open that tries all the disks */

#ifdef msdos

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
#else

FILE *
qfopen( fname, opargs )
char *fname;
char *opargs;
{
	return fopen( fname, opargs );
}
#endif

/* returns true if there is a real B drive in the system */

hasbdrive()
{
	struct REGS theregs;

	int86( 0x11, &theregs, &theregs );	/* equipment interrupt */

	return (theregs.x.ax & 1) && (theregs.x.ax & 0xc0);
}

long
time(tlong)
long *tlong;
{
	struct REGS theregs;
	long tfunc();

	theregs.h.ah = 0x2C;

	intdos( &theregs, &theregs );
	return *tlong = tfunc( &theregs ) / 100L;
}

		
char *doserrs[] = {
"Write Protect",
0,
"Drive not ready",
0,
"Bad Data",
0,
0,
"Funny Disk",
0,
"Out of Paper",
"Write fault",
"Read Fault",
0
};
/* critical dos error */
static char ioerr[] = "I/O Error ";

static char *resmenu[] = {
"Ignore",
"Retry",
"Abort ALICE",
"Fail"
};

criterr( ax, di )
unsigned int ax, di;
{
	char errstr[MX_WIN_WIDTH];
	char *ertype;
	struct menu_data m;
	int i;
	int ret;

	ertype = doserrs[ di & 0xff ];
	sprintf( errstr, "I/O Error (%s)", ertype ? ertype : "General" );
	if( !(ax & 0x8000) ) 
		sprintf( errstr+strlen(errstr)," on disk %c:",(ax & 0xff)+'A');

	if( (di & 0xff) == 0 && (ax & 0xff) < 2 )
		warning( ER(287,
			"DANGER: Do NOT change floppy disks at this time") );
	
	new_menu( &m, errstr );
	for( i = 0; i < 4; i++ )
		add_menu_item( resmenu[i] );
	ret = pop_menu( stdscr, &m, (funcptr)0 );

	if( ret == 2 ) { /* abort */
		/* does this nasty interupt mean trouble? */
		/* should we restore the vector by force instead? */
		errestore();
		endwin();
		}
	return ret;

}

static unsigned intseg = 0, intoff = 0;

ersetup()
{
#ifndef SAI
	union REGS inreg, outreg;
	struct SREGS segreg;
	extern int critvector();
	extern int breaknop();
	funcptr ourfunc;

	/* save away old vector to restore it */
	inreg.h.ah = 0x35;	/* read vector */
	inreg.h.al = 0x24;	/* critical error vector */
	intdos( &inreg, &outreg );

	intoff = outreg.x.bx;
	segread( &segreg );
	intseg = segreg.es;

	/* now set it */

	ourfunc = critvector;
	cintset( 0x24, FP_OFF(ourfunc), FP_SEG(ourfunc) );
	/* now break vector */

	ourfunc = breaknop;
	cintset( 0x23, FP_OFF(ourfunc), FP_SEG(ourfunc) );

	/* all set up */
#endif SAI	
}

errestore()
{
	if( intseg )
		cintset( 0x24, intoff, intseg );
}

cintset( intnum, off, seg )
unsigned intnum;
unsigned off, seg;
{
	union REGS inreg, outreg;
	struct SREGS segreg;

	inreg.h.ah = 0x25;
	inreg.x.dx = off;
	segread( &segreg );
	segreg.ds = seg;
	inreg.h.al = intnum;
	intdosx( &inreg, &outreg, &segreg );
}
/* time delay */
/* hundredths of a second */
#define DAY (24*60*60*100L)
long
tfunc( rstruct )
union REGS *rstruct;
{
	return rstruct->h.dl + 100 * rstruct->h.dh + 6000L * rstruct->h.cl
			+ 360000L * rstruct->h.ch;
}

tdelay( hundredths )
unsigned int hundredths;
{
	union REGS inreg, outreg;
	long start;

	inreg.h.ah = 0x2c;	/* get time function */

	intdos( &inreg, &outreg );
	start = tfunc(&outreg);

	while( ((tfunc(&outreg) - start) % DAY) < hundredths && !got_break() )
		intdos( &inreg, &outreg );
}

/* Constants for 8253 */
#define CLK_2_8253      1193180L        /* Input clock to counter 2 */
#define CMD_8253        0x43            /* 8253 Commend reg. */
#define DATA_2_8253     0x42            /* 8253 Channel 2 Data reg. */
#define PROG_2          0xb6            /* Set 8253 Program Count */
#define MODE_2          0x86            /* Set 8253 Mode, Square wave */

/* Constants for 8253 */
#define B_8255          0x61            /* 8255 B Channel */

/* Bit values of 8255 B Port */
#define MASK_B          0xfc            /* Bits we don't touch */
#define GATE_2          1               /* Channel 2 Input Gate */
#define SPEAKER         2               /* Speaker enable */

/*
 * If GATE_2 is 0 then software control of the speaker is controled
 * by SPEAKER. This is because OUT 2 is high always when GATE_2 is High.
 * See 2-22 and D-9 of IBM-PC tech manual.
 */


/* is the sound currently on? */
static int soundstat = FALSE;
/* do your beeping */

soundon(hz)       /* Set the frequency, 0 == ERROR, 1 == OK */
int hz;
{
#ifndef SIEMENS
	/* You might want to convert these to longs rather than floats */

	long temp;
	unsigned int count;
	
	if( hz <= 19 )
		return tone(FALSE);
	
	temp = ((long) CLK_2_8253) / hz;

	if(temp < 0 || temp > 65535L )
		return(0);
	count = (unsigned int)temp;
#ifdef NASM_SOUND
	outp(CMD_8253,PROG_2);       /* Set free running counter mode */
	outp(DATA_2_8253,count & 0xff);
	outp(DATA_2_8253,count >> 8 );
#else
	soundout( count );
#endif
	/* outp(CMD_8253,MODE_2);*/
	if( !soundstat )
		tone(TRUE);
#endif
}


tone(flag)      /* Enable the tone */
int flag;
{
	unsigned int temp = (inp(B_8255) & MASK_B) ;
	outp(B_8255,temp | ((flag) ? GATE_2 | SPEAKER : 0));
	soundstat = flag;
}



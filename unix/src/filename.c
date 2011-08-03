#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "menu.h"
#include "jump.h"
#include <stdlib.h>

/*
 *DESC: Handle file interfaces with the operating system -- directories,
 *DESC: menus of files, file searching etc.
 */


#ifdef DB_FILENAME
# define PRINTT
#endif
#include "printt.h"

#ifdef msdos
# include "msdos.h"
# include <dos.h>
# define STATUS		((outreg.r_flags & F_CF) ? (outreg.r_ax & 0xff) : 0)
# define strrchr rindex
#endif msdos

#ifdef QNX
# include "io.h"
# include <fsys.h>
#endif QNX

#ifdef unix
# include <sys/types.h>
//# include <sys/dir.h>
# include <sys/stat.h>
#include <dirent.h>
#endif unix

char	FileExt[]	= FILEEXT;
char	TextExt[]	= TEXTEXT;
char	*NoExt		= 0;

static buildFileMenu(), addOptExt();

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
	return ret;
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

	for (name[0] = 0; !strlen(name); ) {
		buildFileMenu(&m, filesInMenu);
		choice = pop_menu(curr_window->w_desc, &m, (funcptr)NULL);
		if (choice == m.num_items - 1) {	/* "Change directory */
			char	buf[MX_WIN_WIDTH];
			getprline("Directory? ", buf, MX_WIN_WIDTH);
			space_strip( buf );
			changeDir(buf);
			buf[0] = 0;
		} else switch (choice) {
		case 0:		/* Do nothing */
			freeFileMenu(&m);
			err_return();

		case 1:		/* Enter a file name */
			getprline("File Name? ", name, MX_WIN_WIDTH);
			space_strip( name );
			break;

		default:
			p = m.item[choice];
			if (p[strlen(p)-1] == '/')		/* "dir/" */
				changeDir(p);
			else
				strcpy(name, p);
			break;
		}
		freeFileMenu(&m);
	}
}


#ifdef msdos
static char dotdotDir[] = ".. <dir>";
#endif

static
buildFileMenu(mp, filesInMenu)
struct menu_data	*mp;
int	filesInMenu;
{
	extern	char	menu_nop[];
	char	*p;
#ifndef OLMSG
	static	char	baddir[]	= "baddir`Bad directory";
#endif

	/* modified to use readdir
	 */
	struct stat	statbuf;
	char	namebuf[NAME_MAX+2];
	DIR *d;
	struct dirent *dent;

	new_menu(mp, "Files");
	add_menu_item(menu_nop);
	add_menu_item("Enter a file name");

	if (!(d = opendir(".")))  {
		warning(ER(206,baddir));
		return;
	}
	while( dent = readdir( d ) ) {
		strncpy(namebuf, dent->d_name, NAME_MAX+1 );
		if( case_equal( namebuf, "." ) )
			continue;
		stat(namebuf, &statbuf);
		if ( S_ISDIR( statbuf.st_mode ) ) {
			strcat(namebuf, "/");
			add_menu_item(allocstring(namebuf));
		} else if (S_ISREG(statbuf.st_mode) &&
			   filesInMenu) {
			char *strrchr();
			p = strrchr(namebuf, '.');
			if (p && (case_equal(FileExt, p+1)))
				add_menu_item(allocstring(namebuf));
		}
	}

	closedir(d);

	add_menu_item("Change directory");
}

#endif SAI
/*
 * Try to convert a text file to an AP file
 */
textToAP(name, extp)
char	*name;
char	*extp;			/* substring of name where extension is */
{
	char	name_ap[100];
	char	sysbuf[200];

	strcpy( name_ap, name );
	strcpy( name_ap  + (extp-name) + 1, FileExt );
	sprintf( sysbuf, "%s +s +L %s %s", APIN, name, name_ap );
	if (!safeshell( sysbuf, FALSE, FALSE )) 	/* no pause */
		/* Copy the AP version of the filename back to the original */
		strcpy( name, name_ap );
	else {
#ifndef SAI
		show_whole_doda();
#endif SAI
		error(ER(65,"conv`Failure converting \"%s\" into \"%s\""), name, name_ap);
		}
}

#ifdef msdos

struct FindFiles *
getDTA()
{
	struct _reg inreg, outreg;
	struct FindFiles *ptr;

	inreg.r_ax = GETDTA;
	intcall( &inreg, &outreg, DOSINT );
	regtop( outreg.r_bx, outreg.r_es, ptr );

	return ptr;
}

int
findFirst(pat, attr)
char *pat;
int attr; 
{
	struct _reg inreg, outreg;

	inreg.r_ax = NFFIRST;
	ptoreg( dsreg, inreg.r_dx, inreg.r_ds, pat );
	inreg.r_cx = attr;
	intcall( &inreg, &outreg, DOSINT );
	return STATUS;
}

int
findNext()
{
	struct _reg inreg, outreg;

	inreg.r_ax = NFNEXT;
	intcall( &inreg, &outreg, DOSINT );
	return STATUS;
}

int
getDisk()
{
	struct _reg inreg, outreg;

	inreg.r_ax = GETDISK;
	intcall( &inreg, &outreg, DOSINT );
	return outreg.r_ax & 0xff;
}

int
getCDir(s, drive)
char	*s;
int	drive;
{
	struct _reg inreg, outreg;

	inreg.r_ax = GETCDIR;
	inreg.r_dx = drive;
	ptoreg( dsreg, inreg.r_si, inreg.r_ds, s );
	intcall( &inreg, &outreg, DOSINT );
	return STATUS;
}

int
selDisk(drive)
int	drive;
{
	struct _reg inreg, outreg;

	inreg.r_ax = SELDSK;
	inreg.r_dx = drive;
	intcall( &inreg, &outreg, DOSINT );
	return outreg.r_ax & 0xff;		/* al holds # of drives */
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
	for (i = 2; i < mp->num_items - 1; i++)
		dmfree(mp->item[i]);
}

changeDir(dir)
char	*dir;
{
	if (chdir(dir) < 0)
		warning(ER(207,"baddir`can't cd to %s"), dir);
}
#endif SAI

static addOptExt(name, optExt)
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

bool emergency_name = FALSE;

newname(name)
char	*name;
{
	extern int redraw_status;
	char *nambuf;
	extern char *mmalloc();

	if (work_fname(curr_workspace) && !emergency_name )
		dmfree(work_fname(curr_workspace));

	/* we don't call allocstring because we do not want to fail here if
	   out of memory */
	if (name && name[0]) {
		if( nambuf = mmalloc( strlen(name)+1 ) ) 
			strcpy( nambuf, name );
		 else {
			nambuf = name;
			emergency_name = TRUE;
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

FILE *
qfopen( fname, opargs )
char *fname;
char *opargs;
{
	char fnbuf[MX_WIN_WIDTH];
	FILE *thedrive;
	int curdrive;
	static int drorder[] = { 1, 0, 2 };

	strcpy( fnbuf, fname );
	if( fnbuf[0] == '?' && fnbuf[1] == ':' ) {
		int numdrives;
		int trydrive;

		/* try default drive */

		if( thedrive = fopen( fname + 2, opargs ) )
			return thedrive;
		curdrive = getDisk( );
		numdrives = selDisk( curdrive ) - 1;

		if( numdrives > MAXTRY-1 )
			numdrives = MAXTRY - 1;
		for( trydrive = numdrives; trydrive >= 0; trydrive-- )
			if( trydrive != curdrive ) {
				fnbuf[0] = 'A' + drorder[trydrive];
				if( thedrive = fopen( fnbuf, opargs ) )
					return thedrive;
				}
		return (FILE *)0;	/* open failed */
		}
	 else
		return fopen( fname, opargs );
}
#else

/* try a lot of places to find a file */

FILE *
qfopen( fname, opargs )
char *fname;
char *opargs;
{
	char fnbuf[512];
	FILE *ret;
	int len;
	char *homedir;

	if( ret = fopen( fname, opargs ) )
		return ret;

	len = strlen( fname );
	if( (homedir = getenv( "ALICEDIR" )) && len+strlen(homedir) < sizeof(fnbuf)-2 ) {
		sprintf( fnbuf, "%s/%s", homedir, fname );
		if( ret = fopen( fnbuf, opargs ) )
			return ret;
		}
	homedir = getenv( "HOME" );
	if( homedir && len+strlen(homedir) < sizeof(fnbuf)-10 ) {
		sprintf( fnbuf, "%s/.alice/%s", homedir, fname );
		if( ret = fopen( fnbuf, opargs ) )
			return ret;
		}
	if( len+strlen(ALICE_DIR) < sizeof(fnbuf)-2 ) {
		sprintf( fnbuf, "%s/%s", ALICE_DIR, fname );
		if( ret = fopen( fnbuf, opargs ) )
			return ret;
		}
	return (FILE *)0;
}
#endif

		

#include <whoami.h>
#include "curses.h"
/*
 *
 *  Looking Glass Software Limited
 *  "Curses" emulating windowing package for the IBM-PC
 *
 *  The following code is a trade secret of Looking Glass Software Limited
 *  and should be protected and considered proprietary, subject to the
 *  terms of the licence under which this code was provided
 *
 */


/* some externals that are used */

int C_echoon = TRUE;
int C_crmode = TRUE;


/*
 * this is pretty boring, but it's more extensive on other machines
 */
init_tty()
{

	C_echoon = C_crmode = TRUE;
		
}

end_tty()
{

}

crmode()
{
	C_crmode = TRUE;
}
nocrmode()
{
	C_crmode = FALSE;
}

echo()
{
	C_echoon = TRUE;
}

noecho()
{
	C_echoon = FALSE;
}

raw()
{
	crmode();
}

noraw()
{
	nocrmode();
}

/* nl has no meaning on this machine */
nl()
{
}

nonl()
{
}

savetty(){}
resetty(){}
gettmode(){}

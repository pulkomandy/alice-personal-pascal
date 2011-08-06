#include	<stdlib.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	<allegro.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"

#define	FSEL	0

#define	ALERT 1

#define	INPUT	2

int	_zehandle; /*temporary? fudge*/
extern	int	handle;
volatile	int	_timer=0;

OBJECT	*_alertbox, *_fselbox, *_inputbox;
extern	GFONT	*	_font, *_sysfonts[8];
extern	unsigned	short	_mice[][36], _font0[], _font1[], _font2[];
extern	_MEVENT	_mevent;
extern	_MOUSE	_mouse;
extern	BITMAP	*_mousebmp;
extern	short	_no_of_fonts;
extern	unsigned	short	_guirsh[];
extern	void	_setmouse();

void _timeit() {
	_timer++;
}
END_OF_FUNCTION(_timeit);

int appl_init(int res) {
	allegro_init();
	install_keyboard(); 
	install_mouse();
	install_timer();
	LOCK_VARIABLE(_timer);
	LOCK_FUNCTION(_timeit);
	install_int(_timeit, 1);
	Setscreen(-1, res);
	_mevent.kinds=M_LEFT_UP|M_RIGHT_UP|M_MIDDLE_UP;
	_mousebmp=create_bitmap(16, 16); _mouse.shape=_mousebmp->dat;
	_setmouse(_mice[0]);
	srand(time(0));
	rsrc_include(_guirsh);
	rsrc_gaddr(0, FSEL, &_fselbox);
	rsrc_gaddr(0, ALERT, &_alertbox);
	rsrc_gaddr(0, INPUT, &_inputbox);
	vst_user_font(handle, (GFONT *)_font0);
	vst_user_font(handle, (GFONT *)_font1);
	vst_user_font(handle, (GFONT *)_font2);
	vst_font(handle, 0);
	return(1);
}

int graf_handle(short *gr_hwchar, short *gr_hhchar, short *gr_hwbox, short *gr_hhbox) {
	*gr_hwchar=_font->max_char_width; *gr_hwbox=_font->max_cell_width;
	*gr_hhchar=*gr_hhbox=_font->form_height;
	return(_zehandle);
}

int appl_exit(void) {
	destroy_bitmap(_mousebmp);
	allegro_exit();
	exit(0);
}

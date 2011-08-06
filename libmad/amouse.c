#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<memory.h>
#include	<allegro.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"


_MOUSE	_mouse;
_MEVENT	_mevent;

short _mshape, _mtot, _mousedrawn, _mousex, _mousey, _mousew, _mouseh;
BITMAP	*_mousebmp;
extern	int	handle;
extern	volatile	int	_timer;
extern	short	_clipx1, _clipx2, _clipy1, _clipy2;
extern	unsigned	char _defpal[];

unsigned	short	_mice[8][36]	=	{
	/* ARROW */
	1,	1,	0,	 1,
	49152,	57344,	61440,	63488, 64512,	65024,	65280,	65408,
	65472,	65504,	65024,	61184, 52992,	34688,	1920,	896,
	0,	16384,	24576,	28672, 30720,	31744,	32256,	32512,
	32640,	31744,	27648,	17920, 1536,	768,	768,	0,
	/* TEXT */
	7,	7,	0,	1,
	8772,	6552,	1056,	576,	576,	576,	576,	576,
	576,	576,	576,	576,	576,	1056,	6552,	8772,
	7224,	1632,	960,	384,	384,	384,	384,	384,
	384,	384,	384,	384,	384,	960,	1632,	7224,
	/* BEE */
	8,	8,	0,	1,
	5246,	5315,	7325,	59709,	14715,	58997,	9386,	14620,
	25254,	52226,	40607,	48417,	47911,	46505,	51947,	31806,
	2048,	2108,	98,	1730,	50820,	6538,	6996,	1760,
	7512,	13308,	24928,	17118,	17624,	19030,	13332,	0,
	/* FINGER */
	0,	0,	0,	1,
	12288,	31744,	32256,	8064,	4032,	16376,	16380,	32764,
	65534,	65534,	32767,	16383,	8191,	4095,	1023,	255,
	12288,	19456,	25088,	6528,	3136,	13048,	10500,	26148,
	37826,	53058,	31811,	8225,	4097,	3137,	896,	192,
	/* OPENHAND */
	8,	8,	0,	1,
	768,	8112,	16376,	16380,	32766,	65534,	65534,	32767,
	32767,	65535,	65535,	32767,	16383,	4095,	511,	63,
	768,	7344,	9288,	8740,	28946,	39042,	33794,	16897,
	28673,	38913,	33793,	16384,	12288,	3584,	448,	48,
	/* THINX */
	7,	7,	0,	1,
	0,	640,	640,	640,	640,	640,	65278,	0,
	65278,	640,	640,	640,	640,	640,	0,	0,
	0,	256,	256,	256,	256,	256,	256,	65534,
	256,	256,	256,	256,	256,	256,	0,	0,
	/* THICKX */
	7,	7,	0,	1,
	0,	0,	1088,	1088,	1088,	1088,	64638,	0,
	0,	64638,	1088,	1088,	1088,	1088,	0,	0,
	0,	0,	896,	896,	896,	896,	896,	65534,
	65534,	896,	896,	896,	896,	896,	0,	0,
	/* OUTLINEX */
	7,	7,	0,	1,
	1984,	1984,	1728,	1728,	1728,	65278,	65278,	49158,
	65278,	65278,	1728,	1728,	1728,	1984,	1984,	0,
	0,	896,	640,	640,	640,	640,	32508,	16388,
	32508,	640,	640,	640,	640,	896,	0,	0,
};

void _hidemouse(void) {
	if(!_mousedrawn) return;
	show_mouse(NULL);
	_mousedrawn=0;
}

void _showmouse(void) {
	if(!_mousedrawn&&handle<4) {
		show_mouse(screen); _mousedrawn=1;
	}
}

void v_show_c(int handle, int count) {
	if(_mtot) {
		if(count) _mtot--;
		else _mtot=0;
		if(!_mtot) _showmouse();
	}
}

void v_hide_c(int handle) {
	if(!_mtot) _hidemouse();
	_mtot++;
}

void _setmouse(unsigned short *shape) {
short i, j, k, fg, bg;
unsigned short *index=&shape[4];
	_hidemouse();
	_mouse.hotx=*shape; _mouse.hoty=shape[1];
	fg=shape[2]; if(!fg) {
      fg=Setcolor(256, _defpal); Setpalette();
   }
	bg=shape[3];
	memset(_mouse.shape, 0, 256);
	i=0; while(i<16) {
		j=0; k=15; while(j<16) {
		 	if((1<<k)&*index) _mouse.shape[i*16+j]=fg;
				j++; k--;
		 }
		 i++; index++;
	}
	i=0; while(i<16) {
		j=0; k=15; while(j<16) {
		 	if((1<<k)&*index) _mouse.shape[i*16+j]=bg;
				j++; k--;
		 }
		 i++; index++;
	}
	set_mouse_sprite(_mousebmp);
	set_mouse_sprite_focus(_mouse.hotx, _mouse.hoty);

	if(_mousedrawn&&handle<4) { _mousedrawn=0; _showmouse(); }
}

int graf_mouse(int mode, void *shape) {
	switch(mode) {
		case M_OFF: _mtot++;
			if(_mtot==1) _hidemouse();
			break;
		 case M_ON: if(_mtot) _mtot--;
			break;
		 case ARROW:
		 case TEXT_CRSR:
		 case BUSYBEE:
		 case POINT_HAND:
		 case FLAT_HAND:
		 case THIN_CROSS:
		 case THICK_CROSS:
		 case OUTLN_CROSS:
		 	_hidemouse(); _setmouse(_mice[_mshape=mode]); _showmouse(); break;
		 case USER_DEF:
			_hidemouse(); _setmouse(shape); _showmouse(); break;
		 default: return(0);
	}
	return(1);
}

void _mouse_event(void) {
short x, y, stbut, flags=0, key1, scan;
	_mevent.key=_mevent.kbstat=0;
	if(key[KEY_RSHIFT]) _mevent.kbstat=1;
	if(key[KEY_LSHIFT]) _mevent.kbstat|=2;
	if(key[KEY_RCONTROL]||key[KEY_LCONTROL]) _mevent.kbstat|=4;
	if(key[KEY_ALT]) _mevent.kbstat|=8;
	if(!(_mtot|_mousedrawn)) _showmouse();
	if(keypressed()) {
      flags|=M_KEYPRESS; key1=readkey(); scan=key1>>8;
		if(scan>=KEY_F1&&scan<=KEY_F12) _mevent.key=F1KEY+scan-KEY_F1;
		else if(scan==KEY_UP)  _mevent.key=UAKEY;
		else if(scan==KEY_DOWN)  _mevent.key=DAKEY;
		else if(scan==KEY_RIGHT)  _mevent.key=RAKEY;
		else if(scan==KEY_LEFT)  _mevent.key=LAKEY;
		else if(scan==KEY_DEL)  _mevent.key=DELKEY;
		else if(scan==KEY_BACKSPACE) _mevent.key=BCKKEY;
		else if(scan==KEY_SPACE) _mevent.key=32;
		else if(scan==KEY_ENTER) _mevent.key=CARRET;
      else _mevent.key=key1&255;
	}
	x=mouse_x; y=mouse_y;
	if(_mevent.x!=x||_mevent.y!=y) {
		flags|=M_MOTION;
		_mevent.x=x; _mevent.y=y;
	}
   stbut=mouse_b&3;
   if(_mevent.buttons!=stbut) {
	  if(stbut&STRBUT&&!(_mevent.buttons&STRBUT))
        flags|=M_RIGHT_DOWN;
     else if(!(stbut&STRBUT)&&_mevent.buttons&STRBUT)
        flags|=M_RIGHT_UP;
	  if(stbut&STLBUT&&!(_mevent.buttons&STLBUT))
        flags|=M_LEFT_DOWN;
     else if(!(stbut&STLBUT)&&_mevent.buttons&STLBUT)
        flags|=M_LEFT_UP;
	  _mevent.buttons=stbut;
   }
   _mevent.kinds=flags; _mevent.time=_timer;
}

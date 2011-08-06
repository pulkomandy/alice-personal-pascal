#include	<stdlib.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	<allegro.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"

short	_scrres, _scrwid, _scrht;
int	_colors, _origcolors;
int	_oldhand;
FDB	_screen;
extern	int	handle;
extern	short	_mousedrawn, _textcol, _linecol, _fillcol, _clipx1,
	_clipx2, _clipy1, _clipy2;
extern	_MEVENT	_mevent;
extern	void	_showmouse(), _hidemouse();
BITMAP	*_logbmp;
PALETTE _mad_palette = {
	{ 63, 63, 63 },	{ 0,  0,  0 },	{ 63, 0,  0 },
	{ 0,  63, 0 },	{ 0,  0,  63 },	{ 0,  63, 63 },
	{ 63, 63, 0 },	{ 63, 0,  63 },
	{ 31, 31, 31 },	{ 15, 15, 15 },	{ 63, 31, 31 },
	{ 31, 63, 31 },	{ 31, 31, 63 },	{ 31, 63, 63 },
	{ 63, 63, 31 },	{ 63, 31, 63 },
	
};

unsigned char _defpal[] = {
	255, 255, 255,	0,  0,  0,	255, 0,  0,
	0,  255, 0,	0,  0,  255, 0,  255, 255,
	255, 255, 0,	255, 0,  255,
	127, 127, 127, 63, 63, 63, 255, 127, 127,
	127, 255, 127,	127, 127, 255,	127, 255, 255,
	255, 255, 127,	255, 127, 255,
};

short	Getrez() { return(_scrres); }

char	*Logbase() { return((char *)_logbmp->dat); }

char	*Physbase() { return(NULL); }

void	Setpalette() {
	/*Refreshes the screen palette to the MAD palette.
	Setpallette is defined to this.*/
	set_palette(_mad_palette);
}

int Setcolor(int number, unsigned char *newcolor) {
	/*If the first parameter is less than 256, the color at that point in
the MAD palette is changed to the second parameter and the first parameter
is returned.
	Otherwise, the second parameter is checked against the used palette.
If one of the used colors is no more than 1 point each of RGB away, its
number is returned.
	Failing this, if there is room, a new color is added and the return
is its number.
	If all else fails, the number of the nearest color is returned.*/
int i, a, b, c, latest;
int diff, overdiff=63*63*63;
unsigned char *_atpal=(unsigned char *)_mad_palette;
	a=*newcolor>>2; b=newcolor[1]>>2; c=newcolor[2]>>2;
	if(number<256) {
		((int *)_mad_palette)[number]=a+b*256+c*65536;
		if(number>=_colors) _colors=number+1;
		return(number);
	}
	i=1; while(i<_colors) {
		diff=(_atpal[i*4]-a)*(_atpal[i*4]-a)+(_atpal[i*4+1]-b)*(_atpal[i*4+1]-b)
			+(_atpal[i*4+2]-c)*(_atpal[i*4+2]-c);
		if(diff<4) return(i);
		if(diff<overdiff) { overdiff=diff; latest=i; }
		i++;
	}
	if(i<256) {
		((int *)_mad_palette)[_colors]=a+b*256+c*65536;
		_colors++;
		return(_colors-1);
	}
	return(latest);
}


void Setscreen(int type, int mode) {
/*	If the type is -1, mode is the resolution that will be set, otherwise
mode is ignored.
	0 copies the virtual screen to the physical screen, and sets the handle
to point to the physical screen.
	1 copies the physical screen to the virtual screen, and sets the handle
to the virtual screen.*/
	switch(type) {
		case 0: if(handle>3) {
				blit(_logbmp, screen, 0, 0, 0, 0, _scrwid, _scrht);
				handle=_oldhand; _showmouse();
			}
			break;
		case 1: if(handle<4) {
				_hidemouse(); _oldhand=handle; handle=4;
				blit(screen, _logbmp, 0, 0, 0, 0, _scrwid, _scrht);
			}
			break;
		default:
			switch(_scrres=mode) {
				case 1:
					_scrwid=640; _scrht=480;
					break;
				case 2:
					_scrwid=800; _scrht=600;
					break;
				default:
					_scrwid=320; _scrht=200;
			}
			set_gfx_mode(GFX_AUTODETECT, _scrwid, _scrht, 0, 0);
			if(_logbmp) destroy_bitmap(_logbmp);
			_logbmp=create_bitmap(_scrwid, _scrht);
			_screen.fd_w=_scrwid; _screen.fd_h=_scrht;
			_clipx1=_clipy1=0; _clipx2=_scrwid-1; _clipy2=_scrht-1;
			_mevent.x=_scrwid>>1; _mevent.y=_scrht>>1;
			position_mouse(_mevent.x, _mevent.y);
			if(_scrres) {
				 ((int *)_mad_palette)[0]=0x3F3F3F;
				 ((int *)_mad_palette)[1]=0;
				 _defpal[0]=_defpal[1]=_defpal[2]=255;
				 _defpal[3]=_defpal[4]=_defpal[5]=0;
			}
	 		else {
				 ((int *)_mad_palette)[1]=0x3F3F3F;
				 ((int *)_mad_palette)[0]=0;
				 _defpal[0]=_defpal[1]=_defpal[2]=0;
				 _defpal[3]=_defpal[4]=_defpal[5]=255;
			}
			vst_color(handle, BLACK); vsl_color(handle, BLACK);
			vsf_color(handle, WHITE);
			_origcolors=_colors=16; Setpalette();
			_mousedrawn=0;
	}
}

/*Not ST but needed IMHO BD*/

int Getpalette() {
/*Gets number of colors used*/
	return(_colors);
}

int Resetpalette(int i) {
	/*If the parameter is 256, the number of colors is reset to the starting 
number, actually 16. Otherwise it is set to the parameter. The return is
the actual number of colors now recognized.
	Note that this routine does not change the screen colors.*/
	_colors=(i==256)?_origcolors:i;
	return(_colors);
}

void Palettize(int length, unsigned char *index, unsigned char *palette) {
/*This routine is passed a length and an index array and color array
of that length. It puts the colors into the MAD palette using Setcolor
and stores the assigned palette numbers in the index.
	The index can now be used to map a picture's pixels onto the screen
or into memory. 0 is _always_ transparent, so not set, but other
indices with the same palette values may be*/
/*My palettes are RGB*256; Allegro's are RGBX*63 Sigh*/
int i=1; 
	while(i<length) { index[i]=Setcolor(256, &palette[i*3]); i++; }
}

void Getcolor(int colno, unsigned char *savecolor) {
/*Atari Setcolor returns the old color; mine returns the number*/
/*Palettize means I don't know my actual colors, so...*/
int d=colno<<2;
unsigned char *_atpal=(unsigned char *)_mad_palette;
	savecolor[0]=_atpal[d]<<2;
	savecolor[1]=_atpal[d+1]<<2;
	savecolor[2]=_atpal[d+2]<<2;
}

int Grabpalette(unsigned char *savepalette) {
/*Copies the whole palette to savepalette, so it can be restored.
Returns the number of colors actually used.*/
int i=0;
	while(i<_colors) { Getcolor(i, &savepalette[i*3]); i++; }
	return(i);
}

void Storepalette(int from, int total, unsigned char *colors) {
/*This routine puts the colors in the array into the palette, starting at a 
point, for a length, ignoring any duplicates or already assigned colors.
It does not change the screen palette.*/
short i=0;
	total=min(total, 256-from);
	while(i<total) {
		Setcolor(from, &colors[i*3]); from++; i++;
	}
}

void Forcepalette(int from, int total, unsigned char *colors) {
/*This routine does change the screen palette.*/
	Storepalette(from, total, colors);
	Setpalette();
}

void Defaultpalette() {
/*Restores all palette conditions to what they were at startup.*/
	Forcepalette(0, 16, _defpal);
	_colors=16;
}


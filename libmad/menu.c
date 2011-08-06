#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<memory.h>
#include	<malloc.h>
#include	<time.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"

OBJECT	*_menu;
extern	short	_scrwid;
short	_boxstart, _menupx[8];
FDB	_menufdb;
extern	FDB	_screen;
extern	int	handle;

int menu_bar(OBJECT *menu, int flag) {
int i;
	if(_menu==menu&&!flag) {
		vro_cpyfm(handle, S_ONLY, _menupx, &_menufdb, &_screen);
		free(_menufdb.fd_addr); _menu=_menufdb.fd_addr=0l;
	}
	if(flag) {
		_menu=menu;
		_menupx[2]=_menupx[6]=_scrwid-1;
		_menupx[3]=_menupx[7]=10;
		if(_menufdb.fd_addr) free(_menufdb.fd_addr);
		_menufdb.fd_addr=(char *)malloc(_scrwid*11);
		_menufdb.fd_w=_scrwid; _menufdb.fd_h=11;
		vro_cpyfm(handle, S_ONLY, _menupx, &_screen, &_menufdb);
		i=_menu->ob_tail;
		_boxstart=_menu[i].ob_head;
		_menu->ob_width=_menu[1].ob_width=
			_menu[i].ob_width=_scrwid;
		objc_draw(_menu, 0, 1, 0, 0, 0, 0);
		objc_draw(_menu, 1, 8, 0, 0, 0, 0);
	}
	return(1);
}

int menu_tnormal(OBJECT *menu, int title, int norm) {
	if(norm) menu[title].ob_flags&=~SELECTED;
	else menu[title].ob_flags|=SELECTED;
	objc_draw(menu, title, 1, 0, 0, 0, 0);
	return(1);
}

int menu_icheck(OBJECT *menu, int title, int check) {
	if(!check) menu[title].ob_flags&=~CHECKED;
	else menu[title].ob_flags|=CHECKED;
	return(1);
}

int menu_ienable(OBJECT *menu, int title, int enable) {
	if(enable) menu[title].ob_flags&=~FEINT;
	else menu[title].ob_flags|=FEINT;
	return(1);
}

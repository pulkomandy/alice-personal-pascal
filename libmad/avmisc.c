#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<malloc.h>
#include	<memory.h>
#include	<allegro.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"

short	_clipx1, _clipx2, _clipy1, _clipy2;
extern	BITMAP	*_logbmp;
extern	short	_scrwid, _scrht, _fillcol;
extern	int	handle;
void *_trout;

int v_opnvwk(int *handle) {
/*At some stage I must rewrite this to really make handles matter
int i=0;
	while(i<4) {
		if(!_vwks[i]) break;
		i++;
	}
	if(i==4) return(*handle);
	_vwks[i]=gl_allocatecontext();
	if(!i) gl_getcontext(_vwks[i]);
	else {
		gl_setcontextvgavirtual(_vgamode);
		gl_getcontext(_vwks[i]);
		gl_setcontext(_vwks[*handle]);
	}
	gl_disableclipping();
	return(i); */
	return(*handle);
}

void v_clsvwk(int handle) {
/*GraphicsContext *gc=_vwks[handle];
	if (gc->modetype == CONTEXT_VIRTUAL) free(gc->vbuf);
	gc=NULL;
	_vwks[handle]=NULL;*/
}

void v_clrwk(int handle) {
	clear_to_color((handle>3)?_logbmp:screen, _fillcol);
}

void vs_clip(int handle, int flag, short *pxy) {
	if(flag) {
       _clipx1=pxy[0]; _clipy1=pxy[1]; _clipx2=pxy[2]; _clipy2=pxy[3];
    }
    else { _clipx1=_clipy1=0; _clipx2=_scrwid-1; _clipy2=_scrht-1; }
}

int vex_retime(int handle, void *newrout, void *oldrout, int time) {
	if(oldrout) oldrout=_trout;
	_trout=newrout;
	return(install_int(_trout, time));
}

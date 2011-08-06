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

short _linecol, _fillcol;
extern	short	_scrwid, _clipx1, _clipx2, _clipy1, _clipy2;
extern	BITMAP	*_logbmp;

int vsl_color(int handle, int color) {
	/*Sets the color for lines.*/
	return(_linecol=(color&255));
}

int vsf_color(int handle, int color) {
	/*Sets the color for fills.*/
	return(_fillcol=(color&255));
}

void v_bar(int handle, short *pxy) {
	/*Fills a rectangle. The array of 4 shorts gives the top left and 
bottom right points of the rectangle.
vr_recfl is defined to this.*/
int x1, y1, x2, y2;
	x1=max(pxy[0], _clipx1); y1=max(pxy[1], _clipy1);
	x2=min(pxy[2], _clipx2); y2=min(pxy[3], _clipy2);
	if(x2>x1&&y2>y1)
		rectfill((handle>3)?_logbmp:screen, x1, y1, x2, y2, _fillcol);
}

void _xor_pixel(BITMAP *scrn, int x, int y) {
	_putpixel(scrn, x, y, _getpixel(scrn, x, y)^1);
}

void _xor_box(int x1, int y1, int x2, int y2) {
	scare_mouse();
	do_line(screen, x1, y1, x2, y1, 0, (void *)_xor_pixel);
	do_line(screen, x2, y1+1, x2, y2, 0, (void *)_xor_pixel);
	do_line(screen, x1, y1+1, x1, y2, 0, (void *)_xor_pixel);
	do_line(screen, x1+1, y2, x2-1, y2, 0, (void *)_xor_pixel);
   unscare_mouse();
}

void v_pline(int handle, int n, short *pxy) {
	/*Draws a set of connected lines. The second parameter is the number
of points, and the third is an array of xy screen coordinates.*/
short i=0;
BITMAP *local;
	if(handle<4) local=screen;
   else local=_logbmp;
	while(--n) {
		line(local, (int)pxy[i], (int)pxy[i+1], (int)pxy[i+2], (int)pxy[i+3], _linecol); i+=2;
	}
}

void vro_cpyfm(int handle, int mode, short *pxy, FDB *src, FDB *dest) {
	/*A versatile blitting routine.
The array of 8 shorts gives the coordinates of top left/bottom right
of a source rectangle and a destination rectangle with respect to the top
left of the Frame they are contained by.
The second parameter is the mode, the blitting operation itself. 4
types are supported:
ALL_WHITE	The source rectangle is filled with color 0
S_ONLY		The source is blitted to the destination
D_ONLY		The source is blitted to the destination, except for
				pixels which are color 0 in the source
ALL_BLACK	The source rectangle is filled with color 1
The last 2 parameters are pointers to a source and destination Frame
Definition Block, a structure with the following definition:
void *fd_addr;
short fd_w;
short fd_h;
short fd_wdwidth;
short fd_stand;
short fd_nplanes;
short fd_r1;
short fd_r2;
short fd_r3;
Only the first 3 elements are used. The first is the starting address
of the frame; if this is NULL, it is assumed that the physical or 
virtual screen is being referred to. fd_w and fd_h are the width and 
height of the frame.*/
/*I'm trying to use my own clipping rectangle*/
short i, j;
unsigned char *datfrom, *datto;
int x, y, w, h, x1, y1, w1, h1, width, height, x2, y2, x3, y3;
BITMAP *damnedcard, *mustbe/*an easier way*/;
	mustbe=(handle<4)?screen:_logbmp;
	x=pxy[0]; y=pxy[1]; x2=pxy[2]; y2=pxy[3];
	x1=pxy[4]; y1=pxy[5]; x3=pxy[6]; y3=pxy[7];
	if(x<0) { x1+=x; x=0; }
	if(y<0) { y1+=y; y=0; }
	if(x1<0) { x+=x1; x1=0; }
	if(y1<0) { y+=y1; y1=0; }
	if(!src->fd_addr) {
		if(x<_clipx1) { x1-=(x-_clipx1); x=_clipx1; }
		if(x2>_clipx2) { x3-=(x2-_clipx2); x2=_clipx2; }
		if(y<_clipy1) { y1-=(y-_clipy1); y=_clipy1; }
		if(y2>_clipx2) { y3-=(y2-_clipy2); y2=_clipy2; }
	}
	if(!dest->fd_addr) {
		if(x1<_clipx1) { x-=(x1-_clipx1); x1=_clipx1; }
		if(x3>_clipx2) { x2-=(x3-_clipx2); x3=_clipx2; }
		if(y1<_clipy1) { y-=(y1-_clipy1); y1=_clipy1; }
		if(y3>_clipy2) { y2-=(y3-_clipy2); y3=_clipy2; }
	}
	w=src->fd_w; w1=dest->fd_w;
	h=src->fd_h; h1=dest->fd_h;
	width=min(x2-x, x3-x1)+1;
	if(width>min(w, w1)) width=min(w, w1);
	if(width<1) return;
	height=min(y2-y, y3-y1)+1;
	if(height>min(h, h1)) height=min(h, h1);
	if(height<1) return;
	if(mode==ALL_WHITE||mode==ALL_BLACK) {
		i=_fillcol; _fillcol=(mode==ALL_BLACK);
		v_bar(handle, pxy); _fillcol=i;
		return;
	}
	switch(mode) {
		case S_ONLY:
			if(src->fd_addr&&dest->fd_addr) {
				datfrom=src->fd_addr+x+y*w;
				datto=dest->fd_addr+x1+y1*w1;
				i=0; while(i<height) {
					memcpy(datto, datfrom, width);
					i++; datfrom+=w; datto+=w1;
				}
			}
			else {
            scare_mouse();
				damnedcard=create_bitmap(width, height);
				if(src->fd_addr) {
					datfrom=src->fd_addr+x+y*w;
					datto=damnedcard->dat;
					i=0; while(i<height) {
						memcpy(datto, datfrom, width);
						i++; datfrom+=w; datto+=width;
					}
					blit(damnedcard, mustbe, 0, 0, x1, y1, width, height);
				}
				else if(dest->fd_addr) {
					blit(mustbe, damnedcard, x, y, 0, 0, width, height);
					datfrom=damnedcard->dat;
					datto=dest->fd_addr+x1+y1*w1;
					i=0; while(i<height) {
						memcpy(datto, datfrom, width);
						i++; datfrom+=width; datto+=w1;
					}
				}
				else {
					blit(mustbe, damnedcard, x, y, 0, 0, width, height);
					blit(damnedcard, mustbe, 0, 0, x1, y1, width, height);
				}
            unscare_mouse();
				destroy_bitmap(damnedcard);
			}
			break;
		case D_ONLY:
			if(src->fd_addr&&dest->fd_addr) {
				datfrom=src->fd_addr+x+y*w;
				datto=dest->fd_addr+x1+y1*w1;
				i=0; while(i<height) {
					j=0; while(j<width) {
						if(datfrom[j]) datto[j]=datfrom[j];
						j++;
					}
					i++; datfrom+=w; datto+=w1;
				}
			}
			else {
            scare_mouse();
				damnedcard=create_bitmap(width, height);
				if(!damnedcard) {
					form_error_text("Uh Oh"); return;
				}
				if(src->fd_addr) {
					datfrom=src->fd_addr+x+y*w;
					datto=damnedcard->dat;
					i=0; while(i<height) {
						memcpy(datto, datfrom, width);
						i++; datfrom+=w; datto+=width;
					}
					masked_blit(damnedcard, mustbe, 0, 0, x1, y1, width, height);
				}
				else if(dest->fd_addr) {
					blit(mustbe, damnedcard, x, y, 0, 0, width, height);
					datfrom=damnedcard->dat;
					datto=dest->fd_addr+x1+y1*w1;
					i=0; while(i<height) {
						j=0; while(j<width) {
							if(datfrom[j]) datto[j]=datfrom[j];
							j++;
						}
						i++; datfrom+=width; datto+=w1;
					}
				}
				else {
					blit(mustbe, damnedcard, x, y, 0, 0, width, height);
					masked_blit(damnedcard, mustbe, 0, 0, x1, y1, width, height);
				}
            unscare_mouse();
				destroy_bitmap(damnedcard);
			}
			break;
	}
}

	/*only usable for turning bits to pixels
	write modes aren't properly supported
void vrt_cpyfm(int handle, int mode, short *pxy, FDB *src, FDB *dest,
	short *cols) {
char *whole, *mover, *line, *m;
short i, j, k, length, height, start, bits, newpx[8];
FDB newfdb;
	m=alloca(src->fd_w*src->fd_h);
	start=pxy[0]+pxy[1]*src->fd_w;
	length=pxy[2]-pxy[0]+1; height=pxy[3]-pxy[1]+1;
	bits=7-(pxy[0]&7);
	line=m; whole=(char *)src->fd_addr+(start>>3);
	i=0; while(i<height) {
		mover=whole; k=bits;
		j=0; while(j<length) {
			*line=((*mover)&(1<<k))?*cols:cols[1];
			if((--k)<0) { k=7; mover++; }
			j++; line++;
		}
		i++; whole+=src->fd_w;
	}
	newpx[0]=newpx[1]=0; newpx[2]=length-1; newpx[3]=height-1;
	memcpy(&newpx[4], &pxy[4], 8);
	newfdb.fd_addr=m; newfdb.fd_w=length; newfdb.fd_h=height;
	vro_cpyfm(handle, (mode==1)?S_ONLY:D_ONLY, pxy, &newfdb, dest);
}*/

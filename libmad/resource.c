#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<allegro.h>
#include	<malloc.h>
#include	<memory.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"
 
char	*_resource;
short	_rsrcin, _cursepx[4];
RSHDR	_rshdr;
TEDINFO	*_teds;
OBJECT	*_dials, **_trees;
extern	int	handle;
extern	short	_textcol, _texteff, _fillcol, _linecol, _fontno, _curpos;
extern	GFONT	*_font;

extern	unsigned	short	_guirsh[];
extern	void	_showmouse(), _hidemouse(), _xor_box();


int _rsrc_fix() {
char **tree;
short j;
TEDINFO *ted;
OBJECT *dialog;
	_teds=(TEDINFO *)&_resource[_rshdr.rsh_tedinfo];
	_dials=(OBJECT *)&_resource[_rshdr.rsh_object];
	_trees=(OBJECT **)&_resource[_rshdr.rsh_trindex];
	ted=_teds; j=0; while(j<_rshdr.rsh_nteds) {
		(ted->te_ptext)+=(long)_resource;
		(ted->te_ptmplt)+=(long)_resource;
		ted++; j++;
	}
	dialog=_dials; j=0; while(j<_rshdr.rsh_nobs) {
		if(dialog->ob_type>R_CHAR) dialog->ob_spec+=(long)_resource;
		j++; dialog++;
	}
	tree=(char **)_trees; j=0; while(j<_rshdr.rsh_ntrees) {
		tree[j++]+=(long)_resource;
	}
	return(1);
}

int rsrc_gaddr(int type, int index, void *address) {
	/*This is normally used to get into an OBJECT pointer the address of
a tree in the resource, in which case the first parameter is 0, the second
the number of the tree in the resource's list (more usually a macro produced
by the resource editor).
	Theoretically, it can also get the address of any object or tedinfo.*/
	if(index<0) return(0);
	switch(type) {
		case R_TREE: if(index>=_rshdr.rsh_ntrees) return(0);
				*(OBJECT **)address=_trees[index]; return(1);
		case R_OBJECT: if(index>=_rshdr.rsh_nobs) return(0);
			*(OBJECT *)address=_dials[index]; return(1);
		case R_TEDINFO: if(index>=_rshdr.rsh_nteds) return(0);
			*(TEDINFO *)address=_teds[index]; return(1);
		default: return(0);
	}
}

void _OLtext(short x, short y, short w, short h, short flags, char *text) {
char check[2];
	if(flags&OUTLINED) {
		x+=((w-strlen(text)*_font->max_cell_width)>>1);
		y+=(h-_font->form_height)>>1;
	}
	v_gtext(handle, x, y+_font->top, text);
	if(flags&CHECKED) {
		check[0]=8; check[1]=0;
		v_gtext(handle, x, y+_font->top, check);
	}
}

void _narrow(OBJECT *dialog, short *curno, int x, int y) {
short i, try, out;
OBJECT *object;
	object=&dialog[*curno];
	out=(object->ob_border==255);
	if(x<-out||y<-out||x>=object->ob_width+out||y>=object->ob_height+out||
		object->ob_flags&DISABLED) {
		*curno=-1; return;
	}
	if((i=object->ob_head)==-1) return;
	while(i!=*curno) {
		try=i;
		_narrow(dialog, &try, x-dialog[i].ob_x, y-dialog[i].ob_y);
		if(try!=-1) { *curno=try; break; }
		i=dialog[i].ob_next;
	}
}

int _relocate(OBJECT *dialog, short currobj, short seekobj, short *hx, short *hy) {
short i, j, x, y;
OBJECT *object=&dialog[currobj];
	x=*hx+object->ob_x; y=*hy+object->ob_y;
	if(currobj==seekobj) { *hx=x; *hy=y; return(1); }
	if(object->ob_flags&LASTOB||(i=object->ob_head)==-1) return(0);
	while(i!=currobj) {
		j=_relocate(dialog, i, seekobj, &x, &y);
		if (j==1) { *hx=x; *hy=y; return(1); }
		i=dialog[i].ob_next;
	}
	return(0);
}

int objc_offset(OBJECT *dialog, int seekobj, short *scr_x, short *scr_y) {
short x=0, y=0;
	if(_relocate(dialog, 0, seekobj, &x, &y)) {
		*scr_x=x; *scr_y=y; return(1);
	}
	return(0);
	
}

int objc_find(OBJECT *dialog, int currobj, int depth, int x, int y) {
/*	This returns the number of the object contained within currobj of dialog 
under relative x, y, or -1. Depth currently ignored*/
short gem=currobj, ox, oy;
	objc_offset(dialog, currobj, &ox, &oy);
	_narrow(dialog, &gem, x-ox, y-oy);
	return((int)gem);
}

void _obcurse(int on) {
short temp=_linecol;
	/*if(!on) vsl_color(handle, _fillcol);*/
	vsl_color(handle, (on)?_textcol:_fillcol);
	v_pline(handle, 2, _cursepx);
	vsl_color(handle, temp);
}

void _objc_draw(OBJECT *dialog, short currobj, short depth, short x, short y) {
short i, flags, n, nextobj, w, h, px[10], x1=x, y1=y;
char *j, *k, *l, *m, comb[256];
OBJECT *object=&dialog[currobj], *nobject;
TEDINFO *ted=(TEDINFO *)object->ob_spec;
	if(!depth||object->ob_flags&HIDETREE) return;
	w=object->ob_width; h=object->ob_height;
	if((flags=object->ob_flags)&SELECTED&&
		object->ob_type!=R_STEXT&&object->ob_type!=R_ETEXT) {
		vst_color(handle, (object->ob_colbyte&15)^1);
		vsf_color(handle, (object->ob_fill)^1);
	}
	else {
		vst_color(handle, object->ob_colbyte&15);
		vsf_color(handle, object->ob_fill);
	}
	vst_effects(handle, (flags&DISABLED)?SHADED:0);
	vst_font(handle, object->ob_font);
	if((flags&OUTLINED)&&object->ob_border!=1) {
		px[0]=x-1; px[1]=y-1; px[2]=x+w; px[3]=y+h;
	}
	else {
		px[0]=x; px[1]=y; px[2]=x+w-1; px[3]=y+h-1;
	}
	vs_clip(handle, 1, px);
	if(object->ob_type!=R_IBOX) v_bar(handle, px);
	switch(object->ob_type) {
		case R_TEXT:
		case R_TITLE: _OLtext(x, y, w, h, flags, (char *)object->ob_spec);
				break;
		case R_CHAR: _OLtext(x, y, w, h, flags|OUTLINED, (char *)&object->ob_spec);
				break;
		case R_FTEXT:
			strcpy(comb, ted->te_ptmplt);
			j=comb; while(*j&&*j!='_') j++;
			k=ted->te_ptext;
			while(*j&&*k) {
					l=j; while(*l=='_') l++;
					if(*l==*k) {
							while(j<l) *(k++)=*(j++)=' ';
						*k=0;
					}
					if(*j=='_'&&*k) { *(j++)=*(k++); }
					else j++;
			}
			_OLtext(x, y, w, h, flags, comb);
			break;
		case R_STEXT:
		case R_ETEXT:
			k=ted->te_ptext;
			if(object->ob_type==R_STEXT) {
				i=0; j=ted->te_ptmplt;
				while(*j) { if(*(j++)=='_') i++; }
				n=strlen(k); if(n>=i) k+=n-i+1;
			}
			if(flags&OUTLINED) {
				x+=((w-strlen(ted->te_ptmplt)*_font->max_cell_width)>>1);
				y+=((h-_font->form_height)>>1)+1;
			}
			m=comb; i=0; while(ted->te_ptmplt[i]!='_') {
				*(m++)=ted->te_ptmplt[i]; i++;
			}
			j=&ted->te_ptmplt[i];
			while(*j&&*k) {
				l=j; while(*l=='_') l++;
				if(*l&&*l==*k) {
					while(j<l) { *(m++)=' '; j++; }
				}
				if(*(j++)=='_'&&*k) { *(m++)=*(k++); }
			}
			*m=0;
			_OLtext(x, y, w, h, flags&~OUTLINED, comb);
			if(*j) {
				_OLtext(x+strlen(comb)*_font->max_cell_width, y, w, h, flags&~OUTLINED, j);
			}
			if(flags&SELECTED) {
				_cursepx[0]=_cursepx[2]=x+/*strlen(comb)*/(i+_curpos)*_font->max_cell_width;
				_cursepx[1]=y; _cursepx[3]=y+h-1;
				_obcurse(1);
			}
	}
	/*if(!(object->ob_flags&HIDETREE)) {*/
		nextobj=(short)object->ob_head;
		if(nextobj>=0) {
			while(nextobj!=currobj) {
				nobject=&dialog[nextobj];
				_objc_draw(dialog, nextobj, depth-1, x+nobject->ob_x, y+nobject->ob_y);
				nextobj=nobject->ob_next;
			}
		}
	/*}*/
	if(flags&OUTLINED) {
		vsl_color(handle, object->ob_colbyte>>4);
		if(object->ob_border!=1||(flags&DEFAULT)) {
			px[0]=px[6]=px[8]=x1-1; px[1]=px[3]=px[9]=y1-1;
			px[2]=px[4]=x1+w; px[5]=px[7]=y1+h;
			v_pline(handle, 5, px);
		}
		if(object->ob_border==1||(flags&DEFAULT)) {
			px[0]=px[6]=px[8]=x1; px[1]=px[3]=px[9]=y1;
			px[2]=px[4]=x1+w-1; px[5]=px[7]=y1+h-1;
			v_pline(handle, 5, px);
		}
	}
}

int objc_draw(OBJECT *dialog, int startobj, int depth, int clipx, int clipy, int
	clipw, int cliph) { /*This draws an object (generally a tree), from a
starting object within it (generally 0), to a depth (generally MAX_DEPTH as
the routine will stop when it runs out of objects anyway). I don't use the
last 4 parameters except to reset the starting object.*/
short oldlcol=_linecol, oldtcol=_textcol, oldfcol=_fillcol, oldfont=_fontno,
	oldteff=_texteff,  x, y;
	scare_mouse();
	if(clipx) dialog->ob_x=clipx;
	if(clipy) dialog->ob_y=clipy;
	if(clipw) dialog->ob_width=clipw;
	if(cliph) dialog->ob_height=cliph;
	objc_offset(dialog, startobj, &x, &y);
	_objc_draw(dialog, startobj, depth, x, y);
	unscare_mouse();
	_linecol=oldlcol; _textcol=oldtcol; _fillcol=oldfcol; _fontno=oldfont;
	_texteff=oldteff;
	vst_font(handle, oldfont);
	vs_clip(handle, 0, NULL);
	return(1);
}

int graf_rubberbox(int ox, int oy, int minw, int minh, 
	short *lastw, short *lasth) {
/*Draws a box right and down from ox, oy, with a minimum width, minw, 
and height, minh, to the mouse position while the left button is held 
down.  The final width and height are returned in lastw, lasth.*/
short button, mx, my, oldx, oldy, badx, bady;
	badx=ox+minw; bady=oy+minh;
	vq_mouse(handle, &button, &oldx, &oldy);
	_xor_box(ox, oy, oldx, oldy);
	while(1) {
		vq_mouse(handle, &button, &mx, &my);
		if(!button) break;
	  if((mx!=oldx||my!=oldy)&&mx>=badx&&my>=bady) {
		 _xor_box(ox, oy, oldx, oldy);
		 oldx=mx; oldy=my;
		_xor_box(ox, oy, oldx, oldy);
	  }
	}
	_xor_box(ox, oy, oldx, oldy);
	*lastw=max(minw, oldx-ox+1);
	*lasth=max(minh, oldy-oy+1);
	return(1);
}

int graf_dragbox(int movew, int moveh, int ox, int oy,
	int bordx, int bordy, int bordw, int bordh, short *lastx,
	short *lasty) {
/*Drags a box of size movew, moveh, starting at ox, oy, around the 
screen between the boundaries given by bordx, bordy, bordw and bordh, 
as long as the left mouse button is held down.  The top left corner of 
the final position is returned in lastx, lasty.*/
short button, mx, my, oldx, oldy, xdisp, ydisp, badx, bady;
	badx=bordx+bordw; bady=bordy+bordh;
	vq_mouse(handle, &button, &oldx, &oldy);
	xdisp=oldx-ox; ydisp=oldy-oy;
	_xor_box(ox, oy, ox+movew-1, oy+moveh-1);
	while(1) {
		vq_mouse(handle, &button, &mx, &my);
		if(!button) break;
		if(mx!=oldx||my!=oldy) {
			if(mx-xdisp>=bordx&&mx-xdisp+movew<badx) oldx=mx;
			if(my-ydisp>=bordy&&my-ydisp+moveh<bady) oldy=my;
			if(ox!=oldx-xdisp||oy!=oldy-ydisp) {
				_xor_box(ox, oy, ox+movew-1, oy+moveh-1);
				ox=oldx-xdisp; oy=oldy-ydisp;
				_xor_box(ox, oy, ox+movew-1, oy+moveh-1);
			}
			position_mouse(oldx, oldy);
		}
	}
	_xor_box(ox, oy, ox+movew-1, oy+moveh-1);
	*lastx=ox; *lasty=oy;
	return(1);
}

int graf_slidebox(OBJECT *slider, int S_Gauge, int S_Slide, int slvh) {
/*	Moves an object within the limits of another object, either vertically 
	(slvh==1) or horizontally (slvh==0).
	Returns the position of the center of the slider relative to the gauge.*/
short button, mx, my, limit, displace, oldslide, gax, gay, safecol=_textcol;
	vq_mouse(handle, &button, &mx, &my);
	objc_offset(slider, S_Gauge, &gax, &gay);
	if(!slvh) {
		oldslide=slider[S_Slide].ob_x;
		limit=slider[S_Gauge].ob_width-slider[S_Slide].ob_width;
		while(button) {
			displace=mx;
			vq_mouse(handle, &button, &mx, &my);
			oldslide+=(mx-displace);
			if(oldslide!=slider[S_Slide].ob_x&&oldslide<limit&&oldslide>=0) {
				slider[S_Slide].ob_x=oldslide;
				_hidemouse();
				_objc_draw(slider, S_Gauge, 8, gax, gay);
				_showmouse();
			}
			else oldslide=slider[S_Slide].ob_x;
		}
		_textcol=safecol;
		return(oldslide+slider[S_Slide].ob_width/2);
	}
	else {
		oldslide=slider[S_Slide].ob_y;
		limit=slider[S_Gauge].ob_height-slider[S_Slide].ob_height;
		while(button) {
			displace=my;
			vq_mouse(handle, &button, &my, &my);
			oldslide+=(my-displace);
			if(oldslide!=slider[S_Slide].ob_y&&oldslide<limit&&oldslide>=0) {
				slider[S_Slide].ob_y=oldslide;
				_hidemouse();
				_objc_draw(slider, S_Gauge, 8, gax, gay);
				_showmouse();
			}
			else oldslide=slider[S_Slide].ob_y;
		}
		_textcol=safecol;
		return(oldslide+slider[S_Slide].ob_height/2);
	}
}

int rsrc_free() {
	/*Frees up the resource.*/
	if(!_rsrcin) free(_resource);
	return(1);
}

int rsrc_load(const char *filename) {
	/*Loads a resource. If it fails, there is no point in 
continuing, so it reports this and closes down the program.*/
FILE *fp;
	if(!(fp=fopen(filename, "rb"))) {
		form_alert(1, "[1][Can't open resource!][Bummer]");
		appl_exit();
	}
	fread(&_rshdr, sizeof(RSHDR), 1, fp);
	_resource=(char *)malloc(_rshdr.rsh_rssize);
	fread(_resource, _rshdr.rsh_rssize, 1, fp);
	fclose(fp);
	_rsrcin=0;
	return(_rsrc_fix());
}

int rsrc_include(void *include) {
/*	Sets up a resource which was compiled in to the program.*/
	memcpy(&_rshdr, include, sizeof(RSHDR));
	_resource=&((char *)include)[sizeof(RSHDR)];
	_rsrcin=1;
	return(_rsrc_fix());
}

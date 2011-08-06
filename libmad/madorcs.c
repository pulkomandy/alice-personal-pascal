#include	<fcntl.h>
#include	"osbind.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<allegro.h>
#include	<math.h>
#include	"vdi.h"
#include	"aes.h"
#include	"madrsc.h"

#define	UCHAR	unsigned char

int	handle;
char	version[]={"MADoRCS|Rsr Maker|11/07/01"};
extern	unsigned	short	rscbuff[];
extern	int	objc_parent();
short	rsc_loaded, tree_open, nobs, ntrees, nteds, button, autosnap, 
	autosize, autohalf, objfont, tcolor=1, bcolor=1, fcolor, is_menu,
	boxa, obj_selected=-1, addobj, pasteobj, copycount, mx, my;
int messlength, copylength, currtree;

char 	selfile[FNSIZE], rscdir[FMSIZE], messages[64000],
	treenames[100][13], copytext[32000];

FILE	*fp;

OBJECT	*menu, *work, *trname, *etxttmp, *txttmp, *chartmp, *fndtxt,
	*fndname, *fndobj, *srtform, *menusamp, *extrinfo, *dialog;
	
RSHDR	rshdr;
TEDINFO	teds[1000];
OBJECT	dials[1000], *trees[100];

typedef	struct	INDEX	{
	char	treetype;
	char	name[13];
} INDEX;

INDEX	rcs_index[1000];

typedef	struct COPY {
	OBJECT	ob;
	TEDINFO	ted;
	char	name[13];
} COPY;

COPY	treecopy[512];

extern	void	objc_paste(), objc_copy(), objc_delete();

void do_form(OBJECT *dialog, short x, short y, short w, short h) {
	objc_draw(dialog, 0, 8, x, y, w, h);
	button=form_do(dialog, 0);
	dialog[button].ob_flags&=~SELECTED;
}

void form_show(OBJECT *dialog, short *x, short *y, short *w, short *h) {
	form_center(dialog, x, y, w, h);
	form_dial(0, 0, 0, 0, 0, *x, *y, *w, *h);
}

void form_close(short x, short y, short w, short h) {
	form_dial(3, 0, 0, 0, 0, x, y, w, h);
}

void tab() {
char i;
	i=9; fwrite(&i, 1, 1, fp);
}

void lineend() {
char i;
	i=10; fwrite(&i, 1, 1, fp);
}

void number(unsigned short i) {
char j[8];
	sprintf(j,"%d,", i); fwrite(&j, strlen(j), 1, fp);
}

void numbertab(unsigned short i) {
	number(i); tab();
}

void numberend(unsigned short i) {
	number(i); lineend();
}

int get_index(OBJECT *obj) {
	return(obj-dials);
}

void draw_menu() {
int i;
	menu_bar(menu, 0);
	menu_icheck(menu, M_AutoSize, autosize);
	menu_icheck(menu, M_AutoSnap, autosnap);
	menu_icheck(menu, M_AutoHalf, autohalf);
	menu_ienable(menu, M_Tree, rsc_loaded&&tree_open==0);
	menu_ienable(menu, M_Reload, rsc_loaded);
	menu_ienable(menu, M_Save, rsc_loaded);
	menu_ienable(menu, M_SaveAs, rsc_loaded);
	menu_ienable(menu, M_SaveC, rsc_loaded);
	menu_ienable(menu, M_Object, tree_open);
	menu_ienable(menu, M_Misc, tree_open);
	menu_ienable(menu, M_Edit, obj_selected>=0||copycount);
	menu_ienable(menu, M_Flags, obj_selected>=0);
	menu_ienable(menu, M_Fill,
		obj_selected>=0&&dialog[obj_selected].ob_type!=R_IBOX);
	menu_ienable(menu, M_Extras, obj_selected>=0);
	menu_ienable(menu, M_Text, obj_selected>=0&&
		dialog[obj_selected].ob_type>=R_CHAR);
	menu_ienable(menu, M_Paste, copycount);
	if(tree_open) {
		menu_ienable(menu, M_Title, is_menu);
		menu_ienable(menu, M_Block, !is_menu);
		menu_ienable(menu, M_IBox, !is_menu);
		menu_ienable(menu, M_Char, !is_menu);
		menu_ienable(menu, M_FText, !is_menu);
		menu_ienable(menu, M_EText, !is_menu);
		menu_ienable(menu, M_SText, !is_menu);
	}
	menu_ienable(menu, M_Sort, obj_selected>=0&&
		dialog[obj_selected].ob_head>=0);
	if(obj_selected<0) {
		menu[M_Border].ob_flags|=DISABLED;
	}
	else {
		if(dialog[obj_selected].ob_flags&OUTLINED) {
			menu[M_Border].ob_flags&=~DISABLED;
			menu[M_Wrap].ob_spec=(dialog[obj_selected].ob_border==1)?
				"  Inside":"  Outside";
		}
		i=dialog[obj_selected].ob_head; while(i>=0&&i!=obj_selected) {
			if(dialog[i].ob_flags&HIDETREE) { i=1000; break; }
			i=dialog[i].ob_next;
		}
		menu_ienable(menu, M_UndoHide, i==1000);
		i=0; while(i<16) {
			menu_icheck(menu, B0+i, i==bcolor);
			menu_icheck(menu, T0+i, i==tcolor);
			menu_icheck(menu, F0+i, i==fcolor);
			i++;
		}
		menu_icheck(menu, M_Small, objfont==2);
		menu_icheck(menu, M_Normal, !objfont);
		menu_icheck(menu, M_Large, objfont==1);
		menu_icheck(menu, M_Selectable, dialog[obj_selected].ob_flags&EXIT);
		menu_icheck(menu, M_Default, dialog[obj_selected].ob_flags&DEFAULT);
		menu_icheck(menu, M_Checked, dialog[obj_selected].ob_flags&CHECKED);
		menu_icheck(menu, M_Disabled, dialog[obj_selected].ob_flags&DISABLED);
		menu_icheck(menu, M_Outlined, dialog[obj_selected].ob_flags&OUTLINED);
	}
	menu_bar(menu, 1);
}

void show_tree(int i) {
short px[4];
	graf_mouse(M_OFF, 0l);
	px[0]=(i%6)*100+20; px[1]=(i/6)*40+30;
	px[2]=px[0]+95; px[3]=px[1]+32;
	if(i<ntrees)
	  vsf_color(handle, ((trees[i])->ob_width<0)?RED:GREEN);
	else vsf_color(handle, BLUE);
	v_bar(handle, px);
	v_gtext(handle, px[0]+(12-strlen(treenames[i]))*4, px[1]+30,
		treenames[i]);
	graf_mouse(M_ON, 0l);
}

void show_trees() {
int i;
	work[W_Title].ob_spec=selfile;
	objc_draw(work, 0, 8, 0, 12, 0, 0);
	tree_open=i=0; while(i<=ntrees) show_tree(i++);
}

void re_store() {
	if(obj_selected>=0) {
		dialog[obj_selected].ob_flags&=~SELECTED;
		dialog[obj_selected].ob_colbyte=(bcolor<<4)+tcolor;
		dialog[obj_selected].ob_fill=fcolor;
		dialog[obj_selected].ob_font=objfont;
		objc_draw(dialog, obj_selected, 8, 0, 0, 0, 0);
	}
}

void show_selected(int junk) {
	obj_selected=junk;
	if(obj_selected>=0) {
		bcolor=dialog[obj_selected].ob_colbyte>>4;
		tcolor=dialog[obj_selected].ob_colbyte&15;
		fcolor=dialog[obj_selected].ob_fill;
		objfont=dialog[obj_selected].ob_font;
		dialog[obj_selected].ob_flags|=SELECTED;
		objc_draw(dialog, obj_selected, 8, 0, 0, 0, 0);
		if(boxa) objc_draw(dialog, boxa, 8, 0, 0, 0, 0);
		draw_menu();
	}
}

void objc_adopt() {
int parent, pobj, sobj;
short ox, oy, nx, ny, width, height;
OBJECT *bigboy=&dialog[obj_selected];
   if(bigboy->ob_type<=R_IBOX) return;
	parent=objc_parent(obj_selected);
	if(parent<0) return;
	ox=bigboy->ob_x; oy=bigboy->ob_y;
	width=bigboy->ob_width; height=bigboy->ob_height;
	pobj=dialog[parent].ob_head;
	while (1) {
		if(pobj!=obj_selected) {
			nx=dialog[pobj].ob_x-ox; ny=dialog[pobj].ob_y-oy;
			if(nx>=0&&ny>=0&&nx<width&&ny<height) {
				if(form_alert(2, "[3][Adopt covered objects?][Leave|Adopt]")==1)
					return;
				else break;
			}
		}
		pobj = dialog[pobj].ob_next;
		if(pobj==parent) return;
	}
	pobj=dialog[parent].ob_head;
	while (1) {
		sobj=dialog[pobj].ob_next;
		if(pobj!=obj_selected) {
			nx=dialog[pobj].ob_x-ox; ny=dialog[pobj].ob_y-oy;
			if(nx>=0&&ny>=0&&nx<width&&ny<height) {
				copycount=copylength=0;
				objc_copy(dialog, sobj, -1, pobj);
				objc_paste(dialog, obj_selected, nx, ny);
				objc_delete(pobj, 0);
				if(sobj>pobj) sobj-=copycount;
				if(obj_selected>pobj) obj_selected-=copycount;
			}
		}
		pobj=sobj; if(pobj==parent) return;
	}
}

int objc_parent(int obj) {
int pobj;
	if(obj<=0) return(-1);
	pobj=obj;
	do {
		obj=pobj;
		pobj = dialog[obj].ob_next;
	} while(dialog[pobj].ob_tail!=obj);
	return(pobj);
}

int objc_prev(int obj) {
int pobj;
	pobj=objc_parent(obj);
	if(pobj>=0) {
		if(dialog[pobj].ob_head==obj) return(-1);
		pobj=dialog[pobj].ob_head;
		while(dialog[pobj].ob_next!=obj) pobj=dialog[pobj].ob_next;
	}
	return(pobj);
}

int objc_children(int obj) {
int pobj, nobj, count=0;
	pobj=dialog[obj].ob_head; if(pobj<0) return(0);
	nobj=dialog[obj].ob_tail;
	while(1) {
		count++;
		if(nobj==pobj) return(count);
		pobj=dialog[pobj].ob_next;
	}
}

int objc_childno(int parent, int obj) {
int nobj, pobj, count=0;
	if(parent<0) return(0);
	pobj=dialog[parent].ob_head;
	if(pobj==obj) return(0);
	while(1) {
		nobj = dialog[pobj].ob_next;
		if(nobj==parent) return(-1);
		count++; if(nobj==obj) return(count);
		pobj=nobj;
	}
}

void kill_box() {
short boxpx[4];
	objc_offset(dialog, boxa, boxpx, &boxpx[1]);
	boxpx[0]--; boxpx[1]--;
	boxpx[2]=boxpx[0]+dialog[boxa].ob_width+2;
	boxpx[3]=boxpx[1]+dialog[boxa].ob_height+2;
	vsf_color(handle, 0);
	graf_mouse(M_OFF, 0l);
	v_bar(handle, boxpx);
	graf_mouse(M_ON, 0l);
	boxa=0;
}

int boxof(int obj) {
int i, j;
	i=dialog[2].ob_head;
	j=dialog[dialog->ob_tail].ob_head;
	while(i!=obj) {
		i=dialog[i].ob_next; j=dialog[j].ob_next;
	}
	return(j);
}

void del_message(char *mess) {
short j, length;
OBJECT *dial=dials;
	length=strlen(mess)+1;
	j=0; while(j<nteds) {
		if(teds[j].te_ptext>mess) teds[j].te_ptext-=length;
		if(teds[j].te_ptmplt>mess) teds[j].te_ptmplt-=length;
		j++;
	}
	j=0; while(j<nobs) {
		if((dial->ob_type==R_TEXT||dial->ob_type==R_TITLE)&&
			dial->ob_spec>mess)
			dial->ob_spec-=length;
		j++; dial++;
	}
	messlength-=length;
	memmove(mess, &mess[length], messlength-(int)messages+(int)mess);
}

void remove_obj(int index) {
short j;
OBJECT *dial=dials, *delthis=&dials[index];
	switch(delthis->ob_type) {
		case R_TEXT:
		case R_TITLE:
			del_message(delthis->ob_spec);
			break;
		case R_FTEXT:
		case R_ETEXT:
		case R_STEXT:
			del_message(((TEDINFO *)delthis->ob_spec)->te_ptext);
			del_message(((TEDINFO *)delthis->ob_spec)->te_ptmplt);
			j=0; while(j<nobs) {
				if((dial->ob_type>=R_FTEXT&&dial->ob_type<=R_STEXT)&&
					dial->ob_spec>delthis->ob_spec)
					dial->ob_spec-=sizeof(TEDINFO);
				j++; dial++;
			}
			nteds--;
			memmove(delthis->ob_spec, delthis->ob_spec+sizeof(TEDINFO),
				(nteds-((TEDINFO *)delthis->ob_spec-teds))*sizeof(TEDINFO));
			break;
	}
	j=0; while(j<ntrees) {
		if(trees[j]>delthis) trees[j]--;
		j++;
	}
	nobs--;
	memmove(delthis, &delthis[1], (nobs-index)*sizeof(OBJECT));
	memmove(&rcs_index[index], &rcs_index[index+1], 
		(nobs-index)*sizeof(INDEX));
}

void objc_delete(int obj, int start) {
short i, index, parent, prev;
OBJECT *dial=dialog, *delthis=&dialog[obj];
	if(!obj&&obj_selected>=0) return; /*deleting a _tree_ starts at 0*/
	prev=objc_prev(obj);
	parent=objc_parent(obj);
	if(start&&delthis->ob_head>=0) {
		if(form_alert(2, "[3][Delete children too?][Leave|DELETE]")==1) {
			i=delthis->ob_head; while(i!=obj) {
				dialog[i].ob_x+=delthis->ob_x;
				dialog[i].ob_y+=delthis->ob_y;
				i=dialog[i].ob_next;
			}
			dialog[delthis->ob_tail].ob_next=delthis->ob_next;
			if(dialog[parent].ob_tail==obj)
				dialog[parent].ob_tail=delthis->ob_tail;
			delthis->ob_next=delthis->ob_head;
			delthis->ob_head=delthis->ob_tail=-1;
		}
	}
	while(delthis->ob_head>=0) {
		objc_delete(delthis->ob_head, 0);
	}
	index=get_index(delthis);
	if(prev<0) {
		if(delthis->ob_next==parent)
			dialog[parent].ob_head=dialog[parent].ob_tail=-1;
	}
	else {
		dialog[prev].ob_next=delthis->ob_next;
		if(delthis->ob_next==parent) dialog[parent].ob_tail=prev;
	}
	if(delthis->ob_flags&LASTOB) delthis[-1].ob_flags|=LASTOB;
	else while(1) {
		if(dial->ob_next>obj) dial->ob_next--;
		if(dial->ob_head>obj) dial->ob_head--;
		if(dial->ob_tail>obj) dial->ob_tail--;
		if(dial->ob_flags&LASTOB) break;
		dial++;
	}
	remove_obj(get_index(delthis));
}

void objc_copy(OBJECT *dialog, int par, int newpar, int child) {
int i, j;
OBJECT *currobj=&dialog[child];
COPY *currcopy=&treecopy[copycount];
	currcopy->ob=*currobj; currcopy->ob.ob_flags&=~SELECTED;
	switch(currobj->ob_type) {
		case R_TEXT:
		case R_TITLE:
			strcpy(&copytext[copylength], currobj->ob_spec);
			currcopy->ob.ob_spec=&copytext[copylength];
			copylength+=strlen(currobj->ob_spec)+1; break;
		case R_ETEXT:
		case R_FTEXT:
		case R_STEXT:
			currcopy->ted=*(TEDINFO *)currobj->ob_spec;
			strcpy(&copytext[copylength],
				((TEDINFO *)currobj->ob_spec)->te_ptext);
			currcopy->ted.te_ptext=&copytext[copylength];
			copylength+=strlen(((TEDINFO *)currobj->ob_spec)->te_ptext)+1;
			strcpy(&copytext[copylength],
				((TEDINFO *)currobj->ob_spec)->te_ptmplt);
			currcopy->ted.te_ptmplt=&copytext[copylength];
			copylength+=strlen(((TEDINFO *)currobj->ob_spec)->te_ptext)+1;
	}
	copycount++;
	if(currobj->ob_head>=0) {
		j=copycount-1;
		currcopy->ob.ob_head=copycount;
		objc_copy(dialog, child, j, currobj->ob_head);
		i=copycount; while(1) {
			if(dialog[i].ob_next==j) break;
			i=dialog[i].ob_next;
		}
		currcopy->ob.ob_tail=copycount-1;
	}
	if(currobj->ob_next!=par) {
		currcopy->ob.ob_next=copycount;
		objc_copy(dialog, par, newpar, currobj->ob_next);
	}
	else currcopy->ob.ob_next=newpar;
	currcopy->ob.ob_flags&=~LASTOB;
	strcpy(currcopy->name, rcs_index[get_index(currobj)].name);
}

void add_obj(int parent, int type, int ox, int oy) {
int child;
short i, j;
OBJECT *dial=dialog, *mummy, *baby, embryo;
	mummy=&dialog[parent];
	memset(&embryo, 0, sizeof(OBJECT));
	embryo.ob_head=embryo.ob_tail=-1;
	embryo.ob_x=ox; embryo.ob_y=oy; embryo.ob_next=parent;
	switch(type) {
		case M_Block: embryo.ob_type=R_BLOCK;
			if(is_menu) {
				embryo.ob_width=48; embryo.ob_height=8;
				embryo.ob_colbyte=16; embryo.ob_border=255;
				embryo.ob_flags=OUTLINED;
			}
			else {
				embryo.ob_width=mummy->ob_width/2;
				embryo.ob_height=mummy->ob_height/2;
			}
			break;
	  	case M_IBox: embryo.ob_type=R_IBOX;
			embryo.ob_width=mummy->ob_width/2;
			embryo.ob_height=mummy->ob_height/2;
			break;
		case M_Char: embryo.ob_type=R_CHAR;
			embryo.ob_width=embryo.ob_height=8;
			(int)embryo.ob_spec='X';
			embryo.ob_colbyte=1;
			break;
		case M_TextOb: embryo.ob_type=R_TEXT;
			if(is_menu) {
				embryo.ob_x=0; embryo.ob_y&=~7;
				embryo.ob_width=dialog[boxa].ob_width;
			}
			else embryo.ob_width=48;
			embryo.ob_height=8;
			embryo.ob_spec=&messages[messlength];
			embryo.ob_colbyte=1;
			strcpy(&messages[messlength], " Text");
			messlength+=6; break;
		case M_Title: embryo.ob_type=R_TITLE;
			embryo.ob_width=56; embryo.ob_height=11;
			embryo.ob_spec=&messages[messlength];
			embryo.ob_colbyte=1;
			strcpy(&messages[messlength], " Title");
			messlength+=7;
			break;
		default: embryo.ob_type=R_FTEXT+(type-M_FText);
			embryo.ob_width=80; embryo.ob_height=8;
			(TEDINFO *)embryo.ob_spec=&teds[nteds];
			embryo.ob_colbyte=1;
			teds[nteds].te_ptext=&messages[messlength];
			strcpy(&messages[messlength], "Text");
			messlength+=5;
			teds[nteds].te_ptmplt=&messages[messlength];
			strcpy(&messages[messlength], "Edit: ____");
			teds[nteds].te_pvalid='X';
			messlength+=11;
			nteds++;
	}
	if(embryo.ob_width>mummy->ob_width-embryo.ob_x)
		embryo.ob_width=mummy->ob_width-embryo.ob_x;
	if(embryo.ob_height>mummy->ob_height-embryo.ob_y)
		embryo.ob_height=mummy->ob_height-embryo.ob_y;
	if(mummy->ob_tail>=0) {
		i=mummy->ob_tail;
		if(dialog[i].ob_flags&LASTOB) {
		  dialog[i].ob_flags&=~LASTOB;
		  embryo.ob_flags|=LASTOB;
		}
	}
	else if(mummy->ob_next!=objc_parent(parent)) {
		i=mummy->ob_next;
	}
	else {
		i=parent;
		if(dialog[parent].ob_flags&LASTOB) {
		  dialog[parent].ob_flags&=~LASTOB;
		  embryo.ob_flags|=LASTOB;
		}
	}
	baby=&dialog[i+1]; child=get_index(baby);
	if(child<nobs) {
		memmove(&baby[1], baby, (nobs-child)*sizeof(OBJECT));
		memmove(&rcs_index[child+1], &rcs_index[child],
			(nobs-child)*sizeof(INDEX));
	}
	*baby=embryo; obj_selected=i+1;
	memset(&rcs_index[child], 0, 2);
	while(1) {
		if(dial->ob_next>=obj_selected) dial->ob_next++;
		if(dial->ob_head>=obj_selected) dial->ob_head++;
		if(dial->ob_tail>=obj_selected) dial->ob_tail++;
		if(dial->ob_flags&LASTOB) break;
		dial++;
	}
	j=mummy->ob_tail; mummy->ob_tail=obj_selected;
	if(j<0) mummy->ob_head=obj_selected;
	else dialog[j].ob_next=obj_selected;
	i=0; while(i<ntrees) {
		if(trees[i]>=baby) trees[i]++;
		i++;
	}
	nobs++;
	objc_adopt();
}

void objc_paste(OBJECT *dialog, int parent, int ox, int oy) {
int child;
short i, j, k;
OBJECT *baby=dialog, *nappy;
TEDINFO *ted;
	if(parent>=0) {
		i=dialog[parent].ob_tail; if(i<0) i=parent;
		if(dialog[i].ob_flags&LASTOB) { 
			dialog[i].ob_flags&=~LASTOB;
			treecopy[copycount-1].ob.ob_flags|=LASTOB;
		}
		else {
			while(1) {
				if(baby->ob_next>i) baby->ob_next+=copycount;
				if(baby->ob_head>i) baby->ob_head+=copycount;
				if(baby->ob_tail>i) baby->ob_tail+=copycount;
				if(baby->ob_flags&LASTOB) break;
				baby++;
			}
		}
		k=i-treecopy[copycount-1].ob.ob_next+1;
		j=0; while(j<copycount) {
			treecopy[j].ob.ob_next+=k;
			if(treecopy[j].ob.ob_head>=0) treecopy[j].ob.ob_head+=k;
			if(treecopy[j].ob.ob_tail>=0) treecopy[j].ob.ob_tail+=k;
			j++;
		}
		baby=&dialog[i+1]; child=get_index(baby);
		if(child<nobs) {
			memmove(&baby[copycount], baby, (nobs-child)*sizeof(OBJECT));
			memmove(&rcs_index[child+copycount], &rcs_index[child],
				(nobs-child)*sizeof(INDEX));
		}
		if(i==parent) dialog[parent].ob_head=i+1;
		else dialog[i].ob_next=i+1;
		dialog[parent].ob_tail=obj_selected=i+1;
	}
	else child=obj_selected=nobs;
	nappy=baby;
	j=0; while(j<copycount) {
		*baby=treecopy[j].ob;
		rcs_index[child+j].treetype=0;
		strcpy(rcs_index[child+j].name, treecopy[j].name);
		switch(baby->ob_type) {
			case R_TEXT:
			case R_TITLE:
				strcpy(&messages[messlength], baby->ob_spec);
				baby->ob_spec=&messages[messlength];
				messlength+=strlen(&messages[messlength])+1;
				break;
			case R_FTEXT:
			case R_ETEXT:
			case R_STEXT:
				ted=&treecopy[j].ted;
				strcpy(&messages[messlength], ted->te_ptext);
				teds[nteds].te_ptext=&messages[messlength];
				messlength+=strlen(ted->te_ptext)+1;
				strcpy(&messages[messlength], ted->te_ptmplt);
				teds[nteds].te_ptmplt=&messages[messlength];
				messlength+=strlen(teds[nteds].te_ptmplt)+1;
				teds[nteds].te_pvalid=ted->te_pvalid;
				teds[nteds].te_ptxtlen=ted->te_ptxtlen;
				(TEDINFO *)baby->ob_spec=&teds[nteds];
				nteds++;
		}
		j++; baby++;
	}
	nappy->ob_x=ox; nappy->ob_y=oy;
	nappy->ob_next=parent;
	i=0; while(i<ntrees) {
		if(trees[i]>=nappy) trees[i]+=copycount;
		i++;
	}
	nobs+=copycount;
}

void add_tree(int type) {
int i;
OBJECT *baby=&dials[nobs], *copymenu=menusamp;
	trees[ntrees]=baby;
	memset(&rcs_index[nobs], 0, 2);
	if(type==M_Menu) {
		rcs_index[nobs].treetype=1;
		sprintf(treenames[ntrees], "Menu%d", ntrees);
		i=0; while(i<10) {
			*baby=*copymenu;
			if(baby->ob_type==R_TITLE||baby->ob_type==R_TEXT) {
				baby->ob_spec=&messages[messlength];
				strcpy(&messages[messlength], copymenu->ob_spec);
				messlength+=strlen(copymenu->ob_spec)+1;
			}
			nobs++; i++;
			memset(&rcs_index[nobs], 0, 2);
			baby++; copymenu++;
		}
	}
	else {
		rcs_index[nobs++].treetype=2;
		sprintf(treenames[ntrees], "Form%d", ntrees);
		memset(baby, 0, sizeof(OBJECT));
		baby->ob_type=R_BLOCK;
		obj_selected=baby->ob_head=baby->ob_tail=baby->ob_next=-1;
		baby->ob_flags=OUTLINED|LASTOB;
		baby->ob_width=baby->ob_height=160;
		baby->ob_colbyte=16; baby->ob_border=1;
	}
	show_tree(ntrees++);
	strcpy(treenames[ntrees], "New");
	show_tree(ntrees);
}

void del_tree() {
	/*dialog=trees[i];*/
   objc_delete(0, 0);
	ntrees--;
   memcpy(&trees[currtree], &trees[currtree+1], (ntrees-currtree)*4);
   memcpy(treenames[currtree], treenames[currtree+1],
      (ntrees-currtree)*13);
	show_trees();
}

void paste_tree() {
short junk=nobs;
	trees[ntrees]=&dials[nobs];
	objc_paste(&dials[nobs], -1, 0, 0);
	rcs_index[junk].treetype= 1+(treecopy[0].ob.ob_width>0);
	if(rcs_index[junk].treetype==1)
		sprintf(treenames[ntrees], "Menu%d", ntrees);
	else sprintf(treenames[ntrees], "Form%d", ntrees);
	show_tree(ntrees++);
	strcpy(treenames[ntrees], "New");
	show_tree(ntrees);
}

void do_extra() {
short i, xinfo, yinfo, winfo, hinfo, x, y, w, h, type, ok=0;
int parent;
OBJECT *currobj=&dialog[obj_selected];
	form_show(extrinfo, &xinfo, &yinfo, &winfo, &hinfo);
	x=currobj->ob_x; y=currobj->ob_y;
	w=currobj->ob_width; h=currobj->ob_height;
	sprintf(((TEDINFO *)extrinfo[Ex_TrInd].ob_spec)->te_ptext,
		"%3d", obj_selected);
	sprintf(((TEDINFO *)extrinfo[Ex_Parent].ob_spec)->te_ptext,
		"%3d", parent=objc_parent(obj_selected));
	sprintf(((TEDINFO *)extrinfo[Ex_Prev].ob_spec)->te_ptext,
		"%3d", objc_prev(obj_selected));
	sprintf(((TEDINFO *)extrinfo[Ex_Next].ob_spec)->te_ptext,
		"%3d", currobj->ob_next);
	sprintf(((TEDINFO *)extrinfo[Ex_First].ob_spec)->te_ptext,
		"%3d", currobj->ob_head);
	sprintf(((TEDINFO *)extrinfo[Ex_Last].ob_spec)->te_ptext,
		"%3d", currobj->ob_tail);
	sprintf(((TEDINFO *)extrinfo[Ex_ChNo].ob_spec)->te_ptext,
		"%3d", objc_childno(parent, obj_selected));
	sprintf(((TEDINFO *)extrinfo[Ex_NoCh].ob_spec)->te_ptext,
		"%3d", objc_children(obj_selected));
	if(currobj->ob_flags&LASTOB)
		extrinfo[Ex_LastOb].ob_flags&=~HIDETREE;
	else extrinfo[Ex_LastOb].ob_flags|=HIDETREE;
	extrinfo[Ex_BlockBox].ob_flags|=(HIDETREE|DISABLED);
	extrinfo[Ex_TextBox].ob_flags|=(HIDETREE|DISABLED);
	extrinfo[Ex_FTextBox].ob_flags|=(HIDETREE|DISABLED);
	type=currobj->ob_type;
	if(type!=R_CHAR) extrinfo[Ex_CharBox].ob_flags|=(HIDETREE|DISABLED);
	else {
		extrinfo[Ex_CharBox].ob_flags&=~(HIDETREE|DISABLED);
		extrinfo[Ex_Char].ob_flags|=SELECTED;
	}
	while(1) {
		switch(type) {
			case R_BLOCK:
			case R_IBOX:
				extrinfo[Ex_BlockBox].ob_flags&=~(HIDETREE|DISABLED);
				extrinfo[Ex_Block+(type==R_IBOX)].ob_flags|=SELECTED;
				extrinfo[Ex_Block+(type!=R_IBOX)].ob_flags&=~SELECTED;
				break;
			case R_TEXT:
			case R_TITLE:
				extrinfo[Ex_TextBox].ob_flags&=~(HIDETREE|DISABLED);
				extrinfo[Ex_Text+(type==R_TITLE)].ob_flags|=SELECTED;
				extrinfo[Ex_Text+(type!=R_TITLE)].ob_flags&=~SELECTED;
				break;
			case R_FTEXT:
			case R_ETEXT:
			case R_STEXT:
				extrinfo[Ex_FTextBox].ob_flags&=~(HIDETREE|DISABLED);
				i=0; while(i<3) {
					if((i+R_FTEXT)==type) {
						extrinfo[Ex_FText+i].ob_flags|=SELECTED;
					}
					else {
						extrinfo[Ex_FText+i].ob_flags&=~SELECTED;
					}
					i++;
				}
		}
		sprintf(((TEDINFO *)extrinfo[Ex_ObX].ob_spec)->te_ptext,
			"%4d", x);
		sprintf(((TEDINFO *)extrinfo[Ex_ObY].ob_spec)->te_ptext,
			"%4d", y);
		sprintf(((TEDINFO *)extrinfo[Ex_ObW].ob_spec)->te_ptext,
			"%4d", w);
		sprintf(((TEDINFO *)extrinfo[Ex_ObH].ob_spec)->te_ptext,
			"%4d", h);
		do_form(extrinfo, xinfo, yinfo, winfo, hinfo);
		x=atoi(((TEDINFO *)extrinfo[Ex_ObX].ob_spec)->te_ptext);
		y=atoi(((TEDINFO *)extrinfo[Ex_ObY].ob_spec)->te_ptext);
		w=atoi(((TEDINFO *)extrinfo[Ex_ObW].ob_spec)->te_ptext);
		h=atoi(((TEDINFO *)extrinfo[Ex_ObH].ob_spec)->te_ptext);
		if(button==Ex_OK) {
			currobj->ob_x=x; currobj->ob_y=y;
			currobj->ob_width=w; currobj->ob_height=h;
			currobj->ob_type=type; ok=1; break;
		}
		else if(button==Ex_Cancel) break;
		else if(button==Ex_Block) {
         if(type==R_IBOX) {
            currobj->ob_type=R_BLOCK; draw_menu();
         } 
         type=R_BLOCK;
      }
		else if(button==Ex_IBox) {
         if(type==R_BLOCK) {
            currobj->ob_type=R_IBOX; draw_menu();
         }
         type=R_IBOX;
      }
		else if(button==Ex_FText) type=R_FTEXT;
		else if(button==Ex_EText) type=R_ETEXT;
		else if(button==Ex_SText) type=R_STEXT;
	}
	form_close(xinfo, yinfo, winfo, hinfo);
	currobj->ob_type=type;
	if(is_menu&&currobj->ob_type==R_BLOCK) {
		i=currobj->ob_head;
		while(i!=obj_selected) {
			dialog[i].ob_width=currobj->ob_width;
			i=dialog[i].ob_next;
		}
	}
	if(ok) objc_draw(dialog, (is_menu)?obj_selected:0, 8, 0, 0, 0, 0);
}

int tweak_menu() {
short d, e, f=0;
	dialog[2].ob_x=0; /*I don't know why my default was 16*/
	d=dialog[2].ob_head;
	e=dialog[dialog->ob_tail].ob_head; while(d!=2) {
		dialog[d].ob_x=f; dialog[e].ob_x=f+1;
		dialog[d].ob_width=strlen(dialog[d].ob_spec)*8+8;
		f+=dialog[d].ob_width;
		d=dialog[d].ob_next; e=dialog[e].ob_next;
	}
	dialog[2].ob_width=f;
	return(f);
}

int yx_compare(short *first, short *second) {
	if(dialog[*first].ob_y<dialog[*second].ob_y) return(-1);
	if(dialog[*first].ob_y>dialog[*second].ob_y) return(1);
	if(dialog[*first].ob_x<dialog[*second].ob_x) return(-1);
	if(dialog[*first].ob_x>dialog[*second].ob_x) return(1);
	return(0);
}

int xy_compare(short *first, short *second) {
	if(dialog[*first].ob_x<dialog[*second].ob_x) return(-1);
	if(dialog[*first].ob_x>dialog[*second].ob_x) return(1);
	if(dialog[*first].ob_y<dialog[*second].ob_y) return(-1);
	if(dialog[*first].ob_y>dialog[*second].ob_y) return(1);
	return(0);
}

int num_compare(short *first, short *second) {
	if(*first<*second) return(-1);
	if(*first>*second) return(1);
	return(0);
}

void do_sort() {
short i, j, k, m, xchar, ychar, wchar, hchar, result=0,
	sortindex[400], numindex[400];
OBJECT temp;
char tempname[14];
	form_show(srtform, &xchar, &ychar, &wchar, &hchar);
	while(1) {
		if(!result) {
			srtform[S_First1].ob_flags|=SELECTED;
			srtform[S_Second2].ob_flags|=SELECTED;
			srtform[S_First2].ob_flags&=~SELECTED;
			srtform[S_Second1].ob_flags&=~SELECTED;
		}
		else {
			srtform[S_First2].ob_flags|=SELECTED;
			srtform[S_Second1].ob_flags|=SELECTED;
			srtform[S_First1].ob_flags&=~SELECTED;
			srtform[S_Second2].ob_flags&=~SELECTED;
		}
		do_form(srtform, xchar, ychar, wchar, hchar);
		if(button==S_Cancel) { result=-1; break; }
		if(button==S_OK) break;
		result=button-S_First1;
	}
	form_close(xchar, ychar, wchar, hchar);
	if(result<0) return;
	memset(numindex, 0, 800);
	memset(sortindex, 0, 800);
	sortindex[0]=dialog[obj_selected].ob_head;
	i=1; while(1) {
		sortindex[i]=dialog[sortindex[i-1]].ob_next;
		if(sortindex[i]==obj_selected) break;
		i++;
	}
	memcpy(numindex, sortindex, 2*i+2);
	qsort(sortindex, i, 2, (void *)((result)?xy_compare:yx_compare));
	qsort(numindex, i, 2, (void *)(num_compare));
	dialog[obj_selected].ob_head=numindex[0];
	dialog[obj_selected].ob_tail=numindex[i-1];
	k=0; while(k<i) {
		m=sortindex[k]; j=numindex[k];
		if(j!=m) {
			temp=dialog[j];
			strcpy(tempname, rcs_index[get_index(&dialog[j])].name);
			strcpy(rcs_index[get_index(&dialog[j])].name,
				rcs_index[get_index(&dialog[m])].name);
			dialog[j]=dialog[m];
			if(dialog[j].ob_flags&LASTOB) {
				dialog[j].ob_flags&=~LASTOB; temp.ob_flags|=LASTOB;
			}
			dialog[m]=temp;
			strcpy(rcs_index[get_index(&dialog[m])].name, tempname);
			qsort(sortindex, i, 2, (void *)((result)?xy_compare:yx_compare));
		}
		k++; dialog[j].ob_next=numindex[k];
	}
}

void do_chartmp() {
short xchar, ychar, wchar, hchar;
	form_show(chartmp, &xchar, &ychar, &wchar, &hchar);
	((TEDINFO *)chartmp[Ch_Name].ob_spec)->te_ptext=
		rcs_index[get_index(&dialog[obj_selected])].name;
	((TEDINFO *)chartmp[Ch_Char].ob_spec)->te_ptext=
		(char *)&(dialog[obj_selected].ob_spec);
	do_form(chartmp, xchar, ychar, wchar, hchar);
	form_close(xchar, ychar, wchar, hchar);
	objc_draw(dialog, obj_selected, 8, 0, 0, 0, 0);
}

void do_txttmp() {
short width, xtxt, ytxt, wtxt, htxt;
char *spec=dialog[obj_selected].ob_spec, mess[77];
	form_show(txttmp, &xtxt, &ytxt, &wtxt, &htxt);
	strcpy(mess, spec); del_message(spec);
	width=strlen(mess);
	((TEDINFO *)txttmp[Tx_Name].ob_spec)->te_ptext=
		rcs_index[get_index(&dialog[obj_selected])].name;
	((TEDINFO *)txttmp[Tx_Text].ob_spec)->te_ptext=mess;
	do_form(txttmp, xtxt, ytxt, wtxt, htxt);
	form_close(xtxt, ytxt, wtxt, htxt);
	strcpy(&messages[messlength], mess);
	dialog[obj_selected].ob_spec=&messages[messlength];
	messlength+=strlen(mess)+1;
	if(is_menu&&dialog[obj_selected].ob_type==R_TITLE) {
		tweak_menu();
		objc_draw(dialog, 2, 8, 0, 0, 0, 0);
	}
	else {
	  if(autosize)
		  dialog[obj_selected].ob_width+=
			  (strlen(mess)-width)*(8-(objfont&2));
	  objc_draw(dialog, obj_selected, 8, 0, 0, 0, 0);
	}
}

void do_etxttmp() {
short i, width, xetxt, yetxt, wetxt, hetxt;
TEDINFO *ted=(TEDINFO *)dialog[obj_selected].ob_spec;
char mess1[77], mess2[77];
	strcpy(mess1, ted->te_ptext); strcpy(mess2, ted->te_ptmplt);
	width=strlen(mess2);
	i=0; while(mess1[i]) {
		if(mess1[i]=='_') mess1[i]='~';
		i++;
	}
	i=0; while(mess2[i]) {
		if(mess2[i]=='_') mess2[i]='~';
		i++;
	}
	del_message(ted->te_ptext); del_message(ted->te_ptmplt);
	if(dialog[obj_selected].ob_type==R_STEXT)
		etxttmp[ET_Length].ob_flags&=~HIDETREE;
	else etxttmp[ET_Length].ob_flags|=HIDETREE;
	if(dialog[obj_selected].ob_type!=R_FTEXT)
		etxttmp[ET_Valid].ob_flags&=~HIDETREE;
	else etxttmp[ET_Valid].ob_flags|=HIDETREE;
	form_show(etxttmp, &xetxt, &yetxt, &wetxt, &hetxt);
	((TEDINFO *)etxttmp[ET_Valid].ob_spec)->te_ptext=
		(char *)&ted->te_pvalid;
	((TEDINFO *)etxttmp[ET_Text].ob_spec)->te_ptext=mess1;
	((TEDINFO *)etxttmp[ET_Template].ob_spec)->te_ptext=mess2;
	sprintf(((TEDINFO *)etxttmp[ET_Length].ob_spec)->te_ptext, 
		"%3d", ted->te_ptxtlen);
	((TEDINFO *)etxttmp[ET_Name].ob_spec)->te_ptext=
		rcs_index[get_index(&dialog[obj_selected])].name;
	do_form(etxttmp, xetxt, yetxt, wetxt, hetxt);
	form_close(xetxt, yetxt, wetxt, hetxt);
	ted->te_ptxtlen=
		atoi(((TEDINFO *)etxttmp[ET_Length].ob_spec)->te_ptext);
	i=0; while(mess1[i]) {
		if(mess1[i]=='~') mess1[i]='_';
		i++;
	}
	i=0; while(mess2[i]) {
		if(mess2[i]=='~') mess2[i]='_';
		i++;
	}
	strcpy(&messages[messlength], mess1);
	ted->te_ptext=&messages[messlength]; messlength+=strlen(mess1)+1;
	strcpy(&messages[messlength], mess2);
	ted->te_ptmplt=&messages[messlength]; messlength+=strlen(mess2)+1;
	if(autosize)
		dialog[obj_selected].ob_width+=
			(strlen(mess2)-width)*(8-(objfont&2));
	objc_draw(dialog, obj_selected, 8, 0, 0, 0, 0);
}

int count_obs() {
int i;
   if(currtree<ntrees-1) i=get_index(trees[currtree+1]);
   else i=nobs;
   return(i-get_index(dialog));
}

void do_fndname() {
short i, j, k, xfind, yfind, wfind, hfind;
char temp[32];
   temp[0]=0;
	form_show(fndname, &xfind, &yfind, &wfind, &hfind);
   ((TEDINFO *)fndname[FN_Name].ob_spec)->te_ptext=temp;
	do_form(fndname, xfind, yfind, wfind, hfind);
	form_close(xfind, yfind, wfind, hfind);
	if(button==FN_OK) {
      j=count_obs(); k=get_index(dialog);
		i=0; while(i<j) {
			if(!strcmp(temp, rcs_index[i+k].name)) break;
			i++;
		}
		if(i<j) { re_store(); show_selected(i); }
		else {
         strcat(temp, " not found");
         form_error_text(temp);
		}
	}
}

void do_fndtxt() {
short i, j, k, xfind, yfind, wfind, hfind;
char temp[80];
OBJECT *locdial=dialog;
	temp[0]=0;
	((TEDINFO *)fndtxt[FT_Text].ob_spec)->te_ptext=temp;
	form_show(fndtxt, &xfind, &yfind, &wfind, &hfind);
	do_form(fndtxt, xfind, yfind, wfind, hfind);
	form_close(xfind, yfind, wfind, hfind);
	if(button==FT_OK) {
      k=count_obs();
		j=0; while(j<k) {
			i=locdial->ob_type;
			if(i==R_TEXT||i==R_TITLE) {
				if(strstr(locdial->ob_spec, temp)) break;
			}
			else if(i>=R_FTEXT) {
				if(strstr(((TEDINFO *)locdial->ob_spec)->te_ptext, temp)||
					strstr(((TEDINFO *)locdial->ob_spec)->te_ptmplt, temp))
					break;
			}
			j++; locdial++;
		}
      if(j==k) {
         strcat(temp, "|not found");
         form_error_text(temp); return;
      }
		else { re_store(); show_selected(j); }
	}
}

void do_fndobj() {
short i, j, xfind, yfind, wfind, hfind;
char temp[32];
	form_show(fndobj, &xfind, &yfind, &wfind, &hfind);
	do_form(fndobj, xfind, yfind, wfind, hfind);
	j=atoi(((TEDINFO *)fndobj[FONo_No].ob_spec)->te_ptext);
	form_close(xfind, yfind, wfind, hfind);
	if(button==FONo_OK) {
      i=count_obs()-1; if(i<j) {
			sprintf(temp, "Last object is %d", i);
         form_error_text(temp);
         j=i;
      }
		re_store(); show_selected(j);
	}
}

void do_treename(int i, int skip) {
short xtree, ytree, wtree, htree;
char spare[20];
	dialog=trees[currtree=i];
	if(skip==2) button=Tr_Edit;
	else {
		form_show(trname, &xtree, &ytree, &wtree, &htree);
      menu_ienable(trname, Tr_Move, i<ntrees-1);
		((TEDINFO *)trname[Tr_Name].ob_spec)->te_ptext=treenames[i];
		do_form(trname, xtree, ytree, wtree, htree);
		form_close(xtree, ytree, wtree, htree);
	}
	if(button==Tr_Edit) {
		work[W_Title].ob_spec=treenames[i];
		objc_draw(work, 0, 8, 0, 12, 0, 0);
		if(rcs_index[get_index(dialog)].treetype==1) {
			is_menu=1;
			dialog->ob_width=dialog[1].ob_width=
				dialog[dialog->ob_tail].ob_width=636;
			objc_draw(dialog, 0, 1, 2, 30, 0, 0);
			objc_draw(dialog, 1, 8, 0, 0, 0, 0);
		}
		else { is_menu=0; objc_draw(dialog, 0, 8, 2, 30, 0, 0); }
		boxa=0; obj_selected=-1;
		tree_open=1; draw_menu();
	}
	else if(button==Tr_Delete) del_tree();
	else if(button==Tr_Copy) {
		copycount=copylength=0; objc_copy(dialog, -1, -1, 0);
	}
	else if(button==Tr_Move) {
		strcpy(spare, treenames[i]);
		copycount=copylength=0; objc_copy(dialog, -1, -1, 0);
		del_tree(); paste_tree();
		strcpy(treenames[ntrees-1], spare);
		show_tree(ntrees-1);
	}
}

void rsr_test() {
short xtest, ytest, wtest, htest, msgbuff[8];
char message[60];
INDEX *testing;
	testing=&rcs_index[get_index(dialog)];
	if(obj_selected>=0) dialog[obj_selected].ob_flags&=~SELECTED;
	if(is_menu) {
		dialog->ob_x=dialog->ob_y=0;
		dialog->ob_width=dialog[1].ob_width=
			dialog[dialog->ob_tail].ob_width=-768;
		menu_bar(menu, 0); menu_bar(dialog, 1);
		while(1) {
			evnt_mesag(msgbuff);
			if(msgbuff[0]==MN_SELECTED) {
				menu_tnormal(dialog, msgbuff[3], 1);
				sprintf(message, "[2][Menu %d: %s|Item %d: %s",
					msgbuff[3], dialog[msgbuff[3]].ob_spec,
					msgbuff[4], dialog[msgbuff[4]].ob_spec);
			if(testing[msgbuff[4]].name[0])
				sprintf(message, "%s|Name %s", message,
					testing[msgbuff[4]].name);
			strcat(message, "][Exit|Test]");
				if(form_alert(2, message)==1) break;
			}
		}
		menu_bar(dialog, 0); menu_bar(menu, 1);
		dialog->ob_x=2; dialog->ob_y=30;
		dialog->ob_width=dialog[1].ob_width=
			dialog[dialog->ob_tail].ob_width=636;
	}
	else {
		form_show(dialog, &xtest, &ytest, &wtest, &htest);
		while(1) {
			do_form(dialog, xtest, ytest, wtest, htest);
			sprintf(message, "[2][Object %d", button);
			if(testing[button].name[0])
				sprintf(message, "%s|Name %s", message,
					testing[button].name);
			strcat(message, "][Exit|Test]");
			if(form_alert(2, message)==1) break;
		}
		form_close(xtest, ytest, wtest, htest);
		if(obj_selected>=0) dialog[obj_selected].ob_flags|=SELECTED;
		objc_draw(dialog, 0, 8, 2, 30, 0, 0);
	}
}

void flup(unsigned short *buffer, int length, int *index) {
int i;
	i=0; while(i<length/2) {
		if(((*index)&7)==4) numberend(buffer[i]);
		else numbertab(buffer[i]);
		i++; (*index)++;
	}
}

void rsr_save(int text) {
short i, j;
int k, length=0;
FILE *fp1;
OBJECT ob, *hob;
TEDINFO ted;
char *temp, hrdfile[FMSIZE], brackets[FNSIZE];
	if(is_menu) {
		dialog->ob_x=dialog->ob_y=0;
		dialog->ob_width=dialog[1].ob_width=
			dialog[dialog->ob_tail].ob_width=-768;
	}
	re_store();
	strcpy(hrdfile, rscdir);
	strcpy(&hrdfile[strlen(hrdfile)-3], "hrd");
	fp=fopen(hrdfile, "wb");
	j=1000; i=0; while(i<nobs) {
		if(rcs_index[i].treetype) {
			fwrite(&j, 2, 1, fp);
			fwrite(treenames[j-1000], 13, 1, fp);
			j++;
		}
		if(rcs_index[i].name[0]) {
			fwrite(&i, 2, 1, fp);
			fwrite(rcs_index[i].name, 13, 1, fp);
		}
		i++;
	}
	fclose(fp);
	hrdfile[strlen(hrdfile)-2]=0;
	fp=fopen(hrdfile, "wa");
	sprintf(brackets, "/*Headers for %s*/\n\n", selfile);
	fputs(brackets, fp);
	i=0; while(i<ntrees) {
		sprintf(brackets, "#define\t%s\t%d\n\n", treenames[i], i);
		fputs(brackets, fp);
		hob=trees[i]; j=0; while(1) {
			temp=rcs_index[get_index(hob)].name;
			if(temp[0]) {
				sprintf(brackets, "#define\t%s\t%d\n", temp, j);
				fputs(brackets, fp);
			}
			if(hob->ob_flags&LASTOB) break;
			j++; hob++;
		}
		i++; fputs("\n", fp);
	}
	fclose(fp);
	rshdr.rsh_tedinfo=messlength+(messlength&1);
	rshdr.rsh_object=rshdr.rsh_tedinfo+nteds*sizeof(TEDINFO);
	rshdr.rsh_trindex=rshdr.rsh_object+nobs*sizeof(OBJECT);
	rshdr.rsh_rssize=rshdr.rsh_trindex+ntrees*4;
	rshdr.rsh_nteds=nteds; rshdr.rsh_nobs=nobs;
	rshdr.rsh_ntrees=ntrees;
	if(text) {
		hrdfile[strlen(hrdfile)-1]='x';
		fp=fopen(hrdfile, "wa");
		strcpy(brackets, "unsigned short rscbuff[] = { ");
		fwrite(&brackets, strlen(brackets), 1, fp);
		flup((short *)&rshdr, sizeof(RSHDR), &length);
		flup((short *)messages, rshdr.rsh_tedinfo, &length);
	}
	fp1=fopen(rscdir, "wb");
	fwrite(&rshdr, sizeof(RSHDR), 1, fp1);
	fwrite(messages, rshdr.rsh_tedinfo, 1, fp1);
	i=0; while(i<nteds) {
		ted=teds[i]; ted.te_ptext-=(int)messages;
		ted.te_ptmplt-=(int)messages;
		fwrite(&ted, sizeof(TEDINFO), 1, fp1);
		if(text) flup((short *)&ted, sizeof(TEDINFO), &length);
		i++;
	}
	i=0; while(i<nobs) {
		ob=dials[i]; k=ob.ob_type;
		if(k==R_ETEXT||k==R_FTEXT||k==R_STEXT)
			ob.ob_spec-=((int)teds-rshdr.rsh_tedinfo);
		if(k==R_TEXT||k==R_TITLE) ob.ob_spec-=(int)messages;
		fwrite(&ob, sizeof(OBJECT), 1, fp1);
		if(text) flup((short *)&ob, sizeof(OBJECT), &length);
		i++;
	}
	i=0; while(i<ntrees) {
		k=((int)trees[i])-(int)dials+rshdr.rsh_object;
		fwrite(&k, 4, 1, fp1);
		if(text) flup((short *)&k, 4, &length);
		i++;
	}
	fclose(fp1);
	if(text) {
		if((length&7)!=5) lineend();
		strcpy(brackets, "};");
		fwrite(&brackets, strlen(brackets), 1, fp);
		fclose(fp);
	}
	if(is_menu) {
		dialog->ob_x=2; dialog->ob_y=30;
		dialog->ob_width=dialog[1].ob_width=
			dialog[dialog->ob_tail].ob_width=636;
	}
}

int rsr_load() {
short j;
TEDINFO *ted;
OBJECT *dialog;
	if(!(fp=fopen(rscdir, "rb"))) {
		form_error_text("Can't open resource!");
		return(0);
	}
	rsc_loaded=-1;
	fread(&rshdr, sizeof(RSHDR), 1, fp);
	nteds=rshdr.rsh_nteds;

	nobs=rshdr.rsh_nobs;
	ntrees=rshdr.rsh_ntrees;
	fread(messages, messlength=rshdr.rsh_tedinfo, 1, fp);
	fread(teds, nteds, sizeof(TEDINFO), fp);
	fread(dials, nobs, sizeof(OBJECT), fp);
	fread(trees, ntrees, 4, fp);
	fclose(fp);
	ted=teds; j=0; while(j<rshdr.rsh_nteds) {
		(ted->te_ptext)+=(long)messages;
		(ted->te_ptmplt)+=(long)messages;
		ted++; j++;
	}
	dialog=dials; j=0; while(j<nobs) {
		if(dialog->ob_type>=R_FTEXT&&dialog->ob_type<=R_STEXT)
			dialog->ob_spec+=(long)teds-rshdr.rsh_tedinfo;
		else if(dialog->ob_type==R_TEXT||dialog->ob_type==R_TITLE)
			dialog->ob_spec+=(long)messages;
		memset(&rcs_index[j], 0, 2);
		if(dialog->ob_next<0)
			rcs_index[j].treetype=1+(dialog->ob_width>0);
		j++; dialog++;
	}
	j=0; while(j<ntrees)
		(long)(trees[j++])+=(long)dials-rshdr.rsh_object;
	return(1);
}

void hrd_load() {
short i, j;
char hrdfile[FMSIZE];
OBJECT *dialog;
	strcpy(hrdfile, rscdir);
	strcpy(&hrdfile[strlen(hrdfile)-3], "hrd");
	if((fp=fopen(hrdfile, "rb"))==NULL) {
		form_error_text("No hrd-file found");
		i=j=0; while(i<nobs) {
			dialog=&dials[i];
			if(rcs_index[i].treetype) {
				if(rcs_index[i].treetype==1)
					sprintf(treenames[j], "Menu%d", j);
				else sprintf(treenames[j], "Form%d", j);
				j++;
			}
			i++;
		}
	}
	else {
		while(!feof(fp)) {
			fread(&i, 2, 1, fp);
			if(i>999) fread(treenames[i-1000], 13, 1, fp);
			else fread(rcs_index[i].name, 13, 1, fp);
		}
		fclose(fp);
	}
	strcpy(treenames[ntrees], "New");
}

void drag_obj() {
short i, j, k, ox, oy, px, py, xdisp, ydisp, parent, outer, obj, pobj, nobj;
OBJECT *currobj=&dialog[obj_selected], *boxobj;
	parent=objc_parent(obj_selected);
	if(is_menu) boxobj=&dialog[outer=parent];
	else { boxobj=dialog; outer=0; }
	objc_offset(dialog, obj_selected, &ox, &oy);
	objc_offset(dialog, outer, &px, &py);
	graf_dragbox(currobj->ob_width, currobj->ob_height,
		ox, oy, px, py, boxobj->ob_width, boxobj->ob_height, &mx, &my);
	if(ox!=mx||oy!=my) {
		if(is_menu) {
			if(currobj->ob_type==R_TEXT) {
				my-=py; oy=currobj->ob_y;
				i=boxobj->ob_head; while(1) {
					if(my>=dialog[i].ob_y&&my<dialog[i].ob_y+dialog[i].ob_height)
						break;
					i=dialog[i].ob_next;
				}
				if(i==obj_selected) return;
				py=dialog[i].ob_y;
				if(oy>py) {
					j=boxobj->ob_head; while(j!=outer) {
						if(dialog[j].ob_y>=py&&dialog[j].ob_y<oy)
							dialog[j].ob_y+=currobj->ob_height;
						j=dialog[j].ob_next;
					}
				}
				else if(oy<py) {
					j=boxobj->ob_head; while(j!=outer) {
						if(dialog[j].ob_y<=py&&dialog[j].ob_y>oy)
							dialog[j].ob_y-=currobj->ob_height;
						j=dialog[j].ob_next;
					}
				}
				currobj->ob_y=py;
			}
			else if(currobj->ob_type==R_TITLE) {
				mx-=px; ox=currobj->ob_x;
				i=boxobj->ob_head; while(1) {
					if(mx>=dialog[i].ob_x&&mx<dialog[i].ob_x+dialog[i].ob_width)
						break;
					i=dialog[i].ob_next;
				}
				if(i==obj_selected) return;
				kill_box(); mx-=ox;
				j=boxof(i); k=boxof(obj_selected);
				if(mx>=0) {
					ox=dialog[i].ob_next;
					oy=boxof(ox);
					px=currobj->ob_next;
					py=boxof(px);
					if(boxobj->ob_head==obj_selected) {
						boxobj->ob_head=px;
						dialog[dialog->ob_tail].ob_head=py;
					}
					else {
						dialog[boxof(objc_prev(obj_selected))].ob_next=py;
						dialog[objc_prev(obj_selected)].ob_next=px;
					}
					dialog[i].ob_next=obj_selected;
					dialog[j].ob_next=k;
					currobj->ob_next=ox;
					dialog[k].ob_next=oy;
					if(dialog[2].ob_tail==i) {
						dialog[2].ob_tail=obj_selected;
						dialog[dialog->ob_tail].ob_tail=k;
					}
				}
				else {
					ox=objc_prev(i);
					oy=boxof(ox);
					px=objc_prev(obj_selected);
					py=boxof(px);
					if(boxobj->ob_tail==obj_selected) {
						boxobj->ob_tail=px;
						dialog[dialog->ob_tail].ob_tail=py;
					}
					else {
						dialog[py].ob_next=dialog[k].ob_next;
						dialog[px].ob_next=currobj->ob_next;
					}
					if(dialog[2].ob_head==i) {
						dialog[2].ob_head=obj_selected;
						dialog[dialog->ob_tail].ob_head=k;
					}
					else {
						dialog[ox].ob_next=obj_selected;
						dialog[oy].ob_next=k;
					}
					currobj->ob_next=i;
					dialog[k].ob_next=j;
				}
				tweak_menu();
			}
		}
		else {
			xdisp=ox-mx; ydisp=oy-my;
			obj=objc_find(dialog, 0, 8, mx, my);
			if(obj!=parent&&obj!=obj_selected) {
				if(dialog[obj].ob_type<=R_IBOX&&
					form_alert(2, "[3][Accept new parent?][Undo|Accept]")==2) {
					nobj=dialog[parent].ob_head; if(nobj==obj_selected) {
						if(currobj->ob_next==parent)
							dialog[parent].ob_head=dialog[parent].ob_tail=-1;
						else dialog[parent].ob_head=currobj->ob_next;
					}
					else while(1) {
						pobj=dialog[nobj].ob_next; if(pobj==obj_selected) {
							dialog[nobj].ob_next=currobj->ob_next; break;
						}
						nobj=pobj;
					}
					nobj=dialog[obj].ob_tail; if(nobj==-1)
						dialog[obj].ob_head=obj_selected;
					else dialog[nobj].ob_next=obj_selected;
					currobj->ob_next=obj; dialog[obj].ob_tail=obj_selected;
					objc_offset(dialog, obj, &xdisp, &ydisp);
					parent=obj;
				}
				else return;
			}
			objc_offset(dialog, parent, &px, &py);
			mx-=px; my-=py;
			if(autosnap) { mx=(mx+4)&~7; my=(my+4)&~7; }
			else if(autohalf) { mx=(mx+2)&~3; my=(my+2)&~3; }
			currobj->ob_x=mx; currobj->ob_y=my;
			objc_adopt();
		}
		objc_draw(dialog, outer, 8, 0, 0, 0, 0);
	}
}

void size_obj() {
short zx, zy, xdisp, ydisp;
int parent;
OBJECT *currobj=&dialog[obj_selected];
	objc_offset(dialog, obj_selected, &zx, &zy);
	xdisp=mx-zx; ydisp=my-zy;
	graf_rubberbox(zx, zy, 12, 6, &xdisp, &ydisp);
	if(xdisp!=mx-zx||ydisp!=my-zy) {
		if(!obj_selected) {
			currobj->ob_width=xdisp+1;
			currobj->ob_height=ydisp+1;
		}
		else {
			parent=objc_parent(obj_selected);
			currobj->ob_width=
				min(dialog[parent].ob_width-currobj->ob_x, xdisp+1);
			currobj->ob_height=
				min(dialog[parent].ob_height-currobj->ob_y, ydisp+1);
		}
		if(is_menu) objc_draw(dialog, boxa, 8, 0, 0, 0, 0);
		else {
			objc_adopt();
			if(!obj_selected) objc_draw(work, 0, 8, 0, 12, 0, 0);
			objc_draw(dialog, 0, 8, 0, 0, 0, 0);
		}
	}
}

void narrow(short *curno, int x, int y) {
short i, try;
OBJECT *object;
	object=&dialog[*curno];
	if(x<0||y<0||x>=object->ob_width||y>=object->ob_height
		||object->ob_flags&HIDETREE) {
		*curno=-1; return;
	}
	if((i=object->ob_head)==-1)
		return;
	while(i!=*curno) {
		try=i;
		narrow(&try, x-dialog[i].ob_x, y-dialog[i].ob_y);
		if(try!=-1) { *curno=try; break; }
		i=dialog[i].ob_next;
	}
}

int obfind(int obj) {
/*	Like MAD objc_find but catches DISABLED objects*/
short gem=obj, ox, oy;
	objc_offset(dialog, obj, &ox, &oy);
	narrow(&gem, mx-ox, my-oy);
	return((int)gem);
}

int in_box() {
	if(obj_selected<0||dialog[obj_selected].ob_type>R_IBOX) {
		form_error_text("Must click in a box");
		return(0);
	}
	return(1);
}


int get_multi() {
short i, j, result, msgbuff[8], px, py, junk, butchange=0;
char message[100];
	result=evnt_multi(MU_MESAG|MU_BUTTON, 1, 3, 3, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, msgbuff, 4, 0, &mx, &my,
		&button, &junk, &junk, &junk);
	if(result&MU_MESAG) {
		evnt_button(1, 1, 0, &junk, &junk, &junk, &junk);
		if(msgbuff[0]==MN_SELECTED) {
			menu_tnormal(menu, msgbuff[3], 1);
			if(msgbuff[4]>=B0&&msgbuff[4]<B0+16) {
				bcolor=msgbuff[4]-B0;
				butchange=1;
			}
			if(msgbuff[4]>=T0&&msgbuff[4]<T0+16) {
				tcolor=msgbuff[4]-T0;
				butchange=1;
			}
			if(msgbuff[4]>=F0&&msgbuff[4]<F0+16) {
				fcolor=msgbuff[4]-F0; butchange=1;
			}
			if(msgbuff[4]>=M_Block&&msgbuff[4]<=M_SText) {
				addobj=msgbuff[4]; graf_mouse(POINT_HAND, 0l);
			}
			else switch(msgbuff[4]) {
				case M_About:
					if(!rsc_loaded) form_error_text(version);
               else {
                  sprintf(message, "No of Trees %d|No of Objects %d|No of Tedinfos %d|Text Length %d",
                     ntrees, nobs, nteds, messlength);
                  form_error_text(message);
               }
					break;
				case M_Quit: return('q');
				case M_New: strcpy(selfile, "new.rsr");
					messlength=nteds=nobs=ntrees=tree_open=0; rsc_loaded=1;
					draw_menu();
					work[W_Title].ob_spec=selfile;
					objc_draw(work, 0, 8, 0, 12, 0, 0); break;
				case M_Load: strcpy(selfile, "*.rsr");
					fsel_exinput(rscdir, selfile, &result, "Load a RSR");
					if(!result) break;
					strcat(rscdir, selfile);
				case M_Reload:
					if((rsc_loaded=rsr_load())) {
						hrd_load(); show_trees(); draw_menu();
					}
					break;
				case M_SaveAs:
					fsel_exinput(rscdir, selfile, &result, "Save RSR");
					if(!result) break;
					strcat(rscdir, selfile);
					if(!tree_open) objc_draw(work, W_Title, 8, 0, 0, 0, 0);
				case M_Save: rsr_save(0); break;
				case M_SaveC: rsr_save(1); break;
				case M_Menu:
				case M_Form: add_tree(msgbuff[4]); break;
				case M_AutoSize: autosize^=1; draw_menu(); break;
				case M_AutoSnap: autosnap^=1; autohalf=0; draw_menu(); break;
				case M_AutoHalf: autosnap=0; autohalf^=1; draw_menu(); break;
				case M_Selectable: dialog[obj_selected].ob_flags^=EXIT;
					draw_menu(); break;
				case M_HideTree: dialog[obj_selected].ob_flags|=HIDETREE;
					re_store(); show_selected(objc_parent(obj_selected));
					break;
				case M_Disabled: dialog[obj_selected].ob_flags^=DISABLED;
				butchange=1; break;
				case M_Default: dialog[obj_selected].ob_flags^=DEFAULT;
				butchange=1; break;
				case M_Outlined: dialog[obj_selected].ob_flags^=OUTLINED;
					dialog[obj_selected].ob_border=1;
				butchange=1; break;
				case M_Checked: dialog[obj_selected].ob_flags^=CHECKED;
					butchange=1; break;
				case M_Wrap:
					dialog[obj_selected].ob_border^=255;
					butchange=1; break;
				case M_Normal: if(objfont) { objfont=0; butchange=1; }
					break;
				case M_Large:
					if(objfont!=1) objfont=1;
					else objfont=0;
					butchange=1; break;
				case M_Small:
					if(objfont<2) objfont=2;
					else objfont=0;
					butchange=1; break;
				case M_NameFind: do_fndname(); break;
				case M_SelectNo: do_fndobj(); break;
				case M_TextFind: do_fndtxt(); break;
				case M_Extras: do_extra(); break;
				case M_Test: rsr_test(); break;
				case M_Delete:
					if(is_menu) {
						if(dialog[obj_selected].ob_type==R_TITLE) {
							kill_box();
							objc_delete(boxof(obj_selected), 0);
							objc_delete(obj_selected, 1); obj_selected=-1;
							tweak_menu();
							objc_draw(dialog, 0, 1, 0, 0, 0, 0);
							objc_draw(dialog, 2, 8, 0, 0, 0, 0);
						}
						else if(dialog[obj_selected].ob_type==R_TEXT) {
							j=dialog[obj_selected].ob_y;
							objc_delete(obj_selected, 1); obj_selected=-1;
							i=dialog[boxa].ob_head; while(i!=boxa) {
								if(dialog[i].ob_y>j)
									dialog[i].ob_y-=8;
								i=dialog[i].ob_next;
							}
							/*i=boxa; */kill_box(); boxa=i;
							dialog[boxa].ob_height-=8;
							objc_draw(dialog, boxa, 8, 0, 0, 0, 0);
						}
					}
					else if(obj_selected) {
						i=objc_parent(obj_selected);
						objc_delete(obj_selected, 1); obj_selected=i;
						objc_draw(dialog, obj_selected, 8, 0, 0, 0, 0);
					}
					draw_menu();
					break;
				case M_UndoHide:
					i=dialog[obj_selected].ob_head;
					if(i>=0) while(i!=obj_selected) {
						dialog[i].ob_flags&=~HIDETREE;
						i=dialog[i].ob_next;
					}
					objc_draw(dialog, obj_selected, 8, 0, 0, 0, 0);
					break;
				case M_Copy: copycount=copylength=0;
					objc_copy(dialog, dialog[obj_selected].ob_next, -1,
						obj_selected);
					treecopy[0].ob.ob_flags&=~SELECTED;
					draw_menu();
					break;
				case M_Cut: copycount=copylength=0;
					objc_copy(dialog, dialog[obj_selected].ob_next, -1,
						obj_selected);
					junk=objc_parent(obj_selected);
					j=dialog[obj_selected].ob_y;
					objc_delete(obj_selected, 0); obj_selected=-1;
					if(boxa&&junk==boxa) {
						i=dialog[boxa].ob_head; while(i!=boxa) {
							if(dialog[i].ob_y>j)
								dialog[i].ob_y-=8;
							i=dialog[i].ob_next;
						}
                  kill_box(); boxa=i;
                  dialog[boxa].ob_height-=8;
					}
               objc_draw(dialog, junk, 8, 0, 0, 0, 0);
					draw_menu();
					break;
				case M_Paste:
					pasteobj=1; graf_mouse(POINT_HAND, 0l); break;
				case M_Sort: do_sort(); break;
			}
		}
		if(butchange) {
			draw_menu(); re_store();
		}
	}
	else if(result&MU_BUTTON) {
		junk=objc_find(work, 0, 8, mx, my);
		if(junk==W_Closer) {
			if(!tree_open) return('q');
			if(is_menu) {
				dialog->ob_x=dialog->ob_y=boxa=0;
				dialog->ob_width=dialog[1].ob_width=
					dialog[dialog->ob_tail].ob_width=-768;
			}
			re_store(); obj_selected=-1;
			show_trees(); draw_menu();
		}
		else if(rsc_loaded&&!tree_open) {
		  	if(mx<600&&mx%100>19&&my>29) {
				junk=((my-30)/40)*6+mx/100;
				if(junk<ntrees) do_treename(junk, button);
				else {
					if(copycount) paste_tree();
				}
			}
		}
		else if(tree_open) {
			junk=obfind(0); if(junk<0) return(0);
			if(boxa) {
				i=obfind(boxa); if(i>0) junk=i;
				else if(junk!=1&&dialog[junk].ob_type!=R_TITLE)
					return(0);
			}
         if(junk) {
				vq_key_s(handle, &i);
				if(i&3) junk=objc_parent(junk);
			}
			if(is_menu) {
				if(boxa) {
					if(junk!=boxa&&objc_parent(junk)!=boxa) {
						if(objc_parent(obj_selected)==boxa||obj_selected==boxa) {
							dialog[obj_selected].ob_flags&=~SELECTED;
							obj_selected=-1;
						}
						kill_box();
					}
					else if((pasteobj&&treecopy[0].ob.ob_type==R_TEXT)
						||addobj==M_TextOb) {
						dialog[obj_selected].ob_flags&=!SELECTED;
						dialog[boxa].ob_height+=8;
						i=dialog[junk].ob_y;
						j=dialog[boxa].ob_head; while(j!=boxa) {
							if(dialog[j].ob_y>=i)
								dialog[j].ob_y+=8;
							j=dialog[j].ob_next;
						}
						if(addobj) add_obj(boxa, M_TextOb, 0, i);
						else {
                     treecopy[0].ob.ob_width=dialog[boxa].ob_width;
							objc_paste(dialog, boxa, 0, i);
						}
						objc_draw(dialog, boxa, 8, 0, 0, 0, 0);
						addobj=pasteobj=0; graf_mouse(ARROW, 0l);
					}
				}
				if(obj_selected>0) re_store();
				if(addobj|pasteobj) {
					if(junk==1&&addobj==M_Title) {
						i=tweak_menu(); dialog[2].ob_width+=56;
						add_obj(2, M_Title, i, 0);
						add_obj(dialog->ob_tail, M_Block, i+1, 0);
						add_obj(dialog[dialog->ob_tail].ob_tail, M_TextOb, 0, 0);
						junk=dialog[2].ob_tail;
						addobj=0; graf_mouse(ARROW, 0l);
					}
					else junk=-1;
				}
				if(dialog[junk].ob_type==R_TITLE) {
					if(boxa) kill_box();
					boxa=boxof(junk);
					show_selected(junk);
				}
			}
			if(junk<0) {
				if(obj_selected>=0) re_store();
				obj_selected=-1; addobj=pasteobj=0;
				graf_mouse(ARROW, 0l);
			}
			else {
				if(junk!=obj_selected) {
					re_store();
					show_selected(junk);
				}
				if(button==2) {
					switch(dialog[obj_selected].ob_type) {
						case R_BLOCK:
						case R_IBOX:
							i=get_index(&dialog[obj_selected]);
							form_input_text(12, "Name",
								rcs_index[i].name, "Box Name");
							break;
						case R_CHAR: do_chartmp(); break;
						case R_TITLE:
						case R_TEXT: do_txttmp(); break;
						case R_ETEXT:
						case R_FTEXT:
						case R_STEXT: do_etxttmp(); break;
					}
				}
				if((addobj|pasteobj)&&in_box()) {
					graf_mouse(ARROW, 0l);
					re_store();
					objc_offset(dialog, obj_selected, &px, &py);
               if(addobj) {
                  add_obj(obj_selected, addobj, mx-px, my-py); addobj=0;
               }
               else {
                  objc_paste(dialog, obj_selected, mx-px, my-py); pasteobj=0;
               }
					objc_draw(dialog, junk, 8, 0, 0, 0, 0);
					show_selected(obj_selected);
				}
				objc_offset(dialog, obj_selected, &px, &py);
				if(mx>px+dialog[obj_selected].ob_width-3&&
					my>py+dialog[obj_selected].ob_height-3)
					size_obj();
				if(obj_selected) drag_obj();
			}
		}
	}
	return(0);
}

void handle_menu() {
short i;
	draw_menu();
	while(1) {
		i=get_multi(); if(i=='q') return;
	}
}

int main(int argc, char *argv[]) {
	appl_init(1);
	rsrc_include(rscbuff);
	rsrc_gaddr(0, Menu, &menu);
	rsrc_gaddr(0, Workspace, &work);
	rsrc_gaddr(0, TreeName, &trname);
	rsrc_gaddr(0, ETextTemp, &etxttmp);
	rsrc_gaddr(0, TextTemp, &txttmp);
	rsrc_gaddr(0, CharTemp, &chartmp);
	rsrc_gaddr(0, FindName, &fndname);
	rsrc_gaddr(0, FindText, &fndtxt);
	rsrc_gaddr(0, FindIndex, &fndobj);
	rsrc_gaddr(0, SortForm, &srtform);
	rsrc_gaddr(0, MenuSamp, &menusamp);
	rsrc_gaddr(0, ExtraInfo, &extrinfo);
	if(argc==2) {
      strcpy(rscdir, argv[1]);
      strcpy(selfile, get_filename(rscdir));
		if((rsc_loaded=rsr_load())) {
			hrd_load(); show_trees();
			fsel_set_extn("rsr");
		}
	}
	handle_menu();
	return(appl_exit());
}
END_OF_MAIN();

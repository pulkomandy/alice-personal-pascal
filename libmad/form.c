#include	<stdlib.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"

#define	_AL_TYPE		1
#define	_AL_STRING1	2
#define	_AL_BUT1		7

#define	_IN_INPUT		 1
#define	_IN_ENTER	2
#define	_IN_TEXT	3

short	_curpos;
int	_rno=-1;
GRECT	_rxywh[32];
char *_rmallocs[32];
extern	int	handle;
extern	FDB	_screen;
extern	short	_mshape, _scrwid, _scrht;
extern	OBJECT	*_alertbox, *_inputbox;
extern	_MEVENT	_mevent;
extern	void	_obcurse(), _showmouse(), _hidemouse(),
	_narrow(), _mouse_event();
extern	GFONT	*_font;

char *_tpos(OBJECT *curredit, short *max_tlength) {
short i=0;
char *text;
	curredit->ob_flags|=SELECTED;
	if(curredit->ob_type==R_STEXT)
		*max_tlength=((TEDINFO *)curredit->ob_spec)->te_ptxtlen-1;
	else {
		text=((TEDINFO *)curredit->ob_spec)->te_ptmplt;
		while(*text) { if(*(text++)=='_') i++; }
		*max_tlength=i;
	}
	text=((TEDINFO *)curredit->ob_spec)->te_ptext;
	_curpos=strlen(text);
	return(text);
}

int _cursex(OBJECT *dialog, int editobj, int mx) {
short foo, bar;
char *temp;
	temp=((TEDINFO *)dialog[editobj].ob_spec)->te_ptmplt;
	objc_offset(dialog, editobj, &foo, &bar);
	bar=(mx-foo)/_font->max_cell_width;
	foo=0; while(temp[foo]!='_') {
		foo++; bar--;
	}
	return(bar);
}

int form_do(OBJECT *dialog, int editobj) {
/*The actual messing with the dialog (but not the drawing - that has to be 
done with objc_draw).
If the second parameter is non-zero, the text cursor is positioned in the 
text of the object with that number. Otherwise, the text cursor goes in the 
first editable object it can find.
Tab cycles through the editables. Backspace and delete both delete from the 
end of the text. Escape deletes the entire text.
Clicking on an object with EXIT in its flags or pressing return terminates 
this routine, and the return is either the number of the clicked object or 
that of the default object.*/
short key, max_tlength, curno=0, temp, mouse=0, oldshape=_mshape, dfault=-1,
	textx;
int result=-1;
OBJECT *curredit, *currover;
char *text, buff[256];
	curredit=0l; if(editobj) curredit=&dialog[editobj];
	else {
		temp=0; do {
			if(dialog[temp].ob_type==R_ETEXT||dialog[temp].ob_type==R_STEXT) {
				curredit=&dialog[editobj=temp];
				break;
			}
		} while(!(dialog[temp++].ob_flags&LASTOB));
	}
	if(editobj) {
		text=_tpos(curredit, &max_tlength);
		objc_draw(dialog, editobj, 1, 0, 0, 0, 0);
	}
	temp=0; do {
		if(dialog[temp].ob_flags&DEFAULT) { dfault=temp; break; }
	} while(!(dialog[temp++].ob_flags&LASTOB));
	currover=dialog;
	temp=0;
	_narrow(dialog, &temp, _mevent.x-_rxywh[_rno].g_x, _mevent.y-_rxywh[_rno].g_y);
	if(temp!=-1) {
		currover=&dialog[curno=temp];
		if(!mouse&&(currover->ob_type==R_ETEXT||currover->ob_type==R_STEXT)) {
			graf_mouse(mouse=TEXT_CRSR, 0l);
		}
		else if(mouse&&!(currover->ob_type==R_ETEXT||currover->ob_type==R_STEXT)) {
			graf_mouse(mouse=ARROW, 0l);
		}
	}
	while(result==-1) {
		_mouse_event();
		if(_mevent.kinds&M_KEYPRESS) {
			key=_mevent.key;
			if(key==CARRET) result=dfault;
			else if(editobj) {
				_obcurse(0);
				if(key==DELKEY||key==BCKKEY) {
					if(_curpos) {
						if(_curpos==strlen(text)) text[--_curpos]=0;
						else {
							strcpy(&text[_curpos-1], &text[_curpos]);
							_curpos--;
						}
					}
				}
				else if(key==TABKEY) {
					temp=editobj; while(1) {
						if(dialog[temp].ob_flags&LASTOB) temp=1;
						else temp++;
						if((dialog[temp].ob_type==R_ETEXT||
							dialog[temp].ob_type==R_STEXT)
							&&!(dialog[temp].ob_flags&HIDETREE))
							break;
					}
					if(temp!=editobj) {
						curredit->ob_flags&=~SELECTED;
						_obcurse(0); /*A02/09*/
						objc_draw(dialog, editobj, 1, 0, 0, 0, 0);
						curredit=&dialog[editobj=temp];
						text=_tpos(curredit, &max_tlength);
					}
				}
				else if(key==LAKEY) {
					if(_curpos) _curpos--;
				}
				else if(key==RAKEY) {
					if(_curpos<strlen(text)) _curpos++;
				}
				else if(key==ESCKEY) *text=_curpos=0;
				else if(key<256) {
					if(((TEDINFO *)curredit->ob_spec)->te_pvalid=='9'
						&&!(isdigit(key)||(!_curpos&&key=='-'))) goto printit;
					if(((TEDINFO *)curredit->ob_spec)->te_pvalid=='A'
						&&!isxdigit(key)) goto printit;
					if(_curpos<max_tlength) {
						if(_curpos==strlen(text)) {
							text[_curpos++]=key; text[_curpos]=0;
						}
						else {
							if(strlen(text)==max_tlength) text[strlen(text)-1]=0;
							strcpy(buff, &text[_curpos]);
							text[_curpos++]=key; strcpy(&text[_curpos], buff);
						}
					}
					else text[_curpos-1]=key;
				}
printit:		objc_draw(dialog, editobj, 1, 0, 0, 0, 0);
			}
		}
		if(_mevent.kinds&M_MOTION) {
			temp=0;
			_narrow(dialog, &temp, _mevent.x-_rxywh[_rno].g_x, _mevent.y-_rxywh[_rno].g_y);
				if(temp!=-1) {
					currover=&dialog[curno=temp];
					if(!mouse&&(currover->ob_type==R_ETEXT||currover->ob_type==R_STEXT)) {
						graf_mouse(mouse=TEXT_CRSR, 0l);
					}
					else if(mouse&&!(currover->ob_type==R_ETEXT||currover->ob_type==R_STEXT)) {
						graf_mouse(mouse=ARROW, 0l);
					}
				}
		}
		if(_mevent.kinds&(M_LEFT_DOWN|M_RIGHT_DOWN)) {
			if(currover->ob_flags&EXIT) result=curno;
			if(currover->ob_type==R_ETEXT||currover->ob_type==R_STEXT) {
				if(curredit==currover) {
					temp=_cursex(dialog, editobj, _mevent.x);
					if(temp!=_curpos) {
						_obcurse(0);
						_curpos=min(temp, strlen(text));
					}
				}
				else {
					curredit->ob_flags&=~SELECTED;
					_obcurse(0);
					objc_draw(dialog, editobj, 1, 0, 0, 0, 0); /*A02/09*/
					curredit=currover; editobj=curno;
					text=_tpos(curredit, &max_tlength);
					temp=_cursex(dialog, editobj, _mevent.x);
					if(temp<_curpos) _curpos=temp;
				}
				objc_draw(dialog, editobj, 1, 0, 0, 0, 0);
			}
		}
	}
	if(editobj) curredit->ob_flags&=~SELECTED;
	dialog[result].ob_flags|=SELECTED;
	graf_mouse(oldshape, 0l);
	return(result);
}

int form_dial(int fo_diflag, int fo_dilittx, int fo_dilitty, int fo_dilittw,
	int fo_dilitth, int dix, int diy, int diw, int dih) {
/*This prepares for drawing the form or undraws it.
If the first parameter is less than 2, a section of the screen with
coordinates given by the last four parameters (usually those set by
form_center) is saved. Otherwise, the previously saved screen is blitted
back in, erasing the form.*/
short px[8];
FDB temp;
	diw+=2; if(dix) dix--;
	dih+=2; if(diy) diy--;
	_hidemouse();
	if(fo_diflag<2) {
		temp.fd_addr=_rmallocs[++_rno]=(char *)malloc(diw*dih);
		_rxywh[_rno].g_x=dix; _rxywh[_rno].g_y=diy;
		temp.fd_w=_rxywh[_rno].g_w=diw;
		temp.fd_h=_rxywh[_rno].g_h=dih;
		px[0]=dix; px[1]=diy; px[2]=dix+diw-1; px[3]=diy+dih-1;
		px[4]=px[5]=0; px[6]=diw-1; px[7]=dih-1;
		vro_cpyfm(handle, S_ONLY, px, &_screen, &temp);
	}
	else {
		temp.fd_addr=_rmallocs[_rno];
		temp.fd_w=diw; temp.fd_h=dih;
		px[0]=px[1]=0; px[2]=diw-1; px[3]=dih-1;
		px[4]=dix; px[5]=diy; px[6]=dix+diw-1; px[7]=diy+dih-1;
		vro_cpyfm(handle, S_ONLY, px, &temp, &_screen);
		free(_rmallocs[_rno]); _rno--;
	}
	_showmouse();
	return(1);
}

int form_center(OBJECT *dialog, short *x, short *y, short *w, short *h) {
/*This does nothing but work out the x and y coordinates for the top left 
of an object that will center it on the screen, and pass them with the 
object's width and height to the parameters and to the dialog itself.
If I wanted off-center dialogs, I would enter the x and y myself.*/
	*w=dialog->ob_width; *h=dialog->ob_height;
	dialog->ob_x=*x=(_scrwid-*w)>>1; dialog->ob_y=*y=(_scrht-*h)>>1;
	return(1);
}

int form_alert(int dfault, const char *alert) {
/*This on its own calls up a small dialog with limited but useful
functionality, using up to 3 buttons and up to 5 lines of text. The int
is the number of the button which pressing return will simulate; this
will be shown as the default. The string is of the form
[number][text|text|...][button|button...], where number is the number 
of the title, text stands for a line of text and button stands for a 
button text. The return is the button, 1-3, actually pressed.*/
short i, buts=0, lines=0, alx, aly, alw, alh, butwidth, linewidth;
char buttexts[3][11], *j, *k;
	i=1; while(i<5) {
		_alertbox[_AL_STRING1+i].ob_flags|=HIDETREE;
		i++;
	}
	switch(alert[1]) {
		case '1': _alertbox[_AL_TYPE].ob_spec="What?"; break;
		case '2': _alertbox[_AL_TYPE].ob_spec="Hmm.."; break;
		default: _alertbox[_AL_TYPE].ob_spec="Stop!";
	}
	k=_alertbox[_AL_STRING1].ob_spec;
	j=(char *)&alert[4]; while(*j!=']') {
		if(*j=='|') {
			j++; lines++;
			*k=0; k=_alertbox[_AL_STRING1+lines].ob_spec;
			_alertbox[_AL_STRING1+lines].ob_flags&=~HIDETREE;
		}
		else *(k++)=*(j++);
	}
	j+=2; *k=0; k=buttexts[0];
	while(*j!=']') {
		if(*j=='|') {
			j++; buts++; *k=0; k=buttexts[buts];
		}
		else *(k++)=*(j++);
	}
	*k=0;
	butwidth=2; i=0; while(i<=buts) {
		_alertbox[_AL_BUT1+i].ob_flags&=~(HIDETREE|DEFAULT);
		_alertbox[_AL_BUT1+i].ob_spec=buttexts[i];
		_alertbox[_AL_BUT1+i].ob_width=(strlen(buttexts[i])<<3)+4;
		butwidth+=_alertbox[_AL_BUT1+i].ob_width+2;
		_alertbox[_AL_BUT1+i].ob_y=lines*12+40; i++;
	}
	while(i<3) _alertbox[_AL_BUT1+i++].ob_flags|=HIDETREE;
	if(dfault) _alertbox[_AL_BUT1+dfault-1].ob_flags|=DEFAULT;
	linewidth=max(butwidth, strlen(_alertbox[_AL_TYPE].ob_spec)*8+2);
	i=0; while(i<lines+1) {
		_alertbox[_AL_STRING1+i].ob_width=
			strlen(_alertbox[_AL_STRING1+i].ob_spec)*8;
		linewidth=max(linewidth, _alertbox[_AL_STRING1+i].ob_width);
		i++;
	}
	linewidth=max(56, linewidth+16);
	_alertbox->ob_width=linewidth;
	_alertbox->ob_height=lines*12+56;
	_alertbox[_AL_TYPE].ob_x=(linewidth-50)>>1;
	switch(buts) {
		case 0:
			_alertbox[_AL_BUT1].ob_x=((linewidth-butwidth)>>1)+1;
			break;
		case 1: _alertbox[_AL_BUT1].ob_x=6;
			_alertbox[_AL_BUT1+1].ob_x=linewidth-6-_alertbox[_AL_BUT1+1].ob_width;
				break;
		default: _alertbox[_AL_BUT1].ob_x=6;
			_alertbox[_AL_BUT1+1].ob_x=linewidth-(_alertbox[_AL_BUT1+1].ob_width>>1);
			_alertbox[_AL_BUT1+2].ob_x=linewidth-6-_alertbox[_AL_BUT1+2].ob_width;
	}
	form_center(_alertbox, &alx, &aly, &alw, &alh);
	form_dial(0,0,0,0,0, alx, aly, alw, alh);
	objc_draw(_alertbox, 0, 8, alx, aly, alw, alh);
	i=form_do(_alertbox, 0);
	_alertbox[i].ob_flags&=~SELECTED;
	form_dial(3,0,0,0,0, alx, aly, alw, alh);
	return(i-_AL_BUT1+1);
}

int form_error(int number) {
/*The ST form_error returned a report of violations of the underlying
operating system. I can't manage that so this is basically a debugging
device which shows a message with the parameter included and waits.*/
char temp[30];
	sprintf(temp, "[0][Error %d][OK]", number);
	form_alert(1, temp);
	return(1);
}

int form_error_text(char *text) {
/*Like form_error, but puts up a message instead of a number*/
char *temp=alloca(strlen(text)+10);
	sprintf(temp, "[0][%s][OK]", text);
	form_alert(1, temp);
	return(1);
}

int form_infobox(OBJECT *dialog) {
	/*Puts up a user-created dialog with only one exit button, mainly to 
present information to the user. The dialog can contain editable strings,
but reading them must be handled explicitly by the program*/
/*See example below*/
short dfault, inx, iny, inw, inh;
	form_center(dialog, &inx, &iny, &inw, &inh);
	form_dial(0,0,0,0,0, inx, iny, inw, inh);
	objc_draw(dialog, 0, 8, inx, iny, inw, inh);
	dfault=form_do(dialog, 0);
	form_dial(3,0,0,0,0, inx, iny, inw, inh);
	dialog[dfault].ob_flags&=~SELECTED;
	return(dfault);
}

void form_input_text(int length, char *ptmplt, char *input, char *label) {
	/*Returns with new input string. Input must be at least length +1 long.
	The label is optional.*/
int i, width;
char *template, mylabel[41];
	if(strlen(label)) strncpy(mylabel, label, 40);
	else strcpy(mylabel, "Input");
	template=alloca(strlen(ptmplt)+length+3);
	sprintf(template, "%s: ", ptmplt);
	i=0; while(i<length) {
		strcat(template, "_"); i++;
	}
	((TEDINFO *)_inputbox[_IN_INPUT].ob_spec)->te_ptext=input;
	((TEDINFO *)_inputbox[_IN_INPUT].ob_spec)->te_ptmplt=template;
	((TEDINFO *)_inputbox[_IN_INPUT].ob_spec)->te_pvalid='X';
	_inputbox[_IN_TEXT].ob_spec=mylabel;
	width=max(strlen(mylabel), strlen(template))*8+24;
	_inputbox[0].ob_width=width;
	_inputbox[_IN_INPUT].ob_width=strlen(template)*8;
	_inputbox[_IN_INPUT].ob_x=(width-_inputbox[_IN_INPUT].ob_width)/2;
	_inputbox[_IN_TEXT].ob_width=strlen(mylabel)*8;
	_inputbox[_IN_TEXT].ob_x=(width-_inputbox[_IN_TEXT].ob_width)/2;
	_inputbox[_IN_ENTER].ob_x=width/2-32;
	form_infobox(_inputbox);
}

int form_input(int length, char *ptmplt, int dfault, char *label) {
	/*Pops up a little dialog box in which you can type a number of length 
digits on a line starting with the template. The default is the default number 
given. The label is optional.
The return is the number you typed.*/
char ptext[length+1];
	sprintf(ptext, "%d", dfault);
	((TEDINFO *)_inputbox[_IN_INPUT].ob_spec)->te_pvalid='9';
	form_input_text(length+1, ptmplt, ptext, label);
	return(atoi(ptext));
}

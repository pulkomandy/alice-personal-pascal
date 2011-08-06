#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<memory.h>
#include	<time.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"

FDB	_boxfdb;
short	_boxpx[8];
extern	FDB	_screen;
extern	short	_scrwid, _boxstart;
extern	int	handle;
extern	OBJECT	*_menu;
extern	_MEVENT	_mevent;
extern	void	_mouse_event();

void vq_mouse(int handle, short *butn, short *x, short *y) {
	/*This returns immediately with the current state of the mouse button 
and the x and y positions of the cursor in its pointers.*/
   _mouse_event();
   *x=_mevent.x; *y=_mevent.y; *butn=_mevent.buttons&3;
}

void vq_key_s(int handle, short *mykey) {
   _mouse_event();
	*mykey=_mevent.kbstat;
}

void vq_keybd(int handle, short *key) {
	/*This returns with the current keypress or 0 in key. Note: ant keypress is 
no longer available after this routine.
Not an ST construct*/
   _mouse_event();
	*key=_mevent.key;
}

int evnt_keybd() {
	/*Waits for a keypress and returns it.*/
	while(1) {
		_mouse_event();
		if(_mevent.kinds&M_KEYPRESS) return(_mevent.key);
	}
}

int evnt_button(int bmaxclicks, int bmask, int bstate, short *x, short *y,
	short *butn, short *kstate) {
	/*Waits for the number of clicks set in the first parameter, and 
returns that number.
The next parameter is the buttons which will register as clicks; 1 is 
left, 2 is right, 3 is either. The third parameter sets whether button 
up (0) or button down (1, 2 or 3) is counted.
When the routine exits, the 4th and 5th parameters contain the x and y 
of the mouse position, the 6th the button states, in the same form as 
the second parameter, the last the key state as in vq_key_s.*/
int clicks=0, flags=0;
	if(bmask&1) flags|=(bstate&1) ? M_LEFT_DOWN : M_LEFT_UP;
	if(bmask&2) flags|=(bstate&2) ? M_RIGHT_DOWN : M_RIGHT_UP;
	while(clicks<bmaxclicks) {
		_mouse_event();
		if(flags&_mevent.kinds) clicks++;
	}
	*x=_mevent.x; *y=_mevent.y;
	*butn=_mevent.buttons; *kstate=_mevent.kbstat;
	return(clicks);
}

void _restore_box(short *tsel, short *bsel, short *ssel) {
	vro_cpyfm(handle, S_ONLY, _boxpx, &_boxfdb, &_screen);
	menu_tnormal(_menu, *tsel, 1);
	_menu[*ssel].ob_flags&=~SELECTED;
	*tsel=*bsel=*ssel=0;
}

int evnt_mesag(short *msgbuff) {
	/*This routine is only partially implemented.
If the mouse moves to the top menu, the dropdown menus appear. A click in 
one of them will result in the message  buffer (an array of 8 shorts) 
containing:
short 0: 10 (a macro, MN_SELECTED, is the mnemonic for this)
short 3: The object index of the menu title
short 4: The object index of the menu entry
Moving outside the active menu area sets the 1st short to 0, and undraws 
the menu box*/
short i, temp, tselect=0, bselect=0, sselect=0, mx, my;
int flags;
	flags=M_LEFT_DOWN | M_LEFT_UP|M_RIGHT_DOWN | M_RIGHT_UP;
	msgbuff[0]=0; _boxfdb.fd_addr=NULL;
	while(!msgbuff[0]) {
		_mouse_event();
		if(_mevent.kinds&M_MOTION) {
			mx=_mevent.x; my=_mevent.y;
			if(_menu&&(temp=objc_find(_menu, 2, 8, mx, my))>2) {
				if(!_boxfdb.fd_addr) {
					_boxpx[2]=_boxpx[6]=_scrwid-1;
					_boxpx[3]=_boxpx[7]=_menu->ob_height-1;
					_boxfdb.fd_addr=(char *)alloca(_scrwid*_menu->ob_height);
					_boxfdb.fd_w=_scrwid; _boxfdb.fd_h=_menu->ob_height;
					vro_cpyfm(handle, S_ONLY, _boxpx, &_screen, &_boxfdb);
				}
				if(temp!=tselect) {
					if(bselect) {
						_restore_box(&tselect, &bselect, &sselect);
					}
         		bselect=_boxstart; i=_menu[2].ob_head; while(i!=temp) {
         			i=_menu[i].ob_next; bselect=_menu[bselect].ob_next;
         		}
         		objc_offset(_menu, bselect, _boxpx, &_boxpx[1]);
               _boxpx[0]--; _boxpx[1]--;
         		_boxpx[2]=_boxpx[0]+_menu[bselect].ob_width+1;
         		_boxpx[3]=_boxpx[1]+_menu[bselect].ob_height+1;
         		memcpy(&_boxpx[4], _boxpx, 8);
					vro_cpyfm(handle, S_ONLY, _boxpx, &_screen, &_boxfdb);
					tselect=temp;
					menu_tnormal(_menu, tselect, 0);
					objc_draw(_menu, bselect, 2, 0, 0, 0, 0);
				}
			}
			else if(bselect) {
				temp=objc_find(_menu, bselect, 2, mx, my);
				if(temp<bselect)
					_restore_box(&tselect, &bselect, &sselect);
				else if(temp>bselect&&temp!=sselect) {
					if(sselect) menu_tnormal(_menu, sselect, 1);
					sselect=temp;
					menu_tnormal(_menu, temp, 0);
				}
			}
		}
		if(_mevent.kinds&M_LEFT_DOWN&&sselect) {
			msgbuff[0]=MN_SELECTED; msgbuff[3]=tselect; msgbuff[4]=sselect;
			_restore_box(&tselect, &bselect, &sselect);
		}
	}
	return(1);
}

int evnt_timer(int locount, int hicount) {
/*The precision of this is implementation dependent.*/
/*Time is in milleseconds*/
int delay;
	_mouse_event();
	delay=_mevent.time+(locount+hicount*65536);
	while(_mevent.time<delay) _mouse_event();
	return(1);
}

int evnt_mouse(int mflags, int mx, int my, int mwidth, int mheight,
	short *x, short *y, short *butn, short *kstate) {
	/*If mflags is set, this returns 1 when the mouse leaves a rectangle; 
otherwise it returns 1 when it enters. The mouse coordinates and button 
state are passed in the pointers.*/
	while(1) {
		_mouse_event();
		if(mflags) {
			if(_mevent.x<mx||_mevent.x>=mx+mwidth||
				_mevent.y<my||_mevent.y>=my+mheight)
				break;
		}
		else {
			if((_mevent.x>=mx&&_mevent.x<mx+mwidth)||
				(_mevent.y>=my&&_mevent.y<my+mheight))
					break;
		}
	}
	*x=_mevent.x; *y=_mevent.y;
	*butn=_mevent.buttons; *kstate=_mevent.kbstat;
	return(1);
}

int evnt_multi(int aflags, int bmaxclicks, int bmask, int bstate,
	int m1flags, int m1x, int m1y, int m1width, int m1height,
	int m2flags, int m2x, int m2y, int m2width, int m2height,
	short *msgbuff,
	int locount, int hicount, short *x, short *y, short *butn,
	short *kstate, short *kreturn, short *breturn) {
	/*This routine is basically a conflation of the other event routines.
The first parameter is the types of event expected, and the return is 
the types which actually occurred, using the following code:
MU_KEYBD 1 keyboard event
MU_BUTTON 2 button event
MU_M1 4 mouse rectangle event 1
MU_M2 8 mouse rectangle event 2
MU_MESAG 16 message event
MU_TIMER 32 timer event
The next 3 have the same meaning as in evnt_button.
The next 10 are 2 images of the evnt_mouse function.
The msgbuff is as in evnt_mesag.
The locount and hicount are the low word and high word of the number of 
milliseconds that the routine will wait for an  event. Take this with a 
large grain of salt; I've tried for an approximation to equivalence to 
ST timings.
The next 4 pointers are as in evnt_button.
The penultimate is the key pressed or 0.
The last is the number of clicks as for evnt_button.
In use, all of the last set of pointers which are unnecessary can be the 
same variable.*/
short i, temp, tselect=0, bselect=0, sselect=0, timeon=0;
int clicks=0, flags=M_MOTION, result=0, delay;
	if(bmask&1) flags|=(bstate&1) ? M_LEFT_DOWN : M_LEFT_UP;
	if(bmask&2) flags|=(bstate&2) ? M_RIGHT_DOWN : M_RIGHT_UP;
	if(aflags&MU_KEYBD) flags|=M_KEYPRESS;
	if(aflags&MU_TIMER) {
      timeon=1; delay=_mevent.time+locount+hicount*65536;
  	}
	_boxfdb.fd_addr=NULL;
	while(!result) {
		_mouse_event();
		*x=_mevent.x; *y=_mevent.y;
		*butn=_mevent.buttons; *kstate=_mevent.kbstat;
		*kreturn=_mevent.key;
		if(_mevent.kinds&M_MOTION&&_menu) {
			if((temp=objc_find(_menu, 2, 8, *x, *y))>2) {
				aflags&=~MU_TIMER;
				if(!_boxfdb.fd_addr) {
					_boxpx[2]=_boxpx[6]=_scrwid-1;
					_boxpx[3]=_boxpx[7]=_menu->ob_height-1;
					_boxfdb.fd_addr=(char *)alloca(_scrwid*_menu->ob_height);
					_boxfdb.fd_w=_scrwid; _boxfdb.fd_h=_menu->ob_height;
					vro_cpyfm(handle, S_ONLY, _boxpx, &_screen, &_boxfdb);
				}
				if(temp!=tselect) {
					if(bselect) {
						_restore_box(&tselect, &bselect, &sselect);
					}
         		bselect=_boxstart; i=_menu[2].ob_head; while(i!=temp) {
         			i=_menu[i].ob_next; bselect=_menu[bselect].ob_next;
         		}
         		objc_offset(_menu, bselect, _boxpx, &_boxpx[1]);
               _boxpx[0]--; _boxpx[1]--;
         		_boxpx[2]=_boxpx[0]+_menu[bselect].ob_width+1;
         		_boxpx[3]=_boxpx[1]+_menu[bselect].ob_height+1;
         		memcpy(&_boxpx[4], _boxpx, 8);
					vro_cpyfm(handle, S_ONLY, _boxpx, &_screen, &_boxfdb);
					tselect=temp;
					menu_tnormal(_menu, tselect, 0);
					objc_draw(_menu, bselect, 2, 0, 0, 0, 0);
				}
			}
			else if(bselect) {
				temp=objc_find(_menu, bselect, 2, *x, *y);
				if(temp<bselect) {
					_restore_box(&tselect, &bselect, &sselect);
				}
				else if(temp>bselect&&temp!=sselect) {
					if(sselect) menu_tnormal(_menu, sselect, 1);
					sselect=temp;
					menu_tnormal(_menu, temp, 0);
				}
			}
		}
		if((aflags&MU_TIMER)&&_mevent.time>=delay) {
			result|=MU_TIMER;
		}
      if(_mevent.kinds&MU_M1) {
      	if(m1flags) {
      		if(_mevent.x<m1x||_mevent.x>=m1x+m1width||
            	_mevent.y<m1y||_mevent.y>=m1y+m1height)
            	result|=MU_M1;
         }
         else {
      		if((_mevent.x>=m1x&&_mevent.x<m1x+m1width)||
            	(_mevent.y>=m1y&&_mevent.y<m1y+m1height))
            	result|=MU_M1;
         }
      }
      if(_mevent.kinds&MU_M2) {
      	if(m2flags) {
      		if(_mevent.x<m2x||_mevent.x>=m2x+m2width||
            	_mevent.y<m2y||_mevent.y>=m2y+m2height)
            	result|=MU_M2;
         }
         else {
      		if((_mevent.x>=m2x&&_mevent.x<m2x+m2width)||
            	(_mevent.y>=m2y&&_mevent.y<m2y+m2height))
            	result|=MU_M2;
         }
      }
		if(_mevent.kinds&flags&M_KEYPRESS) {
			result|=MU_KEYBD;
		}
		if(aflags&MU_MESAG&&(_mevent.kinds&flags&M_LEFT_DOWN)) {
			if(sselect) {
				msgbuff[0]=MN_SELECTED; msgbuff[3]=tselect;
				msgbuff[4]=sselect; result|=MU_MESAG;
				_restore_box(&tselect, &bselect, &sselect);
			}
		}
		if((aflags&MU_BUTTON)&&(_mevent.kinds&flags&(M_BUTTON_CHANGE))) {
			if(++clicks>=bmaxclicks) {
				result|=MU_BUTTON; *breturn=clicks;
			}
		}
      if(timeon&&tselect<0) aflags|=MU_TIMER;
	}
	return(result);
}

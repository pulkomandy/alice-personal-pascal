#include	<stdlib.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<dirent.h>
#include	<string.h>
#include	<sys/param.h>
#include	"osbind.h"
#include	"vdi.h"
#include	"aes.h"

#define _FS_LABEL	1
#define _FS_PATH	2
#define _FS_BACK	3
#define _FS_SELECT	4
#define _FS_FILES	5
#define _FS_FILE1	6
#define _FS_OK	38
#define _FS_CANCEL	39
#define _FS_EXT1	40
#define _FS_GOLEFT	46
#define _FS_GORIGHT	47

typedef struct fileinfo {
	char name[64];	/* name of file */
	char extn[8];	/* extension of it */
	short  dirflag;	/* 0 if not a directory, 1 if is directory */
} FILEINFO;

FILEINFO	_files[400];
short _numfiles, _numdirs, _nummatches, _currfile, _matches[400];
char *_path, *_selfile, *_selext, *_dotat;
extern	OBJECT	*_fselbox;
extern	void	_obcurse(int);
short _fsx, _fsy, _fsw, _fsh;
extern	int	handle;

/*The fileselector, alertbox and inputbox*/
unsigned short _guirsh[] = { 2542,	1006,	1042,	2530,	3,
62,	3,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30720,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	120,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30720,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	120,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30720,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	120,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30720,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	120,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30720,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
120,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30720,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	120,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30720,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	120,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30720,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	120,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30720,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	120,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30720,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	120,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30720,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	120,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30720,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	120,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30720,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
120,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30720,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	120,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30720,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	120,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
17152,	8224,	18432,	8224,	21504,	21592,	17408,	17231,
20480,	22595,	30720,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	120,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30720,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	120,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	16896,	21589,	20308,	78,
21826,	21588,	20047,	18688,	28782,	29813,	20992,	29797,
29301,	110,	26711,	25449,	104,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	30840,	30840,	30840,	30840,	16896,	21589,	20308,
78,	25939,	25964,	29795,	24864,	26144,	27753,	101,
24415,	24415,	24415,	24415,	24415,	24415,	24415,	24415,
24415,	24415,	24415,	24415,	24415,	24415,	24415,	24415,
24415,	24415,	24415,	24320,	24415,	24415,	24415,	24415,
24415,	24415,	24415,	24415,	24415,	24415,	24415,	24415,
24415,	24415,	24415,	24415,	24415,	24415,	95,	25939,
25964,	29795,	17152,	28257,	25955,	108,	24415,	24415,
24415,	24415,	24415,	24415,	24415,	24415,	24415,	24415,
24415,	24415,	24320,	24415,	24415,	24415,	24415,	24415,
24415,	24415,	24415,	24415,	24415,	24415,	95,	42,
30840,	30840,	30840,	30840,	30840,	30840,	30840,	30840,
30840,	24320,	24415,	18688,	28782,	29813,	8250,	24415,
95,	828,	0,	867,	0,	88,	256,	920,
0,	945,	0,	88,	128,	991,	0,	995,
0,	120,	0,	65535,	1,	47,	32788,	16,
1,	0,	0,	2,	30,	320,	200,	2,
65535,	65535,	28,	19,	0,	814,	0,	101,
2,	104,	8,	3,	65535,	65535,	31,	1,
0,	1006,	0,	8,	12,	304,	8,	4,
65535,	65535,	33307,	17,	1,	1,	0,	12,
24,	24,	16,	5,	65535,	65535,	31,	1,
0,	1018,	0,	60,	32,	192,	8,	38,
6,	37,	32793,	16,	1,	0,	0,	0,
44,	320,	136,	7,	65535,	65535,	540,	17,
0,	972,	0,	8,	4,	144,	8,	8,
65535,	65535,	540,	17,	0,	0,	0,	8,
12,	144,	8,	9,	65535,	65535,	540,	17,
0,	19,	0,	8,	20,	144,	8,	10,
65535,	65535,	540,	17,	0,	38,	0,	8,
28,	144,	8,	11,	65535,	65535,	540,	17,
0,	57,	0,	8,	36,	144,	8,	12,
65535,	65535,	540,	17,	0,	76,	0,	8,
44,	144,	8,	13,	65535,	65535,	540,	17,
0,	95,	0,	8,	52,	144,	8,	14,
65535,	65535,	540,	17,	0,	114,	0,	8,
60,	144,	8,	15,	65535,	65535,	540,	17,
0,	133,	0,	8,	68,	144,	8,	16,
65535,	65535,	540,	17,	0,	152,	0,	8,
76,	144,	8,	17,	65535,	65535,	540,	17,
0,	171,	0,	8,	84,	144,	8,	18,
65535,	65535,	540,	17,	0,	190,	0,	8,
92,	144,	8,	19,	65535,	65535,	540,	17,
0,	209,	0,	8,	100,	144,	8,	20,
65535,	65535,	540,	17,	0,	228,	0,	8,
108,	144,	8,	21,	65535,	65535,	540,	17,
0,	247,	0,	8,	116,	144,	8,	22,
65535,	65535,	540,	17,	0,	266,	0,	8,
124,	144,	8,	23,	65535,	65535,	540,	17,
0,	285,	0,	168,	4,	144,	8,	24,
65535,	65535,	540,	17,	0,	304,	0,	168,
12,	144,	8,	25,	65535,	65535,	540,	17,
0,	323,	0,	168,	20,	144,	8,	26,
65535,	65535,	540,	17,	0,	342,	0,	168,
28,	144,	8,	27,	65535,	65535,	540,	17,
0,	361,	0,	168,	36,	144,	8,	28,
65535,	65535,	540,	17,	0,	380,	0,	168,
44,	144,	8,	29,	65535,	65535,	540,	17,
0,	399,	0,	168,	52,	144,	8,	30,
65535,	65535,	540,	17,	0,	418,	0,	168,
60,	144,	8,	31,	65535,	65535,	540,	17,
0,	437,	0,	168,	68,	144,	8,	32,
65535,	65535,	540,	17,	0,	456,	0,	168,
76,	144,	8,	33,	65535,	65535,	540,	17,
0,	475,	0,	168,	84,	144,	8,	34,
65535,	65535,	540,	17,	0,	494,	0,	168,
92,	144,	8,	35,	65535,	65535,	540,	17,
0,	513,	0,	168,	100,	144,	8,	36,
65535,	65535,	540,	17,	0,	532,	0,	168,
108,	144,	8,	37,	65535,	65535,	540,	17,
0,	551,	0,	168,	116,	144,	8,	5,
65535,	65535,	540,	17,	0,	570,	0,	168,
124,	144,	8,	39,	65535,	65535,	33564,	17,
769,	906,	0,	76,	184,	66,	12,	40,
65535,	65535,	33308,	17,	513,	913,	0,	184,
184,	66,	12,	41,	65535,	65535,	540,	17,
0,	970,	0,	60,	22,	24,	8,	42,
65535,	65535,	540,	17,	0,	589,	0,	92,
22,	24,	8,	43,	65535,	65535,	540,	17,
0,	593,	0,	124,	22,	24,	8,	44,
65535,	65535,	540,	17,	0,	597,	0,	156,
22,	24,	8,	45,	65535,	65535,	540,	17,
0,	601,	0,	188,	22,	24,	8,	46,
65535,	65535,	540,	17,	0,	605,	0,	220,
22,	24,	8,	47,	65535,	65535,	33307,	17,
1,	4,	0,	8,	184,	16,	12,	0,
65535,	65535,	35355,	17,	1,	3,	0,	296,
184,	16,	12,	65535,	1,	9,	32788,	16,
1,	0,	0,	2,	30,	272,	104,	2,
65535,	65535,	28,	274,	0,	768,	0,	112,
4,	40,	16,	3,	65535,	65535,	28,	17,
0,	774,	0,	8,	24,	256,	8,	4,
65535,	65535,	28,	17,	0,	609,	0,	8,
36,	256,	8,	5,	65535,	65535,	28,	17,
0,	642,	0,	8,	48,	256,	8,	6,
65535,	65535,	28,	17,	0,	675,	0,	8,
60,	256,	8,	7,	65535,	65535,	28,	17,
0,	708,	0,	8,	72,	256,	8,	8,
65535,	65535,	33564,	17,	1,	807,	0,	8,
88,	66,	12,	9,	65535,	65535,	33308,	17,
1,	741,	0,	104,	88,	66,	12,	0,
65535,	65535,	35356,	17,	1,	748,	0,	192,
88,	66,	12,	65535,	1,	3,	32788,	16,
1,	0,	0,	2,	30,	108,	68,	2,
65535,	65535,	30,	1,	0,	1030,	0,	16,
32,	80,	8,	3,	65535,	65535,	33564,	17,
1,	761,	0,	16,	48,	66,	12,	0,
65535,	65535,	2076,	258,	0,	755,	0,	28,
6,	48,	16,	1042,	0,	2194,	0,	2434,	0	};

void _amess(short i, OBJECT *item) {
short m;
char *fname=item->ob_spec;
FILEINFO *file=&_files[_matches[i]];
 	item->ob_colbyte=file->dirflag+1;
	m=strlen(file->extn);
	if(m) {
		if(strlen(file->name)<18-m)
			sprintf(fname, "%s.%s", file->name, file->extn);
		else {
			strncpy(fname, file->name, 17-m); fname[17-m]='~';
			strcpy(&fname[18-m], file->extn);
		}
	}
	else strncpy(fname, file->name, 18);
}

void f_list(short again) {
OBJECT *start;
short i=0, j;
	menu_ienable(_fselbox, _FS_GOLEFT, j=_currfile);
	menu_ienable(_fselbox, _FS_GORIGHT, j<(_nummatches-32));
	start=&_fselbox[_FS_FILE1];
	while(i++<32) {
		memset(start->ob_spec, 32, 18);
		if(j<_nummatches) {
			_amess(j, start); j++; 
		}
		start++;
	}
	if(again) {
		objc_draw(_fselbox, _FS_FILES, 2, _fsx, _fsy, _fsw, _fsh);
		objc_draw(_fselbox, _FS_GOLEFT, 2, _fsx, _fsy, _fsw, _fsh);
		objc_draw(_fselbox, _FS_GORIGHT, 2, _fsx, _fsy, _fsw, _fsh);
	}
}

int _wmatch(const char *temp, const char *test) {
int i=0;
	while(temp[i]) {
		if(!test[i]) return(1);
		if(temp[i]=='*') return(0);
		if(temp[i]!='?') {
			if(temp[i]!=test[i]) return(1);
		}
		i++;
	}
	return(test[i]);
}

void find_matches() {
short i=0;
char temp[FNSIZE], *extn;
	strcpy(temp, _selfile);
	extn=strrchr(temp, '.');
	if(extn) {
		*extn=0; extn++;
	}
	while(i<_numdirs) { _matches[i]=i; i++; }
   _nummatches=i; while(i<_numfiles) {
   	if(!_wmatch(temp, _files[i].name)) {
			if((extn==NULL&&!_files[i].extn[0])||
				(extn&&!_wmatch(extn, _files[i].extn))) {
      		_matches[_nummatches++]=i;
			}
      }
      i++;
   }
}

int dirsort(FILEINFO *f1, FILEINFO *f2) {
	return(f2->dirflag-f1->dirflag);
}

int filesort(FILEINFO *f1, FILEINFO *f2) {
int i;
	i=strcmp(f1->name, f2->name);
	if(!i) i=strcmp(f1->extn, f2->extn);
   return(i);
}


void _get_path(short again) {
short i;
char *name;
DIR *dp;
struct dirent *ep;
struct stat buf;
	graf_mouse(BUSYBEE, 0l);
	while(chdir(_path)) {
		i=strlen(_path)-2;
		while(_path[i]!='/'&&i) i--;
		_path[i+1]=0; _get_path(again);
	}
	_selext=strrchr(_selfile, '.');
	_numfiles=_currfile=0; dp=opendir ("./");
	while((ep=readdir(dp))) {
		name=ep->d_name;
		if(name[0]!='.') {
			stat(name, &buf);
			_files[_numfiles].dirflag=S_ISDIR(buf.st_mode);
			_dotat=strrchr(name, '.');
			if(_dotat) {
				strcpy(_files[_numfiles].extn, &_dotat[1]);
				i=_dotat-name;
				strncpy(_files[_numfiles].name, name, i);
				_files[_numfiles].name[i]='\0';
			}
			else {
				strcpy(_files[_numfiles].name, name);
				_files[_numfiles].extn[0]='\0';
			}
			_numfiles++; if(_numfiles==400) break;
		}
	}
	closedir(dp);
   qsort(_files, _numfiles, sizeof(FILEINFO), dirsort);
	_numdirs=0; while(_numdirs<_numfiles) {
		if(!_files[_numdirs].dirflag) break;
		_numdirs++;
	}
	if(_numdirs) qsort(_files, _numdirs, sizeof(FILEINFO), filesort);
	if(_numdirs<_numfiles)
		qsort(&_files[_numdirs], _numfiles-_numdirs, sizeof(FILEINFO), filesort);
	find_matches();
	if(again) {
		_obcurse(0);
		objc_draw(_fselbox, _FS_PATH, 1, _fsx, _fsy, _fsw, _fsh);
	}
	graf_mouse(ARROW, 0l);
}

void _norm_path() {
char spare[FMSIZE+1];
	if(_path[0]!='/') {
		getcwd(spare, FMSIZE); strcat(spare, "/");
		strcat(spare, _path); strcpy(_path, spare);
	}
}

int _sort_extns() {
short i, j;
char *extn, wild[4];
	_selext=strrchr(_selfile, '.'); if(_selext) {
		extn=&_selext[1]; if(*extn!='*') {
			if(strlen(extn)<4) strcpy(wild, extn);
			else {
				wild[0]=extn[0]; wild[1]=extn[1];
				wild[2]='*'; wild[3]=0;
			}
			i=1; while(i<6) {
				if(!strcmp(wild, _fselbox[_FS_EXT1+i].ob_spec)) break;
				i++;
			}
			j=(i==6)?5:i; while(j>1) {
				strcpy(_fselbox[_FS_EXT1+j].ob_spec,
					_fselbox[_FS_EXT1+j-1].ob_spec);
				j--;
			}
			strcpy(_fselbox[_FS_EXT1+1].ob_spec, wild);
         return(1);
		}
	}
   return(0);
}

void _refresh_extns() {
short i;
	if(_sort_extns()) {
		i=1; while(i<6) {
			objc_draw(_fselbox, _FS_EXT1+i, 1, _fsx, _fsy, _fsw, _fsh);
			i++;
		}
	}
}

void fsel_set_extn(char *type) {
char temp[10];
	_selfile=temp;
	sprintf(_selfile, "*.%s", type);
	_sort_extns();
}

int fsel_exinput(char *his_path, char *his_selfile, short *button, const char *label) {
int i, but, oldbut=-1;
short foo, bar;
char olddir[FMSIZE+1], lastpath[FMSIZE], lastclick[FNSIZE];
	_fselbox[_FS_LABEL].ob_spec=(char *)label;
	_fselbox[_FS_LABEL].ob_width=strlen(label)*8;
	_fselbox[_FS_LABEL].ob_x=160-(_fselbox[_FS_LABEL].ob_width/2);
	/*strcpy(_selfile, his_selfile);*/
	((TEDINFO *)_fselbox[_FS_SELECT].ob_spec)->te_ptext=_selfile=his_selfile;
	((TEDINFO *)_fselbox[_FS_PATH].ob_spec)->te_ptext=_path=his_path;
	_sort_extns();
	getcwd(olddir, FMSIZE);
   if(!_path[0]) {
   	strcpy(_path, olddir); strcat(_path, "/");
   }
	_norm_path();
	if(!*_selfile) {
		i=strlen(_path)-1;
		if(_path[i]=='/') strcpy(_selfile, "*.*");
		else {
			while(_path[i]!='.'&&i) i--;
			if(i>0) {
				*_selfile='*'; strcpy(&_selfile[1], &_path[i]);
			}
			while(_path[i]!='/'&&i>-1) i--;
			if(i>=0) _path[i]=0;
		}
	}
	form_center(_fselbox, &_fsx, &_fsy, &_fsw, &_fsh);
	form_dial(0,0,0,0,0, _fsx, _fsy, _fsw, _fsh);
	_get_path(0);
	f_list(0);
	objc_draw(_fselbox, 0, 8, _fsx, _fsy, _fsw, _fsh);
	while(1) {
   	strcpy(lastpath, _path);
      strcpy(lastclick, _selfile);
		but=form_do(_fselbox, _FS_SELECT);
		_fselbox[but].ob_flags&=~SELECTED;
		vq_mouse(handle, &foo, &bar, &bar);
		if(but>=_FS_FILE1&&but<_FS_FILE1+32&&(i=but-_FS_FILE1+_currfile)<_nummatches) {
			if(i<_numdirs) {
				strcat(_path, _fselbox[but].ob_spec);
				strcat(_path, "/");
				_get_path(1);
				f_list(1);
			}
			else {
				strcpy(_selfile, _fselbox[but].ob_spec); _refresh_extns();
				if(but==oldbut||foo==2) { *button=1; break; }
				objc_draw(_fselbox, _FS_SELECT, 1, _fsx, _fsy, _fsw, _fsh);
				oldbut=but; but=-1;
			}
		}
		if(but==_FS_GOLEFT&&_currfile) {
			_currfile-=16; f_list(1);
		}
		if(but==_FS_GORIGHT&&_currfile<(_numfiles-32)) {
			_currfile+=16; f_list(1);
		}
		if(but==_FS_CANCEL) { *button=0; break; }
		if(but==_FS_OK) {
      	if(!strcmp(lastpath, _path)&&!strcmp(lastclick, _selfile)) {
				/*strcpy(his_selfile, _selfile);*/
         	*button=1; break;
         }
      	if(strcmp(lastpath, _path)) {
         	_norm_path(); _get_path(1);
         }
         else {
         	_refresh_extns(); find_matches();
         	if(_currfile>=_nummatches) _currfile=0;
         }
			f_list(1);
      }
		if(but==_FS_BACK) {
			i=strlen(_path); if(i>1) {
				i-=2; while(_path[i]!='/'&&i) i--;
				_path[i+1]=0;
				_get_path(1);
				f_list(1);
			}
		}
		if(but>=_FS_EXT1&&but<_FS_EXT1+6) {
         strcpy(_selfile, "*."); strcat(_selfile, _fselbox[but].ob_spec);
			objc_draw(_fselbox, _FS_SELECT, 1, _fsx, _fsy, _fsw, _fsh);
         _refresh_extns();
         find_matches();
         if(_currfile>=_nummatches) _currfile=0;
			f_list(1);
		}
	}
	form_dial(3,0,0,0,0, _fsx, _fsy, _fsw, _fsh);
	chdir(olddir);
	return(1);
}

int fsel_input(char *path, char *selfile, short *button) {
	return(fsel_exinput(path, selfile, button, "Select a file"));
}

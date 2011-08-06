short Getrez(void);
char	*Physbase(void);
char	*Logbase(void);

void	Setscreen(int, int);
void	Setpalette(void);
int	Getpalette(void);
int	Resetpalette(int);
void	Palettize(int, unsigned char *, unsigned char *);
void	Getcolor(int, unsigned char *);
int	Grabpalette(unsigned char *);
void	Storepalette(int, int, unsigned char *);
void	Forcepalette(int, int, unsigned char *);
void	Defaultpalette(void);
#define	Setpallete()	Setpalette()
int Setcolor(int, unsigned char *);

#define	Random()	rand()

#define	max(a,b)    (((a) > (b)) ? (a) : (b))
#define	min(a,b)    (((a) < (b)) ? (a) : (b))

/*Key definitions across libraries*/
#define	CARRET	13
#define	AKEY	'a'
#define	BKEY	'b'
#define	CKEY	'c'
#define	DKEY	'd'
#define	EKEY	'e'
#define	FKEY	'f'
#define	GKEY	'g'
#define	IKEY	'i'
#define	LKEY	'l'
#define	NKEY	'n'
#define	PKEY	'p'
#define	QKEY	'q'
#define	RKEY	'r'
#define	SKEY	's'
#define	UKEY	'u'
#define	XKEY	'x'
#define	YKEY	'y'
#define	COMMAKEY	','
#define	STOPKEY	'.'
#define	SLASHKEY	'/'
#define	SPACEKEY	' '
#define	ESCKEY 27
#define	TABKEY 9
#define	BCKKEY 127
#define	DELKEY 128
#define	F1KEY 323
#define	F2KEY 324
#define	F3KEY 325
#define	F4KEY 326
#define	UAKEY 354
#define	DAKEY 360
#define	RAKEY 358
#define	LAKEY 356
#define	STLBUT	1
#define	STRBUT	2

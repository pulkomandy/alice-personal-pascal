

#define	GM_MENUITEM	1000
#define GM_ENDMENU	1999
#define GM_MESSAGE	2000
#define GM_MSGEND	2999
#define GM_ARROWS	3000
#define GM_ARREND	3007
#define GM_CURSOR	3008

#define GM_END		3010

#define is_gem(X)	( (X >= GM_MENUITEM) && (X <= GM_END))

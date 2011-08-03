
#ifdef QNX
extern unsigned Exc_pending, Exc_bits;
#define EXC_BREAK 2
#define got_break() (Exc_pending & EXC_BREAK)
#define clear_break() Exc_pending &= ~EXC_BREAK; Exc_bits &= ~EXC_BREAK

#endif

#ifdef unix
extern int didbreak;
#define got_break()	didbreak
#define clear_break()	didbreak = FALSE
#endif

#ifdef msdos
#define got_break()	(speek( 0x40, 0x71 ) & 0x80)
#define clear_break()	spoke( 0x40, 0x71, speek( 0x40, 0x71 ) & 0x7f )
#endif


#define DMFREE

#ifdef HERE
# define here()			gotHere(__FILE__, __LINE__)
#else
# define here()
#endif

#ifdef DMFREE
# define dmfree(p)		mfree(p, __FILE__, __LINE__)
#else
# define dmfree(p)		mfree(p)
#endif DMFREE


/*
 * Includes for the huffman encryption of the help files
 */

#ifdef DEMO
#define HELP_VERSION	100
#else
#define HELP_VERSION	-1	/* Version Number.  helpfile.c must be compiled
				 * with the same number
				 */
#endif

#define MAX_STRCOUNT	300
#define	TOKCOUNT	(128 + MAX_STRCOUNT)
#define BUFSIZE		64
#define STRSIZE		40
#define CRMODE		0644
#define MIN_FREQ	10
#define HUFF_HASH_SZ	31	/* number of buckets for the strings */
#define MAX_FILE_COUNT	550
#define PUFFSIZE	64

#define HELPFILE	"helpfile.huf"
#define HELPINDEX	"helpfile.ind"
#define HELPFREQ	"helpfile.frq"

#define	BOF		0	/* seek from beginning of file */

typedef struct hnode
	{
	long count;
	int node_num;
	int ch;
	struct hnode *left,*right;
	} huffnode;

typedef struct hsnode
	{
	int  hash;
	int  h_offset;
	} file_entry;

struct hfstr_node {
      char *hf_name;
      bits16 hf_no;
      struct hfstr_node *hf_next;
      };

typedef struct hfstr_node *hfstr_ptr;


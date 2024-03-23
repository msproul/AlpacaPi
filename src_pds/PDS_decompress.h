//**************************************************************************
//#include	"PDS_decompress.c"

//*	the LONG's defined here must be 32 bit
#define	LONG	int

//**************************************************************************
typedef struct leaf
{
//	struct leaf	*right;
	void		*right;
	short 		dn;
//	struct leaf	*left;
	void		*left;
} NODE;


NODE *new_node(short  value);
void sort_freq(LONG *freq_list, NODE **node_list, LONG num_freq);
NODE *huff_tree(LONG  *hist);
void decmpinit(LONG *hist);
void dcmprs(char *ibuf, char *obuf, LONG *nin, LONG *nout, NODE *root);
void decompress(char *ibuf, char *obuf, LONG  *nin, LONG  *nout);


void	DecompressFreeMemory(void);

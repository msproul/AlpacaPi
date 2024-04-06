//********************************************************************
//*	Voyager Image Decompression Program
//*	Adapted by Mark Sproul
//*	January 1992
//*	for using in my graphics programs
//*
//*	This source code was obtained from the CD-ROMs
//*		"NASA Voyagers to the Outer Planets"
//*
//********************************************************************
//********************************************************************
//*	Voyager Image Decompression Program - C Version for PC, VAX,
//*	UNIX and Macintosh systems.
//*
//*	Decompresses images using Kris Becker's subroutine DECOMP.C
//*	which is included in this program in a shortened version.
//*
//********************************************************************
//*	Edit history
//********************************************************************
//*	Nov 19,	2022	<MLS> After MANY years (30) working on code again
//*	Nov 19,	2022	<MLS> "long" used to be 32 bits, now its 64, this routine needs 32 bit
//*	Nov 19,	2022	<MLS> Added DecompressFreeMemory()
//********************************************************************

#include	<stdlib.h>
#include	<stdio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"PDS_decompress.h"


//**************************************************************************
//* Declare the tree pointer. This pointer will hold the root of the tree
//* once the tree is created by the accompanying routine huff_tree.
//***************************************************************************

NODE	*gTree	=	NULL;


//****************************************************************************
//*_TITLE new_node - allocates a NODE structure and returns a pointer to it
//****************************************************************************
NODE *new_node(short value)	//* I	Value to assign to DN field
{
NODE *temp;		 //* Pointer to the memory block

	//***************************************************************************
	//	Allocate the memory and initialize the fields.
	//***************************************************************************

	temp	=	(NODE *) malloc(sizeof(NODE));

	if (temp != NULL)
	{
		temp->right	=	NULL;
		temp->dn	=	value;
		temp->left	=	NULL;
	}
	else
	{
		printf("\nOut of memory in new_node!\n");
		exit(1);
	}
	return(temp);
}

//****************************************************************************
//*_TITLE sort_freq - sorts frequency and node lists in increasing freq. order*
//*_ARGS	TYPE	NAME			I/O	DESCRIPTION
//****************************************************************************
void sort_freq(	LONG	*freq_list,		//* I	Pointer to frequency list
				NODE	**node_list,	//* I	Pointer to array of node pointers
				LONG	num_freq)		//* I	Number of values in freq list
{
register LONG 		*i;		//* primary pointer into freq_list
register LONG 		*j;		//* secondary pointer into freq_list
register NODE		**k;	//* primary pointer to node_list
register NODE		**l;	//* secondary pointer into node_list
LONG 				temp1;	//* temporary storage for freq_list
NODE				*temp2;	//* temporary storage for node_list
register LONG		cnt;		//* count of list elements


	//************************************************************************
	//*	Save the current element - starting with the second - in temporary
	//*	storage.  Compare with all elements in first part of list moving
	//*	each up one element until the element is larger.  Insert current
	//*	element at this point in list.
	//**************************************************************************

	if (num_freq <= 0) return;		//* If no elements or invalid, return

	for (i=freq_list, k=node_list, cnt=num_freq ; --cnt ; *j=temp1, *l=temp2)
	 {
		temp1	=	*(++i);
		temp2	=	*(++k);

		for (j = i, l = k ; *(j-1) > temp1 ; )
		{
			*j	=	*(j-1);
			*l	=	*(l-1);
			j--;
			l--;
			if ( j <= freq_list) break;
		}
	}
}

//****************************************************************************
//*_TITLE huff_tree - constructs the Huffman tree; returns pointer to root
//*_ARGS	TYPE		NAME			I/O		DESCRIPTION
//****************************************************************************
NODE *huff_tree(LONG 	*hist)		//*	I		First difference histogram
{
LONG			freq_list[512];		//*	Histogram frequency list
NODE			**node_list;		//*	DN pointer array list
register LONG	*fp;				//*	Frequency list pointer
register NODE	**np;		 		//* Node list pointer
register LONG	num_freq;			//* Number non-zero frequencies in histogram
register short	num_nodes;			//* Counter for DN initialization
register short	cnt;				//* Miscellaneous counter
//LONG			sum;				//* Sum of all frequencies
short			znull	=	-1;		//* Null node value
register NODE	*temp;				//* Temporary node pointer


//***************************************************************************
//	Allocate the array of nodes from memory and initialize these with numbers
//	corresponding with the frequency list.  There are only 511 possible
//	permutations of first difference histograms.  There are 512 allocated
//	here to adhere to the FORTRAN version.
//***************************************************************************

	fp			=	freq_list;
	node_list	=	(NODE **) malloc(sizeof(temp) * 512);
	if (node_list == NULL)
	{
		printf("\nOut of memory in huff_tree!\n");
		exit(1);
	}
	np	=	node_list;

	for (num_nodes=1, cnt=512 ; cnt-- ; num_nodes++)
	{
		//**************************************************************************
		//	The following code has been added to standardize the VAX byte order
		//	for the "long int" type.  This code is intended to make the routine
		//	as machine independent as possible.
		//***************************************************************************
		unsigned char	*cp	=	(unsigned char *) hist++;
		unsigned LONG	j;
		short int i;
		for (i=4 ; --i >= 0 ; j = (j << 8) | *(cp+i));

		//* Now make the assignment
		*fp++	=	j;
		temp	=	new_node(num_nodes);
		*np++	=	temp;
	}

	 (*--fp)	=	0;		 //* Ensure the last element is zeroed out.

	//***************************************************************************
	//	Now, sort the frequency list and eliminate all frequencies of zero.
	//****************************************************************************

	num_freq	=	512;
	sort_freq(freq_list,node_list,num_freq);

	fp	=	freq_list;
	np	=	node_list;

	for (num_freq=512 ; (*fp) == 0 && (num_freq) ; fp++, np++, num_freq--);


	//***************************************************************************
	//	Now create the tree.  Note that if there is only one difference value,
	//	it is returned as the root.  On each iteration, a new node is created
	//	and the least frequently occurring difference is assigned to the right
	//	pointer and the next least frequency to the left pointer.  The node
	//	assigned to the left pointer now becomes the combination of the two
	//	nodes and it's frequency is the sum of the two combining nodes.
	//****************************************************************************
	for (temp=(*np) ; (num_freq--) > 1 ; )
	{
		temp		=	new_node(znull);
		temp->right	=	(*np++);
		temp->left	=	(*np);
		*np			=	temp;
		*(fp+1)		=	*(fp+1) + *fp;
		*fp++		=	0;
		sort_freq(fp,np,num_freq);
	}
	return(temp);
}

//***************************************************************************
//*	TITLE decmpinit - initializes the Huffman tree	*
//*	_ARGS  TYPE		NAME	  I/O		DESCRIPTION
//***************************************************************************
void decmpinit(LONG	*hist)	///* I			First-difference histogram.
{
	//****************************************************************************
	//  Simply call the huff_tree routine and return.
	//*****************************************************************************

	gTree	=	huff_tree(hist);
}



//****************************************************************************
//*_TITLE dcmprs - decompresses Huffman coded compressed image lines
//*_ARGS  TYPE	NAME	I/O		DESCRIPTION
//****************************************************************************
void dcmprs(char	*ibuf,		//* I		Compressed data buffer
			char	*obuf,		//* O		Decompressed image line
			LONG 	*nin,		//* I		Number of bytes on input buffer
			LONG 	*nout,		//* I		Number of bytes in output buffer
			NODE	*root)		//* I		Huffman coded tree
{
register NODE			*ptr	=	root;			//* pointer to position in tree
register unsigned char	test;						//* test byte for bit set
register unsigned char	idn;						//* input compressed byte
register char			odn;						//* last dn value decompressed
char					*ilim	=	ibuf + *nin;	//* end of compressed bytes
char					*olim	=	obuf + *nout;	//* end of output buffer

	//**************************************************************************
	//  Check for valid input values for nin, nout and make initial assignments.
	//***************************************************************************

	if (ilim > ibuf && olim > obuf)
	{
		odn	=	*obuf++	=	*ibuf++;
	}
	else
	{
		printf("\nInvalid byte count in dcmprs!\n");
		exit(1);
	}

	//**************************************************************************
	//  Decompress the input buffer.  Assign the first byte to the working
	//  variable, idn.  An arithmatic and (&) is performed using the variable
	//  'test' that is bit shifted to the right.  If the result is 0, then
	//  go to right else go to left.
	//***************************************************************************

	for (idn=(*ibuf) ; ibuf < ilim  ; idn =(*++ibuf))
	{
		for (test=0x80 ; test ; test >>= 1)
		{
			ptr	=	(test & idn) ? ptr->left : ptr->right;

			if (ptr->dn != -1)
			{
				if (obuf >= olim) return;
				odn		-=	ptr->dn + 256;
				*obuf++	=	odn;
				ptr		=	root;
			}
		}
	}
}

//****************************************************************************
//*_TITLE decompress - decompresses image lines stored in compressed format
//*_ARGS	TYPE		NAME	  I/O		DESCRIPTION
//****************************************************************************
void decompress(char		*ibuf,  //* I		 Compressed data buffer
				char		*obuf,  //* O		 Decompressed image line
				LONG		*nin,	//* I		 Number of bytes on input buffer
				LONG		*nout)	//* I		 Number of bytes in output buffer
{
//*************************************************************************
//	This routine is fairly simple as it's only function is to call the
//	routine dcmprs.
//**************************************************************************

	dcmprs(ibuf, obuf, nin, nout, gTree);
}


int		gFreeNodeCounter;
int		gFreeNodeDepth;
int		gFreeNodeMaxDepth;
//**************************************************************************
static void	FreeNode(NODE	*nodePtr)
{
	gFreeNodeCounter++;
	gFreeNodeDepth++;
	if (gFreeNodeDepth > gFreeNodeMaxDepth)
	{
		gFreeNodeMaxDepth	=	gFreeNodeDepth;
	}
	if (nodePtr != NULL)
	{
		if (nodePtr->left != NULL)
		{
			FreeNode(nodePtr->left);
			free(nodePtr->left);
			nodePtr->left	=	NULL;
		}
		if (nodePtr->right != NULL)
		{
			FreeNode(nodePtr->right);
			free(nodePtr->right);
			nodePtr->right	=	NULL;
		}
	}
	gFreeNodeDepth--;
}

//**************************************************************************
//*	attempt to free up the memory used for the tree
//**************************************************************************
void	DecompressFreeMemory(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	gFreeNodeCounter	=	0;
	gFreeNodeDepth		=	0;
	gFreeNodeMaxDepth	=	0;

	if (gTree != NULL)
	{
		FreeNode(gTree);
		free(gTree);
		gTree	=	NULL;
	}
//	CONSOLE_DEBUG_W_NUM("gFreeNodeCounter \t=", gFreeNodeCounter);
//	CONSOLE_DEBUG_W_NUM("gFreeNodeMaxDepth\t=", gFreeNodeMaxDepth);
}

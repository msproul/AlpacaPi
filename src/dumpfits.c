//*****************************************************************************
//*	Dump Fits
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jun 30,	2022	<MLS> Added dumpfits to AlpacaPi project
//*****************************************************************************

#include <string.h>
#include <stdio.h>
#include <fitsio.h>

//*****************************************************************************
int main(int argc, char *argv[])
{
fitsfile	*fptr;
char		card[FLEN_CARD];
int			status;
int			nkeys;
int			iii;
int			fileCntr;

	for (fileCntr=1; fileCntr < argc; fileCntr++)
	{
		status	=	0;	///* MUST initialize status
		fits_open_file(&fptr, argv[fileCntr], READONLY, &status);
		fits_get_hdrspace(fptr, &nkeys, NULL, &status);

		for (iii = 1; iii <= nkeys; iii++)
		{
			fits_read_record(fptr, iii, card, &status); /* read keyword */
			printf("%s\n", card);
		}
		printf("END\n\n");		/* terminate listing with END */
		fits_close_file(fptr, &status);

		if (status)			/* print any error messages */
		{
			fits_report_error(stderr, status);
		}
	}
	return(status);
}

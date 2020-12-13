//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************

#include	<string.h>
#include	<stdio.h>
#include	<fitsio.h>
#include	<stdbool.h>

#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"fits_opencv.h"

//*****************************************************************************
IplImage	*ReadFITSimageIntoOpenCVimage(const char *fitsFileName)
{
fitsfile		*fptr;
int				fitsRetCode;
int				status;
IplImage		*openCvImgPtr;
int				nfound;
long			naxes[3];
long			bitpix;
int				width;
int				height;
char			*pixelPtr;
unsigned short	*uShortPtr;
int				ii;
int				jj;
//short			nullval;
long			firstPixel;
//char			errorString[64];
int				errorCnt;

//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("File:", fitsFileName);


	openCvImgPtr	=	NULL;
	errorCnt		=	0;
	naxes[0]		=	0;
	naxes[1]		=	0;
	naxes[2]		=	0;
	status			=	0;
	fitsRetCode		=	fits_open_file(&fptr, fitsFileName, READONLY, &status);
	if (fitsRetCode == 0)
	{

		//* read the NAXIS key words
		status		=	0;
		fitsRetCode	=	fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nfound, &status);
//		CONSOLE_DEBUG_W_NUM("nfound\t=",	nfound);
//		CONSOLE_DEBUG_W_INT32("naxes[0]\t=",	naxes[0]);
//		CONSOLE_DEBUG_W_INT32("naxes[1]\t=",	naxes[1]);
//		CONSOLE_DEBUG_W_INT32("naxes[2]\t=",	naxes[2]);
//		CONSOLE_DEBUG_W_NUM("TSHORT\t=", TSHORT);
//		CONSOLE_DEBUG_W_NUM("TUSHORT\t=", TUSHORT);

		status		=	0;
		bitpix		=	0;	//*	set a default
		fitsRetCode	=	fits_read_key_lng(fptr, "BITPIX", &bitpix, NULL, &status);
//		CONSOLE_DEBUG_W_INT32("bitpix\t=",	bitpix);

		switch(bitpix)
		{
			case 8:
		//		if (nfound == 2)
				{
					//*	we have an 8 bit B/W image
					width			=	naxes[0];
					height			=	naxes[1];
					firstPixel		=	1;
					openCvImgPtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
					if (openCvImgPtr != NULL)
					{
						pixelPtr	=	openCvImgPtr->imageData;
						for (ii=0; ii<height; ii++)
						{
							//*	now we are going to read in the image a row at a time
							status		=	0;
							//nullval		=	0;
							fitsRetCode	=	fits_read_img(	fptr,
															TBYTE,
															firstPixel,			//*	first pixel
															width,
															NULL,				//*	nullval,
															pixelPtr,
															NULL,
															&status);
							if (fitsRetCode != 0)
							{
								errorCnt++;
								CONSOLE_DEBUG_W_NUM("fitsRetCode\t=",	fitsRetCode);
							}
							pixelPtr	+=	openCvImgPtr->widthStep;
							firstPixel	+=	width;
						}
					}
				}
				break;

			case 16:
				if (nfound == 2)
				{
					//*	we have what we are expecting, a 16 bit B/W image

					width			=	naxes[0];
					height			=	naxes[1];
					firstPixel		=	1;
					openCvImgPtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 1);
					if (openCvImgPtr != NULL)
					{
						pixelPtr	=	openCvImgPtr->imageData;
						for (ii=0; ii<height; ii++)
						{
							//*	now we are going to read in the image a row at a time
							status		=	0;
							//*	http://www.mssl.ucl.ac.uk/swift/om/sw/help/fitsio/node59.html
							//*	https://heasarc.gsfc.nasa.gov/docs/software/fitsio/c/c_user/node20.html
							fitsRetCode	=	fits_read_img(	fptr,
															TUSHORT,			//	was TSHORT
															firstPixel,			//*	first pixel
															width,
															NULL,				//*	nullval,
															pixelPtr,
															NULL,
															&status);
						#if 0
							uShortPtr	=	(short *)pixelPtr;
							for (jj=0; jj<width; jj++)
							{
								if (uShortPtr[jj] < 2000)
								{
									uShortPtr[jj]	=	0;
								}
							}
						#endif
							if (fitsRetCode != 0)
							{
								errorCnt++;
						//		CONSOLE_DEBUG_W_NUM("fitsRetCode\t=",	fitsRetCode);
						//		GetFitsErrorString(fitsRetCode, errorString);
						//		CONSOLE_DEBUG_W_STR("fits_write_pix returned:", errorString);
							}
							pixelPtr	+=	openCvImgPtr->widthStep;
							firstPixel	+=	width;
						}
					}
				}
				break;

			default:
				CONSOLE_DEBUG_W_LONG("Unsupported bit depth\t=",	bitpix);
				break;

		}
		status		=	0;
		fits_close_file(fptr, &status);
	}
	if (errorCnt> 0)
	{
		CONSOLE_DEBUG_W_NUM("errorCnt\t=",	errorCnt);
	}
	return(openCvImgPtr);
}

//*****************************************************************************
//*	Function:	Reads an image into opencv data structure.
//*				uses the extension to determine how to read it
//*****************************************************************************
IplImage	*ReadImageIntoOpenCVimage(const char *imageFileName)
{
IplImage	*openCvImgPtr;
int			fnameLen;
char		extension[8];

//	CONSOLE_DEBUG(__FUNCTION__);

	openCvImgPtr	=	NULL;
	fnameLen		=	strlen(imageFileName);
	strcpy(extension, &imageFileName[fnameLen - 4]);
//	CONSOLE_DEBUG_W_STR("extension\t=", extension);
	if ((strcmp(extension, "fits") == 0) ||
		(strcmp(extension, ".fit") == 0))
	{
		openCvImgPtr	=	ReadFITSimageIntoOpenCVimage(imageFileName);
	}
	else if (strcmp(extension, "csv") == 0)
	{
		openCvImgPtr	=	NULL;
	}
	else
	{
		CONSOLE_DEBUG_W_STR("imageFileName\t=", imageFileName);
		openCvImgPtr	=	cvLoadImage(imageFileName, CV_LOAD_IMAGE_COLOR);
	}
	return(openCvImgPtr);
}

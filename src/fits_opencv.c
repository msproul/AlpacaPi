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
//*	Edit History
//*****************************************************************************
//*	Nov 16,	2021	<MLS> ReadFITSimageIntoOpenCVimage() now handles 8 bit RGB fits files
//*****************************************************************************

#include	<string.h>
#include	<stdio.h>
#include	<fitsio.h>
#include	<stdbool.h>

#ifdef _USE_OPENCV_CPP_
	#include	<opencv2/opencv.hpp>
	#include	<opencv2/core.hpp>
#else
	#include "opencv/highgui.h"
	#include "opencv2/highgui/highgui_c.h"
	#include "opencv2/imgproc/imgproc_c.h"
	#include "opencv2/core/version.hpp"
#endif // _USE_OPENCV_CPP_

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"fits_opencv.h"

#if defined(_USE_OPENCV_CPP_) &&  (CV_MAJOR_VERSION >= 4)

#else
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
char			*pixelBuffer;
int				ccc;
int				iii;
int				jjj;
//unsigned short	*uShortPtr;
//short			nullval;
long			firstPixel;
//char			errorString[64];
int				errorCnt;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("File:", fitsFileName);


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
		CONSOLE_DEBUG_W_NUM("nfound\t=",	nfound);
		CONSOLE_DEBUG_W_LONG("naxes[0]\t=",	naxes[0]);
		CONSOLE_DEBUG_W_LONG("naxes[1]\t=",	naxes[1]);
		CONSOLE_DEBUG_W_LONG("naxes[2]\t=",	naxes[2]);
//		CONSOLE_DEBUG_W_NUM("TSHORT\t=", TSHORT);
//		CONSOLE_DEBUG_W_NUM("TUSHORT\t=", TUSHORT);

		status		=	0;
		bitpix		=	0;	//*	set a default
		fitsRetCode	=	fits_read_key_lng(fptr, "BITPIX", &bitpix, NULL, &status);
		CONSOLE_DEBUG_W_LONG("bitpix\t=",	bitpix);

		switch(bitpix)
		{
			case 8:
				//*	is it color?
				if (nfound == 3)
				{
					//*	we have an 8 bit Color image
					width			=	naxes[0];
					height			=	naxes[1];
					firstPixel		=	1;
					openCvImgPtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
					if (openCvImgPtr != NULL)
					{
						pixelBuffer	=	(char *)malloc(width);
						if (pixelBuffer != NULL)
						{
							for (ccc=0; ccc<3; ccc++)
							{
								pixelPtr	=	openCvImgPtr->imageData;
								for (iii=0; iii<height; iii++)
								{
									//*	now we are going to read in the image a row at a time
									status		=	0;
									//nullval		=	0;
									fitsRetCode	=	fits_read_img(	fptr,
																	TBYTE,
																	firstPixel,			//*	first pixel
																	width,
																	NULL,				//*	nullval,
																	pixelBuffer,
																	NULL,
																	&status);
									if (fitsRetCode != 0)
									{
										errorCnt++;
										CONSOLE_DEBUG_W_NUM("fitsRetCode\t=",	fitsRetCode);
									}
									for (jjj=0; jjj<width; jjj++)
									{
										pixelPtr[(jjj * 3) + ccc]	=	pixelBuffer[jjj];
									}

									pixelPtr	+=	openCvImgPtr->widthStep;
									firstPixel	+=	width;
								}
							}
							free(pixelBuffer);
						}
					}
				}
				else if (nfound == 2)
				{
					//*	we have an 8 bit B/W image
					width			=	naxes[0];
					height			=	naxes[1];
					firstPixel		=	1;
					openCvImgPtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
					if (openCvImgPtr != NULL)
					{
						pixelPtr	=	openCvImgPtr->imageData;
						for (iii=0; iii<height; iii++)
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
				else
				{
					CONSOLE_DEBUG_W_NUM("Invalid format; nfound\t=",	nfound);
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
						for (iii=0; iii<height; iii++)
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
IplImage	*ReadAlpacaBinaryIntoOpenCVimage(const char *binaryFileName)
{
IplImage			*openCvImgPtr;
TYPE_BinaryImageHdr	binaryImageHdr;
FILE				*filePointer;
int					numRead;
size_t				dataByteCount;
unsigned char		*pixelPtr;
int					quality[3] = {16, 200, 0};
int					openCVerr;
char				jpegFileName[64];

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("File:", binaryFileName);


	openCvImgPtr	=	NULL;

	filePointer		=	fopen(binaryFileName, "r");
	if (filePointer != NULL)
	{
		numRead	=	fread(&binaryImageHdr, sizeof(TYPE_BinaryImageHdr), 1, filePointer);
		if (numRead > 0)
		{
			CONSOLE_DEBUG_W_NUM("MetadataVersion\t\t=",			binaryImageHdr.MetadataVersion);
			CONSOLE_DEBUG_W_NUM("ErrorNumber\t\t=",				binaryImageHdr.ErrorNumber);
			CONSOLE_DEBUG_W_NUM("ClientTransactionID\t=",		binaryImageHdr.ClientTransactionID);
			CONSOLE_DEBUG_W_NUM("ServerTransactionID\t=",		binaryImageHdr.ServerTransactionID);
			CONSOLE_DEBUG_W_NUM("DataStart\t\t=",				binaryImageHdr.DataStart);
			CONSOLE_DEBUG_W_NUM("ImageElementType\t=",			binaryImageHdr.ImageElementType);
			CONSOLE_DEBUG_W_NUM("TransmissionElementType\t=",	binaryImageHdr.TransmissionElementType);
			CONSOLE_DEBUG_W_NUM("Rank\t\t\t=",					binaryImageHdr.Rank);
			CONSOLE_DEBUG_W_NUM("Dimension1\t\t=",				binaryImageHdr.Dimension1);
			CONSOLE_DEBUG_W_NUM("Dimension2\t\t=",				binaryImageHdr.Dimension2);
			CONSOLE_DEBUG_W_NUM("Dimension3\t\t=",				binaryImageHdr.Dimension3);
		}

//		openCvImgPtr	=	cvCreateImage(cvSize(	binaryImageHdr.Dimension1, binaryImageHdr.Dimension2),
//													IPL_DEPTH_16U, 1);
//
		openCvImgPtr	=	cvCreateImage(cvSize(	binaryImageHdr.Dimension2, binaryImageHdr.Dimension1),
													IPL_DEPTH_16U, 1);
		if (openCvImgPtr != NULL)
		{
			CONSOLE_DEBUG_W_NUM("openCvImgPtr->width\t=",				openCvImgPtr->width);
			CONSOLE_DEBUG_W_NUM("openCvImgPtr->height\t=",				openCvImgPtr->height);
			CONSOLE_DEBUG_W_NUM("openCvImgPtr->depth\t=",				openCvImgPtr->depth);
			CONSOLE_DEBUG_W_NUM("openCvImgPtr->widthStep\t=",			openCvImgPtr->widthStep);

			dataByteCount	=	binaryImageHdr.Dimension1 * binaryImageHdr.Dimension2 * 2;
			pixelPtr		=	(unsigned char*)openCvImgPtr->imageData;
			numRead			=	fread(pixelPtr, dataByteCount, 1, filePointer);

			sprintf(jpegFileName, "image%dx%d.png", openCvImgPtr->width, openCvImgPtr->height);
			openCVerr	=	cvSaveImage(jpegFileName, openCvImgPtr, quality);
			if (openCVerr != 1)
			{
				CONSOLE_DEBUG_W_NUM("cvSaveImage returned", openCVerr);
			}
		}
		fclose(filePointer);
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
	CONSOLE_DEBUG_W_STR("extension\t=", extension);
	if ((strcmp(extension, "fits") == 0) ||
		(strcmp(extension, ".fit") == 0))
	{
		openCvImgPtr	=	ReadFITSimageIntoOpenCVimage(imageFileName);
	}
	else if ((strcmp(extension, "csv") == 0) ||
			(strcmp(extension, ".csv") == 0))
	{
		openCvImgPtr	=	NULL;
	}
	else if (strcmp(extension, ".bin") == 0)
	{
		openCvImgPtr	=	ReadAlpacaBinaryIntoOpenCVimage(imageFileName);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("imageFileName\t=", imageFileName);
		openCvImgPtr	=	cvLoadImage(imageFileName, CV_LOAD_IMAGE_COLOR);
	}
//	CONSOLE_DEBUG_W_HEX("openCvImgPtr\t=", openCvImgPtr);
	return(openCvImgPtr);
}


#endif // _USE_OPENCV_CPP_ && CV_MAJOR_VERSION

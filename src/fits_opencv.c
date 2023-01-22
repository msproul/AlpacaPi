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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Nov 16,	2021	<MLS> ReadFITSimageIntoOpenCVimage() now handles 8 bit RGB fits files
//*	Apr  9,	2022	<MLS> Fixed RGB color order in 8 bit color FITS files (fits=BGR)
//*	Sep 19,	2022	<MLS> Finished converting ReadFITSimageIntoOpenCVimage() to openCV C++
//*	Dec 26,	2022	<MLS> Added checking for null file name string
//*****************************************************************************

#include	<string.h>
#include	<stdio.h>
#include	<fitsio.h>
#include	<stdbool.h>

#ifdef _USE_OPENCV_CPP_
	#include	<opencv2/opencv.hpp>
	#include	<opencv2/core.hpp>
#else
//	#include "opencv/highgui.h"
	#include "opencv2/highgui/highgui_c.h"
	#include "opencv2/imgproc/imgproc_c.h"
	#include "opencv2/core/version.hpp"
#endif // _USE_OPENCV_CPP_

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#ifdef _ENABLE_NASA_PDS_
	#include	"PDS_ReadNASAfiles.h"
#endif

#include	"alpaca_defs.h"
#include	"fits_opencv.h"

//*****************************************************************************
static void	GetFitsErrorString(int fitsRetCode, char *errorString)
{
	switch(fitsRetCode)
	{
		//*	I will add to this list as needed
		case FILE_NOT_CREATED:	strcpy(errorString,	"FITS: could not create the named file");		break;
		case NUM_OVERFLOW:		strcpy(errorString,	"FITS: overflow during datatype conversion");	break;
		case FILE_NOT_OPENED:	strcpy(errorString,	"FITS: could not open the named file");	break;


		default:				sprintf(errorString, "Err not in table (%d)", fitsRetCode);	break;

	}
}

#if defined(_USE_OPENCV_CPP_)
//*****************************************************************************
cv::Mat	*ReadFITSimageIntoOpenCVimage(const char *fitsFileName)
{
cv::Mat			*openCvImgPtr;
fitsfile		*fptr;
int				fitsRetCode;
int				status;
int				nAxisfound;
long			naxes[3];
long			bitpix;
int				width;
int				height;
unsigned char	*pixelPtr;
char			*pixelBuffer;
int				ccc;
int				iii;
int				jjj;
long			firstPixel;
char			errorString[64];
int				errorCnt;
int				rgbOffset;
int				myWidthStep;
long			fitsDATAMIN;
long			fitsDATAMAX;
char			commentStr[128];
bool			dataMinMaxValid;
double			dataScaleFactor;
int				newDataMin;
int				newDataMax;

	CONSOLE_DEBUG(fitsFileName);


	openCvImgPtr	=	NULL;
	errorCnt		=	0;
	naxes[0]		=	0;
	naxes[1]		=	0;
	naxes[2]		=	0;
	status			=	0;
	newDataMin		=	70000;
	newDataMax		=	0;

	fitsRetCode		=	fits_open_file(&fptr, fitsFileName, READONLY, &status);
	if (fitsRetCode == 0)
	{
		//* read the NAXIS key words
		status		=	0;
		fitsRetCode	=	fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nAxisfound, &status);
//		CONSOLE_DEBUG_W_NUM("nAxisfound\t=",	nAxisfound);
//		CONSOLE_DEBUG_W_LONG("naxes[0]\t=",	naxes[0]);
//		CONSOLE_DEBUG_W_LONG("naxes[1]\t=",	naxes[1]);
//		CONSOLE_DEBUG_W_LONG("naxes[2]\t=",	naxes[2]);
//		CONSOLE_DEBUG_W_NUM("TSHORT\t=", TSHORT);
//		CONSOLE_DEBUG_W_NUM("TUSHORT\t=", TUSHORT);

		fitsDATAMIN	=	-1;
		fitsDATAMAX	=	-1;

		//-------------------------------------------------------------------
		//*	look for data min and max
		dataMinMaxValid	=	true;
		status		=	0;
		fitsRetCode	=	fits_read_key_lng(fptr, "DATAMIN", &fitsDATAMIN, commentStr, &status);
		if (status != 0)
		{
			dataMinMaxValid	=	false;
		}

		status		=	0;
		fitsRetCode	=	fits_read_key_lng(fptr, "DATAMAX", &fitsDATAMAX, commentStr, &status);
		if (status != 0)
		{
			dataMinMaxValid	=	false;
		}

		if (dataMinMaxValid)
		{
			dataScaleFactor	=	(fitsDATAMAX - fitsDATAMIN) / 65535.0;
//			CONSOLE_DEBUG_W_DBL("dataScaleFactor\t=",		dataScaleFactor);
		}


		status		=	0;
		bitpix		=	0;	//*	set a default
		fitsRetCode	=	fits_read_key_lng(fptr, "BITPIX", &bitpix, NULL, &status);
		CONSOLE_DEBUG_W_LONG("bitpix\t=",	bitpix);

		switch(bitpix)
		{
			case 8:
//				CONSOLE_DEBUG("case 8");
				//*	is it color?
				if (nAxisfound == 3)
				{
//					CONSOLE_DEBUG("nAxisfound == 3, Color image");
					//*	we have an 8 bit Color image
					width			=	naxes[0];
					height			=	naxes[1];
					myWidthStep		=	width * 3;
					firstPixel		=	1;
//-					openCvImgPtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
					openCvImgPtr	=	new cv::Mat(height, width, CV_8UC3);
					if (openCvImgPtr != NULL)
					{
//						CONSOLE_DEBUG("New MAT created");
						pixelBuffer	=	(char *)malloc(width * 3);
						if (pixelBuffer != NULL)
						{
							for (ccc=0; ccc<3; ccc++)
							{
								//*	fits files are BGR
								switch(ccc)
								{
									case 0:		rgbOffset	=	2;	break;
									case 1:		rgbOffset	=	1;	break;
									case 2:		rgbOffset	=	0;	break;
									default:	rgbOffset	=	0;	break;
								}
								pixelPtr	=	openCvImgPtr->data;

								for (iii=0; iii<height; iii++)
								{
									//*	now we are going to read in the image a row at a time
									status		=	0;
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
									//	pixelPtr[(jjj * 3) + ccc]	=	pixelBuffer[jjj];
										pixelPtr[(jjj * 3) + rgbOffset]	=	pixelBuffer[jjj];
									}

									pixelPtr	+=	myWidthStep;
									firstPixel	+=	width;
								}
							}
							free(pixelBuffer);
						}
						else
						{
							CONSOLE_DEBUG("malloc failed");
						}
					}
				}
				else if (nAxisfound == 2)
				{
					//*	we have an 8 bit B/W image
					width			=	naxes[0];
					height			=	naxes[1];
					myWidthStep		=	width;
					firstPixel		=	1;
//					openCvImgPtr	=	cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
					openCvImgPtr	=	new cv::Mat(height, width, CV_8UC1);
					if (openCvImgPtr != NULL)
					{
						pixelPtr	=	openCvImgPtr->data;
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
							pixelPtr	+=	myWidthStep;
							firstPixel	+=	width;
						}
					}
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("Invalid format; nAxisfound\t=",	nAxisfound);
				}
				break;

			case 16:
				if (nAxisfound == 2)
				{
					//*	we have what we are expecting, a 16 bit B/W image

					width			=	naxes[0];
					height			=	naxes[1];
					myWidthStep		=	width * 2;
					firstPixel		=	1;
					openCvImgPtr	=	new cv::Mat(height, width, CV_16UC1);
					if (openCvImgPtr != NULL)
					{
						pixelPtr	=	openCvImgPtr->data;
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
							//*	if the data min/max values were in the FITS header
							//*	we can stretch the image accordingly.
							if (dataMinMaxValid && (dataScaleFactor > 0))
							{
							unsigned short	*uShortPtr;
							int				qqq;
							int				oldPixValue;
							int				newPixValue;

								uShortPtr	=	(unsigned short *)pixelPtr;
								for (qqq=0; qqq<width; qqq++)
								{
									oldPixValue		=	uShortPtr[qqq] & 0x00ffff;
									newPixValue		=	(oldPixValue - fitsDATAMIN) / dataScaleFactor;
									uShortPtr[qqq]	=	newPixValue;

									//*	keep track of new min/max
									if (newPixValue < newDataMin)
									{
										newDataMin	=	newPixValue;
									}
									if (newPixValue > newDataMax)
									{
										newDataMax	=	newPixValue;
									}
								}
							}

							if (fitsRetCode != 0)
							{
								errorCnt++;
								CONSOLE_DEBUG_W_NUM("fitsRetCode\t=",	fitsRetCode);
								GetFitsErrorString(fitsRetCode, errorString);
								CONSOLE_DEBUG_W_STR("fits_write_pix returned:", errorString);
							}
							pixelPtr	+=	myWidthStep;
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
	else
	{
		CONSOLE_DEBUG_W_NUM("fits_open_file failed with error code\t=",	fitsRetCode);
		GetFitsErrorString(fitsRetCode, errorString);
		CONSOLE_DEBUG_W_STR("fits_open_file returned:", errorString);

	}
	if (errorCnt > 0)
	{
		CONSOLE_DEBUG_W_NUM("errorCnt\t=",	errorCnt);
	}
//	CONSOLE_DEBUG_W_NUM("newDataMin\t=",	newDataMin);
//	CONSOLE_DEBUG_W_NUM("newDataMax\t=",	newDataMax);

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");

	return(openCvImgPtr);
}

#ifdef _ENABLE_NASA_PDS_
//*****************************************************************************
cv::Mat	*ReadPDSimageIntoOpenCVimage(const char *imageFileName)
{
cv::Mat			*openCvImgPtr;
PDS_header_data	pdsHeader;
bool			validPDSimage;
int				width;
int				height;
unsigned char	*pixelPtr;
size_t			pixelCount;
int				divideFactor;
int				iii;
int				jjj;
int				pdsImgPixIdx;
int				opnCVPixIdx;
int				opnCVlineNum;
uint8_t			pixelValue;

	CONSOLE_DEBUG("***************************************************************");

	openCvImgPtr	=	NULL;
	validPDSimage	=	PDS_ReadImage(imageFileName, &pdsHeader);
	if (validPDSimage)
	{
		if (pdsHeader.imageData != NULL)
		{
			width			=	pdsHeader.lineSamples;
			height			=	pdsHeader.scanLines;

			divideFactor	=	1;
//			while ((width > 1200) || (height > 900))
//			{
//				divideFactor++;
//				width			=	pdsHeader.lineSamples / divideFactor;
//				height			=	pdsHeader.scanLines / divideFactor;
//			}
//			CONSOLE_DEBUG_W_NUM("divideFactor\t=", divideFactor);
			openCvImgPtr	=	new cv::Mat(height, width, CV_8UC1);
			if (openCvImgPtr != NULL)
			{
//				CONSOLE_DEBUG(__FUNCTION__);
//				CONSOLE_DEBUG_W_NUM("width \t=", width);
//				CONSOLE_DEBUG_W_NUM("height\t=", height);
				pixelPtr	=	openCvImgPtr->data;
				pixelCount	=	width * height;
				if (divideFactor == 1)
				{
				//	CONSOLE_DEBUG(__FUNCTION__);
					memcpy(pixelPtr, pdsHeader.imageData, pixelCount);
				}
				else
				{
					//*	we are scaling the image down
//					CONSOLE_DEBUG("we are scaling the image down");
//					CONSOLE_DEBUG_W_NUM("scanLines  \t=", pdsHeader.scanLines);
//					CONSOLE_DEBUG_W_NUM("lineSamples\t=", pdsHeader.lineSamples);
					opnCVlineNum	=	0;
					for (jjj=0; jjj<pdsHeader.scanLines; jjj+=divideFactor)
					{
						opnCVPixIdx		=	opnCVlineNum * width;
						for (iii=0; iii<pdsHeader.lineSamples; iii+=divideFactor)
						{
							pdsImgPixIdx	=	jjj * pdsHeader.lineSamples;
							pdsImgPixIdx	+=	iii;
							pixelValue		=	pdsHeader.imageData[pdsImgPixIdx] & 0x00ff;

							if (opnCVPixIdx < pixelCount)
							{
								pixelPtr[opnCVPixIdx]	=	pixelValue;
								opnCVPixIdx++;
							}
							else
							{
//								CONSOLE_DEBUG_W_NUM("opnCVPixIdx\t=", opnCVPixIdx);
//								CONSOLE_DEBUG_W_NUM("pixelCount \t=", pixelCount);
								break;
							//	CONSOLE_ABORT(__FUNCTION__);
							}
						}
						opnCVlineNum++;
					}
				}
			}
			free(pdsHeader.imageData);
		}
		else
		{
			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	CONSOLE_DEBUG(__FUNCTION__);
	return(openCvImgPtr);
}
#endif // _ENABLE_NASA_PDS_


#if defined(_USE_OPENCV_CPP_)
cv::Mat		gOpenCvImg;
#endif


//*****************************************************************************
//*	Function:	Reads an image into opencv data structure.
//*				uses the extension to determine how to read it
//*****************************************************************************
cv::Mat	*ReadImageIntoOpenCVimage(const char *imageFileName)
{
cv::Mat		*openCvImgPtr;
int			fnameLen;
char		extension[8];

//	CONSOLE_DEBUG(__FUNCTION__);
//	for reference,
//#include	"/usr/local/include/opencv4/opencv2/core/mat.hpp"
	openCvImgPtr	=	NULL;
	if (imageFileName != NULL)
	{
		fnameLen		=	strlen(imageFileName);
		strcpy(extension, &imageFileName[fnameLen - 4]);
		CONSOLE_DEBUG_W_STR("extension\t=", extension);
		if ((strcasecmp(extension, "fits") == 0) ||
			(strcasecmp(extension, ".fit") == 0))
		{
			openCvImgPtr	=	ReadFITSimageIntoOpenCVimage(imageFileName);
		}
		else if ((strcasecmp(extension, "csv") == 0) ||
				(strcasecmp(extension, ".csv") == 0))
		{
			openCvImgPtr	=	NULL;
		}
		else if (strcasecmp(extension, ".bin") == 0)
		{
	//		openCvImgPtr	=	ReadAlpacaBinaryIntoOpenCVimage(imageFileName);
		}
	#ifdef _ENABLE_NASA_PDS_
		else if (	(strcasecmp(extension, ".img") == 0) ||
					(strcasecmp(extension, ".imq") == 0) ||
					(strcasecmp(extension, ".lbl") == 0) ||
					(strcasecmp(extension, ".red") == 0) ||
					(strcasecmp(extension, ".grn") == 0) ||
					(strcasecmp(extension, ".sgr") == 0) ||
					(strcasecmp(extension, ".vio") == 0)
					)
		{
			openCvImgPtr	=	ReadPDSimageIntoOpenCVimage(imageFileName);
		}
	#endif // _ENABLE_NASA_PDS_
		else if ((strcasecmp(extension, ".jpg") == 0) ||
				(strcasecmp(extension, ".png") == 0) ||
				(strcasecmp(extension, ".gif") == 0))
		{
			CONSOLE_DEBUG_W_STR("imageFileName\t=", imageFileName);
			#warning "OpenCV++ not finished"
		#if defined(_USE_OPENCV_CPP_)
			gOpenCvImg		=	cv::imread(imageFileName);
			openCvImgPtr	=	&gOpenCvImg;
		#else
			openCvImgPtr	=	cvLoadImage(imageFileName, CV_LOAD_IMAGE_COLOR);
		#endif
		}
	//	CONSOLE_DEBUG_W_HEX("openCvImgPtr\t=", openCvImgPtr);
	}
	else
	{
		CONSOLE_DEBUG("imageFileName == NULL!!!!!!!!!!!!!!!!!!!!");
	}
	return(openCvImgPtr);
}


#else
//*****************************************************************************
IplImage	*ReadFITSimageIntoOpenCVimage(const char *fitsFileName)
{
fitsfile		*fptr;
int				fitsRetCode;
int				status;
IplImage		*openCvImgPtr;
int				nAxisfound;
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
char			errorString[64];
int				errorCnt;
int				rgbOffset;

	CONSOLE_DEBUG(__FUNCTION__);
	openCvImgPtr	=	NULL;
	if (fitsFileName != NULL)
	{
		CONSOLE_DEBUG_W_STR("File:", fitsFileName);

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
			fitsRetCode	=	fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nAxisfound, &status);
	//		CONSOLE_DEBUG_W_NUM("nAxisfound\t=",	nAxisfound);
	//		CONSOLE_DEBUG_W_LONG("naxes[0]\t=",	naxes[0]);
	//		CONSOLE_DEBUG_W_LONG("naxes[1]\t=",	naxes[1]);
	//		CONSOLE_DEBUG_W_LONG("naxes[2]\t=",	naxes[2]);
	//		CONSOLE_DEBUG_W_NUM("TSHORT\t=", TSHORT);
	//		CONSOLE_DEBUG_W_NUM("TUSHORT\t=", TUSHORT);

			status		=	0;
			bitpix		=	0;	//*	set a default
			fitsRetCode	=	fits_read_key_lng(fptr, "BITPIX", &bitpix, NULL, &status);
	//		CONSOLE_DEBUG_W_LONG("bitpix\t=",	bitpix);

			switch(bitpix)
			{
				case 8:
					//*	is it color?
					if (nAxisfound == 3)
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
									//*	fits files are BGR
									switch(ccc)
									{
										case 0:		rgbOffset	=	2;	break;
										case 1:		rgbOffset	=	1;	break;
										case 2:		rgbOffset	=	0;	break;
										default:	rgbOffset	=	0;	break;
									}
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
										//	pixelPtr[(jjj * 3) + ccc]	=	pixelBuffer[jjj];
											pixelPtr[(jjj * 3) + rgbOffset]	=	pixelBuffer[jjj];
										}

										pixelPtr	+=	openCvImgPtr->widthStep;
										firstPixel	+=	width;
									}
								}
								free(pixelBuffer);
							}
						}
					}
					else if (nAxisfound == 2)
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
						CONSOLE_DEBUG_W_NUM("Invalid format; nAxisfound\t=",	nAxisfound);
					}
					break;

				case 16:
					if (nAxisfound == 2)
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
									CONSOLE_DEBUG_W_NUM("fitsRetCode\t=",	fitsRetCode);
									GetFitsErrorString(fitsRetCode, errorString);
									CONSOLE_DEBUG_W_STR("fits_write_pix returned:", errorString);
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
		else
		{
			CONSOLE_DEBUG_W_NUM("fits_open_file failed with error code\t=",	fitsRetCode);
			GetFitsErrorString(fitsRetCode, errorString);
			CONSOLE_DEBUG_W_STR("fits_open_file returned:", errorString);

		}
		if (errorCnt > 0)
		{
			CONSOLE_DEBUG_W_NUM("errorCnt\t=",	errorCnt);
		}
	}
	else
	{
		CONSOLE_DEBUG("fitsFileName == NULL!!!!!!!!!!!!!!!!!!!!");
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
		#if (CV_MAJOR_VERSION <= 3)
			#warning "CV_MAJOR_VERSION <= 3"
			openCVerr	=	cvSaveImage(jpegFileName, openCvImgPtr, quality);
			if (openCVerr != 1)
			{
				CONSOLE_DEBUG_W_NUM("cvSaveImage returned", openCVerr);
			}
		#elif (CV_MAJOR_VERSION <= 4)
			#warning "CV_MAJOR_VERSION <= 4"
		#endif
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
	if (imageFileName != NULL)
	{
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
		#if (CV_MAJOR_VERSION <= 3)
			#warning "CV_MAJOR_VERSION <= 3"
			openCvImgPtr	=	cvLoadImage(imageFileName, CV_LOAD_IMAGE_COLOR);
		#elif (CV_MAJOR_VERSION <= 4)
			#warning "CV_MAJOR_VERSION <= 4"
		#endif
		}
	//	CONSOLE_DEBUG_W_HEX("openCvImgPtr\t=", openCvImgPtr);
	}
	else
	{
		CONSOLE_DEBUG("imageFileName == NULL!!!!!!!!!!!!!!!!!!!!");
	}
	return(openCvImgPtr);
}

#endif // _USE_OPENCV_CPP_ && CV_MAJOR_VERSION

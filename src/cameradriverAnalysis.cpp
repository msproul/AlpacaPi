//**************************************************************************
//*	Name:			cameradriverAnalysis.cpp
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
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
//*
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*
//*	Fits Info
//*		http://tigra-astronomy.com/sbfitsext-guidelines-for-fits-keywords
//*		http://iraf.noao.edu/projects/ccdmosaic/imagedef/fitsdic.html
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 24,	2019	<MLS> Started on camera analysis code
//*	Nov 24,	2019	<MLS> Added CalculateMaxPixValue()
//*	Dec  5,	2019	<MLS> Added CalculateMinPixValue()
//*	Dec 18,	2019	<MLS> Added CalculateSaturation()
//*	Dec 18,	2019	<MLS> Finished RGB24 processing
//*	Dec 18,	2019	<MLS> Added AutoAdjustExposure()
//*	Dec 18,	2019	<MLS> Added CountSaturationPixels()
//*	Dec 21,	2019	<MLS> Worked on trying to use a PID library, did not work
//*	Dec 25,	2019	<MLS> Added CalculateHistogramArray()
//*	Dec 26,	2019	<MLS> Added SaveHistogramFile()
//*	Jan 12,	2020	<MLS> Added better limit checking to AutoAdjustExposure()
//*	Feb 15,	2020	<MLS> Fixed negative exposure bug in AutoAdjustExposure()
//**************************************************************************

#ifdef _ENABLE_CAMERA_

#include	<stdio.h>
#include	<string.h>

#if defined(__arm__)
	#include <wiringPi.h>
#endif

//#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"



//**************************************************************************
//*	Calculate the minimum pixel value for the current data buffer
//**************************************************************************
uint32_t	CameraDriver::CalculateMinPixValue(void)
{
uint32_t		minPixelValue;
uint32_t		imageDataLen;
uint32_t		currPixValue;
uint32_t		ii;
uint32_t		cc;
uint16_t		*imageDataPtr16bit;
uint8_t			*imageDataPtr8bit;
uint32_t		redValue;
uint32_t		grnValue;
uint32_t		bluValue;

//	CONSOLE_DEBUG(__FUNCTION__);

	minPixelValue	=	65535;
	imageDataLen	=	cCameraXsize * cCameraYsize;

	if (cCameraDataBuffer != NULL)
	{
		GetImage_ROI_info();

		switch(cROIinfo.currentROIimageType)
		{
			case kImageType_RAW8:
			case kImageType_Y8:
				imageDataPtr8bit	=	(uint8_t *)cCameraDataBuffer;
				for (ii=0; ii<imageDataLen; ii++)
				{
					currPixValue	=	imageDataPtr8bit[ii] & 0x00ff;
					if (currPixValue < minPixelValue)
					{
						minPixelValue	=	currPixValue;
					}
				}
				break;

			case kImageType_RAW16:
				imageDataPtr16bit	=	(uint16_t *)cCameraDataBuffer;
				for (ii=0; ii<imageDataLen; ii++)
				{
					currPixValue	=	imageDataPtr16bit[ii] & 0x00ffff;
					if (currPixValue < minPixelValue)
					{
						minPixelValue	=	currPixValue;
					}
				}
				break;


			case kImageType_RGB24:
				imageDataPtr8bit	=	(uint8_t *)cCameraDataBuffer;
				cc					=	0;
				for (ii=0; ii<imageDataLen; ii++)
				{
					redValue	=	imageDataPtr8bit[cc + 0] & 0x00ff;
					grnValue	=	imageDataPtr8bit[cc + 1] & 0x00ff;
					bluValue	=	imageDataPtr8bit[cc + 2] & 0x00ff;
					if (redValue < minPixelValue)
					{
						minPixelValue	=	redValue;
					}
					if (grnValue < minPixelValue)
					{
						minPixelValue	=	grnValue;
					}
					if (bluValue < minPixelValue)
					{
						minPixelValue	=	bluValue;
					}
					cc	+=	3;
				}
				break;

			default:
				minPixelValue	=	0;
				break;

		}

	}
	CONSOLE_DEBUG_W_NUM("minPixelValue\t=",	minPixelValue);
	return(minPixelValue);
}

//**************************************************************************
//*	Calculate the maximum pixel value for the current data buffer
//**************************************************************************
uint32_t	CameraDriver::CalculateMaxPixValue(void)
{
uint32_t		maxPixelValue;
uint32_t		imageDataLen;
uint32_t		currPixValue;
uint32_t		ii;
uint32_t		cc;
uint16_t		*imageDataPtr16bit;
uint8_t			*imageDataPtr8bit;
uint32_t		redValue;
uint32_t		grnValue;
uint32_t		bluValue;

//	CONSOLE_DEBUG(__FUNCTION__);

	maxPixelValue	=	0;
	imageDataLen	=	cCameraXsize * cCameraYsize;

	if (cCameraDataBuffer != NULL)
	{
		GetImage_ROI_info();

		switch(cROIinfo.currentROIimageType)
		{
			case kImageType_RAW8:
			case kImageType_Y8:
				imageDataPtr8bit	=	(uint8_t *)cCameraDataBuffer;
				for (ii=0; ii<imageDataLen; ii++)
				{
					currPixValue	=	imageDataPtr8bit[ii] & 0x00ff;
					if (currPixValue > maxPixelValue)
					{
						maxPixelValue	=	currPixValue;
					}
				}
				break;

			case kImageType_RAW16:
				imageDataPtr16bit	=	(uint16_t *)cCameraDataBuffer;
				for (ii=0; ii<imageDataLen; ii++)
				{
					currPixValue	=	imageDataPtr16bit[ii] & 0x00ffff;
					if (currPixValue > maxPixelValue)
					{
						maxPixelValue	=	currPixValue;
					}
				}
				break;


			case kImageType_RGB24:
				imageDataPtr8bit	=	(uint8_t *)cCameraDataBuffer;
				cc					=	0;
				for (ii=0; ii<imageDataLen; ii++)
				{
					redValue	=	imageDataPtr8bit[cc + 0] & 0x00ff;
					grnValue	=	imageDataPtr8bit[cc + 1] & 0x00ff;
					bluValue	=	imageDataPtr8bit[cc + 2] & 0x00ff;
					if (redValue > maxPixelValue)
					{
						maxPixelValue	=	redValue;
					}
					if (grnValue > maxPixelValue)
					{
						maxPixelValue	=	grnValue;
					}
					if (bluValue > maxPixelValue)
					{
						maxPixelValue	=	bluValue;
					}
					cc	+=	3;
				}
				break;

			default:
				maxPixelValue	=	0;
				break;

		}

	}
//	CONSOLE_DEBUG_W_INT32("maxPixelValue\t=", maxPixelValue);
	return(maxPixelValue);
}


//**************************************************************************
//*	Count the number of pixels at saturation
//*		for raw8, a saturated pixel is one that has a value of 255
//*		for raw16, a saturated pixel is one that has a value of 65535
//*		for RGB24, if any of the 3 RGB values is 255, then that pixel is at saturation
//**************************************************************************
uint32_t	CameraDriver::CountSaturationPixels(void)
{
uint32_t		imageDataLen;
uint32_t		currPixValue;
uint32_t		saturatedPixCnt;
uint32_t		ii;
uint32_t		cc;
uint32_t		redValue;
uint32_t		grnValue;
uint32_t		bluValue;
uint16_t		*imageDataPtr16bit;
uint8_t			*imageDataPtr8bit;

//	CONSOLE_DEBUG(__FUNCTION__);

	imageDataLen	=	cCameraXsize * cCameraYsize;
	saturatedPixCnt	=	0;

	if (cCameraDataBuffer != NULL)
	{
		GetImage_ROI_info();

		switch(cROIinfo.currentROIimageType)
		{
			case kImageType_RAW8:
			case kImageType_Y8:
				imageDataPtr8bit	=	(uint8_t *)cCameraDataBuffer;
				for (ii=0; ii<imageDataLen; ii++)
				{
					currPixValue	=	imageDataPtr8bit[ii] & 0x00ff;
					if (currPixValue == 0x0ff)
					{
						saturatedPixCnt++;
					}
				}
				break;

			case kImageType_RAW16:
				imageDataPtr16bit	=	(uint16_t *)cCameraDataBuffer;
				for (ii=0; ii<imageDataLen; ii++)
				{
					currPixValue	=	imageDataPtr16bit[ii] & 0x00ffff;
					if (currPixValue == 0x0ffff)
					{
						saturatedPixCnt++;
					}
				}
				break;


			case kImageType_RGB24:
				imageDataPtr8bit	=	(uint8_t *)cCameraDataBuffer;
				cc					=	0;
				for (ii=0; ii<imageDataLen; ii++)
				{
					redValue	=	imageDataPtr8bit[cc + 0] & 0x00ff;
					grnValue	=	imageDataPtr8bit[cc + 1] & 0x00ff;
					bluValue	=	imageDataPtr8bit[cc + 2] & 0x00ff;
					if ((redValue == 0x0ff) || (grnValue == 0x0ff) || (bluValue == 0x0ff))
					{
						saturatedPixCnt++;
					}
					cc	+=	3;
				}
				break;

			default:
				saturatedPixCnt	=	0;
				break;

		}
	}

	return(saturatedPixCnt);
}



//**************************************************************************
//*	Calculate the precentage of saturated pixels
//*		for raw8, a saturated pixel is one that has a value of 255
//*		for raw16, a saturated pixel is one that has a value of 65535
//*		for RGB24, if any of the 3 RGB values is 255, then that pixel is at saturation
//*	Return value is a percentage, (0.0 -> 100.0)
//**************************************************************************
float	CameraDriver::CalculateSaturation(void)
{
float			saturatedPrct;
uint32_t		saturatedPixCnt;
uint32_t		imageDataLen;

//	CONSOLE_DEBUG(__FUNCTION__);

	saturatedPixCnt	=	CountSaturationPixels();
	imageDataLen	=	cCameraXsize * cCameraYsize;
	saturatedPrct	=	(saturatedPixCnt * 100.0) / imageDataLen;

//	CONSOLE_DEBUG_W_INT32("saturatedPixCnt\t=",	saturatedPixCnt);
//	CONSOLE_DEBUG_W_DBL("saturatedPrct\t=",		saturatedPrct);

	return(saturatedPrct);
}


//**************************************************************************
//*	returns the maximum pixel value as a percentage
//**************************************************************************
float	CameraDriver::CalculateHistogramMax(void)
{
float			histogramMaxPrct;
uint32_t		maxPixelValue;

//	CONSOLE_DEBUG(__FUNCTION__);

	maxPixelValue	=	CalculateMaxPixValue();
//	CONSOLE_DEBUG_W_INT32("maxPixelValue\t",	maxPixelValue);

	switch(cROIinfo.currentROIimageType)
	{
		case kImageType_RAW8:
		case kImageType_Y8:
		case kImageType_RGB24:
			histogramMaxPrct	=	(100.0 * maxPixelValue) / 255.0;
			break;

		case kImageType_RAW16:
			histogramMaxPrct	=	(100.0 * maxPixelValue) / 65535.0;
			break;


			break;

		default:
			histogramMaxPrct	=	0;
			break;

	}


	return(histogramMaxPrct);
}


//*****************************************************************************
void	CameraDriver::AutoAdjustExposure(void)
{
//uint32_t	maxPixelValue;
float			saturationPrct;
float			histogrmMaxPrct;
float			histogramErr;
long			exposureAdjustment_us;	//*	micro-seconds

//	CONSOLE_DEBUG(__FUNCTION__);

	saturationPrct	=	CalculateSaturation();
	histogrmMaxPrct	=	CalculateHistogramMax();
//	CONSOLE_DEBUG_W_DBL("saturationPrct\t=",	saturationPrct);

	if ((histogrmMaxPrct >= 90.0) && (histogrmMaxPrct < 100.0))
	{
		CONSOLE_DEBUG_W_DBL("PERFECT: Histogram %\t=",	histogrmMaxPrct);
	}
	else if (saturationPrct > 0.0)
	{
		exposureAdjustment_us	=	1;
		if (saturationPrct > 50.0)
		{
			//*	reduce by 1/2
			exposureAdjustment_us	=	cCurrentExposure_us / 2;
		}
		else if (saturationPrct > 40.0)
		{
			//*	subtract 33%
			exposureAdjustment_us	=	(cCurrentExposure_us / 3);
		}
		else if (saturationPrct > 20.0)
		{
			//*	subtract 25%
			exposureAdjustment_us	=	(cCurrentExposure_us / 4);
		}
		else if (saturationPrct > 5.0)
		{
			//*	subtract 10%
			exposureAdjustment_us	=	(cCurrentExposure_us / 10);
		}
		else if (saturationPrct > 1.0)
		{
			//*	subtract 5%
			exposureAdjustment_us	=	(cCurrentExposure_us / 20);
		}
		else if (saturationPrct > 0.5)
		{
			//*	subtract 2%
			exposureAdjustment_us	=	(cCurrentExposure_us / 50);
		}
		else if (saturationPrct > 0.1)
		{
			//*	subtract 1%
			exposureAdjustment_us	=	(cCurrentExposure_us / 100);
		}
		else if (saturationPrct > 0.01)
		{
			//*	subtract 0.5%
			exposureAdjustment_us	=	(cCurrentExposure_us / 200);
		}
		else if (saturationPrct > 0.001)
		{
			//*	subtract 0.2%
			exposureAdjustment_us	=	(cCurrentExposure_us / 500);
		}
		else if (saturationPrct > 0.0001)
		{
			//*	subtract 0.1%
			exposureAdjustment_us	=	(cCurrentExposure_us / 1000);
		}


		if (exposureAdjustment_us < cAutoAdjustStepSz_us)
		{
			exposureAdjustment_us	=	cAutoAdjustStepSz_us;
		}
//		CONSOLE_DEBUG_W_INT32("exposureAdjustment_us\t=",	exposureAdjustment_us);
		cCurrentExposure_us	-=	exposureAdjustment_us;
		if ((cCurrentExposure_us < cExposureMin_us) || (cCurrentExposure_us < 0))
		{
			cCurrentExposure_us	=	cExposureMin_us;
			//*	now double check
			if (cCurrentExposure_us < 1)
			{
				cCurrentExposure_us	=	1;
			}
		}
	}
	else
	{
	double	curExposure_secs;
	double	newExposure_secs;
	double	deltaExposure_secs;

		histogramErr	=	95.0 - histogrmMaxPrct;

		if (histogramErr > 0.0)
		{
//			CONSOLE_DEBUG_W_DBL("histogrmMaxPrct\t=",	histogrmMaxPrct);
//			CONSOLE_DEBUG_W_DBL("histogramErr\t=",	histogramErr);
			curExposure_secs	=	(1.0 * cCurrentExposure_us) / 1000000.0;
			deltaExposure_secs	=	(histogramErr / 100.0) * curExposure_secs;
			newExposure_secs	=	curExposure_secs + deltaExposure_secs;
//			CONSOLE_DEBUG_W_DBL("curExposure_secs\t=",	curExposure_secs);
//			CONSOLE_DEBUG_W_DBL("newExposure_secs\t=",	newExposure_secs);
			cCurrentExposure_us	=	newExposure_secs * 1000000;

		}
		else if (histogrmMaxPrct < 5.0)
		{
			//*	double
			cCurrentExposure_us	+=	cCurrentExposure_us;
		}
		else if (histogrmMaxPrct < 30.0)
		{
			cCurrentExposure_us	+=	(5 * cAutoAdjustStepSz_us);
		}
		else if (histogrmMaxPrct < 85.0)
		{
			cCurrentExposure_us	+=	cAutoAdjustStepSz_us;
		}

		//*	check the limits
		if (cCurrentExposure_us < cExposureMin_us)
		{
			cCurrentExposure_us	=	cExposureMin_us;
		}
		if (cCurrentExposure_us > cExposureMax_us)
		{
			cCurrentExposure_us	=	cExposureMax_us;
		}

		//*	now check the EXTREAME limits
		if (cCurrentExposure_us < 0)
		{
			cCurrentExposure_us	=	100;
		}
		if (cCurrentExposure_us > (10 * 60 * 1000 * 1000))	//*	10 minutes
		{
			cCurrentExposure_us	=	10 * 60 * 1000 * 1000;
		}

	}
//	CONSOLE_DEBUG_W_INT32("cCurrentExposure_us\t=",	cCurrentExposure_us);
}


#ifdef _INCLUDE_HISTOGRAM_
//*****************************************************************************
void	CameraDriver::CalculateHistogramArray(void)
{
int32_t			imageDataLen;
int32_t			currPixValue;
int32_t			ii;
int32_t			cc;
uint16_t		*imageDataPtr16bit;
uint8_t			*imageDataPtr8bit;
int32_t			redValue;
int32_t			grnValue;
int32_t			bluValue;
int32_t			lumValue;		//*	luminance value
int32_t			peakPixelIdx;
int32_t			peakPixelCount;
bool			lookingForMin;

	SETUP_TIMING();

	CONSOLE_DEBUG(__FUNCTION__);
	START_TIMING();

	imageDataLen	=	cCameraXsize * cCameraYsize;

	if (cCameraDataBuffer != NULL)
	{
		cPeakHistogramValue	=	0;
		cMaxHistogramValue	=	0;
		cMaxHistogramPixCnt	=	0;
		cMaxRedValue		=	0;
		cMaxGrnValue		=	0;
		cMaxBluValue		=	0;
		cMaxGryValue		=	0;

		//*	clear out the histogram data array
		memset(cHistogramLum,	0,	sizeof(cHistogramLum));
		memset(cHistogramRed,	0,	sizeof(cHistogramRed));
		memset(cHistogramGrn,	0,	sizeof(cHistogramGrn));
		memset(cHistogramBlu,	0,	sizeof(cHistogramBlu));

		//*	figure out what type of image it is
		GetImage_ROI_info();
		switch(cROIinfo.currentROIimageType)
		{
			case kImageType_RAW8:
			case kImageType_Y8:
				imageDataPtr8bit	=	(uint8_t *)cCameraDataBuffer;
				for (ii=0; ii<imageDataLen; ii++)
				{
					currPixValue	=	imageDataPtr8bit[ii] & 0x00ff;

					cHistogramLum[currPixValue]++;

					if (currPixValue > cMaxGryValue)
					{
						cMaxGryValue	=	currPixValue;
					}
				}
				break;

			case kImageType_RAW16:
				imageDataPtr16bit	=	(uint16_t *)cCameraDataBuffer;
				for (ii=0; ii<imageDataLen; ii++)
				{
					//*	for 16 bit data, we shift it right 8 bits
					currPixValue	=	imageDataPtr16bit[ii] & 0x00ffff;
					currPixValue	=	(currPixValue >> 8) & 0x00ff;
					cHistogramLum[currPixValue]++;
				}
				break;


			case kImageType_RGB24:
				imageDataPtr8bit	=	(uint8_t *)cCameraDataBuffer;
				cc					=	0;
				for (ii=0; ii<imageDataLen; ii++)
				{
					//*	openCV uses BGR instead of RGB
					bluValue	=	imageDataPtr8bit[cc + 0] & 0x00ff;
					grnValue	=	imageDataPtr8bit[cc + 1] & 0x00ff;
					redValue	=	imageDataPtr8bit[cc + 2] & 0x00ff;
					cHistogramRed[redValue]++;
					cHistogramGrn[grnValue]++;
					cHistogramBlu[bluValue]++;

					if (redValue > cMaxRedValue)
					{
						cMaxRedValue	=	redValue;
					}
					if (grnValue > cMaxGrnValue)
					{
						cMaxGrnValue	=	grnValue;
					}
					if (bluValue > cMaxBluValue)
					{
						cMaxBluValue	=	bluValue;
					}

					cc	+=	3;
				}
				//*	now calculate the luminance
				for (ii=0; ii<256; ii++)
				{
					lumValue	=	cHistogramRed[ii];
					lumValue	+=	cHistogramGrn[ii];
					lumValue	+=	cHistogramBlu[ii];
					lumValue	=	(lumValue / 3);

					cHistogramLum[ii]	=	lumValue;
				}

				break;

			default:
				break;

		}
		//*	now go through the array and find the peak value and max value
		peakPixelIdx		=	-1;
		peakPixelCount		=	0;
		lookingForMin		=	true;
		for (ii=0; ii<256; ii++)
		{
			//*	find the minimum value
			if (lookingForMin && (cHistogramLum[ii] > 0))
			{
				cMinHistogramValue	=	ii;
				lookingForMin		=	false;
			}
			//*	find the maximum value
			if (cHistogramLum[ii] > 0)
			{
				cMaxHistogramValue	=	ii;
			}

			//*	find the peak value
			if (cHistogramLum[ii] > peakPixelCount)
			{
				peakPixelIdx	=	ii;
				peakPixelCount	=	cHistogramLum[ii];
			}


		}
		cPeakHistogramValue	=	peakPixelIdx;

		//*	look for maximum pixel counts
		//*	purposely skip the first and last
		cMaxHistogramPixCnt	=	0;
//		for (ii=5; ii<250; ii++)
//		for (ii=1; ii<255; ii++)
		for (ii=0; ii<256; ii++)
		{
			if (cHistogramLum[ii] > cMaxHistogramPixCnt)
			{
				cMaxHistogramPixCnt	=	cHistogramLum[ii];
			}
			if (cHistogramRed[ii] > cMaxHistogramPixCnt)
			{
				cMaxHistogramPixCnt	=	cHistogramRed[ii];
			}
			if (cHistogramGrn[ii] > cMaxHistogramPixCnt)
			{
				cMaxHistogramPixCnt	=	cHistogramGrn[ii];
			}
			if (cHistogramBlu[ii] > cMaxHistogramPixCnt)
			{
				cMaxHistogramPixCnt	=	cHistogramBlu[ii];
			}
		}

		DEBUG_TIMING("Time to save calculate histogram file (milliseconds)\t=");
	}
	else
	{
		CONSOLE_DEBUG("cCameraDataBuffer is NULL");
	}
}

//*****************************************************************************
void	CameraDriver::SaveHistogramFile(void)
{
char	csvPathName[256];
char	csvFileName[256];
int		ii;
FILE	*csvFile;

	strcpy(csvFileName, cFileNameRoot);
	strcat(csvFileName, ".csv");

	strcpy(csvPathName, kImageDataDir);
	strcat(csvPathName, "/");
	strcat(csvPathName, csvFileName);

	csvFile	=	fopen(csvPathName, "w");
	if (csvFile != NULL)
	{
		if (cROIinfo.currentROIimageType == kImageType_RGB24)
		{
			//*	print out lum, red, grn, blu
			for (ii=0; ii<256; ii++)
			{
				fprintf(csvFile,	"%d,%d,%d,%d,%d\n", ii,	cHistogramLum[ii],
															cHistogramRed[ii],
															cHistogramGrn[ii],
															cHistogramBlu[ii]);
			}
		}
		else
		{
			for (ii=0; ii<256; ii++)
			{
				fprintf(csvFile,	"%d,%d\n", ii, cHistogramLum[ii]);
			}
		}

		fclose(csvFile);
		AddToDataProductsList(csvFileName, "Histogram data");
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to create .csv file:", csvFileName);
	}
}


#endif // _INCLUDE_HISTOGRAM_



#endif	//	_ENABLE_CAMERA_

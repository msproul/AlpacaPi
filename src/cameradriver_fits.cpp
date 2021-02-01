//**************************************************************************
//*	Name:			cameradriver_fits.cpp
//*
//*	Author:			Mark Sproul (C) 2019-2020
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
//*		https://free-astro.org/index.php?title=Siril:FITS_orientation
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov  2,	2019	<MLS> Added SaveImageAsFITS()
//*	Nov  3,	2019	<MLS> Added support for FITS file output
//*	Nov  3,	2019	<MLS> Successfully created a FITS image with ZWO camera
//*	Nov 11,	2019	<MLS> Added telescope info comments output in FITS file
//*	Nov 13,	2019	<MLS> 16 bit FITS working on ZWO camera
//*	Nov 13,	2019	<MLS> RGB24 crashes fits processing
//*	Nov 14,	2019	<MLS> Added CreateFitsBGRimage()
//*	Nov 14,	2019	<MLS> RGB24 working in FITS, note: color order is BGR
//*	Nov 16,	2019	<MLS> Added filter wheel info to FITS output
//*	Nov 30,	2019	<MLS> Switched to cfitsio checksum routines (fits_write_chksum)
//*	Dec  5,	2019	<MLS> Added megapixels to FITS comment data
//*	Dec  9,	2019	<MLS> Added OBSTDIA and APTAREA to FITS output
//*	Dec 13,	2019	<MLS> Added focuser info to FITS output
//*	Dec 13,	2019	<MLS> Created cameradriver_fits.cpp
//*	Dec 13,	2019	<MLS> Moved FITS code to separate file
//*	Dec 13,	2019	<MLS> Added WriteFITS_FocuserInfo()
//*	Dec 13,	2019	<MLS> Added WriteFITS_FilterwheelInfo()
//*	Dec 15,	2019	<MLS> Added WriteFITS_ObservatoryInfo()
//*	Dec 15,	2019	<MLS> Added WriteFITS_Seperator()
//*	Dec 15,	2019	<MLS> Added WriteFITS_CameraInfo()
//*	Dec 15,	2019	<MLS> Added WriteFITS_TelescopeInfo()
//*	Dec 19,	2019	<MLS> Added WriteFITS_RotatorInfo()
//*	Dec 23,	2019	<MLS> Added WriteFITS_SoftwareInfo()
//*	Dec 23,	2019	<MLS> FITS data sections not included if info not present
//*	Dec 23,	2019	<MLS> Added Calc_AngularResolution()
//*	Dec 23,	2019	<MLS> Added Calc_ImageScale()
//*	Dec 23,	2019	<MLS> Added Calc_AngularResolutionPerPixel()
//*	Dec 23,	2019	<MLS> Added Calc_FieldOfView_arcSecs()
//*	Dec 30,	2019	<MLS> Added WriteFITS_EnvironmentInfo()
//*	Jan  2,	2020	<MLS> Downloaded fitsverify 4.20, all files pass.
//*	Jan  3,	2020	<MLS> Added dome environment data to FITS output
//*	Jan  6,	2020	<MLS> Added IMAGEID to FITS output for image sequences
//*	Jan 11,	2020	<MLS> Added headerOnly option to fits output
//*	Jan 19,	2020	<MLS> Added AVI fourCC to FITS output
//*	Jan 20,	2020	<MLS> Added libc version to fits output
//*	Jan 21,	2020	<MLS> Updated #ifdefs so that FITS can be disabled
//*	Jan 24,	2020	<MLS> Cleaned up consistency of FITS comment formats
//*	Jan 30,	2020	<MLS> Added other data products list to fits output
//*	Feb  3,	2020	<MLS> Added Platform: to fits output
//*	Feb  4,	2020	<MLS> Added jpeglib version # to fits output
//*	Feb 22,	2020	<MLS> Added gain to FITS camera info
//*	Nov 29,	2020	<MLS> Added WriteFITS_MoonInfo()
//*	Nov 30,	2020	<MLS> Added moon rise and moon set to FITS data
//*	Dec 14,	2020	<MLS> Just discovered a new version of cfitsio (3.49)
//*	Jan 19,	2021	<MLS> Added ROWORDER:BOTTOM-UP to FITS header
//*	Jan 20,	2021	<MLS> Added ExtractFitsHeader()
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FITS_)

#include	<errno.h>
#include	<math.h>
#include	<gnu/libc-version.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#if defined(__arm__)
	#include <wiringPi.h>
#endif

#ifdef _ENABLE_JPEGLIB_
	#include	<jpeglib.h>
#endif

#ifndef _FITSIO_H
	#include	<fitsio.h>
#endif // _FITSIO_H
#ifndef _FITSIO2_H
//	#include	<fitsio2.h>
#endif // _FITSIO2_H

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"observatory_settings.h"
#include	"sidereal.h"
#include	"obsconditions_globals.h"
#include	"moonphase.h"
#include	"MoonRise.h"
#include	"julianTime.h"
#include	"cpu_stats.h"



//*****************************************************************************
static void	FormatLatLonString(double latLonValue, char *latLonString)
{
double	myLatLonValue;
int		degrees;
int		minutes;
double	seconds;
bool	valueIsNegitive;

	if (latLonValue < 0)
	{
		valueIsNegitive	=	true;
		myLatLonValue	=	-latLonValue;
	}
	else
	{
		valueIsNegitive	=	false;
		myLatLonValue	=	latLonValue;
	}
	degrees	=	myLatLonValue;
	minutes	=	myLatLonValue * 60;
	minutes	=	minutes % 60;
	seconds	=	myLatLonValue * 3600.0;
	while (seconds >= 60.0)
	{
		seconds	-=	60.0;
	}
	if (valueIsNegitive)
	{
		sprintf(latLonString, "-%d %2d %5.3f", degrees, minutes, seconds);
	}
	else
	{
		sprintf(latLonString, "+%d %2d %5.3f", degrees, minutes, seconds);
	}

//	CONSOLE_DEBUG_W_DBL("latLonValue\t=", latLonValue);
//	CONSOLE_DEBUG_W_STR("latLonString\t=", latLonString);
}



//*****************************************************************************
//	DATE-OBS	String - The UTC date and time at the start of the exposure in
//	the ISO standard 8601 format: '2002-09-07T15:42:17.123' (CCYY-MM-
//	DDTHH:MM:SS.SSS).
//*****************************************************************************
void	FormatTimeString_TM(struct tm *timeStruct, char *timeString)
{

	if ((timeStruct != NULL) && (timeString != NULL))
	{

		sprintf(timeString, "%d-%02d-%02dT%02d:%02d:%02d.000",
								(1900 + timeStruct->tm_year),
								(1 + timeStruct->tm_mon),
								timeStruct->tm_mday,
								timeStruct->tm_hour,
								timeStruct->tm_min,
								timeStruct->tm_sec);

	}
}



//*****************************************************************************
static void	GetFitsErrorString(int fitsRetCode, char *errorString)
{
	switch(fitsRetCode)
	{
		//*	I will add to this list as needed
		case FILE_NOT_CREATED:	strcpy(errorString,	"FITS: could not create the named file");		break;
		case NUM_OVERFLOW:		strcpy(errorString,	"FITS: overflow during datatype conversion");	break;


		default:				sprintf(errorString, "Err not in table (%d)", fitsRetCode);	break;

	}
}

//*****************************************************************************
void	GetImageTypeString(TYPE_IMAGE_TYPE imageType, char *imageTypeString)
{
	switch(imageType)
	{
		case kImageType_RAW8:	strcpy(imageTypeString,	"RAW8");	break;
		case kImageType_RAW16:	strcpy(imageTypeString,	"RAW16");	break;
		case kImageType_RGB24:	strcpy(imageTypeString,	"RGB24");	break;
		case kImageType_Y8:		strcpy(imageTypeString,	"Y8");		break;
		default:				strcpy(imageTypeString,	"unknown");	break;
	}
}


//*****************************************************************************
//*	returns arc-seconds
//*****************************************************************************
static double	Calc_AngularResolution(double aperture_mm)
{
double	angularResolution_radians;
double	angularResolution_arcSecs;

	if (aperture_mm > 0.0)
	{
		//*	assume middle green wavelength
		angularResolution_radians	=	(1.22 * 550.0) / (aperture_mm * 1000000);
		angularResolution_arcSecs	=	(angularResolution_radians * 180 / M_PI) * 60.0 * 60.0;
	}
	else
	{
		angularResolution_arcSecs	=	0.0;
	}
	return(angularResolution_arcSecs);
}

//*****************************************************************************
//*	returns arc-seconds / micron
//*****************************************************************************
static double	Calc_ImageScale(double focalLength_mm)
{
double	imageScale;

	if (focalLength_mm > 0.0)
	{
		imageScale	=	206.2649 / focalLength_mm;
	}
	else
	{
		imageScale	=	0.0;
	}
	return(imageScale);
}

//*****************************************************************************
//*	returns arc-seconds per pixel
static double	Calc_AngularResolutionPerPixel(double focalLength_mm, double pixelSize_microns)
{
double	imageScale;
double	angularResolution_arcSecs;

	imageScale					=	Calc_ImageScale(focalLength_mm);
	angularResolution_arcSecs	=	pixelSize_microns * imageScale;
	return(angularResolution_arcSecs);
}


//*****************************************************************************
static double	Calc_FieldOfView_arcSecs(double focalLength_mm, double pixelSize_microns, int pixels_x)
{
double	angularResolution;
double	fov_arcSeconds;

	angularResolution	=	Calc_AngularResolutionPerPixel(focalLength_mm, pixelSize_microns);
	fov_arcSeconds		=	angularResolution * pixels_x;
	return(fov_arcSeconds);
}


#ifdef _ENABLE_FILTERWHEEL_
//*****************************************************************************
void	CameraDriver::UpdateFilterwheelLink(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cConnectedFilterWheel	=	(FilterwheelDriver *)FindDeviceByType(kDeviceType_Filterwheel);
	if (cConnectedFilterWheel != NULL)
	{
		cFilterWheelInfoValid	=	true;

	}
}
#endif // _ENABLE_FILTERWHEEL_

#ifdef _ENABLE_FOCUSER_
//*****************************************************************************
void	CameraDriver::UpdateFocuserLink(void)
{
	cConnectedFocuser	=	(FocuserDriver *)FindDeviceByType(kDeviceType_Focuser);
	if (cConnectedFocuser != NULL)
	{
		//*	if there is no focuser specified, get the name from the focuser object
		if (strlen(cTS_info.focuser) == 0)
		{
			cConnectedFocuser->GetFocuserModel(cTS_info.focuser);
		}
		cFocuserInfoValid	=	true;
	}
	if (strlen(cTS_info.focuser) > 0)
	{
		cFocuserInfoValid	=	true;
	}
}
#endif // _ENABLE_FOCUSER_


#ifdef _ENABLE_ROTATOR_
//*****************************************************************************
void	CameraDriver::UpdateRotatorLink(void)
{
	cConnectedRotator	=	(RotatorDriver *)FindDeviceByType(kDeviceType_Rotator);
	if (cConnectedRotator != NULL)
	{
		cRotatorInfoValid	=	true;
	}
}
#endif // _ENABLE_ROTATOR_



#define	_INCLUDE_FITS_SEPARATOR_

//*****************************************************************************
//	https://software.cfht.hawaii.edu/cfitsio/node32.html
//	 Creating a new FITS file
//
//	The following sequence of routine calls illustrate a simple example of writing a new FITS file:
//
//	 1. Create the new file with fits_create_file / ffinit.
//	 2. Write the required primary keywords with fits_create_img / ffcrim.
//	 3. Write any additional keywords with fits_write_key / ffpky.
//	 4. Write the primary array data, if any, with fits_write_img / ffppr.
//	 5. Create another extension, if desired, with fits_create_img / ffcrim
//	    or fits_create_tbl / ffcrtb.
//	 6. Write any additional keywords with fits_write_key / ffpky.
//	 7. Write data to table columns with fits_write_col /ffpcl.
//	 8. Repeat steps 5 - 7 for any more extensions.
//	 9. Close the fits file with fits_close_file / ffclos.
//
//	Note that the application program must not explicitly write the required 'END' keyword
//	at the end of each header; the CFITSIO interface will automatically append the END
//	record whenever the header is closed. As a standard practice, users should always
//	read back any FITS files that they have created to ensure that the header values and
//	data structure are correct and self-consistent.
//*****************************************************************************
//*	http://iraf.noao.edu/projects/ccdmosaic/imagedef/fitsdic.html
//*	https://diffractionlimited.com/help/maximdl/FITS_File_Header_Definitions.htm
//*****************************************************************************
int	CameraDriver::SaveImageAsFITS(bool headerOnly)
{
fitsfile		*fitsFilePtr;
int				fitsRetCode;
int				fitsStatus;
long			naxes[3];
int				axisCnt;
float			bzero;
float			bscale;
char			imageFileName[128];
char			aviFileName[128];
char			imageFilePath[128];
char			errorString[64];
int				fits_bitpix;
int				fitsDataType;
uint32_t		startMillisecs;
uint32_t		stopMillisecs;
uint32_t		deltaMillisecs;
int				iii;

	CONSOLE_DEBUG(__FUNCTION__);
	startMillisecs	=	millis();

	GenerateFileNameRoot();
	strcpy(imageFileName, cFileNameRoot);
	strcpy(imageFileName, cFileNameRoot);
	strcat(imageFileName, ".fits");

	strcpy(imageFilePath, kImageDataDir);
	strcat(imageFilePath, "/");
	strcat(imageFilePath, imageFileName);

	naxes[0]		=	cCameraProp.CameraXsize;
	naxes[1]		=	cCameraProp.CameraYsize;
	naxes[2]		=	3;				//*	only used for color RGB images (3 planes)
	axisCnt			=	2;				//*	for all formats except RGB
	fits_bitpix		=	SHORT_IMG;
	bscale			=	1.0;
	bzero			=	32768.0;

	//*	for information about the BZERO data element, refer to
	//*		https://docs.astropy.org/en/stable/io/fits/usage/image.html

	switch(cROIinfo.currentROIimageType)
	{
		case kImageType_RAW8:
//			CONSOLE_DEBUG("kImageType_RAW8");
			fits_bitpix		=	BYTE_IMG;
			fitsDataType	=	TBYTE;
			bzero			=	0.0;
			break;

		case kImageType_RAW16:
//			CONSOLE_DEBUG("kImageType_RAW16");
			fits_bitpix		=	SHORT_IMG;
			fitsDataType	=	TUSHORT;
			bzero			=	32768.0;
			break;

		//	Fits doesnt support RGB, it has to be 3 arrays, R, G, B
		case kImageType_RGB24:
//			CONSOLE_DEBUG("kImageType_RGB24");
			fits_bitpix		=	8;
			fitsDataType	=	TBYTE;
			bzero			=	0.0;
			axisCnt			=	3;
			break;

		case kImageType_Y8:
//			CONSOLE_DEBUG("kImageType_Y8");
			fits_bitpix		=	BYTE_IMG;
			fitsDataType	=	TUSHORT;
			bzero			=	0.0;
			break;

		default:
			fits_bitpix		=	16;
			fitsDataType	=	TUSHORT;
			break;
	}

	//*	if we are saving for AVI, then we are only saving the header data
	if (headerOnly)
	{
		naxes[0]		=	0;
		naxes[1]		=	0;
		axisCnt			=	0;
	}


	fitsStatus	=	0;
	fitsRetCode	=	fits_create_file(&fitsFilePtr, imageFilePath, &fitsStatus);
	if (fitsRetCode == 0)
	{
		CONSOLE_DEBUG("fits_create_file = SUCCESS");
		//************************************************************
		//*	this MUST be first
		//************************************************************
		fitsStatus	=	0;
		fitsRetCode	=	fits_create_img(fitsFilePtr, fits_bitpix, axisCnt, naxes, &fitsStatus);
		if (fitsRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("fits_create_img returned:", fitsRetCode);
			CONSOLE_DEBUG_W_NUM("fitsStatus:", fitsStatus);
		}
		WriteFITS_Seperator(fitsFilePtr, "");

		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TFLOAT,		"BSCALE",		&bscale,		NULL, &fitsStatus);

		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TFLOAT,		"BZERO",		&bzero,			NULL, &fitsStatus);

//		WriteFITS_Seperator(fitsFilePtr, NULL);

		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"TIMESYS",
												(char *)"UTC approximate",
												"Default time system", &fitsStatus);



		//************************************************************
		//*	output info about the observation
		WriteFITS_ObservationInfo(fitsFilePtr, (headerOnly == false));

		//*	leave FILENAME here so we dont have to pass the filename to the routine
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"FILENAME",
												imageFileName,
												"Orig filename", &fitsStatus);
		//*	were any other data products created
		if (cOtherDataCnt > 0)
		{
		char	tagString[64];

			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													(char *)"Other data products created",
													NULL, &fitsStatus);

			for (iii=0; iii<cOtherDataCnt; iii++)
			{
				sprintf(tagString, "FILENAM%d", (iii + 1));
				fits_write_key(fitsFilePtr, TSTRING,	tagString,
														cOtherDataProducts[iii].filename,
														cOtherDataProducts[iii].comment,
														&fitsStatus);
			}
		}

		//*	https://free-astro.org/index.php?title=Siril:FITS_orientation
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"ROWORDER",
												(char *)"BOTTOM-UP",
												NULL, &fitsStatus);

		if (headerOnly)
		{
			strcpy(aviFileName, cFileNameRoot);
			strcpy(aviFileName, cFileNameRoot);
			strcat(aviFileName, ".avi");

			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"AVI-NAME",
													aviFileName,
													"Filename of .avi data", &fitsStatus);
		}

		//************************************************************
		//*	Camera info
		WriteFITS_CameraInfo(fitsFilePtr);

		//************************************************************
		//*	Telescope info
		WriteFITS_TelescopeInfo(fitsFilePtr);

		//************************************************************
		//*	Focuser info
		WriteFITS_FocuserInfo(fitsFilePtr);

		//************************************************************
		//*	Rotator info
		WriteFITS_RotatorInfo(fitsFilePtr);

		//************************************************************
		//*	Filterwheel info
		WriteFITS_FilterwheelInfo(fitsFilePtr);

		//************************************************************
		//*	Observatory info
		WriteFITS_ObservatoryInfo(fitsFilePtr);

		//************************************************************
		//*	Environment/weather info
		WriteFITS_EnvironmentInfo(fitsFilePtr);

		//************************************************************
		//*	Moon information
		WriteFITS_MoonInfo(fitsFilePtr);

		//************************************************************
		//*	Software info
		WriteFITS_SoftwareInfo(fitsFilePtr);


		//************************************************************
		//*	FITS version info
		WriteFITS_VersionInfo(fitsFilePtr);


		WriteFITS_Seperator(fitsFilePtr, "");
		//------------------------------------------------------------------------
		//*	now deal with the image data
		if ((cCameraDataBuffer != NULL) && (headerOnly == false))
		{
		LONGLONG		nelements;
		long			fpixelArray[4];

//			CONSOLE_DEBUG("Writing image data to FITS file");
			nelements	=	cCameraProp.CameraXsize * cCameraProp.CameraYsize;


			fpixelArray[0]	=	1;
			fpixelArray[1]	=	1;
			fpixelArray[2]	=	1;		//*	RGB images only
			fitsStatus		=	0;
//			CONSOLE_DEBUG_W_INT32("nelements\t=", (long)nelements);
			switch(cROIinfo.currentROIimageType)
			{
				case kImageType_RAW8:
				case kImageType_RAW16:
					fitsRetCode		=	fits_write_pix(	fitsFilePtr,
														fitsDataType,
														fpixelArray,
														nelements,
														cCameraDataBuffer,
														&fitsStatus);
					break;


				//	Fits doesn't support RGB, it has to be 3 arrays, B, G, R
				case kImageType_RGB24:
					CreateFitsBGRimage();
//					CONSOLE_DEBUG(__FUNCTION__);
					if (cCameraBGRbuffer != NULL)
					{
						nelements		=	3 * cCameraProp.CameraXsize * cCameraProp.CameraYsize;
						fitsRetCode		=	fits_write_pix(	fitsFilePtr,
												fitsDataType,
												fpixelArray,
												nelements,
												cCameraBGRbuffer,
												&fitsStatus);
					}
					break;

				case kImageType_Y8:
					break;

				case kImageType_last:
				default:
					break;
			}


			if (fitsRetCode != 0)
			{
				CONSOLE_DEBUG_W_NUM("fits_write_pix returned:", fitsRetCode);
				CONSOLE_DEBUG_W_NUM("fitsStatus:", fitsStatus);
				GetFitsErrorString(fitsRetCode, errorString);
				CONSOLE_DEBUG_W_STR("fits_write_pix returned:", errorString);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,
											"ERROR",
											errorString,
											"fits_write_pix", &fitsStatus);
			}
		}
		else if (headerOnly)
		{
			//*	we are saving an AVI file, there is no data in the FITS file
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													(char *)"This FITS file does not contain any data, header only",
													NULL, &fitsStatus);
			if (cAVIfourcc != 0)
			{
			char	aviString[8];
			char	commentString[80];

				aviString[0]	=	(cAVIfourcc & 0x00ff);
				aviString[1]	=	((cAVIfourcc >> 8) & 0x00ff);
				aviString[2]	=	((cAVIfourcc >> 16) & 0x00ff);
				aviString[3]	=	((cAVIfourcc >> 24) & 0x00ff);
				aviString[4]	=	0;
				sprintf(commentString, "AVI format (codec): %s", aviString);
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														commentString,
														NULL, &fitsStatus);
			}
		}
		else
		{
			CONSOLE_DEBUG("No image data to write");
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"ERROR",
													(void *)"No image data to write",
													NULL, &fitsStatus);
		}

		//------------------------------------------------------------------------
		//*	History
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "HISTORY",		(void *)"Original image created by AlpacaPi Camera Driver", NULL, &fitsStatus);

		fitsStatus	=	0;
		fits_write_chksum(fitsFilePtr, &fitsStatus);

		ExtractFitsHeader(fitsFilePtr);

		fitsStatus	=	0;
		fitsRetCode	=	fits_close_file(fitsFilePtr, &fitsStatus);
		if (fitsRetCode == 0)
		{
			CONSOLE_DEBUG("fits_close_file = SUCCESS");
		}
		else
		{
			GetFitsErrorString(fitsRetCode, errorString);
			CONSOLE_DEBUG_W_NUM("fits_close_file returned:", fitsRetCode);
			CONSOLE_DEBUG_W_STR("fits_close_file returned:", errorString);
			CONSOLE_DEBUG_W_NUM("fitsStatus:", fitsStatus);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to create FITS file:", imageFileName);
		CONSOLE_DEBUG_W_NUM("fits_create_file returned:", fitsRetCode);
		GetFitsErrorString(fitsRetCode, errorString);
		CONSOLE_DEBUG_W_STR("fits_create_file returned:", errorString);
		CONSOLE_DEBUG_W_NUM("errno\t=", errno);

	}

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");

	stopMillisecs	=	millis();
	deltaMillisecs	=	stopMillisecs - startMillisecs;
	CONSOLE_DEBUG_W_NUM("Time to save FITS file (milliseconds)\t=",	deltaMillisecs);

	return(0);

}

//*****************************************************************************
//*	returns the number of lines extracted
//*****************************************************************************
int	CameraDriver::ExtractFitsHeader(fitsfile *fitsFilePtr)
{
char		card[FLEN_CARD];
int			nkeys;
int			fitsStatus;
int			iii;
int			fitsHdrIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
	//*	wipe out any previous data
	for (iii = 0; iii < kMaxFitsRecords; iii++)
	{
		cFitsHeader[iii].fitsRec[0]	=	0;
	}

	fitsStatus	=	0;	//* MUST initialize status
	fits_get_hdrspace(fitsFilePtr, &nkeys, NULL, &fitsStatus);
	fitsHdrIdx	=	0;
	for (iii = 1; iii <= nkeys; iii++)
	{
		card[0]		=	0;
		fitsStatus	=	0;
		fits_read_record(fitsFilePtr, iii, card, &fitsStatus);	//* read keyword
		if ((strlen(card) > 0) && (fitsHdrIdx < kMaxFitsRecords))
		{
//			CONSOLE_DEBUG(card);
			strcpy(cFitsHeader[fitsHdrIdx].fitsRec, card);
			fitsHdrIdx++;
		}
	}
	if (fitsHdrIdx < kMaxFitsRecords)
	{
		strcpy(cFitsHeader[fitsHdrIdx].fitsRec, "END");
	}
	else
	{
		CONSOLE_DEBUG("Ran out of room in fits header buffer");
	}
	return(fitsHdrIdx);
}
				TYPE_FITS_RECORD	cFitsHeader[kMaxFitsRecords];

#pragma mark -


//*****************************************************************************
static void	WriteFitsFloatValue(fitsfile		*fitsFilePtr,
								const char		*keyword,
								const double	value,
								const char		*comment)
{
float	floatValue;
int		fitsStatus;

	floatValue	=	value;
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TFLOAT,		keyword,
											&floatValue,
											comment, &fitsStatus);
}


//*****************************************************************************
static void	WriteFitsFloatValueIfGtZero(	fitsfile		*fitsFilePtr,
											const char		*keyword,
											const double	value,
											const char		*comment)
{
float	floatValue;
int		fitsStatus;

	if (value > 0.0)
	{
		floatValue	=	value;
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TFLOAT,		keyword,
												&floatValue,
												comment, &fitsStatus);
	}
}

//*****************************************************************************
void	CameraDriver::WriteFITS_Seperator(fitsfile *fitsFilePtr, const char *blockName)
{
int		fitsStatus;
char	lineBuff[128];

#ifdef _INCLUDE_FITS_SEPARATOR_
	strcpy(lineBuff, "-------------------------------------");
	if (blockName != NULL)
	{
		strcat(lineBuff, blockName);
	}
	while (strlen(lineBuff) < 75)
	{
		strcat(lineBuff, "-");
	}
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											lineBuff,
											NULL, &fitsStatus);
#endif // _INCLUDE_FITS_SEPARATOR_
}

#pragma mark -

//*****************************************************************************
void	CameraDriver::WriteFITS_CameraInfo(fitsfile *fitsFilePtr)
{
int		fitsStatus;
char	stringBuf[128];
double	megaPixels;
int		intValue;
int		ccdTempErrCode;
char	instrumentString[128];

	CONSOLE_DEBUG(__FUNCTION__);

	WriteFITS_Seperator(fitsFilePtr, "Camera Info");

	//***************************************************
	//*	output info about the camera and instrument
	if (strlen(cTS_info.instrument) > 0)
	{
		strcpy(instrumentString, cTS_info.instrument);
	}
	else if (strlen(cDeviceDescription) > 0)
	{
		strcpy(instrumentString, cDeviceDescription);
	}
	else
	{
		strcpy(instrumentString, "unknown");
	}
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "INSTRUME",	instrumentString, NULL, &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "CAMERA",		cDeviceName, NULL, &fitsStatus);

	if (strlen(cDriverversionStr) > 0)
	{
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr,	TSTRING,
									"CAMSWV",
									cDriverversionStr,
									"Camera Driver version", &fitsStatus);
	}

	if (strlen(cDeviceFirmwareVersStr) > 0)
	{
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr,	TSTRING,
									"CAMFWV",
									cDeviceFirmwareVersStr,
									"Camera Firmware version", &fitsStatus);
	}

	//***************************************************
	//*	output info about the sensor
	if (strlen(cSensorName) > 0)
	{
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "DETECTOR",	cSensorName,		NULL, &fitsStatus);
	}

	sprintf(stringBuf, "[1:%d,1:%d]", cCameraProp.CameraXsize, cCameraProp.CameraYsize);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "DETSIZE",	stringBuf,		"Detector size", &fitsStatus);

	//*	image mode from camera
	GetImageTypeString(cROIinfo.currentROIimageType, stringBuf);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"IMGTYPE",
											stringBuf,
											"Image mode from camera", &fitsStatus);

	ccdTempErrCode	=	Read_SensorTemp();
	if (ccdTempErrCode == 0)
	{
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TDOUBLE,	"CCD-TEMP",
												&cCameraTemp_Dbl,
												"Degrees C", &fitsStatus);
	}


	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"XPIXSZ",
											&cCameraProp.PixelSizeX,
											"X Pixel size in microns", &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"YPIXSZ",
											&cCameraProp.PixelSizeY,
											"Y Pixel size in microns", &fitsStatus);


	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TINT,		"XBINNING",	&cCameraProp.BinX,	NULL, &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TINT,		"YBINNING",	&cCameraProp.BinY,	NULL, &fitsStatus);

	if (cCameraProp.ElectronsPerADU > 0.0)
	{
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TDOUBLE,	"EGAIN",
												&cCameraProp.ElectronsPerADU,
												"Electrons Per ADU",
												&fitsStatus);
	}

	sprintf(stringBuf, "Camera gain [%d:%d]", cCameraProp.GainMin, cCameraProp.GainMax);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr,		TINT,		"GAIN",
												&cCameraProp.Gain,
												stringBuf,
												&fitsStatus);

	//*	ATIK dusk software uses this keyword
	fitsStatus	=	0;
	intValue	=	cIsColorCam;
	if (cROIinfo.currentROIimageType == kImageType_RGB24)
	{
		intValue	=	true;
	}
	else
	{
		intValue	=	false;
	}
	fits_write_key(fitsFilePtr, TLOGICAL,	"ISCOLOUR",
											&intValue,
											"True if image is a color image",
											&fitsStatus);
	//*	camera manufacturer
	if (strlen(cDeviceManufacturer) > 0)
	{
		strcpy(stringBuf, "Camera Manufacturer: ");
		strcat(stringBuf, cDeviceManufacturer);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
	}

	//*	camera name
	if (strlen(cDeviceName) > 0)
	{
		strcpy(stringBuf, "Camera Model: ");
		strcat(stringBuf, cDeviceName);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
	}

	//*	camera description
	if (strlen(cDeviceDescription) > 0)
	{
		strcpy(stringBuf, "Camera Description: ");
		strcat(stringBuf, cDeviceDescription);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
	}

	//*	sensor name
	if (strlen(cSensorName) > 0)
	{
		strcpy(stringBuf, "Camera Sensor: ");
		strcat(stringBuf, cSensorName);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
	}

	sprintf(stringBuf, "Camera image size: %d x %d", cCameraProp.CameraXsize, cCameraProp.CameraYsize);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);

	megaPixels	=	(1.0 * cCameraProp.CameraXsize * cCameraProp.CameraYsize) / (1024 * 1024);
	sprintf(stringBuf, "Camera image size: %1.1f megapixels", megaPixels);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);

	sprintf(stringBuf, "Camera type: %s", (cIsColorCam ? "Color" : "Monochrome"));
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);

	//*	Camera driver version
	if (strlen(cDriverversionStr) > 0)
	{
		strcpy(stringBuf, "Camera Driver Version: ");
		strcat(stringBuf, cDriverversionStr);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
	}

	//*	camera serial number
	if (strlen(cDeviceSerialNum) > 1)
	{
		strcpy(stringBuf, "Camera Serial Number: ");
		strcat(stringBuf, cDeviceSerialNum);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
	}

	sprintf(stringBuf, "Camera bit depth: %d", cBitDepth);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);

	//---------------------------------------------------------------------
	sprintf(stringBuf, "Image Shutter: %d microseconds ", cCameraProp.Lastexposure_duration_us);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);


	//*	this was kept here so we dont have to read the CCD temperature twice
	if (ccdTempErrCode == 0)
	{
		sprintf(stringBuf, "Image Sensor Temperature: %1.1f deg C, %1.1f deg F", cCameraTemp_Dbl, ((cCameraTemp_Dbl * 9.0/5.0) + 32.0));
	}
	else
	{
		strcpy(stringBuf, "Image Sensor Temperature: not supported");
	}
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											stringBuf,
											NULL, &fitsStatus);
}

//#define _FAKE_ENVIRO_DATA_

//*****************************************************************************
void	CameraDriver::WriteFITS_EnvironmentInfo(fitsfile *fitsFilePtr)
{
int			fitsStatus;
double		modifiedJulianDate;
bool		dataSrcStringsAreSame;
bool		printDomeDataSrc;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _FAKE_ENVIRO_DATA_
	gEnvData.siteDataValid	=	true;
	gEnvData.domeDataValid	=	true;

	gettimeofday(&gEnvData.siteLastUpdate, NULL);
	sleep(1);
	gettimeofday(&gEnvData.domeLastUpdate, NULL);
	strcpy(gEnvData.siteDataSource,	"Fake data for testing");
	strcpy(gEnvData.domeDataSource,	"Fake data for testing");

	gEnvData.siteTemperature_valid	=	true;
	gEnvData.siteTemperature_degC	=	23.4;
	gEnvData.siteHumidity			=	47.0;
	gEnvData.sitePressure_kPa		=	99.12345;

	gEnvData.domeTemperature_valid	=	true;
	gEnvData.domeTemperature_degC	=	10.77;
	gEnvData.domeHumidity			=	12.3;

#endif

	//*	This is to prevent the same message from being inserted twice.
	printDomeDataSrc		=	true;
	dataSrcStringsAreSame	=	(strcmp(gEnvData.siteDataSource, gEnvData.domeDataSource) == 0);

	if (gEnvData.siteDataValid || gEnvData.domeDataValid)
	{
		WriteFITS_Seperator(fitsFilePtr, "Environment Info");

//		fitsStatus	=	0;
//		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
//												(void *)"Test data, values not valid",
//												NULL, &fitsStatus);
	}

	if (gEnvData.siteDataValid)
	{
		if (strlen(gEnvData.siteDataSource) > 0)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													gEnvData.siteDataSource,
													NULL, &fitsStatus);
			if (dataSrcStringsAreSame)
			{
				printDomeDataSrc	=	false;
			}
		}
		if (gEnvData.siteLastUpdate.tv_sec > 0)
		{
			modifiedJulianDate	=	Julian_CalcMJD(&gEnvData.siteLastUpdate);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TDOUBLE,	"ENVMJD",
													&modifiedJulianDate,
													"MJD of site environment measurement", &fitsStatus);
		}
		if (gEnvData.siteTemperature_valid)
		{
			WriteFitsFloatValue(		fitsFilePtr,
										"ENVTEM",
										gEnvData.siteTemperature_degC,
										"Site temperature (degrees C)");
		}
		WriteFitsFloatValueIfGtZero(fitsFilePtr,
									"ENVHUM",
									gEnvData.siteHumidity,
									"Site humidity (percent)");

		WriteFitsFloatValueIfGtZero(fitsFilePtr,
									"ENVPRE",
									gEnvData.sitePressure_kPa,
									"Site air pressure (kPascals)");

	}

	//------------------------------------------------------------------------
	if (gEnvData.domeDataValid)
	{
		//*	inside the dome info
		if ((strlen(gEnvData.domeDataSource) > 0) && printDomeDataSrc)
		{
			//*	dont print if its the same as the previous one
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													gEnvData.domeDataSource,
													NULL, &fitsStatus);
		}

		if (gEnvData.domeLastUpdate.tv_sec > 0)
		{
			modifiedJulianDate	=	Julian_CalcMJD(&gEnvData.domeLastUpdate);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TDOUBLE,	"DMEMJD",
													&modifiedJulianDate,
													"MJD of dome environment measurement", &fitsStatus);
		}

		if (gEnvData.domeTemperature_valid)
		{
			WriteFitsFloatValue(		fitsFilePtr,
										"DMETEM",
										gEnvData.domeTemperature_degC,
										"Dome temperature inside (degrees C)");
		}
		WriteFitsFloatValueIfGtZero(fitsFilePtr,
									"DMEHUM",
									gEnvData.domeHumidity,
									"Dome humidity (percent)");

		WriteFitsFloatValueIfGtZero(fitsFilePtr,
									"DMEPRE",
									gEnvData.domePressure_kPa,
									"Dome air pressure (kPascals)");
	}
}

//*****************************************************************************
void	CameraDriver::WriteFITS_FilterwheelInfo(fitsfile *fitsFilePtr)
{
int		fitsStatus;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _ENABLE_FILTERWHEEL_
	if (cConnectedFilterWheel == NULL)
	{
		UpdateFilterwheelLink();
	}
#endif	//	_ENABLE_FILTERWHEEL_

	if (cFilterWheelInfoValid || cTS_info.hasFilterwheel)
	{
		WriteFITS_Seperator(fitsFilePtr, "Filter wheel Info");

		if (strlen(cTS_info.filterwheel) > 0)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"FILTWHL",
													cTS_info.filterwheel,
													"Filter wheel used", &fitsStatus);
		}
	#ifdef _ENABLE_FILTERWHEEL_
		if (cConnectedFilterWheel != NULL)
		{
		int		fwAlpacaErr;		//*	filter wheel alpaca error code
		int		filterPosition;
		char	filterPositionName[48];

			CONSOLE_DEBUG("We have valid filterwheel info");
			CONSOLE_DEBUG("Calling Read_CurrentFilterPositon");

			fwAlpacaErr	=	cConnectedFilterWheel->Read_CurrentFilterPositon(&filterPosition);
			if (fwAlpacaErr == kASCOM_Err_Success)
			{
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TINT,		"FILPOS",
														&filterPosition,
														"Filter wheel position", &fitsStatus);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("cConnectedFilterWheel->Read_CurrentFilterPositon returned ERROR:", fwAlpacaErr);
			}
			CONSOLE_DEBUG("Calling Read_CurrentFilterName");
			fwAlpacaErr	=	cConnectedFilterWheel->Read_CurrentFilterName(filterPositionName);
			if (fwAlpacaErr == kASCOM_Err_Success)
			{
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"FILTER",
														filterPositionName,
														"Name of current filter", &fitsStatus);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("cConnectedFilterWheel->Read_CurrentFilterName returned ERROR:", fwAlpacaErr);
			}
			CONSOLE_DEBUG("Done with filter wheel stuff");
		}
		else
	#endif // _ENABLE_FILTERWHEEL_
		{
			//*	only do this if there is NOT an attached filter wheel
			if (strlen(cTS_info.filterName) > 0)
			{
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"FILTER",
														cTS_info.filterName,
														"Name of current filter", &fitsStatus);
			}
		}
	}
}

//*****************************************************************************
void	CameraDriver::WriteFITS_FocuserInfo(fitsfile *fitsFilePtr)
{
//int		fitsRetCode;
int		fitsStatus;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _ENABLE_FOCUSER_

	if (cConnectedFocuser == NULL)
	{
		UpdateFocuserLink();
	}
#endif	//	_ENABLE_FOCUSER_

	if (cFocuserInfoValid || (strlen(cTS_info.focuser) > 0))
	{
		WriteFITS_Seperator(fitsFilePtr, "Focuser Info");

		if (strlen(cTS_info.focuser) > 0)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"FOCUSER",
													cTS_info.focuser,
													"Focuser used", &fitsStatus);
		}

	#ifdef _ENABLE_FOCUSER_
		if (cConnectedFocuser != NULL)
		{
		long	focuserPosition;
		char	dataBuff[128];
		char	lineBuff[128];
		double	dblValue;

			focuserPosition	=	cConnectedFocuser->GetFocuserPosition();
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TINT,		"TELFOCUS",
													&focuserPosition,
													"Telescope Focuser position", &fitsStatus);

			//*	manufacturer
			cConnectedFocuser->GetFocuserManufacturer(dataBuff);
			if (strlen(dataBuff) > 0)
			{
				strcpy(lineBuff, "Focuser Manufacturer: ");
				strcat(lineBuff, dataBuff);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);

			}

			//*	model
			cConnectedFocuser->GetFocuserModel(dataBuff);
			if (strlen(dataBuff) > 0)
			{
				strcpy(lineBuff, "Focuser Model: ");
				strcat(lineBuff, dataBuff);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);
			}

			//*	version
			cConnectedFocuser->GetFocuserVersion(dataBuff);
			if (strlen(dataBuff) > 0)
			{
				strcpy(lineBuff, "Focuser Version: ");
				strcat(lineBuff, dataBuff);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);
			}

			//*	serial number
			cConnectedFocuser->GetFocuserSerialNumber(dataBuff);
			if (strlen(dataBuff) > 0)
			{
				strcpy(lineBuff, "Focuser Serial Number: ");
				strcat(lineBuff, dataBuff);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);
			}

			//*	Temperature
			dblValue	=	cConnectedFocuser->GetFocuserTemperature();
			if (dblValue != 0.0)
			{

				sprintf(lineBuff, "Focuser Temperature: %1.1f deg C, %1.1f deg F", dblValue, DEGREES_F(dblValue));
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TDOUBLE,	"TELTEM",
														&dblValue,
														"Temperature at the focuser", &fitsStatus);
			}

			//*	Voltage
			dblValue	=	cConnectedFocuser->GetFocuserVoltage();
			if (dblValue > 1.0)
			{
				sprintf(lineBuff, "Focuser Voltage: %1.1f", dblValue);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TDOUBLE,	"TELVOL",
														&dblValue,
														"Voltage at the focuser", &fitsStatus);
			}
		}
	#endif // _ENABLE_FOCUSER_
	}
}

//*****************************************************************************
void	CameraDriver::WriteFITS_ObservatoryInfo(fitsfile *fitsFilePtr)
{
int		fitsStatus;
char	stringBuf[128];

	CONSOLE_DEBUG(__FUNCTION__);

	if (gObseratorySettings.ValidInfo)
	{
		WriteFITS_Seperator(fitsFilePtr, "Observatory Info");

		if (strlen(gObseratorySettings.Name) > 0)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "OBSERVAT",	gObseratorySettings.Name,		NULL, &fitsStatus);
		}
		if (strlen(gObseratorySettings.Website) > 0)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "WEBSITE",		gObseratorySettings.Website,		NULL, &fitsStatus);
		}

		if (gObseratorySettings.ValidLatLon)
		{
			FormatLatLonString(gObseratorySettings.Latitude, stringBuf);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"SITELAT",
													(void *)stringBuf,
													"+DD MM SS.SSS", &fitsStatus);

			FormatLatLonString(gObseratorySettings.Longitude, stringBuf);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"SITELONG",
													(void *)stringBuf,
													"+DD MM SS.SSS", &fitsStatus);

			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TDOUBLE,	"LATITUDE",
													&gObseratorySettings.Latitude,
													"Degrees Latitude", &fitsStatus);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TDOUBLE,	"LONGITUD",
													&gObseratorySettings.Longitude,
													"Degrees Longitude", &fitsStatus);
		}
		if (gObseratorySettings.Elevation_m != 0.0)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TDOUBLE,	"SITEELEV",
													&gObseratorySettings.Elevation_m,
													"Elevation in meters", &fitsStatus);
		}
		//---------------------------------------------------------------------
		//*	Name
		if (strlen(gObseratorySettings.Name) > 0)
		{
			strcpy(stringBuf, "Observatory Name: ");
			strcat(stringBuf, gObseratorySettings.Name);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
		}
		//*	Configuration
		if (strlen(gObseratorySettings.Configuration) > 0)
		{
			strcpy(stringBuf, "Observatory Configuration: ");
			strcat(stringBuf, gObseratorySettings.Configuration);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
		}
		//*	Location
		if (strlen(gObseratorySettings.Location) > 0)
		{
			strcpy(stringBuf, "Observatory Location: ");
			strcat(stringBuf, gObseratorySettings.Location);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
		}
		//*	TimeZone
		if (strlen(gObseratorySettings.TimeZone) > 0)
		{
			strcpy(stringBuf, "Observatory Timezone: ");
			strcat(stringBuf, gObseratorySettings.TimeZone);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
		}


		//*	UTC offset
		sprintf(stringBuf, "Observatory UTC offset: %d", gObseratorySettings.UTCoffset);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);


		//*	UTC Owner
		if (strlen(gObseratorySettings.Owner) > 0)
		{
			strcpy(stringBuf, "Observatory Owner: ");
			strcat(stringBuf, gObseratorySettings.Owner);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
		}
		//*	Email
		if (strlen(gObseratorySettings.Email) > 0)
		{
			strcpy(stringBuf, "Observatory Email: ");
			strcat(stringBuf, gObseratorySettings.Email);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
		}
		//*	AAVSO_ObserverID
		if (strlen(gObseratorySettings.AAVSO_ObserverID) > 0)
		{
			strcpy(stringBuf, "Observatory AAVSO_ObserverID: ");
			strcat(stringBuf, gObseratorySettings.AAVSO_ObserverID);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING, "COMMENT",	stringBuf,		NULL, &fitsStatus);
		}
	}
}





//*****************************************************************************
void	CameraDriver::WriteFITS_ObservationInfo(fitsfile *fitsFilePtr, bool includeAnalysis)
{
int				fitsStatus;
double			exposureTime_Secs;
struct tm		utcTime;
struct tm		siderealTime;
char			stringBuf[128];
unsigned long	minmaxPixelValue;
int				staurationValue;
float			saturationPrcnt;
double			modifiedJulianDate;

	CONSOLE_DEBUG(__FUNCTION__);

	WriteFITS_Seperator(fitsFilePtr, "Observation Info");

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"OBJECT",
											cObjectName,
											"Observation title", &fitsStatus);

	if (strlen(gObseratorySettings.Observer) > 0)
	{
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"OBSERVER",
												gObseratorySettings.Observer,
												NULL, &fitsStatus);
	}

	//*	format the time of exposure start
	FormatTimeStringISO8601(&cCameraProp.Lastexposure_StartTime, stringBuf);
//	CONSOLE_DEBUG_W_STR("stringBuf:", stringBuf);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING, "DATE-OBS",	stringBuf,		"UTC date of observation", &fitsStatus);

	gmtime_r(&cCameraProp.Lastexposure_StartTime.tv_sec, &utcTime);
	CalcSiderealTime(&utcTime, &siderealTime, gObseratorySettings.Longitude);
	FormatTimeString_TM(&siderealTime, stringBuf);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"LST-OBS",
											stringBuf,
											"Local Sidereal Time start of exposure", &fitsStatus);

	modifiedJulianDate	=	Julian_CalcMJD(&cCameraProp.Lastexposure_StartTime);
	fitsStatus			=	0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"MJD-OBS",
											&modifiedJulianDate,
											"MJD of observation", &fitsStatus);
	if (cCameraProp.Lastexposure_EndTime.tv_sec > cCameraProp.Lastexposure_StartTime.tv_sec)
	{
		modifiedJulianDate	=	Julian_CalcMJD(&cCameraProp.Lastexposure_EndTime);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TDOUBLE,	"MJDEND",
												&modifiedJulianDate,
												"MJD at end of exposure", &fitsStatus);
	}

	fitsStatus	=	0;
	exposureTime_Secs	=	(cCameraProp.Lastexposure_duration_us * 1.0) / 1000000.0;
	fits_write_key(fitsFilePtr, TDOUBLE,	"EXPTIME",
											&exposureTime_Secs,
											"Exposure time (seconds)", &fitsStatus);

	if ((cImageMode == kImageMode_Sequence) && (cImageSeqNumber > 0))
	{
		sprintf(stringBuf, "Image Sequence Number (of %d)", cNumFramesRequested);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TINT,		"IMAGEID",
												&cImageSeqNumber,
												stringBuf,
												&fitsStatus);
	}
	else if (cNumFramesSaved > 1)
	{
		sprintf(stringBuf, "Frames saved: %d", cNumFramesSaved);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												stringBuf,
												NULL, &fitsStatus);

		if (cFrameRate > 0.01)
		{
			sprintf(stringBuf, "Frame rate: %1.2f (fps)", cFrameRate);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													stringBuf,
													NULL, &fitsStatus);
		}
	}


	if (includeAnalysis)
	{
		//************************************************************
		//*	Image analysis stuff

		minmaxPixelValue	=	CalculateMinPixValue();
		if (minmaxPixelValue < 65535)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TINT,	"DATAMIN",		//"MNPIXVAL",
												&minmaxPixelValue,
												"Minimum pixel value", &fitsStatus);
		}

		minmaxPixelValue	=	CalculateMaxPixValue();
		if (minmaxPixelValue > 0)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TINT,	"DATAMAX",		//"MXPIXVAL",
												&minmaxPixelValue,
												"Maximum pixel value", &fitsStatus);
		}

		if (cROIinfo.currentROIimageType == kImageType_RAW16)
		{
			staurationValue	=	0x0ffff;
		}
		else
		{
			staurationValue	=	0x0ff;
		}

		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TINT,	"SATURATE",
											&staurationValue,
											"Saturation Value", &fitsStatus);


		saturationPrcnt	=	CalculateSaturation();
		CONSOLE_DEBUG_W_DBL("saturationPrcnt\t: ",		saturationPrcnt);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TFLOAT,	"SATUPRCT",
											&saturationPrcnt,
											"Percentage of pixels at saturation", &fitsStatus);

		//---------------------------------------------------------------------------------------
		//*	Histogram information
		//*	this histogram was already calculated before the FITS routine was called.
		if (cROIinfo.currentROIimageType == kImageType_RAW16)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													(char *)"For 16 bit data, the histogram is based on the high 8 bits",
													NULL, &fitsStatus);
		}
		else if (cROIinfo.currentROIimageType == kImageType_RGB24)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													(char *)"For RGBimages, the histogram is based on luminance (avg of R,G,B)",
													NULL, &fitsStatus);
		}

		sprintf(stringBuf, "Min histogram value: %d", cMinHistogramValue);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												stringBuf,
												NULL, &fitsStatus);

		sprintf(stringBuf, "Peak histogram value: %d", cPeakHistogramValue);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												stringBuf,
												NULL, &fitsStatus);

		sprintf(stringBuf, "Max histogram value: %d", cMaxHistogramValue);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												stringBuf,
												NULL, &fitsStatus);
	}
}

//*****************************************************************************
void	CameraDriver::WriteFITS_RotatorInfo(fitsfile *fitsFilePtr)
{

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _ENABLE_ROTATOR_
	if (cConnectedRotator == NULL)
	{
		UpdateRotatorLink();
	}
#endif // _ENABLE_ROTATOR_

	if (cRotatorInfoValid)
	{
	#ifdef _ENABLE_ROTATOR_
		if (cConnectedRotator != NULL)
		{
		int		fitsStatus;
		long	currentRotatorPos;
		char	lineBuff[128];
		char	dataBuff[128];

			WriteFITS_Seperator(fitsFilePtr, "Rotator Info");
			currentRotatorPos	=	cConnectedRotator->ReadCurrentPoisiton_steps();

			//*	manufacturer
			cConnectedRotator->GetRotatorManufacturer(dataBuff);
			if (strlen(dataBuff) > 0)
			{
				strcpy(lineBuff, "Rotator Manufacturer: ");
				strcat(lineBuff, dataBuff);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);

			}

			//*	model
			cConnectedRotator->GetRotatorModel(dataBuff);
			if (strlen(lineBuff) > 0)
			{
				strcpy(lineBuff, "Rotator Model: ");
				strcat(lineBuff, dataBuff);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);
			}

			//*	serial number
			cConnectedRotator->GetRotatorSerialNumber(dataBuff);
			if (strlen(lineBuff) > 0)
			{
				strcpy(lineBuff, "Rotator Serial Number: ");
				strcat(lineBuff, dataBuff);
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														lineBuff,
														NULL, &fitsStatus);
			}

			sprintf(lineBuff, "Rotator position: %ld", currentRotatorPos);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													lineBuff,
													NULL, &fitsStatus);
		}
	#endif // _ENABLE_ROTATOR_
	}

}

//*****************************************************************************
void	CameraDriver::WriteFITS_SoftwareInfo(fitsfile *fitsFilePtr)
{
int				fitsStatus;
char			stringBuf[128];

	CONSOLE_DEBUG(__FUNCTION__);

	WriteFITS_Seperator(fitsFilePtr, "Software Info");
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"SWCREATE",
											(void *)"AlpacaCameraDriver Software by Mark Sproul, msproul@skychariot.com",
											NULL, &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"IMAGESWV",
											gFullVersionString,
											"Image creation software version", &fitsStatus);

	//*	gcc version
	sprintf(stringBuf, "Compiled %s at %s with gcc version: %s", __DATE__, __TIME__, __VERSION__);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											stringBuf,
											NULL, &fitsStatus);

	//*	libc version
	sprintf(stringBuf, "Using libc version: %s", gnu_get_libc_version ());
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											stringBuf,
											NULL, &fitsStatus);

	//*	cfitsio version
	sprintf(stringBuf,	"Using FITS library (cfitsio) Version: %d.%d", CFITSIO_MAJOR, CFITSIO_MINOR);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											stringBuf,
											NULL, &fitsStatus);

#ifdef _USE_OPENCV_
	sprintf(stringBuf,	"Using OpenCV library Version: %s", CV_VERSION);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											stringBuf,
											NULL, &fitsStatus);
#endif
#ifdef _ENABLE_JPEGLIB_
	sprintf(stringBuf,	"Using jpeglib library Version: %d", JPEG_LIB_VERSION);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											stringBuf,
											NULL, &fitsStatus);
#endif

	//*	OS version
	if (strlen(gOsReleaseString) > 0)
	{
		strcpy(stringBuf, "OS Version: ");
		strcat(stringBuf, gOsReleaseString);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												stringBuf,
												NULL, &fitsStatus);
	}

	//*	cpu info
	if (strlen(gCpuInfoString) > 5)
	{
		strcpy(stringBuf, "Running on: ");
		strcat(stringBuf, gCpuInfoString);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												stringBuf,
												NULL, &fitsStatus);
	}
	//*	platform info
	if (strlen(gPlatformString) > 5)
	{
		strcpy(stringBuf, "Platform: ");
		strcat(stringBuf, gPlatformString);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												stringBuf,
												NULL, &fitsStatus);
	}

}

//*****************************************************************************
void	CameraDriver::WriteFITS_TelescopeInfo(fitsfile *fitsFilePtr)
{
int		ii;
int		fitsStatus;
double	radius;
double	apertureArea;
double	obstructArea;
double	obstructPercent;
char	stringBuf[128];
double	angularResolution_arcSecs;
double	imageScale;
double	angularResolution_perPixel;
double	fov_arcSeconds_X;
double	fov_arcSeconds_Y;
double	f_ratio;

	CONSOLE_DEBUG(__FUNCTION__);

	if (gObseratorySettings.ValidInfo || (strlen(cTelescopeModel) > 0))
	{
		WriteFITS_Seperator(fitsFilePtr, "Telescope Info");
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,		"TELESCOP",
													cTelescopeModel,
													"Telescope", &fitsStatus);

		//-----------------------------------------------------------
		//*	this is from the observatory config file
		if (cTS_info.focalLen_mm > 0)
		{
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TDOUBLE,	"FOCALLEN",
													&cTS_info.focalLen_mm,
													"Focal Length in millimeters", &fitsStatus);
		}

		//*	telescope diameter information
		if (cTS_info.aperature_mm > 0)
		{

			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TDOUBLE,	"APTDIA",
													&cTS_info.aperature_mm,
													"Aperture Diameter in millimeters", &fitsStatus);

			radius			=	cTS_info.aperature_mm / 2;
			apertureArea	=	(radius * radius) * M_PI;
			//*	if we have a secondary, subtract the area of the secondary obstruction
			if (cTS_info.secondary_mm > 0)
			{
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TDOUBLE,	"OBSTDIA",
														&cTS_info.secondary_mm,
														"Obstruction Diameter in millimeters", &fitsStatus);
				radius			=	cTS_info.secondary_mm / 2;
				obstructArea	=	(radius * radius) * M_PI;
				obstructPercent	=	100.0 * (obstructArea / apertureArea);
				obstructPercent	=	round(10.0 * obstructPercent) / 10.0;
				fits_write_key(fitsFilePtr, TDOUBLE,	"OBSTPRCT",
														&obstructPercent,
														"Obstruction percent", &fitsStatus);
				apertureArea	=	apertureArea - obstructArea;
			}
			apertureArea	=	round(10.0 * apertureArea) / 10.0;
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TDOUBLE,	"APTAREA",
													&apertureArea,
													"Aperture Area in millimeters^2", &fitsStatus);

			//--------------------------------------------------------------
			f_ratio	=	cTS_info.focalLen_mm / cTS_info.aperature_mm;
			sprintf(stringBuf, "F-ratio: %1.2f",	f_ratio);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													stringBuf,
													NULL, &fitsStatus);

			imageScale	=	Calc_ImageScale(cTS_info.focalLen_mm);
			sprintf(stringBuf, "Image Scale: %5.4f (arc-seconds / micron)",	imageScale);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													stringBuf,
													NULL, &fitsStatus);

			angularResolution_arcSecs	=	Calc_AngularResolution(cTS_info.aperature_mm);
			sprintf(stringBuf, "Angular Resolution: %5.4f (arc-seconds)",	angularResolution_arcSecs);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													stringBuf,
													NULL, &fitsStatus);

			angularResolution_perPixel	=	Calc_AngularResolutionPerPixel(cTS_info.focalLen_mm, cCameraProp.PixelSizeX);
			sprintf(stringBuf, "Angular resolution per pixel: %5.4f (arc-seconds / pixel)",	angularResolution_perPixel);
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													stringBuf,
													NULL, &fitsStatus);

			fov_arcSeconds_X	=	Calc_FieldOfView_arcSecs(cTS_info.focalLen_mm, cCameraProp.PixelSizeX, cCameraProp.CameraXsize);
			fov_arcSeconds_Y	=	Calc_FieldOfView_arcSecs(cTS_info.focalLen_mm, cCameraProp.PixelSizeX, cCameraProp.CameraYsize);
			sprintf(stringBuf, "Field of view: %1.1f x %1.1f (arc-minutes)", (fov_arcSeconds_X / 60.0), (fov_arcSeconds_Y / 60.0));
			fitsStatus	=	0;
			fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
													stringBuf,
													NULL, &fitsStatus);

			//*	if its a really large field of view, report it in degrees as well.
			if ((fov_arcSeconds_X / 3600.0) >= 1.0)
			{
				sprintf(stringBuf, "Field of view: %1.1f x %1.1f (degrees)", (fov_arcSeconds_X / 3600.0), (fov_arcSeconds_Y / 3600.0));
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														stringBuf,
														NULL, &fitsStatus);
			}
		}

		//*	look through the comments and see if there any to output
		for (ii=0; ii<kMaxComments; ii++)
		{
			if (strlen(cTS_info.comments[ii].text) > 0)
			{
				fitsStatus	=	0;
				fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
														cTS_info.comments[ii].text,
														NULL, &fitsStatus);
			}
		}
	}
}

//*****************************************************************************
void	CameraDriver::WriteFITS_VersionInfo(fitsfile *fitsFilePtr)
{
int		fitsStatus;

	CONSOLE_DEBUG(__FUNCTION__);

	WriteFITS_Seperator(fitsFilePtr, "Version Info");
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"SBSTDVER",
											(void *)"SBFITSEXT Version 1.0 (March 19, 2003)",
											NULL, &fitsStatus);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"KWDICT",
											(char *)"NOAO FITS Keyword Dictionary: Version 1.0 - Jan-2000",
											NULL, &fitsStatus);

	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
											(char *)"http://iraf.noao.edu/projects/ccdmosaic/imagedef/fitsdic.html",
											NULL, &fitsStatus);
}

//*****************************************************************************
void	CameraDriver::WriteFITS_MoonInfo(fitsfile *fitsFilePtr)
{
int			fitsStatus;
float		moonAge;
float		moonIllumination;
char		moonPhaseStr[64];

	CONSOLE_DEBUG(__FUNCTION__);

	WriteFITS_Seperator(fitsFilePtr, "Moon Info");
	GetCurrentMoonPhase(moonPhaseStr);
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TSTRING,	"MOONPHAS",
											moonPhaseStr,
											"Current Moon Phase", &fitsStatus);

	moonAge		=	CalcDaysSinceNewMoon(0, 0, 0);	//*	zero -> current time
	fitsStatus	=	0;
	fits_write_key(fitsFilePtr, TFLOAT,		"MOONAGE",
											&moonAge,
											"Number of days since new moon", &fitsStatus);

	moonIllumination	=	CalcMoonIllumination(0, 0, 0);	//*	zero -> current time
	fitsStatus			=	0;
	fits_write_key(fitsFilePtr, TFLOAT,		"MOONILUM",
											&moonIllumination,
											"Percent illumination", &fitsStatus);

	if (gObseratorySettings.ValidLatLon)
	{
	// Find the last and next lunar set and rise. The utcOffset is only for Arduino.
	MoonRise	mr;
	char		lineBuff[128];

#define utcOffset 0

		time_t	t	=	time(NULL);
		mr.calculate(gObseratorySettings.Latitude, gObseratorySettings.Longitude, t + utcOffset);

		//	Returned values:
		bool	moonVisible	=	mr.isVisible;
//		bool	moonHasRise	=	mr.hasRise;
//		bool	moonHasSet	=	mr.hasSet;
//		float	moonRiseAz	=	mr.riseAz;	// Where the moon will rise/set in degrees from
//		float	moonSetAz	=	mr.setAz;	// North.

		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TLOGICAL,	"MOONVIS",
												&moonVisible,
												(moonVisible ? "Moon is visible" : "Moon is not visible"),
												&fitsStatus);


		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												(char *)"Note: These times are local time zone NOT UTC",
												NULL, &fitsStatus);


		// Additional returned values requiring conversion from UTC to local time zone
		// on the Arduino.
		time_t	moonQueryTime	=	mr.queryTime - utcOffset;
		time_t	moonRiseTime	=	mr.riseTime - utcOffset;
		time_t	moonSetTime		=	mr.setTime - utcOffset;

		// Use the results as desired (use the utcOffset variables on the Arduino):
//		printf("Moon rise/set nearest %.24s for latitude %.2f longitude %.2f:\n",
//					ctime(&mr.queryTime), gObseratorySettings.Latitude, gObseratorySettings.Longitude);

//		printf("Preceding event:\n");
		if ((!mr.hasRise || (mr.hasRise && mr.riseTime > mr.queryTime)) &&
			(!mr.hasSet || (mr.hasSet && mr.setTime > mr.queryTime)))
		{
			sprintf(lineBuff, "No moon rise or set during preceding %d hours\n", MR_WINDOW/2);
		}

		if (mr.hasRise && (mr.riseTime < mr.queryTime))
		{
			sprintf(lineBuff, "Moon rise at %.24s, Azimuth %.2f", ctime(&mr.riseTime), mr.riseAz);
		}
		if (mr.hasSet && (mr.setTime < mr.queryTime))
		{
			sprintf(lineBuff, "Moon set at %.24s, Azimuth %.2f", ctime(&mr.setTime), mr.setAz);
		}

		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												(char *)"Previous moon event:",
												NULL, &fitsStatus);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"MOONPREV",
												lineBuff,
												NULL, &fitsStatus);

		printf("Succeeding event:\n");
		if ((!mr.hasRise || (mr.hasRise && mr.riseTime < mr.queryTime)) &&
			(!mr.hasSet || (mr.hasSet && mr.setTime < mr.queryTime)))
		{
			sprintf(lineBuff, "No moon rise or set during succeeding %d hours", MR_WINDOW/2);
		}
		if (mr.hasRise && (mr.riseTime > mr.queryTime))
		{
			sprintf(lineBuff, "Moon rise at %.24s, Azimuth %.2f", ctime(&mr.riseTime), mr.riseAz);
		}
		if (mr.hasSet && (mr.setTime > mr.queryTime))
		{
			sprintf(lineBuff, "Moon set at %.24s, Azimuth %.2f", ctime(&mr.setTime), mr.setAz);
		}

		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"COMMENT",
												(char *)"Next moon event:",
												NULL, &fitsStatus);
		fitsStatus	=	0;
		fits_write_key(fitsFilePtr, TSTRING,	"MOONNEXT",
												lineBuff,
												NULL, &fitsStatus);


		if (mr.isVisible)
		{
			printf("Moon visible.\n");
		}
		else
		{
			printf("Moon not visible.\n");
		}
	}
}

#pragma mark -

//*****************************************************************************
void		CameraDriver::CreateFitsBGRimage(void)
{
long			frameBufSize;
long			ii;
long			pp;
unsigned char	*redBufPtr;
unsigned char	*grnBufPtr;
unsigned char	*bluBufPtr;

	CONSOLE_DEBUG(__FUNCTION__);

	frameBufSize	=	cCameraProp.CameraXsize * cCameraProp.CameraYsize;
	if (cCameraDataBuffer != NULL)
	{
		if (cCameraBGRbuffer == NULL)
		{
			cCameraBGRbuffer	=	(unsigned char *)malloc(frameBufSize * 3);
		}

		if (cCameraBGRbuffer != NULL)
		{
			bluBufPtr	=	cCameraBGRbuffer;
			grnBufPtr	=	cCameraBGRbuffer + frameBufSize;
			redBufPtr	=	cCameraBGRbuffer + frameBufSize + frameBufSize;
			ii	=	0;
			for (pp=0; pp<frameBufSize; pp++)
			{
				redBufPtr[pp]	=	cCameraDataBuffer[ii++];
				grnBufPtr[pp]	=	cCameraDataBuffer[ii++];
				bluBufPtr[pp]	=	cCameraDataBuffer[ii++];
			}
		}
		else
		{
			CONSOLE_DEBUG("Failed to allocated cCameraBGRbuffer");
		}
	}
	CONSOLE_DEBUG(__FUNCTION__);

}


#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_FITS_)

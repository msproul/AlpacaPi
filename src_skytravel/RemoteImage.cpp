//*****************************************************************************
//*		RemoteImage.cpp		(c) 2021 by Mark Sproul
//*
//*	Description:
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Nov 13,	2021	<MLS> Created RemoteImage.cpp
//*	Nov 13,	2021	<MLS> SDSS image retrieval working
//*	Nov 13,	2021	<MLS> stsci.edu image retrieval working
//*	Nov 16,	2022	<MLS> Added RemoteImage_OpenLatest()
//*****************************************************************************
//*	https://archive.stsci.edu/index.html
//*	https://archive.stsci.edu/cgi-bin/dss_form
//*	https://archive.stsci.edu/dss/script_usage.html
//*
//*	http://skyserver.sdss.org/dr16/en/help/docs/api.aspx
//*****************************************************************************




#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	<pthread.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"controller_image.h"

#include	"helper_functions.h"
#include	"SkyStruc.h"

#include	"RemoteImage.h"

#define	DEGREES(radians)	((radians) * (180.0 / M_PI))
#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))

char	gRemoteImageStatusMsg[64]	=	"";
bool	gRemoteImageReady			=	false;
int		gRemoteSourceID				=   kRemoteSrc_stsci_fits;

static char	gRemoteImageFilename[128]	=	"";
static char	gRemoteImgaeObjectName[128]	=	"";
static bool	gRemoteDataLogging			=	false;


TYPE_RemoteData	gRemoteDataStats[kRemoteSrc_LAST];


static	pthread_t	gRemoteImg_GetData_ThreadID;
static	double		gRemoteImg_RA_degrees;
static	double		gRemoteImg_Dec_degrees;
static	double		gRemoteImg_ArcSecondsPerPixel;
static	double		gRemoteImg_FieldOfView_deg;

static	bool		gRemoteNeedsInit			=	true;
static	bool		gRemoteImg_ThreadIsRunning	=	false;
static	bool		gRemoteImgMsgUpdated		=	false;
//*****************************************************************************
bool	IsRemoteImageRunning(void)
{
	//*	this is a tricky way to get the initialization done
	if (gRemoteNeedsInit)
	{
	int	iii;

		for (iii=0; iii<kRemoteSrc_LAST; iii++)
		{
			memset(&gRemoteDataStats, 0, sizeof(TYPE_RemoteData));
		}
		gRemoteNeedsInit	=	false;
	}

	return(gRemoteImg_ThreadIsRunning);
}

//*****************************************************************************
bool	RemoteDataMsgUpdated(void)
{
	return(gRemoteImgMsgUpdated);
}

//*****************************************************************************
void	ClearRemoteDataMsgFlag(void)
{
	gRemoteImgMsgUpdated	=	false;
}

//*****************************************************************************
static void	FormatRequestURL(	int		remoteSrc,
								double	ra_degrees,
								double	dec_degrees,
								double	scale,
								char	*urlString,
								char	*fileName)
{
char	cmdArgs[128];
double	imgWidth_arcMinutes;
double	imgHighht_arcMinutes;

	urlString[0]	=	0;
	switch(remoteSrc)
	{
		//*	archive.stsci.edu
		//  r  - right ascension
		//  d  - declination
		//  e  - equinox (B1950 or J2000; default: J2000)
		//  h  - height of image (arcminutes; default: 15.0)
		//  w  - width of image (arcminutes; default: 15.0)
		//  f  - image format (FITS or GIF; default: FITS)
		//  c  - compression (UNIX, GZIP, or NONE; default: NONE; compression
		//       applies to FITS only)
		//  v  - Which version of the survey to use:
		//       1  - First Generation survey (garden variety)
		//       2  - Second generation survey (incomplete)
		//       3  - Check the 2nd generation; if no image is available,
		//            then go to the 1st generation.
		//       4  - The Quick V survey (whence came the Guide Stars Catalog;
		//            used mostly for Phase II proposal submission)
		//  s  - Save the file to disk instead of trying to display.
		//       (ON (or anything) or not defined; default: not defined.)
		case kRemoteSrc_stsci_fits:
		case kRemoteSrc_stsci_gif:
			strcpy(gRemoteImageStatusMsg, "Waiting on archive.stsci.edu");
			gRemoteImgMsgUpdated	=	true;
			//https://archive.stsci.edu/cgi-bin/dss_search?v=all&r=15+15+30&d=+33+18+53&h=20.0&w=20.0&e=J2000&f=gif&c=none&fov=NONE
			strcpy(urlString, "https://archive.stsci.edu/cgi-bin/dss_search?v=all");

			//*	Right Ascension info & Declination info
			sprintf(cmdArgs, "&r=%f&d=%f",	ra_degrees, dec_degrees);
			strcat(urlString, cmdArgs);

			imgWidth_arcMinutes		=	gRemoteImg_FieldOfView_deg * 60;
			imgHighht_arcMinutes	=	imgWidth_arcMinutes * 0.75;
			//*	check scale limits
			//	Width parameter out of range: 0 < width <= 120.0
			//	Height parameter out of range: 0 < height <= 120.0
			if (imgWidth_arcMinutes > 100.0)
			{
				imgWidth_arcMinutes		=	100;
				imgHighht_arcMinutes	=	75;
			}

			sprintf(cmdArgs, "&h=%1.1f&w=%1.1f", imgWidth_arcMinutes, imgHighht_arcMinutes);
			strcat(urlString, cmdArgs);

			strcat(urlString, "&e=J2000&c=none&fov=NONE");
			if (remoteSrc == kRemoteSrc_stsci_fits)
			{
				strcpy(fileName, "stsci.fits");
				strcat(urlString, "&f=fits");
			}
			else
			{
				strcpy(fileName, "stsci.gif");
				strcat(urlString, "&f=gif");
			}
			break;

		case kRemoteSrc_SDSS:	//*	SDSS
			//*	scale is in arc seconds
			strcpy(gRemoteImageStatusMsg, "Waiting on skyserver.sdss.org");
			gRemoteImgMsgUpdated	=	true;
			strcpy(fileName, "sdss.jpg");
			strcpy(urlString, "http://skyserver.sdss.org/dr16/SkyServerWS/ImgCutout/getjpeg?");
			sprintf(cmdArgs, "ra=%f&dec=%f&scale=%f",
									gRemoteImg_RA_degrees,
									gRemoteImg_Dec_degrees,
									gRemoteImg_ArcSecondsPerPixel);
			strcat(urlString, cmdArgs);
			//*	add the size
			strcat(urlString, "&height=512&width=512");
			//*	add the options
			//Code	Effect on image
			//G	Grid
			//L	Label
			//P	PhotoObjs
			//S	SpecObjs
			//T	TargetObjs
			//O	Outline
			//B	BoundingBox
			//F	Fields
			//M	Masks
			//Q	Plates
			//I	InvertImage
			strcat(urlString, "&opt=GL");

			break;

	}
	CONSOLE_DEBUG(urlString);
}


//*****************************************************************************
//*	http://skyserver.sdss.org/dr16/en/help/docs/api.aspx
//*	http://skyserver.sdss.org/dr16/SkyServerWS/ImgCutout/getjpeg?ra=184.9511&dec=-0.8754&scale=1.4&height=512&width=512
//*****************************************************************************
static void	*RemoteImage_GetDataThead(void *arg)
{
char	urlString[256];
char	fileName[32];
char	curlCmdLine[512];
int		systemRetCode;
long	startTimeMilliSecs;
long	endTimeMilliSecs;
long	deltaMilliSecs;
double	deltaSeconds;
int		mySourceID;

	gRemoteImg_ThreadIsRunning	=	true;
	strcpy(gRemoteImageFilename, "");


	//*	save a copy of the source ID in case it gets changed while we are processing
	mySourceID	=	gRemoteSourceID;

	startTimeMilliSecs	=	millis();

	//*	make sure the source ID is in range
	if ((mySourceID < 0) || (mySourceID >= kRemoteSrc_LAST))
	{
		mySourceID	=	0;
	}

	FormatRequestURL(	mySourceID,
						gRemoteImg_RA_degrees,
						gRemoteImg_Dec_degrees,
						gRemoteImg_ArcSecondsPerPixel,
						urlString,
						fileName);


	strcpy(gRemoteDataStats[mySourceID].LastCmdString, urlString);


	strcpy(curlCmdLine, "curl --output ");
	strcat(curlCmdLine, fileName);
	strcat(curlCmdLine, " -X GET \"");
	strcat(curlCmdLine, urlString);
	strcat(curlCmdLine, "\"");

	switch(mySourceID)
	{
		case kRemoteSrc_stsci_fits:
			break;

		case kRemoteSrc_SDSS:
			strcat(curlCmdLine, ";./fitsview ");
			strcat(curlCmdLine, fileName);
			strcat(curlCmdLine, " >/dev/null &");
			break;
	}

	CONSOLE_DEBUG(curlCmdLine);
	systemRetCode	=	system(curlCmdLine);
	if (systemRetCode == 0)
	{
		strcpy(gRemoteImageStatusMsg, "Remote Image success");
		gRemoteImgMsgUpdated	=	true;

		//*	increment the counter
		gRemoteDataStats[mySourceID].RequestCount++;

		strcpy(gRemoteImageFilename, fileName);
		gRemoteImageReady	=	true;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("ERROR system() returned", systemRetCode);
		strcpy(gRemoteImageStatusMsg, "Remote Image failed!!!!!!!!!");
		gRemoteImgMsgUpdated	=	true;
	}

	//*	are we logging activity to disk?
	if (gRemoteDataLogging)
	{


	}

	endTimeMilliSecs	=	millis();
	deltaMilliSecs		=	endTimeMilliSecs - startTimeMilliSecs;
	deltaSeconds		=	(1.0 * deltaMilliSecs) / 1000.0;
	sprintf(curlCmdLine, " Elapsed time=%3.2f seconds", deltaSeconds);
	strcat(gRemoteImageStatusMsg, curlCmdLine);
	gRemoteImg_ThreadIsRunning	=	false;
	return(NULL);
}



//*****************************************************************************
//*	returns 0=OK, -1, failed to create, +1 busy
//*****************************************************************************
//*	inputs:
//*		arcSecondsPerPixel or fieldOfView_deg, use only one.
//*****************************************************************************
int	GetRemoteImage(		double	ra_Radians,
						double	dec_Radians,
						double	arcSecondsPerPixel,
						double	fieldOfView_deg,
						char	*objectName)
{
int		threadStatus;
int		threadErr;

	CONSOLE_DEBUG_W_STR("objectName\t=", objectName);

	threadStatus	=	-1;
	if (gRemoteImg_ThreadIsRunning == false)
	{
		gRemoteImg_ThreadIsRunning		=	true;
		gRemoteImg_RA_degrees			=	DEGREES(ra_Radians);
		gRemoteImg_Dec_degrees			=	DEGREES(dec_Radians);
		gRemoteImg_ArcSecondsPerPixel	=	arcSecondsPerPixel;
		gRemoteImg_FieldOfView_deg		=	fieldOfView_deg;
		strcpy(gRemoteImgaeObjectName, objectName);

		CONSOLE_DEBUG("Staring Remote image Thread");
		threadErr	=	pthread_create(	&gRemoteImg_GetData_ThreadID,
										NULL,
										&RemoteImage_GetDataThead,
										NULL);
		if (threadErr == 0)
		{
			CONSOLE_DEBUG("Image thread created successfully");
			threadStatus	=	0;
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Error on thread creation, Error number:", threadErr);
		}
	}
	else
	{
		threadStatus	=	1;
	}
	return(threadStatus);
}


//*****************************************************************************
void	RemoteImage_OpenLatest(void)
{
char	myOjbectName[128];

	CONSOLE_DEBUG(__FUNCTION__);

	if (strlen(gRemoteImageFilename) > 0)
	{
		CONSOLE_DEBUG_W_STR("Creating Image Window with image:", gRemoteImageFilename);

		if (strlen(gRemoteImgaeObjectName) > 0)
		{
			strcpy(myOjbectName, gRemoteImgaeObjectName);
		}
		else
		{
			strcpy(myOjbectName, gRemoteImageFilename);
		}
		CONSOLE_DEBUG_W_STR("gRemoteImgaeObjectName\t=",	gRemoteImgaeObjectName);
		CONSOLE_DEBUG_W_STR("gRemoteImageFilename  \t=",	gRemoteImageFilename);
		CONSOLE_DEBUG_W_STR("myOjbectName          \t=",	myOjbectName);

		new ControllerImage(myOjbectName, gRemoteImageFilename);

		gRemoteImageReady	=	false;
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");

}

//*****************************************************************************
//*		windowtab_moon.cpp		(c) 2021 by Mark Sproul
//*
//*	Description:	C++ Client to talk to Alpaca devices
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
//*	Edit History
//*****************************************************************************
//*	Feb  4,	2021	<MLS> Created windowtab_moon.cpp
//*	Feb 14,	2021	<MLS> Added moon image, will add phases later
//*	Mar 25,	2021	<MLS> Added support for multiple moon FITS images to step through
//*	Mar 25,	2021	<MLS> Added ReadMoonDirectory() & ReadMoonImage()
//*****************************************************************************

#include	<dirent.h>
#include	<errno.h>

#include	<fitsio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_moon.h"
#include	"controller.h"

#include	"moonphase.h"
#include	"fits_opencv.h"


#define		kMoonImageDir	"moon_fits"

//**************************************************************************************
WindowTabMoon::WindowTabMoon(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cMoonFileCnt		=	0;
	cLastUpdateTime_ms	=	0;
	cFirstRead			=	true;
	cMoonImage			=	NULL;

	strcpy(cFitsHeaderBuffer, "Fits Header");

	SetupWindowControls();

	ReadMoonDirectory();

	RunBackgroundTasks();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabMoon::~WindowTabMoon(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabMoon::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		iii;
int		saveYloc;
int		moonXloc;
int		availablePixels;
int		myButtonWidth;
//	CONSOLE_DEBUG(__FUNCTION__);


	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kMoon_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kMoon_Title, "Phase of the Moon");
	SetBGcolorFromWindowName(kMoon_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	saveYloc		=	yLoc;

//	yLoc			+=	12;

	iii	=	kMoon_AgeLbl;
	while (iii <= kMoon_Illumination)
	{

		SetWidget(				iii,	cClm1_offset,			yLoc,		cBtnWidth,		cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_Text);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		iii++;

		SetWidget(				iii,	cClm2_offset,			yLoc,		cBtnWidth,		cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_Text);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		iii++;

		yLoc			+=	cBtnHeight;
		yLoc			+=	2;
	}

	SetWidgetText(kMoon_AgeLbl,				"Moon age (days)");
	SetWidgetText(kMoon_PhaseLbl,			"Moon Phase");
	SetWidgetText(kMoon_IlluminationLbl,	"Illumination (%)");

	//-------------------------------------------------------------------
	//*	space for the FITS header
	SetWidget(				kMoon_FitsHeader,	cClm1_offset,		yLoc,	cClm3_offset,		cHeight- yLoc - 5);
	SetWidgetType(			kMoon_FitsHeader,	kWidgetType_MultiLineText);
	SetWidgetJustification(	kMoon_FitsHeader,	kJustification_Left);
	SetWidgetFont(			kMoon_FitsHeader,	kFont_Small);
	SetWidgetTextPtr(		kMoon_FitsHeader,	cFitsHeaderBuffer);

	//-------------------------------------------------------------------
	//*	navigation buttons across the top of the image area
	yLoc			=	saveYloc;
	xLoc			=	cClm3_offset + 10;
	myButtonWidth	=	cBtnWidth - 10;
	iii				=	kMoon_Btn_Prev;
	while (iii <= kMoon_Btn_Reload)
	{

		SetWidget(				iii,	xLoc,			yLoc,		myButtonWidth,		cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));
		SetWidgetBGColor(		iii,	CV_RGB(255,	255,	255));
		iii++;

		xLoc	+=	myButtonWidth;
		xLoc	+=	2;
	}
	SetWidgetText(kMoon_Btn_Prev,				"Prev");
	SetWidgetText(kMoon_Btn_Today,				"Today");
	SetWidgetText(kMoon_Btn_Next,				"Next");
	SetWidgetText(kMoon_Btn_Reload,				"Reload");


	yLoc			+=	cBtnHeight;
	yLoc			+=	2;


	cMaxMoonImgSize	=   675;
	availablePixels	=	cWidth - cClm3_offset;
	moonXloc		=	cClm3_offset + ((availablePixels - cMaxMoonImgSize) / 2);
	yLoc			+=	5;

	SetWidget(				kMoon_Image,	moonXloc,		yLoc,	cMaxMoonImgSize,		cMaxMoonImgSize);
	SetWidgetType(			kMoon_Image,	kWidgetType_Text);
	SetWidgetText(			kMoon_Image,	"Moon image goes here");
	SetWidgetBorder(		kMoon_Image,	false);
//	SetWidgetBorderColor(	kMoon_Image,	CV_RGB(0,	0,	255));
	SetWidgetOutlineBox(	kMoon_MoonImgOutline, kMoon_Image, kMoon_Image);

	cMoonImage	=	cvLoadImage("moon_fits/moon.jpg", CV_LOAD_IMAGE_COLOR);
	if (cMoonImage != NULL)
	{
		SetWidgetImage(kMoon_Image, cMoonImage);
	}
	yLoc	+=	cMaxMoonImgSize;
	yLoc	+=	2;

	//-------------------------------------------------------------------
	//*	space for the File name
	SetWidget(		kMoon_FileName,	moonXloc,		yLoc,	cMaxMoonImgSize,cBtnHeight);
	SetWidgetType(	kMoon_FileName,	kWidgetType_Text);
	SetWidgetFont(	kMoon_FileName,	kFont_Medium);
}

//**************************************************************************************
void WindowTabMoon::RunBackgroundTasks(void)
{
uint32_t	currentMilliSecs;
uint32_t	deltaMilliSecs;
double		moonAge;
double		moonIllumination;
char		moonPhaseStr[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cLastUpdateTime_ms;
//	if (cFirstRead || (deltaMilliSecs > 60000))
	if (cFirstRead || (deltaMilliSecs > 5000))
	{
		GetCurrentMoonPhase(moonPhaseStr);
		moonAge				=	CalcDaysSinceNewMoon(0, 0, 0);	//*	zero -> current time
		moonIllumination	=	CalcMoonIllumination(0, 0, 0);	//*	zero -> current time

		SetWidgetNumber(kMoon_Age,			moonAge);
		SetWidgetText(	kMoon_Phase,		moonPhaseStr);
		SetWidgetNumber(kMoon_Illumination,	moonIllumination);


		cLastUpdateTime_ms	=	currentMilliSecs;

		ForceUpdate();

		cFirstRead	=	false;
	}
}


//*****************************************************************************
void	WindowTabMoon::ProcessButtonClick(const int buttonIdx)
{
	switch(buttonIdx)
	{
		case kMoon_Btn_Prev:
			PrevImage();
			break;

		case kMoon_Btn_Today:
			break;


		case kMoon_Btn_Next:
			NextImage();
			break;

		case kMoon_Btn_Reload:
			ReadMoonDirectory();
			break;


	}
}

//**************************************************************************************
void WindowTabMoon::ReadMoonDirectory(void)
{
DIR					*directory;
struct dirent		*dir;
int					errorCode;
bool				keepGoing;
int					fileIdx;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, kMoonImageDir);


	directory	=	opendir(kMoonImageDir);
	if (directory != NULL)
	{
		keepGoing		=	true;
		fileIdx			=	0;
		while (keepGoing)
		{
			dir	=	readdir(directory);
			if (dir != NULL)
			{
				if (dir->d_name[0] == '.')
				{
					//*	ignore . and ..
				}
				else if (dir->d_type == DT_DIR)
				{
					//*	ignore directories
				}
				else
				{
					if (fileIdx < kMaxMoonFiles)
					{
						//*	make sure its a FITS file
						if (strstr(dir->d_name, ".fits") != NULL)
						{
							CONSOLE_DEBUG(dir->d_name);
							strcpy(cMoonFileList[fileIdx].fitsFileName, dir->d_name);

							fileIdx++;
						}
						else
						{
							CONSOLE_DEBUG_W_STR("Ignoring", dir->d_name);
						}
					}
					else
					{
						CONSOLE_DEBUG("Exceeded file list max count!!!!!!");
						keepGoing	=	false;
					}
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
		cMoonFileCnt	=	fileIdx;

		errorCode		=	closedir(directory);

		if (errorCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("closedir errorCode\t=", errorCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open", kMoonImageDir);
		CONSOLE_DEBUG_W_NUM("errno\t=", errno);
	}
}


//**************************************************************************************
void WindowTabMoon::ReadFitsHeader(const char *fitsFilePath)
{
fitsfile	*fptr;
char		card[FLEN_CARD];
int			status;
int			nkeys;
int			iii;

	CONSOLE_DEBUG(__FUNCTION__);

	status = 0;	///* MUST initialize status
	fits_open_file(&fptr, fitsFilePath, READONLY, &status);
	fits_get_hdrspace(fptr, &nkeys, NULL, &status);

	cFitsHeaderBuffer[0]	=	0;

	for (iii = 1; iii <= nkeys; iii++)
	{
		fits_read_record(fptr, iii, card, &status);	//* read keyword
		if (strncmp(card, "MOON", 4) == 0)
		{
			strcat(cFitsHeaderBuffer, card);
			strcat(cFitsHeaderBuffer, "\r");
		}
	}
	//strcat(cFitsHeaderBuffer, "END\r");		//* terminate listing with END
	fits_close_file(fptr, &status);

	CONSOLE_DEBUG_W_NUM("bufflen=", strlen(cFitsHeaderBuffer));
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void WindowTabMoon::ReadMoonImage(const char *moonFileName)
{
IplImage		*myOpenCVimage;
IplImage		*smallImg;
int				divideFactor;
int				newWidth;
int				newHeight;
char			myMoonFilePath[128];


	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetImage(kMoon_Image, NULL);
	if (cMoonImage != NULL)
	{
		cvReleaseImage(&cMoonImage);
		cMoonImage	=	NULL;
	}

	strcpy(myMoonFilePath, kMoonImageDir);
	strcat(myMoonFilePath, "/");
	strcat(myMoonFilePath, moonFileName);


	myOpenCVimage	=	ReadFITSimageIntoOpenCVimage(myMoonFilePath);
	//*	did we open the image OK?
	if (myOpenCVimage != NULL)
	{
		//*	check the size
		if (myOpenCVimage->width > cMaxMoonImgSize)
		{
//			CONSOLE_DEBUG("Resizing openCV image");
			//*	we have to resize the image
			newWidth		=	myOpenCVimage->width;
			newHeight		=	myOpenCVimage->height;
			divideFactor	=	1;
			while ((newWidth > cMaxMoonImgSize) || (newHeight > cMaxMoonImgSize))
			{
				divideFactor++;
				newWidth		=	myOpenCVimage->width / divideFactor;
				newHeight		=	myOpenCVimage->height / divideFactor;
			}
//			CONSOLE_DEBUG("-----------------------");
//			CONSOLE_DEBUG_W_NUM("divideFactor\t=", divideFactor);
//			CONSOLE_DEBUG_W_NUM("New width   \t=", newWidth);
//			CONSOLE_DEBUG_W_NUM("New height  \t=", newHeight);
			//*	now check the bit depth
			if ((myOpenCVimage->nChannels == 1) && (myOpenCVimage->depth == 8))
			{
//				CONSOLE_DEBUG("1 x 8");
				smallImg	=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 1);
				if (smallImg != NULL)
				{
					cvResize(myOpenCVimage, smallImg, CV_INTER_LINEAR);
					cvReleaseImage(&myOpenCVimage);
					myOpenCVimage	=	NULL;
					cMoonImage		=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 3);
					if (cMoonImage != NULL)
					{
						cvCvtColor(smallImg, cMoonImage, CV_GRAY2RGB);
					}
					else
					{
						CONSOLE_DEBUG("Failed to create cMoonImage");
					}
					cvReleaseImage(&smallImg);
				}
			}
			else if ((myOpenCVimage->nChannels == 3) && (myOpenCVimage->depth == 8))
			{
//				CONSOLE_DEBUG("3 x 8");
				CONSOLE_DEBUG_W_NUM("New width   \t=", newWidth);
				CONSOLE_DEBUG_W_NUM("New height  \t=", newHeight);
				smallImg	=	cvCreateImage(cvSize(newWidth, newHeight), IPL_DEPTH_8U, 3);
				if (smallImg != NULL)
				{
					cvResize(myOpenCVimage, smallImg, CV_INTER_LINEAR);
					cvReleaseImage(&myOpenCVimage);

					cMoonImage		=	smallImg;
				}
				else
				{
					CONSOLE_DEBUG("Failed to create small image");
				}
			}
			else
			{
				CONSOLE_DEBUG("DONT KNOW WHAT TO DO YET");
			}
		}
		else
		{
			cMoonImage	=	myOpenCVimage;
		}

		ReadFitsHeader(myMoonFilePath);
	}
	if (cMoonImage != NULL)
	{
	//	CONSOLE_DEBUG("fits image  OK");
		SetWidgetImage(kMoon_Image, cMoonImage);
		SetWidgetText(kMoon_FileName,	moonFileName);
	}
	else
	{
		CONSOLE_DEBUG("Failed to load new image");
		SetWidgetText(kMoon_FileName,	"Failed to read fits file");
	}
	ForceUpdate();
}

//**************************************************************************************
void WindowTabMoon::NextImage(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cCurrentMoonIdx++;
	if (cCurrentMoonIdx >= cMoonFileCnt)
	{
		cCurrentMoonIdx	=	0;
	}
	ReadMoonImage(cMoonFileList[cCurrentMoonIdx].fitsFileName);
}

//**************************************************************************************
void WindowTabMoon::PrevImage(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cCurrentMoonIdx--;
	if (cCurrentMoonIdx < 0)
	{
		cCurrentMoonIdx	=	cMoonFileCnt - 1;
	}
	ReadMoonImage(cMoonFileList[cCurrentMoonIdx].fitsFileName);
}

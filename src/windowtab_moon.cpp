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
//*	Sep 19,	2022	<MLS> Finished openCV C++ conversion for moon display
//*****************************************************************************

#include	<dirent.h>
#include	<errno.h>

#include	<fitsio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_moon.h"
#include	"controller.h"
#include	"helper_functions.h"

#include	"moonphase.h"
#include	"fits_opencv.h"
#include	"opencv_utils.h"

#define		kMoonImageDir	"moon_fits"

//**************************************************************************************
WindowTabMoon::WindowTabMoon(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
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

	RunWindowBackgroundTasks();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabMoon::~WindowTabMoon(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
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
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		iii++;

		SetWidget(				iii,	cClm2_offset,			yLoc,		cBtnWidth,		cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_TextBox);
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
//	SetWidgetFont(			kMoon_FitsHeader,	kFont_MonoSpace);
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
	SetWidgetType(			kMoon_Image,	kWidgetType_TextBox);
	SetWidgetText(			kMoon_Image,	"Moon image goes here");
	SetWidgetBorder(		kMoon_Image,	false);
//	SetWidgetBorderColor(	kMoon_Image,	CV_RGB(0,	0,	255));
	SetWidgetOutlineBox(	kMoon_MoonImgOutline, kMoon_Image, kMoon_Image);

#ifdef _USE_OPENCV_CPP_
	cMoonImage		=	new cv::Mat();
	if (cMoonImage != NULL)
	{
		*cMoonImage	=	cv::imread("moon_fits/moon.jpg");
	}
#elif (CV_MAJOR_VERSION <= 3)
	cMoonImage	=	cvLoadImage("moon_fits/moon.jpg", CV_LOAD_IMAGE_COLOR);
#endif // _USE_OPENCV_CPP_
	if (cMoonImage != NULL)
	{
		SetWidgetImage(kMoon_Image, cMoonImage);
	}
	yLoc	+=	cMaxMoonImgSize;
	yLoc	+=	2;

	//-------------------------------------------------------------------
	//*	space for the File name
	SetWidget(		kMoon_FileName,	moonXloc,		yLoc,	cMaxMoonImgSize,cBtnHeight);
	SetWidgetType(	kMoon_FileName,	kWidgetType_TextBox);
	SetWidgetFont(	kMoon_FileName,	kFont_Medium);
}

//**************************************************************************************
void WindowTabMoon::RunWindowBackgroundTasks(void)
{
uint32_t	currentMilliSecs;
uint32_t	deltaMilliSecs;
double		moonAge;
double		moonIllumination;
char		moonPhaseStr[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cLastUpdateTime_ms;
	if (cFirstRead || (deltaMilliSecs > 5000))
	{
		GetCurrentMoonPhase(moonPhaseStr);
		moonAge				=	CalcDaysSinceNewMoon(0, 0, 0);	//*	zero -> current time
		moonIllumination	=	CalcMoonIllumination(0, 0, 0);	//*	zero -> current time

		SetWidgetNumber(kMoon_Age,			moonAge);
		SetWidgetText(	kMoon_Phase,		moonPhaseStr);
		SetWidgetNumber(kMoon_Illumination,	moonIllumination);


		cLastUpdateTime_ms	=	currentMilliSecs;

		ForceWindowUpdate();

		cFirstRead	=	false;
	}
}


//*****************************************************************************
void	WindowTabMoon::ProcessButtonClick(const int buttonIdx, const int flags)
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
//							CONSOLE_DEBUG(dir->d_name);
							strcpy(cMoonFileList[fileIdx].fitsFileName, dir->d_name);

							fileIdx++;
						}
						else
						{
//							CONSOLE_DEBUG_W_STR("Ignoring", dir->d_name);
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
int			fitsRetCode;
char		card[FLEN_CARD];
int			status;
int			nkeys;
int			iii;
bool		includeRecord;

	CONSOLE_DEBUG(__FUNCTION__);

	status		=	0;		//* MUST initialize status
	fitsRetCode	=	fits_open_file(&fptr, fitsFilePath, READONLY, &status);
	if (fitsRetCode == 0)
	{

		cFitsHeaderBuffer[0]	=	0;
		strcat(cFitsHeaderBuffer, fitsFilePath);
		strcat(cFitsHeaderBuffer, "\r \r");

		fits_get_hdrspace(fptr, &nkeys, NULL, &status);
		for (iii = 1; iii <= nkeys; iii++)
		{
			fits_read_record(fptr, iii, card, &status);	//* read keyword
			includeRecord	=	false;

			if (strncmp(card, "BITPIX", 6) == 0)
			{
				includeRecord	=	true;
			}
			if (strncmp(card, "NAXIS ", 6) == 0)
			{
				includeRecord	=	true;
			}
			if (strncmp(card, "MOON", 4) == 0)
			{
				includeRecord	=	true;
			}
			if (strncmp(card, "CAMERA", 6) == 0)
			{
				includeRecord	=	true;
			}
			if (strncmp(card, "TELESCOP", 8) == 0)
			{
				includeRecord	=	true;
			}
			if (strncmp(card, "FILTER", 6) == 0)
			{
				includeRecord	=	true;
			}


			if (includeRecord)
			{
				strcat(cFitsHeaderBuffer, card);
				strcat(cFitsHeaderBuffer, "\r");
			}
		}
		//strcat(cFitsHeaderBuffer, "END\r");		//* terminate listing with END
		fits_close_file(fptr, &status);
	}
	CONSOLE_DEBUG_W_LONG("bufflen=", (long int)strlen(cFitsHeaderBuffer));
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void WindowTabMoon::ReadMoonImage(const char *moonFileName)
{
#if defined(_USE_OPENCV_CPP_)
	cv::Mat		*myOpenCVimage;
	cv::Mat		*smallImg;
#else
	IplImage	*myOpenCVimage;
	IplImage	*smallImg;
#endif
int				divideFactor;
int				newWidth;
int				newHeight;
int				oldImgRowStepSize;
int				oldImgChannels;
char			myMoonFilePath[128];

	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetImage(kMoon_Image, NULL);
	if (cMoonImage != NULL)
	{
#ifdef _USE_OPENCV_CPP_
		try
		{
			delete cMoonImage;
		}
		catch(cv::Exception& ex)
		{
			CONSOLE_DEBUG("delete cMoonImage; had an exception");
			CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
		}
#else
		cvReleaseImage(&cMoonImage);
#endif // _USE_OPENCV_CPP_
		cMoonImage	=	NULL;
	}

	strcpy(myMoonFilePath, kMoonImageDir);
	strcat(myMoonFilePath, "/");
	strcat(myMoonFilePath, moonFileName);


	myOpenCVimage	=	ReadFITSimageIntoOpenCVimage(myMoonFilePath);
	//*	did we open the image OK?
	if (myOpenCVimage != NULL)
	{
		CONSOLE_DEBUG("ReadFITSimageIntoOpenCVimage() OK");
	#ifdef _USE_OPENCV_CPP_
		DumpCVMatStruct(myOpenCVimage);

		newWidth			=	myOpenCVimage->cols;
		newHeight			=	myOpenCVimage->rows;
		oldImgRowStepSize	=	myOpenCVimage->step[0];
		oldImgChannels		=	myOpenCVimage->step[1];
		CONSOLE_DEBUG_W_NUM("oldImgRowStepSize\t=",		oldImgRowStepSize);
		CONSOLE_DEBUG_W_NUM("oldImgChannels\t=",		oldImgChannels);
	#else
		newWidth		=	myOpenCVimage->width;
		newHeight		=	myOpenCVimage->height;
	#endif // _USE_OPENCV_CPP_
		//*	check the size
		if (newWidth > cMaxMoonImgSize)
		{
			CONSOLE_DEBUG("Resizing openCV image");
			//*	we have to resize the image
			divideFactor	=	1;
			while ((newWidth > cMaxMoonImgSize) || (newHeight > cMaxMoonImgSize))
			{
				divideFactor++;
			#ifdef _USE_OPENCV_CPP_
				newWidth		=	myOpenCVimage->cols / divideFactor;
				newHeight		=	myOpenCVimage->rows / divideFactor;
			#else
				newWidth		=	myOpenCVimage->width / divideFactor;
				newHeight		=	myOpenCVimage->height / divideFactor;
			#endif // _USE_OPENCV_CPP_
			}
			CONSOLE_DEBUG("-----------------------");
			CONSOLE_DEBUG_W_NUM("divideFactor\t=", divideFactor);
			CONSOLE_DEBUG_W_NUM("New width   \t=", newWidth);
			CONSOLE_DEBUG_W_NUM("New height  \t=", newHeight);
		#ifdef _USE_OPENCV_CPP_
			smallImg	=	new cv::Mat(newHeight, newWidth, CV_8UC3);
			if (smallImg != NULL)
			{
				CONSOLE_DEBUG("smallImg ok");
				switch(oldImgChannels)
				{
					case 1:
						{
						cv::Mat		*rgbImage;

							rgbImage	=   ConvertImageToRGB(myOpenCVimage);
							if (rgbImage != NULL)
							{
								cv::resize(	*rgbImage,
											*smallImg,
											smallImg->size(),
											0,
											0,
											cv::INTER_LINEAR);

								delete rgbImage;
							}
							else
							{
								CONSOLE_DEBUG("ConvertImageToRGB() failed!!!");
							}
						}
						break;

					case 3:
						cv::resize(	*myOpenCVimage,
									*smallImg,
									smallImg->size(),
									0,
									0,
									cv::INTER_LINEAR);
						break;

				}
				cMoonImage	=	smallImg;
			}
			else
			{
				CONSOLE_DEBUG("new cv::Mat() failed");
			}

		#else
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
				CONSOLE_DEBUG_W_NUM("nChannels\t=", myOpenCVimage->nChannels);
				CONSOLE_DEBUG_W_NUM("depth    \t=", myOpenCVimage->depth);
			}
		#endif // _USE_OPENCV_CPP_
		}
		else
		{
			CONSOLE_DEBUG("Image size is OK");
			cMoonImage	=	myOpenCVimage;
		}

		ReadFitsHeader(myMoonFilePath);
	}

	if (cMoonImage != NULL)
	{
		CONSOLE_DEBUG("fits image  OK");
		SetWidgetImage(kMoon_Image, cMoonImage);
		SetWidgetText(kMoon_FileName,	moonFileName);
	}
	else
	{
	char	errMsgString[128];
		CONSOLE_DEBUG("Failed to load new image");
		strcpy(errMsgString, "Failed to read fits file:");
		strcat(errMsgString, moonFileName);
		SetWidgetText(kMoon_FileName,	errMsgString);
	}
	ForceWindowUpdate();
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



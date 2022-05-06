//*****************************************************************************
//*		controller.cpp		(c) 2020 by Mark Sproul
//*				Controller base class
//*
//*	Description:	This is a base class for a opencv based controller window
//*					that controls Alpaca devices
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
//*	Feb 20,	2020	<MLS> Created controller.cpp
//*	Feb 23,	2020	<MLS> Now using host name from /etc/hosts for window name
//*	Feb 25,	2020	<MLS> Added DrawWidgetMultiLineText()
//*	Mar  2,	2020	<MLS> Added background color to controller class
//*	Mar  2,	2020	<MLS> Added AlpacaGetIntegerValue() & DrawWidgetRadioButton()
//*	Mar  3,	2020	<MLS> Added AlpacaGetBooleanValue()
//*	Mar  3,	2020	<MLS> Added ProcessDoubleClick()
//*	Mar  8,	2020	<MLS> Added DrawWidgetGraph()
//*	Mar  9,	2020	<MLS> Added cLastAlpacaErrNum & cLastAlpacaErrStr
//*	Mar  9,	2020	<MLS> Control-q now sets keep running flag to false and quits
//*	Mar 14,	2020	<MLS> Control-w closes current window
//*	Mar 14,	2020	<MLS> Added Controller_HandleKeyDown()
//*	Mar 14,	2020	<MLS> Can now close a window independently
//*	Mar 14,	2020	<MLS> Image widget now centers undersized image
//*	Mar 15,	2020	<MLS> Added kFont_RadioBtn
//*	Mar 22,	2020	<MLS> Moved AlpacaGetStatus_ReadAll() into parent class
//*	Mar 23,	2020	<MLS> Added SetWidgetCrossedout()
//*	Mar 23,	2020	<MLS> Updated DrawWidgetMultiLineText() to understand justification
//*	Apr  8,	2020	<MLS> Added DrawWidgetIcon()
//*	May  5,	2020	<MLS> Added DrawOneWidget() & DrawWidgetButton()
//*	May  5,	2020	<MLS> Added button highlighting when clicked
//*	May 25,	2020	<MLS> Doubleclick in tab bar now resets window back to correct size
//*	Jun 30,	2020	<MLS> Added RefreshWindow()
//*	Jul  9,	2020	<MLS> Added UpdateWindowTabColors()
//*	Dec 29,	2020	<MLS> Added SetWidgetImage()
//*	Dec 31,	2020	<MLS> Added DisplayButtonHelpText()
//*	Jan  1,	2021	<MLS> Added UpdateWindowAsNeeded()
//*	Jan  1,	2021	<MLS> Added IsWidgetButton()
//*	Jan  6,	2021	<MLS> Started on text input field
//*	Jan  6,	2021	<MLS> Added HandleKeyDownInTextWidget()
//*	Jan  6,	2021	<MLS> Added GetWidgetText()
//*	Jan 15,	2021	<MLS> Added DrawWidgetTextWithTabs()
//*	Jan 16,	2021	<MLS> Added Close box in Window Tab area
//*	Jan 20,	2021	<MLS> Added GetCurrentTabName()
//*	Jan 26,	2021	<MLS> Added more text buffer overflow checking
//*	Feb  6,	2021	<MLS> Minor fix to tab display when clicked
//*	Feb 13,	2021	<MLS> Added UpdateSupportedActions()
//*	Feb 19,	2021	<MLS> Added ClearCapabilitiesList() & AddCapability()
//*	Feb 19,	2021	<MLS> Added UpdateCapabilityList()
//*	Feb 20,	2021	<MLS> Added ReadOneDriverCapability()
//*	Feb 27,	2021	<MLS> Added mouse event info to ProcessDoubleClick()
//*	Apr  2,	2021	<MLS> Added DrawWidgetImage()
//*	Apr  5,	2021	<MLS> Added 2nd version DrawWidgetImage()
//*	Jul 18,	2021	<MLS> Added DrawWidgetScrollBar()
//*	Aug  9,	2021	<MLS> Used try/catch for the first time (in ~Controller())
//*	Aug 11,	2021	<MLS> Added SetCurrentTab()
//*	Aug 28,	2021	<MLS> R-Pi == CV_EVENT_MOUSEHWHEEL instead of CV_EVENT_MOUSEWHEEL
//*	Sep  5,	2021	<MLS> Started on separate thread for background tasks
//*	Sep  5,	2021	<MLS> Added _USE_BACKGROUND_THREAD_
//*	Sep  5,	2021	<MLS> Added StartBackgroundThread()
//*	Sep  8,	2021	<MLS> Added startBackGroundThread option to constructor
//*	Sep  8,	2021	<MLS> Added AlpacaProcessReadAll_Common()
//*	Sep 14,	2021	<MLS> Added InitFonts()
//*	Sep 22,	2021	<MLS> Added enableDebug arg to RunBackgroundTasks()
//*	Sep 24,	2021	<MLS> Changed default behavior to NOT start background task
//*	Sep 24,	2021	<MLS> Added DumpControllerBackGroundTaskStatus() for debugging
//*	Oct  7,	2021	<MLS> Removed startBackGroundThread option from constructor
//*	Oct 25,	2021	<MLS> Added ActivateWindow()
//*	Nov 13,	2021	<MLS> Added ProcessDoubleClick_RtBtn()
//*	Dec 14,	2021	<MLS> Changed how "connected" is handled,
//*	Dec 14,	2021	<MLS> 			only connect if not already connected
//*	Dec 14,	2021	<MLS> Added CheckConnectedState()
//*	Feb 17,	2022	<MLS> Started working on opencv C++ support
//*	Feb 17,	2022	<MLS> Added SetFontInfo() and TYPE_FontInfo
//*	Feb 18,	2022	<MLS> Added DrawWidgetBackground()
//*	Feb 20,	2022	<MLS> Added a bunch of Low Level Drawing commands LLD_....
//*	Apr  9,	2022	<MLS> Made some changes to be compatible with openCV ver 2
//*	Apr 11,	2022	<MLS> Added ProcessControllerWindows()
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>



#include	"discovery_lib.h"
#include	"helper_functions.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"widget.h"
#include	"controller.h"


#ifdef _ENABLE_CVFONT_
	CvFont		gTextFont[kFont_last];
#endif

Controller	*gControllerList[kMaxControllers];
int			gControllerCnt			=	-1;
bool		gFontsNeedInit			=	true;
char		gColorOverRide			=	0;
Controller	*gCurrentActiveWindow	=	NULL;
bool		gDebugBackgroundThread	=	false;


TYPE_FontInfo	gFontInfo[kFont_last];

//*****************************************************************************
static void	SetFontInfo(TYPE_FontInfo *fontInfo, int fontID, double scale, int thickness)
{
	fontInfo->fontID	=	fontID;
	fontInfo->thickness	=	thickness;
	fontInfo->scale		=	scale;
}

//*****************************************************************************
static void	InitFonts(void)
{
//int		iii;
//double	myFontDX;


	SetFontInfo(&gFontInfo[kFont_Small],			cv::FONT_HERSHEY_PLAIN,	0.7, 1);
	SetFontInfo(&gFontInfo[kFont_RadioBtn],			cv::FONT_HERSHEY_PLAIN,	0.8, 1);
	SetFontInfo(&gFontInfo[kFont_TextList],			cv::FONT_HERSHEY_PLAIN,	0.9, 1);
	SetFontInfo(&gFontInfo[kFont_Medium],			cv::FONT_HERSHEY_PLAIN,	1.0, 1);
	SetFontInfo(&gFontInfo[kFont_Large],			cv::FONT_HERSHEY_PLAIN,	1.7, 1);
	SetFontInfo(&gFontInfo[kFont_MonoSpace],		cv::FONT_HERSHEY_PLAIN,	0.7, 1);
	SetFontInfo(&gFontInfo[kFont_Triplex_Small],	cv::FONT_HERSHEY_TRIPLEX,	0.5, 1);
	SetFontInfo(&gFontInfo[kFont_Triplex_Large],	cv::FONT_HERSHEY_TRIPLEX,	0.75, 1);
	SetFontInfo(&gFontInfo[kFont_Script_Large],		cv::FONT_HERSHEY_SCRIPT_COMPLEX,	2.00, 1);


#ifdef _ENABLE_CVFONT_
	gTextFont[kFont_Small]		=	cvFont(0.7, 1);
	gTextFont[kFont_RadioBtn]	=	cvFont(0.8, 1);
	gTextFont[kFont_TextList]	=	cvFont(0.9, 1);
	gTextFont[kFont_Medium]		=	cvFont(1.0, 1);
	gTextFont[kFont_Large]		=	cvFont(1.7, 1);
//	gTextFont[kFont_Simplex]	=	cvFont(1.7, 1);

//	cvInitFont( &gTextFont[kFont_MonoSpace], CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, CV_AA );
	cvInitFont( &gTextFont[kFont_MonoSpace], CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0, 1, CV_AA );
	gTextFont[kFont_MonoSpace].dx	=	1.5;


//	cvInitFont( &gTextFont[kFont_Simplex], CV_FONT_HERSHEY_COMPLEX, 0.5, 0.5, 0, 1, CV_AA );


	cvInitFont( &gTextFont[kFont_Triplex_Small],	CV_FONT_HERSHEY_TRIPLEX, 0.5, 0.5, 0, 1, CV_AA );
	cvInitFont( &gTextFont[kFont_Triplex_Large],	CV_FONT_HERSHEY_TRIPLEX, 0.75, 0.75, 0, 1, CV_AA );
	cvInitFont( &gTextFont[kFont_Script_Large],		CV_FONT_HERSHEY_SCRIPT_COMPLEX, 2.00, 2.00, 0, 1, CV_AA );

//	for (iii=0; iii<kFont_last; iii++)
//	{
//		myFontDX	=	gTextFont[iii].dx;
//		CONSOLE_DEBUG_W_DBL("Font dx\t=",	myFontDX);
//		CONSOLE_DEBUG_W_HEX("Font dx\t=",	gTextFont[iii].dx);
//		CONSOLE_DEBUG_W_NUM("font_face\t=",	gTextFont[iii].font_face);
//
//	}
//	CONSOLE_ABORT(__FUNCTION__);
#endif // _ENABLE_CVFONT_

	gFontsNeedInit	=	false;

}

//*****************************************************************************
static void	InitControllerList(void)
{
int		ii;

	for (ii=0; ii<kMaxControllers; ii++)
	{
		gControllerList[ii]	=	NULL;
	}
	gControllerCnt	=	0;

#ifdef __arm__
//	gDebugBackgroundThread	=	true;
#endif // __arm__
}


//*****************************************************************************
static void	LiveWindowMouseCallback(int event, int x, int y, int flags, void* param)
{
Controller	*myController;

//	CONSOLE_DEBUG(__FUNCTION__);

	myController	=	(Controller *)param;
	if (myController != NULL)
	{
		if (myController->cMagicCookie == kMagicCookieValue)
		{
//			CONSOLE_DEBUG("Magic cookie tastes good");
			gCurrentActiveWindow	=	myController;
			myController->ProcessMouseEvent(event, x, y, flags);
		}
		else
		{
			CONSOLE_DEBUG("Magic cookie is incorrect");
			CONSOLE_ABORT("Magic cookie is incorrect");
		}
	}
	else
	{
		CONSOLE_DEBUG("myController is NULL");
	}
}

//*****************************************************************************
//*	returns the number of active windows
//*****************************************************************************
int	ProcessControllerWindows(void)
{
int		activeObjCnt;
int		keyPressed;
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);
	activeObjCnt	=	0;
	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
			activeObjCnt++;
			gControllerList[iii]->HandleWindow();
		//	usleep(10);
		#if (CV_MAJOR_VERSION >= 3)
			keyPressed	=	cv::waitKeyEx(50);
		#else
			keyPressed	=	cvWaitKey(50);
		#endif
			if (keyPressed > 0)
			{
				Controller_HandleKeyDown(keyPressed);
			}

			if (gControllerList[iii]->cKeepRunning == false)
			{
				CONSOLE_DEBUG_W_NUM("Deleting control #", iii);
				CONSOLE_DEBUG_W_STR("Deleting window", gControllerList[iii]->cWindowName);
				delete gControllerList[iii];
				gControllerList[iii]	=	NULL;
			}
		}
	}
	return(activeObjCnt);
}

#if 0
//*****************************************************************************
static void	CycleThoughWindows(void)
{
int		iii;
int		nextCtrlIdx;
	//*	this should cycle through the windows, but it does not work.
	nextCtrlIdx	=	-1;

	CONSOLE_DEBUG("TAB");
	for (iii=0; iii<gControllerCnt; iii++)
	{
		if (gCurrentActiveWindow == gControllerList[iii])
		{
			nextCtrlIdx	=	iii+1;
		}
	}
	if ((nextCtrlIdx < 0) || (nextCtrlIdx >= gControllerCnt))
	{
		nextCtrlIdx	=	0;
	}
	gCurrentActiveWindow	=	gControllerList[nextCtrlIdx];

	if (gCurrentActiveWindow != NULL)
	{
		CONSOLE_DEBUG(gCurrentActiveWindow->cWindowName);

		gCurrentActiveWindow->HandleWindowUpdate();
	}
	else
	{
		CONSOLE_DEBUG("gCurrentActiveWindow is NULL");
	}
}
#endif // 0

//*****************************************************************************
void	Controller_HandleKeyDown(const int keyPressed)
{
	CONSOLE_DEBUG_W_HEX(__FUNCTION__, keyPressed);

	if (gCurrentActiveWindow != NULL)
	{
	#if 0
		if ((keyPressed & 0x0ff) == 0x09)
		{
			CycleThoughWindows();
		}
	#endif
		{
			gCurrentActiveWindow->HandleKeyDown(keyPressed);
		}
	}
	else
	{
		CONSOLE_DEBUG("No window active");
	}
}

//*****************************************************************************
static void	DumpCVMatStruct(cv::Mat *theImageMat)
{

//	CONSOLE_DEBUG_W_NUM("theImageMat->cols\t\t=",	theImageMat->cols);
//	CONSOLE_DEBUG_W_NUM("theImageMat->rows\t\t=",	theImageMat->rows);
//	CONSOLE_DEBUG_W_NUM("theImageMat->dims\t\t=",	theImageMat->dims);
//	if (theImageMat->dims >= 1)
//	{
//		CONSOLE_DEBUG_W_LONG("theImageMat->step[0]\t=",	theImageMat->step[0]);
//	}
//	if (theImageMat->dims >= 2)
//	{
//		CONSOLE_DEBUG_W_LONG("theImageMat->step[1]\t=",	theImageMat->step[1]);
//	}
//	if (theImageMat->dims >= 3)
//	{
//		CONSOLE_DEBUG_W_LONG("theImageMat->step[2]\t=",	theImageMat->step[2]);
//	}

//	CONSOLE_ABORT(__FUNCTION__);
}


//*****************************************************************************
Controller::Controller(	const char	*argWindowName,
						const int	xSize,
						const int	ySize)
{
int			iii;
int			objCntr;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, argWindowName);
	CONSOLE_DEBUG_W_NUM("xSize        \t=",	xSize);
	CONSOLE_DEBUG_W_NUM("ySize        \t=",	ySize);

	memset(&cCommonProp, 0, sizeof(TYPE_CommonProperties));

	cMagicCookie				=	kMagicCookieValue;

	cKeepRunning				=	true;
	cDebugCounter				=	0;
	cUpdateProtect				=	false;
	cHas_readall				=	false;
	cReadStartup				=	true;
	cLeftButtonDown				=	false;
	cRightButtonDown			=	false;
	cLastClicked_Btn			=	-1;
	cLastClicked_Tab			=	-1;
	cHighlightedBtn				=	-1;
	cCurTextInput_Widget		=	-1;

	//*	thread stuff
	cButtonClickInProgress		=	false;
	cBackgroundTaskActive		=	false;
	cBackGroundThreadCreated	=	false;
	cBackgroundThreadID			=	-1;

	cOnLine						=	true;		//*	assume its online, if it wasnt, we wouldnt be here
	cAlpacaVersionString[0]		=	0;
	cLastAlpacaCmdString[0]		=	0;
	cLastAlpacaErrStr[0]		=	0;
	cAlpacaDeviceTypeStr[0]		=	0;
	cAlpacaDeviceNameStr[0]		=	0;
	cLastUpdate_milliSecs		=	millis();

	//*	low level drawing stuff
	cCurrentLineWidth	=	1;

#ifdef _CONTROLLER_USES_ALPACA_
	ClearCapabilitiesList();
#endif // _CONTROLLER_USES_ALPACA_


	if (gControllerCnt < 0)
	{
		InitControllerList();
	}

	//*	find the first empty slot in the list
	for (iii=0; iii < kMaxControllers; iii++)
	{
		if (gControllerList[iii] == NULL)
		{
//			CONSOLE_DEBUG_W_STR("Controller added to list", argWindowName);
			gControllerList[iii]	=	this;
			break;
		}
	}
	//*	now go thru and see how many are in the list
	objCntr	=	0;
	for (iii=0; iii < kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
			objCntr++;
		}
	}
	gControllerCnt	=	objCntr;

	InitWindowTabs();

//	CONSOLE_DEBUG_W_NUM("xSize\t=",	xSize);
//	CONSOLE_DEBUG_W_NUM("ySize\t=",	ySize);
	strcpy(cWindowName, argWindowName);
//	CONSOLE_DEBUG_W_STR("cWindowName\t=",	cWindowName);
	cWidth				=	xSize;
	cHeight				=	ySize;
	cUpdateWindow		=	true;
	cLastAlpacaErrNum	=	kASCOM_Err_Success;

	cBackGrndColor		=	CV_RGB(0,	0,	0);

#ifdef _USE_OPENCV_CPP_
	cOpenCV_matImage	=	new cv::Mat(cHeight, cWidth, CV_8UC3);
	DumpCVMatStruct(cOpenCV_matImage);
#else
	cOpenCV_Image		=	cvCreateImage(cvSize(cWidth, cHeight), IPL_DEPTH_8U, 3);
#endif // _USE_OPENCV_CPP_

	cv::namedWindow(	cWindowName,
				//	(CV_WINDOW_NORMAL)
//	--->>>>good		(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
				#if (CV_MAJOR_VERSION >= 3)
					(cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO)
				#else
					(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO)
				#endif
				//	(CV_WINDOW_AUTOSIZE)
				//	(CV_WINDOW_NORMAL | CV_GUI_EXPANDED)
				//	(CV_WINDOW_NORMAL | CV_WINDOW_FULLSCREEN | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
				//	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
				//+	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
				//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
				//	(CV_WINDOW_AUTOSIZE)
					);

	cv::resizeWindow(	cWindowName, cWidth, cHeight);
	cv::moveWindow(		cWindowName, (20 + ((gControllerCnt - 1) * (150))), 10);

//	CONSOLE_DEBUG("Setting mouse call back routine!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	cv::setMouseCallback(	cWindowName,
							LiveWindowMouseCallback,
							(void *)this);



	if (gFontsNeedInit)
	{
		InitFonts();
	}



//	CONSOLE_DEBUG_W_DBL("font dx=", gTextFont[kFont_Simplex].dx);
//	gTextFont[kFont_Simplex].dx	=	20.0;
//	CONSOLE_DEBUG_W_DBL("font dx=", gTextFont[kFont_Simplex].dx);

	//*	set defaults for the tabs
	for (iii=0; iii < kMaxTabs; iii++)
	{
		memset((void *)&cTabList[iii], 0, sizeof(TYPE_WIDGET));
		cTabList[iii].bgColor			=	CV_RGB(128,	128,	128);
		cTabList[iii].textColor			=	CV_RGB(0,	0,		255);
		cTabList[iii].borderColor		=	CV_RGB(255,	255,	255);
		cTabList[iii].fontNum			=	kFont_Medium;
		cTabList[iii].justification		=	kJustification_Center;
		cTabList[iii].widgetType		=	kWidgetType_Button;
	}
	cTabCount		=	0;
	cCurrentTabNum	=	0;

	SetupWindowControls();

	gCurrentActiveWindow	=	this;

}


//**************************************************************************************
// Destructor
//**************************************************************************************
Controller::~Controller(void)
{
int		iii;

	CONSOLE_DEBUG_W_STR("Deleting window:\t", cWindowName);
//	CONSOLE_DEBUG_W_NUM("gControllerCnt\t=:", gControllerCnt);

#ifdef _USE_BACKGROUND_THREAD_
	//*	we have to kill the background thread
	int		threadCancelErr;
		if (cBackGroundThreadCreated)
		{
			CONSOLE_DEBUG("Canceling background thread");
			threadCancelErr	=	pthread_cancel(cBackgroundThreadID);
			if (threadCancelErr == 0)
			{
				CONSOLE_DEBUG_W_STR("Thread canceled OK:\t", cWindowName);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("Thread canceled failed, errno\t=", errno);
			}
		}
#endif // _USE_BACKGROUND_THREAD_

	//*	if we are the active window, make sure we dont get any more key presses
	if (gCurrentActiveWindow == this)
	{
		gCurrentActiveWindow	=	NULL;
	}
#ifdef _USE_OPENCV_CPP_
	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCV_matImage != NULL)
	{
	CONSOLE_DEBUG(__FUNCTION__);
//		cv::(&cOpenCV_matImage);
//		cOpenCV_matImage	=	NULL;
		//---try------try------try------try------try------try---
		try
		{
			CONSOLE_DEBUG("try delete cOpenCV_matImage");
			delete cOpenCV_matImage;
			cOpenCV_matImage	=	NULL;
		}
		catch(cv::Exception& ex)
		{
			CONSOLE_DEBUG("delete cOpenCV_matImage; had an exception");
			CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
		}
	}

#if (CV_MAJOR_VERSION >= 4)
	//---try------try------try------try------try------try---
	try
	{
	std::string	myWindowName(cWindowName);

		CONSOLE_DEBUG_W_STR("try cv::destroyWindow", cWindowName);
		CONSOLE_DEBUG_W_STR("myWindowName", myWindowName.c_str());
//		cv::destroyWindow(cWindowName);
		cv::destroyWindow(myWindowName);
		CONSOLE_DEBUG("Waiting");
		cv::waitKey(200);
	}
	catch(cv::Exception& ex)
	{
		//*	we sometimes can open the same window twice, this should not happen but sometimes does.
		//*	this catch prevents opencv from crashing
		CONSOLE_DEBUG("cvDestroyWindow() had an exception");
//+++	if (ex.code != CV_StsAssert)
		{
			CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
		}
	}
	//---end------end------end------end------end------end---
#else	//	(CV_MAJOR_VERSION >= 4)
	#warning "Compiling for openCV 3 or eariler"
	//---try------try------try------try------try------try---
	try
	{
		cvDestroyWindow(cWindowName);
	}
	catch(cv::Exception& ex)
	{
		//*	we sometimes can open the same window twice, this should not happen but sometimes does.
		//*	this catch prevents opencv from crashing
		CONSOLE_DEBUG("cvDestroyWindow() had an exception");
		if (ex.code != CV_StsAssert)
		{
			CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
		}
	}
	//---end------end------end------end------end------end---
#endif	//	(CV_MAJOR_VERSION >= 4)

#else
	//*	release the image
	if (cOpenCV_Image != NULL)
	{
//		CONSOLE_DEBUG_W_HEX("Release cOpenCV_Image", (long)cOpenCV_Image);
		cvReleaseImage(&cOpenCV_Image);
		cOpenCV_Image	=	NULL;
	}
	//---try------try------try------try------try------try---
	try
	{
		cvDestroyWindow(cWindowName);
	}
	catch(cv::Exception& ex)
	{
		//*	we sometimes can open the same window twice, this should not happen but sometimes does.
		//*	this catch prevents opencv from crashing
		CONSOLE_DEBUG("cvDestroyWindow() had an exception");
		if (ex.code != CV_StsAssert)
		{
			CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
		}
	}
	//---end------end------end------end------end------end---
#endif // _USE_OPENCV_CPP_

//	CONSOLE_DEBUG("Removing from list");
	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] == this)
		{
//			CONSOLE_DEBUG("Found controller in controller list, setting to NULL");
			gControllerList[iii]	=	NULL;
		}
	}
	CONSOLE_DEBUG("Done");
}

//**************************************************************************************
void	Controller::CheckConnectedState(void)
{
#ifdef _CONTROLLER_USES_ALPACA_
bool		validData;

	CONSOLE_DEBUG(__FUNCTION__);
	if (strlen(cAlpacaDeviceTypeStr) > 0)
	{
		//*	see if its connected
		validData	=	AlpacaGetCommonConnectedState(cAlpacaDeviceTypeStr);
		if (validData == false)
		{
			CONSOLE_DEBUG("No valid data from AlpacaGetCommonConnectedState()");
		}
		CONSOLE_DEBUG_W_NUM("cCommonProp.Connected\t=", cCommonProp.Connected);

		//*	if its not connected, send the connect command
		if (cCommonProp.Connected == false)
		{
			validData	=	AlpacaSetConnected(cAlpacaDeviceTypeStr, true);
			if (validData)
			{
				cReadStartup	=	true;
			}
		}
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
#endif // _CONTROLLER_USES_ALPACA_
}


//**************************************************************************************
void	Controller::SetupWindowControls(void)
{
//	CONSOLE_DEBUG("SetupWindowControls must be overloaded");
}

//*****************************************************************************
void	Controller::SetWindowIPaddrInfo(	const char	*textString,
											const bool	onLine)
{
int		iii;

//	CONSOLE_DEBUG_W_2STR(__FUNCTION__, cWindowName, (onLine ? "online" : "OFFLINE"));

	for (iii=0; iii<kMaxTabs; iii++)
	{
		if (cWindowTabs[iii] != NULL)
		{
			cWindowTabs[iii]->SetWindowIPaddrInfo(textString, onLine);
		}
	}
	cUpdateWindow	=	true;
}


//**************************************************************************************
void	Controller::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	Controller::HandleWindow(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

#ifndef _USE_BACKGROUND_THREAD_
	RunBackgroundTasks(__FUNCTION__, false);
#endif


	if (cUpdateWindow)
	{
//		CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
		HandleWindowUpdate();
		cUpdateWindow	=	false;
	}
}

//**************************************************************************************
void	Controller::HandleWindowUpdate(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _USE_OPENCV_CPP_
	if (cOpenCV_matImage != NULL)
	{
		//*	Do drawing
		DrawWindow();

		cv::imshow(cWindowName, *cOpenCV_matImage);
		cv::waitKey(5);
//		cvUpdateWindow(cWindowName);//
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_matImage is NULL");
	}
#else
	if (cOpenCV_Image != NULL)
	{
		//*	Do drawing
		DrawWindow();

		cvShowImage(cWindowName, cOpenCV_Image);
//		cvUpdateWindow(cWindowName);//
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_
}

//*****************************************************************************
//*	this routine only redraws the widgets marked as needing updating
//*****************************************************************************
void	Controller::UpdateWindowAsNeeded(void)
{
int				iii;
int				updatedCnt;
TYPE_WIDGET		*myWidgetPtr;

	if (cUpdateProtect)
	{
		return;
	}
	cUpdateProtect	=	true;
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);
#ifdef _USE_OPENCV_CPP_
	if (cOpenCV_matImage != NULL)
#else
	if (cOpenCV_Image != NULL)
#endif
	{
		if (cCurrentTabObjPtr != NULL)
		{
			myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
		}
		else
		{
			CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
			myWidgetPtr	=	NULL;
		}

		if (myWidgetPtr != NULL)
		{
			updatedCnt	=	0;
			//*	draw the widgets
			for (iii=0; iii<kMaxWidgets; iii++)
			{
				if (myWidgetPtr[iii].needsUpdated)
				{
					//*	first erase that rectangle with background color


					//*	draw the widget that needs updating
					DrawOneWidget(&myWidgetPtr[iii], iii);
					myWidgetPtr[iii].needsUpdated	=	false;
					updatedCnt++;
				}
			}
		#ifdef _USE_OPENCV_CPP_
			cv::imshow(cWindowName, *cOpenCV_matImage);
		#else
			cvShowImage(cWindowName, cOpenCV_Image);
		#endif
			cv::waitKey(15);
//			CONSOLE_DEBUG_W_NUM("updatedCnt\t=", updatedCnt);
		}
		else
		{
			CONSOLE_DEBUG("widget ptr is NULL");
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
	cUpdateProtect	=	false;
}


//*****************************************************************************
int	Controller::FindClickedTab(const int xxx, const int yyy)
{
int		iii;
bool	ptInWidget;
int		widgetIdx;

	widgetIdx	=	-1;
	iii			=	0;
	while ((iii < cTabCount) && (widgetIdx < 0))
	{
		if (cTabList[iii].valid)
		{
			ptInWidget	=	PointInWidget(xxx, yyy, &cTabList[iii]);
			if (ptInWidget)
			{
				widgetIdx	=	iii;
			}
		}
		iii++;
	}
	return(widgetIdx);
}


//*****************************************************************************
void	Controller::InitWindowTabs(void)
{
int		iii;

	for (iii=0; iii<kMaxTabs; iii++)
	{
		cWindowTabs[iii]	=	NULL;
	}
	cCurrentTabObjPtr	=	NULL;
}



//**************************************************************************************
//*	returns the vertical offset for the tabs
//*	a windowtab's first tab MUST be #1, NOT 0, 0 is reserved for the close box
//**************************************************************************************
int	Controller::SetTabCount(const int newTabCount)
{
int		iii;
int		tabWidth;
int		tabHeight;
int		tabLeft;

	//*	first go through and initialize all of the tab data
	for (iii=0; iii<cTabCount; iii++)
	{
		memset((void *)&cTabList[iii], 0, sizeof(TYPE_WIDGET));
	}

	if (newTabCount <= kMaxTabs)
	{
		cTabCount	=	newTabCount;
	}
	else
	{
		CONSOLE_DEBUG("Too many tabs");
		cTabCount	=	kMaxTabs;
	}



	tabHeight	=	24;
	tabLeft		=	0;
	cTabList[0].valid			=	true;
	cTabList[0].widgetType		=	kWidgetType_Button;
	cTabList[0].left			=	tabLeft;
	cTabList[0].top				=	0;
	cTabList[0].width			=	tabHeight;
	cTabList[0].height			=	tabHeight;
	cTabList[0].fontNum			=	kFont_Large;
	cTabList[0].crossedOut		=	true;
	cTabList[0].textString[0]	=	'X';
	cTabList[0].textString[1]	=	0;
	cTabList[0].textColor		=	CV_RGB(255, 0, 0);

	tabLeft	+=	tabHeight + 1;

	tabWidth	=	((cWidth - tabLeft) / (cTabCount - 1)) - 1;

	for (iii=1; iii<cTabCount; iii++)
	{
		cTabList[iii].valid			=	true;
//		cTabList[iii].widgetType	=	kWidgetType_TextBox;
		cTabList[iii].widgetType	=	kWidgetType_Button;
		cTabList[iii].left			=	tabLeft;
		cTabList[iii].top			=	0;
		cTabList[iii].width			=	tabWidth;
		cTabList[iii].height		=	tabHeight;
		cTabList[iii].fontNum		=	kFont_Small;

		sprintf(cTabList[iii].textString, "Tab-%d", (iii+1));
		tabLeft	+=	tabWidth + 1;
	}
	cCurrentTabNum		=	1;
	cUpdateWindow		=	true;
	return(tabHeight + 2);
}

//**************************************************************************************
void	Controller::SetTabText(const int tabIdx, const char *tabName)
{

	if ((tabIdx >= 0) && (tabIdx < kMaxTabs))
	{
		strcpy(cTabList[tabIdx].textString, tabName);
		cUpdateWindow	=	true;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("tabIdx out of range\t=", tabIdx);
	}
}

//**************************************************************************************
void	Controller::GetCurrentTabName(char *currentTabName)
{
	if ((cCurrentTabNum >= 0) && (cCurrentTabNum < kMaxTabs))
	{
		strcpy(currentTabName, cTabList[cCurrentTabNum].textString);
	}
}

//*****************************************************************************
void	Controller::SetCurrentTab(const int tabIdx)
{
	if ((tabIdx >= 1) && (tabIdx < cTabCount))
	{
		cCurrentTabNum		=	tabIdx;
		cUpdateWindow		=	true;
		cCurrentTabObjPtr	=	cWindowTabs[cCurrentTabNum];
	}
}

//*****************************************************************************
void	Controller::ProcessTabClick(const int tabIdx)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (tabIdx == 0)
	{
		cKeepRunning	=	false;
	}
	else if (tabIdx != cCurrentTabNum)
	{
		if ((cCurrentTabNum >= 1) && (cCurrentTabNum < cTabCount))
		{
//			CONSOLE_DEBUG_W_NUM("Switching tabs to #", tabIdx);
			cCurrentTabNum		=	tabIdx;
			cUpdateWindow		=	true;
			cCurrentTabObjPtr	=	cWindowTabs[cCurrentTabNum];

			if (cCurrentTabObjPtr != NULL)
			{
				cCurrentTabObjPtr->ActivateWindow();
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Tab number out of bounds:", tabIdx);
			CONSOLE_DEBUG_W_NUM("cTabCount\t=", cTabCount);
			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		//*	do nothing
//		CONSOLE_DEBUG("Do nothing");
	}
}


//*****************************************************************************
int	Controller::FindClickedWidget(const int xxx, const int yyy)
{
int		widgetIdx;

	if (cCurrentTabObjPtr != NULL)
	{
		widgetIdx	=	cCurrentTabObjPtr->FindClickedWidget(xxx, yyy);
	}
	else
	{
		widgetIdx	=	-1;
		CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
	}
	return(widgetIdx);
}

//*****************************************************************************
bool	Controller::IsWidgetButton(const int widgetIdx)
{
bool		widgetIsButton;

	if (cCurrentTabObjPtr != NULL)
	{
		widgetIsButton	=	cCurrentTabObjPtr->IsWidgetButton(widgetIdx);
	}
	else
	{
		widgetIsButton	=	false;
		CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
	}
	return(widgetIsButton);
}

//*****************************************************************************
bool	Controller::IsWidgetTextInput(const int widgetIdx)
{
bool		widgetIsButton;

	if (cCurrentTabObjPtr != NULL)
	{
		widgetIsButton	=	cCurrentTabObjPtr->IsWidgetTextInput(widgetIdx);
	}
	else
	{
		widgetIsButton	=	false;
		CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
	}
	return(widgetIsButton);
}

//*****************************************************************************
void	Controller::ProcessButtonClick(const int buttonIdx)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCurrentTabObjPtr != NULL)
	{
		cCurrentTabObjPtr->ProcessButtonClick(buttonIdx);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Un-handled button click, tab=\t",	cCurrentTabNum);
		CONSOLE_DEBUG_W_NUM("buttonIdx=\t",	buttonIdx);
	}
}

//*****************************************************************************
void	Controller::ProcessDoubleClick(	const int	widgetIdx,
										const int	event,
										const int	xxx,
										const int	yyy,
										const int	flags)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (cCurrentTabObjPtr != NULL)
	{
		cCurrentTabObjPtr->ProcessDoubleClick(widgetIdx,  event,  xxx,  yyy,  flags);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Un-handled DOUBLE click, tab=\t",	cCurrentTabNum);
		CONSOLE_DEBUG_W_NUM("widgetIdx=\t",	widgetIdx);
	}
}

//*****************************************************************************
void	Controller::ProcessDoubleClick_RtBtn(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCurrentTabObjPtr != NULL)
	{
		cCurrentTabObjPtr->ProcessDoubleClick_RtBtn(widgetIdx,  event,  xxx,  yyy,  flags);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Un-handled DOUBLE click, tab=\t",	cCurrentTabNum);
		CONSOLE_DEBUG_W_NUM("widgetIdx=\t",	widgetIdx);
	}
}


//*****************************************************************************
void	Controller::DisplayButtonHelpText(const int buttonIdx)
{
bool	updateOccurred;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCurrentTabObjPtr != NULL)
	{
		updateOccurred	=	cCurrentTabObjPtr->DisplayButtonHelpText(buttonIdx);
		if (updateOccurred)
		{
			UpdateWindowAsNeeded();
		}
	}
}


//*****************************************************************************
void	Controller::ProcessMouseEvent(int event, int xxx, int yyy, int flags)
{
int		clickedBtn;
int		myWidgitIdx;
bool	widgetIsButton;
int		wheelMovement;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("EVENT=", event);
	myWidgitIdx	=	FindClickedWidget(xxx,  yyy);
	switch(event)
	{
	//	case CV_EVENT_MOUSEMOVE:
		case cv::EVENT_MOUSEMOVE:
		//	CONSOLE_DEBUG("CV_EVENT_MOUSEMOVE");
			cCurrentMouseX	=	xxx;
			cCurrentMouseY	=	yyy;
			if (cLeftButtonDown)
			{
				if (cHighlightedBtn >= 0)
				{
					//*	if we are no longer in the button, turn off highlighting
					if (myWidgitIdx != cHighlightedBtn)
					{
						SetWidgetHighlighted(cCurrentTabNum, cHighlightedBtn, false);
						DrawOneWidget(cHighlightedBtn);
						cHighlightedBtn	=	-1;
					}
				}
				else if ((cLastClicked_Btn >= 0) && (myWidgitIdx == cLastClicked_Btn))
				{
					//*	we are back in the clicked button, highlight it again
					cHighlightedBtn	=	cLastClicked_Btn;
					//*	highlight the button
					SetWidgetHighlighted(cCurrentTabNum, cHighlightedBtn, true);
					DrawOneWidget(cHighlightedBtn);
				}
				else if (cCurrentTabObjPtr != NULL)
				{
					cCurrentTabObjPtr->ProcessMouseLeftButtonDragged(myWidgitIdx, event,  xxx,  yyy,  flags);
				}
			}
			else
			{
				//*	this will allow the display of help text
				myWidgitIdx	=	FindClickedWidget(xxx,  yyy);
				if (myWidgitIdx >= 0)
				{
					DisplayButtonHelpText(myWidgitIdx);
				}
			}
			break;

		case cv::EVENT_LBUTTONDOWN:
//			CONSOLE_DEBUG("CV_EVENT_LBUTTONDOWN");
			cCurTextInput_Widget	=	-1;
			cLeftButtonDown			=	true;
			cLastLClickX			=	xxx;
			cLastLClickY			=	yyy;
			//*	keep track of the what button/tab was clicked on
			cLastClicked_Tab		=	FindClickedTab(xxx,  yyy);
			if (cLastClicked_Tab >= 0)
			{
//				CONSOLE_DEBUG_W_NUM("cLastClicked_Tab\t=", cLastClicked_Tab);
				cTabList[cLastClicked_Tab].highLighted	=	true;
				DrawWidgetButton(&cTabList[cLastClicked_Tab]);
			#ifdef _USE_OPENCV_CPP_
				cv::imshow(cWindowName, *cOpenCV_matImage);
			#else
				cvShowImage(cWindowName, cOpenCV_Image);
			#endif // _USE_OPENCV_CPP_
				cTabList[cLastClicked_Tab].highLighted	=	false;
			}

			cLastClicked_Btn		=	FindClickedWidget(xxx,  yyy);
			if (cLastClicked_Btn >= 0)
			{
				widgetIsButton	=	IsWidgetButton(cLastClicked_Btn);
				if (widgetIsButton)
				{
	//				CONSOLE_DEBUG("Highlighting button");
					cHighlightedBtn	=	cLastClicked_Btn;
					//*	highlight the button
					SetWidgetHighlighted(cCurrentTabNum, cHighlightedBtn, true);
					DrawOneWidget(cHighlightedBtn);
				}
				else if (IsWidgetTextInput(cLastClicked_Btn))
				{
					cCurTextInput_Widget	=	cLastClicked_Btn;
					cLastClicked_Btn		=	-1;
//					CONSOLE_DEBUG("Text input!!!!!");
				}
				else
				{
					cLastClicked_Btn	=	-1;
//					CONSOLE_DEBUG("Not a button or text input");
				}

				if (cCurrentTabObjPtr != NULL)
				{
					cCurrentTabObjPtr->ProcessMouseLeftButtonDown(myWidgitIdx, event,  xxx,  yyy,  flags);
				}
			}
			break;

		case cv::EVENT_RBUTTONDOWN:
			cRightButtonDown	=	true;
			CONSOLE_DEBUG("CV_EVENT_RBUTTONDOWN");
			break;

		case cv::EVENT_MBUTTONDOWN:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONDOWN");
			break;

		case cv::EVENT_LBUTTONUP:
//			CONSOLE_DEBUG("CV_EVENT_LBUTTONUP");
			cLeftButtonDown	=	false;
			if (cHighlightedBtn >= 0)
			{
//				CONSOLE_DEBUG("Turning OFF highlight");
				//*	turn off button highlite
				SetWidgetHighlighted(cCurrentTabNum, cHighlightedBtn, false);
				DrawOneWidget(cHighlightedBtn);
				cHighlightedBtn	=	-1;
			}
			//*	check for tab click first
			clickedBtn		=	FindClickedTab(xxx,  yyy);
			if (clickedBtn >= 0)
			{
				if (clickedBtn == cLastClicked_Tab)
				{
					ProcessTabClick(clickedBtn);
				}
			//	CONSOLE_DEBUG(__FUNCTION__);
			}
			else
			{
				//*	now check for button (widget) click
				clickedBtn		=	FindClickedWidget(xxx,  yyy);
				if ((clickedBtn >= 0) && (clickedBtn == cLastClicked_Btn))
				{
					ProcessButtonClick(clickedBtn);
				}
			}
			cLastClicked_Btn	=	-1;


			if (cCurrentTabObjPtr != NULL)
			{
				cCurrentTabObjPtr->ProcessMouseLeftButtonUp(myWidgitIdx, event,  xxx,  yyy,  flags);
			}
			break;

		case cv::EVENT_RBUTTONUP:
			cRightButtonDown	=	false;
			CONSOLE_DEBUG("CV_EVENT_RBUTTONUP");
			break;

		case cv::EVENT_MBUTTONUP:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONUP");
			break;

		case cv::EVENT_LBUTTONDBLCLK:
		//	CONSOLE_DEBUG("CV_EVENT_LBUTTONDBLCLK");
			clickedBtn		=	FindClickedTab(xxx,  yyy);
			if (clickedBtn >= 0)
			{
				//*	we have a double click in the tab bar
			//	cvResizeWindow(cWindowName, cWidth, cHeight);
				cv::resizeWindow(cWindowName, cWidth, cHeight);
			}
			else
			{
				clickedBtn		=	FindClickedWidget(xxx,  yyy);
//				CONSOLE_DEBUG_W_NUM("Double click on widget#\t",	clickedBtn);
				if (clickedBtn >= 0)
				{
					ProcessDoubleClick(clickedBtn,  event,  xxx,  yyy,  flags);
				}
			}
			break;

		case cv::EVENT_RBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_RBUTTONDBLCLK");
			clickedBtn		=	FindClickedTab(xxx,  yyy);
			if (clickedBtn >= 0)
			{
				CONSOLE_DEBUG("CV_EVENT_RBUTTONDBLCLK");
				//*	we have a double click in the tab bar
				cv::resizeWindow(cWindowName, cWidth, cHeight);
			}
			else
			{
				CONSOLE_DEBUG("CV_EVENT_RBUTTONDBLCLK");
				clickedBtn		=	FindClickedWidget(xxx,  yyy);
//				CONSOLE_DEBUG_W_NUM("Double click on widget#\t",	clickedBtn);
				if (clickedBtn >= 0)
				{
					ProcessDoubleClick_RtBtn(clickedBtn,  event,  xxx,  yyy,  flags);
				}
			}
			break;

		case cv::EVENT_MBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONDBLCLK");
			break;

#if (CV_MAJOR_VERSION >= 3)
		case cv::EVENT_MOUSEWHEEL:
		case cv::EVENT_MOUSEHWHEEL:
//			CONSOLE_DEBUG_W_HEX("flags\t=", flags);
			wheelMovement	=	flags & 0xffff0000;
			wheelMovement	/=	65536;
			if (cCurrentTabObjPtr != NULL)
			{
				cCurrentTabObjPtr->ProcessMouseWheelMoved(	myWidgitIdx,
															event,
															xxx,
															yyy,
															wheelMovement,
															flags);
			}
			break;
#else
	#warning "Mouse wheel events not supported, "
#endif
		default:
			CONSOLE_DEBUG_W_NUM("UNKNOWN EVENT", event);
			break;
	}

	//*	the window tab may also want to deal with a mouse event
	if (cCurrentTabObjPtr != NULL)
	{
		cCurrentTabObjPtr->ProcessMouseEvent(myWidgitIdx, event,  xxx,  yyy,  flags);
	}
//	if (event == cv::EVENT_LBUTTONUP)
//	{
//		CONSOLE_DEBUG("exit");
//	}
}

//**************************************************************************************
void	Controller::DrawWidgetBackground(TYPE_WIDGET *theWidget)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, theWidget->textString);
	cCurrentColor	=	theWidget->bgColor;
	LLD_FillRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);


	if (theWidget->includeBorder)
	{
		cCurrentColor	=	theWidget->borderColor;
		LLD_FrameRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);
	}
}

//**************************************************************************************
void	Controller::DrawWidgetButton(TYPE_WIDGET *theWidget)
{

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, theWidget->textString);
#ifdef _USE_OPENCV_CPP_
	cv::Scalar	myGBcolor;
#else
	cv::Scalar	myGBcolor;
#endif // _USE_OPENCV_CPP_
	DrawWidgetBackground(theWidget);

	myGBcolor	=	theWidget->bgColor;
	if (theWidget->highLighted)
	{
//		CONSOLE_DEBUG("Button is highlighted");
		myGBcolor.val[0]	*=	0.75;
		myGBcolor.val[1]	*=	0.75;
		myGBcolor.val[2]	*=	0.75;
		myGBcolor.val[3]	*=	0.75;
		cCurrentColor	=	myGBcolor;
		LLD_FillRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);
	}
	if (theWidget->selected)
	{
		cCurrentColor	=	theWidget->borderColor;
		LLD_FillRect(	theWidget->left		+ 3,
						theWidget->top		+ 3,
						theWidget->width	- 6,
						theWidget->height	- 6);
	}

	if (theWidget->includeBorder)
	{
		cCurrentColor	=	theWidget->borderColor;
		LLD_FrameRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);
	}
	DrawWidgetText(theWidget);
}

//**************************************************************************************
void	Controller::DrawWidgetTextWithTabs(TYPE_WIDGET *theWidget)
{
int			textOffsetY;
int			curFontNum;
char		textBuffer[kMaxWidgetStrLen];
int			ccc;
int			iii;
char		theChar;
int			sLen;
int			currentTabStop;
int			textLoc_X;
int			textLoc_Y;

//	CONSOLE_DEBUG(__FUNCTION__);

	DrawWidgetBackground(theWidget);

	sLen	=	strlen(theWidget->textString);
	if (sLen > 0)
	{
		LLD_GetTextSize("test", theWidget->fontNum);
		curFontNum		=	theWidget->fontNum;

		textLoc_X	=	theWidget->left + 7;
		textOffsetY	=	(theWidget->height / 2) - (cCurrentFontHeight / 2) + cCurrentFontBaseLine + 5;
		textLoc_Y	=	theWidget->top + textOffsetY;

		ccc				=	0;
		currentTabStop	=	-1;
		cCurrentColor	=	theWidget->textColor;
		for (iii=0; iii<=sLen; iii++)
		{
			theChar	=	theWidget->textString[iii];
			if ((theChar == 0x09) || (theChar == 0))
			{
				textBuffer[ccc]	=	0;
				textLoc_X		=	theWidget->left + 7;
				if (currentTabStop >= 0)
				{
					textLoc_X	+=	theWidget->tabStops[currentTabStop];
				}
				LLD_DrawCString(textLoc_X, textLoc_Y, textBuffer, curFontNum);

				currentTabStop++;
				ccc			=	0;
			}
			else
			{
				textBuffer[ccc++]	=	theChar;
				textBuffer[ccc]		=	0;
			}
		}
	}
}

//**************************************************************************************
//*	this routine ONLY draws the text,
//**************************************************************************************
void	Controller::DrawWidgetText(TYPE_WIDGET *theWidget, int horzOffset, int vertOffset)

{
//CvSize		textSize;
int			textWidthPixels;
int			textLoc_X;
int			textLoc_Y;
int			textOffsetX;
int			textOffsetY;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (strlen(theWidget->textString) > 0)
	{
		textWidthPixels		=	LLD_GetTextSize(theWidget->textString, theWidget->fontNum);
		switch (theWidget->justification)
		{
			case kJustification_Left:
				textLoc_X	=	theWidget->left + 7;
				break;

			case kJustification_Center:
			case kJustification_Right:
			default:
				textOffsetX	=	(theWidget->width / 2) - (textWidthPixels / 2);
				textLoc_X	=	theWidget->left + textOffsetX;
				break;

		}
		textOffsetY	=	(theWidget->height / 2) - (cCurrentFontHeight / 2) + cCurrentFontBaseLine + 5;
		textLoc_Y	=	theWidget->top + textOffsetY;

		//*	add in the offsets passed in
		textLoc_X	+=	horzOffset;
		textLoc_Y	+=	vertOffset;

		if (theWidget->highLighted)
		{
			textLoc_X	+=	2;
			textLoc_Y	+=	2;
		}
		cCurrentColor	=	theWidget->textColor;

		LLD_DrawCString(textLoc_X, textLoc_Y, theWidget->textString, theWidget->fontNum);
	}
}

//**************************************************************************************
void	Controller::DrawWidgetTextBox(TYPE_WIDGET *theWidget)
{
	DrawWidgetBackground(theWidget);
	DrawWidgetText(theWidget);
}

#define	kMaxTextLineLen	512
//**************************************************************************************
void	Controller::DrawWidgetMultiLineText(TYPE_WIDGET *theWidget)
{
char		lineBuff[kMaxTextLineLen];
int			iii;
int			ccc;
int			sLen;
int			textOffsetX;
bool		drawTextFlg;
char		theChar;
char		previousChar;
char		*myStringPtr;
int			textWidthPixels;
int			textLoc_X;
int			textLoc_Y;

//	CONSOLE_DEBUG(__FUNCTION__);
	DrawWidgetBackground(theWidget);
	if (theWidget->textPtr != NULL)
	{
		myStringPtr	=	theWidget->textPtr;
	}
	else
	{
		myStringPtr	=	theWidget->textString;
	}

	if (strlen(myStringPtr) > 0)
	{
		previousChar	=	0;
		textWidthPixels	=	LLD_GetTextSize("test", theWidget->fontNum);

		textLoc_X	=	theWidget->left + 10;
		textLoc_Y	=	theWidget->top + cCurrentFontHeight + cCurrentFontBaseLine;

		ccc			=	0;
		sLen		=	strlen(myStringPtr);
		drawTextFlg	=	false;
		for (iii=0; iii<=sLen; iii++)
		{
			theChar	=	myStringPtr[iii];
			if (theChar > 0x20)
			{
				if (ccc < kMaxTextLineLen)
				{
					lineBuff[ccc++]	=	theChar;
					lineBuff[ccc]	=	0;
				}
			}
			else if (theChar == 0x20)
			{
				if (ccc < kMaxTextLineLen)
				{
					lineBuff[ccc++]	=	theChar;
					lineBuff[ccc]	=	0;
				}

				//*	check the width of the line to make sure its going to fit
				textWidthPixels	=	LLD_GetTextSize(lineBuff, theWidget->fontNum);
				if (textWidthPixels > (theWidget->width - 150))
				{
					drawTextFlg	=	true;
				}
			}
			else
			{
				//*	its a control char (probably a CR or LF), draw the line
				if ((previousChar != 0x0d) && (previousChar != 0x0a))
				{
					drawTextFlg		=	true;
				}
			}
			previousChar	=	theChar;

			if (drawTextFlg)
			{
				textWidthPixels	=	LLD_GetTextSize(lineBuff, theWidget->fontNum);

				//*	this allows for blank lines
				if (strlen(lineBuff) > 0)
				{
					switch (theWidget->justification)
					{
						case kJustification_Left:
							textLoc_X	=	theWidget->left + 7;
							break;

						case kJustification_Center:
						case kJustification_Right:
						default:
							textOffsetX	=	(theWidget->width / 2) - (textWidthPixels / 2);
							textLoc_X	=	theWidget->left + textOffsetX;
							break;

					}
					cCurrentColor	=	theWidget->textColor;
					LLD_DrawCString(textLoc_X, textLoc_Y, lineBuff, theWidget->fontNum);
				}
				ccc				=	0;
				lineBuff[ccc]	=	0;
				textLoc_Y		+=	cCurrentFontHeight;
				textLoc_Y		+=	cCurrentFontBaseLine;
				drawTextFlg		=	false;
			}
		}
	}
}

//**************************************************************************************
void	Controller::DrawWidgetRadioButton(TYPE_WIDGET *theWidget)
{
int			radius;
int			pt1_X;
int			pt1_Y;

	//*	Radio button calculations
	pt1_X		=	theWidget->left + (theWidget->height / 2);
	pt1_Y		=	theWidget->top + (theWidget->height / 2);
	radius		=	theWidget->height / 3;

	DrawWidgetBackground(theWidget);
	//*	draw the radio button
	cCurrentColor	=	theWidget->borderColor;
	LLD_FrameEllipse(pt1_X, pt1_Y, radius, radius);
	//*	check to see if its selected
	if (theWidget->selected)
	{
		cCurrentColor	=	theWidget->borderColor;
		LLD_FillEllipse(pt1_X, pt1_Y, radius - 2, radius - 2);
	}

	DrawWidgetText(theWidget, ((2 * radius) + 8));
}

//**************************************************************************************
void	Controller::DrawWidgetCheckBox(TYPE_WIDGET *theWidget)
{
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;

#define	kInset	2

	DrawWidgetBackground(theWidget);
	//*	draw the Check box

	cCurrentColor	=	CV_RGB(128, 128, 128);
	LLD_FillRect(theWidget->left,	theWidget->top,	theWidget->height,	theWidget->height);

	//*	check to see if its selected
	if (theWidget->selected)
	{
		pt1_X	=	theWidget->left + kInset;
		pt1_Y	=	theWidget->top + kInset;

		pt2_X	=	theWidget->left + theWidget->height - kInset;
		pt2_Y	=	theWidget->top + theWidget->height - kInset;
		cCurrentColor	=	theWidget->borderColor;
		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_LineTo(pt2_X, pt2_Y);

		pt1_X	=	theWidget->left + theWidget->height - kInset;
		pt1_Y	=	theWidget->top + kInset;

		pt2_X	=	theWidget->left + kInset;
		pt2_Y	=	theWidget->top + theWidget->height - kInset;
		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_LineTo(pt2_X, pt2_Y);

	}
	DrawWidgetText(theWidget, (theWidget->height + 8), 0);
}

//**************************************************************************************
void	Controller::DrawWidgetSlider(TYPE_WIDGET *theWidget)
{
int			textOffsetY;
double		sliderDelata;
char		leftString[32];
char		rightString[32];
int			vertCenter;
int			radius;
int			sliderOffset;
int			textLoc_X;
int			textLoc_Y;
int			textWidthPixels;
int			sliderLeft;
int			sliderWidth;

	sliderDelata	=	theWidget->sliderMax - theWidget->sliderMin;
	if (sliderDelata >= 10)
	{
		sprintf(leftString,		"%1.0f",	theWidget->sliderMin);
		sprintf(rightString,	"%1.0f",	theWidget->sliderMax);
	}
	else if (sliderDelata >= 1)
	{
		sprintf(leftString,		"%1.1f",	theWidget->sliderMin);
		sprintf(rightString,	"%1.1f",	theWidget->sliderMax);
	}
	else
	{
		sprintf(leftString,		"%1.3f",	theWidget->sliderMin);
		sprintf(rightString,	"%1.3f",	theWidget->sliderMax);
	}

	DrawWidgetBackground(theWidget);

	textWidthPixels		=	LLD_GetTextSize(rightString, theWidget->fontNum);

	//*	calculate location for left string
	textLoc_X	=	theWidget->left + 7;
	textOffsetY	=	(theWidget->height / 2) - (cCurrentFontHeight / 2) + cCurrentFontBaseLine + 5;
	textLoc_Y	=	theWidget->top + textOffsetY;


	LLD_DrawCString(textLoc_X, textLoc_Y, leftString, theWidget->fontNum);

	//*	calculate location for right string
	textLoc_X	=	(theWidget->left + theWidget->width)- textWidthPixels;
	textLoc_X	-=	5;
	LLD_DrawCString(textLoc_X, textLoc_Y, rightString, theWidget->fontNum);

	//*	now draw the slider itself
	vertCenter		=	theWidget->top + (theWidget->height / 2);
	sliderLeft		=	theWidget->left + 50;
	sliderWidth		=	theWidget->width - 100;
	LLD_FrameRect(	sliderLeft,
					vertCenter - 1,
					sliderWidth,
					3);

	//*	draw the indicator
	sliderOffset	=	(theWidget->sliderValue / sliderDelata) * sliderWidth;
	textLoc_X		=	sliderLeft + sliderOffset;
	textLoc_Y		=	vertCenter;
	radius			=	4;

//		cCurrentColor	=	theWidget->textColor;
	cCurrentColor	=	CV_RGB(255, 0, 0);
	LLD_FillEllipse(textLoc_X, textLoc_Y, radius, radius);
}

//**************************************************************************************
void	Controller::DrawWidgetScrollBar(TYPE_WIDGET *theWidget)
{
cv::Rect		myCVrect;
cv::Point		sliderLoc;
//int			textOffsetY;
//char		leftString[32];
//char		rightString[32];
int			vertCenter;
bool		scrollBarIsVertical;
int			scrollBar_left;
int			scrollBar_top;
int			scrollBar_width;
int			scrollBar_height;
int			scrollBar_Travel;

	DrawWidgetBackground(theWidget);

	//*	figure out it if it is horizontal or veritcal
	if (theWidget->height > theWidget->width)
	{
		scrollBarIsVertical	=	true;
	}
	else
	{
		scrollBarIsVertical	=	false;
	}

	//*	now draw the slider itself
	if (scrollBarIsVertical)
	{
		scrollBar_left		=	theWidget->left + 1;
		scrollBar_width		=	theWidget->width - 2;
		scrollBar_Travel	=	theWidget->height;

		if (theWidget->scrollBarMax > 0)
		{
			scrollBar_top		=	(theWidget->scrollBarValue * scrollBar_Travel) / theWidget->scrollBarMax;
		}
		else
		{
			scrollBar_top		=	theWidget->top + 1;
		}
		scrollBar_height	=	20;
	}
	else
	{
		vertCenter			=	theWidget->top + (theWidget->height / 2);
		scrollBar_left		=	theWidget->left + 50;
		scrollBar_top		=	vertCenter - 1;
		scrollBar_width		=	theWidget->width - 100;
		scrollBar_height	=	3;
	}
//		cCurrentColor	=	theWidget->textColor;
	cCurrentColor	=	CV_RGB(0, 255, 255);

	LLD_FillRect(scrollBar_left,	scrollBar_top,	scrollBar_width,	scrollBar_height);
}


//**************************************************************************************
void	Controller::DrawWidgetOutlineBox(TYPE_WIDGET *theWidget)
{
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;

//	CONSOLE_DEBUG(__FUNCTION__);

	cCurrentColor	=	theWidget->borderColor;
	LLD_FrameRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);

	if (theWidget->crossedOut)
	{
		pt1_X	=	theWidget->left;
		pt1_Y	=	theWidget->top;

		pt2_X	=	theWidget->left + theWidget->width;
		pt2_Y	=	theWidget->top + theWidget->height;
		cCurrentColor		=	CV_RGB(255, 0, 0);
		cCurrentLineWidth	=	2;

		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_LineTo(pt2_X, pt2_Y);


		pt1_X	=	theWidget->left + theWidget->width;
		pt1_Y	=	theWidget->top;

		pt2_X	=	theWidget->left;
		pt2_Y	=	theWidget->top + theWidget->height;
		LLD_MoveTo(pt1_X, pt1_Y);
		LLD_LineTo(pt2_X, pt2_Y);
		cCurrentLineWidth	=	1;
	}
}

//**************************************************************************************
void	Controller::DrawWidgetGraph(TYPE_WIDGET *theWidget)
{
int		iii;
int		pt1_X;
int		pt1_Y;
int		pt2_X;
int		pt2_Y;

	DrawWidgetBackground(theWidget);
	if (theWidget->graphArrayPtr != NULL)
	{
		cCurrentColor	=	CV_RGB(255,	0,	0);
		for (iii=0; iii<theWidget->graphArrayCnt; iii++)
		{
			pt1_X	=	iii + 3;
			pt1_Y	=	theWidget->top + theWidget->height;

			pt2_X	=	iii + 3;
			pt2_Y	=	pt1_Y - (theWidget->graphArrayPtr[iii] * 3);
			LLD_MoveTo(pt1_X, pt1_Y);
			LLD_LineTo(pt2_X, pt2_Y);
//				cvLine(	cOpenCV_Image,
//						pt1,
//						pt2,
//						CV_RGB(255,	0,	0),	//	color,
//						1,					//	int thickness CV_DEFAULT(1),
//						8,					//	int line_type CV_DEFAULT(8),
//						0);					//	int shift CV_DEFAULT(0));

		}
	}
}



//**************************************************************************************
void	Controller::DrawWidgetIcon(TYPE_WIDGET *theWidget)
{
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;

	DrawWidgetBackground(theWidget);
	cCurrentColor	=	theWidget->textColor;
	switch(theWidget->iconNum)
	{
		case kIcon_UpArrow:
			pt1_X	=	theWidget->left + (theWidget->width / 2);
			pt1_Y	=	theWidget->top + 1;

			pt2_X	=	theWidget->left + (theWidget->width / 2);
			pt2_Y	=	theWidget->top + theWidget->height;
			LLD_MoveTo(pt1_X, pt1_Y);
			LLD_LineTo(pt2_X, pt2_Y);

			pt2_X	=	theWidget->left;
			pt2_Y	=	theWidget->top + (theWidget->width / 2);
			LLD_MoveTo(pt1_X, pt1_Y);
			LLD_LineTo(pt2_X, pt2_Y);

			pt2_X	=	theWidget->left + theWidget->width;
			pt2_Y	=	theWidget->top + (theWidget->width / 2);
			LLD_MoveTo(pt1_X, pt1_Y);
			LLD_LineTo(pt2_X, pt2_Y);
			break;

		case kIcon_DownArrow:
			pt1_X	=	theWidget->left + (theWidget->width / 2);
			pt1_Y	=	theWidget->top +1;

			pt2_X	=	theWidget->left + (theWidget->width / 2);
			pt2_Y	=	theWidget->top + theWidget->height - 1;
			LLD_MoveTo(pt1_X, pt1_Y);
			LLD_LineTo(pt2_X, pt2_Y);


			pt1_X	=	theWidget->left + (theWidget->width / 2);
			pt1_Y	=	theWidget->top + theWidget->height - 2;

			pt2_X	=	theWidget->left;
			pt2_Y	=	theWidget->top + (theWidget->width / 2);
			LLD_MoveTo(pt1_X, pt1_Y);
			LLD_LineTo(pt2_X, pt2_Y);

			pt2_X	=	theWidget->left + theWidget->width;
			pt2_Y	=	theWidget->top + (theWidget->width / 2);
			LLD_MoveTo(pt1_X, pt1_Y);
			LLD_LineTo(pt2_X, pt2_Y);
			break;
	}
}

//**************************************************************************************
void	Controller::DrawWidgetProgressBar(TYPE_WIDGET *theWidget)
{
double	percentComplete;
char	textString[64];
int		newWidth;
int		textLoc_X;
int		textLoc_Y;

//	CONSOLE_DEBUG(__FUNCTION__);
	percentComplete	=	theWidget->sliderValue / theWidget->sliderMax;
	sprintf(textString, "Downloading - %3.1f%% complete", (percentComplete * 100.0));

	DrawWidgetBackground(theWidget);

	cCurrentColor	=	theWidget->bgColor;
	LLD_FillRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);

	newWidth		=	theWidget->width * percentComplete;
	cCurrentColor	=	theWidget->textColor;
	LLD_FillRect(theWidget->left,	theWidget->top,	newWidth,	theWidget->height);

	cCurrentColor	=	theWidget->borderColor;
	LLD_FrameRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);

	textLoc_X	=	theWidget->left + 10;
	textLoc_Y	=	theWidget->top + (theWidget->height / 2) + 5;
	LLD_DrawCString(textLoc_X, textLoc_Y, textString, kFont_Medium);
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	Controller::DrawWidgetImage(TYPE_WIDGET *theWidget, cv::Mat *theOpenCVimage)
{
int			delta;
cv::Rect	widgetRoiRect;
cv::Mat		image_roi;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	if (theWidget->left == 272)
//	{
//		CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//		DumpWidget(theWidget);
//	}

	if (cOpenCV_matImage != NULL)
	{
		if (theOpenCVimage != NULL)
		{
//			CONSOLE_DEBUG("Drawing image");
			widgetRoiRect.x			=	theWidget->left;
			widgetRoiRect.y			=	theWidget->top;
			widgetRoiRect.width		=	theWidget->width;
			widgetRoiRect.height	=	theWidget->height;

			//*	check to see if the image is BIGGER than the widget rect
			if (theOpenCVimage->cols > widgetRoiRect.width)
			{
				CONSOLE_DEBUG_W_NUM("Image is too big", theOpenCVimage->cols);
			}

			theWidget->roiRect	=	widgetRoiRect;
//	CONSOLE_DEBUG_W_NUM("theWidget->roiRect.x\t=",		theWidget->roiRect.x);
//	CONSOLE_DEBUG_W_NUM("theWidget->roiRect.y\t=",		theWidget->roiRect.y);
//	CONSOLE_DEBUG_W_NUM("theWidget->roiRect.width\t=",	theWidget->roiRect.width);
//	CONSOLE_DEBUG_W_NUM("theWidget->roiRect.height\t=",	theWidget->roiRect.height);
			//*	we have to make sure the the destination rect is the same as the src rect
			//*	if its smaller, then center it
			if (theOpenCVimage->cols < widgetRoiRect.width)
			{
				delta					=	widgetRoiRect.width - theOpenCVimage->cols;
				theWidget->roiRect.x	=	widgetRoiRect.x + (delta / 2);
			}
			if (theOpenCVimage->rows < widgetRoiRect.height)
			{
				delta					=	widgetRoiRect.height - theOpenCVimage->rows;
				theWidget->roiRect.y	=	widgetRoiRect.y + (delta / 2);
			}
			theWidget->roiRect.width	=	theOpenCVimage->cols;
			theWidget->roiRect.height	=	theOpenCVimage->rows;

//			cvSetImageROI(cOpenCV_matImage,  theWidget->roiRect);
//			cvCopy(theWidget->openCVimagePtr, cOpenCV_matImage);
//			cvResetImageROI(cOpenCV_matImage);

			//---try------try------try------try------try------try---
			try
			{
				image_roi		=	cv::Mat(*cOpenCV_matImage, theWidget->roiRect);
			}
			catch(cv::Exception& ex)
			{
				CONSOLE_DEBUG("=========================================================");
				CONSOLE_DEBUG("cv::Mat() had an exception");
				CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);

				CONSOLE_DEBUG_W_NUM("theWidget->roiRect.x\t=",		theWidget->roiRect.x);
				CONSOLE_DEBUG_W_NUM("theWidget->roiRect.y\t=",		theWidget->roiRect.y);
				CONSOLE_DEBUG_W_NUM("theWidget->roiRect.width\t=",	theWidget->roiRect.width);
				CONSOLE_DEBUG_W_NUM("theWidget->roiRect.height\t=",	theWidget->roiRect.height);
			}
			//---try------try------try------try------try------try---
			try
			{
				//*	mostly working
				theOpenCVimage->copyTo(image_roi);
			}
			catch(cv::Exception& ex)
			{
				//*	this catch prevents opencv from crashing
				CONSOLE_DEBUG("=========================================================");
				CONSOLE_DEBUG("copyTo() had an exception");
				CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
			//	CONSOLE_ABORT(__FUNCTION__);
			}

			//*	draw the border if enabled
			if (theWidget->includeBorder)
			{
			//	CONSOLE_DEBUG(__FUNCTION__);
				LLD_FrameRect(&theWidget->roiRect);
			}
		}
		else
		{
			CONSOLE_DEBUG("Image ptr is null");
			if (theWidget->includeBorder)
			{
				widgetRoiRect.x			=	theWidget->left;
				widgetRoiRect.y			=	theWidget->top;
				widgetRoiRect.width		=	theWidget->width;
				widgetRoiRect.height	=	theWidget->height;

				LLD_FrameRect(&widgetRoiRect);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
}

#else
//**************************************************************************************
void	Controller::DrawWidgetImage(TYPE_WIDGET *theWidget, IplImage *theOpenCVimage)
{
int			delta;
cv::Rect	widgetRect;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cOpenCV_Image != NULL)
	{
		if (theOpenCVimage != NULL)
		{

		//	CONSOLE_DEBUG_W_NUM("Drawing image, widget#", widgetIdx);
			widgetRect.x		=	theWidget->left;
			widgetRect.y		=	theWidget->top;
			widgetRect.width	=	theWidget->width;
			widgetRect.height	=	theWidget->height;

			//*	check to see if the image is BIGGER than the widget rect
			if (theOpenCVimage->width > widgetRect.width)
			{
				CONSOLE_DEBUG_W_NUM("Image is too big", theOpenCVimage->width);
			}
		#if (CV_MAJOR_VERSION == 3)
			theWidget->roiRect			=	widgetRect;
		#else
			theWidget->roiRect.x		=	widgetRect.x;
			theWidget->roiRect.y		=	widgetRect.y;
			theWidget->roiRect.width	=	widgetRect.width;
			theWidget->roiRect.height	=	widgetRect.height;
		#endif
			//*	we have to make sure the the destination rect is the same as the src rect
			//*	if its smaller, then center it
			if (theOpenCVimage->width < widgetRect.width)
			{
				delta					=	widgetRect.width - theOpenCVimage->width;
				theWidget->roiRect.x	=	widgetRect.x + (delta / 2);
			}
			if (theOpenCVimage->height < widgetRect.height)
			{
				delta					=	widgetRect.height - theOpenCVimage->height;
				theWidget->roiRect.y	=	widgetRect.y + (delta / 2);
			}
			theWidget->roiRect.width	=	theOpenCVimage->width;
			theWidget->roiRect.height	=	theOpenCVimage->height;

			cvSetImageROI(cOpenCV_Image,  theWidget->roiRect);
			cvCopy(theWidget->openCVimagePtr, cOpenCV_Image);
			cvResetImageROI(cOpenCV_Image);

			//*	draw the border if enabled
			if (theWidget->includeBorder)
			{

				cvRectangleR(	cOpenCV_Image,
								theWidget->roiRect,
								theWidget->borderColor,		//	color,
								1,							//	int thickness CV_DEFAULT(1),
								8,							//	int line_type CV_DEFAULT(8),
								0);							//	int shift CV_DEFAULT(0));
			}

		}
		else
		{
			CONSOLE_DEBUG("Image ptr is null");
			if (theWidget->includeBorder)
			{
				widgetRect.x		=	theWidget->left;
				widgetRect.y		=	theWidget->top;
				widgetRect.width	=	theWidget->width;
				widgetRect.height	=	theWidget->height;

				cvRectangleR(	cOpenCV_Image,
								widgetRect,
								theWidget->borderColor,		//	color,
								1,							//	int thickness CV_DEFAULT(1),
								8,							//	int line_type CV_DEFAULT(8),
								0);							//	int shift CV_DEFAULT(0));
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is null");
	}
}
#endif // _USE_OPENCV_CPP_


//**************************************************************************************
void	Controller::DrawWidgetImage(TYPE_WIDGET *theWidget)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (theWidget->openCVimagePtr != NULL)
	{
		DrawWidgetImage(theWidget, theWidget->openCVimagePtr);
	}
	else
	{
		CONSOLE_DEBUG("theWidget->openCVimagePtr is null");
		cCurrentColor	=	CV_RGB(100, 100, 100);
		LLD_FillRect(theWidget->left, theWidget->top, theWidget->width, theWidget->height);
	}
}

//**************************************************************************************
void	Controller::DrawWindowTabs(void)
{
int				iii;

	//*	draw the tabs
	if (cTabCount > 0)
	{
		//*	first set the bg colors
		for (iii=0; iii<cTabCount; iii++)
		{
			if (iii == cCurrentTabNum)
			{
				cTabList[iii].bgColor	=	CV_RGB(220,	220,	220);
			}
			else
			{
				cTabList[iii].bgColor	=	CV_RGB(128,	128,	128);
			}
		}


		for (iii=0; iii<cTabCount; iii++)
		{
			if (cTabList[iii].valid)
			{
				DrawWidgetTextBox(&cTabList[iii]);
			}
		}
	}
}

//**************************************************************************************
//*	this will re-draw one widget in the CURRENT tab
void	Controller::DrawOneWidget(const int widgetIdx)
{
TYPE_WIDGET		*myWidgetPtr;

#ifdef _USE_OPENCV_CPP_
	if (cOpenCV_matImage != NULL)
	{
		if (cCurrentTabObjPtr != NULL)
		{
			myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
			if (myWidgetPtr != NULL)
			{
				DrawOneWidget(&myWidgetPtr[widgetIdx], widgetIdx);
				cv::imshow(cWindowName, *cOpenCV_matImage);
			}
		}
	}
#else
	if (cOpenCV_Image != NULL)
	{
		if (cCurrentTabObjPtr != NULL)
		{
			myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
			if (myWidgetPtr != NULL)
			{
				DrawOneWidget(&myWidgetPtr[widgetIdx], widgetIdx);
				cvShowImage(cWindowName, cOpenCV_Image);
			}
		}
	}
#endif
}

//**************************************************************************************
void	Controller::DrawOneWidget(TYPE_WIDGET *theWidget, const int widgetIdx)
{
cv::Rect		widgetRect;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	theWidget->needsUpdated	=	false;	//*	record the fact that this widget has been updated
	switch(theWidget->widgetType)
	{
		case kWidgetType_Button:
			DrawWidgetButton(theWidget);
			break;

		case kWidgetType_Graph:
			DrawWidgetGraph(theWidget);
			if (theWidget->includeBorder)
			{
				cCurrentColor	=	theWidget->borderColor;
				LLD_FrameRect(theWidget->left,	theWidget->top,	theWidget->width,	theWidget->height);
			}
			break;

		case kWidgetType_CustomGraphic:
			if (cCurrentTabObjPtr != NULL)
			{
				DrawWidgetBackground(theWidget);
			#ifdef _USE_OPENCV_CPP_
				cCurrentTabObjPtr->DrawWidgetCustomGraphic(cOpenCV_matImage, widgetIdx);
			#else
				cCurrentTabObjPtr->DrawWidgetCustomGraphic(cOpenCV_Image, widgetIdx);
			#endif
			}
			else
			{
				CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
			}
			break;

		case kWidgetType_Image:
			DrawWidgetImage(theWidget);
			break;

		case kWidgetType_MultiLineText:
			DrawWidgetMultiLineText(theWidget);
			break;

		case kWidgetType_RadioButton:
			DrawWidgetRadioButton(theWidget);
			break;

		case kWidgetType_CheckBox:
			DrawWidgetCheckBox(theWidget);
			break;

		case kWidgetType_Slider:
			DrawWidgetSlider(theWidget);
			break;

		case kWidgetType_ScrollBar:
			DrawWidgetScrollBar(theWidget);
			break;

		case kWidgetType_OutlineBox:
			DrawWidgetOutlineBox(theWidget);
			break;

		case kWidgetType_Icon:
			DrawWidgetIcon(theWidget);
			break;

		case kWidgetType_ProessBar:
			DrawWidgetProgressBar(theWidget);
			break;

		case kWidgetType_TextBox:
		default:
			if (theWidget->hasTabs)
			{
				DrawWidgetTextWithTabs(theWidget);
			}
			else
			{
				DrawWidgetTextBox(theWidget);
			}
			break;
	}
}


//**************************************************************************************
void	Controller::DrawWindowWidgets(void)
{
int				iii;
TYPE_WIDGET		*myWidgetPtr;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	if (cCurrentTabObjPtr != NULL)
	{
		myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
	}
	else
	{
		CONSOLE_DEBUG(__FUNCTION__);
		CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
		myWidgetPtr	=	NULL;
	}

	if (myWidgetPtr != NULL)
	{
		//*	draw the widgets
		for (iii=0; iii<kMaxWidgets; iii++)
		{
			if (myWidgetPtr[iii].valid)
			{
				DrawOneWidget(&myWidgetPtr[iii], iii);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("widget ptr is NULL");
	}
}


//**************************************************************************************
void	Controller::DrawWindow(void)
{

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
#ifdef _USE_OPENCV_CPP_
	if (cOpenCV_matImage != NULL)
	{
	cv::Rect	myCVrect;
		myCVrect.x		=	0;
		myCVrect.y		=	0;
		myCVrect.width	=	cWidth;
		myCVrect.height	=	cHeight;

		cv::rectangle(	*cOpenCV_matImage,
						myCVrect,
						cBackGrndColor,			//	color,
					#if (CV_MAJOR_VERSION >= 3)
						cv::FILLED				//	int thickness CV_DEFAULT(1),
					#else
						CV_FILLED
					#endif
						);

		DrawWindowTabs();

		//*	draw the widgets
		DrawWindowWidgets();
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
#else
	if (cOpenCV_Image != NULL)
	{
	CvRect		myCVrect;
		myCVrect.x		=	0;
		myCVrect.y		=	0;
		myCVrect.width	=	cWidth;
		myCVrect.height	=	cHeight;
		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						cBackGrndColor,			//	color,
					#if (CV_MAJOR_VERSION >= 3)
						cv::FILLED,				//	int thickness CV_DEFAULT(1),
					#else
						CV_FILLED,
					#endif
						8,						//	int line_type CV_DEFAULT(8),
						0);						//	int shift CV_DEFAULT(0));

		DrawWindowTabs();

		//*	draw the widgets
		DrawWindowWidgets();
	}
#endif // _USE_OPENCV_CPP_
}


//**************************************************************************************
void	Controller::HandleKeyDown(const int keyPressed)
{
int			openCVerr;
char		imageFileName[512];
#ifndef _USE_OPENCV_CPP_
int			quality[3] = {16, 200, 0};
#endif
bool		stillNeedsHandled;
char		currentTabName[64]	=	"";

//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", keyPressed);

	stillNeedsHandled	=	true;

	//*	check for control key
	if (keyPressed & 0x040000)
	{
		//*	we have a control key
//		CONSOLE_DEBUG("Control is down");
		stillNeedsHandled	=	false;

		switch(tolower(keyPressed & 0x007f))
		{
			case 'r':
				RefreshWindow();
				break;

			//	ctrl-s   ^s
			case 'd':
			case 's':
				CONSOLE_DEBUG("Save file");
				GetCurrentTabName(currentTabName);
				sprintf(imageFileName, "%s-%s-screenshot.jpg", cWindowName, currentTabName);
			#ifdef _USE_OPENCV_CPP_
			//	openCVerr	=	cv::imwrite(imageFileName, *cOpenCV_matImage, quality);
				openCVerr	=	cv::imwrite(imageFileName, *cOpenCV_matImage);
			#else
				openCVerr	=	cvSaveImage(imageFileName, cOpenCV_Image, quality);
			#endif
				CONSOLE_DEBUG_W_NUM("openCVerr\t=", openCVerr);
				break;

			//*	^q  ctrl-q
			case 'q':
//				CONSOLE_DEBUG_W_STR("Quit from  window \t=", cWindowName);
				gKeepRunning	=	false;
				break;

			//*	^w ctrl-w close window
			case 'w':
//				CONSOLE_DEBUG_W_STR("Close window \t=", cWindowName);
				cKeepRunning	=	false;
				break;

			default:
				stillNeedsHandled	=	true;
				break;
		}
	}
	if (stillNeedsHandled)
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		if (cCurTextInput_Widget >= 0)
		{
			HandleKeyDownInTextWidget(cCurrentTabNum, cCurTextInput_Widget, keyPressed);
		}
		else if (cCurrentTabObjPtr != NULL)
		{
			cCurrentTabObjPtr->HandleKeyDown(keyPressed);
		}
	}
}

//**************************************************************************************
void	Controller::HandleKeyDownInTextWidget(const int tabNum, const int widgetIdx,const int keyPressed)
{
char	myTextString[512];
int		textLen;
int		myChar;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", keyPressed);
//	CONSOLE_DEBUG_W_HEX("0x00ffe0  \t=", (keyPressed & 0x00ffe0));

	GetWidgetText(tabNum, widgetIdx, myTextString);
	textLen	=	strlen(myTextString);
	myChar	=	0;
	//*	check for control key down
	if (keyPressed & 0x040000)
	{
		myChar	=	keyPressed & 0x1f;
	}
	else if ((keyPressed & 0x00ffe0) == 0x00ffe0)
	{
		//	0x10FFE1
		//	0x00FFE0
		//*	ignore these
		myChar	=	0;
//		CONSOLE_DEBUG_W_HEX("myChar\t=", myChar);
	}
	else if ((keyPressed & 0x00ff80) == 0x0ff00)
	{
		myChar	=	keyPressed & 0x7f;
		if (myChar >= 0x20)
		{
			myChar	=	0;
		}
	}
	else
	{
		myChar	=	keyPressed & 0x7f;
	}

//	CONSOLE_DEBUG_W_HEX("myChar\t=", myChar);
	switch(myChar)
	{
		case 0x08:	//*	back space
			textLen--;
			if (textLen < 0)
			{
				textLen	=	0;
			}
			myTextString[textLen]	=	0;
			break;

		default:
			if (myChar >= 0x20)
			{
				myTextString[textLen]	=	myChar;
				myTextString[textLen+1]	=	0;
			}
	}

//	CONSOLE_DEBUG_W_STR("myTextString\t=", myTextString);

	strcat(myTextString, "_");
	SetWidgetText(tabNum, widgetIdx, myTextString);
}


//**************************************************************************************
void	Controller::RefreshWindow(void)
{
//*	do nothing, this is intended for a subclass to over ride if needed
}

#pragma mark -

//**************************************************************************************
//*	these routines handle multiple tabs
//**************************************************************************************

//**************************************************************************************
void	Controller::SetTabWindow(const int tabNum, WindowTab *theTabObjectPtr)
{
	if ((tabNum >= 0) && (tabNum < kMaxTabs))
	{
		cWindowTabs[tabNum]	=	theTabObjectPtr;
		//*	If the current tab is NOT set, set it tab 1
		if ((tabNum == 1) && (cCurrentTabObjPtr == NULL))
		{
			cCurrentTabObjPtr	=	theTabObjectPtr;
			cCurrentTabNum		=	tabNum;
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetValid(	const int tabNum, const int widgetIdx, bool valid)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, widgetIdx);
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetValid(widgetIdx, valid);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetText(const int tabNum, const int widgetIdx, const char *newText)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("tabNum\t=",	tabNum);
//	CONSOLE_DEBUG_W_NUM("widgetIdx\t=",	widgetIdx);
//	CONSOLE_DEBUG_W_STR("newText\t=",	newText);
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetText(widgetIdx, newText);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
			CONSOLE_DEBUG("cWindowTabs[tabNum] is NULL")
		}
	}
	else
	{
		CONSOLE_DEBUG("tabNum is out of bounds")
	}
}

//**************************************************************************************
void	Controller::GetWidgetText(const int tabNum, const int widgetIdx, char *getText)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->GetWidgetText(widgetIdx, getText);
		}
		else
		{
			CONSOLE_DEBUG("cWindowTabs[tabNum] is NULL")
		}
	}
	else
	{
		CONSOLE_DEBUG("tabNum is out of bounds")
	}
}


//**************************************************************************************
void	Controller::SetWidgetNumber(const int tabNum, const int widgetIdx, const int number)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetNumber(widgetIdx, number);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetNumber(const int tabNum, const int widgetIdx, const double number)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetNumber(widgetIdx, number);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetType(	const int tabNum, const int widgetIdx, const int widetType)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetType(widgetIdx, widetType);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetFont(const int tabNum, const int widgetIdx, int fontNum)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetFont(widgetIdx, fontNum);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	Controller::SetWidgetTextColor(		const int tabNum, const int widgetIdx, cv::Scalar newtextColor)
#else
//**************************************************************************************
void	Controller::SetWidgetTextColor(		const int tabNum, const int widgetIdx, cv::Scalar newtextColor)
#endif
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetTextColor(widgetIdx, newtextColor);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	Controller::SetWidgetBGColor(const int tabNum, const int widgetIdx, cv::Scalar newBGcolor)
#else
//**************************************************************************************
void	Controller::SetWidgetBGColor(const int tabNum, const int widgetIdx, cv::Scalar newBGcolor)
#endif
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetBGColor(widgetIdx, newBGcolor);
			if (tabNum == cCurrentTabNum)
			{
//				CONSOLE_DEBUG("update window")
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	Controller::SetWidgetBorderColor(	const int tabNum, const int widgetIdx, cv::Scalar newBorderColor)
#else
//**************************************************************************************
void	Controller::SetWidgetBorderColor(	const int tabNum, const int widgetIdx, cv::Scalar newBorderColor)
#endif
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetBorderColor(widgetIdx, newBorderColor);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	Controller::SetWidgetImage(	const int tabNum, const int widgetIdx, cv::Mat *argImagePtr)
#else
//**************************************************************************************
void	Controller::SetWidgetImage(	const int tabNum, const int widgetIdx, IplImage *argImagePtr)
#endif
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetImage(widgetIdx, argImagePtr);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetChecked(		const int tabNum, const int widgetIdx, bool checked)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetChecked(widgetIdx, checked);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetCrossedout(const int tabNum, const int widgetIdx, bool crossedout)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetCrossedout(widgetIdx, crossedout);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetSliderLimits(	const int	tabNum,
											const int	widgetIdx,
											double		sliderMin,
											double		sliderMax)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetSliderLimits(widgetIdx, sliderMin, sliderMax);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetSliderValue(	const int	tabNum,
											const int	widgetIdx,
											double		sliderValue)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetSliderValue(widgetIdx, sliderValue);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetHighlighted(	const int	tabNum,
											const int	widgetIdx,
											bool		highlighted)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetHighlighted(widgetIdx, highlighted);
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetProgress(	const int	tabNum,
										const int	widgetIdx,
										const int	currPosition,
										const int	totalValue)
{
TYPE_WIDGET		*myWidgetPtr;

//	CONSOLE_DEBUG_W_NUM("currPosition\t=", currPosition);
//	CONSOLE_DEBUG_W_NUM("totalValue\t=", totalValue);

	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetProgress(widgetIdx, currPosition, totalValue);

			if (cCurrentTabObjPtr != NULL)
			{
				myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
				if (myWidgetPtr != NULL)
				{
					DrawOneWidget(&myWidgetPtr[widgetIdx], widgetIdx);
				#ifdef _USE_OPENCV_CPP_
					cv::imshow(cWindowName, *cOpenCV_matImage);
				#else
					cvShowImage(cWindowName, cOpenCV_Image);
				#endif // _USE_OPENCV_CPP_
				}
			}
		}
		else
		{
		}
	}
}



//**************************************************************************************
void	Controller::UpdateWindowTabColors(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

#pragma mark -

//*****************************************************************************
bool	PointInWidget(const int xPoint, const int yPoint, TYPE_WIDGET *theWidget)
{
bool	inRect;

	inRect	=	true;
	if (theWidget->valid == false)
	{
		inRect	=	false;
	}
	else if (xPoint < theWidget->left)
	{
		inRect	=	false;
	}
	else if (xPoint > (theWidget->left + theWidget->width))
	{
		inRect	=	false;
	}
	else if (yPoint < theWidget->top)
	{
		inRect	=	false;
	}
	else if (yPoint > (theWidget->top + theWidget->height))
	{
		inRect	=	false;
	}

	return(inRect);
}


//*****************************************************************************
void	DumpWidget(TYPE_WIDGET *theWidget)
{
	CONSOLE_DEBUG_W_NUM("theWidget->valid\t\t=",		theWidget->valid);
	CONSOLE_DEBUG_W_NUM("theWidget->widgetType\t=",		theWidget->widgetType);
	CONSOLE_DEBUG_W_NUM("theWidget->left\t\t=",			theWidget->left);
	CONSOLE_DEBUG_W_NUM("theWidget->top\t\t=",			theWidget->top);
	CONSOLE_DEBUG_W_NUM("theWidget->width\t\t=",		theWidget->width);
	CONSOLE_DEBUG_W_NUM("theWidget->height\t\t=",		theWidget->height);
	CONSOLE_DEBUG_W_NUM("theWidget->includeBorder\t=",	theWidget->includeBorder);
	CONSOLE_DEBUG_W_NUM("theWidget->fontNum\t\t=",		theWidget->fontNum);
	CONSOLE_DEBUG_W_NUM("theWidget->selected\t=",		theWidget->selected);
}




//******************************************************************************
//*	<C 20# >00
//*	<C 01# >ffff
//*	<C 02# >0000
//*	<C 03# >39e7
//*	<C 04# >f800
//*	<C 05# >f800
//*	<C 06# >8000
//*	<C 07# >fe00
//*	<C 08# >07ff
//*	<C 12# >39e7
//*	<C 14# >ffff
//*	<C 11# >000f
//*	<C 13# >f800
//******************************************************************************
#ifdef _USE_OPENCV_CPP_
//******************************************************************************
cv::Scalar	Color16BitTo24Bit(const unsigned int color16)
#else
//******************************************************************************
cv::Scalar	Color16BitTo24Bit(const unsigned int color16)
#endif
{
int			redValue;
int			grnValue;
int			bluValue;
#ifdef _USE_OPENCV_CPP_
	cv::Scalar	rgbValue;
#else
	cv::Scalar	rgbValue;
#endif // _USE_OPENCV_CPP_

	redValue	=	((color16 & 0x0ffff) >> 8)	& 0x00f8;
	grnValue	=	((color16 & 0x0ffff) >> 3)	& 0x00fC;
	bluValue	=	((color16 & 0x0ffff) << 3) 	& 0x00f8;

//	CONSOLE_DEBUG_W_HEX("color16\t=",	color16);
//	CONSOLE_DEBUG_W_HEX("redValue\t=",	redValue);
//	CONSOLE_DEBUG_W_HEX("grnValue\t=",	grnValue);
//	CONSOLE_DEBUG_W_HEX("bluValue\t=",	bluValue);

	rgbValue	=	CV_RGB(redValue, grnValue, bluValue);
	return(rgbValue);
}

#ifdef _USE_OPENCV_CPP_
	cv::Mat		*gAlpacaLogoPtr	=	NULL;
	cv::Mat		gAlpacaLogo;
#else
	IplImage	*gAlpacaLogoPtr	=	NULL;
#endif
//*****************************************************************************
void	LoadAlpacaLogo(void)
{
#ifdef _USE_OPENCV_CPP_
//	gAlpacaLogoPtr	=	new cv::imread("logos/AlpacaLogo-vsmall.png", CV_LOAD_IMAGE_COLOR);
//	gAlpacaLogoPtr	=	new cv::Mat("logos/AlpacaLogo-vsmall.png");
	gAlpacaLogo		=	cv::imread("logos/AlpacaLogo-vsmall.png");
	gAlpacaLogoPtr	=	&gAlpacaLogo;

//*	debugging
//	cv::namedWindow("Logo");			//Declaring an window to show ROI
//	cv::imshow("Logo", gAlpacaLogo);	//Showing actual image

#else
	if (gAlpacaLogoPtr == NULL)
	{
		gAlpacaLogoPtr	=	cvLoadImage("logos/AlpacaLogo-vsmall.png");
	}
	#endif
}

//*****************************************************************************
//*	this steps through the Controller Object List to see if there is a window by this name
//*****************************************************************************
bool	CheckForOpenWindowByName(const char *windowName)
{
int		iii;
bool	windowExists;

	windowExists	=	false;
	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
			if (strcmp(gControllerList[iii]->cWindowName, windowName) == 0)
			{
				windowExists	=	true;
				break;
			}
		}
	}
	return(windowExists);
}




#ifdef _CONTROLLER_USES_ALPACA_


//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
TYPE_CAPABILITY		cCapabilitiesList[kMaxCapabilities];

//**************************************************************************************
void	Controller::ClearCapabilitiesList(void)
{
int	iii;

	for (iii=0; iii<kMaxCapabilities; iii++)
	{
		memset(&cCapabilitiesList[iii], 0, sizeof(TYPE_CAPABILITY));
	}
}

//**************************************************************************************
void	Controller::AddCapability(const char *capability, const char *value)
{
int		iii;
int		foundIdx;

//	CONSOLE_DEBUG_W_2STR(__FUNCTION__, capability, value);

	foundIdx	=	-1;
	iii			=	0;

	while ((foundIdx < 0) && (iii<kMaxCapabilities))
	{
		if (cCapabilitiesList[iii].capabilityName[0] == 0)
		{
			foundIdx	=	iii;
			strcpy(cCapabilitiesList[iii].capabilityName, capability);
		}
		else if (strcasecmp(capability, cCapabilitiesList[iii].capabilityName) == 0)
		{
			foundIdx	=	iii;
		}
		iii++;
	}
	if ((foundIdx >= 0) && (foundIdx < kMaxCapabilities))
	{
		strcpy(cCapabilitiesList[foundIdx].capabilityValue, value);

		UpdateCapabilityList();
	}
}

//*****************************************************************************
void	Controller::ReadOneDriverCapability(const char	*driverNameStr,
											const char	*propertyStr,
											const char	*reportedStr,
											bool		*booleanValue)
{
bool			validData;
bool			argBoolean;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	CONSOLE_DEBUG_W_2STR(driverNameStr, propertyStr, reportedStr);

	argBoolean	=	false;
	validData	=	AlpacaGetBooleanValue(	driverNameStr, propertyStr,	NULL,	&argBoolean);
	if (validData)
	{
		AddCapability(reportedStr, (argBoolean ? "\tTrue" : "False"));
		*booleanValue	=	argBoolean;
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
	}
}

//**************************************************************************************
//*	this is intended to get over ridden by a sub class
void	Controller::UpdateCapabilityList(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//*****************************************************************************
void	Controller::UpdateSupportedActions(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

#endif // _CONTROLLER_USES_ALPACA_

//*****************************************************************************
//*	the arg is pointer to "this"
//	_USE_BACKGROUND_THREAD_
//*****************************************************************************
static void	*ControllerBackgroundThread(void *arg)
{
Controller	*myControllerPtr;

	CONSOLE_DEBUG("*************************************************");
	CONSOLE_DEBUG(__FUNCTION__);
	myControllerPtr	=	(Controller *)arg;
	if (myControllerPtr != NULL)
	{
		CONSOLE_DEBUG("Valid Controller ptr");
		while(myControllerPtr->cMagicCookie == kMagicCookieValue)
		{
			if (gDebugBackgroundThread)
			{
				CONSOLE_DEBUG_W_STR("Calling RunBackgroundTasks() for", myControllerPtr->cWindowName);
			}
			myControllerPtr->cBackgroundTaskActive	=	true;
			myControllerPtr->RunBackgroundTasks(__FUNCTION__, gDebugBackgroundThread);
			myControllerPtr->cBackgroundTaskActive	=	false;
			usleep(200 * 1000);	//*	200 milliseconds
		}
		CONSOLE_ABORT("Magic cookie is stale")
	}
	else
	{
		CONSOLE_ABORT("pointer to controller object was NULL")
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit thread!!!!!!!!!!!!!!!!!!!");
	return(NULL);
}

//**************************************************************************************
//*	returns thread error
//**************************************************************************************
int	Controller::StartBackgroundThread(void)
{
int			threadErr;

	CONSOLE_DEBUG("***************************************************************");
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("Starting background thread for window:",	cWindowName);
	threadErr			=	pthread_create(&cBackgroundThreadID, NULL, &ControllerBackgroundThread, this);
	if (threadErr == 0)
	{
		cBackGroundThreadCreated	=	true;
	}
	return(threadErr);
}

//**************************************************************************************
//*	Low level drawing routines
//**************************************************************************************
void	Controller::LLD_MoveTo(const int xx, const int yy)
{
	cCurrentXloc	=	xx;
	cCurrentYloc	=	yy;

}

//**************************************************************************************
void	Controller::LLD_LineTo(const int xx, const int yy)
{
//	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _USE_OPENCV_CPP_
	if (cOpenCV_matImage != NULL)
	{
	cv::Point		pt1;
	cv::Point		pt2;
		pt1.x	=	cCurrentXloc;
		pt1.y	=	cCurrentYloc;

		pt2.x	=	xx;
		pt2.y	=	yy;
		cv::line(	*cOpenCV_matImage,
				pt1,
				pt2,
				cCurrentColor,		//	color,
				cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
				8,					//	int line_type CV_DEFAULT(8),
				0);					//	int shift CV_DEFAULT(0));

		cCurrentXloc	=	xx;
		cCurrentYloc	=	yy;
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_matImage is NULL");
	}
#else
	if (cOpenCV_Image != NULL)
	{
	CvPoint		pt1;
	CvPoint		pt2;
		pt1.x	=	cCurrentXloc;
		pt1.y	=	cCurrentYloc;

		pt2.x	=	xx;
		pt2.y	=	yy;
		cvLine(	cOpenCV_Image,
				pt1,
				pt2,
				cCurrentColor,		//	color,
				cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
				8,					//	int line_type CV_DEFAULT(8),
				0);					//	int shift CV_DEFAULT(0));

		cCurrentXloc	=	xx;
		cCurrentYloc	=	yy;
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_

}


//**************************************************************************************
//*	Low Level FrameRect
//**************************************************************************************
void	Controller::LLD_FrameRect(int left, int top, int width, int height, int lineWidth)
{

	if ((width > 0) && (height > 0))
	{
	#ifdef _USE_OPENCV_CPP_
		if (cOpenCV_matImage != NULL)
		{
		cv::Rect	myCVrect;

			myCVrect.x		=	left;
			myCVrect.y		=	top;
			myCVrect.width	=	width;
			myCVrect.height	=	height;

			cv::rectangle(	*cOpenCV_matImage,
							myCVrect,
							cCurrentColor,
							lineWidth);

		}
	#else
		if (cOpenCV_Image != NULL)
		{
		CvRect		myCVrect;
			myCVrect.x		=	left;
			myCVrect.y		=	top;
			myCVrect.width	=	width;
			myCVrect.height	=	height;

			cvRectangleR(	cOpenCV_Image,
							myCVrect,
							cCurrentColor,				//	color,
							lineWidth,					//	int thickness CV_DEFAULT(1),
							8,							//	int line_type CV_DEFAULT(8),
							0);							//	int shift CV_DEFAULT(0));

		}
	#endif // _USE_OPENCV_CPP_
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("width\t=", width);
		CONSOLE_DEBUG_W_NUM("height\t=", height);
//		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
void	Controller::LLD_FrameRect(cv::Rect *theRect)
{
	LLD_FrameRect(theRect->x, theRect->y, theRect->width, theRect->height);
}


//**************************************************************************************
//*	Low Level FrameRect
//**************************************************************************************
void	Controller::LLD_FillRect(int left, int top, int width, int height)
{
#ifdef _USE_OPENCV_CPP_
	if (cOpenCV_matImage != NULL)
	{
	cv::Rect	myCVrect;

		myCVrect.x		=	left;
		myCVrect.y		=	top;
		myCVrect.width	=	width;
		myCVrect.height	=	height;

		cv::rectangle(	*cOpenCV_matImage,
						myCVrect,
						cCurrentColor,
					#if (CV_MAJOR_VERSION >= 3)
						cv::FILLED				//	int thickness CV_DEFAULT(1),
					#else
						CV_FILLED
					#endif
						);

	}
#else
	if (cOpenCV_Image != NULL)
	{
	CvRect		myCVrect;

		myCVrect.x		=	left;
		myCVrect.y		=	top;
		myCVrect.width	=	width;
		myCVrect.height	=	height;

		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						cCurrentColor,				//	color,
					#if (CV_MAJOR_VERSION >= 3)
						cv::FILLED,				//	int thickness CV_DEFAULT(1),
					#else
						CV_FILLED,
					#endif
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));
	}
#endif // _USE_OPENCV_CPP_
}

//**************************************************************************************
//*	Low Level FrameRect
//**************************************************************************************
void	Controller::LLD_DrawCString(	const int	xx,
										const int	yy,
										const char	*textString,
										const int	fontIndex)

{

	if (strlen(textString) > 0)
	{
#ifdef _USE_OPENCV_CPP_
		if (cOpenCV_matImage != NULL)
		{
		cv::Point		textLoc;
			textLoc.x	=	xx;
			textLoc.y	=	yy;

			cv::putText(	*cOpenCV_matImage,
							textString,
							textLoc,
							gFontInfo[fontIndex].fontID,
							gFontInfo[fontIndex].scale,
							cCurrentColor,
							gFontInfo[fontIndex].thickness
							);
		}
#else
//		CONSOLE_DEBUG(textString);
		if (cOpenCV_Image != NULL)
		{
		CvPoint		textLoc;

			textLoc.x	=	xx;
			textLoc.y	=	yy;

			cvPutText(	cOpenCV_Image,
						textString,
						textLoc,
						&gTextFont[fontIndex],
						cCurrentColor
						);
		}
		else
		{
			CONSOLE_ABORT(__FUNCTION__);
		}
#endif // _USE_OPENCV_CPP_
	}
}

//**************************************************************************************
void	Controller::LLD_FrameEllipse(	int xCenter, int yCenter, int xRadius, int yRadius)
{
#ifdef _USE_OPENCV_CPP_
	if (cOpenCV_matImage != NULL)
	{
	cv::Point	center;
	cv::Size	axes;

		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	1 * xRadius;
			axes.height	=	1 * yRadius;

			cv::ellipse(	*cOpenCV_matImage,
							center,
							axes,
							0.0,				//*	angle
							0.0,				//*	start_angle
							360.0,				//*	end_angle
							cCurrentColor,		//	color,
							cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
							8,					//	int line_type CV_DEFAULT(8),
							0);					//	int shift CV_DEFAULT(0));
		}
		else
		{
			CONSOLE_ABORT("Invalid arguments");
			CONSOLE_DEBUG_W_NUM("xCenter\t=", xCenter);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", yCenter);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
#else
	if (cOpenCV_Image != NULL)
	{
	CvPoint	center;
	CvSize	axes;
		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	1 * xRadius;
			axes.height	=	1 * yRadius;

			cvEllipse(	cOpenCV_Image,
						center,
						axes,
						0.0,				//*	angle
						0.0,				//*	start_angle
						360.0,				//*	end_angle
						cCurrentColor,		//	color,
						cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
						8,					//	int line_type CV_DEFAULT(8),
						0);					//	int shift CV_DEFAULT(0));
		}
		else
		{
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_

}

//**************************************************************************************
void	Controller::LLD_FillEllipse(	int xCenter, int yCenter, int xRadius, int yRadius)
{

#ifdef _USE_OPENCV_CPP_
	if (cOpenCV_matImage != NULL)
	{
	cv::Point	center;
	cv::Size	axes;

		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	1 * xRadius;
			axes.height	=	1 * yRadius;

			cv::ellipse(	*cOpenCV_matImage,
							center,
							axes,
							0.0,			//*	angle
							0.0,			//*	start_angle
							360.0,			//*	end_angle
							cCurrentColor,	//	color,
						#if (CV_MAJOR_VERSION >= 3)
							cv::FILLED				//	int thickness CV_DEFAULT(1),
						#else
							CV_FILLED
						#endif
							);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("xCenter\t=", xCenter);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", yCenter);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_matImage is NULL");
	}

#else
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCV_Image != NULL)
	{
	CvPoint	center;
	CvSize	axes;

		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	1 * xRadius;
			axes.height	=	1 * yRadius;

			cvEllipse(	cOpenCV_Image,
						center,
						axes,
						0.0,			//*	angle
						0.0,			//*	start_angle
						360.0,			//*	end_angle
						cCurrentColor,	//	color,
					#if (CV_MAJOR_VERSION >= 3)
						cv::FILLED,				//	int thickness CV_DEFAULT(1),
					#else
						CV_FILLED,
					#endif
						8,				//	int line_type CV_DEFAULT(8),
						0);				//	int shift CV_DEFAULT(0));
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("xCenter\t=", xCenter);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", yCenter);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_

}

//*****************************************************************************
int	Controller::LLD_GetTextSize(const char *textString, const int fontIndex)
{
int		textWidthPixels;

#ifdef _USE_OPENCV_CPP_
cv::Size		textSize;
	textSize	=	cv::getTextSize(textString,
									gFontInfo[fontIndex].fontID,
									gFontInfo[fontIndex].scale,
									gFontInfo[fontIndex].thickness,
									&cCurrentFontBaseLine);
	textWidthPixels		=	textSize.width;
	cCurrentFontHeight	=	textSize.height;
#else

CvSize		textSize;
	cvGetTextSize(	textString,
					&gTextFont[fontIndex],
					&textSize,
					&cCurrentFontBaseLine);
	textWidthPixels		=	textSize.width;
	cCurrentFontHeight	=	textSize.height;
#endif

	return(textWidthPixels);
}


//**************************************************************************************
void	DumpControllerBackGroundTaskStatus(void)
{
int	iii;

	CONSOLE_DEBUG(__FUNCTION__);
	printf("--------------------------------------------------------------\r\n");

	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
			printf("%d\t",		iii);
			printf("%-25s\t",	gControllerList[iii]->cWindowName);
			printf("%20s\t",	gControllerList[iii]->cBackGroundThreadCreated ? "Created" : "Not Created");

			printf("\r\n");
		}
	}
}

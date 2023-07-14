//*****************************************************************************
//*		windowtab_multicam.cpp		(c) 2023 by Mark Sproul
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jun 16,	2023	<MLS> Created windowtab_multicam.cpp
//*	Jun 19,	2023	<MLS> Added DeviceState to multicam
//*****************************************************************************


#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"windowtab_multicam.h"


#include	"controller_multicam.h"

#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabMulticam::WindowTabMulticam(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName,
								const bool	parentIsSkyTravel)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabMulticam::~WindowTabMulticam(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabMulticam::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		myBtnWidth;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kMultiCam_Title, kMultiCam_Connected, yLoc, "MultiCam");

	xLoc	=	3;
	SetWidget(				kMultiCam_CameraList,	xLoc, yLoc,	cWidth - 6,	100);
	SetWidgetType(			kMultiCam_CameraList,	kWidgetType_MultiLineText);
	SetWidgetFont(			kMultiCam_CameraList,	kFont_TextList);
	SetWidgetJustification(	kMultiCam_CameraList,	kJustification_Left);
	yLoc	+=	100;
	yLoc	+=	2;



	//-----------------------------------------------------------
	myBtnWidth	=	(2 * cClmWidth) - 3;
	xLoc		=	cClm1_offset;
	for (iii=kMultiCam_StartExpBtn; iii<=kMultiCam_StartExpBtn; iii++)
	{
		SetWidget(				iii,	xLoc,		yLoc,	myBtnWidth,	cLrgBtnHeight);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetBGColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));
		SetWidgetBorderColor(	iii,	CV_RGB(0,	0,	0));
		SetWidgetFont(			iii,	kFont_Medium);
		xLoc	+=	myBtnWidth;
		xLoc	+=	3;
	}
	SetWidgetBGColor(	kMultiCam_StartExpBtn,		CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kMultiCam_StartExpBtn,		CV_RGB(0,	0,	0));
	SetWidgetText(		kMultiCam_StartExpBtn,		"Start Exposure");
	yLoc	+=	cLrgBtnHeight;
	yLoc	+=	2;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kMultiCam_IPaddr,				//	ipaddrBox
							kMultiCam_Readall,				//	readAllBox
							kMultiCam_DeviceState,
							kMultiCam_AlpacaErrorMsg,		//	errorMsgBox
							kMultiCam_LastCmdString,		//	lastCmdWidgetIdx
							kMultiCam_AlpacaLogo,			//	logoWidgetIdx
							-1,								//	helpBtnBox
							false,							//	logoSideOfScreen, false=left
							-1);							//	connectBtnBox

}


//*****************************************************************************
void	WindowTabMulticam::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool			validData;
//SJP_Parser_t	jsonResponse;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

	//*	clear out any existing error message
	SetWidgetText(	kMultiCam_AlpacaErrorMsg, 	"---");
	validData	=	true;
	switch(buttonIdx)
	{
		case kMultiCam_StartExpBtn:
//			sprintf(dataString, "", (buttonIdx - kCameraBox_ReadMode0));
			//	AlpacaSendPutCmdwResponse
			validData	=	AlpacaSendPutCmd(	"multicam", "startexposure",	NULL);
			if (validData != true)
			{
				CONSOLE_DEBUG("AlpacaSendPutCmd failed");
			}
			break;

	}

	DisplayLastAlpacaCommand();
}

//*****************************************************************************
void	WindowTabMulticam::ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
	CONSOLE_DEBUG(__FUNCTION__);
//	switch(widgetIdx)
//	{
//		case kMultiCam_Title:
//		case kMultiCam_Connected:
//			break;
//
//		default:
//			CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);
//			break;
//	}
}



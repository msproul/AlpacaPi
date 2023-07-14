//*****************************************************************************
//*		windowtab_camvideo.cpp		(c) 2023 by Mark Sproul
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
//*	Jun 19,	2023	<MLS> Created windowtab_camvideo.cpp
//*****************************************************************************


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_camvideo.h"
#include	"controller.h"
#include	"controller_cam_normal.h"


#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabCamVideo::WindowTabCamVideo(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabCamVideo::~WindowTabCamVideo(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabCamVideo::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		boxID;
int		btnWidth;

//	CONSOLE_DEBUG(__FUNCTION__);


	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kCamVideo_Title, kCamVideo_Connected, yLoc, "Video Controls");

	yLoc		+=	10;
	xLoc		=	4;
	btnWidth	=	100;
	for (boxID = kCamVideo_StartVideoBtn; boxID <= kCamVideo_StopVideoBtn; boxID++)
	{
		SetWidget(				boxID,	xLoc,	yLoc,	btnWidth,	cBtnHeight);
		SetWidgetType(			boxID,	kWidgetType_Button);
		SetWidgetFont(			boxID,	kFont_Medium);
		SetWidgetBGColor(		boxID,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		boxID,	CV_RGB(0,	0,	0));
		SetWidgetBorderColor(	boxID,	CV_RGB(0,	0,	0));
		xLoc	+=	btnWidth;
		xLoc	+=	3;
	}
	SetWidgetText(kCamVideo_StartVideoBtn,	"Start Video");
	SetWidgetText(kCamVideo_StopVideoBtn,	"Stop Video");

	SetWidgetType(		kCamVideo_RecordingStatus,	kWidgetType_TextBox);
	SetWidgetText(		kCamVideo_RecordingStatus,	"idle");
	SetWidgetTextColor(	kCamVideo_RecordingStatus,	CV_RGB(255,	0,	0));
	SetWidgetBGColor(	kCamVideo_RecordingStatus,	CV_RGB(0,	0,	0));


	yLoc		+=	cBtnHeight;
	yLoc		+=	2;

	xLoc		=	4;
	btnWidth	=	200;
	for (boxID = kCamVideo_IncludeTimeStamp; boxID <= kCamVideo_LastChkBox; boxID++)
	{
		SetWidget(				boxID,	xLoc,	yLoc,	btnWidth,	cSmallBtnHt);
		SetWidgetType(			boxID,	kWidgetType_CheckBox);
		SetWidgetFont(			boxID,	kFont_Medium);
		SetWidgetTextColor(		boxID,	CV_RGB(255,	255,	255));
		yLoc	+=	cSmallBtnHt;
		yLoc	+=	2;
	}
	SetWidgetText(kCamVideo_IncludeTimeStamp,	"Include Timestamp");
	SetWidgetText(kCamVideo_LastChkBox,			"Last one");


	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kCamVideo_IPaddr,
							kCamVideo_Readall,
							kCamVideo_DeviceSelect,
							kCamVideo_AlpacaErrorMsg,
							kCamVideo_LastCmdString,
							kCamVideo_AlpacaLogo,
							-1);
	SetWidgetText(kCamVideo_LastCmdString,			"Last cmd");
//	DumpWidgetList(kCamVideo_Title, kCamVideo_last-1, __FILE__);
}

//**************************************************************************************
void	WindowTabCamVideo::ActivateWindow(void)
{
	DumpWidgetList(kCamVideo_Title, kCamVideo_last-1, __FILE__);
}

//*****************************************************************************
void	WindowTabCamVideo::ProcessButtonClick(const int buttonIdx, const int flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

	switch(buttonIdx)
	{
		case kCamVideo_StartVideoBtn:
			StartVideo();
			break;

		case kCamVideo_StopVideoBtn:
			StopVideo();
			break;

		case kCamVideo_IncludeTimeStamp:
			break;

	}
	DisplayLastAlpacaCommand();
	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabCamVideo::StartVideo(void)
{
bool				validData;
SJP_Parser_t		jsonParser;
ControllerCamNormal	*parrentCamController;

	CONSOLE_DEBUG(__FUNCTION__);

	parrentCamController	=	(ControllerCamNormal*)cParentObjPtr;
	if (parrentCamController != NULL)
	{
		validData	=	parrentCamController->AlpacaSendPutCmdwResponse(	"camera", "startvideo",	NULL, &jsonParser);
		if (validData)
		{
			SetWidgetText(	kCamVideo_RecordingStatus,	"Recording");
		}
		else
		{
			SetWidgetText(	kCamVideo_RecordingStatus,	"failed");
		}
		SJP_DumpJsonData(&jsonParser, __FUNCTION__);
	}

}

//*****************************************************************************
void	WindowTabCamVideo::StopVideo(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetText(		kCamVideo_RecordingStatus,	"Stopped");
}


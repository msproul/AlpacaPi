//*****************************************************************************
//*		windowtab_dome.cpp		(c) 2020 by Mark Sproul
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
//*	Apr 18,	2020	<MLS> Created windowtab_dome.cpp
//*	May  1,	2020	<MLS> Added shutter and slaved to dome control display
//*	May  1,	2020	<MLS> Added OpenShutter() & CloseShutter()
//*	May  4,	2020	<MLS> Slave mode set/unset logic working now
//*	May  7,	2020	<MLS> Added SendShutterCommand() to repleace multiple other cmds
//*****************************************************************************

#ifdef _ENABLE_CTRL_DOME_

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"windowtab_dome.h"

#ifdef _ENABLE_SKYTRAVEL_
	#include	"controller_skytravel.h"
#else
	#include	"controller_dome.h"
#endif

#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabDome::WindowTabDome(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabDome::~WindowTabDome(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	WindowTabDome::SetupWindowControls(void)
{
int		yLoc;
int		btnWidth;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	//============================================
	yLoc			=	cTabVertOffset;

	//============================================
	SetWidget(kDomeBox_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kDomeBox_Title, "AlpacaPi Dome");
	SetBGcolorFromWindowName(kDomeBox_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;


	btnWidth		=	(cClmWidth * 3);

	//============================================
	SetWidget(			kDomeBox_CurPosLabel,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetTextColor(	kDomeBox_CurPosLabel,	CV_RGB(255,	255,	255));
	SetWidgetText(		kDomeBox_CurPosLabel,	"Position");
	SetWidgetBorder(	kDomeBox_CurPosLabel,	false);

	SetWidget(			kDomeBox_CurPosition,	cClm4_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetTextColor(	kDomeBox_CurPosition,	CV_RGB(0,	255,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;


	//============================================
	SetWidget(			kDomeBox_AzimLabel,		cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetTextColor(	kDomeBox_AzimLabel,		CV_RGB(255,	255,	255));
	SetWidgetText(		kDomeBox_AzimLabel,		"Azimuth");
	SetWidgetBorder(	kDomeBox_AzimLabel,		false);

	SetWidget(			kDomeBox_Azimuth,		cClm4_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_Azimuth,		"Azimuth");
	SetWidgetTextColor(	kDomeBox_Azimuth,		CV_RGB(0,	255,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//============================================
	SetWidget(			kDomeBox_AltitudeLabel,		cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetTextColor(	kDomeBox_AltitudeLabel,		CV_RGB(255,	255,	255));
	SetWidgetText(		kDomeBox_AltitudeLabel,		"Altitude");
	SetWidgetBorder(	kDomeBox_AltitudeLabel,		false);

	SetWidget(			kDomeBox_Altitude,			cClm4_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_Altitude,			"Altitude");
	SetWidgetTextColor(	kDomeBox_Altitude,			CV_RGB(0,	255,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;


	//============================================
	SetWidget(			kDomeBox_ShutterLabel,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_ShutterLabel,	"Shutter");
	SetWidgetTextColor(	kDomeBox_ShutterLabel,	CV_RGB(255,	255,	255));
	SetWidgetBorder(	kDomeBox_ShutterLabel,	false);

	SetWidget(			kDomeBox_ShutterStatus,	cClm4_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetTextColor(	kDomeBox_ShutterStatus,	CV_RGB(0,	255,	0));
	SetWidgetText(		kDomeBox_ShutterStatus,	"---");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//============================================
	SetWidget(			kDomeBox_SlavedLabel,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_SlavedLabel,	"Slaved");
	SetWidgetTextColor(	kDomeBox_SlavedLabel,	CV_RGB(255,	255,	255));
	SetWidgetBorder(	kDomeBox_SlavedLabel,	false);

	SetWidget(			kDomeBox_SlavedStatus,	cClm4_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetTextColor(	kDomeBox_SlavedStatus,	CV_RGB(0,	255,	0));
	SetWidgetText(		kDomeBox_SlavedStatus,	"---");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//============================================
	btnWidth		=	(cClmWidth * 3);
	SetWidget(		kDomeBox_GoHome,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(	kDomeBox_GoHome, 	"Go Home");

	SetWidget(		kDomeBox_GoPark,	cClm4_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(	kDomeBox_GoPark, 	"Go Park");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//**************************************************************
	btnWidth		=	cClmWidth * 2;
	SetWidget(			kDomeBox_GoLeft,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_GoLeft,	"Go Left");

	SetWidget(			kDomeBox_GoRight,	cClm5_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_GoRight,	"Go Right");

	SetWidget(			kDomeBox_Stop,	cClm3_offset+6,		yLoc,		btnWidth-8,		3 * cBtnHeight);
	SetWidgetText(		kDomeBox_Stop,	"STOP");
	SetWidgetBGColor(	kDomeBox_Stop,	CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kDomeBox_Stop,	CV_RGB(255,	255,	255));
	SetWidgetFont(		kDomeBox_Stop,	kFont_Large);
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;


	//**************************************************************
	SetWidget(			kDomeBox_SlowLeft,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_SlowLeft,	"Slow Left");

	SetWidget(			kDomeBox_SlowRight,	cClm5_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_SlowRight, "Slow Right");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//**************************************************************
	SetWidget(			kDomeBox_BumpLeft,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_BumpLeft, "Bump Left");

	SetWidget(			kDomeBox_BumpRight,	cClm5_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_BumpRight, "Bump Right");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//============================================
	btnWidth		=	(cClmWidth * 6);
	SetWidget(			kDomeBox_ToggleSlaveMode,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_ToggleSlaveMode, 	"Enable Slave mode");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	for (iii=kDomeBox_GoHome; iii<=kDomeBox_ToggleSlaveMode; iii++)
	{
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_Medium);
//		SetWidgetBGColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));
		SetWidgetBorderColor(	iii,	CV_RGB(0,	0,	0));

		SetWidgetValid(			iii,	false);
	}

	SetWidgetBGColor(	kDomeBox_Stop,		CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kDomeBox_Stop,		CV_RGB(255,	255,	255));
	SetWidgetFont(		kDomeBox_Stop,		kFont_Large);


	//============================================
	yLoc			+=	5;
	btnWidth		=	(cClmWidth * 3);
	SetWidget(			kDomeBox_OpenShutter,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetText(		kDomeBox_OpenShutter, 	"Open Shutter");
	SetWidgetType(		kDomeBox_OpenShutter,	kWidgetType_Button);
	SetWidgetBGColor(	kDomeBox_OpenShutter,	CV_RGB(0,	128,	240));
	SetWidgetTextColor(	kDomeBox_OpenShutter,	CV_RGB(0,	0,	0));

	SetWidget(			kDomeBox_CloseShutter,	cClm4_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetType(		kDomeBox_CloseShutter,	kWidgetType_Button);
	SetWidgetText(		kDomeBox_CloseShutter, 	"Close Shutter");
	SetWidgetBGColor(	kDomeBox_CloseShutter,	CV_RGB(0,	128,	240));
	SetWidgetTextColor(	kDomeBox_CloseShutter,	CV_RGB(0,	0,	0));

	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//============================================
	btnWidth		=	(cClmWidth * 6);
	SetWidget(			kDomeBox_StopShutter,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetType(		kDomeBox_StopShutter,	kWidgetType_Button);
	SetWidgetText(		kDomeBox_StopShutter, 	"STOP Shutter");
	SetWidgetBGColor(	kDomeBox_StopShutter,	CV_RGB(255,	0,	0));
	SetWidgetTextColor(	kDomeBox_StopShutter,	CV_RGB(255,	255,	255));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;


	//============================================
	btnWidth		=	(cClmWidth * 6);
	SetWidget(			kDomeBox_ErrorMsg,	cClm1_offset,		yLoc,		btnWidth,		cBtnHeight);
	SetWidgetFont(		kDomeBox_ErrorMsg,	kFont_Medium);
	SetWidgetTextColor(	kDomeBox_ErrorMsg,	CV_RGB(255,	0,	0));
	SetWidgetText(		kDomeBox_ErrorMsg, 	"---");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;


	SetAlpacaLogo(kDomeBox_AlpacaLogo, kDomeBox_LastCmdString);

	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kDomeBox_IPaddr, kDomeBox_Readall, kDomeBox_AlpacaDrvrVersion, -1);


#ifdef _ENABLE_SKYTRAVEL_
int	myBtnHeight;
int	xLoc;

	btnWidth		=	100;
	myBtnHeight		=	cBtnHeight - 4;
	xLoc			=	cWidth - (btnWidth + 5);
	yLoc			=	cHeight- (myBtnHeight + 5);
	SetWidget(			kDomeBox_Rescan,	xLoc,		yLoc,		btnWidth,		myBtnHeight);
	SetWidgetFont(		kDomeBox_Rescan,	kFont_Medium);
	SetWidgetText(		kDomeBox_Rescan, 	"Re-scan");
	SetWidgetBGColor(	kDomeBox_Rescan,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kDomeBox_Rescan,	CV_RGB(0,	0,	0));

#endif


}

//******************************************************************************
void	WindowTabDome::DrawGraphWidget(IplImage *openCV_Image, const int widgitIdx)
{
CvRect		myCVrect;

	myCVrect.x		=	cWidgetList[widgitIdx].left;
	myCVrect.y		=	cWidgetList[widgitIdx].top;
	myCVrect.width	=	cWidgetList[widgitIdx].width;
	myCVrect.height	=	cWidgetList[widgitIdx].height;


	cvRectangleR(	openCV_Image,
					myCVrect,
					cWidgetList[widgitIdx].bgColor,			//	CvScalar color,
					CV_FILLED,								//	int thickness CV_DEFAULT(1),
					8,										//	int line_type CV_DEFAULT(8),
					0);										//	int shift CV_DEFAULT(0));

//	cvRectangleR(	openCV_Image,
//					myCVrect,
//					cWidgetList[widgitIdx].boarderColor,	//	CvScalar color,
//					1,										//	int thickness CV_DEFAULT(1),
//					8,										//	int line_type CV_DEFAULT(8),
//					0);										//	int shift CV_DEFAULT(0));

	switch(widgitIdx)
	{

		default:
			CONSOLE_DEBUG_W_NUM("widgitIdx\t",	widgitIdx);
			break;
	}
}

//*****************************************************************************
bool	WindowTabDome::SendAlpacaCmdToDome(	const char		*theCommand,
											const char		*dataString,
											SJP_Parser_t	*jsonParser)
{
bool	validData	=	false;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, theCommand);

#ifdef _ENABLE_SKYTRAVEL_
ControllerSkytravel	*myControllerObj;

	myControllerObj	=	(ControllerSkytravel *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		validData	=	AlpacaSendPutCmd(	&myControllerObj->cDomeIpAddress,
											myControllerObj->cDomeIpPort,
											"dome",
											myControllerObj->cDomeAlpacaDeviceNum,
											theCommand,
											dataString,
											jsonParser);
	}
	else
	{
		CONSOLE_DEBUG("myControllerObj is NULL");
	}
#else
	validData	=	AlpacaSendPutCmd(	"dome",	theCommand,	"", jsonParser);

#endif
	return(validData);
}

//*****************************************************************************
void	WindowTabDome::ProcessButtonClick(const int buttonIdx)
{
bool			validData;
SJP_Parser_t	jsonResponse;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

	//*	clear out any existing error message
	SetWidgetText(	kDomeBox_ErrorMsg, 	"---");
	validData	=	true;
	switch(buttonIdx)
	{
		case kDomeBox_GoHome:
			validData	=	SendAlpacaCmdToDome("findhome",		"",	&jsonResponse);
			break;

		case kDomeBox_GoPark:
			validData	=	SendAlpacaCmdToDome("park",			"",	&jsonResponse);
			break;

		case kDomeBox_Stop:
			validData	=	SendAlpacaCmdToDome("abortslew",	"",	&jsonResponse);
			break;

		case kDomeBox_BumpLeft:
			validData	=	SendAlpacaCmdToDome("bumpleft",		"",	&jsonResponse);
			break;

		case kDomeBox_BumpRight:
			validData	=	SendAlpacaCmdToDome("bumpright",	"",	&jsonResponse);
			break;

		case kDomeBox_GoLeft:
			validData	=	SendAlpacaCmdToDome("goleft",		"",	&jsonResponse);
			break;

		case kDomeBox_GoRight:
			validData	=	SendAlpacaCmdToDome("goright",		"",	&jsonResponse);
			break;

		case kDomeBox_SlowLeft:
			validData	=	SendAlpacaCmdToDome("slowleft",		"",	&jsonResponse);
			break;

		case kDomeBox_SlowRight:
			validData	=	SendAlpacaCmdToDome("slowright",	"",	&jsonResponse);
			break;

		case kDomeBox_ToggleSlaveMode:
			ToggleSlaveMode();
			break;

		//*	the shutter commands use a different routine to support the external shutter device
		//*	this routine takes care of external/internal shutter control
		//*	if external shutter is not enabled, it calls the same routine as above
		case kDomeBox_OpenShutter:
			SendShutterCommand("openshutter");
			break;

		case kDomeBox_CloseShutter:
			SendShutterCommand("closeshutter");
			break;

		case kDomeBox_StopShutter:
			SendShutterCommand("abortslew");
			break;

	#ifdef _ENABLE_SKYTRAVEL_
		case kDomeBox_Rescan:
			break;
	#endif // _ENABLE_SKYTRAVEL_
	}
	if (validData == false)
	{
		CONSOLE_DEBUG("AlpacaSendPutCmd failed");
	}

	DisplayLastAlpacaCommand();
}


//*****************************************************************************
void	WindowTabDome::SendShutterCommand(const char *shutterCmd)
{
	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _ENABLE_EXTERNAL_SHUTTER_

	#ifndef _ENABLE_SKYTRAVEL_
	ControllerDome	*myDomeController;

		CONSOLE_DEBUG(__FUNCTION__);
		myDomeController	=	(ControllerDome *)cParentObjPtr;

		if (myDomeController != NULL)
		{
			myDomeController->SendShutterCommand(shutterCmd);
		}
		else
		{
			CONSOLE_DEBUG("myDomeController is NULL");
		}
	#endif
#else
bool			validData;
SJP_Parser_t	jsonResponse;

	CONSOLE_DEBUG(__FUNCTION__);
	validData	=	SendAlpacaCmdToDome(shutterCmd,	"",	&jsonResponse);

#endif

}


//*****************************************************************************
void	WindowTabDome::ToggleSlaveMode(void)
{
#ifndef _ENABLE_SKYTRAVEL_
bool			validData;
//int				myShutterStatus;
int				mySlavedMode;
SJP_Parser_t	jsonParser;
int				alpacaErrorCode;
char			alpacaErrorMsg[128];
char			textString[256];
ControllerDome	*myDomeController;

	CONSOLE_DEBUG(__FUNCTION__);
	myDomeController	=	(ControllerDome *)cParentObjPtr;

	if (myDomeController != NULL)
	{
		//*	we need to find out the current slaved mode
		mySlavedMode	=	myDomeController->cSlaved;
//		myShutterStatus	=	myDomeController->cShutterStatus;
		if (mySlavedMode)
		{
			CONSOLE_DEBUG("Slave mode is ON, turning off");
			//*	send the command to turn off slave mode
			validData	=	SendAlpacaCmdToDome("slaved",	"Slaved=false");
		}
		else
		{
			CONSOLE_DEBUG("Slave mode is OFF, turning ON");
			validData		=	SendAlpacaCmdToDome("slaved",	"Slaved=true", &jsonParser);
			alpacaErrorCode	=	myDomeController->AlpacaCheckForErrors(&jsonParser, alpacaErrorMsg, false);
			if (alpacaErrorCode != kASCOM_Err_Success)
			{
				CONSOLE_DEBUG_W_STR("alpacaErrorMsg\t=", alpacaErrorMsg);
				sprintf(textString, "Err# %d - %s", alpacaErrorCode, alpacaErrorMsg);
				SetWidgetText(	kDomeBox_ErrorMsg, 	textString);

			}

	//		if (myShutterStatus == kShutterStatus_Open)
	//		{
	//			validData	=	SendAlpacaCmdToDome("slaved",	"Slaved=true");
	//		}
	//		else
	//		{
	//			SetWidgetText(		kDomeBox_ErrorMsg, 	"Can't enable slave mode unless shutter is open");
	//		}
		}
		if (validData == false)
		{
			CONSOLE_DEBUG("AlpacaSendPutCmd failed");
		}
	}
	else
	{
		CONSOLE_DEBUG("myDomeController is NULL");
	}
#endif
}

//*****************************************************************************
void	WindowTabDome::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
//	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kDomeBox_ErrorMsg, errorMsgString);
}


#endif // _ENABLE_CTRL_DOME_


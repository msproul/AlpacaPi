//*****************************************************************************
//*		windowtab_camcooler.cpp		(c) 2023 by Mark Sproul
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Apr 28,	2023	<MLS> Created windowtab_camcooler.cpp
//*	Jun 18,	2023	<MLS> Added DeviceState to cam cooler
//*****************************************************************************

#ifdef _ENABLE_CTRL_CAMERA_

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"
#include	"windowtab.h"
#include	"windowtab_camcooler.h"
#include	"controller.h"
#include	"controller_camera.h"
#include	"alpacadriver_helper.h"


//**************************************************************************************
WindowTabCamCooler::WindowTabCamCooler(	const int	xSize,
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
WindowTabCamCooler::~WindowTabCamCooler(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabCamCooler::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		rowNumber;
int		iii;
int		cooler_xLoc;
int		graphHieght;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kCamCooler_Title, -1, yLoc, "AlpacaPi project");

	//=======================================================
	//*	Cooler check box
	cooler_xLoc	=	cClm1_offset + 2;
	SetWidget(		kCamCooler_CoolerChkBox,	cooler_xLoc,	yLoc,	cClmWidth,	cBtnHeight	);
	SetWidgetType(	kCamCooler_CoolerChkBox,	kWidgetType_CheckBox);
	SetWidgetText(	kCamCooler_CoolerChkBox,	"Camera Cooler");
	SetWidgetFont(	kCamCooler_CoolerChkBox,	kFont_Medium);
	yLoc		+=	cBtnHeight;
	yLoc	+=	2;

	//*	Temperature
	rowNumber	=	0;
	iii			=	kCamCooler_Temp_Label;
	while (iii < kCamCooler_TargetTemp)
	{
		rowNumber++;
		xLoc	=	cooler_xLoc;
		SetWidget(		iii,	xLoc,	yLoc,	cClmWidth,		cBtnHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		xLoc	+=	cClmWidth;
		xLoc	+=	2;
		iii++;

		SetWidget(		iii,	xLoc,	yLoc,	cClmWidth * 2,	cBtnHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		xLoc	+=	cClmWidth * 2;
		xLoc	+=	2;
		iii++;

		SetWidget(		iii,	xLoc,	yLoc,	cClmWidth * 2,	cBtnHeight);
		SetWidgetFont(	iii,	kFont_Medium);
		xLoc	+=	cClmWidth * 2;
		xLoc	+=	2;
		iii++;

		if (rowNumber == 2)
		{
			SetWidget(			iii,	xLoc,	yLoc,	cClmWidth - 5,	cBtnHeight);
			SetWidgetFont(		iii,	kFont_Medium);
			SetWidgetType(		iii,	kWidgetType_Button);
			SetWidgetBGColor(	iii,	CV_RGB(255,	255,	255));
			SetWidgetTextColor(	iii,	CV_RGB(0,	0,	0));
			SetWidgetText(		iii,	"Set");
			xLoc	+=	cClmWidth;
			xLoc	+=	2;
			iii++;
		}

		yLoc	+=	cBtnHeight;
		yLoc	+=	2;
	}
	SetWidgetValid(kCamCooler_Temperature_unused, false);
	SetWidgetText(	kCamCooler_Temp_Label,		"Temp");
	SetWidgetText(	kCamCooler_TargetTempLBL,	"Target");
	SetWidgetType(			kCamCooler_TargetTempInput,	kWidgetType_TextInput);
	SetWidgetBGColor(		kCamCooler_TargetTempInput,	CV_RGB(128,	128,	128));
	SetWidgetJustification(	kCamCooler_TargetTempInput,	kJustification_Center);

	//----------------------------------------------------------
	graphHieght	=	120;
	//*	create the graph
	SetWidget(		kCamCooler_TempGraph,	cClm1_offset+ 3,	yLoc,	cWidth - 12,	graphHieght);
	SetWidgetType(	kCamCooler_TempGraph,	kWidgetType_Graph);
	yLoc	+=	graphHieght;
	yLoc	+=	2;

	SetWidget(		kCamCooler_PowerGraph,	cClm1_offset+ 3,	yLoc,	cWidth - 12,	graphHieght);
	SetWidgetType(	kCamCooler_PowerGraph,	kWidgetType_Graph);
	yLoc	+=	graphHieght;
	yLoc	+=	2;

	SetWidgetOutlineBox(kCamCooler_TempOutline, kCamCooler_CoolerChkBox, (kCamCooler_TempOutline -1));

	yLoc			+=	100;
	yLoc			+=	4;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kCamCooler_IPaddr,
							kCamCooler_Readall,
							kCamCooler_DeviceSelect,
							kCamCooler_AlpacaErrorMsg,
							kCamCooler_LastCmdString,
							kCamCooler_AlpacaLogo,
							-1);
}

//**************************************************************************************
void	WindowTabCamCooler::ActivateWindow(void)
{
	DumpWidgetList(kCamCooler_Title, kCamCooler_last-1, __FUNCTION__);
}

//**************************************************************************************
void	WindowTabCamCooler::SetTemperatureGraphPtrs(double *arrayPtr, int arrayCnt)
{
	cWidgetList[kCamCooler_TempGraph].graphArrayPtr	=	arrayPtr;
	cWidgetList[kCamCooler_TempGraph].graphArrayCnt	=	arrayCnt;

	//*	this is redundant for now, to make the graph code happy
	cWidgetList[kCamCooler_PowerGraph].graphArrayPtr	=	arrayPtr;
	cWidgetList[kCamCooler_PowerGraph].graphArrayCnt	=	arrayCnt;

}

//**************************************************************************************
void	WindowTabCamCooler::SetTempartueDisplayEnable(bool enabled)
{

	SetWidgetValid(kCamCooler_Temp_Label,	enabled);
	SetWidgetValid(kCamCooler_Temperature,	enabled);
	SetWidgetValid(kCamCooler_CoolerChkBox,	enabled);
#ifdef _TEMP_GRAPH_
	SetWidgetValid(kCamCooler_TempGraph,	enabled);
	SetWidgetValid(kCamCooler_TempOutline,	enabled);
#endif // _TEMP_GRAPH_
}

//**************************************************************************************
void	WindowTabCamCooler::SetCCDTargetTemperature(void)
{
char				myWidgetText[kMaxWidgetStrLen];
bool				tempValueIsValid;
double				newTargetTemp;
ControllerCamera	*myCameraController;
TYPE_ASCOM_STATUS	alpacaErrorCode;

	SetWidgetText(kCamCooler_AlpacaErrorMsg, "Not finished yet");
	myWidgetText[0]		=	0;
	tempValueIsValid	=	false;
	newTargetTemp		=	0.0;
	GetWidgetText(kCamCooler_TargetTempInput, myWidgetText);

	//*	check for valid values
	if (strlen(myWidgetText) > 0)
	{
		StripLeadingSpaces(myWidgetText);
		if (isdigit(myWidgetText[0]) || (myWidgetText[0] == '-'))
		{
			newTargetTemp		=	atof(myWidgetText);
			tempValueIsValid	=	true;
		}
	}

	//-----------------------------------------
	if (tempValueIsValid)
	{
		SetWidgetNumber(kCamCooler_AlpacaErrorMsg, newTargetTemp);

	//	CONSOLE_DEBUG(__FUNCTION__);
		myCameraController	=	(ControllerCamera *)cParentObjPtr;

		if (myCameraController != NULL)
		{
			alpacaErrorCode	=	myCameraController->SetCCDtargetTemperature(newTargetTemp);
			if (alpacaErrorCode != kASCOM_Err_Success)
			{
				SetWidgetText(kCamCooler_AlpacaErrorMsg, myCameraController->cLastAlpacaErrStr);
			}
		}
		else
		{
			CONSOLE_DEBUG("myCameraController is NULL");
		}
	}
	else
	{
		SetWidgetText(kCamCooler_AlpacaErrorMsg, "Please enter a valid value");
	}
}


//*****************************************************************************
void	WindowTabCamCooler::ProcessButtonClick(const int buttonIdx, const int flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetText(kCamCooler_AlpacaErrorMsg, "");
	switch(buttonIdx)
	{
		case kCamCooler_CoolerChkBox:
			ToggleCooler();
			ForceAlpacaUpdate();
			break;

		case kCamCooler_TargetTempSetBtn:
			SetCCDTargetTemperature();
			ForceWindowUpdate();
			break;

		default:
			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);
			break;
	}
	DisplayLastAlpacaCommand();
}

//*****************************************************************************
void	WindowTabCamCooler::ProcessDoubleClick(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);

	switch(widgetIdx)
	{
		default:
			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

//*****************************************************************************
void	WindowTabCamCooler::ToggleCooler(void)
{
ControllerCamera	*myCameraController;
TYPE_ASCOM_STATUS	alpacaErrorCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		alpacaErrorCode	=	myCameraController->ToggleCooler();
		if (alpacaErrorCode != kASCOM_Err_Success)
		{
			SetWidgetText(kCamCooler_AlpacaErrorMsg, myCameraController->cLastAlpacaErrStr);
		}
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

#endif // _ENABLE_CTRL_CAMERA_

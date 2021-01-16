//*****************************************************************************
//*		windowtab_alpacalist.cpp		(c) 2021 by Mark Sproul
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
//*	Jan 13,	2021	<MLS> Created windowtab_alpacalist.cpp
//*****************************************************************************


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"discoverythread.h"

#include	"windowtab_alpacalist.h"

#include	"controller.h"
#include	"controller_cam_normal.h"
#include	"controller_switch.h"



//**************************************************************************************
WindowTabAlpacaList::WindowTabAlpacaList(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cAlpacaDevCnt		=	0;
	cPrevAlpacaDevCnt	=	0;

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabAlpacaList::~WindowTabAlpacaList(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabAlpacaList::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		textBoxHt;
int		textBoxWd;
int		iii;
short	tabArray[kMaxTabStops]	=	{200, 400, 600, 800, 0};

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kAlpacaList_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kAlpacaList_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kAlpacaList_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//=======================================================
	xLoc		=	10;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	for (iii=kAlpacaList_AlpacaDev_01; iii<=kAlpacaList_AlpacaDev_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
	//	SetWidgetType(			iii,	kWidgetType_CheckBox);
	//	SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetJustification(	iii,	kJustification_Left);
	//	SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetFont(			iii,	kFont_TextList);
	//	SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBorder(		iii,	false);
		SetWidgetTabStops(		iii,	tabArray);

		yLoc			+=	textBoxHt;
		yLoc			+=	4;
	}


	SetWidget(				kAlpacaList_AlpacaDev_Total,	0,			yLoc,		cWidth / 2,		cTitleHeight);
	SetWidgetFont(			kAlpacaList_AlpacaDev_Total,	kFont_Medium);
	SetWidgetText(			kAlpacaList_AlpacaDev_Total,	"Total units =?");
	SetWidgetJustification(	kAlpacaList_AlpacaDev_Total,	kJustification_Left);
	SetWidgetTextColor(		kAlpacaList_AlpacaDev_Total,	CV_RGB(255,	255,	255));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

//	SetAlpacaLogo(kAlpacaList_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
//	SetIPaddressBoxes(kAlpacaList_IPaddr, kAlpacaList_Readall, kAlpacaList_AlpacaDrvrVersion, -1);
//	SetIPaddressBoxes(kAlpacaList_IPaddr, kAlpacaList_Readall, -1, -1);
}

//*****************************************************************************
void	WindowTabAlpacaList::ProcessDoubleClick(const int buttonIdx)
{
int		tableIdx;
char	windowName[64];
bool	windowExists;
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);

	if ((buttonIdx >= kAlpacaList_AlpacaDev_01) && (buttonIdx <= kAlpacaList_AlpacaDev_Last))
	{
		tableIdx	=	buttonIdx - kAlpacaList_AlpacaDev_01;
		if (tableIdx >= 0)
		{
			switch(gRemoteList[tableIdx].deviceTypeEnum)
			{

				case kDeviceType_Camera:
					strcpy(windowName, "Camera-");
					strcat(windowName, gRemoteList[tableIdx].hostName);
					windowExists	=	CheckForOpenWindowByName(windowName);
					if (windowExists)
					{
						CONSOLE_DEBUG_W_STR("Window already open:", windowName);
					}
					else
					{
						new ControllerCamNormal(windowName, &gRemoteList[tableIdx]);
					}
					break;

				case kDeviceType_CoverCalibrator:
				case kDeviceType_Dome:
				case kDeviceType_Filterwheel:
				case kDeviceType_Focuser:
				case kDeviceType_Management:
				case kDeviceType_Observingconditions:
				case kDeviceType_Rotator:
				case kDeviceType_Telescope:
				case kDeviceType_SafetyMonitor:
					break;

				case kDeviceType_Switch:
					strcpy(windowName, "Switch-");
					strcat(windowName, gRemoteList[tableIdx].hostName);
					windowExists	=	CheckForOpenWindowByName(windowName);
					if (windowExists)
					{
						CONSOLE_DEBUG_W_STR("Window already open:", windowName);
					}
					else
					{
						new ControllerSwitch(	windowName,
												&gRemoteList[tableIdx].deviceAddress,
												gRemoteList[tableIdx].port,
												gRemoteList[tableIdx].alpacaDeviceNum);
					}
					break;

				//*	extras defined by MLS
				case kDeviceType_Multicam:
				case kDeviceType_Shutter:
				case kDeviceType_SlitTracker:
					break;

			}
		}
	}
}


//**************************************************************************************
void	WindowTabAlpacaList::UpdateList(void)
{
int		iii;
char	ipAddrSt[32];
char	textString[128];
int		boxId;
int		myDevCount;

//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);

	myDevCount	=	0;
	for (iii=0; iii<gRemoteCnt; iii++)
	{
		inet_ntop(AF_INET, &(gRemoteList[iii].deviceAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);


		sprintf(textString, "%s:%d\t%s\t%s\t%s",	ipAddrSt,
													gRemoteList[iii].port,
													gRemoteList[iii].hostName,
													gRemoteList[iii].deviceTypeStr,
													gRemoteList[iii].deviceNameStr);


		//					gRemoteList[iii].alpacaDeviceNum,
		//					gRemoteList[iii].notSeenCounter);

		boxId	=	kAlpacaList_AlpacaDev_01 + iii;
		if (boxId <= kAlpacaList_AlpacaDev_Last)
		{
			SetWidgetText(boxId, textString);

			if (gRemoteList[iii].deviceTypeEnum == kDeviceType_Focuser)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(0,	255,	0));
			}
			if (gRemoteList[iii].deviceTypeEnum == kDeviceType_Camera)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(255,	255,	0));
			}
			if (gRemoteList[iii].deviceTypeEnum == kDeviceType_Switch)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(0,	255,	255));
			}

			myDevCount++;
		}
	}
	sprintf(textString, "Total Alpaca Devices found=%d", gRemoteCnt);
	SetWidgetText(kAlpacaList_AlpacaDev_Total, textString);


	cAlpacaDevCnt		=	myDevCount;
	if (cAlpacaDevCnt != cPrevAlpacaDevCnt)
	{
		ForceUpdate();
		cPrevAlpacaDevCnt	=	cAlpacaDevCnt;
	}
}







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
//*	Jan 21,	2021	<MLS> Added sortable columns
//*****************************************************************************

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"discoverythread.h"

#include	"windowtab_alpacalist.h"

#include	"controller.h"
#include	"controller_cam_normal.h"
#include	"controller_dome.h"
#include	"controller_focus.h"
#include	"controller_switch.h"
#include	"controller_telescope.h"



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
	cIncludeManagment	=	false;
	cSortColumn			=	-1;

	ClearRemoteDeviceList();

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
int		widgetWidth;
int		iii;
short	tabArray[kMaxTabStops]	=	{200, 400, 600, 1000, 1199, 0};
int		clmnHdr_xLoc;
int		clmnHdrWidth;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kAlpacaList_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kAlpacaList_Title, "AlpacaPi Device List (double click on colored entries)");
	SetBGcolorFromWindowName(kAlpacaList_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	clmnHdr_xLoc		=	1;
	iii	=	kAlpacaList_ClmTitle1;
	while(iii <= kAlpacaList_ClmTitle5)
	{
		clmnHdrWidth	=	tabArray[iii - kAlpacaList_ClmTitle1] - clmnHdr_xLoc;

//		CONSOLE_DEBUG_W_NUM("clmnHdr_xLoc\t=",	clmnHdr_xLoc);
//		CONSOLE_DEBUG_W_NUM("clmnHdrWidth\t=",	clmnHdrWidth);

		SetWidget(				iii,	clmnHdr_xLoc,			yLoc,		clmnHdrWidth,		cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
		//	SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));

		clmnHdr_xLoc	=	tabArray[iii - kAlpacaList_ClmTitle1];;
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kAlpacaList_ClmTitle1,	"ip-addr:port");
	SetWidgetText(		kAlpacaList_ClmTitle2,	"/etc/hosts");
	SetWidgetText(		kAlpacaList_ClmTitle3,	"type");
	SetWidgetText(		kAlpacaList_ClmTitle4,	"name");
	SetWidgetText(		kAlpacaList_ClmTitle5,	"TBD");
	yLoc			+=	cRadioBtnHt;
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

	xLoc		=	0;
	widgetWidth	=	cWidth / 2;
	SetWidget(				kAlpacaList_AlpacaDev_Total,	xLoc,	yLoc,	widgetWidth,	cTitleHeight);
	SetWidgetFont(			kAlpacaList_AlpacaDev_Total,	kFont_Medium);
	SetWidgetText(			kAlpacaList_AlpacaDev_Total,	"Total units =?");
	SetWidgetJustification(	kAlpacaList_AlpacaDev_Total,	kJustification_Left);
	SetWidgetTextColor(		kAlpacaList_AlpacaDev_Total,	CV_RGB(255,	255,	255));

	xLoc		+=	widgetWidth;
	xLoc		+=	2;

	widgetWidth	=	cWidth / 5;
	SetWidget(			kAlpacaList_ChkBx_IncManagment,	xLoc,	yLoc,		widgetWidth,	cTitleHeight);
	SetWidgetFont(		kAlpacaList_ChkBx_IncManagment,	kFont_Medium);
	SetWidgetType(		kAlpacaList_ChkBx_IncManagment,	kWidgetType_CheckBox);
	SetWidgetText(		kAlpacaList_ChkBx_IncManagment,	"Include Management");
	SetWidgetChecked(	kAlpacaList_ChkBx_IncManagment, cIncludeManagment);

	xLoc		+=	widgetWidth;
	xLoc		+=	2;

	SetWidget(			kAlpacaList_Btn_Refresh,	xLoc,	yLoc,		widgetWidth,		cTitleHeight);
	SetWidgetFont(		kAlpacaList_Btn_Refresh,	kFont_Medium);
	SetWidgetType(		kAlpacaList_Btn_Refresh,	kWidgetType_Button);
	SetWidgetText(		kAlpacaList_Btn_Refresh,	"Refresh");




	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

//	SetAlpacaLogo(kAlpacaList_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
//	SetIPaddressBoxes(kAlpacaList_IPaddr, kAlpacaList_Readall, kAlpacaList_AlpacaDrvrVersion, -1);
//	SetIPaddressBoxes(kAlpacaList_IPaddr, kAlpacaList_Readall, -1, -1);
}

//*****************************************************************************
void	WindowTabAlpacaList::ProcessButtonClick(const int buttonIdx)
{

	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kAlpacaList_ClmTitle1:
		case kAlpacaList_ClmTitle2:
		case kAlpacaList_ClmTitle3:
		case kAlpacaList_ClmTitle4:
		case kAlpacaList_ClmTitle5:
			cSortColumn	=	buttonIdx - kAlpacaList_ClmTitle1;
			UpdateSortOrder();
			ForceUpdate();
			break;

		case kAlpacaList_ChkBx_IncManagment:
			cIncludeManagment	=	!cIncludeManagment;
			SetWidgetChecked(kAlpacaList_ChkBx_IncManagment, cIncludeManagment);
		//*	fall through to force refresh

		case kAlpacaList_Btn_Refresh:
			ClearRemoteDeviceList();
			ForceUpdate();
			break;
	}
}

//*****************************************************************************
void	WindowTabAlpacaList::ProcessDoubleClick(const int buttonIdx)
{
int		deviceIdx;
char	windowName[64];
bool	windowExists;
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);

	if ((buttonIdx >= kAlpacaList_AlpacaDev_01) && (buttonIdx <= kAlpacaList_AlpacaDev_Last))
	{
		deviceIdx	=	buttonIdx - kAlpacaList_AlpacaDev_01;
		if (deviceIdx >= 0)
		{
			switch(cRemoteDeviceList[deviceIdx].deviceTypeEnum)
			{

			case kDeviceType_Camera:
				strcpy(windowName, "Camera-");
				strcat(windowName, cRemoteDeviceList[deviceIdx].hostName);
				windowExists	=	CheckForOpenWindowByName(windowName);
				if (windowExists)
				{
					CONSOLE_DEBUG_W_STR("Window already open:", windowName);
				}
				else
				{
					new ControllerCamNormal(windowName, &cRemoteDeviceList[deviceIdx]);
				}
				break;

			case kDeviceType_CoverCalibrator:
				break;

			case kDeviceType_Dome:
				strcpy(windowName, "Dome-");
				strcat(windowName, cRemoteDeviceList[deviceIdx].hostName);
				windowExists	=	CheckForOpenWindowByName(windowName);
				if (windowExists)
				{
					CONSOLE_DEBUG_W_STR("Window already open:", windowName);
				}
				else
				{
					new ControllerDome(windowName, &cRemoteDeviceList[deviceIdx]);
				}
				break;

			case kDeviceType_Filterwheel:
				break;

			case kDeviceType_Focuser:
				GenerateFocuserWindowName(&cRemoteDeviceList[deviceIdx], 1, windowName);
				windowExists	=	CheckForOpenWindowByName(windowName);
				if (windowExists)
				{
					CONSOLE_DEBUG_W_STR("Window already open:", windowName);
				}
				else
				{
					CheckForFocuser(&cRemoteDeviceList[deviceIdx]);
				}
				break;

			case kDeviceType_Management:
			case kDeviceType_Observingconditions:
			case kDeviceType_Rotator:
				break;

			case kDeviceType_Telescope:
				strcpy(windowName, "Telescope-");
				strcat(windowName, cRemoteDeviceList[deviceIdx].hostName);
				windowExists	=	CheckForOpenWindowByName(windowName);
				if (windowExists)
				{
					CONSOLE_DEBUG_W_STR("Window already open:", windowName);
				}
				else
				{
					new ControllerTelescope(	windowName,
												&cRemoteDeviceList[deviceIdx].deviceAddress,
												cRemoteDeviceList[deviceIdx].port,
												cRemoteDeviceList[deviceIdx].alpacaDeviceNum);
				}
				break;

			case kDeviceType_SafetyMonitor:
				break;

			case kDeviceType_Switch:
				strcpy(windowName, "Switch-");
				strcat(windowName, cRemoteDeviceList[deviceIdx].hostName);
				windowExists	=	CheckForOpenWindowByName(windowName);
				if (windowExists)
				{
					CONSOLE_DEBUG_W_STR("Window already open:", windowName);
				}
				else
				{
					new ControllerSwitch(	windowName,
											&cRemoteDeviceList[deviceIdx].deviceAddress,
											cRemoteDeviceList[deviceIdx].port,
											cRemoteDeviceList[deviceIdx].alpacaDeviceNum);
				}
				break;

			//*	extras defined by MLS
			case kDeviceType_Multicam:
			case kDeviceType_Shutter:
			case kDeviceType_SlitTracker:
			case kDeviceType_undefined:
			case kDeviceType_last:
				break;

			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);
			CONSOLE_DEBUG_W_NUM("deviceIdx\t=", deviceIdx);
			CONSOLE_ABORT(__FUNCTION__);
		}
	}
}


//**************************************************************************************
static int	FindDeviceInList(TYPE_REMOTE_DEV *theDevice, TYPE_REMOTE_DEV *theList, int maxDevices)
{
	int		foundIndex;
	int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	foundIndex	=	-1;
	iii			=	0;
	while ((foundIndex < 0) && (iii < maxDevices))
	{
		if ((theDevice->deviceAddress.sin_addr.s_addr == theList[iii].deviceAddress.sin_addr.s_addr) &&
				(theDevice->port == theList[iii].port) &&
				(strcasecmp(theDevice->deviceTypeStr, theList[iii].deviceTypeStr) == 0))
		{
			foundIndex	=	iii;
		}
		iii++;
	}
	return(foundIndex);
}

//**************************************************************************************
void	WindowTabAlpacaList::ClearRemoteDeviceList(void)
{
	int		iii;

	for (iii=kAlpacaList_AlpacaDev_01; iii<=kAlpacaList_AlpacaDev_Last; iii++)
	{
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetText(			iii,	"");

	}

	for (iii=0; iii<kMaxDeviceCnt; iii++)
	{
		memset(&cRemoteDeviceList[iii], 0, sizeof(TYPE_REMOTE_DEV));
	}
	cAlpacaDevCnt		=	0;
	cPrevAlpacaDevCnt	=	-1;
	cSortColumn			=	-1;

	SetWidgetBGColor(kAlpacaList_ClmTitle1,	((cSortColumn == 0) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAlpacaList_ClmTitle2,	((cSortColumn == 1) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAlpacaList_ClmTitle3,	((cSortColumn == 2) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAlpacaList_ClmTitle4,	((cSortColumn == 3) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAlpacaList_ClmTitle5,	((cSortColumn == 4) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));

}

//**************************************************************************************
void	WindowTabAlpacaList::UpdateRemoteDeviceList(void)
{
	int		iii;
	bool	includeDevice;
	int		foundIndex;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);

	for (iii=0; iii<gRemoteCnt; iii++)
	{
		//*	first, see if this device is already in our list.
		foundIndex		=	FindDeviceInList(&gRemoteList[iii], cRemoteDeviceList, kMaxDeviceCnt);
		if (foundIndex >= 0)
		{
			//*	its already in the list
		}
		else
		{
			includeDevice	=	true;

			//		CONSOLE_DEBUG_W_STR("deviceTypeStr\t=", gRemoteList[iii].deviceTypeStr);
			if ((cIncludeManagment == false) && (strcasecmp(gRemoteList[iii].deviceTypeStr, "management") == 0))
			{
				includeDevice	=	false;
			}
			if (includeDevice)
			{
				if (cAlpacaDevCnt < kMaxDeviceCnt)
				{
					cRemoteDeviceList[cAlpacaDevCnt]			=	gRemoteList[iii];
					cRemoteDeviceList[cAlpacaDevCnt].validEntry	=	true;

					cAlpacaDevCnt++;
				}
			}
		}
	}

	//*	now update the widget text
	UpdateOnScreenWidgetList();

	if (cAlpacaDevCnt != cPrevAlpacaDevCnt)
	{
		ForceUpdate();
		cPrevAlpacaDevCnt	=	cAlpacaDevCnt;
	}
}

//**************************************************************************************
void	WindowTabAlpacaList::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
char	textString[128];
char	ipAddrStr[32];
int		myDevCount;

//	CONSOLE_DEBUG(__FUNCTION__);

	iii			=	0;
	myDevCount	=	0;
	while ((iii < kMaxDeviceCnt))
	{
		boxId	=	iii + kAlpacaList_AlpacaDev_01;
		if ((boxId <= kAlpacaList_AlpacaDev_Last) && (cRemoteDeviceList[iii].validEntry))
		{
			inet_ntop(AF_INET, &(cRemoteDeviceList[iii].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);

			sprintf(textString, "%s:%d\t%s\t%s\t%s",	ipAddrStr,
					cRemoteDeviceList[iii].port,
					cRemoteDeviceList[iii].hostName,
					cRemoteDeviceList[iii].deviceTypeStr,
					cRemoteDeviceList[iii].deviceNameStr);
			SetWidgetText(boxId, textString);

			SetWidgetTextColor(		boxId,	CV_RGB(255,	255,	255));

			if (cRemoteDeviceList[iii].deviceTypeEnum == kDeviceType_Focuser)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(0,	255,	0));		//*	green
			}
			if (cRemoteDeviceList[iii].deviceTypeEnum == kDeviceType_Camera)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(255,	255,	0));		//*	yellow
			}
			if (cRemoteDeviceList[iii].deviceTypeEnum == kDeviceType_Switch)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(0,	255,	255));		//*	cyan
			}
			if (cRemoteDeviceList[iii].deviceTypeEnum == kDeviceType_Dome)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(255,	0,	255));			//*	magenta
			}
			if (cRemoteDeviceList[iii].deviceTypeEnum == kDeviceType_Telescope)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(100,	100,	255));		//*	blue
			}
			myDevCount++;
		}
		else
		{
			//	CONSOLE_DEBUG_W_NUM("iii\t=", iii);
		}
		iii++;
	}
	sprintf(textString, "Total Alpaca Devices found=%d", myDevCount);
	SetWidgetText(kAlpacaList_AlpacaDev_Total, textString);

	SetWidgetBGColor(kAlpacaList_ClmTitle1,	((cSortColumn == 0) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAlpacaList_ClmTitle2,	((cSortColumn == 1) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAlpacaList_ClmTitle3,	((cSortColumn == 2) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAlpacaList_ClmTitle4,	((cSortColumn == 3) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kAlpacaList_ClmTitle5,	((cSortColumn == 4) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));

}

static int	gSortColumn;

//**************************************************************************************
static  int RemoteObjectQsortProc(const void *e1, const void *e2)
{
	TYPE_REMOTE_DEV	*obj1, *obj2;
	int				returnValue;
	uint32_t		ipAddr1;
	uint32_t		ipAddr2;


	obj1		=	(TYPE_REMOTE_DEV *)e1;
	obj2		=	(TYPE_REMOTE_DEV *)e2;

	returnValue	=	0;
	switch(gSortColumn)
	{
	case 0:
		returnValue	=	0;	//*	let the default code below take care of this
		break;

	case 1:
		returnValue	=	strcasecmp(obj1->hostName, obj2->hostName);
		break;

	case 2:
		returnValue	=	strcasecmp(obj1->deviceTypeStr, obj2->deviceTypeStr);
		break;

	case 3:
		returnValue	=	strcasecmp(obj1->deviceNameStr, obj2->deviceNameStr);
		break;

	case 4:
		break;
	}
	//*	if they are the same, sort by address
	if (returnValue == 0)
	{
		ipAddr1	=	(obj1->deviceAddress.sin_addr.s_addr & 0x000000ff) << 24;
		ipAddr1	+=	(obj1->deviceAddress.sin_addr.s_addr & 0x0000ff00) << 8;
		ipAddr1	+=	(obj1->deviceAddress.sin_addr.s_addr & 0x00ff0000) >> 8;
		ipAddr1	+=	(obj1->deviceAddress.sin_addr.s_addr & 0xff000000) >> 24;

		ipAddr2	=	(obj2->deviceAddress.sin_addr.s_addr & 0x000000ff) << 24;
		ipAddr2	+=	(obj2->deviceAddress.sin_addr.s_addr & 0x0000ff00) << 8;
		ipAddr2	+=	(obj2->deviceAddress.sin_addr.s_addr & 0x00ff0000) >> 8;
		ipAddr2	+=	(obj2->deviceAddress.sin_addr.s_addr & 0xff000000) >> 24;

		if (ipAddr1 < ipAddr2)
		{
			returnValue	=	-1;
		}
		else if (ipAddr1 > ipAddr2)
		{
			returnValue	=	1;
		}
	}

//	CONSOLE_DEBUG_W_NUM("returnValue\t=", returnValue);

	return(returnValue);

}


//**************************************************************************************
void	WindowTabAlpacaList::UpdateSortOrder(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cAlpacaDevCnt\t=", cAlpacaDevCnt);

	if (cSortColumn >= 0)
	{
		gSortColumn	=	cSortColumn;
		qsort(cRemoteDeviceList, cAlpacaDevCnt, sizeof(TYPE_REMOTE_DEV), RemoteObjectQsortProc);

		UpdateOnScreenWidgetList();
	}

}







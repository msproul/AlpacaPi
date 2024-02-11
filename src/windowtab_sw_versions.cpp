//*****************************************************************************
//*		windowtab_sw_versions.cpp		(c) 2024 by Mark Sproul
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
//*	Feb 10,	2024	<MLS> Created windowtab_sw_versions.cpp
//*****************************************************************************

#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"discoverythread.h"
#include	"discovery_lib.h"
#include	"linuxerrors.h"
#include	"helper_functions.h"

#include	"windowtab_sw_versions.h"

#include	"controller.h"
#include	"controller_alpacaUnit.h"

static bool	gInvertSort_SwVersion	=	false;
static int	gSortColumn;

//**************************************************************************************
WindowTabSwVersions::WindowTabSwVersions(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn			=	-1;
	cFirstLineIdx		=	0;

	SetupWindowControls();
	UpdateButtons();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabSwVersions::~WindowTabSwVersions(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabSwVersions::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		textBoxHt;
int		textBoxWd;
int		iii;
short	widthArray[kMaxTabStops]	=	{150, 100, 100, 125, 125, 125, 125, 125, 125, 125, 0, 0, 0};
short	tabArray[kMaxTabStops];
int		clmnHdr_xLoc;
int		clmnHdrWidth;
int		tabOffset;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kSWversionsList_Title, -1, yLoc, "Software Versions on Alpaca Devices");

	//------------------------------------------
	//*	set up the tab stops
	for (iii=0; iii<kMaxTabStops; iii++)
	{
		tabArray[iii]	=	0;
	}
	tabOffset	=	widthArray[0];
	iii			=	0;
	while ((widthArray[iii] > 0) && (iii < kMaxTabStops))
	{
		tabArray[iii]	=	tabOffset;
		tabOffset		+=	widthArray[iii+1];
		iii++;
	}

	//------------------------------------------
	clmnHdr_xLoc		=	1;
	iii	=	kSWversionsList_ClmTitle1;
	while (iii < kSWversionsList_ClmOutline)
	{
		clmnHdrWidth	=	tabArray[iii - kSWversionsList_ClmTitle1] - clmnHdr_xLoc;


		SetWidget(				iii,	clmnHdr_xLoc,	yLoc,	clmnHdrWidth,	cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(192,	192,	192));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,		0));

		clmnHdr_xLoc	=	tabArray[iii - kSWversionsList_ClmTitle1];
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kSWversionsList_ClmTitle1,	"ip-address");
	SetWidgetText(		kSWversionsList_ClmTitle2,	"port");
	SetWidgetText(		kSWversionsList_ClmTitle3,	"/etc/hosts");
	SetWidgetText(		kSWversionsList_ClmTitle4,	"openCV");
	SetWidgetText(		kSWversionsList_ClmTitle5,	"cFitsIo");
	SetWidgetText(		kSWversionsList_ClmTitle6,	"WiringPi");
	SetWidgetText(		kSWversionsList_ClmTitle7,	"Hardware");
	SetWidgetText(		kSWversionsList_ClmTitle8,	"tbd");
	SetWidgetText(		kSWversionsList_ClmTitle9,	"tbd");
	SetWidgetText(		kSWversionsList_ClmTitle10,	"tbd");

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;
	yLoc			+=	6;

	//=======================================================
	xLoc		=	10;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	for (iii=kSWversionsList_AlpacaDev_01; iii <= kSWversionsList_AlpacaDev_Last; iii++)
	{
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBorder(		iii,	false);
		SetWidgetTabStops(		iii,	tabArray);
		SetWidgetBGColorSelected(iii,	CV_RGB(100,	100,	100));

		yLoc			+=	textBoxHt;
		yLoc			+=	4;
	}

	xLoc		=	0;

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	SetAlpacaLogoBottomCorner(kSWversionsList_AlpacaLogo);
}

//*****************************************************************************
void	WindowTabSwVersions::SetNewSelectedDevice(int deviceIndex)
{
int		iii;
	if (deviceIndex >= 0)
	{
		//*	clear out previous selections
		for (iii=0; iii<kMaxAlpacaIPaddrCnt; iii++)
		{
			gAlpacaUnitList[iii].lineSelected	=	false;
		}
		gAlpacaUnitList[deviceIndex].lineSelected	=	true;
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
}

//*****************************************************************************
void	WindowTabSwVersions::HandleKeyDown(const int keyPressed)
{
int		deviceIndex;
int		iii;
int		theExtendedChar;

//	CONSOLE_DEBUG_W_HEX("keyPressed=", keyPressed);

	//*	find a selected device
	deviceIndex	=	-1;
	for (iii=0; iii<gAlpacaUnitCnt; iii++)
	{
		if (gAlpacaUnitList[iii].lineSelected)
		{
			deviceIndex	=	iii;
			break;
		}
	}

	theExtendedChar	=	keyPressed & 0x00ffff;
	switch(theExtendedChar)
	{
		//*	return, open current selected entry
		case 0x0d:
		case 0xff8d:
			if ((deviceIndex >= 0) && (deviceIndex < gAlpacaUnitCnt))
			{
				CreateAlpacaUnitWindow(&gAlpacaUnitList[deviceIndex]);
			}
			break;

		//*	delete key, delete the current entry
		case 0x0000ff:
		case 0x00ffff:
		case 0x0080ff:
			CONSOLE_DEBUG_W_HEX("Ignored: theExtendedChar\t=", theExtendedChar);
			if (deviceIndex >= 0)
			{
				//*	delete that device out of the list
				iii	=	deviceIndex;
				while (iii < gAlpacaUnitCnt)
				{
					//*	move the rest up one slot in the list
					gAlpacaUnitList[iii]	=	gAlpacaUnitList[iii+1];
					iii++;
				}
				//*	clear the rest of the list
				while (iii < kMaxAlpacaIPaddrCnt)
				{
					memset(&gAlpacaUnitList[iii], 0, sizeof(TYPE_ALPACA_UNIT));
					iii++;
				}
				gAlpacaUnitCnt--;
				//*	make sure we dont run out of bounds
				if (gAlpacaUnitCnt < 0)
				{
					gAlpacaUnitCnt	=	0;
				}
				UpdateOnScreenWidgetList();
				ForceWindowUpdate();
			}
			break;

		//*	up arrow key
		case 0x00ff52:
			deviceIndex	-=	1;
			if (deviceIndex >= 0)
			{
				SetNewSelectedDevice(deviceIndex);
			}
			break;

		//*	down arrow key
		case 0x00ff54:
			deviceIndex	+=	1;
			if (deviceIndex >= 0)
			{
				SetNewSelectedDevice(deviceIndex);
			}
			break;

		default:
			CONSOLE_DEBUG_W_HEX("Ignored: keyPressed     \t=", keyPressed);
			CONSOLE_DEBUG_W_HEX("Ignored: theExtendedChar\t=", theExtendedChar);
			break;
	}
}

//*****************************************************************************
void	WindowTabSwVersions::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool	updateFlag;
int		newSortColumn;

//	CONSOLE_DEBUG(__FUNCTION__);
	updateFlag	=	true;
	switch(buttonIdx)
	{
		case kSWversionsList_ClmTitle1:
		case kSWversionsList_ClmTitle2:
		case kSWversionsList_ClmTitle3:
		case kSWversionsList_ClmTitle4:
		case kSWversionsList_ClmTitle5:
		case kSWversionsList_ClmTitle6:
		case kSWversionsList_ClmTitle7:
		case kSWversionsList_ClmTitle8:
		case kSWversionsList_ClmTitle9:
		case kSWversionsList_ClmTitle10:
			newSortColumn	=	buttonIdx - kSWversionsList_ClmTitle1;
			if (newSortColumn == cSortColumn)
			{
				gInvertSort_SwVersion	=	!gInvertSort_SwVersion;
			}
			else
			{
				gInvertSort_SwVersion	=	false;
			}
			cSortColumn	=	newSortColumn;
			gSortColumn	=	cSortColumn;

//			UpdateSortOrder();
			ForceWindowUpdate();
			break;

		default:
			updateFlag	=	false;
			break;
	}
	if (updateFlag)
	{
		UpdateButtons();
		ForceWindowUpdate();
	}
}


//**************************************************************************************
void	WindowTabSwVersions::UpdateButtons(void)
{
//	SetWidgetChecked(kSWversionsList_TempModeRaw, (cGraphMode == kGraphMode_Raw));
//	SetWidgetChecked(kSWversionsList_TempModeAvg, (cGraphMode == kGraphMode_Avg5));
}


//*****************************************************************************
void	WindowTabSwVersions::ProcessDoubleClick(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
int		deviceIdx;

	deviceIdx	=	cFirstLineIdx + (widgetIdx - kSWversionsList_AlpacaDev_01);

	if ((deviceIdx >= 0) && (deviceIdx < gAlpacaUnitCnt))
	{
		CreateAlpacaUnitWindow(&gAlpacaUnitList[deviceIdx]);
	}
	else
	{
		CONSOLE_DEBUG("Index out of range");
	}
}

//*****************************************************************************
void	WindowTabSwVersions::ProcessMouseEvent(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{

//	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);
	switch(event)
	{
		case cv::EVENT_MOUSEMOVE:
			break;

		case cv::EVENT_LBUTTONDOWN:
//			CONSOLE_DEBUG_W_NUM("EVENT_LBUTTONDOWN", widgetIdx);
			cLeftButtonDown	=	true;
			break;

		case cv::EVENT_LBUTTONUP:
			CONSOLE_DEBUG_W_NUM("EVENT_LBUTTONUP", widgetIdx);
			cLeftButtonDown	=	false;
			ProcessLineSelect(widgetIdx);
			break;

//
//		case cv::EVENT_RBUTTONDOWN:
//			cRightButtonDown		=	true;
//			break;
//
//		case cv::EVENT_MBUTTONDOWN:
//			break;
//
//		case cv::EVENT_RBUTTONUP:
//			cRightButtonDown		=	false;
//			break;
//
//		case cv::EVENT_MBUTTONUP:
//			break;
//
//		case cv::EVENT_LBUTTONDBLCLK:
//			break;
//
//		case cv::EVENT_RBUTTONDBLCLK:
//			break;
//
//		case cv::EVENT_MBUTTONDBLCLK:
//			break;
//
#if (CV_MAJOR_VERSION >= 3)
		case cv::EVENT_MOUSEWHEEL:
			break;
		case cv::EVENT_MOUSEHWHEEL:
			break;
#endif
		default:
//			CONSOLE_DEBUG_W_NUM("UNKNOWN EVENT", event);
			break;
	}
}

//*****************************************************************************
void	WindowTabSwVersions::ProcessMouseWheelMoved(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);

	//*	make sure we have something to scroll
	if (gAlpacaUnitCnt > 0)
	{
		cFirstLineIdx	+=	wheelMovement;
		if (cFirstLineIdx < 0)
		{
			cFirstLineIdx	=	0;
		}
		if (cFirstLineIdx >= gAlpacaUnitCnt)
		{
			cFirstLineIdx	=	gAlpacaUnitCnt -1;
		}
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
}

//**************************************************************************************
void	WindowTabSwVersions::ProcessLineSelect(int widgetIdx)
{
int		deviceIdx;
int		adjustedIdx;
//int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= kSWversionsList_AlpacaDev_01) && (widgetIdx < kSWversionsList_AlpacaDev_Last))
	{
		adjustedIdx	=	widgetIdx - kSWversionsList_AlpacaDev_01;
		deviceIdx	=	adjustedIdx + cFirstLineIdx;

		if ((deviceIdx >= 0) && (deviceIdx < kMaxAlpacaIPaddrCnt))
		{
			SetNewSelectedDevice(deviceIdx);
//
//			//*	clear out previous selections
//			for (iii=0; iii<kMaxAlpacaIPaddrCnt; iii++)
//			{
//				gAlpacaUnitList[iii].lineSelected	=	false;
//			}
//			gAlpacaUnitList[deviceIdx].lineSelected	=	true;
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("deviceIdx is out of bounds\t=", deviceIdx);
		}
//		UpdateOnScreenWidgetList();
//		ForceWindowUpdate();
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx is out of range\t=", widgetIdx);
	}
}

//**************************************************************************************
void	WindowTabSwVersions::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
int		jjj;
char	textString[128];
char	extraString[32];
char	ipAddrStr[32];
int		deviceIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_ABORT(__FUNCTION__);

	//*	limit how far we can scroll
	if (cFirstLineIdx >= gAlpacaUnitCnt)
	{
		cFirstLineIdx	=	gAlpacaUnitCnt -1;
	}
	if (cFirstLineIdx < 0)
	{
		cFirstLineIdx	=	0;
	}

	cLinesOnScreen	=	(kSWversionsList_AlpacaDev_Last - kSWversionsList_AlpacaDev_01) + 1;
	iii				=	0;
	while (iii < cLinesOnScreen)
	{
		boxId		=	iii + kSWversionsList_AlpacaDev_01;
		deviceIdx	=	iii + cFirstLineIdx;
		if ((boxId < kSWversionsList_AlpacaDev_Last) && (deviceIdx < gAlpacaUnitCnt))
		{
			inet_ntop(AF_INET, &(gAlpacaUnitList[deviceIdx].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);

			sprintf(textString, "%s\t%d\t%s",	ipAddrStr,
												gAlpacaUnitList[deviceIdx].port,
												gAlpacaUnitList[deviceIdx].hostName);

			for (jjj=0; jjj<kSoftwareVers_last; jjj++)
			{
				sprintf(extraString, "\t%s", gAlpacaUnitList[deviceIdx].SoftwareVersion[jjj].SoftwareVerStr);
				strcat(textString, extraString);
			}
			strcat(textString, "\t");
			strcat(textString, gAlpacaUnitList[deviceIdx].versionString);

			//-----------------------------------------------------
			//*	deal with selected state
			if (gAlpacaUnitList[deviceIdx].lineSelected == true)
			{
				SetWidgetLineSelect(boxId, true);
			}
			else
			{
				SetWidgetLineSelect(boxId, false);
			}
			SetWidgetText(boxId, textString);
		}
		else if (boxId <= kSWversionsList_AlpacaDev_Last)
		{
			SetWidgetTextColor(		boxId,	CV_RGB(255,	255,	255));
			SetWidgetLineSelect(	boxId, false);
			SetWidgetText(boxId, "---");
		}
		else
		{
			SetWidgetText(boxId, "");
		}
		iii++;
	}
	sprintf(textString, "Total Alpaca IP address found=%d", gAlpacaUnitCnt);
	SetWidgetText(kSWversionsList_AlpacaDev_Total, textString);

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "exit");
}



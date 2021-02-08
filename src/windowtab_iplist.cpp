//*****************************************************************************
//*		windowtab_iplist.cpp		(c) 2021 by Mark Sproul
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
//*	Feb  5,	2021	<MLS> Created windowtab_iplist.cpp
//*****************************************************************************

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"discoverythread.h"
#include	"discovery_lib.h"

#include	"windowtab_iplist.h"

#include	"controller.h"



//**************************************************************************************
WindowTabIPList::WindowTabIPList(	const int	xSize,
									const int	ySize,
									CvScalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn			=	-1;


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabIPList::~WindowTabIPList(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabIPList::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		textBoxHt;
int		textBoxWd;
int		widgetWidth;
int		iii;
short	tabArray[kMaxTabStops]	=	{150, 250, 500, 600, 700, 1100, 1150, 1199, 0};
int		clmnHdr_xLoc;
int		clmnHdrWidth;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kIPaddrList_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kIPaddrList_Title, "AlpacaPi IP address List (This is primarily for debugging)");
	SetBGcolorFromWindowName(kIPaddrList_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//------------------------------------------
	xLoc	=	5;
	SetWidget(		kIPaddrList_DiscoveryThrdStatus,	xLoc,		yLoc,		cBtnWidth * 2,	cTitleHeight);
	SetWidgetType(	kIPaddrList_DiscoveryThrdStatus,	kWidgetType_Text);
	SetWidgetFont(	kIPaddrList_DiscoveryThrdStatus,	kFont_Medium);
	SetWidgetText(	kIPaddrList_DiscoveryThrdStatus,	"Status");
	xLoc	+=	cBtnWidth * 2;
	xLoc	+=	2;

	SetWidget(			kIPaddrList_DiscoveryThrdStop,	xLoc,		yLoc,		cBtnWidth,		cTitleHeight);
	SetWidgetType(		kIPaddrList_DiscoveryThrdStop,	kWidgetType_Button);
	SetWidgetFont(		kIPaddrList_DiscoveryThrdStop,	kFont_Medium);
	SetWidgetBGColor(	kIPaddrList_DiscoveryThrdStop,	CV_RGB(255,	255,	255));
	SetWidgetText(		kIPaddrList_DiscoveryThrdStop,	"Stop");
	xLoc	+=	cBtnWidth;
	xLoc	+=	2;

	SetWidget(			kIPaddrList_DiscoveryThrdReStart,	xLoc,		yLoc,		cBtnWidth,		cTitleHeight);
	SetWidgetType(		kIPaddrList_DiscoveryThrdReStart,	kWidgetType_Button);
	SetWidgetFont(		kIPaddrList_DiscoveryThrdReStart,	kFont_Medium);
	SetWidgetBGColor(	kIPaddrList_DiscoveryThrdReStart,	CV_RGB(255,	255,	255));
	SetWidgetText(		kIPaddrList_DiscoveryThrdReStart,	"Restart");
	xLoc	+=	cBtnWidth;
	xLoc	+=	2;

	SetWidget(			kIPaddrList_DiscoveryClear,	xLoc,		yLoc,		cBtnWidth,		cTitleHeight);
	SetWidgetType(		kIPaddrList_DiscoveryClear,	kWidgetType_Button);
	SetWidgetFont(		kIPaddrList_DiscoveryClear,	kFont_Medium);
	SetWidgetBGColor(	kIPaddrList_DiscoveryClear,	CV_RGB(255,	255,	255));
	SetWidgetText(		kIPaddrList_DiscoveryClear,	"Clear");
	xLoc	+=	cBtnWidth;
	xLoc	+=	2;



	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//------------------------------------------
	xLoc	=	5;
	SetWidget(				kIPaddrList_ErrorMsg,	xLoc,		yLoc,		tabArray[5],		cTitleHeight);
	SetWidgetType(			kIPaddrList_ErrorMsg,	kWidgetType_Text);
	SetWidgetJustification(	kIPaddrList_ErrorMsg,	kJustification_Left);
	SetWidgetFont(			kIPaddrList_ErrorMsg,	kFont_Medium);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;



	clmnHdr_xLoc		=	1;
	iii	=	kIPaddrList_ClmTitle1;
	while(iii <= kIPaddrList_ClmTitle6)
	{
		clmnHdrWidth	=	tabArray[iii - kIPaddrList_ClmTitle1] - clmnHdr_xLoc;


		SetWidget(				iii,	clmnHdr_xLoc,			yLoc,		clmnHdrWidth,		cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Text);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(192,	192,	192));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,		0));

		clmnHdr_xLoc	=	tabArray[iii - kIPaddrList_ClmTitle1];;
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kIPaddrList_ClmTitle1,	"ip-address");
	SetWidgetText(		kIPaddrList_ClmTitle2,	"port");
	SetWidgetText(		kIPaddrList_ClmTitle3,	"/etc/hosts");
	SetWidgetText(		kIPaddrList_ClmTitle4,	"Valid Count");
	SetWidgetText(		kIPaddrList_ClmTitle5,	"Error Count");
	SetWidgetText(		kIPaddrList_ClmTitle6,	"Status");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;
	yLoc			+=	6;

	SetWidgetOutlineBox(kIPaddrList_Outline, kIPaddrList_DiscoveryThrdStatus, kIPaddrList_ClmTitle6);



	//=======================================================
	xLoc		=	10;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	for (iii=kIPaddrList_AlpacaDev_01; iii<=kIPaddrList_AlpacaDev_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBorder(		iii,	false);
		SetWidgetTabStops(		iii,	tabArray);

		yLoc			+=	textBoxHt;
		yLoc			+=	4;
	}

	xLoc		=	0;
	widgetWidth	=	cWidth / 2;
	SetWidget(				kIPaddrList_AlpacaDev_Total,	xLoc,	yLoc,	widgetWidth,	cTitleHeight);
	SetWidgetFont(			kIPaddrList_AlpacaDev_Total,	kFont_Medium);
	SetWidgetText(			kIPaddrList_AlpacaDev_Total,	"Total units =?");
	SetWidgetJustification(	kIPaddrList_AlpacaDev_Total,	kJustification_Left);
	SetWidgetTextColor(		kIPaddrList_AlpacaDev_Total,	CV_RGB(255,	255,	255));

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

}

//*****************************************************************************
void	WindowTabIPList::ProcessButtonClick(const int buttonIdx)
{
bool	updateFlag;

//	CONSOLE_DEBUG(__FUNCTION__);
	updateFlag	=	true;
	switch(buttonIdx)
	{
		case kIPaddrList_DiscoveryThrdStop:
			gDiscoveryThreadKeepRunning	=	false;
			SetWidgetText(	kIPaddrList_ErrorMsg,	"Stop message sent");
			break;

		case kIPaddrList_DiscoveryThrdReStart:
			if (gDiscoveryThreadIsRunning == false)
			{
				StartDiscoveryQuerryThread();
				SetWidgetText(	kIPaddrList_ErrorMsg,	"Start message sent");
			}
			else
			{
				SetWidgetText(	kIPaddrList_ErrorMsg,	"Discovery thread already running");
			}
			break;


		case kIPaddrList_DiscoveryClear:
			Discovery_ClearIPAddrList();
			ClearIPaddrList();
			break;

		default:
			updateFlag	=	false;
			break;
	}
	if (updateFlag)
	{
		ForceUpdate();
	}
}

//*****************************************************************************
void	WindowTabIPList::ProcessDoubleClick(const int buttonIdx)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);
}

//**************************************************************************************
void	WindowTabIPList::ClearIPaddrList(void)
{
int		boxId;

	for (boxId=kIPaddrList_AlpacaDev_01; boxId <= kIPaddrList_AlpacaDev_Last; boxId++)
	{
		SetWidgetTextColor(	boxId,	CV_RGB(255,	255,	255));
		SetWidgetText(		boxId,	"");
	}
}

//**************************************************************************************
void	WindowTabIPList::UpdateIPaddrList(void)
{
int		boxId;
int		iii;
char	textString[128];
char	ipAddrStr[32];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (gDiscoveryThreadIsRunning)
	{
		SetWidgetText(		kIPaddrList_DiscoveryThrdStatus,	"Discovery Thread is running");
		SetWidgetBGColor(	kIPaddrList_DiscoveryThrdStatus,	CV_RGB(0,	255,	0));
		SetWidgetTextColor(	kIPaddrList_DiscoveryThrdStatus,	CV_RGB(0,	0,	0));

		SetWidgetBGColor(	kIPaddrList_DiscoveryThrdReStart,	CV_RGB(128,	128,	128));
	}
	else
	{
		SetWidgetText(		kIPaddrList_DiscoveryThrdStatus,	"Discovery Thread is stopped");
		SetWidgetBGColor(	kIPaddrList_DiscoveryThrdStatus,	CV_RGB(255,	0,	0));
		SetWidgetTextColor(	kIPaddrList_DiscoveryThrdStatus,	CV_RGB(255,	255,	255));

		SetWidgetBGColor(	kIPaddrList_DiscoveryThrdReStart,	CV_RGB(255,	255,	255));
	}

	iii			=	0;
	for (iii=0; iii<gAlpacaUnitCnt; iii++)
	{
		boxId	=	iii + kIPaddrList_AlpacaDev_01;
		if (boxId <= kIPaddrList_AlpacaDev_Last)
		{
			inet_ntop(AF_INET, &(gAlpacaUnitList[iii].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);

			sprintf(textString, "%s\t%d\t%s\t%d\t%d",	ipAddrStr,
														gAlpacaUnitList[iii].port,
														gAlpacaUnitList[iii].hostName,
														gAlpacaUnitList[iii].queryOKcnt,
														gAlpacaUnitList[iii].queryERRcnt);

			if (gAlpacaUnitList[iii].currentlyActive)
			{
				strcat(textString, "\t");
				strcat(textString, gAlpacaUnitList[iii].versionString);

				SetWidgetTextColor(		boxId,	CV_RGB(0,	255,	0));
			}
			else if (gAlpacaUnitList[iii].queryERRcnt > 0)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(255,	0,	0));
				strcat(textString, "\tOff-line");
			}
			SetWidgetText(boxId, textString);

		}
		else
		{
			CONSOLE_DEBUG("Too many IP addresses");
		}
	}
	sprintf(textString, "Total Alpaca IP address found=%d", gAlpacaUnitCnt);
	SetWidgetText(kIPaddrList_AlpacaDev_Total, textString);

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "exit");
}


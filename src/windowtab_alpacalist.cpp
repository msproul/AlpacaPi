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
//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kAlpacaList_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kAlpacaList_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kAlpacaList_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

#if 0
	SetWidget(		kAlpacaList_ControllerVersion,	0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetFont(	kAlpacaList_ControllerVersion,	kFont_Medium);
	SetWidgetText(	kAlpacaList_ControllerVersion,	gFullVersionString);
	SetWidgetTextColor(kAlpacaList_ControllerVersion,	CV_RGB(255,	255,	255));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
#endif // 0

	xLoc		=	10;
	textBoxHt	=	17;
	textBoxWd	=	cWidth - (xLoc + 3);
	for (iii=kAlpacaList_AlpacaDev_01; iii<=kAlpacaList_AlpacaDev_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetType(			iii,	kWidgetType_CheckBox);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));

		yLoc			+=	textBoxHt;
		yLoc			+=	4;
	}



	SetAlpacaLogo(kAlpacaList_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kAlpacaList_IPaddr, kAlpacaList_Readall, kAlpacaList_AlpacaDrvrVersion, -1);
}



//TYPE_REMOTE_DEV		gRemoteList[kMaxDeviceListCnt];
//int					gRemoteCnt		=	0;
//**************************************************************************************
void	WindowTabAlpacaList::UpdateList(void)
{
int		iii;
char	ipAddrSt[32];
char	textString[128];
int		boxId;
int		myDevCount;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);

	myDevCount	=	0;
	for (iii=0; iii<gRemoteCnt; iii++)
	{
		inet_ntop(AF_INET, &(gRemoteList[iii].deviceAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);

		//*	i hope to find a better way to make this look pretty
		sprintf(textString, "%s:%d", ipAddrSt, gRemoteList[iii].port);
		while (strlen(textString) < 30)	{	strcat(textString, " ");	}

		strcat(textString,	gRemoteList[iii].deviceType);
		while (strlen(textString) < 50)	{	strcat(textString, " ");	}

		strcat(textString,	gRemoteList[iii].deviceName);
		while (strlen(textString) < 70)	{	strcat(textString, " ");	}


		//					gRemoteList[iii].alpacaDeviceNum,
		//					gRemoteList[iii].notSeenCounter);

		boxId	=	kAlpacaList_AlpacaDev_01 + iii;
		if (boxId <= kAlpacaList_AlpacaDev_Last)
		{
			SetWidgetText(boxId, textString);
			myDevCount++;
		}
	}
	cAlpacaDevCnt		=	myDevCount;
	if (cAlpacaDevCnt != cPrevAlpacaDevCnt)
	{
		ForceUpdate();
		cPrevAlpacaDevCnt	=	cAlpacaDevCnt;
	}
}







//*****************************************************************************
//*		windowtab_deviceselect.cpp		(c) 2021 by Mark Sproul
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
//*	Jan 26,	2021	<MLS> Created windowtab_deviceselect.cpp
//*****************************************************************************

#include	"alpaca_defs.h"
#include	"discoverythread.h"

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_deviceselect.h"
#include	"controller.h"
#include	"controller_skytravel.h"



//**************************************************************************************
WindowTabDeviceSelect::WindowTabDeviceSelect(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cSelectedDomeIdx		=	0;
	cSelectedTelescopeIdx	=	0;


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabDeviceSelect::~WindowTabDeviceSelect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
static char	gAlpacaPiTxt1[]	=
{
	"This screen allows you to select which devices are used by SkyTravel\r"
	"They are listed in the order of discovery\r"
	"This is only useful if you have more than one in a category\r"
	"The lists will automatically update if new devices appear\r"
};

//**************************************************************************************
void	WindowTabDeviceSelect::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		save_yLoc;
int		textBoxHt;
int		iii;
int		myRadioBtnWidth;

	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kDeviceSelect_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kDeviceSelect_Title, "Alpaca Device Selection");
	SetBGcolorFromWindowName(kDeviceSelect_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

//	SetWidget(		kDeviceSelect_ControllerVersion,	0,			yLoc,		cWidth,		cTitleHeight);
//	SetWidgetFont(	kDeviceSelect_ControllerVersion,	kFont_Medium);
//	SetWidgetText(	kDeviceSelect_ControllerVersion,	gFullVersionString);
//	SetWidgetTextColor(kDeviceSelect_ControllerVersion,	CV_RGB(255,	255,	255));
//	yLoc			+=	cTitleHeight;
//	yLoc			+=	2;

	textBoxHt		=	90;
	SetWidget(				kDeviceSelect_TextBox1,	0,			yLoc,		cWidth,		textBoxHt);
	SetWidgetType(			kDeviceSelect_TextBox1,	kWidgetType_MultiLineText);
	SetWidgetJustification(	kDeviceSelect_TextBox1,	kJustification_Left);
	SetWidgetFont(			kDeviceSelect_TextBox1,	kFont_Medium);
	SetWidgetTextColor(		kDeviceSelect_TextBox1,	CV_RGB(255,	255,	255));
	SetWidgetText(			kDeviceSelect_TextBox1, gAlpacaPiTxt1);
	yLoc			+=	textBoxHt;
	yLoc			+=	2;


	yLoc			+=	10;
	save_yLoc		=	yLoc;
	//------------------------------------------
	xLoc			=	5;
	myRadioBtnWidth	=	500;
	SetWidget(			kDeviceSelect_DomeTitle,	xLoc,		yLoc,		myRadioBtnWidth,		cBtnHeight);
	SetWidgetFont(		kDeviceSelect_DomeTitle,	kFont_Medium);
	SetWidgetTextColor(	kDeviceSelect_DomeTitle,	CV_RGB(255,	255,	255));
	SetWidgetText(		kDeviceSelect_DomeTitle, 	"Select Dome");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	for (iii=kDeviceSelect_Dome1; iii < kDeviceSelect_DomeOutline; iii++)
	{
		SetWidget(				iii,	xLoc,		yLoc,		myRadioBtnWidth,		cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_RadioButton);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetText(			iii, 	"Dome");

		yLoc			+=	cBtnHeight;
		yLoc			+=	2;
	}

	SetWidgetOutlineBox(kDeviceSelect_DomeOutline, kDeviceSelect_DomeTitle, (kDeviceSelect_DomeOutline -1));
	SetWidgetChecked(kDeviceSelect_Dome1, true);

	//------------------------------------------
	yLoc			=	save_yLoc;
	xLoc			+=	myRadioBtnWidth;
	xLoc			+=	10;
	SetWidget(			kDeviceSelect_TelescopeTitle,	xLoc,		yLoc,		myRadioBtnWidth,		cBtnHeight);
	SetWidgetFont(		kDeviceSelect_TelescopeTitle,	kFont_Medium);
	SetWidgetTextColor(	kDeviceSelect_TelescopeTitle,	CV_RGB(255,	255,	255));
	SetWidgetText(		kDeviceSelect_TelescopeTitle, 	"Select Telescope");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	for (iii=kDeviceSelect_Telescope1; iii < kDeviceSelect_TelescopeOutline; iii++)
	{
		SetWidget(				iii,	xLoc,		yLoc,		myRadioBtnWidth,		cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_RadioButton);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetText(			iii, 	"Telescope");

		yLoc			+=	cBtnHeight;
		yLoc			+=	2;
	}

	SetWidgetOutlineBox(kDeviceSelect_TelescopeOutline, kDeviceSelect_TelescopeTitle, (kDeviceSelect_TelescopeOutline -1));
	SetWidgetChecked(kDeviceSelect_Telescope1, true);


//	SetAlpacaLogo(kDeviceSelect_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
//	SetIPaddressBoxes(kDeviceSelect_IPaddr, kDeviceSelect_Readall, kDeviceSelect_AlpacaDrvrVersion, -1);
}

//*****************************************************************************
void	WindowTabDeviceSelect::ProcessButtonClick(const int buttonIdx)
{
bool				forceUpdate;
int					previousDevIdx;
int					newDevIdx;
ControllerSkytravel	*myControllerObj;



	CONSOLE_DEBUG(__FUNCTION__);

	forceUpdate	=	true;
	switch(buttonIdx)
	{
		case kDeviceSelect_Dome1:
		case kDeviceSelect_Dome2:
		case kDeviceSelect_Dome3:
		case kDeviceSelect_Dome4:
		case kDeviceSelect_Dome5:
			if (cDomeCnt > 1)
			{
				SetWidgetChecked(kDeviceSelect_Dome1, (buttonIdx == kDeviceSelect_Dome1));
				SetWidgetChecked(kDeviceSelect_Dome2, (buttonIdx == kDeviceSelect_Dome2));
				SetWidgetChecked(kDeviceSelect_Dome3, (buttonIdx == kDeviceSelect_Dome3));
				SetWidgetChecked(kDeviceSelect_Dome4, (buttonIdx == kDeviceSelect_Dome4));
				SetWidgetChecked(kDeviceSelect_Dome5, (buttonIdx == kDeviceSelect_Dome5));
				previousDevIdx	=	cSelectedDomeIdx;
				newDevIdx		=	buttonIdx - kDeviceSelect_Dome1;
				if (newDevIdx != previousDevIdx)
				{
					//*	it has changed, update SkyTravel
					myControllerObj	=	(ControllerSkytravel *)cParentObjPtr;
					if (myControllerObj != NULL)
					{
						cSelectedDomeIdx		=	newDevIdx;

						myControllerObj->SetDomeIPaddress(&cDeviceList_Domes[cSelectedDomeIdx]);
					}
				}
			}
			break;

		case kDeviceSelect_Telescope1:
		case kDeviceSelect_Telescope2:
		case kDeviceSelect_Telescope3:
		case kDeviceSelect_Telescope4:
		case kDeviceSelect_Telescope5:
			if (cTelescopeCnt > 1)
			{
				SetWidgetChecked(kDeviceSelect_Telescope1, (buttonIdx == kDeviceSelect_Telescope1));
				SetWidgetChecked(kDeviceSelect_Telescope2, (buttonIdx == kDeviceSelect_Telescope2));
				SetWidgetChecked(kDeviceSelect_Telescope3, (buttonIdx == kDeviceSelect_Telescope3));
				SetWidgetChecked(kDeviceSelect_Telescope4, (buttonIdx == kDeviceSelect_Telescope4));
				SetWidgetChecked(kDeviceSelect_Telescope5, (buttonIdx == kDeviceSelect_Telescope5));
				previousDevIdx	=	cSelectedTelescopeIdx;
				newDevIdx		=	buttonIdx - kDeviceSelect_Telescope1;
				if (newDevIdx != previousDevIdx)
				{
					//*	it has changed, update SkyTravel
					myControllerObj	=	(ControllerSkytravel *)cParentObjPtr;
					if (myControllerObj != NULL)
					{
						cSelectedTelescopeIdx		=	newDevIdx;

						myControllerObj->SetTelescopeIPaddress(&cDeviceList_Telescopes[cSelectedTelescopeIdx]);
					}
				}
			}
			break;

		default:
			forceUpdate	=	true;
			break;

	}
	if (forceUpdate)
	{
		ForceUpdate();
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
				(strcasecmp(theDevice->deviceTypeStr, theList[iii].deviceTypeStr) == 0) &&
				(theDevice->alpacaDeviceNum == theList[iii].alpacaDeviceNum))
		{
			foundIndex	=	iii;
		}
		iii++;
	}
	return(foundIndex);
}

//**************************************************************************************
void	WindowTabDeviceSelect::UpdateRemoteDeviceList(void)
{
int		iii;
int		foundIndex;
bool	domeListChanged;
bool	telescopeListChanged;
char	textString[128];
char	ipAddrStr[32];
int		boxId;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);

	domeListChanged			=	false;
	telescopeListChanged	=	false;

	for (iii=0; iii<gRemoteCnt; iii++)
	{
		//-----------------------------------------------------
		//*	check for DOME devices
		//*	first, see if this device is already in our list.
		if (gRemoteList[iii].deviceTypeEnum == kDeviceType_Dome)
		{
			foundIndex		=	FindDeviceInList(&gRemoteList[iii], cDeviceList_Domes, cDomeCnt);
			if (foundIndex >= 0)
			{
				//*	its already in the list
			}
			else
			{
				if (cDomeCnt < kSelectionCnt)
				{
					cDeviceList_Domes[cDomeCnt]				=	gRemoteList[iii];
					cDeviceList_Domes[cDomeCnt].validEntry	=	true;

					cDomeCnt++;
					domeListChanged	=	true;
				}
			}
		}
		//-----------------------------------------------------
		//*	check for TELESCOPE devices
		//*	first, see if this device is already in our list.
		if (gRemoteList[iii].deviceTypeEnum == kDeviceType_Telescope)
		{
			foundIndex		=	FindDeviceInList(&gRemoteList[iii], cDeviceList_Telescopes, cTelescopeCnt);
			if (foundIndex >= 0)
			{
				//*	its already in the list
			}
			else
			{
				if (cTelescopeCnt < kSelectionCnt)
				{
					cDeviceList_Telescopes[cTelescopeCnt]				=	gRemoteList[iii];
					cDeviceList_Telescopes[cTelescopeCnt].validEntry	=	true;

					cTelescopeCnt++;
					telescopeListChanged	=	true;
				}
			}
		}
	}

	if (domeListChanged)
	{
		//*	update the widget check box strings
		for (iii=0; iii<cDomeCnt; iii++)
		{
			inet_ntop(AF_INET, &(cDeviceList_Domes[iii].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
			sprintf(textString, "%s:%d/%d-%s",	ipAddrStr,
					cDeviceList_Domes[iii].port,
					cDeviceList_Domes[iii].alpacaDeviceNum,
				//	cDeviceList_Domes[iii].hostName,
					cDeviceList_Domes[iii].deviceNameStr);
			boxId	=	kDeviceSelect_Dome1 + iii;
			SetWidgetText(boxId, textString);
			SetWidgetValid(boxId, true);
		}
		for (iii=cDomeCnt; iii<kSelectionCnt; iii++)
		{
			boxId	=	kDeviceSelect_Dome1 + iii;
			SetWidgetValid(boxId, false);
		}
	}
	if (telescopeListChanged)
	{
		//*	update the widget check box strings
		for (iii=0; iii<cTelescopeCnt; iii++)
		{
			inet_ntop(AF_INET, &(cDeviceList_Telescopes[iii].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
			sprintf(textString, "%s:%d/%d-%s",	ipAddrStr,
					cDeviceList_Telescopes[iii].port,
					cDeviceList_Telescopes[iii].alpacaDeviceNum,
				//	cDeviceList_Telescopes[iii].hostName,
					cDeviceList_Telescopes[iii].deviceNameStr);
			boxId	=	kDeviceSelect_Telescope1 + iii;
			SetWidgetText(boxId, textString);
			SetWidgetValid(boxId, true);
		}
		for (iii=cTelescopeCnt; iii<kSelectionCnt; iii++)
		{
			boxId	=	kDeviceSelect_Telescope1 + iii;
			SetWidgetValid(boxId, false);
		}
	}
}



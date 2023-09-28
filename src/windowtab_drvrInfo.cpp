//*****************************************************************************
//*		windowtab_drvrInfo.cpp		(c) 2020 by Mark Sproul
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
//*	Feb  9,	2021	<MLS> Created windowtab_drvrInfo.cpp
//*	Jun  1,	2022	<MLS> Added "Launch Web" button
//*	Jun 18,	2023	<MLS> Added DeviceState to driver info
//*	Sep 26,	2023	<MLS> Added Restart to info window
//*****************************************************************************



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_drvrInfo.h"
#include	"controller.h"


#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabDriverInfo::WindowTabDriverInfo(	const int	xSize,
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
WindowTabDriverInfo::~WindowTabDriverInfo(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabDriverInfo::SetupWindowControls(void)
{
int		yLoc;
int		textBoxHt;
int		boxWidth;
int		boxID;
int		firstItemID;
int		lastItemID;
char	restartInfoText[]	=	"Restart will cause the device driver instance to be deleted and recreated, this should only be used if the driver stops working";

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kDriverInfo_Title, -1, yLoc, "Remote device information");

	yLoc	+=	2;
	yLoc	+=	2;


	textBoxHt	=	cTitleHeight * 2;
//	textBoxHt	+=	cTitleHeight / 2;
	boxWidth	=	135;
	boxWidth	=	cWidth - 6;
	boxID		=	kFirstBoxID;
	while (boxID <= kLastBoxID)
	{
		//*	setup the label
		firstItemID	=	boxID;
		SetWidget(				boxID,	0,			yLoc,		boxWidth,		cSmallBtnHt);
		SetWidgetType(			boxID,	kWidgetType_TextBox);
		SetWidgetJustification(	boxID,	kJustification_Center);
		SetWidgetFont(			boxID,	kFont_TextList);
		SetWidgetTextColor(		boxID,	CV_RGB(255,	0,	0));
		SetWidgetBorder(		boxID, false);
		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;

		boxID++;
		//*	setup the text area
		lastItemID	=	boxID;
		SetWidget(				boxID,	0,			yLoc,		boxWidth,		textBoxHt);
		SetWidgetType(			boxID,	kWidgetType_MultiLineText);
		SetWidgetJustification(	boxID,	kJustification_Left);
		SetWidgetFont(			boxID,	kFont_TextList);
		SetWidgetTextColor(		boxID,	CV_RGB(255,	255, 255));
		SetWidgetBorderColor(	boxID,	CV_RGB(255,	255,	0));
		SetWidgetBorder(		boxID, false);
		yLoc			+=	textBoxHt;
		yLoc			+=	2;

		boxID++;
		//*	setup the outline
		SetWidgetOutlineBox(boxID, firstItemID, lastItemID);
		yLoc			+=	2;
		yLoc			+=	2;

		boxID++;
	}

	SetWidgetText(	kDriverInfo_Name_Lbl,				"Name");
	SetWidgetText(	kDriverInfo_Description_Lbl,		"Description");
	SetWidgetText(	kDriverInfo_DriverInfo_Lbl,			"DriverInfo");
	SetWidgetText(	kDriverInfo_DriverVersion_Lbl,		"DriverVersion");
	SetWidgetText(	kDriverInfo_InterfaceVersion_Lbl,	"InterfaceVersion");


	SetWidget(				kDriverInfo_Restart_Info,	0,			yLoc,		boxWidth,		textBoxHt);
	SetWidgetType(			kDriverInfo_Restart_Info,	kWidgetType_MultiLineText);
	SetWidgetJustification(	kDriverInfo_Restart_Info,	kJustification_Left);
	SetWidgetFont(			kDriverInfo_Restart_Info,	kFont_TextList);
	SetWidgetTextColor(		kDriverInfo_Restart_Info,	CV_RGB(255,	255, 255));
	SetWidgetBorderColor(	kDriverInfo_Restart_Info,	CV_RGB(255,	255,	0));
	SetWidgetBorder(		kDriverInfo_Restart_Info, false);
	SetWidgetText(			kDriverInfo_Restart_Info,	restartInfoText);
	yLoc			+=	textBoxHt;
	yLoc			+=	2;

	SetWidget(				kDriverInfo_Restart_Button,	5,	yLoc,	150,	cBtnHeight);
	SetWidgetType(			kDriverInfo_Restart_Button,	kWidgetType_Button);
	SetWidgetText(			kDriverInfo_Restart_Button,	"Restart");
	SetWidgetTextColor(		kDriverInfo_Restart_Button,	CV_RGB(0,	0, 0));
	SetWidgetBGColor(		kDriverInfo_Restart_Button,	CV_RGB(255,	255, 255));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;
	//*	setup the outline
	SetWidgetOutlineBox(kDriverInfo_Restart_Outline, kDriverInfo_Restart_Info, (kDriverInfo_Restart_Outline - 1));
	yLoc			+=	2;


	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kDriverInfo_IPaddr,
							kDriverInfo_Readall,
							kDriverInfo_DeviceState,
							-1,
							-1,
							-1,
							-1);

	//---------------------------------------------------------
	//*	Now go back and adjust the width of the IP address box so we can fit the LAUNCH button
	//---------------------------------------------------------
	boxWidth	=	115;
	cWidgetList[kDriverInfo_IPaddr].width		-=	boxWidth;
	cWidgetList[kDriverInfo_IPaddr].width		-=	3;
	//*	web launch button
	SetWidget(	kDriverInfo_LaunchWeb,	(cWidth - boxWidth),
										(cHeight - cBtnHeight),
										boxWidth,
										(cBtnHeight-1));
	SetWidgetType(		kDriverInfo_LaunchWeb,	kWidgetType_Button);
	SetWidgetBGColor(	kDriverInfo_LaunchWeb,	CV_RGB(255,	255, 255));
	SetWidgetTextColor(	kDriverInfo_LaunchWeb,	CV_RGB(0,	0,	0));
	SetWidgetFont(		kDriverInfo_LaunchWeb,	kFont_TextList);
	SetWidgetText(		kDriverInfo_LaunchWeb,	"Launch Web");
}

//*****************************************************************************
void	WindowTabDriverInfo::ProcessButtonClick(const int buttonIdx, const int flags)
{
char		ipAddrStr[64];
char		dataString[256];
Controller	*myControllerPtr;
bool		validData;

	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kDriverInfo_Restart_Button:
			if (strlen(cAlpacaDeviceTypeStr) > 0)
			{
				validData	=	AlpacaSendPutCmd(	cAlpacaDeviceTypeStr, "restart",	NULL);
				if (validData == false)
				{
					CONSOLE_DEBUG("restart failed");
				}
			}
			else
			{
				CONSOLE_DEBUG("cAlpacaDeviceTypeStr not set");
			}
			break;

		case kDriverInfo_LaunchWeb:
			myControllerPtr	=   (Controller *)cParentObjPtr;
			if (myControllerPtr != NULL)
			{
				inet_ntop(AF_INET, &(myControllerPtr->cDeviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
				sprintf(dataString, "%s http://%s:%d/setup &",
										gWebBrowserCmdString,
										ipAddrStr,
										myControllerPtr->cPort);
				CONSOLE_DEBUG(dataString);
				RunCommandLine(dataString);
			}
			else
			{
				CONSOLE_DEBUG("myControllerPtr is NULL!!!!");
			}
			break;
	}
}

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
//	CONSOLE_DEBUG(__FUNCTION__);
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
//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kDriverInfo_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kDriverInfo_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kDriverInfo_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	SetWidget(		kDriverInfo_SubTitle,	0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetFont(	kDriverInfo_SubTitle,	kFont_Medium);
	SetWidgetText(	kDriverInfo_SubTitle,	"Remote device information");
	SetWidgetTextColor(kDriverInfo_SubTitle,	CV_RGB(255,	255,	0));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;


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

//	SetAlpacaLogo(kDriverInfo_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
//	SetIPaddressBoxes(kDriverInfo_IPaddr, kDriverInfo_Readall, kDriverInfo_AlpacaDrvrVersion, -1);
	SetIPaddressBoxes(kDriverInfo_IPaddr, kDriverInfo_Readall, -1, -1);

	//*	adjust the width of the IP address box
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

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
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
			break;
	}
}

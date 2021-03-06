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
WindowTabDriverInfo::~WindowTabDriverInfo(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabDriverInfo::SetupWindowControls(void)
{
int		yLoc;
int		textBoxHt;
int		labelX;
int		labelY;
int		labelWidth;
int		boxID;

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


	textBoxHt	=	cTitleHeight * 2;
	labelWidth	=	135;
	labelX		=	cWidth - labelWidth - 5;
	boxID		=	kFirstBoxID;
	while (boxID<=kLastBoxID)
	{
		SetWidget(				boxID,	0,			yLoc,		cWidth,		textBoxHt);
		SetWidgetType(			boxID,	kWidgetType_MultiLineText);
		SetWidgetJustification(	boxID,	kJustification_Left);
		SetWidgetFont(			boxID,	kFont_TextList);
		SetWidgetTextColor(		boxID,	CV_RGB(255,	255,	255));

		boxID++;
		labelY	=	yLoc + textBoxHt - cSmallBtnHt - 2;
		SetWidget(				boxID,	labelX,			labelY,		labelWidth,		cSmallBtnHt);
		SetWidgetType(			boxID,	kWidgetType_Text);
		SetWidgetJustification(	boxID,	kJustification_Left);
		SetWidgetFont(			boxID,	kFont_TextList);
		SetWidgetTextColor(		boxID,	CV_RGB(255,	0,	0));
		SetWidgetBorder(		boxID, false);



		yLoc			+=	textBoxHt;
		yLoc			+=	2;

		boxID++;
	}

	SetWidgetText(	kDriverInfo_Name_Lbl,				"Name");
	SetWidgetText(	kDriverInfo_Description_Lbl,		"Description");
	SetWidgetText(	kDriverInfo_DriverInfo_Lbl,			"DriverInfo");
	SetWidgetText(	kDriverInfo_DriverVersion_Lbl,		"DriverVersion");
	SetWidgetText(	kDriverInfo_InterfaceVersion_Lbl,	"InterfaceVersion");

	SetAlpacaLogo(kDriverInfo_AlpacaLogo, -1);

	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kDriverInfo_IPaddr, kDriverInfo_Readall, kDriverInfo_AlpacaDrvrVersion, -1);
}




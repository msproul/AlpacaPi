//*****************************************************************************
//*		windowtab_imageinfo.cpp		(c) 2023 by Mark Sproul
//*
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
//*	Feb 20,	2023	<MLS> Created windowtab_imageinfo.cpp
//*****************************************************************************

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"discoverythread.h"
#include	"sendrequest_lib.h"

#include	"windowtab.h"
#include	"windowtab_imageinfo.h"




//**************************************************************************************
WindowTabImageInfo::WindowTabImageInfo(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName,
										TYPE_BinaryImageHdr	*binaryImageHdr)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("xSize        \t=",	xSize);
	CONSOLE_DEBUG_W_NUM("ySize        \t=",	ySize);

	SetupWindowControls();
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabImageInfo::~WindowTabImageInfo(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}


//**************************************************************************************
void	WindowTabImageInfo::SetupWindowControls(void)
{
int			xLoc;
int			yLoc;
int			yLocTop;
int			iii;
int			textBoxWidth;
int			valueBoxWidth;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cWidth        \t=",	cWidth);
	//------------------------------------------
	xLoc			=	5;
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kImageInfo_Title, -1, yLoc, "AlpacaPi project");

	//------------------------------------------
	SetWidget(		kImageInfo_Description,		1,		yLoc,	cWidth-2,		cTitleHeight);
	SetWidgetType(	kImageInfo_Description, kWidgetType_TextBox);
	SetWidgetBorder(kImageInfo_Description,	true);
	SetWidgetText(	kImageInfo_Description, "Image details including download information");
	SetWidgetTextColor(kImageInfo_Description,		CV_RGB(255,255,255));

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	yLocTop			=	yLoc;	//*	save for 2nd column

	textBoxWidth	=	225;
	valueBoxWidth	=	100;
	CONSOLE_DEBUG("kImageInfo_AlpacaBinaryTitle");
	CONSOLE_DEBUG_W_NUM("xLoc\t=",	xLoc);
	CONSOLE_DEBUG_W_NUM("yLoc\t=",	yLoc);

	SetWidget(				kImageInfo_AlpacaBinaryTitle,	xLoc,	yLoc,	(textBoxWidth + valueBoxWidth),	cSmallBtnHt);
	SetWidgetType(			kImageInfo_AlpacaBinaryTitle, 	kWidgetType_TextBox);
	SetWidgetFont(			kImageInfo_AlpacaBinaryTitle, 	kFont_TextList);
	SetWidgetBorder(		kImageInfo_AlpacaBinaryTitle,	true);
	SetWidgetTextColor(		kImageInfo_AlpacaBinaryTitle,	CV_RGB(255,255,255));
	SetWidgetText(			kImageInfo_AlpacaBinaryTitle,	"Alpaca ImageBinary header");
	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;


	iii				= kImageInfo_MetadataVersion;
	while (iii <= kImageInfo_Dimension3)
	{
		SetWidget(				iii,	xLoc,	yLoc,	textBoxWidth,	cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Left);
		SetWidgetBorder(		iii,	true);
		SetWidgetTextColor(		iii,	CV_RGB(255,255,255));
		SetWidgetText(			iii,	"---------");
		iii++;

		SetWidget(				iii,	(xLoc + textBoxWidth + 2),	yLoc,	valueBoxWidth,	cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Center);
		SetWidgetTextColor(		iii,	CV_RGB(255,255,255));
		SetWidgetText(			iii,	"N/A");

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;

		iii++;
	}
	SetWidgetOutlineBox(kImageInfo_BinaryOutline, kImageInfo_AlpacaBinaryTitle, (kImageInfo_BinaryOutline - 1));


	SetWidgetText(kImageInfo_MetadataVersion,		"MetadataVersion");
	SetWidgetText(kImageInfo_ErrorNumber,			"ErrorNumber");
	SetWidgetText(kImageInfo_ClientTransactionID,	"ClientTransactionID");
	SetWidgetText(kImageInfo_ServerTransactionID,	"ServerTransactionID");
	SetWidgetText(kImageInfo_DataStart,				"DataStart");
	SetWidgetText(kImageInfo_ImageElementType,		"ImageElementType");
	SetWidgetText(kImageInfo_TransmissionElementType,	"TransmissionElementType");
	SetWidgetText(kImageInfo_Rank,					"Rank");
	SetWidgetText(kImageInfo_Dimension1,			"Dimension1");
	SetWidgetText(kImageInfo_Dimension2,			"Dimension2");
	SetWidgetText(kImageInfo_Dimension3,			"Dimension3");

	//=============================================================
	//*	this is the download stats info boxes
	xLoc	+=	(textBoxWidth + valueBoxWidth + 8);
	yLoc	=	yLocTop;
	SetWidget(				kImageInfo_DownLoadTitle,	xLoc,	yLoc,	(textBoxWidth + valueBoxWidth),	cSmallBtnHt);
	SetWidgetType(			kImageInfo_DownLoadTitle, 	kWidgetType_TextBox);
	SetWidgetFont(			kImageInfo_DownLoadTitle, 	kFont_TextList);
	SetWidgetBorder(		kImageInfo_DownLoadTitle,	true);
	SetWidgetTextColor(		kImageInfo_DownLoadTitle,	CV_RGB(255,255,255));
	SetWidgetText(			kImageInfo_DownLoadTitle,	"Download stats");
	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;

	iii		=	kImageInfo_DownLoadMBytes;
	while (iii < kImageInfo_DownLoadOutline)
	{
		SetWidget(				iii,	xLoc,	yLoc,	textBoxWidth,	cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Left);
		SetWidgetBorder(		iii,	true);
		SetWidgetTextColor(		iii,	CV_RGB(255,255,255));
		SetWidgetText(			iii,	"---------");
		iii++;

		SetWidget(				iii,	(xLoc + textBoxWidth + 2),	yLoc,	valueBoxWidth,	cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Center);
		SetWidgetTextColor(		iii,	CV_RGB(255,255,255));
		SetWidgetText(			iii,	"N/A");

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;

		iii++;
	}
	SetWidgetOutlineBox(kImageInfo_DownLoadOutline, kImageInfo_DownLoadTitle, (kImageInfo_DownLoadOutline - 1));

	SetWidgetText(kImageInfo_DownLoadMBytes,	"Received Data (mBytes)");
	SetWidgetText(kImageInfo_DownLoadSeconds,	"Duration (seconds)");
	SetWidgetText(kImageInfo_DownLoadSpeed,		"Speed (mBytes / sec)");
}




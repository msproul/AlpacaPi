//*****************************************************************************
//*		windowtab_RemoteData.cpp		(c) 2021 by Mark Sproul
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Nov 14,	2021	<MLS> Created windowtab_RemoteData.cpp
//*****************************************************************************


#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"alpaca_defs.h"
#include	"windowtab.h"
#include	"windowtab_RemoteData.h"
#include	"controller.h"
#include	"controller_skytravel.h"

#include	"RemoteImage.h"


//**************************************************************************************
WindowTabRemoteData::WindowTabRemoteData(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cLastRemoteImageUpdate_ms	=	0;

	SetupWindowControls();

	UpdateSettings();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabRemoteData::~WindowTabRemoteData(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabRemoteData::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		valueWitdth1;
int		valueWitdth2;
int		valueWitdth3;
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kRemoteData_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kRemoteData_Title, "SkyTravel settings");
	SetBGcolorFromWindowName(kRemoteData_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;


	//----------------------------------------------------------------------
	xLoc			=	5;
	valueWitdth1	=	160;
	valueWitdth2	=	75;
	valueWitdth3	=	cWidth - valueWitdth1 - valueWitdth2 -xLoc - 15;

	iii	=	kRemoteData_stsci_fits_checkbox;
	while (iii < kRemoteData_ImageOptOutline)
	{
		xLoc		=	5;
		SetWidget(		iii,	xLoc,			yLoc,		valueWitdth1,		cTitleHeight);
		SetWidgetType(	iii,	kWidgetType_RadioButton);
		SetWidgetFont(	iii,	kFont_Medium);
		iii++;
		xLoc		+=	valueWitdth1;
		xLoc		+=	3;

		//*	the count box
		SetWidget(			iii,	xLoc,			yLoc,	valueWitdth2,		cTitleHeight);
		SetWidgetFont(		iii,	kFont_Medium);
		iii++;
		xLoc		+=	valueWitdth2;
		xLoc		+=	3;

		//*	last command box
		SetWidget(				iii,	xLoc,			yLoc,	valueWitdth3,		cTitleHeight);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetJustification(	iii,	kJustification_Left);
		iii++;
		//xLoc		+=	valueWitdth3;
		//xLoc		+=	3;

		yLoc			+=	cTitleHeight;
		yLoc			+=	2;
	}
	SetWidgetOutlineBox(kRemoteData_ImageOptOutline, kRemoteData_stsci_fits_checkbox, (kRemoteData_ImageOptOutline -1));

	SetWidgetText(		kRemoteData_stsci_fits_checkbox,		"stsci.edu (fits)");
	SetWidgetText(		kRemoteData_stsci_gif_checkbox,	"stsci.edu (gif)");
	SetWidgetText(		kRemoteData_SDSS_checkbox,		"SDSS data");

	xLoc		=	5;


	SetWidget(				kRemoteData_StatusMsg,	xLoc,	yLoc,	550,		cTitleHeight);
	SetWidgetType(			kRemoteData_StatusMsg,	kWidgetType_Text);
	SetWidgetFont(			kRemoteData_StatusMsg,	kFont_TextList);
	SetWidgetJustification(	kRemoteData_StatusMsg,	kJustification_Left);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;




	SetAlpacaLogoBottomCorner(kSkyT_Settings_AlpacaLogo);

}

//**************************************************************************************
//*	gets called when the window tab changes
//**************************************************************************************
void	WindowTabRemoteData::ActivateWindow(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	UpdateSettings();
}

//**************************************************************************************
void WindowTabRemoteData::RunBackgroundTasks(void)
{
uint32_t			currentMilliSecs;
uint32_t			deltaMilliSecs;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	currentMilliSecs	=	millis();

	//-----------------------------------------------------------
	//*	check on remote imaging
	deltaMilliSecs		=	currentMilliSecs - cLastRemoteImageUpdate_ms;
	if (deltaMilliSecs >= 2000)
	{
		UpdateSettings();
		cLastRemoteImageUpdate_ms		=	millis();
	}
}

//*****************************************************************************
void	WindowTabRemoteData::ProcessButtonClick(const int buttonIdx)
{

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);

	switch(buttonIdx)
	{
		case kRemoteData_stsci_fits_checkbox:
			gRemoteSourceID	=	kRemoteSrc_stsci_fits;
			break;

		case kRemoteData_stsci_gif_checkbox:
			gRemoteSourceID	=	kRemoteSrc_stsci_gif;
			break;

		case kRemoteData_SDSS_checkbox:
			gRemoteSourceID	=	kRemoteSrc_SDSS;
			break;
	}
	UpdateSettings();
}

//*****************************************************************************
void	WindowTabRemoteData::ProcessDoubleClick(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{

//	switch(widgetIdx)
//	{
//
//	}
}

//*****************************************************************************
//*	update the various settings check boxes etc
//*****************************************************************************
void	WindowTabRemoteData::UpdateSettings(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetText(kRemoteData_StatusMsg,	gRemoteImageStatusMsg);

	SetWidgetChecked(	kRemoteData_stsci_fits_checkbox,	(gRemoteSourceID == kRemoteSrc_stsci_fits));
	SetWidgetChecked(	kRemoteData_stsci_gif_checkbox,		(gRemoteSourceID == kRemoteSrc_stsci_gif));
	SetWidgetChecked(	kRemoteData_SDSS_checkbox,			(gRemoteSourceID == kRemoteSrc_SDSS));

	SetWidgetNumber(kRemoteData_stsci_fits_Count,	gRemoteDataStats[kRemoteSrc_stsci_fits].RequestCount);
	SetWidgetNumber(kRemoteData_stsci_gif_Count,	gRemoteDataStats[kRemoteSrc_stsci_gif].RequestCount);
	SetWidgetNumber(kRemoteData_SDSS_Count,			gRemoteDataStats[kRemoteSrc_SDSS].RequestCount);

	SetWidgetText(kRemoteData_stsci_fits_LastCmd,	gRemoteDataStats[kRemoteSrc_stsci_fits].LastCmdString);
	SetWidgetText(kRemoteData_stsci_gif_LastCmd,	gRemoteDataStats[kRemoteSrc_stsci_gif].LastCmdString);
	SetWidgetText(kRemoteData_SDSS_LastCmd,			gRemoteDataStats[kRemoteSrc_SDSS].LastCmdString);


	ForceUpdate();
}



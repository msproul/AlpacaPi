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
//*	Jan  6,	2022	<MLS> Started working on Remote GAIA sql server interface
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

#ifdef 	_ENABLE_REMOTE_GAIA_
	#include	"RemoteGaia.h"
	#include	"controller_GaiaRemote.h"
#endif




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
int		xLoc2;
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


#ifdef 	_ENABLE_REMOTE_GAIA_
	yLoc			+=	20;
	valueWitdth1	=	200;
	valueWitdth2	=	350;
	xLoc2			=	xLoc + valueWitdth1 + 2;
	SetWidget(		kRemoteData_EnableRemoteGAIA,	xLoc,	yLoc,	(valueWitdth1 + valueWitdth2),		cSmallBtnHt);
	SetWidgetType(	kRemoteData_EnableRemoteGAIA,	kWidgetType_CheckBox);
	SetWidgetFont(	kRemoteData_EnableRemoteGAIA,	kFont_Medium);
	SetWidgetText(	kRemoteData_EnableRemoteGAIA,	"Enable Remote GAIA SQL Server");
	SetWidgetBorder(kRemoteData_EnableRemoteGAIA,	false);

	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;

	iii	=	kRemoteData_SQLserverTxt;
	while (iii < kRemoteData_OpenSQLWindowBtn)
	{

		SetWidget(				iii,	xLoc,	yLoc,	valueWitdth1,		cSmallBtnHt);
		SetWidgetType(			iii,	kWidgetType_Text);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetJustification(	iii,	kJustification_Left);
		iii++;

		SetWidget(				iii,	xLoc2,	yLoc,	valueWitdth2,		cSmallBtnHt);
		SetWidgetType(			iii,	kWidgetType_Text);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetJustification(	iii,	kJustification_Left);
		iii++;

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(	kRemoteData_SQLserverTxt,		"SQL Server");
	SetWidgetText(	kRemoteData_SQLportTxt,			"Port");
	SetWidgetText(	kRemoteData_SQLusernameTxt,		"username");

	SetWidgetText(		kRemoteData_SQLserverValue,		gGaiaSQLsever_IPaddr);
	SetWidgetNumber(	kRemoteData_SQLportValue,		gGaiaSQLsever_Port);
	SetWidgetText(		kRemoteData_SQLusernameValue,	gGaiaSQLsever_UserName);


	SetWidget(				kRemoteData_OpenSQLWindowBtn,	xLoc,	yLoc,	(valueWitdth1 + valueWitdth2),		cSmallBtnHt);
	SetWidgetType(			kRemoteData_OpenSQLWindowBtn,	kWidgetType_Button);
	SetWidgetJustification(	kRemoteData_OpenSQLWindowBtn,	kJustification_Center);
	SetWidgetFont(			kRemoteData_OpenSQLWindowBtn,	kFont_Medium);
	SetWidgetText(			kRemoteData_OpenSQLWindowBtn,	"Open SQL Request List Window");
	SetWidgetBGColor(		kRemoteData_OpenSQLWindowBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(		kRemoteData_OpenSQLWindowBtn,	CV_RGB(0,	0,	0));


	SetWidgetOutlineBox(kRemoteData_GAIAoutline, kRemoteData_EnableRemoteGAIA, (kRemoteData_GAIAoutline -1));


#endif // _ENABLE_REMOTE_GAIA_



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

	#ifdef 	_ENABLE_REMOTE_GAIA_
		case kRemoteData_EnableRemoteGAIA:
			gST_DispOptions.RemoteGAIAenabled	=	!gST_DispOptions.RemoteGAIAenabled;
			break;

		case kRemoteData_OpenSQLWindowBtn:
			CreateGaiaRemoteListWindow();
			break;
	#endif // _ENABLE_REMOTE_GAIA_
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

	SetWidgetNumber(kRemoteData_stsci_fits_Count,		gRemoteDataStats[kRemoteSrc_stsci_fits].RequestCount);
	SetWidgetNumber(kRemoteData_stsci_gif_Count,		gRemoteDataStats[kRemoteSrc_stsci_gif].RequestCount);
	SetWidgetNumber(kRemoteData_SDSS_Count,				gRemoteDataStats[kRemoteSrc_SDSS].RequestCount);

	SetWidgetText(kRemoteData_stsci_fits_LastCmd,		gRemoteDataStats[kRemoteSrc_stsci_fits].LastCmdString);
	SetWidgetText(kRemoteData_stsci_gif_LastCmd,		gRemoteDataStats[kRemoteSrc_stsci_gif].LastCmdString);
	SetWidgetText(kRemoteData_SDSS_LastCmd,				gRemoteDataStats[kRemoteSrc_SDSS].LastCmdString);

#ifdef 	_ENABLE_REMOTE_GAIA_
	SetWidgetChecked(	kRemoteData_EnableRemoteGAIA,	gST_DispOptions.RemoteGAIAenabled);
#endif

	ForceUpdate();
}



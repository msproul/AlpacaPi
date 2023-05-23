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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Nov 14,	2021	<MLS> Created windowtab_RemoteData.cpp
//*	Jan  6,	2022	<MLS> Started working on Remote GAIA sql server interface
//*	Jan 19,	2022	<MLS> Added SQL logging enable/disable
//*	Feb 10,	2022	<MLS> Added 1x1, 3x1, and 3x3 gaia request modes
//*	Mar 13,	2022	<MLS> Added radio buttons for different SQL databases
//*	Sep 22,	2022	<MLS> Added support for remote SQL database list
//*	Sep 22,	2022	<MLS> Added UpdateDataBaseButtons()
//*	Sep 22,	2022	<MLS> Added UpdateSelectedDataBase()
//*	Sep 23,	2022	<MLS> After database list update, default db is set to "gaia"
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
	#include	"GaiaSQL.h"
	#include	"RemoteGaia.h"
	#include	"controller_GaiaRemote.h"
#endif




//**************************************************************************************
WindowTabRemoteData::WindowTabRemoteData(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("RemoteGAIAenabled is", (gST_DispOptions.RemoteGAIAenabled ? "enabled" : "disabled"));

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

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kRemoteData_Title, -1, yLoc, "Remote Data Options");

	yLoc	+=	2;
	yLoc	+=	2;

	//----------------------------------------------------------------------
	xLoc	=	5;

	SetWidget(			kRemoteData_ControlDblClkTitle,	xLoc,	yLoc,	(cWidth - 15),		cTitleHeight);
	SetWidgetText(		kRemoteData_ControlDblClkTitle, "Ctrl-DoubleClick image servers");
	SetWidgetBorder(	kRemoteData_ControlDblClkTitle, false);
	SetWidgetTextColor(	kRemoteData_ControlDblClkTitle,		CV_RGB(0, 0xff, 0));
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

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
		SetWidgetBorder(iii,	true);
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
	SetWidgetOutlineBox(kRemoteData_ImageOptOutline, kRemoteData_ControlDblClkTitle, (kRemoteData_ImageOptOutline -1));

	SetWidgetText(		kRemoteData_stsci_fits_checkbox,	"stsci.edu (fits)");
	SetWidgetText(		kRemoteData_stsci_gif_checkbox,		"stsci.edu (gif)");
	SetWidgetText(		kRemoteData_SDSS_checkbox,			"SDSS data");

	xLoc		=	5;


	SetWidget(				kRemoteData_StatusMsg,	(xLoc - 2),	yLoc,	(cWidth - 10),		cTitleHeight);
	SetWidgetType(			kRemoteData_StatusMsg,	kWidgetType_TextBox);
	SetWidgetFont(			kRemoteData_StatusMsg,	kFont_TextList);
	SetWidgetJustification(	kRemoteData_StatusMsg,	kJustification_Left);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;


#ifdef 	_ENABLE_REMOTE_GAIA_
int		xLoc2;
int		yLoc2;
int		databaseWidth;
int		valueWitdth4;
int		fullBoxWidth;
char	helpMsg[128];
#define		kFullBoxWidth	600


	yLoc			+=	20;
	valueWitdth1	=	150;
	valueWitdth2	=	kFullBoxWidth - valueWitdth1;
	fullBoxWidth	=	valueWitdth1 + valueWitdth2;
	valueWitdth4	=	(fullBoxWidth / 4) - 3;
	databaseWidth	=	(fullBoxWidth / 4) - 3;
	xLoc2			=	xLoc + valueWitdth1 + 2;


	SetWidget(			kRemoteData_RemoteDataTitle,	xLoc,	yLoc,	(fullBoxWidth + databaseWidth),		cBtnHeight);
	SetWidgetType(		kRemoteData_RemoteDataTitle,	kWidgetType_TextBox);
	SetWidgetFont(		kRemoteData_RemoteDataTitle,	kFont_Large);
	SetWidgetText(		kRemoteData_RemoteDataTitle,	"Remote star databases via SQL Server");
	SetWidgetTextColor(	kRemoteData_RemoteDataTitle,	CV_RGB(0, 0xff, 0));
	SetWidgetBorder(	kRemoteData_RemoteDataTitle,	false);
	yLoc	+=	cBtnHeight;
	yLoc	+=	2;
	yLoc2	=	yLoc;


	SetWidget(		kRemoteData_EnableRemoteGAIA,	xLoc,	yLoc,	fullBoxWidth,		cSmallBtnHt);
	SetWidgetType(	kRemoteData_EnableRemoteGAIA,	kWidgetType_CheckBox);
	SetWidgetFont(	kRemoteData_EnableRemoteGAIA,	kFont_Medium);
	SetWidgetText(	kRemoteData_EnableRemoteGAIA,	"Enable Remote SQL Server Requests");
	SetWidgetBorder(kRemoteData_EnableRemoteGAIA,	false);

	yLoc	+=	cSmallBtnHt;
	yLoc	+=	2;

	SetWidget(			kRemoteData_EditSQLSettingsBtn,	xLoc,	yLoc,	fullBoxWidth + 2,		cBtnHeight);
	SetWidgetType(		kRemoteData_EditSQLSettingsBtn,	kWidgetType_Button);
	SetWidgetFont(		kRemoteData_EditSQLSettingsBtn,	kFont_Medium);
	SetWidgetBGColor(	kRemoteData_EditSQLSettingsBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kRemoteData_EditSQLSettingsBtn,	CV_RGB(0,	0,	0));
	sprintf(helpMsg, "Edit %s to change these settings", kSQLserverConfigFile);
	SetWidgetText(	kRemoteData_EditSQLSettingsBtn,	helpMsg);
	yLoc	+=	cBtnHeight;
	yLoc	+=	2;

	iii		=	kRemoteData_SQLserverTxt;

	while (iii < kRemoteData_EnableSQLlogging)
	{
		SetWidget(				iii,	xLoc,	yLoc,	valueWitdth1,		cSmallBtnHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetJustification(	iii,	kJustification_Left);
		iii++;

		SetWidget(				iii,	xLoc2,	yLoc,	valueWitdth2,		cSmallBtnHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetJustification(	iii,	kJustification_Left);
		iii++;

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(		kRemoteData_SQLserverTxt,		"SQL Server");
	SetWidgetText(		kRemoteData_SQLportTxt,			"Port");
	SetWidgetText(		kRemoteData_SQLusernameTxt,		"username");

	SetWidgetText(		kRemoteData_SQLserverValue,		gSQLsever_IPaddr);
	SetWidgetNumber(	kRemoteData_SQLportValue,		gSQLsever_Port);
	SetWidgetText(		kRemoteData_SQLusernameValue,	gSQLsever_UserName);

	//----------------------------------------------------------------------
	SetWidget(			kRemoteData_EnableSQLlogging,	xLoc,	yLoc,	fullBoxWidth,		cSmallBtnHt);
	SetWidgetType(		kRemoteData_EnableSQLlogging,	kWidgetType_CheckBox);
	SetWidgetFont(		kRemoteData_EnableSQLlogging,	kFont_Medium);
	SetWidgetText(		kRemoteData_EnableSQLlogging,	"Enable SQL Logging");
	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;

	//----------------------------------------------------------------------
	iii				=	kRemoteData_GaiaReqMode;
	xLoc2			=	8;
	while (iii <= kRemoteData_GaiaReqMode3x3)
	{

		SetWidget(				iii,	xLoc2,	yLoc,	databaseWidth,		cSmallBtnHt);
		SetWidgetType(			iii,	kWidgetType_RadioButton);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetJustification(	iii,	kJustification_Left);
		iii++;

		xLoc2			+=	valueWitdth4;
		xLoc2			+=	2;
	}
	SetWidgetType(			kRemoteData_GaiaReqMode,	kWidgetType_TextBox);
	SetWidgetText(			kRemoteData_GaiaReqMode,	"Request Mode");
	SetWidgetText(			kRemoteData_GaiaReqMode1x1,	"1 x 1");
	SetWidgetText(			kRemoteData_GaiaReqMode3x1,	"3 x 1");
	SetWidgetText(			kRemoteData_GaiaReqMode3x3,	"3 x 3");

	//*	request mode outline
	SetWidgetOutlineBox(kRemoteData_GaiaReqModeOutLine, kRemoteData_GaiaReqMode, (kRemoteData_GaiaReqModeOutLine -1));

	yLoc			+=	cSmallBtnHt;
	yLoc			+=	2;

	//----------------------------------------------------------------------
	SetWidget(				kRemoteData_OpenSQLWindowBtn,	xLoc,	yLoc,	fullBoxWidth,		cBtnHeight);
	SetWidgetType(			kRemoteData_OpenSQLWindowBtn,	kWidgetType_Button);
	SetWidgetJustification(	kRemoteData_OpenSQLWindowBtn,	kJustification_Center);
	SetWidgetFont(			kRemoteData_OpenSQLWindowBtn,	kFont_Medium);
	SetWidgetText(			kRemoteData_OpenSQLWindowBtn,	"Open SQL Request List Window");
	SetWidgetBGColor(		kRemoteData_OpenSQLWindowBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(		kRemoteData_OpenSQLWindowBtn,	CV_RGB(0,	0,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	valueWitdth1	=	2 * fullBoxWidth / 3;
	valueWitdth2	=	fullBoxWidth / 3;
	xLoc2			=	xLoc + valueWitdth1 + 2;
	SetWidget(			kRemoteData_GaiaSearchField,	xLoc,	yLoc,	valueWitdth1,		cBtnHeight);
	SetWidgetFont(		kRemoteData_GaiaSearchField,	kFont_Medium);
	SetWidgetType(		kRemoteData_GaiaSearchField,	kWidgetType_TextInput);
	SetWidgetBGColor(	kRemoteData_GaiaSearchField,	CV_RGB(128,	128,	128));
//	SetWidgetText(		kRemoteData_GaiaSearchField,	"4098214367441486592");
//	SetWidgetText(		kRemoteData_GaiaSearchField,	"133768513079427328");
	SetWidgetText(		kRemoteData_GaiaSearchField,	"2093214988170429184");

	SetWidget(			kRemoteData_GaiaSearchBtn,	xLoc2,	yLoc,	valueWitdth2,		cBtnHeight);
	SetWidgetType(		kRemoteData_GaiaSearchBtn,	kWidgetType_Button);
	SetWidgetText(		kRemoteData_GaiaSearchBtn,	"SQL Search");
	SetWidgetBGColor(	kRemoteData_GaiaSearchBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kRemoteData_GaiaSearchBtn,	CV_RGB(0,	0,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//--------------------------------------------
	SetWidget(			kRemoteData_UpdateDBlistBtn,	xLoc,	yLoc,	fullBoxWidth,		cBtnHeight);
	SetWidgetType(		kRemoteData_UpdateDBlistBtn,	kWidgetType_Button);
	SetWidgetText(		kRemoteData_UpdateDBlistBtn,	"Update database list from server");
	SetWidgetBGColor(	kRemoteData_UpdateDBlistBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kRemoteData_UpdateDBlistBtn,	CV_RGB(0,	0,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;


	//--------------------------------------------
	SetWidget(			kRemoteData_SQLerrorMsgBox,		xLoc,	yLoc,	fullBoxWidth,		cBtnHeight);
	SetWidgetType(		kRemoteData_SQLerrorMsgBox,		kWidgetType_Button);
	SetWidgetText(		kRemoteData_SQLerrorMsgBox,		"SQL Errors");
	SetWidgetFont(		kRemoteData_SQLerrorMsgBox,		kFont_Medium);
	SetWidgetTextColor(	kRemoteData_SQLerrorMsgBox,		CV_RGB(255,	0,	0));
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//----------------------------------------------------------------------
	//*	now deal with the data base entries
	xLoc2			=	cClm4_offset + 20;
	valueWitdth4	=	cClmWidth - 10;
	iii				=	kRemoteData_SQLdatabaseTxt;
	while (iii <= kRemoteData_SQLdatabase10)
	{
		SetWidget(				iii,	xLoc2,	yLoc2,	valueWitdth4,		cSmallBtnHt);
		SetWidgetType(			iii,	kWidgetType_RadioButton);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetText(			iii,	"--");
//		SetWidgetBorder(		iii,	true);


		iii++;

		yLoc2			+=	cSmallBtnHt;
		yLoc2			+=	2;
	}
	SetWidgetType(			kRemoteData_SQLdatabaseTxt,	kWidgetType_TextBox);
	SetWidgetText(			kRemoteData_SQLdatabaseTxt,	"database:");

	//*	now go through and set the names of the valid databases
	UpdateDataBaseButtons();

	SetWidgetChecked(kRemoteData_SQLdatabase1,	true);
	SetWidgetOutlineBox(kRemoteData_DBoutLine, kRemoteData_SQLdatabaseTxt, (kRemoteData_DBoutLine - 1));


	SetWidgetOutlineBox(kRemoteData_GAIAoutline, kRemoteData_RemoteDataTitle, (kRemoteData_GAIAoutline - 1));
#endif // _ENABLE_REMOTE_GAIA_

	SetAlpacaLogoBottomCorner(kSkyT_Settings_AlpacaLogo);
}

#ifdef 	_ENABLE_REMOTE_GAIA_
//**************************************************************************************
void	WindowTabRemoteData::UpdateDataBaseButtons(void)
{
int		dbNumber;
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);
	//*	now go through and set the names of the valid databases
	dbNumber		=	0;
	iii				=	kRemoteData_SQLdatabase1;
	while (iii <= kRemoteData_SQLdatabase10)
	{
		if (dbNumber < gDataBaseNameCnt)
		{
			SetWidgetText(iii,	gDataBaseNames[dbNumber].Name);
			SetWidgetValid(iii, true);
			dbNumber++;
		}
		else
		{
			SetWidgetValid(iii, false);
		}
		iii++;
	}
}

//**************************************************************************************
//*	specify -1 to select the default database
//**************************************************************************************
void	WindowTabRemoteData::UpdateSelectedDataBase(const int newDBnumber)
{
int		iii;
int		myDBnumber;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("newDBnumber\t", newDBnumber);

	if (newDBnumber >= 0)
	{
		myDBnumber	=	newDBnumber;
	}
	else
	{
		//*	default to the "gaia" database
		myDBnumber	=	0;
		CONSOLE_DEBUG_W_NUM("gDataBaseNameCnt", gDataBaseNameCnt);
		for (iii=0; iii<gDataBaseNameCnt; iii++)
		{
			if (strcasecmp("gaia", gDataBaseNames[iii].Name) == 0)
			{
				myDBnumber	=	iii;
				CONSOLE_DEBUG_W_NUM("Found GAIA at index", myDBnumber);
				break;
			}
		}
	}
	//*	clear the existing check mark
	for (iii=kRemoteData_SQLdatabase1;  iii<=kRemoteData_SQLdatabase10; iii++)
	{
		SetWidgetChecked(iii,	false);
	}
	CONSOLE_DEBUG_W_NUM("myDBnumber\t=", myDBnumber);
	if (myDBnumber < kMaxDataBaseNames)
	{
		//*	update the new database string
		strcpy(gSQLsever_Database, gDataBaseNames[myDBnumber].Name);
		//*	set the appropriate check mark
		SetWidgetChecked((kRemoteData_SQLdatabase1 + myDBnumber),	true);
	}
}
#endif // _ENABLE_REMOTE_GAIA_



//**************************************************************************************
//*	gets called when the window tab changes
//**************************************************************************************
void	WindowTabRemoteData::ActivateWindow(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	UpdateSettings();
}

//**************************************************************************************
void WindowTabRemoteData::RunWindowBackgroundTasks(void)
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
void	WindowTabRemoteData::ProcessButtonClick(const int buttonIdx, const int flags)
{
#ifdef 	_ENABLE_REMOTE_GAIA_
	char				searchText[64];
	TYPE_CelestData		sqlStarData;
	bool				validGaiaData;
	int					dbNumber;
	int					databaseCnt;
	char				errorMessage[256];
	char				fullErrMsg[256];

	SetWidgetText(kRemoteData_SQLerrorMsgBox, "");
#endif

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
			if (gST_DispOptions.RemoteGAIAenabled)
			{
				//*	this will start the thread if it is not already running
				StartGaiaSQLthread();
			}
			break;

		case kRemoteData_EditSQLSettingsBtn:
			EditTextFile(kSQLserverConfigFile);
			break;

		case kRemoteData_EnableSQLlogging:
			gEnableSQLlogging	=	!gEnableSQLlogging;
			break;

		case kRemoteData_SQLdatabase1:
		case kRemoteData_SQLdatabase2:
		case kRemoteData_SQLdatabase3:
		case kRemoteData_SQLdatabase4:
		case kRemoteData_SQLdatabase5:
		case kRemoteData_SQLdatabase6:
		case kRemoteData_SQLdatabase7:
		case kRemoteData_SQLdatabase8:
		case kRemoteData_SQLdatabase9:
		case kRemoteData_SQLdatabase10:
			dbNumber	=	buttonIdx - kRemoteData_SQLdatabase1;
			if ((dbNumber >= 0) && (dbNumber < gDataBaseNameCnt))
			{
				UpdateSelectedDataBase(dbNumber);

				SetWidgetChecked(buttonIdx,	true);
			}
			break;

		case kRemoteData_GaiaReqMode1x1:
			gST_DispOptions.GaiaRequestMode	=	kGaiaRequestMode_1x1;
			break;

		case kRemoteData_GaiaReqMode3x1:
			gST_DispOptions.GaiaRequestMode	=	kGaiaRequestMode_3x1;
			break;

		case kRemoteData_GaiaReqMode3x3:
			gST_DispOptions.GaiaRequestMode	=	kGaiaRequestMode_3x3;
			break;

		case kRemoteData_OpenSQLWindowBtn:
			CreateGaiaRemoteListWindow();
			break;

		case kRemoteData_GaiaSearchBtn:
			GetWidgetText(kRemoteData_GaiaSearchField, searchText);
			CONSOLE_DEBUG_W_STR("Searching GAIA for", searchText);
			validGaiaData	=	GetSQLdataFromIDnumber(searchText, &sqlStarData, errorMessage);
			if (validGaiaData)
			{
				Center_CelestralObject(&sqlStarData);
				strcpy(gRemoteImageStatusMsg, "");
			}
			else
			{
				CONSOLE_DEBUG("Gaia object not found!!!!!");
				CONSOLE_DEBUG(errorMessage);
//				strcpy(gRemoteImageStatusMsg, errorMessage);
				strcpy(fullErrMsg, "Failed to find object:");
				strcat(fullErrMsg, errorMessage);
				SetWidgetText(kRemoteData_SQLerrorMsgBox, fullErrMsg);
			}
			break;

		case kRemoteData_UpdateDBlistBtn:
			databaseCnt	=	UpdateDataBaseListFromServer(errorMessage);
			if (databaseCnt > 0)
			{
				UpdateDataBaseButtons();
				UpdateSelectedDataBase(-1);	//*	set to default
			}
			else
			{
				SetWidgetText(kRemoteData_SQLerrorMsgBox, errorMessage);
			}
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
	SetWidgetChecked(	kRemoteData_GaiaReqMode1x1, 	(gST_DispOptions.GaiaRequestMode == kGaiaRequestMode_1x1));
	SetWidgetChecked(	kRemoteData_GaiaReqMode3x1, 	(gST_DispOptions.GaiaRequestMode == kGaiaRequestMode_3x1));
	SetWidgetChecked(	kRemoteData_GaiaReqMode3x3,		(gST_DispOptions.GaiaRequestMode == kGaiaRequestMode_3x3));


	SetWidgetChecked(	kRemoteData_EnableRemoteGAIA,	gST_DispOptions.RemoteGAIAenabled);
	SetWidgetChecked(	kRemoteData_EnableSQLlogging, 	gEnableSQLlogging);


#endif

	ForceWindowUpdate();
}



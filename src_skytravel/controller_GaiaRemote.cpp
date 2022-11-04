//*****************************************************************************
//*		controller_GaiaRemote.cpp		(c) 2022 by Mark Sproul
//*
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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  9,	2022	<MLS> Created controller_GaiaRemote.cpp
//*****************************************************************************
#ifdef _ENABLE_REMOTE_SQL_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#define	kWindowWidth	999
#define	kWindowHeight	700

#include	"windowtab_about.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"controller_GaiaRemote.h"

//**************************************************************************************
enum
{
	kTab_Gaia_list	=	1,
	kTab_About,

	kTab_Count

};



//**************************************************************************************
void	CreateGaiaRemoteListWindow(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	new ControllerGaiaRemote();
}


//**************************************************************************************
ControllerGaiaRemote::ControllerGaiaRemote(void)
	:Controller("Gaia Remote", kWindowWidth,  kWindowHeight)
{
	CONSOLE_DEBUG(__FUNCTION__);

//+	cGaiaListTabObjPtr		=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	SetupWindowControls();

//	cLastUpdate_milliSecs	=	millis();

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerGaiaRemote::~ControllerGaiaRemote(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//+	DELETE_OBJ_IF_VALID(cGaiaListTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerGaiaRemote::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Gaia_list,		"Gaia Remote List");
	SetTabText(kTab_About,			"About");

	//--------------------------------------------
	cGaiaListTabObjPtr	=	new WindowTabGaiaRemote(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cGaiaListTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Gaia_list,	cGaiaListTabObjPtr);
		cGaiaListTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

}

//**************************************************************************************
void	ControllerGaiaRemote::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
long	delteaMillSecs;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cReadStartup)
	{
		CONSOLE_DEBUG_W_STR("cReadStartup", cWindowName);

		//*	so the window shows up
		HandleWindowUpdate();
		cv::waitKey(60);



		cReadStartup	=	false;
	}

	delteaMillSecs	=	millis() - cLastUpdate_milliSecs;
	if (delteaMillSecs > 2000)
	{
		//*	do what needs to be done here
		if (cGaiaListTabObjPtr != NULL)
		{
			cGaiaListTabObjPtr->UpdateOnScreenWidgetList();
		}

		cLastUpdate_milliSecs	=	millis();
	}
}

//*****************************************************************************
bool	ControllerGaiaRemote::AlpacaGetStartupData(void)
{
bool			validData;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	validData	=	true;
	return(validData);
}

//*****************************************************************************
void	ControllerGaiaRemote::UpdateCommonProperties(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_Name,				cCommonProp.Name);

}

#endif // _ENABLE_REMOTE_SQL_

//*****************************************************************************
//*		controller_startup.cpp		(c) 2023 by Mark Sproul
//*
//*	Description:
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
//*	May 21,	2023	<MLS> Created controller_startup.cpp
//*	May 23,	2023	<MLS> Added SetStartupTextStatus()
//*	Jun 14,	2023	<MLS> Fixed bug when startup box is closed
//*	Jun 23,	2023	<MLS> Added SetStartupUpdate()
//*	Jun 25,	2023	<MLS> Added RunBackgroundTasks() to startup for auto refresh and auto close
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_startup.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_startup.h"

#define	kWindowWidth	450
#define	kWindowHeight	650

ControllerStartup	*gStartupController	=	NULL;

//**************************************************************************************
enum
{
	kTab_Startup	=	1,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
int	SetStartupText(const char *startupMsg)
{
int		widgetIdx;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, startupMsg);
	widgetIdx	=	0;
	if (gStartupController != NULL)
	{
		widgetIdx	=	gStartupController->SetStartupText(startupMsg);
	}
	return(widgetIdx);
}

//**************************************************************************************
void	SetStartupTextStatus(const int widgetIdx, const char *statusText)
{
	if (gStartupController != NULL)
	{
		gStartupController->SetStartupTextStatus(widgetIdx, statusText);
	}
}

//**************************************************************************************
void	SetStartupUpdate(void)
{
	if (gStartupController != NULL)
	{
		gStartupController->cUpdateWindow	=	true;
		gStartupController->DrawWindow();
	}
}

//**************************************************************************************
void	CreateStartupScreen(void)
{
	new ControllerStartup();
	cv::waitKey(50);
	sleep(1);
}

//**************************************************************************************
ControllerStartup::ControllerStartup(void)
	:Controller("SkyTravel", kWindowWidth,  kWindowHeight)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	gStartupController	=	this;
	cCurrentMsgIdx		=   kStartup_TextBox1;
	cBackGroundTaskCntr	=	0;
	SetupWindowControls();

	HandleWindowUpdate();
#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerStartup::~ControllerStartup(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	gStartupController	=	NULL;
	DELETE_OBJ_IF_VALID(cStartupTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerStartup::SetupWindowControls(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetTabCount(kTab_Count);
	SetTabText(kTab_Startup,	"Startup");
	SetTabText(kTab_About,		"About");

	cStartupTabObjPtr	=	new WindowTabStartup(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cStartupTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Startup,	cStartupTabObjPtr);
		cStartupTabObjPtr->SetParentObjectPtr(this);
	}

	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
}

//**************************************************************************************
int	ControllerStartup::SetStartupText(const char *startupMsg)
{
int		widgetIdx;

//	CONSOLE_DEBUG_W_STR("startupMsg\t=",	startupMsg);
	if ((cCurrentMsgIdx >= kStartup_TextBox1) && (cCurrentMsgIdx <= kStartup_TextBoxN))
	{
		SetWidgetText(kTab_Startup, cCurrentMsgIdx, startupMsg);
		widgetIdx	=	cCurrentMsgIdx;
		cCurrentMsgIdx++;

		HandleWindowUpdate();
		cv::waitKey(50);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("startupMsg\t=",	startupMsg);
		CONSOLE_ABORT(__FUNCTION__);
	}
	return(widgetIdx);
}

//**************************************************************************************
void	ControllerStartup::SetStartupTextStatus(const int widgetIdx, const char *statusText)
{
char	messageTxt[256];

//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= kStartup_TextBox1) && (widgetIdx <= kStartup_TextBoxN))
	{
		if (strlen(statusText) > 0)
		{
			GetWidgetText(kTab_Startup, widgetIdx, messageTxt);
			strcat(messageTxt, "\t");
			strcat(messageTxt, statusText);
			SetWidgetText(kTab_Startup, widgetIdx, messageTxt);

			cUpdateWindow	=	true;
			HandleWindowUpdate();
			cv::waitKey(50);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("widgetIdx \t=",	widgetIdx);
			CONSOLE_DEBUG_W_STR("statusText\t=",	statusText);
			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx \t=",	widgetIdx);
		CONSOLE_DEBUG_W_STR("statusText\t=",	statusText);
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
//*	background task is so that we can auto update and auto close this startup window
//**************************************************************************************
void	ControllerStartup::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cBackGroundTaskCntr++;
	cUpdateWindow	=	true;
	if (cBackGroundTaskCntr > 3)
	{
		SetWidgetValid(kTab_Startup,	kStartup_CloseBtn, true);
	}
	if (cBackGroundTaskCntr > 100)
	{
		cKeepRunning	=	false;
	}
	sleep(1);
}

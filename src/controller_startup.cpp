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
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab_startup.h"

#include	"controller.h"
#include	"controller_startup.h"

#define	kWindowWidth	450
#define	kWindowHeight	650

ControllerStartup	*gStartupController	=	NULL;

//**************************************************************************************
enum
{
	kTab_Startup	=	1,

	kTab_Count

};

//**************************************************************************************
int	SetStartupText(const char *startupMsg)
{
int		widgetIdx;

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
void	CreateStartupScreen(void)
{
	gStartupController	=	new ControllerStartup();
	cv::waitKey(50);
	sleep(1);
}

//**************************************************************************************
void	CloseStartupScreen(void)
{
	if (gStartupController != NULL)
	{
		delete gStartupController;
		gStartupController	=	NULL;
	}
}

//**************************************************************************************
ControllerStartup::ControllerStartup(void)
	:Controller("Startup", kWindowWidth,  kWindowHeight)
{
	cCurrentMsgIdx	=   kStartup_TextBox1;
	SetupWindowControls();

	HandleWindowUpdate();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerStartup::~ControllerStartup(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerStartup::SetupWindowControls(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetTabCount(kTab_Count);
	SetTabText(kTab_Startup,	"Startup");
//	SetTabText(kTab_About,		"About");

	cStartupTabObjPtr	=	new WindowTabStartup(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cStartupTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Startup,	cStartupTabObjPtr);
		cStartupTabObjPtr->SetParentObjectPtr(this);
	}

//	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
//	if (cAboutBoxTabObjPtr != NULL)
//	{
//		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
//		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
//	}

}


//**************************************************************************************
int	ControllerStartup::SetStartupText(const char *startupMsg)
{
int		widgetIdx;

	SetWidgetText(kTab_Startup, cCurrentMsgIdx, startupMsg);
	widgetIdx	=	cCurrentMsgIdx;
	cCurrentMsgIdx++;

	HandleWindowUpdate();
	cv::waitKey(50);
	return(widgetIdx);
}

//**************************************************************************************
void	ControllerStartup::SetStartupTextStatus(const int widgetIdx, const char *statusText)
{
char	messageTxt[256];

	GetWidgetText(kTab_Startup, widgetIdx, messageTxt);
	strcat(messageTxt, "\t");
	strcat(messageTxt, statusText);
//	strcat(messageTxt, "---");
	SetWidgetText(kTab_Startup, widgetIdx, messageTxt);

	HandleWindowUpdate();
	cv::waitKey(50);
}

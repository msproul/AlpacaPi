//*****************************************************************************
//*		controller_aavso.cpp		(c) 2021 by Mark Sproul
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
//*	Jul 17,	2021	<MLS> Created controller_aavso.cpp
//*****************************************************************************


#define _ENABLE_AAVSO_

#ifdef _ENABLE_AAVSO_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	999
#define	kWindowHeight	700

#include	"windowtab_aavsolist.h"
#include	"windowtab_about.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"controller_aavso.h"

//**************************************************************************************
enum
{
	kTab_AAVSO_list	=	1,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
void	CreateAAVSOlistWindow(void)
{
	new ControllerAAVSOlist("AAVSO List");
}

//**************************************************************************************
ControllerAAVSOlist::ControllerAAVSOlist(	const char			*argWindowName)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("kWindowWidth\t=", kWindowWidth);
	CONSOLE_DEBUG_W_NUM("kWindowHeight\t=", kWindowHeight);

	cAAVSOlistTabObjPtr		=	NULL;
	cAboutBoxTabObjPtr		=	NULL;



	SetupWindowControls();

//	SetWidgetText(kTab_FilterWheel,		kFilterWheel_AlpacaDrvrVersion,		cAlpacaVersionString);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerAAVSOlist::~ControllerAAVSOlist(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	DELETE_OBJ_IF_VALID(cAAVSOlistTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerAAVSOlist::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_AAVSO_list,		"AAVSO Object List");
	SetTabText(kTab_About,			"About");


	//--------------------------------------------
	cAAVSOlistTabObjPtr	=	new WindowTabAAVSOlist(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAAVSOlistTabObjPtr != NULL)
	{
		SetTabWindow(kTab_AAVSO_list,	cAAVSOlistTabObjPtr);
		cAAVSOlistTabObjPtr->SetParentObjectPtr(this);
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
void	ControllerAAVSOlist::RunBackgroundTasks(void)
{
long	delteaMillSecs;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cReadStartup)
	{
		CONSOLE_DEBUG_W_STR("cReadStartup", cWindowName);

		//*	so the window shows up
		HandleWindowUpdate();
		cvWaitKey(60);



		cReadStartup	=	false;
	}

	delteaMillSecs	=	millis() - cLastUpdate_milliSecs;
	if (delteaMillSecs > 5000)
	{
		//*	do what needs to be done here
		cLastUpdate_milliSecs	=	millis();
	}
}

//*****************************************************************************
bool	ControllerAAVSOlist::AlpacaGetStartupData(void)
{
bool			validData;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	validData	=	true;
	return(validData);
}

//*****************************************************************************
void	ControllerAAVSOlist::UpdateCommonProperties(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_Name,				cCommonProp.Name);

}





#endif // _ENABLE_AAVSO_

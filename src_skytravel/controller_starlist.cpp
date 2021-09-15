//*****************************************************************************
//*		controller_starlist.cpp		(c) 2021 by Mark Sproul
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
//*	Sep  6,	2021	<MLS> Created controller_starlist.cpp
//*	Sep 12,	2021	<MLS> Combined AAVSO list with starlist
//*****************************************************************************


#define _ENABLE_STARLIST_

#ifdef _ENABLE_STARLIST_


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

#include	"windowtab_starlist.h"
#include	"windowtab_about.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"controller_starlist.h"

//**************************************************************************************
enum
{
	kTab_Star_list	=	1,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
void	CreateStarlistWindow(const char *windowName, TYPE_CelestData *starListPtr, int starListCount)
{
	new ControllerStarlist(windowName, starListPtr,  starListCount);
}

//**************************************************************************************
ControllerStarlist::ControllerStarlist(	const char		*argWindowName,
										TYPE_CelestData	*argStarList,
										int				argStarListCount)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("kWindowWidth\t=", kWindowWidth);
	CONSOLE_DEBUG_W_NUM("kWindowHeight\t=", kWindowHeight);

	cStarListTabObjPtr		=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	CONSOLE_DEBUG_W_NUM("argStarListCount\t=", argStarListCount);


	SetupWindowControls();


	if (cStarListTabObjPtr != NULL)
	{
		cStarListTabObjPtr->SetStarDataPointers(argStarList, argStarListCount);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerStarlist::~ControllerStarlist(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	DELETE_OBJ_IF_VALID(cStarListTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerStarlist::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Star_list,		"Star Object List");
	SetTabText(kTab_About,			"About");

	//--------------------------------------------
	cStarListTabObjPtr	=	new WindowTabStarList(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cStarListTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Star_list,	cStarListTabObjPtr);
		cStarListTabObjPtr->SetParentObjectPtr(this);
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
void	ControllerStarlist::RunBackgroundTasks(void)
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
bool	ControllerStarlist::AlpacaGetStartupData(void)
{
bool			validData;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	validData	=	true;
	return(validData);
}

//*****************************************************************************
void	ControllerStarlist::UpdateCommonProperties(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_Name,				cCommonProp.Name);

}





#endif // _ENABLE_AAVSO_

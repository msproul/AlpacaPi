//*****************************************************************************
//*		controller_constList.cpp
//*		(c) 2021 by Mark Sproul
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
//*	Oct 26,	2021	<MLS> Created controller_constList.cpp
//*****************************************************************************


#define _ENABLE_CONSTELLATIONLIST_

#ifdef _ENABLE_CONSTELLATIONLIST_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	999
#define	kWindowHeight	700

#include	"windowtab_starlist.h"
#include	"windowtab_about.h"
#include	"helper_functions.h"

#include	"ConstellationData.h"

#include	"controller.h"
#include	"controller_constList.h"

//**************************************************************************************
enum
{
	kTab_Constellation_list	=	1,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
void	CreateConstellationListWindow(	TYPE_ConstVector	*constellationsPtr,
										const int			constellationCnt)
{
	new ControllerConstellationList(constellationsPtr, constellationCnt);
}

//**************************************************************************************
ControllerConstellationList::ControllerConstellationList(	TYPE_ConstVector	*argConstellations,
															const int			argConstellationCnt)
	:Controller("Constellations List", kWindowWidth,  kWindowHeight)
{
	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("kWindowWidth\t=", kWindowWidth);
//	CONSOLE_DEBUG_W_NUM("kWindowHeight\t=", kWindowHeight);

	cConstListTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr	=	NULL;

	CONSOLE_DEBUG_W_NUM("argConstellationCnt\t=", argConstellationCnt);


	SetupWindowControls();


	if (argConstellations != NULL)
	{
		cConstListTabObjPtr->SetStarDataPointers(argConstellations, argConstellationCnt);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerConstellationList::~ControllerConstellationList(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	DELETE_OBJ_IF_VALID(cConstListTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerConstellationList::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Constellation_list,	"Constellation List");
	SetTabText(kTab_About,				"About");

	//--------------------------------------------
	cConstListTabObjPtr	=	new WindowTabConstellationList(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cConstListTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Constellation_list,	cConstListTabObjPtr);
		cConstListTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
}

//*****************************************************************************
void	ControllerConstellationList::UpdateCommonProperties(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_Name,				cCommonProp.Name);

}

#endif // _ENABLE_CONSTELLATIONLIST_

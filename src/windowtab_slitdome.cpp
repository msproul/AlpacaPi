//*****************************************************************************
//*		windowtab_slitdome.cpp		(c) 2023 by Mark Sproul
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
//*	Mar  8,	2023	<MLS> Created windowtab_slitdome.cpp
//*	Mar  8,	2023	<MLS> Added GetDomeData()
//*****************************************************************************

#if defined(_ENABLE_SLIT_TRACKER_)

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_slitdome.h"
#include	"controller.h"
#include	"slittracker_data.h"
#include	"controller_slit.h"



//**************************************************************************************
WindowTabSlitDome::WindowTabSlitDome(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);

	memset(&cSlitProp, 0, sizeof(TYPE_SlittrackerProperties));
	cSlitProp.TrackingEnabled	=	false;
	cEnableAutomaticDomeUpdates	=	false;
	cDomePropPtr				=	NULL;

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabSlitDome::~WindowTabSlitDome(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabSlitDome::SetupWindowControls(void)
{
int		yLoc;
int		xLoc;
int		infoBoxWidth;
int		boxWidthHalf;
int		boxWidthQrtr;
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kSlitDome_Title, -1, yLoc, "SlitTracker Dome Connections");
	xLoc	=	5;
	//==========================================
	infoBoxWidth	=	cWidth - (2 * xLoc);
	boxWidthHalf	=	(infoBoxWidth / 2) - 1;
	boxWidthQrtr	=	(boxWidthHalf / 2) - 1;
	SetWidget(			kSlitDome_DomeInfoLabel,	xLoc,	yLoc,	infoBoxWidth,	cBtnHeight);
	SetWidgetFont(		kSlitDome_DomeInfoLabel,	kFont_Medium);
	SetWidgetText(		kSlitDome_DomeInfoLabel,	"Dome Connection");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//------------------------------
	SetWidget(			kSlitDome_DomeIPaddr,		xLoc,	yLoc,	boxWidthHalf,	cBtnHeight);
	SetWidgetFont(		kSlitDome_DomeIPaddr,		kFont_Medium);
	SetWidgetText(		kSlitDome_DomeIPaddr,		"1.1.1.1");
	xLoc	+=	boxWidthHalf;
	xLoc	+=	1;

	//------------------------------
	SetWidget(			kSlitDome_DomeAlpacaPort,	xLoc,	yLoc,	boxWidthQrtr,	cBtnHeight);
	SetWidgetFont(		kSlitDome_DomeAlpacaPort,	kFont_Medium);
	SetWidgetText(		kSlitDome_DomeAlpacaPort,	"1234");
	xLoc	+=	boxWidthQrtr;
	xLoc	+=	1;

	//------------------------------
	SetWidget(			kSlitDome_DomeDevNum,		xLoc,	yLoc,	boxWidthQrtr,	cBtnHeight);
	SetWidgetFont(		kSlitDome_DomeDevNum,		kFont_Medium);
	SetWidgetNumber(	kSlitDome_DomeDevNum,		0);

	xLoc	=	5;
	yLoc	+=	cBtnHeight;
	yLoc	+=	2;

	//------------------------------
	SetWidget(			kSlitDome_DomeName,	xLoc,	yLoc,	infoBoxWidth,	cBtnHeight);
	SetWidgetFont(		kSlitDome_DomeName,	kFont_Medium);
	SetWidgetText(		kSlitDome_DomeName,	"---");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//------------------------------
	SetWidget(			kSlitDome_DomeDescription,	xLoc,	yLoc,	infoBoxWidth,	cBtnHeight);
	SetWidgetFont(		kSlitDome_DomeDescription,	kFont_Medium);
	SetWidgetText(		kSlitDome_DomeDescription,	"---");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	iii	=	kSlitDome_DomeShutterLbl;
	while (iii < kSlitDome_DomeAzimuth)
	{
		xLoc	=	5;
		SetWidget(			iii,	xLoc,	yLoc,	boxWidthHalf,	cBtnHeight);
		SetWidgetFont(		iii,	kFont_Medium);
		xLoc	+=	boxWidthHalf;
		xLoc	+=	1;
		iii++;

		SetWidget(			iii,	xLoc,	yLoc,	boxWidthHalf,	cBtnHeight);
		SetWidgetFont(		iii,	kFont_Medium);
		iii++;

		yLoc			+=	cBtnHeight;
		yLoc			+=	2;
	}
	xLoc	=	5;
	SetWidgetText(		kSlitDome_DomeShutterLbl,	"Shutter Status");
	SetWidgetText(		kSlitDome_DomePositionLbl,	"Dome Position");
	SetWidgetText(		kSlitDome_DomeAzimuthLbl,	"Dome Azimuth");

	//--------------------------
	SetWidget(			kSlitDome_DomeGetDataBtn,	xLoc,	yLoc,	infoBoxWidth,	cBtnHeight);
	SetWidgetType(		kSlitDome_DomeGetDataBtn,	kWidgetType_Button);
	SetWidgetFont(		kSlitDome_DomeGetDataBtn,	kFont_Medium);
	SetWidgetBGColor(	kSlitDome_DomeGetDataBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kSlitDome_DomeGetDataBtn,	CV_RGB(255,	0,	0));
	SetWidgetText(		kSlitDome_DomeGetDataBtn,	"Get Dome Data");
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	SetWidgetOutlineBox(kSlitDome_DomeOutline, kSlitDome_DomeInfoLabel, (kSlitDome_DomeOutline - 1));
	yLoc			+=	2;

	//--------------------------
	iii	=	kSlitDome_DomeEnableData;
	while (iii <= kSlitDome_DomeEnableTracking)
	{
		SetWidget(		iii,	xLoc,	yLoc,	infoBoxWidth,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_CheckBox);
		SetWidgetFont(	iii,	kFont_Medium);
		iii++;
		yLoc			+=	cSmallBtnHt;
		yLoc			+=	2;
	}
	SetWidgetText(	kSlitDome_DomeEnableData,	"Enable Automatic Dome updates");
	SetWidgetText(	kSlitDome_DomeEnableTracking,		"Enable Dome Tracking");

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kSlitDome_IPaddr,
							kSlitDome_Readall,
							kSlitDome_AlpacaErrorMsg,
							kSlitDome_LastCmdString,
							kSlitDome_AlpacaLogo,
							-1);
}


//*****************************************************************************
void	WindowTabSlitDome::ProcessButtonClick(const int buttonIdx, const int flags)
{
ControllerSlit	*mySlitController;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);
	SetWidgetText(kSlitDome_AlpacaErrorMsg, "");

	mySlitController	=	(ControllerSlit *)cParentObjPtr;
	switch(buttonIdx)
	{
		case kSlitDome_DomeGetDataBtn:
			CONSOLE_DEBUG("kSlitDome_DomeGetDataBtn");
			GetDomeData();
			break;


		case kSlitDome_DomeEnableData:
			if (mySlitController != NULL)
			{
				mySlitController->SetButtonOption(buttonIdx, !cEnableAutomaticDomeUpdates);
			}
			break;

		case kSlitDome_DomeEnableTracking:
//			CONSOLE_DEBUG("kSlitDome_DomeEnableTracking");
			if (mySlitController != NULL)
			{
//				CONSOLE_DEBUG_W_NUM("cDomePropPtr->ShutterStatus\t=", cDomePropPtr->ShutterStatus);
				//*	the shutter MUST be open to turn on tracking, however
				//*	if it is already enabled, we can turn it off
				if ((cDomePropPtr->ShutterStatus == kShutterStatus_Open) ||
					(cDomePropPtr->ShutterStatus == kShutterStatus_Opening) ||
					(cSlitProp.TrackingEnabled == true))
				{
					mySlitController->SetAlpacaEnableTracking(!cSlitProp.TrackingEnabled);
				}
				else
				{
					CONSOLE_DEBUG("Shutter is NOT open");
					SetWidgetText(kSlitDome_AlpacaErrorMsg, "Shutter must be open for tracking");
					ForceWindowUpdate();
				}
			}
			else
			{
				CONSOLE_ABORT(__FUNCTION__);
			}
			break;


		default:
			break;
	}
}

//*****************************************************************************
void	WindowTabSlitDome::SetDomePropertiesPtr(TYPE_DomeProperties *domePropPtr)
{
	//*	this is a pointer to the cDomeProp structure in the parent controller
	cDomePropPtr	=	domePropPtr;
}

//*****************************************************************************
//*	yes, ProcessButtonClick calls the controller object which turns around
//*	and calls this routine, it is important to make sure everything is synced up
//*****************************************************************************
void	WindowTabSlitDome::UpdateButtons(const int widgetBtnIdx, const bool newState)
{
//ControllerSlit	*mySlitController;
//
//	mySlitController	=	(ControllerSlit *)cParentObjPtr;

	switch(widgetBtnIdx)
	{
		case kSlitDome_DomeEnableData:
			cEnableAutomaticDomeUpdates	=	newState;
			SetWidgetChecked(kSlitDome_DomeEnableData, cEnableAutomaticDomeUpdates);
			break;

		case kSlitDome_DomeEnableTracking:
			cSlitProp.TrackingEnabled	=	newState;
			SetWidgetChecked(kSlitDome_DomeEnableTracking, cSlitProp.TrackingEnabled);
			break;

		default:
			CONSOLE_ABORT(__FUNCTION__);
			break;
	}
	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabSlitDome::GetDomeData(void)
{
ControllerSlit	*mySlitController;

	SetWidgetText(kSlitDome_DomeName,			"---");
	SetWidgetText(kSlitDome_DomeDescription,	"---");


	mySlitController	=	(ControllerSlit *)cParentObjPtr;
	if (mySlitController != NULL)
	{
		mySlitController->GetDomeData_Startup();
	}
}
#endif // _ENABLE_SLIT_TRACKER_


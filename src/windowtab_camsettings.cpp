//*****************************************************************************
//*		windowtab_camsettings.cpp		(c) 2020 by Mark Sproul
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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 15,	2020	<MLS> Created windowtab_camsettings.cpp
//*	Mar 15,	2020	<MLS> Added SetObjectText()
//*	Mar 19,	2020	<MLS> Added SetExposureValues()
//*	Mar 22,	2020	<MLS> Added SetFileNameOptions()
//*	Apr  7,	2020	<MLS> Added Dark, Flat and Bias to settings options
//*	Jul  2,	2020	<MLS> Added filename include refID radio button
//*****************************************************************************


#ifdef _ENABLE_CTRL_CAMERA_

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"windowtab_camsettings.h"

#include	"controller.h"
#include	"controller_camera.h"

//**************************************************************************************
typedef struct
{
	char	name[16];
	char	prefix[16];
	double	exposureMin;
	double	exposureMax;
	double	exposureStep;

} TYPE_PRESETS;

//**************************************************************************************
TYPE_PRESETS	gPresets[]		=
{
	{	"Test",		"Test",		0.00001,	20000.0,	0.001	},
	{	"Dark",		"Dark",		0.000032,	5.0,		0.00001	},
	{	"Flat",		"Flat",		0.000032,	5.0,		0.00001	},
	{	"Bias",		"Bias",		0.000032,	5.0,		0.00001	},
	{	"Moon",		"Moon",		0.000032,	5.0,		0.00001	},
	{	"Sun",		"Sun",		0.000032,	1.0,		0.00001	},
	{	"Mercury",	"Merc",		0.000032,	20000.0,	0.001	},
	{	"Venus",	"Ven",		0.000032,	20000.0,	0.001	},
	{	"Mars",		"Mars",		0.000032,	20000.0,	0.001	},
	{	"Jupiter",	"Jup",		0.000032,	20000.0,	0.001	},
	{	"Saturn",	"Sat",		0.000032,	20000.0,	0.001	},
	{	"Uranus",	"Uran",		0.000032,	20000.0,	0.001	},
	{	"Neptune",	"Nept",		0.000032,	20000.0,	0.001	},
	{	"Pluto",	"Plut",		0.000032,	20000.0,	0.001	},
	{	"Star",		"Star",		0.000032,	20000.0,	0.001	},
	{	"Deepsky",	"DSO",		0.000032,	20000.0,	0.01	},
	{	"Other",	"Other",	0.000032,	20000.0,	0.001	},


	{	"",			"",			0.0,		0.0,		0.0		}
};

int	gCurrentPresetExp	=	0;



//**************************************************************************************
WindowTabCamSettings::WindowTabCamSettings(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName,
											const char	*deviceName,
											const bool	hasFilterWheel)

	:WindowTab(xSize, ySize, backGrndColor, windowName)
{

	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabCamSettings::~WindowTabCamSettings(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

#define	kFileListHeight	200
#define	kLabelHeight	25
//**************************************************************************************
void	WindowTabCamSettings::SetupWindowControls(void)
{
int			yLoc;
int			iii;
int			clm1Xoffset;
int			clm2Xoffset;
int			clm1Width;
int			clm2Width;
int			saveYloc;
char		buttonText[64];

	CONSOLE_DEBUG(__FUNCTION__);
	//==========================================
	yLoc			=	cTabVertOffset;

	//==========================================
	SetWidget(		kCamSet_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetFont(	kCamSet_Title, kFont_Medium);
	SetBGcolorFromWindowName(kCamSet_Title);

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	saveYloc		=	yLoc;


	SetWidget(		kCamSet_P_Title,	5,			yLoc,		cWidth/3,	cRadioBtnHt);
	SetWidgetText(	kCamSet_P_Title,	"Object");
	SetWidgetFont(	kCamSet_P_Title,	kFont_RadioBtn);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;

	//*	set up the preset buttons
	for (iii=kCamSet_Preset01; iii<kCamSet_ObjOutline; iii++)
	{
		SetWidget(			iii,	5,			yLoc,		cWidth/3,	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_RadioButton);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		SetWidgetBGColor(	iii,	CV_RGB(0,	0,	0));
		SetWidgetTextColor(	iii,	CV_RGB(255,	255,	255));
		SetWidgetText(		iii,	gPresets[iii - kCamSet_Preset01].name);
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
	SetWidgetOutlineBox(	kCamSet_ObjOutline,
							kCamSet_P_Title,
							(kCamSet_ObjOutline - 1));
	yLoc	=	saveYloc;
	//==========================================
	//*	set up the time buttons
	clm2Xoffset	=	(cWidth / 3) + 7;
	clm2Width	=	cWidth - clm2Xoffset;
	clm2Width	-=	2;
	yLoc		=	saveYloc;

	SetWidget(		kCamSet_T_Title,	clm2Xoffset,	yLoc,	clm2Width,	cRadioBtnHt);
	SetWidgetText(	kCamSet_T_Title,	"MinExp     MaxExp     expStep");
	SetWidgetFont(	kCamSet_T_Title,	kFont_RadioBtn);
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	for (iii=kCamSet_Time01; iii<kCamSet_TimeOutline; iii++)
	{
		sprintf(buttonText, "%7.6f -> %10.4f : %7.6f",	gPresets[iii - kCamSet_Time01].exposureMin,
														gPresets[iii - kCamSet_Time01].exposureMax,
														gPresets[iii - kCamSet_Time01].exposureStep);

		SetWidget(			iii,	clm2Xoffset,	yLoc,	clm2Width,	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_RadioButton);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		SetWidgetBGColor(	iii,	CV_RGB(0,	0,	0));
		SetWidgetTextColor(	iii,	CV_RGB(255,	255,	255));
		SetWidgetText(		iii,	buttonText);
		yLoc			+=	cRadioBtnHt;
		yLoc			+=	2;
	}
	SetWidgetOutlineBox(	kCamSet_TimeOutline,
							kCamSet_T_Title,
							(kCamSet_TimeOutline - 1));
	SetWidgetChecked((kCamSet_Time01 + gCurrentPresetExp), true);


	//==========================================
	//*	set up filename options
	yLoc		+=	cRadioBtnHt;
	yLoc		+=	2;
	clm1Xoffset	=	2;
	clm1Width	=	cWidth / 2;

	SetWidget(			kCamSet_FilenameTitle,	clm1Xoffset,	yLoc,		clm1Width,	cRadioBtnHt);
	SetWidgetFont(		kCamSet_FilenameTitle,	kFont_RadioBtn);
	SetWidgetText(		kCamSet_FilenameTitle,	"Filename options");
	yLoc		+=	cRadioBtnHt;
	yLoc		+=	2;

	SetWidget(			kCamSet_FN_IncFilter,	clm1Xoffset,	yLoc,		clm1Width,	cRadioBtnHt);
	SetWidgetType(		kCamSet_FN_IncFilter,	kWidgetType_CheckBox);
	SetWidgetFont(		kCamSet_FN_IncFilter,	kFont_RadioBtn);
	SetWidgetText(		kCamSet_FN_IncFilter,	"Include filter");
	SetWidgetTextColor(	kCamSet_FN_IncFilter,	CV_RGB(255,	255,	255));
	yLoc	+=	cRadioBtnHt;
	yLoc	+=	2;

	SetWidget(			kCamSet_FN_IncCamera,	clm1Xoffset,	yLoc,		clm1Width,	cRadioBtnHt);
	SetWidgetType(		kCamSet_FN_IncCamera,	kWidgetType_CheckBox);
	SetWidgetFont(		kCamSet_FN_IncCamera,	kFont_RadioBtn);
	SetWidgetText(		kCamSet_FN_IncCamera,	"Include camera");
	SetWidgetTextColor(	kCamSet_FN_IncCamera,	CV_RGB(255,	255,	255));
	yLoc	+=	cRadioBtnHt;
	yLoc	+=	2;

	SetWidget(			kCamSet_FN_IncSerialNum,	clm1Xoffset,	yLoc,	clm1Width,	cRadioBtnHt);
	SetWidgetType(		kCamSet_FN_IncSerialNum,	kWidgetType_CheckBox);
	SetWidgetFont(		kCamSet_FN_IncSerialNum,	kFont_RadioBtn);
	SetWidgetText(		kCamSet_FN_IncSerialNum,	"Include serial number");
	SetWidgetTextColor(	kCamSet_FN_IncSerialNum,	CV_RGB(255,	255,	255));
	yLoc	+=	cRadioBtnHt;
	yLoc	+=	2;

	SetWidget(			kCamSet_FN_IncRefID,	clm1Xoffset,	yLoc,	clm1Width,	cRadioBtnHt);
	SetWidgetType(		kCamSet_FN_IncRefID,	kWidgetType_CheckBox);
	SetWidgetFont(		kCamSet_FN_IncRefID,	kFont_RadioBtn);
	SetWidgetText(		kCamSet_FN_IncRefID,	"Include refID");
	SetWidgetTextColor(	kCamSet_FN_IncRefID,	CV_RGB(255,	255,	255));
	yLoc	+=	cRadioBtnHt;
	yLoc	+=	2;


	SetWidgetOutlineBox(	kCamSet_FilenameOutline,
							kCamSet_FilenameTitle,
							(kCamSet_FilenameOutline - 1));

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kCamSet_IPaddr,
							kCamSet_Readall,
							kCamSet_AlpacaErrorMsg,
							kCamSet_LastCmdString,
							kCamSet_AlpacaLogo,
							-1);
}

//**************************************************************************************
void	WindowTabCamSettings::SetObjectText(const int presetIndex)
{
ControllerCamera	*myCameraController;
char				objectString[32];
char				prefixString[32];

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(objectString, gPresets[presetIndex].name);
	strcpy(prefixString, gPresets[presetIndex].prefix);

	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->SetObjectText(objectString, prefixString);
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}

//**************************************************************************************
void	WindowTabCamSettings::SetExposureRange(const int expTimeIdx)
{
int	iii;

	CONSOLE_DEBUG(__FUNCTION__);

	gCurrentPresetExp	=	expTimeIdx;
	for (iii=kCamSet_Time01; iii<=kCamSet_Time14; iii++)
	{
		SetWidgetChecked(iii, false);
	}
	SetWidgetChecked((kCamSet_Time01 + gCurrentPresetExp), true);
}


//**************************************************************************************
void	WindowTabCamSettings::SetFileNameOptions(const int fnOptionBtn)
{
ControllerCamera	*myCameraController;

	CONSOLE_DEBUG(__FUNCTION__);

	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->SetFileNameOptions(fnOptionBtn);
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}


//**************************************************************************************
void	WindowTabCamSettings::ProcessButtonClick(const int buttonIdx, const int flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	switch(buttonIdx)
	{
		case kCamSet_Preset01:
		case kCamSet_Preset02:
		case kCamSet_Preset03:
		case kCamSet_Preset04:
		case kCamSet_Preset05:
		case kCamSet_Preset06:
		case kCamSet_Preset07:
		case kCamSet_Preset08:
		case kCamSet_Preset09:
		case kCamSet_Preset10:
		case kCamSet_Preset11:
		case kCamSet_Preset12:
		case kCamSet_Preset13:
		case kCamSet_Preset14:
		case kCamSet_Preset15:
		case kCamSet_Preset16:
		case kCamSet_Preset17:
			SetObjectText(buttonIdx - kCamSet_Preset01);
			break;

		case kCamSet_Time01:
		case kCamSet_Time02:
		case kCamSet_Time03:
		case kCamSet_Time04:
		case kCamSet_Time05:
		case kCamSet_Time06:
		case kCamSet_Time07:
		case kCamSet_Time08:
		case kCamSet_Time09:
		case kCamSet_Time10:
		case kCamSet_Time11:
		case kCamSet_Time12:
		case kCamSet_Time13:
		case kCamSet_Time14:
		case kCamSet_Time15:
		case kCamSet_Time16:
		case kCamSet_Time17:
			SetExposureRange(buttonIdx - kCamSet_Time01);
			break;

		case kCamSet_FN_IncFilter:
		case kCamSet_FN_IncCamera:
		case kCamSet_FN_IncSerialNum:
		case kCamSet_FN_IncRefID:
			SetFileNameOptions(buttonIdx);
			break;


		case kCamSet_Title:
		case kCamSet_IPaddr:
		default:
			//*	do nothing
			break;
	}
	DisplayLastAlpacaCommand();
}

//**************************************************************************************
void	WindowTabCamSettings::ProcessDoubleClick(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)

{
//	CONSOLE_DEBUG(__FUNCTION__);
	switch(widgetIdx)
	{
		case kCamSet_Title:
		case kCamSet_IPaddr:
			//*	do nothing
			break;

		default:
			CONSOLE_DEBUG_W_NUM("double click on ", widgetIdx);
			break;
	}
}

//*****************************************************************************
void	WindowTabCamSettings::UpdateSettings_Object(const char *filePrefix)
{
int		iii;
int		presetIdx;
char	otherString[48];

//	CONSOLE_DEBUG(__FUNCTION__);
	presetIdx	=	-1;
	iii			=	0;
	while ((gPresets[iii].name[0] > 0x20) && (presetIdx < 0))
	{
		if ((strcasecmp(filePrefix, gPresets[iii].name) == 0) ||
			(strcasecmp(filePrefix, gPresets[iii].prefix) == 0))
		{
			presetIdx	=	iii;
		}
		iii++;
	}

	//*	turn off all of the check boxes
	for (iii=kCamSet_Preset01; iii<kCamSet_ObjOutline; iii++)
	{
		SetWidgetChecked(iii, false);
	}
	//*	now set the one we found
	if (presetIdx >= 0)
	{
		SetWidgetChecked((kCamSet_Preset01 + presetIdx), true);
	}
	else
	{
		strcpy(otherString, "Other-");
		strcat(otherString, filePrefix);
 		SetWidgetChecked(	(kCamSet_ObjOutline - 1), true);
 		SetWidgetText(		(kCamSet_ObjOutline - 1), otherString);
  }
}


#endif // _ENABLE_CTRL_CAMERA_


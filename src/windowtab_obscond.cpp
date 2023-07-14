//*****************************************************************************
//*		windowtab_obscond.cpp		(c) 2022 by Mark Sproul
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
//*	Sep 25,	2022	<MLS> Created windowtab_obscond.cpp
//*	Sep 25,	2022	<MLS> Added UpdateObservationValues()
//*	Sep 26,	2022	<MLS> Added UpdateOneValue()
//*****************************************************************************


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_obscond.h"
#include	"controller.h"


#define	kAboutBoxHeight	100

//**************************************************************************************
WindowTabObsCond::WindowTabObsCond(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);


	SetupWindowControls();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabObsCond::~WindowTabObsCond(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabObsCond::SetupWindowControls(void)
{
int		yLoc;
int		boxNumber;
int		titleLeft;
int		titleWidth;
int		valueLeft;
int		valueWidth;
int		unitsLeft;
int		unitsWidth;

int		boxLeft;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kObsCond_Title, kObsCond_Connected, yLoc, "Observing Conditions");

	boxLeft			=	2;
	yLoc			+=	10;
	titleLeft		=	boxLeft + 2;
	titleWidth		=	160;

	valueLeft		=	titleLeft + titleWidth  + 2;
	valueWidth		=	110;

	unitsLeft		=	valueLeft + valueWidth  + 2;
	unitsWidth		=	145;

	boxNumber		=	kObsCond_Averageperiod_Title;
	while (boxNumber <= kObsCond_WindSpeed)
	{
		//*	set the title box
		SetWidget(				boxNumber,	titleLeft,		yLoc,	titleWidth,	cSmallBtnHt);
		SetWidgetFont(			boxNumber,	kFont_Medium);
		SetWidgetJustification(	boxNumber,	kJustification_Left);
		boxNumber++;

		//*	set the value box
		SetWidget(				boxNumber,	valueLeft,		yLoc,	valueWidth,	cSmallBtnHt);
		SetWidgetFont(			boxNumber,	kFont_Medium);
		SetWidgetText(			boxNumber,	"-----");
		boxNumber++;

		//*	set the units box
		SetWidget(				boxNumber,	unitsLeft,		yLoc,	unitsWidth,	cSmallBtnHt);
		SetWidgetFont(			boxNumber,	kFont_Medium);
		SetWidgetText(			boxNumber,	"-----");
		boxNumber++;

		yLoc			+=	cSmallBtnHt;
		yLoc			+=	4;
	}
	SetWidgetText(	kObsCond_Averageperiod_Title,	"Averageperiod");
	SetWidgetText(	kObsCond_Cloudcover_Title,		"Cloudcover");
	SetWidgetText(	kObsCond_DewPoint_Title,		"DewPoint");
	SetWidgetText(	kObsCond_Humidity_Title,		"Humidity");
	SetWidgetText(	kObsCond_Pressure_hPa_Title,	"Pressure");
	SetWidgetText(	kObsCond_RainRate_Title,		"RainRate");
	SetWidgetText(	kObsCond_SkyBrightness_Title,	"SkyBrightness");
	SetWidgetText(	kObsCond_SkyQuality_Title,		"SkyQuality");
	SetWidgetText(	kObsCond_SkyTemperature_Title,	"SkyTemperature");
	SetWidgetText(	kObsCond_StarFWHM_Title,		"StarFWHM");
	SetWidgetText(	kObsCond_Temperature_Title,		"Temperature");
	SetWidgetText(	kObsCond_TempDegreesF_Title,	"Temperature");
	SetWidgetText(	kObsCond_WindDirection_Title,	"WindDirection");
	SetWidgetText(	kObsCond_WindGust_Title,		"WindGust");
	SetWidgetText(	kObsCond_WindSpeed_Title,		"WindSpeed");


//CloudCover	0.833	Percent
//DewPoint	-3.228	Unknown
//Humidity	51.667	Percent
//Pressure	1022.500	hPa
//RainRate	0.000	mm/hr
//SkyBrightness	88.333	Lux
//SkyQuality	18.333	Mag/arcsec^2
//StarFWHM	0.933	arcsec
//SkyTemperature	-27.500	Celsius
//Temperature	5.967	Celsius
//WindDirection	200.867	Degrees
//WindGust	3.377	mph
//WindSpeed	0.638	mph

	SetWidgetText(	kObsCond_Averageperiod_Units,	"Hours");
	SetWidgetText(	kObsCond_Cloudcover_Units,		"Percent");
	SetWidgetText(	kObsCond_DewPoint_Units,		"Deg C");
	SetWidgetText(	kObsCond_Humidity_Units,		"Percent");
	SetWidgetText(	kObsCond_Pressure_hPa_Units,	"hPa");
	SetWidgetText(	kObsCond_RainRate_Units,		"mm/hr");
	SetWidgetText(	kObsCond_SkyBrightness_Units,	"Lux");
	SetWidgetText(	kObsCond_SkyQuality_Units,		"Mag/arcsec^2");
	SetWidgetText(	kObsCond_SkyTemperature_Units,	"Deg C");
	SetWidgetText(	kObsCond_StarFWHM_Units,		"arcsec");
	SetWidgetText(	kObsCond_Temperature_Units,		"Deg C");
	SetWidgetText(	kObsCond_TempDegreesF_Units,	"Deg F");
	SetWidgetText(	kObsCond_WindDirection_Units,	"Degrees");
	SetWidgetText(	kObsCond_WindGust_Units,		"mph");
	SetWidgetText(	kObsCond_WindSpeed_Units,		"mph");

	yLoc			+=	4;


	cFirstRead	=	true;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kObsCond_IPaddr,
							kObsCond_Readall,
							kObsCond_DeviceState,
							kObsCond_AlpacaErrorMsg,
							kObsCond_LastCmdString,
							kObsCond_AlpacaLogo,
							-1);
}

//*****************************************************************************
void	WindowTabObsCond::ProcessButtonClick(const int buttonIdx, const int flags)
{

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

//	switch(buttonIdx)
//	{
//
//	}
//	DisplayLastAlpacaCommand();
}


//*****************************************************************************
void	WindowTabObsCond::UpdateObservationValues(TYPE_ObsConditionProperties *obsCondProp)
{
TYPE_InstSensor		temperatureDegF;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	make a copy for displaying degrees F
	temperatureDegF			=	obsCondProp->Temperature;
	temperatureDegF.Value	=	(temperatureDegF.Value * (9.0 /5.0)) + 32.0;

	//*	last argument is decimal places											***
	SetWidgetSensorValue(	kObsCond_Averageperiod,		&obsCondProp->Averageperiod,	5);
	SetWidgetSensorValue(	kObsCond_Cloudcover,		&obsCondProp->CloudCover,		1);
	SetWidgetSensorValue(	kObsCond_DewPoint,			&obsCondProp->DewPoint,			1);
	SetWidgetSensorValue(	kObsCond_Humidity,			&obsCondProp->Humidity,			1);
	SetWidgetSensorValue(	kObsCond_Pressure_hPa,		&obsCondProp->Pressure, 		0);
	SetWidgetSensorValue(	kObsCond_RainRate,			&obsCondProp->RainRate, 		1);
	SetWidgetSensorValue(	kObsCond_SkyBrightness,		&obsCondProp->SkyBrightness,	1);
	SetWidgetSensorValue(	kObsCond_SkyQuality,		&obsCondProp->SkyQuality,		1);
	SetWidgetSensorValue(	kObsCond_SkyTemperature,	&obsCondProp->SkyTemperature,	1);
	SetWidgetSensorValue(	kObsCond_StarFWHM,			&obsCondProp->StarFWHM,			3);
	SetWidgetSensorValue(	kObsCond_Temperature,		&obsCondProp->Temperature,	 	1);
	SetWidgetSensorValue(	kObsCond_TempDegreesF,		&temperatureDegF,	 			1);
	SetWidgetSensorValue(	kObsCond_WindDirection,		&obsCondProp->WindDirection,	1);
	SetWidgetSensorValue(	kObsCond_WindGust,			&obsCondProp->WindGust,			1);
	SetWidgetSensorValue(	kObsCond_WindSpeed,			&obsCondProp->WindSpeed,		1);

	ForceWindowUpdate();
}

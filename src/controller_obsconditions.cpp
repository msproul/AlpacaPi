//*****************************************************************************
//*		controller_obsconditions.cpp		(c) 2022 by Mark Sproul
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Sep 25,	2022	<MLS> Created controller_obsconditions.cpp
//*	Oct  3,	2022	<MLS> Fixed connection status for observing conditions window
//*****************************************************************************


#ifdef _ENABLE_CTRL_OBS_CONDITIONS_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"




#include	"alpaca_defs.h"
#include	"alpacadriver_helper.h"
#include	"windowtab_obscond.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"

#include	"sendrequest_lib.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"controller_obsconditions.h"

#define	kWindowWidth	450
#define	kWindowHeight	700


//**************************************************************************************
enum
{
	kTab_ObsCond	=	1,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerObsCond::ControllerObsCond(	const char			*argWindowName,
										struct sockaddr_in	*deviceAddress,
										const int			port,
										const int			deviceNum)

	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{

	cAlpacaDevNum			=	deviceNum;
	cFirstDataRead			=	true;

	memset((void *)&cObsCondProp, 0, sizeof(TYPE_ObsConditionProperties));

	cLastUpdate_milliSecs	=	millis();


	strcpy(cAlpacaDeviceTypeStr,	"observingconditions");

	if (deviceAddress != NULL)
	{
		cDeviceAddress	=	*deviceAddress;
		cPort			=	port;
		cValidIPaddr	=	true;
	}
	SetupWindowControls();

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	lineBuff[128];
	char	ipString[32];

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);
		SetWindowIPaddrInfo(lineBuff, true);

		CheckConnectedState();		//*	check connected and connect if not already connected
	}

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerObsCond::~ControllerObsCond(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	DELETE_OBJ_IF_VALID(cObsConditionsTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//**************************************************************************************
void	ControllerObsCond::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_ObsCond,	"Obs Conditions");
	SetTabText(kTab_DriverInfo,	"Driver Info");
	SetTabText(kTab_About,		"About");


	cObsConditionsTabObjPtr	=	new WindowTabObsCond(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cObsConditionsTabObjPtr != NULL)
	{
		SetTabWindow(kTab_ObsCond,	cObsConditionsTabObjPtr);
		cObsConditionsTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cDriverInfoTabObjPtr		=	new WindowTabDriverInfo(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDriverInfoTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DriverInfo,	cDriverInfoTabObjPtr);
		cDriverInfoTabObjPtr->SetParentObjectPtr(this);
	}

	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
}

//**************************************************************************************
void	ControllerObsCond::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;

	if (cReadStartup)
	{
		AlpacaGetStartupData();
		AlpacaGetCommonProperties_OneAAT(cAlpacaDeviceTypeStr);
		UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);

		cReadStartup	=	false;
	}


	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (cFirstDataRead || (deltaSeconds > 10))
	{
		needToUpdate	=	true;
	}
	if (cForceAlpacaUpdate)	//*	force update is set when a switch is clicked
	{
		needToUpdate		=	true;
		cForceAlpacaUpdate	=	false;
	}

	if (needToUpdate)
	{
		//*	is the IP address valid
		if (cValidIPaddr)
		{
			validData	=	AlpacaGetStatus();
			if (validData == false)
			{
				CONSOLE_DEBUG("Failed to get data");
			}

			UpdateConnectedIndicator(kTab_ObsCond,		kObsCond_Connected);
		}
	}
}

//*****************************************************************************
void	ControllerObsCond::AlpacaProcessSupportedActions(const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString)
{

	if (strcasecmp(valueString, "readall") == 0)
	{
		cHas_readall	=	true;
	}
	else if (strcasecmp(valueString, "foo") == 0)
	{
		//*	you get the idea
	}
}

//*****************************************************************************
//*	Get data One At A Time
//*****************************************************************************
bool	ControllerObsCond::AlpacaGetStartupData_OneAAT(void)
{
//SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
//char			dataString[128];
//int				jjj;
//double			myDoubleValue;

	CONSOLE_DEBUG(__FUNCTION__);


	validData	=	AlpacaGetStringValue(cAlpacaDeviceTypeStr, "description",	NULL,	alpacaString);
	if (validData)
	{
		CONSOLE_DEBUG_W_STR("description\t=", alpacaString);
		strcpy(cAlpacaVersionString, alpacaString);
	}
	return(validData);
}

//*****************************************************************************
bool	ControllerObsCond::AlpacaGetStartupData(void)
{
bool			validData;
//int				jjj;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions(cAlpacaDeviceTypeStr, cAlpacaDevNum);

	if (validData)
	{
//		SetWidgetValid(kTab_ObsCond,		kSwitchBox_Readall,		cHas_readall);
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}

	if (cHas_readall)
	{
		//*	use readall to get the startup data
		validData	=	AlpacaGetStatus_ReadAll(cAlpacaDeviceTypeStr, cAlpacaDevNum);
	}
//-	else
	{
		validData	=	AlpacaGetStartupData_OneAAT();
	}

	cLastUpdate_milliSecs	=	millis();

	return(validData);
}

//*****************************************************************************
//*	Get Status, One At A Time
//*****************************************************************************
bool	ControllerObsCond::AlpacaGetStatus_OneAAT(void)
{
bool			validData;
int				myFailureCount;
double			argDouble;
bool			returnValueIsValid;

	CONSOLE_DEBUG(__FUNCTION__);

	myFailureCount	=	0;
//	double	Averageperiod;			//*	Returns the time period over which observations will be averaged
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"averageperiod",		NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.Averageperiod.ValidData	=	returnValueIsValid;
		cObsCondProp.Averageperiod.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	Cloudcover;				//*	Returns the amount of sky obscured by cloud
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"cloudcover",		NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.Cloudcover.ValidData	=	returnValueIsValid;
		cObsCondProp.Cloudcover.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	Dewpoint;				//*	Returns the atmospheric dew point at the observatory
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"dewpoint",		NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.Dewpoint.ValidData	=	returnValueIsValid;
		cObsCondProp.Dewpoint.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	Humidity;				//*	Returns the atmospheric humidity at the observatory
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"humidity",		NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.Humidity.ValidData	=	returnValueIsValid;
		cObsCondProp.Humidity.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	Pressure_hPa;			//*	Returns the atmospheric pressure at the observatory.
//									//*	hectoPascals
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"pressure",		NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.Pressure.ValidData	=	returnValueIsValid;
		cObsCondProp.Pressure.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	RainRate;				//*	Returns the rain rate at the observatory.
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"rainrate",		NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.RainRate.ValidData	=	returnValueIsValid;
		cObsCondProp.RainRate.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	SkyBrightness;			//*	Returns the sky brightness at the observatory
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"skybrightness",		NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.SkyBrightness.ValidData	=	returnValueIsValid;
		cObsCondProp.SkyBrightness.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	SkyQuality;				//*	Returns the sky quality at the observatory
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"skyquality",	NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.SkyQuality.ValidData	=	returnValueIsValid;
		cObsCondProp.SkyQuality.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	SkyTemperature;	//*	Returns the sky temperature at the observatory
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"skytemperature",	NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.SkyTemperature.ValidData	=	returnValueIsValid;
		cObsCondProp.SkyTemperature.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	StarFWHM;				//*	Returns the seeing at the observatory
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"starfwhm",	NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.StarFWHM.ValidData	=	returnValueIsValid;
		cObsCondProp.StarFWHM.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	Temperature_DegC;		//*	Returns the temperature at the observatory
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"temperature",	NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.Temperature.ValidData	=	returnValueIsValid;
		cObsCondProp.Temperature.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	WindDirection;			//*	Returns the wind direction at the observatory
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"winddirection",	NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.WindDirection.ValidData	=	returnValueIsValid;
		cObsCondProp.WindDirection.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	WindGust;				//*	Returns the peak 3 second wind gust at the observatory over the last 2 minutes
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"windgust",	NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.WindGust.ValidData	=	returnValueIsValid;
		cObsCondProp.WindGust.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}
//	double	WindSpeed;				//*	Returns the wind speed at the observatory.
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"windspeed",	NULL,	&argDouble, &returnValueIsValid);
	if (validData)
	{
		cObsCondProp.WindSpeed.ValidData	=	returnValueIsValid;
		cObsCondProp.WindSpeed.Value		=	argDouble;
	}
	else
	{
		myFailureCount++;
	}

	//*	all done, how many errors
	if (myFailureCount < 2)
	{
		validData	=	true;
	}
	else
	{
		validData	=	false;
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "exit");
	return(validData);
}

//*****************************************************************************
bool	ControllerObsCond::AlpacaGetStatus(void)
{
bool			validData;

bool	previousOnLineState;

//	CONSOLE_DEBUG(__FUNCTION__);

	previousOnLineState	=	cOnLine;
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll(cAlpacaDeviceTypeStr, cAlpacaDevNum);
	}
	else
	{
		validData	=	AlpacaGetStatus_OneAAT();
		AlpacaGetCommonConnectedState(cAlpacaDeviceTypeStr);
	}
//	DumpObservingconditionsProp(&cObsCondProp, __FUNCTION__);

	if (validData)
	{
		if (cOnLine == false)
		{
			//*	if we were previously off line, force reading startup again
			cReadStartup	=	true;
		}
//		SetWidgetBGColor(	kTab_Switch,	kSwitchBox_IPaddr,	CV_RGB(0,	0,	0));
//		SetWidgetTextColor(	kTab_Switch,	kSwitchBox_IPaddr,	CV_RGB(255,	0,	0));

		cOnLine	=	true;

		//*	the data was valid,update the display
		if (cObsConditionsTabObjPtr != NULL)
		{
			cObsConditionsTabObjPtr->UpdateObservationValues(&cObsCondProp);
		}
	}
	else
	{
//		SetWidgetBGColor(	kTab_Switch,	kSwitchBox_IPaddr,	CV_RGB(255,	0,	0));
//		SetWidgetTextColor(	kTab_Switch,	kSwitchBox_IPaddr,	CV_RGB(0,	0,	0));
		cOnLine	=	false;
	}



	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
	return(validData);
}

//*****************************************************************************
void	ControllerObsCond::AlpacaProcessReadAll(	const char	*deviceTypeStr,
													const int	deviceNum,
													const char	*keywordString,
													const char	*valueString)
{

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR(keywordString, valueString);
	if (strcasecmp(keywordString, "connected") == 0)
	{
		cCommonProp.Connected	=  IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "version") == 0)
	{
		//*	"version": "AlpacaPi - V0.2.2-beta build #32",
		strcpy(cAlpacaVersionString, valueString);
	}
	//	double	Averageperiod;			//*	Returns the time period over which observations will be averaged
	else if (strcasecmp(keywordString, "Averageperiod") == 0)
	{
		cObsCondProp.Averageperiod.Value		=	atof(valueString);
		cObsCondProp.Averageperiod.ValidData	=	true;
	}
	//	double	Cloudcover;				//*	Returns the amount of sky obscured by cloud
	else if (strcasecmp(keywordString, "Cloudcover") == 0)
	{
		cObsCondProp.Cloudcover.Value		=	atof(valueString);
		cObsCondProp.Cloudcover.ValidData	=	true;
	}
	//	double	Dewpoint;				//*	Returns the atmospheric dew point at the observatory
	else if (strcasecmp(keywordString, "Dewpoint") == 0)
	{
		cObsCondProp.Dewpoint.Value		=	atof(valueString);
		cObsCondProp.Dewpoint.ValidData	=	true;
	}
	//	double	Humidity;				//*	Returns the atmospheric humidity at the observatory
	else if (strcasecmp(keywordString, "Humidity") == 0)
	{
		cObsCondProp.Humidity.Value		=	atof(valueString);
		cObsCondProp.Humidity.ValidData	=	true;
	}
	//	double	Pressure_hPa;			//*	Returns the atmospheric pressure at the observatory.
	//									//*	hectoPascals
	else if (strcasecmp(keywordString, "Pressure") == 0)
	{
		cObsCondProp.Pressure.Value		=	atof(valueString);
		cObsCondProp.Pressure.ValidData	=	true;
	}
	//	double	RainRate;				//*	Returns the rain rate at the observatory.
	else if (strcasecmp(keywordString, "RainRate") == 0)
	{
		cObsCondProp.RainRate.Value	=	atof(valueString);
		cObsCondProp.RainRate.ValidData	=	true;
	}
	//	double	SkyBrightness;			//*	Returns the sky brightness at the observatory
	else if (strcasecmp(keywordString, "SkyBrightness") == 0)
	{
		cObsCondProp.SkyBrightness.Value	=	atof(valueString);
		cObsCondProp.Cloudcover.ValidData	=	true;
	}
	//	double	SkyQuality;				//*	Returns the sky quality at the observatory
	else if (strcasecmp(keywordString, "SkyQuality") == 0)
	{
		cObsCondProp.SkyQuality.Value		=	atof(valueString);
		cObsCondProp.SkyQuality.ValidData	=	true;
	}
	//	double	SkyTemperature_DegC;	//*	Returns the sky temperature at the observatory
	else if (strcasecmp(keywordString, "SkyTemperature") == 0)
	{
		cObsCondProp.SkyTemperature.Value		=	atof(valueString);
		cObsCondProp.SkyTemperature.ValidData	=	true;
	}
	//	double	StarFWHM;				//*	Returns the seeing at the observatory
	else if (strcasecmp(keywordString, "StarFWHM") == 0)
	{
		cObsCondProp.StarFWHM.Value		=	atof(valueString);
		cObsCondProp.StarFWHM.ValidData	=	true;
	}
	//	double	Temperature_DegC;		//*	Returns the temperature at the observatory
	else if (strcasecmp(keywordString, "Temperature") == 0)
	{
		cObsCondProp.Temperature.Value	=	atof(valueString);
		cObsCondProp.Temperature.ValidData	=	true;
	}
	//	double	WindDirection;			//*	Returns the wind direction at the observatory
	else if (strcasecmp(keywordString, "WindDirection") == 0)
	{
		cObsCondProp.WindDirection.Value		=	atof(valueString);
		cObsCondProp.WindDirection.ValidData	=	true;
	}
	//	double	WindGust;				//*	Returns the peak 3 second wind gust at the observatory over the last 2 minutes
	else if (strcasecmp(keywordString, "WindGust") == 0)
	{
		cObsCondProp.WindGust.Value		=	atof(valueString);
		cObsCondProp.WindGust.ValidData	=	true;
	}
	//	double	WindSpeed;				//*	Returns the wind speed at the observatory.
	else if (strcasecmp(keywordString, "WindSpeed") == 0)
	{
		cObsCondProp.WindSpeed.Value		=	atof(valueString);
		cObsCondProp.WindSpeed.ValidData	=	true;
	}
	////	double	sensordescription,		//*	Return a sensor description
	else if (strcasecmp(keywordString, "sensordescription") == 0)
	{
//		cObsCondProp.sensordescription	=	atof(valueString);
	}
	////	double	timesincelastupdate,	//*	Return the time since the sensor value was last updated
	else if (strcasecmp(keywordString, "timesincelastupdate") == 0)
	{
//		cObsCondProp.timesincelastupdate	=	atof(valueString);
	}
	else
	{
		AlpacaProcessReadAll_Common(	deviceTypeStr,
										deviceNum,
										keywordString,
										valueString);
	}
}

//*****************************************************************************
void	ControllerObsCond::UpdateCommonProperties(void)
{
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);

}

#endif // _ENABLE_CTRL_OBS_CONDITIONS_

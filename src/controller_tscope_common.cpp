//*****************************************************************************
//*		controller_tscope_common.cpp		(c) 2021 by Mark Sproul
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
//*	Jan 23,	2021	<MLS> Created controller_tscope_common.cpp
//*****************************************************************************

//*	this file gets INCLUDED at the end of either controller_telescope OR controller_skytravel



//*****************************************************************************
void	PARENT_CLASS::AlpacaProcessSupportedActions_Telescope(	const int	deviveNum,
																const char	*valueString)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, valueString);

	if (strcasecmp(valueString, "readall") == 0)
	{
	#ifdef _PARENT_IS_TELESCOPE_
		cHas_readall			=	true;
	#else
		cTelescopeHas_readall	=	true;
	#endif // _PARENT_IS_TELESCOPE_
	}
}


//*****************************************************************************
bool	PARENT_CLASS::AlpacaProcessReadAll_Telescope(	const int	deviceNum,
														const char	*keywordString,
														const char *valueString)
{
bool	dataWasHandled;

//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
	dataWasHandled	=	true;
	if (strcasecmp(keywordString, "Declination") == 0)
	{
		cTelescopeProp.Declination	=	atof(valueString);
		Update_TelescopeDeclination();
	}
	else if (strcasecmp(keywordString, "RightAscension") == 0)
	{
		cTelescopeProp.RightAscension	=	atof(valueString);
		Update_TelescopeRtAscension();
	}
	else if (strcasecmp(keywordString, "Tracking") == 0)
	{
		cTelescopeProp.Tracking	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "Slewing") == 0)
	{
		cTelescopeProp.Slewing	=	IsTrueFalse(valueString);
	}
	else
	{
		dataWasHandled	=	false;
	}


#ifdef _PARENT_IS_TELESCOPE_
	if (strcasecmp(keywordString, "version") == 0)
	{
		//*	"version": "AlpacaPi - V0.2.2-beta build #32",
		strcpy(cAlpacaVersionString, valueString);
		SetWidgetText(kTab_Telescope,	kTelescope_AlpacaDrvrVersion,		cAlpacaVersionString);
		SetWidgetText(kTab_DriverInfo,	kDriverInfo_AlpacaDrvrVersion,		cAlpacaVersionString);
		dataWasHandled	=	true;
	}
#endif // _PARENT_IS_TELESCOPE_

	return(dataWasHandled);
}

//*****************************************************************************
void	PARENT_CLASS::ReadOneTelescopeCapability(	const char	*propertyStr,
													const char	*reportedStr,
													bool		*booleanValue)
{

	ReadOneDriverCapability("telescope", propertyStr, reportedStr, booleanValue);
}

//*****************************************************************************
bool	PARENT_CLASS::AlpacaGetStartupData_TelescopeOneAAT(void)
{
bool			validData;
int				myFailureCount;
double			argDoubleMin;
double			argDoubleMax;
char			alpacaString[64];
int				iii;

//	CONSOLE_DEBUG(__FUNCTION__);
#ifndef _PARENT_IS_TELESCOPE_
//	CONSOLE_DEBUG("NOT   _PARENT_IS_TELESCOPE_");
	cDeviceAddress	=	cTelescopeIpAddress;
	cPort			=	cTelescopeIpPort;
	cAlpacaDevNum	=	cTelescopeAlpacaDeviceNum;
#endif

	ReadOneTelescopeCapability("canfindhome",			"CanFindHome",			&cTelescopeProp.CanFindHome);
	ReadOneTelescopeCapability("canpark",				"CanPark",				&cTelescopeProp.CanPark);
	ReadOneTelescopeCapability("canpulseguide",			"CanPulseGuide",		&cTelescopeProp.CanPulseGuide);
	ReadOneTelescopeCapability("cansetdeclinationrate",	"CanSetDeclinationRate",
																				&cTelescopeProp.CanSetDeclinationRate);
	ReadOneTelescopeCapability("cansetguiderates",		"CanSetGuideRates",		&cTelescopeProp.CanSetGuideRates);
	ReadOneTelescopeCapability("cansetpark",			"CanSetPark",			&cTelescopeProp.CanSetPark);

	ReadOneTelescopeCapability("cansetpierside",		"CanSetPierSide",		&cTelescopeProp.CanSetPierSide);
	ReadOneTelescopeCapability("cansetrightascensionrate",
														"CanSetRightAscensionRate",
																				&cTelescopeProp.CanSetRightAscensionRate);
	ReadOneTelescopeCapability("cansettracking",		"CanSetTracking",		&cTelescopeProp.CanSetTracking);
	ReadOneTelescopeCapability("canslew",				"CanSlew",				&cTelescopeProp.CanSlew);
	ReadOneTelescopeCapability("canslewaltaz",			"CanSlewAltAz",			&cTelescopeProp.CanSlewAltAz);
	ReadOneTelescopeCapability("canslewaltazasync",		"CanSlewAltAzAsync",	&cTelescopeProp.CanSlewAltAzAsync);
	ReadOneTelescopeCapability("canslewasync",			"CanSlewAsync",			&cTelescopeProp.CanSlewAsync);
	ReadOneTelescopeCapability("cansync",				"CanSync",				&cTelescopeProp.CanSync);
	ReadOneTelescopeCapability("cansyncaltaz",			"CanSyncAltAz",			&cTelescopeProp.CanSyncAltAz);
	ReadOneTelescopeCapability("canunpark",				"CanUnpark",			&cTelescopeProp.CanUnpark);
	ReadOneTelescopeCapability("doesrefraction",		"DoesRefraction",		&cTelescopeProp.DoesRefraction);


	myFailureCount	=	0;
	//*	There are supposed to be 3 axis, but we only use 2 and the simulator only reports 2
	for (iii = 0; iii< 2; iii++)
	{
		argDoubleMax	=	-1;
		argDoubleMin	=	-1;
		//========================================================
		//api/v1/telescope/0/axisrates?ClientID=1&ClientTransactionID=1234&Axis=0
		sprintf(alpacaString,	"axisrates?Axis=%d",  iii);

		validData	=	AlpacaGetMinMax(	"telescope",
											alpacaString,
											NULL,
											&argDoubleMin,
											&argDoubleMax);
		if (validData)
		{
			cTelescopeProp.AxisRates[iii].Minimum	=	argDoubleMin;
			cTelescopeProp.AxisRates[iii].Maximum	=	argDoubleMax;
//			CONSOLE_DEBUG_W_DBL("argDoubleMin\t=",	argDoubleMin);
//			CONSOLE_DEBUG_W_DBL("argDoubleMax\t=",	argDoubleMax);
		}
		else
		{
			CONSOLE_DEBUG("Failed");
			cReadFailureCnt++;
			myFailureCount++;
		}
	}

	return(true);
}

//*****************************************************************************
//*	Get Status, One At A Time
//*****************************************************************************
bool	PARENT_CLASS::AlpacaGetStatus_TelescopeOneAAT(void)
{
bool			validData;
int				myFailureCount;
double			argDouble;
bool			argBoolean;
//	CONSOLE_DEBUG(__FUNCTION__);
#ifndef _PARENT_IS_TELESCOPE_
//	CONSOLE_DEBUG("NOT   _PARENT_IS_TELESCOPE_");
	cDeviceAddress	=	cTelescopeIpAddress;
	cPort			=	cTelescopeIpPort;
	cAlpacaDevNum	=	cTelescopeAlpacaDeviceNum;
#endif

	myFailureCount	=	0;



	//========================================================
	validData	=	AlpacaGetDoubleValue(	"telescope", "declination",	NULL,	&argDouble);
	if (validData)
	{
		cTelescopeProp.Declination	=	argDouble;
		Update_TelescopeDeclination();
//		CONSOLE_DEBUG_W_DBL("cTelescopeProp.Declination\t=", cTelescopeProp.Declination);
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetDoubleValue(	"telescope", "rightascension",	NULL,	&argDouble);
	if (validData)
	{
		cTelescopeProp.RightAscension	=	argDouble;
		Update_TelescopeRtAscension();
//		CONSOLE_DEBUG_W_DBL("cTelescopeProp.RightAscension\t=", cTelescopeProp.RightAscension);
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetBooleanValue(	"telescope", "tracking",	NULL,	&argBoolean);
	if (validData)
	{
		cTelescopeProp.Tracking	=	argBoolean;
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetBooleanValue(	"telescope", "slewing",	NULL,	&argBoolean);
	if (validData)
	{
		cTelescopeProp.Slewing	=	argBoolean;
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}

	if (myFailureCount < 2)
	{
		validData	=	true;
	}
	else
	{
		validData	=	false;
	}
	return(validData);
}


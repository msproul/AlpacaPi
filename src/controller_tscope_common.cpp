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
//*	May 12,	2022	<MLS> Added support for TrackingRate
//*	May 29,	2022	<MLS> Parsing more fields in AlpacaProcessReadAll_Telescope()
//*	May 30,	2022	<MLS> Added more fields to AlpacaGetStatus_TelescopeOneAAT()
//*	May 31,	2022	<MLS> Added ability to determine if driver supports refraction
//*	Jun  3,	2022	<MLS> Added MountData_SaveRA() & MountData_SaveDec()
//*	Sep 28,	2022	<MLS> Consolidated 3 Save routines into one
//*	Sep 28,	2002	<MLS> Added MountData_SaveData()
//*	Sep 28,	2002	<MLS> Deleted MountData_SaveRA, MountData_SaveHA, MountData_SaveDec
//*****************************************************************************

//*****************************************************************************
//!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//*
//*	this file gets INCLUDED at the end of either controller_telescope OR controller_skytravel
//*	PARENT_CLASS is defined in those files
//*****************************************************************************

#if defined(_PARENT_IS_SKYTRAVEL_) && !defined( __ARM_ARCH )
#include	"MountData.h"

double	gHourAngleData[kMaxMountData + 10];
double	gRightAsceData[kMaxMountData + 10];
double	gDeclinationData[kMaxMountData + 10];

//-static int	gSecsSinceMidnight	=	0;
static bool	gInitMountData		=	true;

//*****************************************************************************
void	MountData_Init(void)
{
int		iii;

	for (iii=0; iii<kMaxMountData; iii++)
	{
		gHourAngleData[iii]	=	0.0;
		gRightAsceData[iii]	=	0.0;
		gDeclinationData[iii]	=	0.0;
	}
}

//*****************************************************************************
enum
{
	kMountData_HA	=	0,
	kMountData_RA,
	kMountData_DEC
};

//*****************************************************************************
//*	to make this cleaner, all three data points are saved by the same routine into different buffers
//*****************************************************************************
static void	MountData_SaveData(const int whichData, const double newEntryInDegrees)
{
time_t		currentTime;
int			secsPastMignight;
int			tableIndex;
int			jjj;
double		myDataDegrees;
double		*arrayPointer;

//	CONSOLE_DEBUG_W_DBL("newEntryInDegrees\t\t=", newEntryInDegrees);
	if (gInitMountData)
	{
		MountData_Init();
		gInitMountData	=	false;
	}


	myDataDegrees	=	newEntryInDegrees;
	if (myDataDegrees > 180.0)
	{
		myDataDegrees	-=	360.0;
	}
//	CONSOLE_DEBUG_W_DBL("myDataDegrees\t\t=", myDataDegrees);
	currentTime			=	time(NULL);

	secsPastMignight	=	currentTime % kSecondsPerDay;
	tableIndex			=	secsPastMignight / 4;
	if (tableIndex < kMaxMountData)
	{

		switch(whichData)
		{
			case kMountData_HA:
				arrayPointer	=	gHourAngleData;
				break;

			case kMountData_RA:
				arrayPointer	=	gRightAsceData;
				break;

			case kMountData_DEC:
				arrayPointer	=	gDeclinationData;
				break;

			default:
				CONSOLE_ABORT("Invalid data index");
				break;
		}
		arrayPointer[tableIndex]	=	myDataDegrees;

		//*	there is a possibility that a value might not be filled in in the table.
		//*	to protect against that, we are going to put the next 5 values the same as this one
		//*	the buffer is 10 values larger so we wont over flow the buffer
		for (jjj=0; jjj<5; jjj++)
		{
			arrayPointer[tableIndex + jjj]	=	myDataDegrees;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("tableIndex      \t=", tableIndex);
	}
}

#endif //	defined(_PARENT_IS_SKYTRAVEL_) && !defined( __ARM_ARCH )

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
	//=================================================================================
	if (strcasecmp(keywordString,			"AlginmentMode") == 0)
	{
	//+	cTelescopeProp.AlginmentMode	=	IsTrueFalse(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"AtHome") == 0)
	{
		cTelescopeProp.AtHome	=	IsTrueFalse(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"AtPark") == 0)
	{
		cTelescopeProp.AtPark	=	IsTrueFalse(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"Declination") == 0)
	{
		cTelescopeProp.Declination	=	AsciiToDouble(valueString);
		Update_TelescopeDeclination();
#if defined(_PARENT_IS_SKYTRAVEL_) && !defined( __ARM_ARCH )
		MountData_SaveData(kMountData_DEC, cTelescopeProp.Declination);
#endif
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"DoesRefraction") == 0)
	{
		cTelescopeProp.DoesRefraction	=	IsTrueFalse(valueString);
		Update_TelescopeDeclination();
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"PhysicalSideOfPier") == 0)
	{
		cTelescopeProp.PhysicalSideOfPier	=	(TYPE_PierSide)atoi(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"HourAngle") == 0)
	{
		cTelescopeProp.hourAngleIsValid	=	true;
		cTelescopeProp.HourAngle		=	AsciiToDouble(valueString);
		if (cTelescopeProp.HourAngle < 0.0)
		{
			cTelescopeProp.HourAngle	+=	24.0;
		}
	}
#if defined(_PARENT_IS_SKYTRAVEL_) && !defined( __ARM_ARCH )
	//=================================================================================
	else if (strcasecmp(keywordString,		"HourAngle-degrees") == 0)
	{
		cTelescopeProp.HourAngle_deg	=	AsciiToDouble(valueString);
		MountData_SaveData(kMountData_HA, cTelescopeProp.HourAngle_deg);
	}
#endif // _PARENT_IS_SKYTRAVEL_
	//=================================================================================
	else if (strcasecmp(keywordString,		"RightAscension") == 0)
	{
		cTelescopeProp.RightAscension	=	AsciiToDouble(valueString);
		Update_TelescopeRtAscension();
#if defined(_PARENT_IS_SKYTRAVEL_) && !defined( __ARM_ARCH )
		MountData_SaveData(kMountData_RA, cTelescopeProp.RightAscension * 15.0);
#endif
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"SideOfPier") == 0)
	{
		cTelescopeProp.SideOfPier	=	(TYPE_PierSide)atoi(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"Slewing") == 0)
	{
		cTelescopeProp.Slewing	=	IsTrueFalse(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"Tracking") == 0)
	{
		cTelescopeProp.Tracking	=	IsTrueFalse(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString,		"TrackingRate") == 0)
	{
		cTelescopeProp.TrackingRate	=	(TYPE_DriveRates)atoi(valueString);
	}
	//=================================================================================
	else
	{
//		CONSOLE_DEBUG_W_2STR("ignored=",	keywordString, valueString);
		dataWasHandled	=	false;
	}

#ifdef _PARENT_IS_TELESCOPE_
	if (strcasecmp(keywordString, "version") == 0)
	{
		//*	"version": "AlpacaPi - V0.2.2-beta build #32",
		strcpy(cAlpacaVersionString, valueString);
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
double			argDouble;
char			alpacaString[64];
int				iii;
bool			argBoolean;

	CONSOLE_DEBUG(__FUNCTION__);
#ifndef _PARENT_IS_TELESCOPE_
//	CONSOLE_DEBUG("NOT   _PARENT_IS_TELESCOPE_");
	cDeviceAddress	=	cTelescopeIpAddress;
	cPort			=	cTelescopeIpPort;
	cAlpacaDevNum	=	cTelescopeAlpacaDeviceNum;
#endif

	ReadOneTelescopeCapability("canfindhome",			"CanFindHome",			&cTelescopeProp.CanFindHome);
	ReadOneTelescopeCapability("canmoveaxis?Axis=0",	"CanMoveAxis-RA",		&cTelescopeProp.CanMoveAxis[0]);
	ReadOneTelescopeCapability("canmoveaxis?Axis=1",	"CanMoveAxis-DEC",		&cTelescopeProp.CanMoveAxis[1]);
	ReadOneTelescopeCapability("canmoveaxis?Axis=2",	"CanMoveAxis-3",		&cTelescopeProp.CanMoveAxis[2]);
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

	//========================================================
	//*	we have to look at DoesRefraction again, to see if it is supported
	validData	=	AlpacaGetBooleanValue(	"telescope", "doesrefraction",	NULL,	&argBoolean);
	if (validData)
	{
		if (cLastAlpacaErrNum == kASCOM_Err_Success)
		{
			cTelescopeProp.driverSupportsRefraction	=	true;
		}
	}



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
			CONSOLE_DEBUG_W_DBL("argDoubleMin\t=",	argDoubleMin);
			CONSOLE_DEBUG_W_DBL("argDoubleMax\t=",	argDoubleMax);
		}
		else
		{
			CONSOLE_DEBUG("Failed");
			cReadFailureCnt++;
			myFailureCount++;
		}
	}

	//========================================================
	validData	=	AlpacaGetDoubleValue(	"telescope", "sitelatitude",	NULL,	&argDouble);
	if (validData)
	{
		cTelescopeProp.SiteLatitude	=	argDouble;
	//	CONSOLE_DEBUG_W_DBL("SiteLatitude\t=", cTelescopeProp.SiteLatitude);
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetDoubleValue(	"telescope", "sitelongitude",	NULL,	&argDouble);
	if (validData)
	{
		cTelescopeProp.SiteLongitude	=	argDouble;
	//	CONSOLE_DEBUG_W_DBL("SiteLongitude\t=", cTelescopeProp.SiteLongitude);
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetDoubleValue(	"telescope", "siteelevation",	NULL,	&argDouble);
	if (validData)
	{
		cTelescopeProp.SiteElevation	=	argDouble;
	//	CONSOLE_DEBUG_W_DBL("SiteElevation\t=", cTelescopeProp.SiteElevation);
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
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
int				argInt;
//	CONSOLE_DEBUG(__FUNCTION__);
#ifndef _PARENT_IS_TELESCOPE_
//	CONSOLE_DEBUG("NOT   _PARENT_IS_TELESCOPE_");
	cDeviceAddress	=	cTelescopeIpAddress;
	cPort			=	cTelescopeIpPort;
	cAlpacaDevNum	=	cTelescopeAlpacaDeviceNum;
#endif

	myFailureCount	=	0;

	//========================================================
	validData	=	AlpacaGetBooleanValue(	"telescope", "athome",	NULL,	&argBoolean);
	if (validData)
	{
		cTelescopeProp.AtHome	=	argBoolean;
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetBooleanValue(	"telescope", "atpark",	NULL,	&argBoolean);
	if (validData)
	{
		cTelescopeProp.AtPark	=	argBoolean;
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetBooleanValue(	"telescope", "ispulseguiding",	NULL,	&argBoolean);
	if (validData)
	{
		cTelescopeProp.IsPulseGuiding	=	argBoolean;
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}

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
	validData	=	AlpacaGetBooleanValue(	"telescope", "doesrefraction",	NULL,	&argBoolean);
	if (validData)
	{
		cTelescopeProp.DoesRefraction	=	argBoolean;
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetDoubleValue(	"telescope", "guideratedeclination",	NULL,	&argDouble);
	if (validData)
	{
		cTelescopeProp.GuideRateDeclination	=	argDouble;
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetDoubleValue(	"telescope", "guideraterightascension",	NULL,	&argDouble);
	if (validData)
	{
		cTelescopeProp.GuideRateRightAscension	=	argDouble;
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
	validData	=	AlpacaGetIntegerValue(	"telescope", "trackingrate",	NULL,	&argInt);
	if (validData)
	{
		cTelescopeProp.TrackingRate	=	(TYPE_DriveRates)argInt;
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


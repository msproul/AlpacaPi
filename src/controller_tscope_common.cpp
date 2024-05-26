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
//*	Sep 28,	2022	<MLS> Added MountData_SaveData()
//*	Sep 28,	2022	<MLS> Deleted MountData_SaveRA, MountData_SaveHA, MountData_SaveDec
//*	Jun 25,	2023	<MLS> Added AlpacaGetCapabilities()
//*	Jun 29,	2023	<MLS> Added AlpacaProcessReadAll_TelescopeIdx()
//*	Jan 15,	2024	<MLS> Added support for HourAngle_deg to AlpacaProcessReadAll_TelescopeIdx()
//*****************************************************************************

#ifndef _TELESCOPE_CMDS_H_
	#include	"telescope_AlpacaCmds.h"
#endif

//*****************************************************************************
//!!!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//*
//*	this file gets INCLUDED at the end of either controller_telescope OR controller_skytravel
//*	PARENT_CLASS is defined in those files
//*****************************************************************************


//*****************************************************************************
#include	"MountData.h"
#if defined(_PARENT_IS_SKYTRAVEL_) && !defined( __ARM_ARCH )
	static void	MountData_SaveData(const int whichData, const double newEntryInDegrees);
#else
//*****************************************************************************
static void	MountData_SaveData(const int whichData, const double newEntryInDegrees)
{
	//	Do nothing
}
#endif


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
bool	PARENT_CLASS::AlpacaProcessReadAll_TelescopeIdx(const int	deviceNum,
														const int	keywordEnum,
														const char *valueString)
{
bool	dataWasHandled;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("json=",	valueString);
	dataWasHandled	=	true;
	switch(keywordEnum)
	{
		case kCmd_Telescope_alignmentmode:			//*	Returns the current mount alignment mode
			cTelescopeProp.AlginmentMode	=	(TYPE_AlignmentModes)atoi(valueString);
			break;

		case kCmd_Telescope_altitude:				//*	Returns the mount's Altitude above the horizon.
			cTelescopeProp.Altitude	=	atof(valueString);
			break;

		case kCmd_Telescope_aperturearea:			//*	Returns the telescope's aperture.
			cTelescopeProp.ApertureArea	=	atof(valueString);
			break;

		case kCmd_Telescope_aperturediameter:		//*	Returns the telescope's effective aperture.
			cTelescopeProp.ApertureDiameter	=	atof(valueString);
			break;

		case kCmd_Telescope_athome:					//*	Indicates whether the mount is at the home position.
			cTelescopeProp.AtHome	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_atpark:					//*	Indicates whether the telescope is at the park position.
			cTelescopeProp.AtPark	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_azimuth:				//*	Returns the telescope's azimuth.
			break;

		case kCmd_Telescope_canfindhome:			//*	Indicates whether the mount can find the home position.
			cTelescopeProp.CanFindHome	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_canpark:				//*	Indicates whether the telescope can be parked.
			cTelescopeProp.CanPark	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_canpulseguide:			//*	Indicates whether the telescope can be pulse guided.
			cTelescopeProp.CanPulseGuide	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_cansetdeclinationrate:	//*	Indicates whether the DeclinationRate property can be changed.
			cTelescopeProp.CanSetDeclinationRate	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_cansetguiderates:		//*	Indicates whether the DeclinationRate property can be changed.
			cTelescopeProp.CanSetGuideRates	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_cansetpark:				//*	Indicates whether the telescope park position can be set.
			cTelescopeProp.CanSetPark	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_cansetpierside:			//*	Indicates whether the telescope SideOfPier can be set.
			cTelescopeProp.CanSetPierSide	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_cansetrightascensionrate://*	Indicates whether the RightAscensionRate property can be changed.
			cTelescopeProp.CanSetRightAscensionRate	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_cansettracking:			//*	Indicates whether the Tracking property can be changed.
			cTelescopeProp.CanSetTracking	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_canslew:				//*	Indicates whether the telescope can slew synchronously.
			cTelescopeProp.CanSlew	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_canslewaltaz:			//*	Indicates whether the telescope can slew synchronously to AltAz coordinates.
			cTelescopeProp.CanSlewAltAz	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_canslewaltazasync:		//*	Indicates whether the telescope can slew asynchronously to AltAz coordinates.
			cTelescopeProp.CanSlewAltAzAsync	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_canslewasync:			//*	Indicates whether the telescope can slew asynchronously.
			cTelescopeProp.CanSlewAsync	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_cansync:				//*	Indicates whether the telescope can sync to equatorial coordinates.
			cTelescopeProp.CanSync	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_cansyncaltaz:			//*	Indicates whether the telescope can sync to local horizontal coordinates.
			cTelescopeProp.CanSyncAltAz	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_canunpark:				//*	True if this telescope is capable of programmed unparking (Unpark() method).
			cTelescopeProp.CanUnpark	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_declination:			//*	Returns the telescope's declination.
			cTelescopeProp.Declination	=	AsciiToDouble(valueString);
			Update_TelescopeDeclination();
			MountData_SaveData(kMountData_DEC, cTelescopeProp.Declination);
			break;

		case kCmd_Telescope_declinationrate:		//*	Returns the telescope's declination tracking rate.
			cTelescopeProp.DeclinationRate	=	atof(valueString);
			break;

		case kCmd_Telescope_doesrefraction:			//*	Indicates whether atmospheric refraction is applied to coordinates.
			cTelescopeProp.DoesRefraction	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_equatorialsystem:		//*	Returns the current equatorial coordinate system used by this telescope.
			break;

		case kCmd_Telescope_focallength:			//*	Returns the telescope's focal length in meters.
			cTelescopeProp.FocalLength		=	atof(valueString);
			break;

		case kCmd_Telescope_guideratedeclination:	//*	Returns the current Declination rate offset for telescope guiding
			cTelescopeProp.GuideRateDeclination		=	atof(valueString);
			break;

		case kCmd_Telescope_guideraterightascension://*	Returns the current RightAscension rate offset for telescope guiding
			cTelescopeProp.GuideRateRightAscension		=	atof(valueString);
			break;

		case kCmd_Telescope_ispulseguiding:			//*	Indicates whether the telescope is currently executing a PulseGuide command
			cTelescopeProp.IsPulseGuiding	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_rightascension:			//*	Returns the telescope's right ascension coordinate.
			cTelescopeProp.RightAscension	=	AsciiToDouble(valueString);
			Update_TelescopeRtAscension();
			MountData_SaveData(kMountData_RA, cTelescopeProp.RightAscension * 15.0);
			break;

		case kCmd_Telescope_rightascensionrate:		//*	Returns the telescope's right ascension tracking rate.
			cTelescopeProp.RightAscensionRate		=	atof(valueString);
			break;

		case kCmd_Telescope_sideofpier:				//*	Returns the mount's pointing state.
			cTelescopeProp.SideOfPier	=	(TYPE_PierSide)atoi(valueString);
			Update_TelescopeSideOfPier();
			break;

		case kCmd_Telescope_siderealtime:			//*	Returns the local apparent sidereal time.
			break;

		case kCmd_Telescope_siteelevation:			//*	Returns the observing site's elevation above mean sea level.
			cTelescopeProp.SiteElevation		=	atof(valueString);
			break;

		case kCmd_Telescope_sitelatitude:			//*	Returns the observing site's latitude.
			cTelescopeProp.SiteLatitude		=	atof(valueString);
			break;

		case kCmd_Telescope_sitelongitude:			//*	Returns the observing site's longitude.
			cTelescopeProp.SiteLongitude		=	atof(valueString);
			break;

		case kCmd_Telescope_slewing:				//*	Indicates whether the telescope is currently slewing.
			cTelescopeProp.Slewing	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_slewsettletime:			//*	Returns the post-slew settling time.
			break;

		case kCmd_Telescope_targetdeclination:		//*	Returns the current target declination.
			break;

		case kCmd_Telescope_targetrightascension:	//*	Returns the current target right ascension.
			break;

		case kCmd_Telescope_tracking:				//*	Indicates whether the telescope is tracking.
			cTelescopeProp.Tracking	=	IsTrueFalse(valueString);
			break;

		case kCmd_Telescope_trackingrate:			//*	Returns the current tracking rate.
			cTelescopeProp.TrackingRate	=	(TYPE_DriveRates)atoi(valueString);
			break;

		case kCmd_Telescope_trackingrates:			//*	Returns a collection of supported DriveRates values.
			break;

		case kCmd_Telescope_utcdate:				//*	Returns the UTC date/time of the telescope's internal clock.
			break;

		//--------------------------------------------------------------------
		//*	these are the methods
		case kCmd_Telescope_axisrates:				//*	Returns the rates at which the telescope may be moved about the specified axis.
			break;

		case kCmd_Telescope_canmoveaxis:			//*	Indicates whether the telescope can move the requested axis.
			break;

		case kCmd_Telescope_destinationsideofpier:	//*	Predicts the pointing state after a German equatorial mount slews to given coordinates.
			break;

		//---------------------------------------------------------------
		//*	added by MLS
		case kCmd_Telescope_hourangle:
			cTelescopeProp.hourAngleIsValid	=	true;
			cTelescopeProp.HourAngle		=	AsciiToDouble(valueString);
			if (cTelescopeProp.HourAngle < 0.0)
			{
				cTelescopeProp.HourAngle	+=	24.0;
			}
			break;

		case kCmd_Telescope_HourAngleDegrees:
			cTelescopeProp.HourAngle_deg	=	AsciiToDouble(valueString);
			MountData_SaveData(kMountData_HA, cTelescopeProp.HourAngle_deg);
			break;

		case kCmd_Telescope_physicalsideofpier:
			cTelescopeProp.PhysicalSideOfPier	=	(TYPE_PierSide)atoi(valueString);
			Update_TelescopeSideOfPier();
			break;

		//*	returns array of 3 (roll, pitch, yaw) acceleration vectors
		case kCmd_Telescope_imu:
//			CONSOLE_DEBUG("kCmd_Telescope_imu");
			break;

		case kCmd_Telescope_IMU_Roll:
//			CONSOLE_DEBUG("kCmd_Telescope_IMU_Roll");
			cTelescopeProp.IMU_Roll		=	atof(valueString);
			break;

		case kCmd_Telescope_IMU_Pitch:
			cTelescopeProp.IMU_Pitch	=	atof(valueString);
			break;

		case kCmd_Telescope_IMU_Yaw:
			cTelescopeProp.IMU_Yaw		=	atof(valueString);
			break;


		default:
			CONSOLE_DEBUG_W_NUM("keywordEnum\t=",	keywordEnum);
			CONSOLE_DEBUG_W_STR("valueString\t=",	valueString);
			dataWasHandled	=	false;
			break;
	}
	return(dataWasHandled);
}

//*****************************************************************************
void	PARENT_CLASS::ReadOneTelescopeCapability(	const char	*propertyStr,
													const char	*reportedStr,
													bool		*booleanValue)
{
	ReadOneDriverCapability("telescope", propertyStr, reportedStr, booleanValue);
}

#ifdef _PARENT_IS_SKYTRAVEL_
//*****************************************************************************
void	PARENT_CLASS::AlpacaGetCapabilities(void)
{
	//*	this is a dummy place holder, the real one is in controller_telescope.cpp
	//*	this one is for the skytravel controller
	CONSOLE_DEBUG(__FUNCTION__);
}
#endif // _PARENT_IS_SKYTRAVEL_


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
	SetCommandLookupTable(gTelescopeCmdTable);

	//========================================================
	//*	we have to look at DoesRefraction again, to see if it is supported
	CONSOLE_DEBUG("Sending cmd: doesrefraction");
	validData	=	AlpacaGetBooleanValue(	"telescope", "doesrefraction",	NULL,	&argBoolean);
	if (validData)
	{
		if (cLastAlpacaErrNum == kASCOM_Err_Success)
		{
			cTelescopeProp.driverSupportsRefraction	=	true;
		}
	}
//	AlpacaGetCapabilities();

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
	CONSOLE_DEBUG("exit");

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
	CONSOLE_DEBUG(__FUNCTION__);
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
	CONSOLE_DEBUG("Sending cmd: doesrefraction");
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


#if defined(_PARENT_IS_SKYTRAVEL_) && !defined( __ARM_ARCH )

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
		gHourAngleData[iii]		=	0.0;
		gRightAsceData[iii]		=	0.0;
		gDeclinationData[iii]	=	0.0;
	}
}


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

	//*	compute the index into the 24 hour arrays
	//*	One entry ever 4 seconds
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

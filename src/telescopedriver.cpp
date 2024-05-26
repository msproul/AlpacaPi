//**************************************************************************
//*	Name:			telescopedriver.cpp
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:	C++ Driver for Alpaca protocol
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
//*
//*	Usage notes:	This driver does not implement any actual device,
//*					you must create a sub-class that does the actual control
//*
//*	References:		https://ascom-standards.org/api/
//*					https://ascom-standards.org/Help/Developer/html/N_ASCOM_DeviceInterface.htm
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec  5,	2020	<MLS> Created telescopedriver.cpp
//*	Dec  5,	2020	<MLS> CONFORM-telescope -> lots of errors
//*	Dec  5,	2020	<MLS> CONFORM-telescope -> 25 errors, 0 warnings and 2 issues
//*	Dec  6,	2020	<MLS> CONFORM-telescope -> tracking rates disabled
//*	Dec  6,	2020	<MLS> CONFORM-telescope -> 4 errors, 0 warnings and 0 issues
//*	Dec  6,	2020	<MLS> CONFORM-telescope - tracking rates enabled
//*	Dec  6,	2020	<MLS> CONFORM-telescope -> 35 errors, 0 warnings and 3 issues
//*	Dec  7,	2020	<MLS> CONFORM-telescope -> 0 errors, 0 warnings and 6 issues
//*	Jan 22,	2021	<MLS> Added Telescope_SyncToRA_DEC()
//*	Jan 22,	2021	<MLS> Added Telescope_SlewToRA_DEC()
//*	Jan 23,	2021	<MLS> Added Telescope_TrackingOnOff()
//*	Jan 24,	2021	<MLS> Converted TelescopeDriver to use properties struct
//*	Feb 15,	2021	<MLS> Added Telescope_MoveAxis()
//*	Apr 20,	2021	<MLS> Added Telescope_FindHome(), Telescope_SetPark()
//*	Apr 20,	2021	<MLS> Added Telescope_SlewToAltAz(), Telescope_UnPark()
//*	Apr 21,	2021	<MLS> Finished stubbing out the routines need in the subclass
//*	Feb 28,	2022	<MLS> Put_SyncToAltAz() now returns InvalidOperation if tracking is false
//*	Feb 28,	2022	<MLS> Added cDriverSupports_Refraction
//*	Mar  2,	2022	<MLS> Setting Connected now working for telescope driver
//*	Mar 25,	2022	<MLS> Added RunStateMachine() to telescopedriver
//*	May 13,	2022	<MLS> Made Get_AxisRates() understand specified axis number
//*	May 28,	2022	<MLS> Added Put_SideOfPier()
//*	May 28,	2022	<MLS> Added Get_PhysicalSideOfPier()
//*	May 28,	2022	<MLS> Added cDriverSupports_LimitSwitches
//*	May 28,	2022	<MLS> Added Telescope_GetLimitSwitchStatus()
//*	May 30,	2022	<MLS> If observatory settings is valid, copy to telescope properties
//*	May 30,	2022	<MLS> Added cDriverSupports_SlewSettleTime
//*	May 30,	2022	<MLS> Fixed CanMoveAxis to handle 3 axis as per the spec
//*	May 31,	2022	<MLS> Added Get_HourAngle()
//*	May 31,	2022	<MLS> Added Telescope_GetPhysicalSideOfPier()
//*	Jun  1,	2022	<MLS> Added IMU support for RA Hour Angle (compile time option)
//*	Jun 15,	2022	<MLS> Updated Telescope_GetPhysicalSideOfPier() to know about IMU
//*	Jun 17,	2022	<MLS> Working on Hour Angle
//*	Jun 18,	2022	<MLS> Cleaning up how Park is handled
//*	Jun 18,	2023	<MLS> Added DeviceState_Add_Content() to telescope driver
//*	Jun 21,	2023	<MLS> Added UTCDate to DeviceState output
//*	Jan 15,	2024	<MLS> Added Get_IMU()
//*	Jan 16,	2024	<MLS> Added Telescope_CalculateSideOfPier()
//*	Feb 11,	2024	<MLS> Added IMU-Roll, IMU-Pitch, & IMU-Yaw to telescope readall
//*	Apr 24,	2024	<MLS> Working on GPS input for telescope site location
//*	Apr 24,	2024	<MLS> Added GPS_TelescopeThread()
//*	Apr 24,	2024	<MLS> Added Set_SiteLatitude(), Set_SiteLongitude(), Set_SiteAltitude()
//*	Apr 25,	2024	<MLS> Added averaging to Lat/Lon updates from GPS
//*	Apr 28,	2024	<MLS> Added DumpTelescopeDriverStruct()
//*	May 10,	2024	<MLS> Added Telescope_TrackingRate()
//*	May 13,	2024	<MLS> Fixed logic in Put_SlewToTargetAsync() to match Alpaca specs
//*	May 13,	2024	<MLS> Added Telescope_CalculateDestinationSideOfPier()
//*	May 14,	2024	<MLS> Fixed no Park error in FindHome(), MoveAxis() && SlewToCoordinatesAsync()
//*	May 15,	2024	<MLS> Added range checks to MoveAxis() to satisfy CONFORM
//*	May 17,	2024	<MLS> Added ExtractRaDecArguments() & ExtractAltAzArguments()
//*	May 17,	2024	<MLS> Added http error 400 processing to telescope driver
//*****************************************************************************


//*****************************************************************************
//	http://www.bbastrodesigns.com/equatTrackingRatesCalc.html
//
//	Years ago E.S. King developed an algorithm that took into account refraction in
//	calculating a telescope's drive rate. He was able to achieve multi-hour unguided
//	exposures by slightly altering the drive rate of the gravity powered mechanical drive
//	of his refractor at periodic intervals. He would add or remove very small weights.
//	His table of tracking rates was consolidated to one best average rate and become
//	known as the King Rate. You can see that for much of the sky, the tracking rate is
//	slightly less than the sidereal rate of 15.041 arcseconds per second.
//	You can also see curiosities such as the tracking rate under the pole is faster
//	than the sidereal rate. You can also see artifacts caused by the real vs refracted
//	pole when comparing the tracking rate calculated by time interval which uses the real pole,
//	to the King Rate which uses the refracted pole.
//
//	Today the King Rate is not much discussed because of auto-guiding, periodic error in
//	popular gear sets and computer controlled altazimuth telescopes.
//
//	E.S. King was one of our great astronomers, contributing to astrophotography,
//	photometry, telescope drives, mirror testing and the cold camera.
//	His book, "The History of the Telescope", is still the book that others are compared against.
//	You can read some of his writings in the Amateur Telescope Making book series.
//	A crater on the Moon is co-named for him.
//*****************************************************************************
//2024-05-17	09:46:42.593 Found 1 error, 38 issues and 130 information messages.
//2024-05-17	10:14:38.089 Found 1 error, 37 issues and 130 information messages.
//2024-05-17	10:27:22.224 Found 2 errors, 36 issues and 130 information messages.
//2024-05-17	11:18:22.179 Found 2 errors, 33 issues and 130 information messages.
//2024-05-17	11:24:16.267 Found 1 error, 23 issues and 130 information messages.
//2024-05-17	12:06:24.746 Found 1 error, 12 issues and 129 information messages.
//2024-05-17	12:24:23.699 Found 1 error, 11 issues and 129 information messages.
//2024-05-17	12:39:57.603 Found 1 error, 4 issues and 130 information messages.
//2024-05-17	13:19:53.095 Found 1 error, 4 issues and 130 information messages.

//-----------------------------------------------------------------------------------------

#ifdef _ENABLE_TELESCOPE_

#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"JsonResponse.h"
#include	"sidereal.h"

#include	"observatory_settings.h"
#ifdef _ENABLE_GLOBAL_GPS_
//	#include	"gps_data.h"
	#include	"ParseNMEA.h"
#endif

//----------------------------------------------------------------------
#include	"telescopedriver.h"

#ifdef _ENABLE_TELESCOPE_EXP_SCI_
	#include	"telescopedriver_ExpSci.h"
#endif

#ifdef _ENABLE_TELESCOPE_LX200_
	#include	"telescopedriver_lx200.h"
#endif

#ifdef _ENABLE_TELESCOPE_RIGEL_
	#include	"telescopedriver_Rigel.h"
#endif

#ifdef _ENABLE_TELESCOPE_SKYWATCH_
	#include	"telescopedriver_skywatch.h"
#endif

#ifdef _ENABLE_TELESCOPE_SERVO_
	#include	"telescopedriver_servo.h"
#endif

#ifdef _ENABLE_TELESCOPE_SIMULATOR_
	#include	"telescopedriver_sim.h"
#endif

#ifdef _ENABLE_IMU_
	#include "imu_lib.h"
#endif

#include	"telescope_AlpacaCmds.h"
#include	"telescope_AlpacaCmds.cpp"

#ifdef _ENABLE_GLOBAL_GPS_
	static void	*GPS_TelescopeThread(void *arg);
#endif

//**************************************************************************************
int	CreateTelescopeObjects(void)
{
int	telescopeCnt;

	CONSOLE_DEBUG(__FUNCTION__);

	telescopeCnt	=	0;


#ifdef _ENABLE_TELESCOPE_EXP_SCI_
	telescopeCnt	+=	CreateTelescopeObjects_ExploreScientific();
#endif

#ifdef _ENABLE_TELESCOPE_LX200_
	new TelescopeDriverLX200(kDevCon_Ethernet, "192.168.1.104:49152");
	telescopeCnt++;
#endif

#ifdef _ENABLE_TELESCOPE_RIGEL_
//	new TelescopeDriverRigel(kDevCon_Custom, "");
	new TelescopeDriverRigel();
	telescopeCnt++;
#endif

#ifdef _ENABLE_TELESCOPE_SKYWATCH_
	new TelescopeDriverSkyWatch(kDevCon_Serial, "/dev/ttyS0");
	telescopeCnt++;
#endif

#ifdef _ENABLE_TELESCOPE_SERVO_
	new TelescopeDriverServo();
	telescopeCnt++;
#endif

#ifdef _ENABLE_TELESCOPE_SIMULATOR_
	CreateTelescopeObjects_Simulator();
	telescopeCnt++;
#endif
	return(telescopeCnt);
}


//**************************************************************************************
TelescopeDriver::TelescopeDriver(void)
	:AlpacaDriver(kDeviceType_Telescope)
{
int		iii;
	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name, "Telescope");
	cCommonProp.InterfaceVersion	=	3;
	cDriverCmdTablePtr				=	gTelescopeCmdTable;

	//--------------------------------------------------------------------
	//*	set the defaults, everything to false or zero
	memset((void *)&cTelescopeProp, 0, sizeof(TYPE_TelescopeProperties));
	cTelescopeProp.AlginmentMode		=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.EquatorialSystem		=	kECT_equOther;
	cTelescopeProp.SideOfPier			=	kPierSide_pierUnknown;
	cTelescopeProp.PhysicalSideOfPier	=	kPierSide_NotAvailable;
	cTelescopeProp.TrackingRate			=	kDriveRate_driveSidereal;

	//*	set them all to false, let the sub class decide what it can do
	cTelescopeProp.CanFindHome					=	false;
	cTelescopeProp.CanMoveAxis[kAxis_RA]		=	false;
	cTelescopeProp.CanMoveAxis[kAxis_DEC]		=	false;
	cTelescopeProp.CanMoveAxis[kAxis_Tertiary]	=	false;
	cTelescopeProp.CanPark						=	false;
	cTelescopeProp.CanPulseGuide				=	false;
	cTelescopeProp.CanSetDeclinationRate		=	false;
	cTelescopeProp.CanSetGuideRates				=	false;
	cTelescopeProp.CanSetPark					=	false;
	cTelescopeProp.CanSetPierSide				=	false;
	cTelescopeProp.CanSetRightAscensionRate		=	false;
	cTelescopeProp.CanSetTracking				=	false;
	cTelescopeProp.CanSlew						=	false;
	cTelescopeProp.CanSlewAltAz					=	false;
	cTelescopeProp.CanSlewAltAzAsync			=	false;
	cTelescopeProp.CanSlewAsync					=	false;
	cTelescopeProp.CanSync						=	false;
	cTelescopeProp.CanSyncAltAz					=	false;
	cTelescopeProp.CanUnpark					=	false;
	cTelescopeProp.DoesRefraction				=	false;
	cTelescopeProp.RightAscension				=	0.0;
	cTelescopeProp.Declination					=	0.0;

	//*	Set these to true if the system supports it
	//*	cTelescopeProp.DoesRefraction is used to enable/disable if it is supported
	cDriverSupports_Refraction		=	false;		//*	can be over-ridden by sub class
	cDriverSupports_LimitSwitches	=	false;		//*	can be over-ridden by sub class
	cDriverSupports_SlewSettleTime	=	false;

	//*	Set default axis rates
	for (iii=0; iii<3; iii++)
	{
		cTelescopeProp.AxisRates[iii].Minimum	=	0.0;
		cTelescopeProp.AxisRates[iii].Maximum	=	4.0 + iii;	//*	the extra iii is for testing
	}

	//*	there are a bunch of static settings that conform needs to be happy
	//*	these are temporary to get CONFORM to work
	//*	set some defaults
	cTelescopeProp.ApertureDiameter		=	16 * 25.4;
	cTelescopeProp.FocalLength			=	cTelescopeProp.ApertureDiameter * 4;	//*	F-ration = 4
	cTelescopeProp.ApertureArea			=	M_PI * ((cTelescopeProp.ApertureDiameter/2) * (cTelescopeProp.ApertureDiameter/2));

	//------------------------------------------------------------------
	//*	check to see if the observatory settings is valid
	CONSOLE_DEBUG_W_BOOL("gObservatorySettingsOK\t=",			gObservatorySettingsOK);
	CONSOLE_DEBUG_W_BOOL("gObseratorySettings.ValidInfo\t=",	gObseratorySettings.ValidInfo);
	if (gObservatorySettingsOK && gObseratorySettings.ValidInfo)
	{
		//*	now set the things we do know
		cTelescopeProp.SiteLatitude		=	gObseratorySettings.Latitude_deg;
		cTelescopeProp.SiteLongitude	=	gObseratorySettings.Longitude_deg;
		cTelescopeProp.SiteElevation	=	gObseratorySettings.Elevation_m;

//		CONSOLE_DEBUG_W_DBL("cTelescopeProp.SiteLatitude\t=",	cTelescopeProp.SiteLatitude);
//		CONSOLE_DEBUG_W_DBL("cTelescopeProp.SiteLongitude\t=",	cTelescopeProp.SiteLongitude);
//		CONSOLE_DEBUG_W_DBL("cTelescopeProp.SiteElevation\t=",	cTelescopeProp.SiteElevation);
//		CONSOLE_ABORT(__FUNCTION__);

		//*	make sure there is valid information
		if ((gObseratorySettings.TS_info[0].aperature_mm > 0.0) && (gObseratorySettings.TS_info[0].focalLen_mm > 0.0))
		{
		double	aperatureRadius;

			cTelescopeProp.ApertureDiameter		=	gObseratorySettings.TS_info[0].aperature_mm;
			cTelescopeProp.FocalLength			=	gObseratorySettings.TS_info[0].focalLen_mm;
			aperatureRadius						=	cTelescopeProp.ApertureDiameter / 2.0;
			cTelescopeProp.ApertureArea			=	M_PI * (aperatureRadius* aperatureRadius);
		}
	}
	else
	{
		CONSOLE_DEBUG("observatory settings are not valid");
//		CONSOLE_ABORT(__FUNCTION__);
	}

#ifdef _ENABLE_IMU_
//	IMU_BNO055_StartBackgroundThread();
	IMU_StartBackgroundThread(NULL);
#endif // _ENABLE_IMU_


#ifdef _ENABLE_GLOBAL_GPS_
int		threadErr;

	CONSOLE_DEBUG("Creating GPS_TelescopeThread");
	cGPStelescopeKeepRunning	=	true;
	threadErr	=	pthread_create(&cGPStelescopeThreadID, NULL, &GPS_TelescopeThread, this);
	if (threadErr != 0)
	{
		CONSOLE_DEBUG_W_NUM("threadErr=", threadErr);
	}
#endif // _ENABLE_GLOBAL_GPS_
}


//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriver::~TelescopeDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_GLOBAL_GPS_
	cGPStelescopeKeepRunning	=	false;
#endif
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;

//	CONSOLE_DEBUG_W_STR("htmlData\t=", reqData->htmlData);
	if (cVerboseDebug)
	{
		if (strcmp(reqData->deviceCommand, "readall") != 0)
		{
			CONSOLE_DEBUG_W_STR("deviceCommand\t=",	reqData->deviceCommand);
		}
	}

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;

	strcpy(alpacaErrMsg, "");

	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer);

	//*	this is not part of the protocol, I am using it for testing
	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Device",
								cCommonProp.Name,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Command",
								reqData->deviceCommand,
								INCLUDE_COMMA);

	//*	look up the command
//	CONSOLE_DEBUG_W_STR("deviceCommand\t=", reqData->deviceCommand);
	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gTelescopeCmdTable, &cmdType);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gTelescopeCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Device specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_Telescope_alignmentmode:	//*	Returns the current mount alignment mode
			alpacaErrCode	=	Get_Alignmentmode(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_altitude:					//*	Returns the mount's Altitude above the horizon.
			alpacaErrCode	=	Get_Altitude(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_aperturearea:				//*	Returns the telescope's aperture.
			alpacaErrCode	=	Get_ApertureArea(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_aperturediameter:			//*	Returns the telescope's effective aperture.
			alpacaErrCode	=	Get_ApertureDiameter(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_athome:						//*	Indicates whether the mount is at the home position.
			alpacaErrCode	=	Get_AtHome(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_atpark:						//*	Indicates whether the telescope is at the park position.
			alpacaErrCode	=	Get_AtPark(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_azimuth:					//*	Returns the telescope's aperture.
			alpacaErrCode	=	Get_Azimuth(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canfindhome:				//*	Indicates whether the mount can find the home position.
			alpacaErrCode	=	Get_CanFindHome(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canpark:					//*	Indicates whether the telescope can be parked.
			alpacaErrCode	=	Get_CanPark(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canpulseguide:				//*	Indicates whether the telescope can be pulse guided.
			alpacaErrCode	=	Get_CanPulseGuide(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_cansetdeclinationrate:		//*	Indicates whether the DeclinationRate property can be changed.
			alpacaErrCode	=	Get_CanSetDeclinationRate(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_cansetguiderates:			//*	Indicates whether the DeclinationRate property can be changed.
			alpacaErrCode	=	Get_CanSetGuideRates(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_cansetpark:					//*	Indicates whether the telescope park position can be set.
			alpacaErrCode	=	Get_CanSetPark(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_cansetpierside:				//*	Indicates whether the telescope SideOfPier can be set.
			alpacaErrCode	=	Get_CanSetPierSide(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_cansetrightascensionrate:	//*	Indicates whether the RightAscensionRate property can be changed.
			alpacaErrCode	=	Get_CanSetRightAscensionRate(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_cansettracking:				//*	Indicates whether the Tracking property can be changed.
			alpacaErrCode	=	Get_CanSetTracking(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canslew:					//*	Indicates whether the telescope can slew synchronously.
			alpacaErrCode	=	Get_CanSlew(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canslewaltaz:				//*	Indicates whether the telescope can slew synchronously to AltAz coordinates.
			alpacaErrCode	=	Get_CanSlewAltAz(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canslewaltazasync:			//*	Indicates whether the telescope can slew asynchronously to AltAz coordinates.
			alpacaErrCode	=	Get_CanSlewAltAzAsync(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canslewasync:				//*	Indicates whether the telescope can slew asynchronously.
			alpacaErrCode	=	Get_CanSlewAsync(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_cansync:					//*	Indicates whether the telescope can sync to equatorial coordinates.
			alpacaErrCode	=	Get_CanSync(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_cansyncaltaz:				//*	Indicates whether the telescope can sync to local horizontal coordinates.
			alpacaErrCode	=	Get_CanSyncAltAz(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canunpark:
			alpacaErrCode	=	Get_CanUnpark(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_declination:				//*	Returns the telescope's declination.
			alpacaErrCode	=	Get_Declination(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_declinationrate:			//*	Returns the telescope's declination tracking rate.
														//*	Sets the telescope's declination tracking rate.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_DeclinationRate(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_DeclinationRate(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_doesrefraction:				//*	Indicates whether atmospheric refraction is applied to coordinates.
														//*	Determines whether atmospheric refraction is applied to coordinates.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_DoesRefraction(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_DoesRefraction(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_equatorialsystem:			//*	Returns the current equatorial coordinate system used by this telescope.
			alpacaErrCode	=	Get_EquatorialSystem(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_focallength:				//*	Returns the telescope's focal length in meters.
			alpacaErrCode	=	Get_FocalLength(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_guideratedeclination:		//*	Returns the current Declination rate offset for telescope guiding
														//*	Sets the current Declination rate offset for telescope guiding.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_GuideRateDeclination(reqData, alpacaErrMsg, gValueString);

			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_GuideRateDeclination(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_guideraterightascension:	//*	Returns the current RightAscension rate offset for telescope guiding
														//*	Sets the current RightAscension rate offset for telescope guiding.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_GuideRateRightAscension(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_GuideRateRightAscension(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_ispulseguiding:				//*	Indicates whether the telescope is currently executing a PulseGuide command
			alpacaErrCode	=	Get_IsPulseGuiding(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_rightascension:				//*	Returns the telescope's right ascension coordinate.
			alpacaErrCode	=	Get_RightAscension(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_rightascensionrate:			//*	Returns the telescope's right ascension tracking rate.
														//*	Sets the telescope's right ascension tracking rate.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_RightAscensionRate(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_RightAscensionRate(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_sideofpier:					//*	Returns the mount's pointing state.
														//*	Sets the mount's pointing state.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_SideOfPier(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SideOfPier(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_siderealtime:				//*	Returns the local apparent sidereal time.
			alpacaErrCode	=	Get_SiderealTime(reqData, alpacaErrMsg, gValueString);
			break;


		case kCmd_Telescope_siteelevation:				//*	Returns the observing site's elevation above mean sea level.
														//*	Sets the observing site's elevation above mean sea level.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_SiteElevation(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SiteElevation(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_sitelatitude:				//*	Returns the observing site's latitude.
														//*	Sets the observing site's latitude.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_SiteLatitude(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SiteLatitude(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_sitelongitude:				//*	Returns the observing site's longitude.
														//*	Sets the observing site's longitude.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_SiteLongitude(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SiteLongitude(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_slewing:					//*	Indicates whether the telescope is currently slewing.
			alpacaErrCode	=	Get_Slewing(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_slewsettletime:				//*	Returns the post-slew settling time.
														//*	Sets the post-slew settling time.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_SlewSettleTime(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewSettleTime(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_targetdeclination:			//*	Returns the current target declination.
														//*	Sets the target declination of a slew or sync.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_TargetDeclination(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_TargetDeclination(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_targetrightascension:		//*	Returns the current target right ascension.
														//*	Sets the target right ascension of a slew or sync.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_TargetRightAscension(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_TargetRightAscension(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_tracking:					//*	Indicates whether the telescope is tracking.
														//*	Enables or disables telescope tracking.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_Tracking(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Tracking(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_trackingrate:				//*	Returns the current tracking rate.
														//*	Sets the mount's tracking rate.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_TrackingRate(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_TrackingRate(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_trackingrates:				//*	Returns a collection of supported DriveRates values.
			alpacaErrCode	=	Get_TrackingRates(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_utcdate:					//*	Returns the UTC date/time of the telescope's internal clock.
														//*	Sets the UTC date/time of the telescope's internal clock.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_UTCdate(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_UTCdate(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_abortslew:					//*	Immediately stops a slew in progress.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_AbortSlew(reqData, alpacaErrMsg);
			}
			else
			{
				CONSOLE_DEBUG("Abort slew is PUT only!!!!!");
			}
			break;

		case kCmd_Telescope_axisrates:					//*	Returns the rates at which the telescope may be moved about the specified axis.
			alpacaErrCode	=	Get_AxisRates(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_canmoveaxis:				//*	Indicates whether the telescope can move the requested axis.
			alpacaErrCode	=	Get_CanMoveAxis(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_destinationsideofpier:		//*	Predicts the pointing state after a German equatorial mount slews to given coordinates.
			alpacaErrCode	=	Get_DestinationSideOfPier(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_findhome:					//*	Moves the mount to the "home" position.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_FindHome(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_moveaxis:					//*	Moves a telescope axis at the given rate.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_MoveAxis(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_park:						//*	Park the mount
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Park(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_pulseguide:					//*	Moves the scope in the given direction for the given time.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_PulseGuide(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_setpark:					//*	Sets the telescope's park position
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SetPark(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_slewtoaltaz:				//*	Synchronously slew to the given local horizontal coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewToAltAz(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_slewtoaltazasync:			//*	Asynchronously slew to the given local horizontal coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewToAltAzAsync(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_slewtocoordinates:			//*	Synchronously slew to the given equatorial coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewToCoordinates(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_slewtocoordinatesasync:		//*	Asynchronously slew to the given equatorial coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewToCoordinatesAsync(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_slewtotarget:				//*	Synchronously slew to the TargetRightAscension and TargetDeclination coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewToTarget(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_slewtotargetasync:			//*	Asynchronously slew to the TargetRightAscension and TargetDeclination coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewToTargetAsync(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_synctoaltaz:				//*	Syncs to the given local horizontal coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SyncToAltAz(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_synctocoordinates:			//*	Syncs to the given equatorial coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SyncToCoordinates(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_synctotarget:				//*	Syncs to the TargetRightAscension and TargetDeclination coordinates.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SyncToTarget(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Telescope_unpark:						//*	Unparks the mount.
			alpacaErrCode	=	Put_UnPark(reqData, alpacaErrMsg);
			break;

		//----------------------------------------------------------------------------------------
		//*	extras, NON ASCOM!!!!
		case kCmd_Telescope_hourangle:
			alpacaErrCode	=	Get_HourAngle(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Telescope_physicalsideofpier:
			alpacaErrCode	=	Get_PhysicalSideOfPier(reqData, alpacaErrMsg, gValueString);
			break;

#ifdef _ENABLE_IMU_
		case kCmd_Telescope_imu:
			alpacaErrCode	=	Get_IMU(reqData, alpacaErrMsg, gValueString);
			break;
#endif // _ENABLE_IMU_

		case kCmd_Telescope_readall:
			alpacaErrCode	=	Get_Readall(reqData, alpacaErrMsg);
			break;

		//----------------------------------------------------------------------------------------
		//*	let anything undefined go to the common command processor
		//----------------------------------------------------------------------------------------
		default:
			if (cmdEnumValue < 999)
			{
				CONSOLE_DEBUG(__FUNCTION__);
				CONSOLE_DEBUG(reqData->cmdBuffer);
				CONSOLE_DEBUG(reqData->contentData);
			}
			alpacaErrCode	=	ProcessCommand_Common(reqData, cmdEnumValue, alpacaErrMsg);
			break;
	}
	if (alpacaErrCode != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
		CONSOLE_DEBUG_W_STR("deviceCommand\t=", reqData->deviceCommand);

	}
//	CONSOLE_DEBUG_W_NUM("Calling RecordCmdStats(), cmdEnumValue=", cmdEnumValue);
	RecordCmdStats(cmdEnumValue, reqData->get_putIndicator, alpacaErrCode);

	if (cSendJSONresponse)	//*	False for setupdialog and camera binary data
	{
		//*	send the response information
		JsonResponse_Add_Uint32(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ClientTransactionID",
									reqData->ClientTransactionID,
									INCLUDE_COMMA);

		JsonResponse_Add_Uint32(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ServerTransactionID",
									gServerTransactionID,
									INCLUDE_COMMA);

		JsonResponse_Add_Int32(		mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ErrorNumber",
									alpacaErrCode,
									INCLUDE_COMMA);

		JsonResponse_Add_String(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ErrorMessage",
									alpacaErrMsg,
									NO_COMMA);

		JsonResponse_Add_Finish(	mySocket,
									reqData->httpRetCode,
									reqData->jsonTextBuffer,
									(cHttpHeaderSent == false));

	}
	//*	this is for the logging function
	strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
bool	TelescopeDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
	bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gTelescopeCmdTable, getPut);
	return(foundIt);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_Alignmentmode(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode;
char					alignmentModeStr[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cTelescopeProp.AlginmentMode,
								INCLUDE_COMMA);

		switch(cTelescopeProp.AlginmentMode)
		{
			case kAlignmentMode_algAltAz:		//*	Altitude-Azimuth alignment.
				strcpy(alignmentModeStr, "Altitude-Azimuth mount");
				break;

			case kAlignmentMode_algPolar:		//*	Polar (equatorial) mount other than German equatorial.
				strcpy(alignmentModeStr, "Polar (equatorial) mount");
				break;

			case kAlignmentMode_algGermanPolar:
				strcpy(alignmentModeStr, "German equatorial mount");
				break;

		}
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"alignmentmode-str",
									alignmentModeStr,
									INCLUDE_COMMA);
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_Altitude(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.Altitude,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_ApertureArea(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.ApertureArea,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_ApertureDiameter(	TYPE_GetPutRequestData *reqData,
															char *alpacaErrMsg,
															const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.ApertureDiameter,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_AtHome(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.AtHome,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_AtPark(TYPE_GetPutRequestData *reqData,
												char					*alpacaErrMsg,
												const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.AtPark,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_Azimuth(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.Azimuth,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanFindHome(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanFindHome,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanPark(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanPark,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanPulseGuide(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanPulseGuide,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSetDeclinationRate(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSetDeclinationRate,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSetGuideRates(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSetGuideRates,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSetPark(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSetPark,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSetPierSide(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSetPierSide,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSetRightAscensionRate(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSetRightAscensionRate,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSetTracking(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSetTracking,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSlew(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSlew,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSlewAltAz(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSlewAltAz,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSlewAltAzAsync(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSlewAltAzAsync,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSlewAsync(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSlewAsync,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSync(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSync,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanSyncAltAz(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanSyncAltAz,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanUnpark(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.CanUnpark,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_Declination(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					declinationStr[48];

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.Declination,
							INCLUDE_COMMA);

	//*	extra... add the string value
	FormatHHMMSSdd(cTelescopeProp.Declination, declinationStr, true);
	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Declination-str",
								declinationStr,
								INCLUDE_COMMA);

	FormatHHMMSS(cTelescopeProp.Declination, declinationStr, true);
	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Declination-str2",
								declinationStr,
								INCLUDE_COMMA);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_DeclinationRate(	TYPE_GetPutRequestData *reqData,
															char *alpacaErrMsg,
															const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	if (cTelescopeProp.CanSetDeclinationRate == false)
	{
		cTelescopeProp.DeclinationRate	=	0.0;
	}

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.DeclinationRate,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_DeclinationRate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					decRateFound;
char					decRateString[64];
double					newDecRate;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cTelescopeProp.CanSetDeclinationRate)
	{
		decRateFound		=	GetKeyWordArgument(	reqData->contentData,
													"DeclinationRate",
													decRateString,
													sizeof(decRateString),
													kRequireCase,
													kArgumentIsNumeric);
		if (decRateFound)
		{
			if (IsValidNumericString(decRateString))
			{
				newDecRate			=	AsciiToDouble(decRateString);
				CONSOLE_DEBUG_W_DBL("newDecRate\t=", newDecRate);
				cTelescopeProp.DeclinationRate	=	newDecRate;
				alpacaErrCode					=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "DeclinationRate value invalid");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "DeclinationRate argument is missing");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetDeclinationRate is false");
	}
	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_DoesRefraction(TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	if (cDriverSupports_Refraction)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cTelescopeProp.DoesRefraction,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Driver does not support refraction");
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_DoesRefraction(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					doseRefractionFound;
char					doseRefractionString[64];

	CONSOLE_DEBUG(__FUNCTION__);

	doseRefractionFound		=	GetKeyWordArgument(	reqData->contentData,
													"DoesRefraction",
													doseRefractionString,
													sizeof(doseRefractionString));
	if (doseRefractionFound)
	{
		if (IsValidTrueFalseString(doseRefractionString) == false)
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "DoesRefraction value is not true/false");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "DoesRefraction is missing");
	}
	if (cDriverSupports_Refraction)
	{
		if (doseRefractionFound)
		{
			cTelescopeProp.DoesRefraction	=	IsTrueFalse(doseRefractionString);
			alpacaErrCode					=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "DoesRefraction is missing");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Driver does not support refraction");
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_EquatorialSystem(	TYPE_GetPutRequestData	*reqData,
															char					*alpacaErrMsg,
															const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					extraString[128];

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.EquatorialSystem,
							INCLUDE_COMMA);

		switch(cTelescopeProp.EquatorialSystem)
		{
			case kECT_equOther:		//*	Altitude-Azimuth alignment.
				strcpy(extraString, "Custom or unknown equinox and/or reference frame");
				break;

			case kECT_equTopocentric:		//*	Polar (equatorial) mount other than German equatorial.
				strcpy(extraString, "Topocentric coordinates");
				break;

			case kECT_equJ2000:
				strcpy(extraString, "J2000 equator/equinox");
				break;

			case kECT_equJ2050:
				strcpy(extraString, "J2050 equator/equinox");
				break;

			case kECT_equB1950:
				strcpy(extraString, "B1950 equinox, FK4");
				break;

		}
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"EquatorialSystem-str",
									extraString,
									INCLUDE_COMMA);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_FocalLength(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.FocalLength,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_GuideRateDeclination(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.GuideRateDeclination,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_GuideRateDeclination(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				guideRateDeclinationFound;
char				guideRateDeclinationStr[64];
double				newGuideRateDeclination;
bool				newGuidRateValid;

	CONSOLE_DEBUG(__FUNCTION__);

	guideRateDeclinationFound	=	GetKeyWordArgument(	reqData->contentData,
														"GuideRateDeclination",
														guideRateDeclinationStr,
														sizeof(guideRateDeclinationStr),
														kRequireCase,
														kArgumentIsNumeric);
	newGuidRateValid	=	false;
	if (guideRateDeclinationFound)
	{
		if (IsValidNumericString(guideRateDeclinationStr))
		{
			newGuideRateDeclination	=	AsciiToDouble(guideRateDeclinationStr);
			newGuidRateValid		=	true;
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "GuideRateDeclination value is invalid");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "GuideRateDeclination is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}


	if (cTelescopeProp.CanSetGuideRates)
	{
		if (newGuidRateValid)
		{
			cTelescopeProp.GuideRateDeclination	=	newGuideRateDeclination;
			alpacaErrCode						=	kASCOM_Err_Success;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetGuideRates is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_GuideRateRightAscension(	TYPE_GetPutRequestData *reqData,
																	char *alpacaErrMsg,
																	const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.GuideRateRightAscension,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_GuideRateRightAscension(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				guideRateRightAscensionFound;
char				guideRateRightAscensionStr[64];
double				newGuideRateRightAscension;
bool				guideRateIsValid;

	CONSOLE_DEBUG(__FUNCTION__);
	guideRateRightAscensionFound	=	GetKeyWordArgument(	reqData->contentData,
															"GuideRateRightAscension",
															guideRateRightAscensionStr,
															sizeof(guideRateRightAscensionStr),
															kRequireCase,
															kArgumentIsNumeric);
	guideRateIsValid	=	false;
	if (guideRateRightAscensionFound)
	{
		if (IsValidNumericString(guideRateRightAscensionStr))
		{
			newGuideRateRightAscension	=	AsciiToDouble(guideRateRightAscensionStr);
			guideRateIsValid			=	true;
//			CONSOLE_DEBUG_W_STR("guideRateRightAscensionStr\t=", guideRateRightAscensionStr);
//			CONSOLE_DEBUG_W_DBL("newGuideRateRightAscension\t=", newGuideRateRightAscension);
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "GuideRateRightAscension value is invalid");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "GuideRateRightAscension argument is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	if (cTelescopeProp.CanSetGuideRates)
	{
		if (guideRateIsValid)
		{
			cTelescopeProp.GuideRateRightAscension	=	newGuideRateRightAscension;
			alpacaErrCode							=	kASCOM_Err_Success;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetGuideRates is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_IsPulseGuiding(TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;

//	JsonResponse_Add_Bool(	reqData->socket,
//							reqData->jsonTextBuffer,
//							kMaxJsonBuffLen,
//							responseString,
//							cTelescopeProp.IsPulseGuiding,
//							INCLUDE_COMMA);
//	if (cTelescopeProp.CanPulseGuide)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cTelescopeProp.IsPulseGuiding,
								INCLUDE_COMMA);
		alpacaErrCode	=	kASCOM_Err_Success;
	}
//	else
//	{
//		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
//		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanPulseGuide is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
//	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_RightAscension(TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					rightAscString[48];

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.RightAscension,
							INCLUDE_COMMA);

	//*	extra... add the string value
	FormatHHMMSSdd(cTelescopeProp.RightAscension, rightAscString, true);
	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"RightAscension-str",
								rightAscString,
								INCLUDE_COMMA);

	FormatHHMMSS(cTelescopeProp.RightAscension, rightAscString, true);
	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"RightAscension-str2",
								rightAscString,
								INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_RightAscensionRate(	TYPE_GetPutRequestData *reqData,
																char		*alpacaErrMsg,
																const char	*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.RightAscensionRate,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_RightAscensionRate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				rightAscenRateFound;
char				rightAscenRateString[64];
double				newrightAscenRate;
bool				rightAscRateValid;

	CONSOLE_DEBUG(__FUNCTION__);
	rightAscenRateFound		=	GetKeyWordArgument(	reqData->contentData,
													"RightAscensionRate",
													rightAscenRateString,
													sizeof(rightAscenRateString),
													kRequireCase,
													kArgumentIsNumeric);
	rightAscRateValid		=	false;
	if (rightAscenRateFound)
	{
		if (IsValidNumericString(rightAscenRateString))
		{
			newrightAscenRate	=	AsciiToDouble(rightAscenRateString);
			rightAscRateValid	=	true;
	//		CONSOLE_DEBUG_W_STR("rightAscenRateString\t=", rightAscenRateString);
	//		CONSOLE_DEBUG_W_DBL("newrightAscenRate\t=", newrightAscenRate);
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "RightAscensionRate value is invalid");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "RightAscensionRate is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	if (cTelescopeProp.CanSetRightAscensionRate)
	{
		if (rightAscRateValid)
		{
			cTelescopeProp.RightAscensionRate	=	newrightAscenRate;
			alpacaErrCode						=	kASCOM_Err_Success;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetRightAscensionRate is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
void	GetSideOfPierString(TYPE_PierSide sideOfPier, char *sideOfPierString)
{
	switch(sideOfPier)
	{
		case kPierSide_pierUnknown:		//*	Polar (equatorial) mount other than German equatorial.
			strcpy(sideOfPierString, "Unknown or indeterminate.");
			break;

		case kPierSide_pierEast:		//*	Altitude-Azimuth alignment.
			strcpy(sideOfPierString, "Normal pointing state - Mount on the East side of pier (looking West)");
			break;

		case kPierSide_pierWest:
			strcpy(sideOfPierString, "Through the pole pointing state - Mount on the West side of pier (looking East)");
			break;

		case kPierSide_NotAvailable:
		default:
			strcpy(sideOfPierString, "Not available");
			break;
	}
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SideOfPier(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					extraString[128];

	cTelescopeProp.SideOfPier	=	Telescope_CalculateSideOfPier();

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.SideOfPier,
							INCLUDE_COMMA);
	GetSideOfPierString(cTelescopeProp.SideOfPier, extraString);
	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"SideOfPier-str",
								extraString,
								INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SideOfPier(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
bool				sideOfPierFound;
char				sideOfPierString[64];
//int					newSideOfPier;

	CONSOLE_DEBUG(__FUNCTION__);
	sideOfPierFound		=	GetKeyWordArgument(	reqData->contentData,
												"SideOfPier",
												sideOfPierString,
												sizeof(sideOfPierString),
												kRequireCase,
												kArgumentIsNumeric);
	if (sideOfPierFound)
	{
		if (IsValidNumericString(sideOfPierString))
		{
//			newSideOfPier	=	atoi(sideOfPierString);
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SideOfPier is value is invalid");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SideOfPier is missing");
		CONSOLE_DEBUG(alpacaErrMsg);

	}

	alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Put_SideOfPier not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SiderealTime(	TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	cTelescopeProp.SiderealTime	=	CalcSiderealTime_dbl(NULL, gObseratorySettings.Longitude_deg);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.SiderealTime,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SiteElevation(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.SiteElevation,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SiteElevation(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				siteElevFound;
char				siteElevString[64];
double				newElevation;

	CONSOLE_DEBUG(__FUNCTION__);

	siteElevFound		=	GetKeyWordArgument(	reqData->contentData,
												"SiteElevation",
												siteElevString,
												sizeof(siteElevString),
												kRequireCase,
												kArgumentIsNumeric);
	if (siteElevFound)
	{
		if (IsValidNumericString(siteElevString))
		{
			newElevation	=	AsciiToDouble(siteElevString);
			CONSOLE_DEBUG_W_STR("siteElevString\t=", siteElevString);
			CONSOLE_DEBUG_W_DBL("newElevation  \t=", newElevation);

			if ((newElevation >= -300.0) && (newElevation <= 10000.0))
			{
				cTelescopeProp.SiteElevation	=	newElevation;
				alpacaErrCode					=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteElevation is out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteElevation is not numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteElevation is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SiteLatitude(	TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.SiteLatitude,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SiteLatitude(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				siteLatFound;
char				siteLatString[64];
double				newLatitude;

	CONSOLE_DEBUG(__FUNCTION__);
	//curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/telescope/0/sitelatitude"
	//	-H "accept: application/json"
	//	-H "Content-Type: application/x-www-form-urlencoded"
	//	-d "SiteLatitude=51.3&ClientID=1&ClientTransactionID=3"

	siteLatFound		=	GetKeyWordArgument(	reqData->contentData,
												"SiteLatitude",
												siteLatString,
												sizeof(siteLatString),
												kRequireCase,
												kArgumentIsNumeric);
	if (siteLatFound)
	{
		if (IsValidNumericString(siteLatString))
		{
			newLatitude	=	AsciiToDouble(siteLatString);

			if ((newLatitude >= -90.0) && (newLatitude <= 90.0))
			{
				cTelescopeProp.SiteLatitude	=	newLatitude;
				alpacaErrCode				=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLatitude is out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLatitude is not numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLatitude is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SiteLongitude(	TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.SiteLongitude,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SiteLongitude(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				siteLonFound;
char				siteLonString[64];
double				newLongitude;


	CONSOLE_DEBUG(__FUNCTION__);
	siteLonFound		=	GetKeyWordArgument(	reqData->contentData,
												"SiteLongitude",
												siteLonString,
												sizeof(siteLonString),
												kRequireCase,
												kArgumentIsNumeric);
	if (siteLonFound)
	{
		if (IsValidNumericString(siteLonString))
		{
			newLongitude	=	AsciiToDouble(siteLonString);
	//		CONSOLE_DEBUG_W_STR("siteLonString\t=", siteLonString);
	//		CONSOLE_DEBUG_W_DBL("newLongitude\t=", newLongitude);

			if ((newLongitude >= -180.0) && (newLongitude <= 180.0))
			{
				cTelescopeProp.SiteLongitude	=	newLongitude;
				alpacaErrCode					=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLongitude is out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLongitude is non-numberic");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLongitude is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_Slewing(	TYPE_GetPutRequestData	*reqData,
													char					*alpacaErrMsg,
													const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.Slewing,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
//*	slew settle time is for synchronous slewing commands and really does not apply to Alpaca
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SlewSettleTime(TYPE_GetPutRequestData *reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	if (cDriverSupports_SlewSettleTime)
	{
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cTelescopeProp.SlewSettleTime,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewSettleTime not supported");
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	slew settle time is for synchronous slewing commands and really does not apply to Alpaca
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewSettleTime(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				slewSettleTimeFound;
char				slewSettleTimeString[64];
int					newSlewSettleTime;
bool				slewSettleTimeValid;

	CONSOLE_DEBUG(__FUNCTION__);
	slewSettleTimeFound		=	GetKeyWordArgument(	reqData->contentData,
													"SlewSettleTime",
													slewSettleTimeString,
													sizeof(slewSettleTimeString));
	slewSettleTimeValid	=	false;
	if (slewSettleTimeFound)
	{
		if (IsValidNumericString(slewSettleTimeString))
		{
			newSlewSettleTime	=	atoi(slewSettleTimeString);
			slewSettleTimeValid	=	true;
//			CONSOLE_DEBUG_W_STR("slewSettleTimeString\t=",	slewSettleTimeString);
//			CONSOLE_DEBUG_W_NUM("newSlewSettleTime   \t=",	newSlewSettleTime);
			if (newSlewSettleTime >= 0)
			{
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewSettleTime is out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewSettleTime is non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewSettleTime is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	if (cDriverSupports_SlewSettleTime)
	{
		if (slewSettleTimeValid)
		{
			cTelescopeProp.SlewSettleTime	=	newSlewSettleTime;
			alpacaErrCode					=	kASCOM_Err_Success;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewSettleTime not supported");
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_TargetDeclination(	TYPE_GetPutRequestData *reqData,
															char *alpacaErrMsg,
															const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.TargetDeclination,
							INCLUDE_COMMA);
	if (cTelescopeProp.TargetDec_HasBeenSet)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidOperation;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetDeclination has not been set");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	ClientTransactionID=144&ClientID=27725&TargetDeclination=0
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_TargetDeclination(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				targetDeclinationFound;
char				targetDeclinationString[64];
double				newTargetDeclination;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	targetDeclinationFound	=	GetKeyWordArgument(	reqData->contentData,
													"TargetDeclination",
													targetDeclinationString,
													sizeof(targetDeclinationString),
													kRequireCase,
													kArgumentIsNumeric);
	if (targetDeclinationFound)
	{
		if (IsValidNumericString(targetDeclinationString))
		{
			newTargetDeclination		=	AsciiToDouble(targetDeclinationString);
			CONSOLE_DEBUG_W_DBL("newTargetDeclination\t=", newTargetDeclination);

			if ((newTargetDeclination >= -90.0) && (newTargetDeclination <= 90.0))
			{
				cTelescopeProp.Declination			=	newTargetDeclination;
				cTelescopeProp.TargetDeclination	=	newTargetDeclination;
				cTelescopeProp.TargetDec_HasBeenSet	=	true;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetDeclination out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetDeclination is non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetDeclination missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_TargetRightAscension(	TYPE_GetPutRequestData *reqData,
																char *alpacaErrMsg,
																const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.TargetRightAscension,
							INCLUDE_COMMA);

	if (cTelescopeProp.TargetRA_HasBeenSet)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidOperation;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetRightAscension has not been set");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	ClientTransactionID=143&ClientID=27725&TargetRightAscension=0
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_TargetRightAscension(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				targetRightAscensionFound;
char				targetRightAscensionString[64];
double				newTargetRightAscension;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	targetRightAscensionFound	=	GetKeyWordArgument(	reqData->contentData,
														"TargetRightAscension",
														targetRightAscensionString,
														sizeof(targetRightAscensionString),
														kRequireCase,
														kArgumentIsNumeric);
	if (targetRightAscensionFound)
	{
		if (IsValidNumericString(targetRightAscensionString))
		{
			newTargetRightAscension		=	AsciiToDouble(targetRightAscensionString);
			CONSOLE_DEBUG_W_DBL("newTargetRightAscension\t=", newTargetRightAscension);

			if ((newTargetRightAscension >= 0.0) && (newTargetRightAscension <= 24.0))
			{
				cTelescopeProp.RightAscension		=	newTargetRightAscension;
				cTelescopeProp.TargetRightAscension	=	newTargetRightAscension;
				cTelescopeProp.TargetRA_HasBeenSet	=	true;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetRightAscension out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetRightAscension non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetRightAscension missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_Tracking(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.Tracking,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_Tracking(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool					trackingFound;
char					trackingString[64];

	CONSOLE_DEBUG(__FUNCTION__);

	if (cTelescopeProp.CanSetTracking)
	{
		trackingFound		=	GetKeyWordArgument(	reqData->contentData,
													"Tracking",
													trackingString,
													sizeof(trackingString));
		if (trackingFound)
		{
			if (IsValidTrueFalseString(trackingString))
			{
				cTelescopeProp.Tracking		=	IsTrueFalse(trackingString);
				if (cTelescopeProp.Tracking)
				{
					cTelescopeProp.AtPark	=	false;
				}
				alpacaErrCode	=	Telescope_TrackingOnOff(cTelescopeProp.Tracking, alpacaErrMsg);
				CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
				CONSOLE_DEBUG(cTelescopeProp.Tracking ? "Tracking is ENABLED" : "Tracking is DISABLED");
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Tracking is not true/false");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Tracking is missing");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetTracking is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_TrackingRate(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					extraString[128];

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.TrackingRate,
							INCLUDE_COMMA);

	switch(cTelescopeProp.TrackingRate)
	{
		case kDriveRate_driveSidereal:		//*	Sidereal tracking rate (15.041 arcseconds per second).
			strcpy(extraString, "Sidereal tracking rate (15.041 arcseconds per second).");
			break;

		case kDriveRate_driveLunar:			//*	Lunar tracking rate (14.685 arcseconds per second).
			strcpy(extraString, "Lunar tracking rate (14.685 arcseconds per second).");
			break;

		case kDriveRate_driveSolar:			//*	Solar tracking rate (15.0 arcseconds per second)
			strcpy(extraString, "Solar tracking rate (15.0 arcseconds per second))");
			break;

		case kDriveRate_driveKing:			//*	King tracking rate (15.0369 arcseconds per second).
			strcpy(extraString, "King tracking rate (15.0369 arcseconds per second).");
			break;

		case kDriveRate_Count:
			//*	keep the compiler happy
			break;

	}
	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"TrackingRate-str",
								extraString,
								INCLUDE_COMMA);


	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_TrackingRate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					trackingRateFound;
char					trackingRateString[64];
TYPE_DriveRates			newtrackingRate;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	trackingRateFound		=	GetKeyWordArgument(	reqData->contentData,
													"TrackingRate",
													trackingRateString,
													sizeof(trackingRateString));
	if (trackingRateFound)
	{
		if (IsValidNumericString(trackingRateString))
		{
			newtrackingRate		=	(TYPE_DriveRates)atoi(trackingRateString);
			CONSOLE_DEBUG_W_NUM("newtrackingRate\t=", newtrackingRate);
			if ((newtrackingRate >= kDriveRate_driveSidereal) && (newtrackingRate < kDriveRate_Count))
			{
				cTelescopeProp.TrackingRate	=	newtrackingRate;
				alpacaErrCode				=	kASCOM_Err_Success;
				//*	we do NOT want to start tracking if not currently tracking
				if (cTelescopeProp.Tracking)
				{
					CONSOLE_DEBUG("Telescope is tracking, update the new value");
					alpacaErrCode			=	Telescope_TrackingRate(cTelescopeProp.TrackingRate, alpacaErrMsg);
				}
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TrackingRate is out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TrackingRate is non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TrackingRate is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_TrackingRates(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

//	return(kASCOM_Err_ActionNotImplemented);


	JsonResponse_Add_ArrayStart(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString);


	//	kDriveRate_driveSidereal	=	0,	//*	Sidereal tracking rate (15.041 arcseconds per second).
	//	kDriveRate_driveLunar		=	1,	//*	Lunar tracking rate (14.685 arcseconds per second).
	//	kDriveRate_driveSolar		=	2,	//*	Solar tracking rate (15.0 arcseconds per second).
	//	kDriveRate_driveKing		=	3,	//*	King tracking rate (15.0369 arcseconds per second).

	JsonResponse_Add_RawText(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"0,1,2,3");
//								"0");

	JsonResponse_Add_ArrayEnd(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								INCLUDE_COMMA);


	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_UTCdate(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					timeStampString[128];
struct timeval			currentTime;

//	"2022-05-30T13:49:10.4766414Z"		correct
//	"2022-05-30T13:48:55.094"

	gettimeofday(&currentTime, NULL);
	FormatTimeStringISO8601(&currentTime, timeStampString);
	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							timeStampString,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_UTCdate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
bool					utcDateFound;
char					utcDateString[64];

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);


	utcDateFound	=	GetKeyWordArgument(	reqData->contentData,
											"UTCDate",
											utcDateString,
											sizeof(utcDateString));
	if (utcDateFound)
	{
		//*	we have to parse the ISO8601 time string
		if (IsValidNumericString(utcDateString))
		{

		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "UTCDate is ill-formated");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "UTCDate not implemented");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "UTCDate is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*****************************************************************************
//*	ASCOM docs call these METHODS
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_AbortSlew(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrMsg[0]	=	0;
	if (cTelescopeProp.AtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
//		CONSOLE_DEBUG(alpacaErrMsg);
		//*	send the abort anyway
		Telescope_AbortSlew(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	Telescope_AbortSlew(alpacaErrMsg);
	}

	if (alpacaErrCode != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
		CONSOLE_DEBUG_W_STR("alpacaErrMsg\t=", alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_AxisRates(	TYPE_GetPutRequestData	*reqData,
													char					*alpacaErrMsg,
													const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					axisFound;
char					axisString[64];
int						axisNumber;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	axisFound		=	GetKeyWordArgument(	reqData->contentData,
											"Axis",
											axisString,
											sizeof(axisString),
											kIgnoreCase);
	if (axisFound)
	{
		if (IsValidNumericString(axisString))
		{
			axisNumber		=	atoi(axisString);
		}
		else
		{
			axisNumber		=	-1;
		}

//		CONSOLE_DEBUG_W_NUM("axisNumber\t=", axisNumber);
		if ((axisNumber >= 0) && (axisNumber <= 2))
		{
			JsonResponse_Add_ArrayStart(reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										responseString);

			JsonResponse_Add_RawText(	reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"\r\n\t\t{\r\n");


			JsonResponse_Add_Double(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Minimum",
									cTelescopeProp.AxisRates[axisNumber].Minimum,
									INCLUDE_COMMA);

			JsonResponse_Add_Double(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Maximum",
									cTelescopeProp.AxisRates[axisNumber].Maximum,
									NO_COMMA);

			JsonResponse_Add_RawText(	reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"\t\t}\r\n");


			JsonResponse_Add_ArrayEnd(	reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										INCLUDE_COMMA);
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is out of bounds");
//			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is missing");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_CanMoveAxis(	TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					axisFound;
char					axisString[64];
int						axisNumber;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	axisFound		=	GetKeyWordArgument(	reqData->contentData,
											"Axis",
											axisString,
											sizeof(axisString),
											kIgnoreCase);
	if (axisFound)
	{
		if (IsValidNumericString(axisString))
		{
			axisNumber		=	atoi(axisString);
		}
		else
		{
			axisNumber		=	-1;
		}
//		CONSOLE_DEBUG_W_NUM("axisNumber\t=", axisNumber);
		if ((axisNumber >= 0) && (axisNumber <= 2))
		{

			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									cTelescopeProp.CanMoveAxis[axisNumber],
									INCLUDE_COMMA);

		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::ExtractRaDecArguments(	TYPE_GetPutRequestData	*reqData,
															double					*rightAscension,
															double					*declination,
															char					*alpacaErrMsg,
															const bool				ingoreCase)
{
bool				rightAscensionFound;
char				rightAscensionStr[64];
bool				declinationFound;
char				declinationStr[64];
double				newRightAscension;
double				newDeclination;
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InvalidValue;

	rightAscensionFound		=	GetKeyWordArgument(	reqData->contentData,
													"RightAscension",
													rightAscensionStr,
													sizeof(rightAscensionStr),
													ingoreCase,
													kArgumentIsNumeric);

	declinationFound		=	GetKeyWordArgument(	reqData->contentData,
													"Declination",
													declinationStr,
													sizeof(rightAscensionStr),
													ingoreCase,
													kArgumentIsNumeric);
	if (rightAscensionFound && declinationFound)
	{
		if (IsValidNumericString(rightAscensionStr) && IsValidNumericString(declinationStr))
		{
			newRightAscension	=	AsciiToDouble(rightAscensionStr);
			newDeclination		=	AsciiToDouble(declinationStr);

//			CONSOLE_DEBUG_W_DBL("newRightAscension\t=",	newRightAscension);
//			CONSOLE_DEBUG_W_DBL("newDeclination\t=",	newDeclination);
			if ((newRightAscension >= 0.0) && (newRightAscension <= 24.0) &&
				(newDeclination >= -90.0) && (newDeclination <= 90.0))
			{
				alpacaErrCode	=	kASCOM_Err_Success;
				*rightAscension	=	newRightAscension;
				*declination	=	newDeclination;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "RA and/or DEC out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "RA and/or DEC non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "RA and/or DEC not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::ExtractAltAzArguments(	TYPE_GetPutRequestData	*reqData,
															double					*altitude,
															double					*azimuth,
															char					*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InvalidValue;
bool				azimuthFound;
bool				altitudeFound;
char				azimuthString[64];
char				altitudeString[64];
double				newAlt_degrees;
double				newAz_degrees;

	CONSOLE_DEBUG(__FUNCTION__);
	altitudeFound	=	GetKeyWordArgument(	reqData->contentData,
											"Altitude",
											altitudeString,
											sizeof(altitudeString),
											kRequireCase,
											kArgumentIsNumeric);

	azimuthFound	=	GetKeyWordArgument(	reqData->contentData,
											"Azimuth",
											azimuthString,
											sizeof(azimuthString),
											kRequireCase,
											kArgumentIsNumeric);

	if (altitudeFound && azimuthFound)
	{
		if (IsValidNumericString(altitudeString) && IsValidNumericString(azimuthString))
		{
			newAz_degrees	=	AsciiToDouble(azimuthString);
			newAlt_degrees	=	AsciiToDouble(altitudeString);
			if ((newAz_degrees >= 0.0) && (newAz_degrees < 360.0))
			{
				*azimuth		=	newAz_degrees;
				*altitude		=	newAlt_degrees;
				alpacaErrCode	=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Azimuth or Altitude out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Azimuth or Altitude not numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Azimuth or Altitude not found");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	.../destinationsideofpier?RightAscension=3&Declination=0&ClientID=1&ClientTransactionID=1234"
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_DestinationSideOfPier(	TYPE_GetPutRequestData	*reqData,
																char					*alpacaErrMsg,
																const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
double				newRightAscension;
double				newDeclination;
char				extraString[64];

	alpacaErrCode	=	ExtractRaDecArguments(reqData, &newRightAscension, &newDeclination, alpacaErrMsg, kIgnoreCase);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		cTelescopeProp.DestinationSideOfPier	=	Telescope_CalculateDestinationSideOfPier(newRightAscension, newDeclination);
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cTelescopeProp.DestinationSideOfPier,
								INCLUDE_COMMA);
		GetSideOfPierString(cTelescopeProp.DestinationSideOfPier, extraString);

		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"DestSideOfPier-str",
									extraString,
									INCLUDE_COMMA);
		alpacaErrCode	=	kASCOM_Err_Success;
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_FindHome(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cTelescopeProp.AtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
	}
	else if (cTelescopeProp.CanFindHome)
	{
		alpacaErrCode	=	Telescope_FindHome(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanFindHome is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_MoveAxis(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					axisFound;
bool					rateFound;
char					axisString[64];
char					rateString[64];
int						axisNumber;
double					newRate;
bool					previousTrackingState;

//	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrMsg[0]	=	0;

	if (cTelescopeProp.AtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
	}
	else if (cTelescopeProp.CanMoveAxis)
	{
		axisFound		=	GetKeyWordArgument(	reqData->contentData,
												"Axis",
												axisString,
												sizeof(axisString),
												kRequireCase);

		rateFound		=	GetKeyWordArgument(	reqData->contentData,
												"Rate",
												rateString,
												sizeof(rateString),
												kRequireCase,
												kArgumentIsNumeric);

		if (axisFound && rateFound)
		{
			if (IsValidNumericString(axisString))
			{
				axisNumber		=	atoi(axisString);
			}
			else
			{
				axisNumber		=	-1;
			}
			CONSOLE_DEBUG_W_NUM("axisNumber\t=", axisNumber);
			if ((axisNumber >= 0) && (axisNumber <= 2))
			{
				if (IsValidNumericString(rateString))
				{
					newRate		=	AsciiToDouble(rateString);
					//*	May 15 2124	<TODO> Add support for multiple ranges
					if ((fabs(newRate) >= cTelescopeProp.AxisRates[axisNumber].Minimum) &&
						(fabs(newRate) <= cTelescopeProp.AxisRates[axisNumber].Maximum))
					{
						CONSOLE_DEBUG_W_DBL("newRate\t=", newRate);
						JsonResponse_Add_Double(reqData->socket,
												reqData->jsonTextBuffer,
												kMaxJsonBuffLen,
												"Rate",
												newRate,
												INCLUDE_COMMA);

						previousTrackingState	=	cTelescopeProp.Tracking;	//*	save current tracking state
						alpacaErrCode			=	Telescope_MoveAxis(axisNumber, newRate, alpacaErrMsg);
						if (alpacaErrCode == kASCOM_Err_Success)
						{
							//*	restore the tracking rate
	//						Telescope_TrackingOnOff(previousTrackingState, alpacaErrMsg);
						}
						else
						{
							CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
							CONSOLE_DEBUG(alpacaErrMsg);
						}
					}
					else
					{
						alpacaErrCode			=	kASCOM_Err_InvalidValue;
						reqData->httpRetCode	=	400;
						GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Rate out of allowed range");
		//				CONSOLE_DEBUG(alpacaErrMsg);
					}
				}
				else
				{
					alpacaErrCode			=	kASCOM_Err_InvalidValue;
					reqData->httpRetCode	=	400;
					GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Rate out non-numeric");
	//				CONSOLE_DEBUG(alpacaErrMsg);
				}
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is out of bounds");
//				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is missing");
//			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "CanMoveAxis is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	api/v1/telescope/0/pulseguide" -H  "accept: application/json"
//	-H  "Content-Type: application/x-www-form-urlencoded"
//	-d "ClientID=124&ClientTransactionID=56&Direction=0&Duration=5"
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_PulseGuide(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

bool					directionFound;
char					directionString[64];
TYPE_GuideDirections	directionValue;
bool					durationFound;
char					durationString[64];
int						durationValue;
bool					dataIsValid;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_BOOL("cTelescopeProp.CanPulseGuide\t=", cTelescopeProp.CanPulseGuide);
//	CONSOLE_DEBUG(reqData->contentData);

	directionFound	=	GetKeyWordArgument(	reqData->contentData,
													"Direction",
													directionString,
													sizeof(directionString));

	durationFound	=	GetKeyWordArgument(	reqData->contentData,
													"Duration",
													durationString,
													sizeof(durationString));
	dataIsValid	=	false;
	if (directionFound && durationFound)
	{
		if (IsValidNumericString(directionString) && IsValidNumericString(durationString))
		{
			directionValue	=	(TYPE_GuideDirections)atoi(directionString);
			durationValue	=	atoi(durationString);

			if ((directionValue >= kGuide_North) && (directionValue < kGuide_last))
			{
				dataIsValid	=	true;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				CONSOLE_DEBUG("Direction or Duration out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			CONSOLE_DEBUG("Direction or Duration non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Direction or Duration missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	if (cTelescopeProp.AtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cTelescopeProp.CanPulseGuide)
	{
		if (dataIsValid)
		{
			if (durationValue > 100)
			{
				CONSOLE_DEBUG_W_NUM("durationValue\t", durationValue);
			}
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanPulseGuide is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SetPark(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cTelescopeProp.CanSetPark)
	{
		alpacaErrCode	=	Telescope_SetPark(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetPark is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToAltAz(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
double				newAz_degrees;
double				newAlt_degrees;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	ExtractAltAzArguments(reqData, &newAz_degrees, &newAlt_degrees, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		if (cTelescopeProp.CanSlewAltAz)
		{
			alpacaErrCode	=	Telescope_SlewToAltAz(newAlt_degrees, newAz_degrees, alpacaErrMsg);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlewAltAz is false");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToAltAzAsync(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
double				newAz_degrees;
double				newAlt_degrees;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	ExtractAltAzArguments(reqData, &newAz_degrees, &newAlt_degrees, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		if (cTelescopeProp.CanSlewAltAzAsync)
		{
			alpacaErrCode	=	Telescope_SlewToAltAz(newAlt_degrees, newAz_degrees, alpacaErrMsg);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlewAltAzAsync is false");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	ClientTransactionID=105&ClientID=46225&RightAscension=18.14&Declination=1
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToCoordinates(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				newRightAscension;
double				newDeclination;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	ExtractRaDecArguments(reqData, &newRightAscension, &newDeclination, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		if (cTelescopeProp.AtPark)
		{
			alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Use async methods instead");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToCoordinatesAsync(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				newRightAscension;
double				newDeclination;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	ExtractRaDecArguments(reqData, &newRightAscension, &newDeclination, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{

		if (cTelescopeProp.AtPark)
		{
			alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
		else if (cTelescopeProp.CanSlewAsync)
		{
			//*	these need to be set to keep CONFORM happy	<MLS> 5/14/2024
			cTelescopeProp.TargetRightAscension	=	newRightAscension;
			cTelescopeProp.TargetDeclination	=	newDeclination;

			cTelescopeProp.TargetDec_HasBeenSet	=	true;
			cTelescopeProp.TargetRA_HasBeenSet	=	true;

			alpacaErrCode			=	Telescope_SlewToRA_DEC(	newRightAscension,
																newDeclination,
																alpacaErrMsg);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlewAsync is false");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToTarget(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cTelescopeProp.AtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cTelescopeProp.CanSlew)
	{
		alpacaErrCode			=	kASCOM_Err_Success;
		cTelescopeProp.AtPark	=	false;
		CONSOLE_DEBUG_W_DBL("cTargetRightAscension\t=",	cTelescopeProp.TargetRightAscension);
		CONSOLE_DEBUG_W_DBL("cTargetDeclination\t=",	cTelescopeProp.TargetDeclination);

	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlew is false, use SlewToTargetAsync");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	RightAscension=12.3456&Declination=55.9875
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToTargetAsync(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cTelescopeProp.AtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cTelescopeProp.CanSlewAsync)
	{
		if (cTelescopeProp.TargetRA_HasBeenSet && cTelescopeProp.TargetDec_HasBeenSet)
		{
			if ((cTelescopeProp.TargetRightAscension >= 0.0) && (cTelescopeProp.TargetRightAscension <= 24.0) &&
				(cTelescopeProp.TargetDeclination >= -90.0) && (cTelescopeProp.TargetDeclination <= 90.0))
			{
				alpacaErrCode	=	Telescope_SlewToRA_DEC(	cTelescopeProp.TargetRightAscension,
															cTelescopeProp.TargetDeclination,
															alpacaErrMsg);
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "values out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Target RA/DEC not set");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "CanSlewAsync is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_Park(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cTelescopeProp.CanPark)
	{
		//*	it is up to the sub class to actually set AtPark to true
		alpacaErrCode	=	Telescope_Park(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cTelescopeProp.CanPark is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Azimuth=11.33&Altitude=33.44"
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SyncToAltAz(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
double				newAz_degrees;
double				newAlt_degrees;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	ExtractAltAzArguments(reqData, &newAz_degrees, &newAlt_degrees, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		if (cTelescopeProp.CanSyncAltAz)
		{
			if (cTelescopeProp.Tracking == false)
			{
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Tracking is false");
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_Success;
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "CanSyncAltAz is false");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	ClientTransactionID=145&ClientID=1911&RightAscension=17.803889&Declination=-100
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SyncToCoordinates(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				newRightAscension;
double				newDeclination;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	ExtractRaDecArguments(reqData, &newRightAscension, &newDeclination, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{

		if (cTelescopeProp.AtPark)
		{
			alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	//	else if (cTelescopeProp.Tracking == false)
	//	{
	//		//*	make CONFORM happy
	//		alpacaErrCode	=	kASCOM_Err_InvalidOperation;
	//		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While not tracking");
	//		CONSOLE_DEBUG(alpacaErrMsg);
	//	}
		else if (cTelescopeProp.CanSync)
		{
			cTelescopeProp.RightAscension		=	newRightAscension;
			cTelescopeProp.Declination			=	newDeclination;
			cTelescopeProp.TargetRightAscension	=	newRightAscension;
			cTelescopeProp.TargetDeclination	=	newDeclination;

			cTelescopeProp.TargetDec_HasBeenSet	=	true;
			cTelescopeProp.TargetRA_HasBeenSet	=	true;

			alpacaErrCode	=	Telescope_SyncToRA_DEC(	cTelescopeProp.RightAscension,
														cTelescopeProp.Declination,
														alpacaErrMsg);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSync is false");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SyncToTarget(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	if (cTelescopeProp.AtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cTelescopeProp.CanSync)
	{
		cTelescopeProp.RightAscension		=	cTelescopeProp.TargetRightAscension;
		cTelescopeProp.Declination			=	cTelescopeProp.TargetDeclination;

		cTelescopeProp.TargetDec_HasBeenSet	=	true;
		cTelescopeProp.TargetRA_HasBeenSet	=	true;

		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSync is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_UnPark(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cTelescopeProp.CanUnpark)
	{
		alpacaErrCode			=	Telescope_UnPark(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "CanUnpark is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//	https://www.bogan.ca/astro/telescopes/coodcvtn.html
//	ra = Right Ascension of the celestial object
//	lst = Local Siderial Time
//	H = lst - ra
//	therefore
//	ra =  lst - H
//*****************************************************************************

//*****************************************************************************
//*	this is NOT part of the ASCOM standard
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_HourAngle(	TYPE_GetPutRequestData *reqData,
													char *alpacaErrMsg,
													const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					hourAngleString[48];
double					hourAngle_Degrees;

#define _FAKE_HOUR_ANGLE_

#ifdef _ENABLE_IMU_

//	hourAngle_Degrees			=	IMU_BNO055_GetAverageRoll();
	hourAngle_Degrees			=	IMU_GetAverageRoll();
	if (hourAngle_Degrees <= 0.0)
	{
		//*	we are on the east side of the pier, add 90 degrees
		hourAngle_Degrees	+=	90.0;
	}
	cTelescopeProp.HourAngle	=	hourAngle_Degrees / 15.0;

//	CONSOLE_DEBUG_W_DBL("hourAngle_Degrees\t=",	hourAngle_Degrees);
//	CONSOLE_DEBUG_W_DBL("HourAngle  \t=",	cTelescopeProp.HourAngle);

#elif defined(_FAKE_HOUR_ANGLE_)
	cTelescopeProp.SiderealTime	=	CalcSiderealTime_dbl(NULL, gObseratorySettings.Longitude_deg);
	cTelescopeProp.HourAngle	=	cTelescopeProp.SiderealTime - cTelescopeProp.RightAscension;
	if (cTelescopeProp.HourAngle < 0.0)
	{
		cTelescopeProp.HourAngle	+=	24.0;
	}
	hourAngle_Degrees	=	cTelescopeProp.HourAngle * 15;
#endif // _ENABLE_IMU_

	FormatHHMMSSdd(cTelescopeProp.HourAngle, hourAngleString, true);
//	CONSOLE_DEBUG(hourAngleString);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.HourAngle,
							INCLUDE_COMMA);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"HourAngle-degrees",
							hourAngle_Degrees,
							INCLUDE_COMMA);

	//*	extra... add the string value
	FormatHHMMSSdd(cTelescopeProp.HourAngle, hourAngleString, true);
	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"HourAngle-str",
								hourAngleString,
								INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	this is NOT part of the ASCOM standard
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_PhysicalSideOfPier(TYPE_GetPutRequestData *reqData,
															char *alpacaErrMsg,
															const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					extraString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	cTelescopeProp.PhysicalSideOfPier	=	Telescope_GetPhysicalSideOfPier();

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cTelescopeProp.PhysicalSideOfPier,
							INCLUDE_COMMA);

	switch(cTelescopeProp.PhysicalSideOfPier)
	{
		case kPierSide_NotAvailable:
			strcpy(extraString, "Not available");
			break;

		case kPierSide_pierUnknown:		//*	Polar (equatorial) mount other than German equatorial.
			strcpy(extraString, "Unknown or indeterminate");
			break;

		case kPierSide_pierEast:		//*	Altitude-Azimuth alignment.
			strcpy(extraString, "Physically East of the pier");
			break;

		case kPierSide_pierWest:
			strcpy(extraString, "Physically West of the pier");
			break;

		default:
			strcpy(extraString, "Error");
			break;
	}
	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"PhysicalSideOfPier-str",
								extraString,
								INCLUDE_COMMA);
	return(alpacaErrCode);
}

#ifdef _ENABLE_IMU_
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_IMU(	TYPE_GetPutRequestData *reqData,
												char *alpacaErrMsg,
												const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					imuArrayText[256];

	cTelescopeProp.IMU_Roll		=	IMU_GetAverageRoll();
	cTelescopeProp.IMU_Pitch	=	IMU_GetAveragePitch();
	cTelescopeProp.IMU_Yaw		=	IMU_GetAverageYaw();

	JsonResponse_Add_ArrayStart(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString);


	sprintf(imuArrayText, "%1.4f,%1.4f,%1.4f", cTelescopeProp.IMU_Roll, cTelescopeProp.IMU_Pitch, cTelescopeProp.IMU_Yaw);
	JsonResponse_Add_RawText(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								imuArrayText);

	JsonResponse_Add_ArrayEnd(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								INCLUDE_COMMA);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"IMU-Roll",
							cTelescopeProp.IMU_Roll,
							INCLUDE_COMMA);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"IMU-Pitch",
							cTelescopeProp.IMU_Pitch,
							INCLUDE_COMMA);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"IMU-Yaw",
							cTelescopeProp.IMU_Yaw,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}
#endif // _ENABLE_IMU_

//*****************************************************************************
bool	TelescopeDriver::DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen)
{
char					timeStampString[128];
struct timeval			currentTime;

	gettimeofday(&currentTime, NULL);
	FormatTimeStringISO8601(&currentTime, timeStampString);

	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Altitude",			cTelescopeProp.Altitude);
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"AtHome",			cTelescopeProp.AtHome);
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"AtPark",			cTelescopeProp.AtPark);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Azimuth",			cTelescopeProp.Azimuth);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Declination",		cTelescopeProp.Declination);
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"IsPulseGuiding",	cTelescopeProp.IsPulseGuiding);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"RightAscension",	cTelescopeProp.RightAscension);
	DeviceState_Add_Int(socketFD,	jsonTextBuffer, maxLen,	"SideOfPier",		cTelescopeProp.SideOfPier);
//	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"SiderealTime",		cTelescopeProp.SiderealTime);
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"Slewing",			cTelescopeProp.Slewing);
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"Tracking",			cTelescopeProp.Tracking);
	DeviceState_Add_Str(socketFD,	jsonTextBuffer, maxLen,	"UTCDate",			timeStampString);

	return(true);
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;
int		mySocket;

//	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(	reqData, alpacaErrMsg);

		//*	make local copies of the data structure to make the code easier to read
		mySocket	=	reqData->socket;


		alpacaErrCode	=	Get_Alignmentmode(			reqData, alpacaErrMsg, "alignmentmode");
		alpacaErrCode	=	Get_Altitude(				reqData, alpacaErrMsg, "altitude");
		alpacaErrCode	=	Get_ApertureArea(			reqData, alpacaErrMsg, "apertureArea");
		alpacaErrCode	=	Get_ApertureDiameter(		reqData, alpacaErrMsg, "apertureDiameter");
		alpacaErrCode	=	Get_AtHome(					reqData, alpacaErrMsg, "atHome");
		alpacaErrCode	=	Get_AtPark(					reqData, alpacaErrMsg, "atPark");
		alpacaErrCode	=	Get_Azimuth(				reqData, alpacaErrMsg, "azimuth");
		alpacaErrCode	=	Get_CanFindHome(			reqData, alpacaErrMsg, "CanFindHome");
		alpacaErrCode	=	Get_CanPark(				reqData, alpacaErrMsg, "CanPark");
		alpacaErrCode	=	Get_CanPulseGuide(			reqData, alpacaErrMsg, "CanPulseGuide");
		alpacaErrCode	=	Get_CanSetDeclinationRate(	reqData, alpacaErrMsg, "CanSetDeclinationRate");
		alpacaErrCode	=	Get_CanSetGuideRates(		reqData, alpacaErrMsg, "CanSetGuideRates");
		alpacaErrCode	=	Get_CanSetPark(				reqData, alpacaErrMsg, "CanSetPark");
		alpacaErrCode	=	Get_CanSetPierSide(			reqData, alpacaErrMsg, "CanSetPierSide");
		alpacaErrCode	=	Get_CanSetRightAscensionRate(reqData, alpacaErrMsg, "CanSetRightAscensionRate");
		alpacaErrCode	=	Get_CanSetTracking(			reqData, alpacaErrMsg, "CanSetTracking");
		alpacaErrCode	=	Get_CanSlew(				reqData, alpacaErrMsg, "CanSlew");
		alpacaErrCode	=	Get_CanSlewAltAz(			reqData, alpacaErrMsg, "CanSlewAltAz");
		alpacaErrCode	=	Get_CanSlewAltAzAsync(		reqData, alpacaErrMsg, "CanSlewAltAzAsync");
		alpacaErrCode	=	Get_CanSlewAsync(			reqData, alpacaErrMsg, "CanSlewAsync");
		alpacaErrCode	=	Get_CanSync(				reqData, alpacaErrMsg, "CanSync");
		alpacaErrCode	=	Get_CanSyncAltAz(			reqData, alpacaErrMsg, "CanSyncAltAz");
		alpacaErrCode	=	Get_CanUnpark(				reqData, alpacaErrMsg, "CanUnpark");
		alpacaErrCode	=	Get_Declination(			reqData, alpacaErrMsg, "Declination");
		alpacaErrCode	=	Get_DeclinationRate(		reqData, alpacaErrMsg, "DeclinationRate");
//		alpacaErrCode	=	Get_DestinationSideOfPier(	reqData, alpacaErrMsg, "DestinationSideOfPier");
		alpacaErrCode	=	Get_DoesRefraction(			reqData, alpacaErrMsg, "DoesRefraction");
		alpacaErrCode	=	Get_EquatorialSystem(		reqData, alpacaErrMsg, "EquatorialSystem");
		alpacaErrCode	=	Get_FocalLength(			reqData, alpacaErrMsg, "FocalLength");
		alpacaErrCode	=	Get_GuideRateDeclination(	reqData, alpacaErrMsg, "GuideRateDeclination");
		alpacaErrCode	=	Get_GuideRateRightAscension(reqData, alpacaErrMsg, "GuideRateRightAscension");
		alpacaErrCode	=	Get_IsPulseGuiding(			reqData, alpacaErrMsg, "IsPulseGuiding");
		alpacaErrCode	=	Get_RightAscension(			reqData, alpacaErrMsg, "RightAscension");
		alpacaErrCode	=	Get_RightAscensionRate(		reqData, alpacaErrMsg, "RightAscensionRate");
		alpacaErrCode	=	Get_SideOfPier(				reqData, alpacaErrMsg, "SideOfPier");
		alpacaErrCode	=	Get_SiderealTime(			reqData, alpacaErrMsg, "SiderealTime");
		alpacaErrCode	=	Get_SiteElevation(			reqData, alpacaErrMsg, "SiteElevation");
		alpacaErrCode	=	Get_SiteLatitude(			reqData, alpacaErrMsg, "SiteLatitude");
		alpacaErrCode	=	Get_SiteLongitude(			reqData, alpacaErrMsg, "SiteLongitude");
		alpacaErrCode	=	Get_Slewing(				reqData, alpacaErrMsg, "Slewing");
		alpacaErrCode	=	Get_SlewSettleTime(			reqData, alpacaErrMsg, "SlewSettleTime");
		alpacaErrCode	=	Get_TargetDeclination(		reqData, alpacaErrMsg, "TargetDeclination");
		alpacaErrCode	=	Get_TargetRightAscension(	reqData, alpacaErrMsg, "TargetRightAscension");
		alpacaErrCode	=	Get_Tracking(				reqData, alpacaErrMsg, "Tracking");
		alpacaErrCode	=	Get_TrackingRate(			reqData, alpacaErrMsg, "TrackingRate");
		alpacaErrCode	=	Get_TrackingRates(			reqData, alpacaErrMsg, "TrackingRates");
		alpacaErrCode	=	Get_UTCdate(				reqData, alpacaErrMsg, "UTCdate");

		//*	these are considered methods by ASCOM
//		alpacaErrCode	=	Get_AxisRates(				reqData, alpacaErrMsg, "AxisRates");
//		alpacaErrCode	=	Get_CanMoveAxis(			reqData, alpacaErrMsg, "CanMoveAxis");

		//===============================================================
		JsonResponse_Add_String(mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Comment",
								"Non-standard alpaca commands follow",
								INCLUDE_COMMA);

		alpacaErrCode	=	Get_HourAngle(			reqData, alpacaErrMsg, "HourAngle");
		alpacaErrCode	=	Get_PhysicalSideOfPier(	reqData, alpacaErrMsg, "PhysicalSideOfPier");
#ifdef _ENABLE_IMU_
char	imudataString[256];
		alpacaErrCode	=	Get_IMU(				reqData, alpacaErrMsg, "IMU");
		IMU_GetIMUtypeString(imudataString);
		JsonResponse_Add_String(mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"IMU-Type",
								imudataString,
								INCLUDE_COMMA);
#endif // _ENABLE_IMU_


		JsonResponse_Add_String(mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"version",
								gFullVersionString,
								INCLUDE_COMMA);


		alpacaErrCode	=	kASCOM_Err_Success;
		strcpy(alpacaErrMsg, "");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
void	TelescopeDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int		mySocketFD;

	if (reqData != NULL)
	{
		mySocketFD		=	reqData->socket;
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");

		SocketWriteData(mySocketFD,	"<H2>AlpacaPi Telescope</H2>\r\n");

		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	}
}

//*****************************************************************************
int32_t	TelescopeDriver::RunStateMachine(void)
{
	//*	this routine should be over-ridden, not required, but this is where you would do
	//*	any work you need to do.  Do it in the sub class NOT HERE
	return(5 * 1000 * 1000);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);

}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_FindHome(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_MoveAxis(const int axisNum, const double moveRate_degPerSec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;

	//*	needs to be over-ridden
	CONSOLE_DEBUG(__FUNCTION__);
	switch(axisNum)
	{
		case 0:
			if (moveRate_degPerSec > 0)
			{

			}
			else
			{

			}
			cTelescopeProp.Slewing	=	true;
			break;

		case 1:
			if (moveRate_degPerSec > 0)
			{

			}
			else
			{

			}
			cTelescopeProp.Slewing	=	true;
			break;

		default:
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
			break;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_Park(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden
	cTelescopeProp.AtPark	=	true;
	alpacaErrCode			=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_SetPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_SlewToAltAz(const double newAlt_Degrees, const double newAz_Degrees, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_TrackingOnOff(const bool newTrackingState, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden
	CONSOLE_DEBUG(__FUNCTION__);

	if (newTrackingState)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_TrackingRate(TYPE_DriveRates newTrackingRate, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	needs to be over-ridden
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Telescope_UnPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//*	can be over-ridden
	cTelescopeProp.AtPark	=	false;
	alpacaErrCode			=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
int	TelescopeDriver::Telescope_GetLimitSwitchStatus(const TYPE_LIMITSWITCH whichLimit)
{
int		limitSwichStatus;

	//*	needs to be over-ridden
	switch(whichLimit)
	{
		case kLimitSwitch_RA_East:
			limitSwichStatus	=	-1;
			break;

		case kLimitSwitch_RA_West:
			limitSwichStatus	=	-1;
			break;

		default:
			limitSwichStatus	=	-1;
			break;

	}
	return(limitSwichStatus);
}

//*****************************************************************************
TYPE_PierSide	TelescopeDriver::Telescope_GetPhysicalSideOfPier(void)
{
TYPE_PierSide	physicalSideOfPier	=	kPierSide_NotAvailable;

	//* this can be over-ridden
//	CONSOLE_DEBUG(__FUNCTION__);

	physicalSideOfPier	=   cTelescopeProp.PhysicalSideOfPier;

#ifdef _ENABLE_IMU_
double		imuRollAngle_Degrees;

//	CONSOLE_DEBUG("_ENABLE_IMU_ is enabled");
	//*	The sensor must be placed at the 12:00 position of the axis, that its
	//*	When the counter weights are at the lowest point, the ROLL axis MUST read ZERO (or very near zero)
	//*	Standing to the south of the telescope, looking along the RA axis at the North Celestial Poll
	//*	When the telescope rotates to the right(clockwise / east), the ROLL angle must be NEGATIVE
	//*	When the telescope rotates to the left (counter-clockwise /west ), the ROLL angle must be POSITIVE
	if (gIMUisOnLine)
	{
		imuRollAngle_Degrees	=	IMU_GetAverageRoll();
		if (imuRollAngle_Degrees < 0.0)
		{
			physicalSideOfPier	=	kPierSide_pierEast;
		}
		else
		{
			physicalSideOfPier	=	kPierSide_pierWest;
		}
	}
	else
	{
		physicalSideOfPier	=	kPierSide_pierUnknown;
	}
//	CONSOLE_DEBUG_W_DBL("imuRollAngle_Degrees\t=",	imuRollAngle_Degrees);
//	CONSOLE_DEBUG_W_NUM("physicalSideOfPier  \t=",	physicalSideOfPier);

#endif
	return(physicalSideOfPier);
}

//*****************************************************************************
//	kPierSide_pierEast		=	0,	//*	Normal pointing state - Mount on the East side of pier (looking West)
//	kPierSide_pierWest		=	1	//*	Through the pole pointing state - Mount on the West side of pier (looking East)
//*****************************************************************************
TYPE_PierSide	TelescopeDriver::Telescope_CalculateSideOfPier(void)
{
TYPE_PierSide	sideOfPier			=	kPierSide_NotAvailable;
TYPE_PierSide	physicalSideOfPier	=	kPierSide_NotAvailable;
double			myElevation;

//	CONSOLE_DEBUG(__FUNCTION__);
	physicalSideOfPier	=	Telescope_GetPhysicalSideOfPier();
	myElevation			=	cTelescopeProp.Declination - cTelescopeProp.SiteLatitude;
//	CONSOLE_DEBUG_W_DBL("cTelescopeProp.Declination \t=",	cTelescopeProp.Declination);
//	CONSOLE_DEBUG_W_DBL("cTelescopeProp.SiteLatitude\t=",	cTelescopeProp.SiteLatitude);
//	CONSOLE_DEBUG_W_DBL("myElevation                \t=",	myElevation);

	//----------------------------------------------------------------------
	//*	the logic here is not complete and might not be correct.
	//*	it is a good start to a difficult problem
	//----------------------------------------------------------------------
	if ((physicalSideOfPier == kPierSide_pierWest) && (myElevation < 90.0))
	{
		sideOfPier	=	kPierSide_pierWest;
	}
	else if ((physicalSideOfPier == kPierSide_pierEast) && (myElevation < 90.0))
	{
		sideOfPier	=	kPierSide_pierEast;
	}
	else
	{
		sideOfPier	=	kPierSide_pierUnknown;
	}
	return(sideOfPier);
}

//*****************************************************************************
//*	copied from Explore Scientific Driver.vb
//--------------------------------------------------------------------------------------
//Public Function DestinationSideOfPier(RightAscension As Double, Declination As Double) As PierSide Implements ITelescopeV3.DestinationSideOfPier
//
//	Try
//		Dim HA As Double
//		HA = SiderealTime - RightAscension
//		If HA < -12.0# Then
//			HA = HA + 24.0#
//		ElseIf HA >= 12.0# Then
//			HA = HA - 24.0#
//		End If
//
//		If Telescope.SiteLatitudeValue >= 0 Then
//			If HA < 0.0# Then
//				Return PierSide.pierWest
//			Else
//				Return PierSide.pierEast
//			End If
//		ElseIf Telescope.SiteLatitudeValue < 0 Then
//			If HA < 0.0# Then
//				Return PierSide.pierEast
//			Else
//				Return PierSide.pierWest
//			End If
//		End If
//
//		'If HA < 0.0# Then
//		'Return PierSide.pierWest
//		'Else
//		'Return PierSide.pierEast
//		'End If
//
//		'Return PierSide.pierUnknown
//	Catch ex As Exception
//		TL.LogMessage("DestinationSideOfPier", "Invalid Operation")
//		Throw New ASCOM.InvalidOperationException("DestinationSideOfPier")
//	End Try
//
//End Function
//*****************************************************************************
TYPE_PierSide	TelescopeDriver::Telescope_CalculateDestinationSideOfPier(	const double	newRtAscen_Hours,
																			const double	newDeclination_Degrees)
{
double			hourAngle;
TYPE_PierSide	mySideOfPier;

	mySideOfPier				=   kPierSide_pierUnknown;
	cTelescopeProp.SiderealTime	=	CalcSiderealTime_dbl(NULL, gObseratorySettings.Longitude_deg);
	hourAngle					=	cTelescopeProp.SiderealTime - newRtAscen_Hours;
	if (hourAngle < -12)
	{
		hourAngle	+=	24.0;
	}
	else if (hourAngle >= 12)
	{
		hourAngle	-=	24.0;
	}

	if (gObseratorySettings.Latitude_deg >= 0.0)
	{
		if (hourAngle < 0.0)
		{
			mySideOfPier	=   kPierSide_pierWest;
		}
		else
		{
			mySideOfPier	=   kPierSide_pierEast;
		}
	}
	else if (gObseratorySettings.Latitude_deg < 0.0)
	{
		if (hourAngle < 0.0)
		{
			mySideOfPier	=   kPierSide_pierEast;
		}
		else
		{
			mySideOfPier	=   kPierSide_pierWest;
		}
	}
	return(mySideOfPier);
}

//**************************************************************************************
void	DumpTelescopeDriverStruct(TYPE_TelescopeProperties *telescopeDriver)
{
	CONSOLE_DEBUG_W_BOOL("CanFindHome                \t=",	telescopeDriver->CanFindHome);
	CONSOLE_DEBUG_W_BOOL("CanMoveAxis[kAxis_RA]      \t=",	telescopeDriver->CanMoveAxis[kAxis_RA]);
	CONSOLE_DEBUG_W_BOOL("CanMoveAxis[kAxis_DEC]     \t=",	telescopeDriver->CanMoveAxis[kAxis_DEC]);
	CONSOLE_DEBUG_W_BOOL("CanMoveAxis[kAxis_Tertiary]\t=",	telescopeDriver->CanMoveAxis[kAxis_Tertiary]);
	CONSOLE_DEBUG_W_BOOL("CanPark                    \t=",	telescopeDriver->CanPark);
	CONSOLE_DEBUG_W_BOOL("CanPulseGuide              \t=",	telescopeDriver->CanPulseGuide);
	CONSOLE_DEBUG_W_BOOL("CanSetDeclinationRate      \t=",	telescopeDriver->CanSetDeclinationRate);
	CONSOLE_DEBUG_W_BOOL("CanSetGuideRates           \t=",	telescopeDriver->CanSetGuideRates);
	CONSOLE_DEBUG_W_BOOL("CanSetPark                 \t=",	telescopeDriver->CanSetPark);
	CONSOLE_DEBUG_W_BOOL("CanSetPierSide             \t=",	telescopeDriver->CanSetPierSide);
	CONSOLE_DEBUG_W_BOOL("CanSetRightAscensionRate   \t=",	telescopeDriver->CanSetRightAscensionRate);
	CONSOLE_DEBUG_W_BOOL("CanSetTracking             \t=",	telescopeDriver->CanSetTracking);
	CONSOLE_DEBUG_W_BOOL("CanSlew                    \t=",	telescopeDriver->CanSlew);
	CONSOLE_DEBUG_W_BOOL("CanSlewAltAz               \t=",	telescopeDriver->CanSlewAltAz);
	CONSOLE_DEBUG_W_BOOL("CanSlewAltAzAsync          \t=",	telescopeDriver->CanSlewAltAzAsync);
	CONSOLE_DEBUG_W_BOOL("CanSlewAsync               \t=",	telescopeDriver->CanSlewAsync);
	CONSOLE_DEBUG_W_BOOL("CanSync                    \t=",	telescopeDriver->CanSync);
	CONSOLE_DEBUG_W_BOOL("CanSyncAltAz               \t=",	telescopeDriver->CanSyncAltAz);
	CONSOLE_DEBUG_W_BOOL("CanUnpark                  \t=",	telescopeDriver->CanUnpark);
}

#ifdef _ENABLE_GLOBAL_GPS_
//**************************************************************************************
void	TelescopeDriver::Set_SiteLatitude(const double newSiteLatitude)
{
	if ((newSiteLatitude >= -90.0) && (newSiteLatitude <= 90.0))
	{
		cTelescopeProp.SiteLatitude		=	newSiteLatitude;
	}
}

//**************************************************************************************
void	TelescopeDriver::Set_SiteLongitude(const double newSiteLongitude)
{
	if ((newSiteLongitude >= -180.0) && (newSiteLongitude <= 360.0))
	{
		cTelescopeProp.SiteLongitude		=	newSiteLongitude;
	}
}
				void						Set_SiteAltitude(const double newSiteAlititude);
//**************************************************************************************
void	TelescopeDriver::Set_SiteAltitude(const double newSiteAlititude)
{
	if ((newSiteAlititude >= -200.0) && (newSiteAlititude <= 10000.0))
	{
		cTelescopeProp.SiteElevation		=	newSiteAlititude;
	}
}

#define	kTelescopeLatLonAvgCnt	32
//**************************************************************************************
static double	ComputeAveage(const double *numberList, const int avgCount)
{
double	total;
int		iii;

	total	=	0.0;
	for (iii=0; iii<avgCount; iii++)
	{
		total	+=	numberList[iii];
	}
	return(total / avgCount);
}

//**************************************************************************************
static void	*GPS_TelescopeThread(void *arg)
{
TelescopeDriver	*myTelescopeDriver;
double			latitudeArray[kTelescopeLatLonAvgCnt];
double			longitudeArray[kTelescopeLatLonAvgCnt];
double			altitudeArray[kTelescopeLatLonAvgCnt];
double			avgValue;
int				iii;
int				sleepDuration;
int				latLonArrayIdx;
int				altArrayIdx;
int				latlonUpdateCnt;
int				altitudeUpdateCnt;
//	CONSOLE_DEBUG(__FUNCTION__);

	if (arg != NULL)
	{
		for (iii=0; iii<kTelescopeLatLonAvgCnt; iii++)
		{
			latitudeArray[iii]	=	0.0;
			longitudeArray[iii]	=	0.0;
			altitudeArray[iii]	=	0.0;
		}
		latlonUpdateCnt		=	0;
		altitudeUpdateCnt	=	0;
		latLonArrayIdx		=	0;
		altArrayIdx			=	0;
		myTelescopeDriver	=	(TelescopeDriver *)arg;
		while (myTelescopeDriver->cGPStelescopeKeepRunning)
		{
			sleepDuration	=	60;
			if (gNMEAdata.validData)
			{
				if (gNMEAdata.validLatLon)
				{
//					CONSOLE_DEBUG("Updating Telescope lat/lon from gps");

					//*	save in the array for averaging
					latitudeArray[latLonArrayIdx]	=	gNMEAdata.lat_average;
					longitudeArray[latLonArrayIdx]	=	gNMEAdata.lon_average;
					latLonArrayIdx++;
					if (latLonArrayIdx >= kTelescopeLatLonAvgCnt)
					{
						latLonArrayIdx	=	0;
					}
					if (latlonUpdateCnt > kTelescopeLatLonAvgCnt)
					{
						avgValue	=	ComputeAveage(latitudeArray, kTelescopeLatLonAvgCnt);
						myTelescopeDriver->Set_SiteLatitude(avgValue);
						avgValue	=	ComputeAveage(longitudeArray, kTelescopeLatLonAvgCnt);
						myTelescopeDriver->Set_SiteLongitude(avgValue);
					}
					else
					{
						myTelescopeDriver->Set_SiteLatitude(gNMEAdata.lat_average);
						myTelescopeDriver->Set_SiteLongitude(gNMEAdata.lon_average);
					}
					latlonUpdateCnt++;
				}
				if (gNMEAdata.validAlt)
				{
//					CONSOLE_DEBUG("Updating Telescope alititude from gps");
					altitudeArray[altArrayIdx]	=	gNMEAdata.alt_average;
					altArrayIdx++;
					if (altArrayIdx >= kTelescopeLatLonAvgCnt)
					{
						altArrayIdx	=	0;
					}
					if (altitudeUpdateCnt > kTelescopeLatLonAvgCnt)
					{
						avgValue	=	ComputeAveage(altitudeArray, kTelescopeLatLonAvgCnt);
						myTelescopeDriver->Set_SiteAltitude(avgValue);
					}
					else
					{
						myTelescopeDriver->Set_SiteAltitude(gNMEAdata.alt_average);
					}
					altitudeUpdateCnt++;
				}
				if (latlonUpdateCnt < 10)
				{
					sleepDuration	=	15;
				}
			}
			sleep(sleepDuration);
		}
	}
//	else
//	{
//		CONSOLE_DEBUG("Invalid");
//	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT!!!!!!!!!!!!!!");
	return(NULL);
}
#endif // _ENABLE_GLOBAL_GPS_



#endif	//	_ENABLE_TELESCOPE_

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
//*	Dec  5,	2020	<MLS> CONFORM-telescope -> 25 errors, 0 warnings and 2 issues (tracking rates disabled)
//*	Dec  6,	2020	<MLS> CONFORM-telescope -> 4 errors, 0 warnings and 0 issues (tracking rates disabled)
//*	Dec  6,	2020	<MLS> CONFORM-telescope -> 35 errors, 0 warnings and 3 issues (tracking rates enabled)
//*	Dec  7,	2020	<MLS> CONFORM-telescope -> 0 errors, 0 warnings and 6 issues
//*****************************************************************************


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

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"JsonResponse.h"
#include	"sidereal.h"

#include	"observatory_settings.h"

#include	"telescopedriver.h"


//*****************************************************************************
//*	Telescope Specific Methods
//*****************************************************************************
enum
{

	kCmd_Telescope_alignmentmode	=	0,	//*	Returns the current mount alignment mode
	kCmd_Telescope_altitude,				//*	Returns the mount's Altitude above the horizon.
	kCmd_Telescope_aperturearea,			//*	Returns the telescope's aperture.
	kCmd_Telescope_aperturediameter,		//*	Returns the telescope's effective aperture.
	kCmd_Telescope_athome,					//*	Indicates whether the mount is at the home position.
	kCmd_Telescope_atpark,					//*	Indicates whether the telescope is at the park position.
	kCmd_Telescope_azimuth,					//*	Returns the telescope's aperture.
	kCmd_Telescope_canfindhome,				//*	Indicates whether the mount can find the home position.
	kCmd_Telescope_canpark,					//*	Indicates whether the telescope can be parked.
	kCmd_Telescope_canpulseguide,			//*	Indicates whether the telescope can be pulse guided.
	kCmd_Telescope_cansetdeclinationrate,	//*	Indicates whether the DeclinationRate property can be changed.
	kCmd_Telescope_cansetguiderates,		//*	Indicates whether the DeclinationRate property can be changed.
	kCmd_Telescope_cansetpark,				//*	Indicates whether the telescope park position can be set.
	kCmd_Telescope_cansetpierside,			//*	Indicates whether the telescope SideOfPier can be set.
	kCmd_Telescope_cansetrightascensionrate,//*	Indicates whether the RightAscensionRate property can be changed.
	kCmd_Telescope_cansettracking,			//*	Indicates whether the Tracking property can be changed.
	kCmd_Telescope_canslew,					//*	Indicates whether the telescope can slew synchronously.
	kCmd_Telescope_canslewaltaz,			//*	Indicates whether the telescope can slew synchronously to AltAz coordinates.
	kCmd_Telescope_canslewaltazasync,		//*	Indicates whether the telescope can slew asynchronously to AltAz coordinates.
	kCmd_Telescope_canslewasync,			//*	Indicates whether the telescope can slew asynchronously.
	kCmd_Telescope_cansync,					//*	Indicates whether the telescope can sync to equatorial coordinates.
	kCmd_Telescope_cansyncaltaz,			//*	Indicates whether the telescope can sync to local horizontal coordinates.
	kCmd_Telescope_canunpark,				//*	??
	kCmd_Telescope_declination,				//*	Returns the telescope's declination.
	kCmd_Telescope_declinationrate,			//*	Returns the telescope's declination tracking rate.
											//*	Sets the telescope's declination tracking rate.
	kCmd_Telescope_doesrefraction,			//*	Indicates whether atmospheric refraction is applied to coordinates.
											//*	Determines whether atmospheric refraction is applied to coordinates.
	kCmd_Telescope_equatorialsystem,		//*	Returns the current equatorial coordinate system used by this telescope.
	kCmd_Telescope_focallength,				//*	Returns the telescope's focal length in meters.
	kCmd_Telescope_guideratedeclination,	//*	Returns the current Declination rate offset for telescope guiding
											//*	Sets the current Declination rate offset for telescope guiding.
	kCmd_Telescope_guideraterightascension,	//*	Returns the current RightAscension rate offset for telescope guiding
											//*	Sets the current RightAscension rate offset for telescope guiding.
	kCmd_Telescope_ispulseguiding,			//*	Indicates whether the telescope is currently executing a PulseGuide command
	kCmd_Telescope_rightascension,			//*	Returns the telescope's right ascension coordinate.
	kCmd_Telescope_rightascensionrate,		//*	Returns the telescope's right ascension tracking rate.
											//*	Sets the telescope's right ascension tracking rate.
	kCmd_Telescope_sideofpier,				//*	Returns the mount's pointing state.
											//*	Sets the mount's pointing state.
	kCmd_Telescope_siderealtime,			//*	Returns the local apparent sidereal time.
	kCmd_Telescope_siteelevation,			//*	Returns the observing site's elevation above mean sea level.
											//*	Sets the observing site's elevation above mean sea level.
	kCmd_Telescope_sitelatitude,			//*	Returns the observing site's latitude.
											//*	Sets the observing site's latitude.
	kCmd_Telescope_sitelongitude,			//*	Returns the observing site's longitude.
											//*	Sets the observing site's longitude.
	kCmd_Telescope_slewing,					//*	Indicates whether the telescope is currently slewing.
	kCmd_Telescope_slewsettletime,			//*	Returns the post-slew settling time.
											//*	Sets the post-slew settling time.
	kCmd_Telescope_targetdeclination,		//*	Returns the current target declination.
											//*	Sets the target declination of a slew or sync.
	kCmd_Telescope_targetrightascension,	//*	Returns the current target right ascension.
											//*	Sets the target right ascension of a slew or sync.
	kCmd_Telescope_tracking,				//*	Indicates whether the telescope is tracking.
											//*	Enables or disables telescope tracking.
	kCmd_Telescope_trackingrate,			//*	Returns the current tracking rate.
											//*	Sets the mount's tracking rate.
	kCmd_Telescope_trackingrates,			//*	Returns a collection of supported DriveRates values.
	kCmd_Telescope_utcdate,					//*	Returns the UTC date/time of the telescope's internal clock.
											//*	Sets the UTC date/time of the telescope's internal clock.
	kCmd_Telescope_abortslew,				//*	Immediately stops a slew in progress.
	kCmd_Telescope_axisrates,				//*	Returns the rates at which the telescope may be moved about the specified axis.
	kCmd_Telescope_canmoveaxis,				//*	Indicates whether the telescope can move the requested axis.
	kCmd_Telescope_destinationsideofpier,	//*	Predicts the pointing state after a German equatorial mount slews to given coordinates.
	kCmd_Telescope_findhome,				//*	Moves the mount to the "home" position.
	kCmd_Telescope_moveaxis,				//*	Moves a telescope axis at the given rate.
	kCmd_Telescope_park,					//*	Park the mount
	kCmd_Telescope_pulseguide,				//*	Moves the scope in the given direction for the given time.
	kCmd_Telescope_setpark,					//*	Sets the telescope's park position
	kCmd_Telescope_slewtoaltaz,				//*	Synchronously slew to the given local horizontal coordinates.
	kCmd_Telescope_slewtoaltazasync,		//*	Asynchronously slew to the given local horizontal coordinates.
	kCmd_Telescope_slewtocoordinates,		//*	Synchronously slew to the given equatorial coordinates.
	kCmd_Telescope_slewtocoordinatesasync,	//*	Asynchronously slew to the given equatorial coordinates.
	kCmd_Telescope_slewtotarget,			//*	Synchronously slew to the TargetRightAscension and TargetDeclination coordinates.
	kCmd_Telescope_slewtotargetasync,		//*	Asynchronously slew to the TargetRightAscension and TargetDeclination coordinates.
	kCmd_Telescope_synctoaltaz,				//*	Syncs to the given local horizontal coordinates.
	kCmd_Telescope_synctocoordinates,		//*	Syncs to the given equatorial coordinates.
	kCmd_Telescope_synctotarget,			//*	Syncs to the TargetRightAscension and TargetDeclination coordinates.
	kCmd_Telescope_unpark,					//*	Unparks the mount.

	//*	added by MLS
	kCmd_Telescope_Extras,
	kCmd_Telescope_readall,

};

//*****************************************************************************
const TYPE_CmdEntry	gTelescopeCmdTable[]	=
{


	{	"alignmentmode",			kCmd_Telescope_alignmentmode,			kCmdType_GET	},	//*	Returns the current mount alignment mode
	{	"altitude",					kCmd_Telescope_altitude,				kCmdType_GET	},	//*	Returns the mount's Altitude above the horizon.
	{	"aperturearea",				kCmd_Telescope_aperturearea,			kCmdType_GET	},	//*	Returns the telescope's aperture.
	{	"aperturediameter",			kCmd_Telescope_aperturediameter,		kCmdType_GET	},	//*	Returns the telescope's effective aperture.
	{	"athome",					kCmd_Telescope_athome,					kCmdType_GET	},	//*	Indicates whether the mount is at the home position.
	{	"atpark",					kCmd_Telescope_atpark,					kCmdType_GET	},	//*	Indicates whether the telescope is at the park position.
	{	"azimuth",					kCmd_Telescope_azimuth,					kCmdType_GET	},	//*	Returns the telescope's aperture.
	{	"canfindhome",				kCmd_Telescope_canfindhome,				kCmdType_GET	},	//*	Indicates whether the mount can find the home position.
	{	"canpark",					kCmd_Telescope_canpark,					kCmdType_GET	},	//*	Indicates whether the telescope can be parked.
	{	"canpulseguide",			kCmd_Telescope_canpulseguide,			kCmdType_GET	},	//*	Indicates whether the telescope can be pulse guided.
	{	"cansetdeclinationrate",	kCmd_Telescope_cansetdeclinationrate,	kCmdType_GET	},	//*	Indicates whether the DeclinationRate property can be changed.
	{	"cansetguiderates",			kCmd_Telescope_cansetguiderates,		kCmdType_GET	},	//*	Indicates whether the DeclinationRate property can be changed.
	{	"cansetpark",				kCmd_Telescope_cansetpark,				kCmdType_GET	},	//*	Indicates whether the telescope park position can be set.
	{	"cansetpierside",			kCmd_Telescope_cansetpierside,			kCmdType_GET	},	//*	Indicates whether the telescope SideOfPier can be set.
	{	"cansetrightascensionrate",	kCmd_Telescope_cansetrightascensionrate,kCmdType_GET	},	//*	Indicates whether the RightAscensionRate property can be changed.
	{	"cansettracking",			kCmd_Telescope_cansettracking,			kCmdType_GET	},	//*	Indicates whether the Tracking property can be changed.
	{	"canslew",					kCmd_Telescope_canslew,					kCmdType_GET	},	//*	Indicates whether the telescope can slew synchronously.
	{	"canslewaltaz",				kCmd_Telescope_canslewaltaz,			kCmdType_GET	},	//*	Indicates whether the telescope can slew synchronously to AltAz coordinates.
	{	"canslewaltazasync",		kCmd_Telescope_canslewaltazasync,		kCmdType_GET	},	//*	Indicates whether the telescope can slew asynchronously to AltAz coordinates.
	{	"canslewasync",				kCmd_Telescope_canslewasync,			kCmdType_GET	},	//*	Indicates whether the telescope can slew asynchronously.
	{	"cansync",					kCmd_Telescope_cansync,					kCmdType_GET	},	//*	Indicates whether the telescope can sync to equatorial coordinates.
	{	"cansyncaltaz",				kCmd_Telescope_cansyncaltaz,			kCmdType_GET	},	//*	Indicates whether the telescope can sync to local horizontal coordinates.
	{	"canunpark",				kCmd_Telescope_canunpark,				kCmdType_GET	},	//*	??
	{	"declination",				kCmd_Telescope_declination,				kCmdType_GET	},	//*	Returns the telescope's declination.
	{	"declinationrate",			kCmd_Telescope_declinationrate,			kCmdType_BOTH	},	//*	Returns the telescope's declination tracking rate.
																								//*	Sets the telescope's declination tracking rate.
	{	"doesrefraction",			kCmd_Telescope_doesrefraction,			kCmdType_BOTH	},	//*	Indicates whether atmospheric refraction is applied to coordinates.
																								//*	Determines whether atmospheric refraction is applied to coordinates.
	{	"equatorialsystem",			kCmd_Telescope_equatorialsystem,		kCmdType_GET	},	//*	Returns the current equatorial coordinate system used by this telescope.
	{	"focallength",				kCmd_Telescope_focallength,				kCmdType_GET	},	//*	Returns the telescope's focal length in meters.
	{	"guideratedeclination",		kCmd_Telescope_guideratedeclination,	kCmdType_BOTH	},	//*	Returns the current Declination rate offset for telescope guiding
																								//*	Sets the current Declination rate offset for telescope guiding.
	{	"guideraterightascension",	kCmd_Telescope_guideraterightascension,	kCmdType_BOTH	},	//*	Returns the current RightAscension rate offset for telescope guiding
																								//*	Sets the current RightAscension rate offset for telescope guiding.
	{	"ispulseguiding",			kCmd_Telescope_ispulseguiding,			kCmdType_GET	},	//*	Indicates whether the telescope is currently executing a PulseGuide command
	{	"rightascension",			kCmd_Telescope_rightascension,			kCmdType_GET	},	//*	Returns the telescope's right ascension coordinate.
	{	"rightascensionrate",		kCmd_Telescope_rightascensionrate,		kCmdType_BOTH	},	//*	Returns the telescope's right ascension tracking rate.
																								//*	Sets the telescope's right ascension tracking rate.
	{	"sideofpier",				kCmd_Telescope_sideofpier,				kCmdType_BOTH	},	//*	Returns the mount's pointing state.
																								//*	Sets the mount's pointing state.
	{	"siderealtime",				kCmd_Telescope_siderealtime,			kCmdType_GET	},	//*	Returns the local apparent sidereal time.
	{	"siteelevation",			kCmd_Telescope_siteelevation,			kCmdType_BOTH	},	//*	Returns the observing site's elevation above mean sea level.
																								//*	Sets the observing site's elevation above mean sea level.
	{	"sitelatitude",				kCmd_Telescope_sitelatitude,			kCmdType_BOTH	},	//*	Returns the observing site's latitude.
																								//*	Sets the observing site's latitude.
	{	"sitelongitude",			kCmd_Telescope_sitelongitude,			kCmdType_BOTH	},	//*	Returns the observing site's longitude.
																								//*	Sets the observing site's longitude.
	{	"slewing",					kCmd_Telescope_slewing,					kCmdType_GET	},	//*	Indicates whether the telescope is currently slewing.
	{	"slewsettletime",			kCmd_Telescope_slewsettletime,			kCmdType_BOTH	},	//*	Returns the post-slew settling time.
																								//*	Sets the post-slew settling time.
	{	"targetdeclination",		kCmd_Telescope_targetdeclination,		kCmdType_BOTH	},	//*	Returns the current target declination.
																								//*	Sets the target declination of a slew or sync.
	{	"targetrightascension",		kCmd_Telescope_targetrightascension,	kCmdType_BOTH	},	//*	Returns the current target right ascension.,
																								//*	Sets the target right ascension of a slew or sync.
	{	"tracking",					kCmd_Telescope_tracking,				kCmdType_BOTH	},	//*	Indicates whether the telescope is tracking.
																								//*	Enables or disables telescope tracking.
	{	"trackingrate",				kCmd_Telescope_trackingrate,			kCmdType_BOTH	},	//*	Returns the current tracking rate.
																								//*	Sets the mount's tracking rate.
	{	"trackingrates",			kCmd_Telescope_trackingrates,			kCmdType_GET	},	//*	Returns a collection of supported DriveRates values.
	{	"utcdate",					kCmd_Telescope_utcdate,					kCmdType_BOTH	},	//*	Returns the UTC date/time of the telescope's internal clock.
																								//*	Sets the UTC date/time of the telescope's internal clock.
	{	"abortslew",				kCmd_Telescope_abortslew,				kCmdType_PUT	},	//*	Immediately stops a slew in progress.
	{	"axisrates",				kCmd_Telescope_axisrates,				kCmdType_GET	},	//*	Returns the rates at which the telescope may be moved about the specified axis.
	{	"canmoveaxis",				kCmd_Telescope_canmoveaxis,				kCmdType_GET	},	//*	Indicates whether the telescope can move the requested axis.
	{	"destinationsideofpier",	kCmd_Telescope_destinationsideofpier,	kCmdType_GET	},	//*	Predicts the pointing state after a German equatorial mount slews to given coordinates.
	{	"findhome",					kCmd_Telescope_findhome,				kCmdType_PUT	},	//*	Moves the mount to the "home" position.
	{	"moveaxis",					kCmd_Telescope_moveaxis,				kCmdType_PUT	},	//*	Moves a telescope axis at the given rate.
	{	"park",						kCmd_Telescope_park,					kCmdType_PUT	},	//*	Park the mount
	{	"pulseguide",				kCmd_Telescope_pulseguide,				kCmdType_PUT	},	//*	Moves the scope in the given direction for the given time.
	{	"setpark",					kCmd_Telescope_setpark,					kCmdType_PUT	},	//*	Sets the telescope's park position
	{	"slewtoaltaz",				kCmd_Telescope_slewtoaltaz,				kCmdType_PUT	},	//*	Synchronously slew to the given local horizontal coordinates.
	{	"slewtoaltazasync",			kCmd_Telescope_slewtoaltazasync,		kCmdType_PUT	},	//*	Asynchronously slew to the given local horizontal coordinates.
	{	"slewtocoordinates",		kCmd_Telescope_slewtocoordinates,		kCmdType_PUT	},	//*	Synchronously slew to the given equatorial coordinates.
	{	"slewtocoordinatesasync",	kCmd_Telescope_slewtocoordinatesasync,	kCmdType_PUT	},	//*	Asynchronously slew to the given equatorial coordinates.
	{	"slewtotarget",				kCmd_Telescope_slewtotarget,			kCmdType_PUT	},	//*	Synchronously slew to the TargetRightAscension and TargetDeclination coordinates.
	{	"slewtotargetasync",		kCmd_Telescope_slewtotargetasync,		kCmdType_PUT	},	//*	Asynchronously slew to the TargetRightAscension and TargetDeclination coordinates.
	{	"synctoaltaz",				kCmd_Telescope_synctoaltaz,				kCmdType_PUT	},	//*	Syncs to the given local horizontal coordinates.
	{	"synctocoordinates",		kCmd_Telescope_synctocoordinates,		kCmdType_PUT	},	//*	Syncs to the given equatorial coordinates.
	{	"synctotarget",				kCmd_Telescope_synctotarget,			kCmdType_PUT	},	//*	Syncs to the TargetRightAscension and TargetDeclination coordinates.
	{	"unpark",					kCmd_Telescope_unpark,					kCmdType_PUT	},	//*	Unparks the mount.


	//*	added by MLS
	{	"--extras",					kCmd_Telescope_Extras,					kCmdType_GET	},
	{	"readall",					kCmd_Telescope_readall,					kCmdType_GET	},

	{	"",						-1,	0x00	}
};

//**************************************************************************************
void	CreateTelescopeObjects(void)
{
	new TelescopeDriver();
}


//**************************************************************************************
TelescopeDriver::TelescopeDriver(void)
	:AlpacaDriver(kDeviceType_Telescope)
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceName, "Telescope");

	//*	set the defaults, everything to false or zero
	cDriverVersion				=	3;
	cAlginmentMode				=	kAlignmentMode_algGermanPolar;
	cAltitude					=	0.0;
	cApertureArea				=	0.0;
	cApertureDiameter			=	0.0;
	cAtHome						=	false;
	cAtPark						=	false;
	cAzimuth					=	0.0;
	cCanFindHome				=	false;
	cCanMoveAxis				=	false;
	cCanPark					=	false;
	cCanPulseGuide				=	false;
	cCanSetDeclinationRate		=	false;
	cCanSetGuideRates			=	false;
	cCanSetPark					=	false;
	cCanSetPierSide				=	false;
	cCanSetRightAscensionRate	=	false;
	cCanSetTracking				=	false;
	cCanSlew					=	false;
	cCanSlewAltAz				=	false;
	cCanSlewAltAzAsync			=	false;
	cCanSlewAsync				=	false;
	cCanSync					=	false;
	cCanSyncAltAz				=	false;
	cCanUnpark					=	false;
	cDeclination				=	0.0;
	cDeclinationRate			=	0.0;
	cDoesRefraction				=	false;
	cEquatorialSystem			=	kECT_equOther;
	cFocalLength				=	0.0;
	cGuideRateDeclination		=	0.0;
	cGuideRateRightAscension	=	0.0;
	cIsPulseGuiding				=	false;
	cRightAscension				=	0.0;
	cRightAscensionRate			=	0.0;
	cSideOfPier					=	kPierSide_pierUnknown;
	cSiderealTime				=	0.0;
	cSiteElevation				=	0.0;
	cSiteLatitude				=	0.0;
	cSiteLongitude				=	0.0;
	cSlewing					=	false;
	cSlewSettleTime				=	0;
	cTargetDec_HasBeenSet		=	false;
	cTargetRA_HasBeenSet		=	false;
	cTargetDeclination			=	0.0;
	cTargetRightAscension		=	0.0;
	cTracking					=	false;
	cTrackingRate				=	kDriveRate_driveSidereal;
//+	cTrackingRates;
//+	cUTCDate;

	if (gObseratorySettings.ValidInfo)
	{
		//*	now set the things we do know
		cSiteElevation			=	gObseratorySettings.Elevation_m;
		cSiteLatitude			=	gObseratorySettings.Latitude;
		cSiteLongitude			=	gObseratorySettings.Longitude;
	}


	//*	these are temporary to get CONFORM to work
	cApertureDiameter			=	16 * 25.4;
	cFocalLength				=	cApertureDiameter * 4;
	cApertureArea				=	M_PI * ((cApertureDiameter/2) * (cApertureDiameter/2));



	//*	turn some of the features back on for CONFORM testing
	cCanSetTracking				=	true;
	cCanPark					=	true;
	cCanUnpark					=	true;
	cCanSync					=	true;
	cCanSlew					=	true;
	cCanSetRightAscensionRate	=	true;
	cCanSetDeclinationRate		=	true;
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriver::~TelescopeDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;

	strcpy(alpacaErrMsg, "");
	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;

	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer, kMaxJsonBuffLen);

	//*	this is not part of the protocol, I am using it for testing
	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Device",
								cDeviceName,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Command",
								reqData->deviceCommand,
								INCLUDE_COMMA);

	//*	look up the command
//	CONSOLE_DEBUG_W_STR("deviceCommand\t=", reqData->deviceCommand);
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
		alpacaErrCode	=	Put_AbortSlew(reqData, alpacaErrMsg);
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
		alpacaErrCode	=	Put_FindHome(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_moveaxis:					//*	Moves a telescope axis at the given rate.
		alpacaErrCode	=	Put_MoveAxis(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_park:						//*	Park the mount
		alpacaErrCode	=	Put_Park(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_pulseguide:					//*	Moves the scope in the given direction for the given time.
		alpacaErrCode	=	Put_PulseGuide(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_setpark:					//*	Sets the telescope's park position
		alpacaErrCode	=	Put_SetPark(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_slewtoaltaz:				//*	Synchronously slew to the given local horizontal coordinates.
		alpacaErrCode	=	Put_SlewToAltAz(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_slewtoaltazasync:			//*	Asynchronously slew to the given local horizontal coordinates.
		alpacaErrCode	=	Put_SlewToAltAzAsync(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_slewtocoordinates:			//*	Synchronously slew to the given equatorial coordinates.
		alpacaErrCode	=	Put_SlewToCoordinates(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_slewtocoordinatesasync:		//*	Asynchronously slew to the given equatorial coordinates.
		alpacaErrCode	=	Put_SlewToCoordinatesAsync(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_slewtotarget:				//*	Synchronously slew to the TargetRightAscension and TargetDeclination coordinates.
		alpacaErrCode	=	Put_SlewToTarget(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_slewtotargetasync:			//*	Asynchronously slew to the TargetRightAscension and TargetDeclination coordinates.
		alpacaErrCode	=	Put_SlewToTargetAsync(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_synctoaltaz:				//*	Syncs to the given local horizontal coordinates.
		alpacaErrCode	=	Put_SyncToAltAz(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_synctocoordinates:			//*	Syncs to the given equatorial coordinates.
		alpacaErrCode	=	Put_SyncToCoordinates(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_synctotarget:				//*	Syncs to the TargetRightAscension and TargetDeclination coordinates.
		alpacaErrCode	=	Put_SyncToTarget(reqData, alpacaErrMsg);
		break;

	case kCmd_Telescope_unpark:						//*	Unparks the mount.
		alpacaErrCode	=	Put_UnPark(reqData, alpacaErrMsg);
		break;


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
			CONSOLE_ABORT("aborting");
		}
		alpacaErrCode	=	ProcessCommand_Common(reqData, cmdEnumValue, alpacaErrMsg);
		break;
	}
//	CONSOLE_DEBUG_W_NUM("Calling RecordCmdStats(), cmdEnumValue=", cmdEnumValue);
	RecordCmdStats(cmdEnumValue, reqData->get_putIndicator, alpacaErrCode);

	//*	send the response information
	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ClientTransactionID",
								gClientTransactionID,
								INCLUDE_COMMA);

	JsonResponse_Add_Int32(		mySocket,
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
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								kInclude_HTTP_Header);

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
								cAlginmentMode,
								INCLUDE_COMMA);

		switch(cAlginmentMode)
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
							cAltitude,
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
							cApertureArea,
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
							cApertureDiameter,
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
							cAtHome,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_AtPark(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cAtPark,
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
							cAzimuth,
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
							cCanFindHome,
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
							cCanPark,
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
							cCanPulseGuide,
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
							cCanSetDeclinationRate,
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
							cCanSetGuideRates,
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
							cCanSetPark,
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
							cCanSetPierSide,
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
							cCanSetRightAscensionRate,
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
							cCanSetTracking,
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
							cCanSlew,
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
							cCanSlewAltAz,
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
							cCanSlewAltAzAsync,
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
							cCanSlewAsync,
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
							cCanSync,
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
							cCanSyncAltAz,
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
							cCanUnpark,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_Declination(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cDeclination,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_DeclinationRate(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	if (cCanSetDeclinationRate == false)
	{
		cDeclinationRate	=	0.0;
	}

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cDeclinationRate,
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

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCanSetDeclinationRate)
	{
		decRateFound		=	GetKeyWordArgument(	reqData->contentData,
													"DeclinationRate",
													decRateString,
													(sizeof(decRateString) -1));
		if (decRateFound)
		{
			newDecRate			=	atof(decRateString);
			CONSOLE_DEBUG_W_DBL("newDecRate\t=", newDecRate);
			cDeclinationRate	=	newDecRate;
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			CONSOLE_DEBUG("DeclinationRate not found in command string");
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "DeclinationRate missing");
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetDeclinationRate is false");
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_DoesRefraction(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cDoesRefraction,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_DoesRefraction(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					doseRefractionFound;
char					doseRefractionString[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	doseRefractionFound		=	GetKeyWordArgument(	reqData->contentData,
												"DoesRefraction",
												doseRefractionString,
												(sizeof(doseRefractionString) -1));
	if (doseRefractionFound)
	{
		cDoesRefraction		=	IsTrueFalse(doseRefractionString);

		alpacaErrCode		=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "DoesRefraction is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_EquatorialSystem(	TYPE_GetPutRequestData *reqData,
															char *alpacaErrMsg,
															const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					extraString[128];

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cEquatorialSystem,
							INCLUDE_COMMA);

		switch(cEquatorialSystem)
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
							cFocalLength,
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
							cGuideRateDeclination,
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

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCanSetGuideRates)
	{
		guideRateDeclinationFound		=	GetKeyWordArgument(	reqData->contentData,
														"GuideRateDeclination",
														guideRateDeclinationStr,
														(sizeof(guideRateDeclinationStr) -1));
		if (guideRateDeclinationFound)
		{
			newGuideRateDeclination	=	atof(guideRateDeclinationStr);
			CONSOLE_DEBUG_W_STR("guideRateDeclinationStr\t=", guideRateDeclinationStr);
			CONSOLE_DEBUG_W_DBL("newGuideRateDeclination\t=", newGuideRateDeclination);

			cGuideRateDeclination	=	newGuideRateDeclination;
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "GuideRateDeclination is missing");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetGuideRates is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

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
							cGuideRateRightAscension,
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

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCanSetGuideRates)
	{
		guideRateRightAscensionFound		=	GetKeyWordArgument(	reqData->contentData,
														"GuideRateRightAscension",
														guideRateRightAscensionStr,
														(sizeof(guideRateRightAscensionStr) -1));
		if (guideRateRightAscensionFound)
		{
			newGuideRateRightAscension	=	atof(guideRateRightAscensionStr);
			CONSOLE_DEBUG_W_STR("guideRateRightAscensionStr\t=", guideRateRightAscensionStr);
			CONSOLE_DEBUG_W_DBL("newGuideRateRightAscension\t=", newGuideRateRightAscension);

			cGuideRateRightAscension	=	newGuideRateRightAscension;
			alpacaErrCode				=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "GuideRateRightAscension is missing");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetGuideRates is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_IsPulseGuiding(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cIsPulseGuiding,
							INCLUDE_COMMA);

	if (cCanPulseGuide)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanPulseGuide is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_RightAscension(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cRightAscension,
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
							cRightAscensionRate,
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

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCanSetRightAscensionRate)
	{
		rightAscenRateFound		=	GetKeyWordArgument(	reqData->contentData,
														"RightAscensionRate",
														rightAscenRateString,
														(sizeof(rightAscenRateString) -1));
		if (rightAscenRateFound)
		{
			newrightAscenRate	=	atof(rightAscenRateString);
			CONSOLE_DEBUG_W_STR("rightAscenRateString\t=", rightAscenRateString);
			CONSOLE_DEBUG_W_DBL("newrightAscenRate\t=", newrightAscenRate);

			cRightAscensionRate	=	newrightAscenRate;
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			CONSOLE_DEBUG("RightAscensionRate not found in command string");
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "RightAscensionRate is missing");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSetRightAscensionRate is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}



//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SideOfPier(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
char					extraString[128];

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cSideOfPier,
							INCLUDE_COMMA);

		switch(cSideOfPier)
		{
			case kPierSide_pierEast:		//*	Altitude-Azimuth alignment.
				strcpy(extraString, "Normal pointing state - Mount on the East side of pier (looking West)");
				break;

			case kPierSide_pierUnknown:		//*	Polar (equatorial) mount other than German equatorial.
				strcpy(extraString, "Unknown or indeterminate.");
				break;

			case kPierSide_pierWest:
				strcpy(extraString, "Through the pole pointing state - Mount on the West side of pier (looking East)");
				break;

		}
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"SideOfPier-str",
									extraString,
									INCLUDE_COMMA);


	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SiderealTime(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	cSiderealTime	=	CalcSiderealTime_dbl(NULL, gObseratorySettings.Longitude);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cSiderealTime,
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
							cSiteElevation,
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

//	CONSOLE_DEBUG(__FUNCTION__);

	siteElevFound		=	GetKeyWordArgument(	reqData->contentData,
												"SiteElevation",
												siteElevString,
												(sizeof(siteElevString) -1));
	if (siteElevFound)
	{
		newElevation	=	atof(siteElevString);
//		CONSOLE_DEBUG_W_STR("siteElevString\t=", siteElevString);
//		CONSOLE_DEBUG_W_DBL("newElevitude\t=", newElevation);

		if ((newElevation >= -300.0) && (newElevation <= 10000.0))
		{
			cSiteElevation	=	newElevation;
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
//			CONSOLE_DEBUG_W_DBL("INVALID VALUE, newElevation\t=", newElevation);
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteElevation is out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteElevation is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SiteLatitude(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cSiteLatitude,
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

//curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/telescope/0/sitelatitude"
//	-H "accept: application/json"
//	-H "Content-Type: application/x-www-form-urlencoded"
//	-d "SiteLatitude=51.3&ClientID=1&ClientTransactionID=3"

	siteLatFound		=	GetKeyWordArgument(	reqData->contentData,
												"SiteLatitude",
												siteLatString,
												(sizeof(siteLatString) -1));
	if (siteLatFound)
	{
		newLatitude	=	atof(siteLatString);

		if ((newLatitude >= -90.0) && (newLatitude <= 90.0))
		{
			cSiteLatitude	=	newLatitude;
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLatitude is out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLatitude is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SiteLongitude(	TYPE_GetPutRequestData *reqData,
														char *alpacaErrMsg,
														const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cSiteLongitude,
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


	siteLonFound		=	GetKeyWordArgument(	reqData->contentData,
												"SiteLongitude",
												siteLonString,
												(sizeof(siteLonString) -1));
	if (siteLonFound)
	{
		newLongitude	=	atof(siteLonString);
//		CONSOLE_DEBUG_W_STR("siteLonString\t=", siteLonString);
//		CONSOLE_DEBUG_W_DBL("newLongitude\t=", newLongitude);

		if ((newLongitude >= -180.0) && (newLongitude <= 180.0))
		{
			cSiteLongitude	=	newLongitude;
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
//			CONSOLE_DEBUG_W_DBL("INVALID VALUE, newLongitude\t=", newLongitude);
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLongitude is out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SiteLongitude is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

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
							cSlewing,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_SlewSettleTime(	TYPE_GetPutRequestData *reqData,
															char *alpacaErrMsg,
															const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cSlewSettleTime,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewSettleTime(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				slewSettleTimeFound;
char				slewSettleTimeString[64];
short				newSlewSettleTime;

//	CONSOLE_DEBUG(__FUNCTION__);

	slewSettleTimeFound		=	GetKeyWordArgument(	reqData->contentData,
												"SlewSettleTime",
												slewSettleTimeString,
												(sizeof(slewSettleTimeString) -1));
	if (slewSettleTimeFound)
	{
		newSlewSettleTime	=	atoi(slewSettleTimeString);
//		CONSOLE_DEBUG_W_STR("slewSettleTimeString\t=", slewSettleTimeString);
//		CONSOLE_DEBUG_W_DBL("newSlewSettleTime\t=", newSlewSettleTime);
		if (newSlewSettleTime >= 0)
		{
			cSlewSettleTime	=	newSlewSettleTime;
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewSettleTime is out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewSettleTime is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

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
							cTargetDeclination,
							INCLUDE_COMMA);
	if (cTargetDec_HasBeenSet)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidOperation;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetDeclination has not been set");
		CONSOLE_DEBUG(alpacaErrMsg);
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

	targetDeclinationFound		=	GetKeyWordArgument(	reqData->contentData,
													"TargetDeclination",
													targetDeclinationString,
													(sizeof(targetDeclinationString) -1));
	if (targetDeclinationFound)
	{
		newTargetDeclination		=	atof(targetDeclinationString);
		CONSOLE_DEBUG_W_DBL("newTargetDeclination\t=", newTargetDeclination);

		if ((newTargetDeclination >= -90.0) && (newTargetDeclination <= 90.0))
		{
			cDeclination			=	newTargetDeclination;
			cTargetDeclination		=	newTargetDeclination;
			cTargetDec_HasBeenSet	=	true;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetDeclination out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetDeclination missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
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
							cTargetRightAscension,
							INCLUDE_COMMA);

	if (cTargetRA_HasBeenSet)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidOperation;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetRightAscension has not been set");
		CONSOLE_DEBUG(alpacaErrMsg);
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
	CONSOLE_DEBUG(reqData->contentData);

	targetRightAscensionFound		=	GetKeyWordArgument(	reqData->contentData,
													"TargetRightAscension",
													targetRightAscensionString,
													(sizeof(targetRightAscensionString) -1));
	if (targetRightAscensionFound)
	{
		newTargetRightAscension		=	atof(targetRightAscensionString);
		CONSOLE_DEBUG_W_DBL("newTargetRightAscension\t=", newTargetRightAscension);

		if ((newTargetRightAscension >= 0.0) && (newTargetRightAscension <= 24.0))
		{
			cRightAscension			=	newTargetRightAscension;
			cTargetRightAscension	=	newTargetRightAscension;
			cTargetRA_HasBeenSet	=	true;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetRightAscension out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TargetRightAscension missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
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
							cTracking,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_Tracking(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					trackingFound;
char					trackingString[64];

	CONSOLE_DEBUG(__FUNCTION__);

	if (cCanSetTracking)
	{
		trackingFound		=	GetKeyWordArgument(	reqData->contentData,
													"Tracking",
													trackingString,
													(sizeof(trackingString) -1));
		if (trackingFound)
		{
			cTracking		=	IsTrueFalse(trackingString);
			if (cTracking)
			{
				cAtPark			=	false;
			}

			alpacaErrCode	=	kASCOM_Err_Success;

			CONSOLE_DEBUG(cTracking ? "Tracking is ENABLED" : "Tracking is DISABLED");
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
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
							cTrackingRate,
							INCLUDE_COMMA);

	switch(cTrackingRate)
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

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	trackingRateFound		=	GetKeyWordArgument(	reqData->contentData,
													"TrackingRate",
													trackingRateString,
													(sizeof(trackingRateString) -1));
	if (trackingRateFound)
	{
		newtrackingRate		=	(TYPE_DriveRates)atoi(trackingRateString);
		CONSOLE_DEBUG_W_NUM("newtrackingRate\t=", newtrackingRate);
		if ((newtrackingRate >= kDriveRate_driveSidereal) && (newtrackingRate <= kDriveRate_driveKing))
		{
			cTrackingRate		=	newtrackingRate;
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TrackingRate is out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		CONSOLE_DEBUG("TrackingRate not found in command string");
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TrackingRate is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

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
struct timeval			currentTIme;

	gettimeofday(&currentTIme, NULL);

	FormatTimeStringISO8601(&currentTIme, timeStampString);
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
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

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

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);
	if (cAtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
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

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);


	axisFound		=	GetKeyWordArgument(	reqData->contentData,
													"Axis",
													axisString,
													(sizeof(axisString) -1));
	if (axisFound)
	{
		axisNumber		=	atoi(axisString);
		CONSOLE_DEBUG_W_NUM("axisNumber\t=", axisNumber);
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
									1.5,
									INCLUDE_COMMA);

			JsonResponse_Add_Double(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Maximum",
									1.5,
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
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

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
	CONSOLE_DEBUG(reqData->contentData);

	axisFound		=	GetKeyWordArgument(	reqData->contentData,
													"Axis",
													axisString,
													(sizeof(axisString) -1));
	if (axisFound)
	{
		axisNumber		=	atoi(axisString);
		CONSOLE_DEBUG_W_NUM("axisNumber\t=", axisNumber);
		if ((axisNumber >= 0) && (axisNumber <= 2))
		{

			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									cCanMoveAxis,
									INCLUDE_COMMA);

		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is out of bounds");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Get_DestinationSideOfPier(	TYPE_GetPutRequestData	*reqData,
																char					*alpacaErrMsg,
																const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_FindHome(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	if (cCanFindHome)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
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


//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	if (cCanMoveAxis)
	{
		axisFound		=	GetKeyWordArgument(	reqData->contentData,
														"Axis",
														axisString,
														(sizeof(axisString) -1));
		rateFound		=	GetKeyWordArgument(	reqData->contentData,
														"Rate",
														rateString,
														(sizeof(rateString) -1));

		if (axisFound)
		{
			axisNumber		=	atoi(axisString);
			CONSOLE_DEBUG_W_NUM("axisNumber\t=", axisNumber);
			if ((axisNumber >= 0) && (axisNumber <= 2))
			{
				if (rateFound)
				{
					newRate		=	atof(rateString);

					JsonResponse_Add_Double(reqData->socket,
											reqData->jsonTextBuffer,
											kMaxJsonBuffLen,
											"Rate",
											newRate,
											INCLUDE_COMMA);
				}
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Axis is missing");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanMoveAxis is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_PulseGuide(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					axisFound;
bool					rateFound;
char					axisString[64];
char					rateString[64];
int						axisNumber;
double					newRate;


//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	if (cCanPulseGuide)
	{
		axisFound		=	GetKeyWordArgument(	reqData->contentData,
														"Axis",
														axisString,
														(sizeof(axisString) -1));
		rateFound		=	GetKeyWordArgument(	reqData->contentData,
														"Rate",
														rateString,
														(sizeof(rateString) -1));

		if (axisFound)
		{
			axisNumber		=	atoi(axisString);
			CONSOLE_DEBUG_W_NUM("axisNumber\t=", axisNumber);
			if ((axisNumber >= 0) && (axisNumber <= 2))
			{
				if (rateFound)
				{
					newRate		=	atof(rateString);

					JsonResponse_Add_Double(reqData->socket,
											reqData->jsonTextBuffer,
											kMaxJsonBuffLen,
											"Rate",
											newRate,
											INCLUDE_COMMA);
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("INVALID VALUE: axisNumber\t=", axisNumber);
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
			}
		}
		else
		{
			CONSOLE_DEBUG("Axis not found in command string");
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanPulseGuide is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SetPark(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cCanSetPark)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
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
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cCanSlewAltAz)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlewAltAz is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToAltAzAsync(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
//bool				rightAscensionFound;
//char				rightAscensionStr[64];
//bool				declinationFound;
//char				declinationStr[64];

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cCanSlewAltAzAsync)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlewAltAzAsync is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}


//*****************************************************************************
//*	ClientTransactionID=105&ClientID=46225&RightAscension=18.14&Declination=1
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToCoordinates(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				rightAscensionFound;
char				rightAscensionStr[64];
bool				declinationFound;
char				declinationStr[64];
double				newRightAscension;
double				newDeclination;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cAtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cCanSlew)
	{
		rightAscensionFound		=	GetKeyWordArgument(	reqData->contentData,
														"RightAscension",
														rightAscensionStr,
														(sizeof(rightAscensionStr) -1));

		declinationFound		=	GetKeyWordArgument(	reqData->contentData,
														"Declination",
														declinationStr,
														(sizeof(rightAscensionStr) -1));

		if (rightAscensionFound && declinationFound)
		{
			newRightAscension	=	atof(rightAscensionStr);
			newDeclination		=	atof(declinationStr);

			CONSOLE_DEBUG_W_DBL("newRightAscension\t=",	newRightAscension);
			CONSOLE_DEBUG_W_DBL("newDeclination\t=",	newDeclination);

			if ((newRightAscension >= 0.0) && (newRightAscension <= 24.0) &&
				(newDeclination >= -90.0) && (newDeclination <= 90.0))
			{
				cRightAscension			=	newRightAscension;
				cDeclination			=	newDeclination;
				cTargetRightAscension	=	newRightAscension;
				cTargetDeclination		=	newDeclination;

				cTargetDec_HasBeenSet	=	true;
				cTargetRA_HasBeenSet	=	true;

				alpacaErrCode			=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "values out of bounds");

				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Values out of bounds");


			//	sprintf(alpacaErrMsg, "AlpacaPi: values out of bounds, l#%d", __LINE__);
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlew is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToCoordinatesAsync(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cCanSlewAsync)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlewAsync is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToTarget(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cAtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cCanSlew)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
		cAtPark			=	false;
		CONSOLE_DEBUG_W_DBL("cTargetRightAscension\t=",	cTargetRightAscension);
		CONSOLE_DEBUG_W_DBL("cTargetDeclination\t=",	cTargetDeclination);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlew is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SlewToTargetAsync(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cAtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cCanSlewAsync)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSlew is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_Park(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCanPark)
	{
		alpacaErrCode	=	kASCOM_Err_Success;

		cAtPark			=	true;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanPark is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}



//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SyncToAltAz(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cCanSyncAltAz)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSyncAltAz is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	ClientTransactionID=145&ClientID=1911&RightAscension=17.803889&Declination=-100
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SyncToCoordinates(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				rightAscensionFound;
char				rightAscensionStr[64];
double				newRightAscension;

bool				declinationFound;
char				declinationStr[64];
double				newDeclination;


	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);

	if (cAtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cCanSync)
	{
		rightAscensionFound		=	GetKeyWordArgument(	reqData->contentData,
														"RightAscension",
														rightAscensionStr,
														(sizeof(rightAscensionStr) -1));
		declinationFound		=	GetKeyWordArgument(	reqData->contentData,
														"Declination",
														declinationStr,
														(sizeof(declinationStr) -1));
		if (rightAscensionFound && declinationFound)
		{
			newRightAscension	=	atof(rightAscensionStr);
			newDeclination		=	atof(declinationStr);

			if ((newRightAscension >= 0.0) && (newRightAscension <= 24.0) &&
				(newDeclination >= -90.0) && (newDeclination <= 90.0))
			{
				cRightAscension			=	newRightAscension;
				cDeclination			=	newDeclination;
				cTargetRightAscension	=	newRightAscension;
				cTargetDeclination		=	newDeclination;

				cTargetDec_HasBeenSet	=	true;
				cTargetRA_HasBeenSet	=	true;

				alpacaErrCode			=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "values out of bounds");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cmd values missing");
				CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanSync is false");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Put_SyncToTarget(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(reqData->contentData);

	if (cAtPark)
	{
		alpacaErrCode	=	kASCOM_Err_InvalidWhileParked;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid While Parked");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (cCanSync)
	{
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

	if (cCanUnpark)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
		cAtPark			=	false;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "cCanUnpark is false");
//		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
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

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
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
		alpacaErrCode	=	Get_AxisRates(				reqData, alpacaErrMsg, "AxisRates");
		alpacaErrCode	=	Get_CanMoveAxis(			reqData, alpacaErrMsg, "CanMoveAxis");
		alpacaErrCode	=	Get_DestinationSideOfPier(	reqData, alpacaErrMsg, "DestinationSideOfPier");


		//===============================================================
		JsonResponse_Add_String(mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Comment",
								"Non-standard alpaca commands follow",
								INCLUDE_COMMA);

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

		SocketWriteData(mySocketFD,	"<H2>Telescope</H2>\r\n");

		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		GenerateHTMLcmdLinkTable(mySocketFD, "telescope", cDeviceNum, gTelescopeCmdTable);
	}
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriver::Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg)
{
	TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be over-ridden");

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);

}

#endif	//	_ENABLE_TELESCOPE_

//*****************************************************************************
//*	Name:			telescope_AlpacaCmds.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:	Command table for Telescope
//*					This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created telescope_AlpacaCmds.cpp
//*	Jan 15,	2024	<MLS> Added alternate command table to telescope
//*****************************************************************************


#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _TELESCOPE_ALPACA_CMDS_H_
	#include	"telescope_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gTelescopeCmdTable[]	=
{
	{	"alignmentmode",			kCmd_Telescope_alignmentmode,			kCmdType_GET	},	//*	Returns the current mount alignment mode
	{	"altitude",					kCmd_Telescope_altitude,				kCmdType_GET	},	//*	Returns the mount's Altitude above the horizon.
	{	"aperturearea",				kCmd_Telescope_aperturearea,			kCmdType_GET	},	//*	Returns the telescope's aperture.
	{	"aperturediameter",			kCmd_Telescope_aperturediameter,		kCmdType_GET	},	//*	Returns the telescope's effective aperture.
	{	"athome",					kCmd_Telescope_athome,					kCmdType_GET	},	//*	Indicates whether the mount is at the home position.
	{	"atpark",					kCmd_Telescope_atpark,					kCmdType_GET	},	//*	Indicates whether the telescope is at the park position.
	{	"azimuth",					kCmd_Telescope_azimuth,					kCmdType_GET	},	//*	Returns the telescope's azimuth.
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
	{	"hourangle",				kCmd_Telescope_hourangle,				kCmdType_GET	},
	{	"physicalsideofpier",		kCmd_Telescope_physicalsideofpier,		kCmdType_GET	},
#ifdef _ENABLE_IMU_
	{	"imu",						kCmd_Telescope_imu,						kCmdType_GET	},
#endif
	{	"readall",					kCmd_Telescope_readall,					kCmdType_GET	},

	{	"",						-1,	0x00	}
};


//*****************************************************************************
TYPE_CmdEntry	gTelescopeExtrasTable[]	=
{
	{	"HourAngle-degrees",	kCmd_Telescope_HourAngleDegrees,		kCmdType_GET	},
	{	"",						-1,								0						}
};


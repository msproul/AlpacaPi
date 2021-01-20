//*****************************************************************************
//*	Mar  6,	2020	<MLS> Created alpaca_defs.h
//*	Sep  8,	2020	<MLS> Added TYPE_ASCOM_STATUS to ASCOM error return codes
//*	Dec  6,	2020	<MLS> Version V0.3.8-beta
//*	Dec 10,	2020	<MLS> Version V0.3.9-beta
//*****************************************************************************
//*	Jan  1,	2019	-----------------------------------------------------------
//*	Jan  1,	2020	-----------------------------------------------------------
//*	Jan  1,	2021	-----------------------------------------------------------
//*****************************************************************************
//#include	"alpaca_defs.h"



#ifndef	_ALPACA_DEFS_H_
#define	_ALPACA_DEFS_H_

#define	kApplicationName	"AlpacaPi"
#define	kVersionString		"V0.3.9-beta"
#define	kBuildNumber		82


#define kAlpacaDiscoveryPORT	32227

#if defined(__ARM_ARCH) && !defined(__arm__)
	#define __arm__
#endif

//*****************************************************************************
//*	defined by the ascom standarads
//*	https://ascom-standards.org/api/
//*****************************************************************************
typedef enum
{
	kALPACA_CameraState_Idle		=	0,
	kALPACA_CameraState_Waiting		=	1,
	kALPACA_CameraState_Exposing	=	2,
	kALPACA_CameraState_Reading		=	3,
	kALPACA_CameraState_Download	=	4,
	kALPACA_CameraState_Error		=	5

} TYPE_ALPACA_CAMERASTATE;

//*****************************************************************************
//*	Shutter Status
//*	defined by the ascom standarads
//*	https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__shutterstatus
//*****************************************************************************
enum
{
	kShutterStatus_Unknown	=	-1,		//*	not part of the alpaca standard

	kShutterStatus_Open		=	0,
	kShutterStatus_Closed	=	1,
	kShutterStatus_Opening	=	2,
	kShutterStatus_Closing	=	3,
	kShutterStatus_Error	=	4

};


//*****************************************************************************
enum TYPE_ASCOM_STATUS
{
	kASCOM_Err_Success					=	0,
	kASCOM_Err_NotImplemented			=	0x400,
	kASCOM_Err_PropertyNotImplemented	=	0x400,		//*	use this one
	kASCOM_Err_MethodNotImplemented		=	0x400,		//*	or this one
	kASCOM_Err_InvalidValue				=	0x401,
	kASCOM_Err_ValueNotSet				=	0x402,
	kASCOM_Err_NotConnected				=	0x407,
	kASCOM_Err_InvalidWhileParked		=	0x408,
	kASCOM_Err_InvalidWhileSlaved		=	0x409,
	kASCOM_Err_InvalidOperation			=	0x40B,
	kASCOM_Err_ActionNotImplemented		=	0x40C,

	//*	0x0500 to 0x0fff are driver defined.
	kASCOM_Err_NotSupported				=	0x500,
	kASCOM_Err_FailedToTakePicture		=	0x501,
	kASCOM_Err_CameraDriverErr			=	0x502,
	kASCOM_Err_CameraBusy				=	0x503,
	kASCOM_Err_DataFailure				=	0x504,
	kASCOM_Err_Unknown					=	0x505,
	kASCOM_Err_RequestFormatErr			=	0x506,
	kASCOM_Err_InternalError			=	0x507,


	kASCOM_Err_FailedUnknown,


	kASCOM_Err_last

};


//*****************************************************************************
enum CalibratorStatus
{

	kCalibrator_NotPresent	=	0,	//*	This device does not have a calibration capability
	kCalibrator_Off			=	1,	//*	The calibrator is off
	kCalibrator_NotReady	=	2,	//*	The calibrator is stabilising or is not yet in the commanded state
	kCalibrator_Ready		=	3,	//*	The calibrator is ready for use
	kCalibrator_Unknown		=	4,	//*	The calibrator state is unknown
	kCalibrator_Error		=	5	//*	The calibrator encountered an error when changing state

};


//*****************************************************************************
enum CoverStatus
{

	kCover_NotPresent	=	0,	//*	This device does not have a cover that can be closed independently
	kCover_Closed		=	1,	//*	The cover is closed
	kCover_Moving		=	2,	//*	The cover is moving to a new position
	kCover_Open			=	3,	//*	The cover is open
	kCover_Unknown		=	4,	//*	The state of the cover is unknown
	kCover_Error		=	5	//*	The device encountered an error when changing state
};

//*****************************************************************************
enum TYPE_AlignmentModes
{

	kAlignmentMode_algAltAz			=	0,	//*	Altitude-Azimuth alignment.
	kAlignmentMode_algPolar			=	1,	//*	Polar (equatorial) mount other than German equatorial.
	kAlignmentMode_algGermanPolar	=	2,	//*	German equatorial mount.
};

//*****************************************************************************
enum TYPE_EquatorialCoordinateType
{

	kECT_equOther				=	0,	//*	Custom or unknown equinox and/or reference frame.
	kECT_equTopocentric			=	1,	//*	Topocentric coordinates. Coordinates of the object at the current date having allowed for annual aberration, precession and nutation. This is the most common coordinate type for amateur telescopes.
	kECT_equJ2000				=	2,	//*	J2000 equator/equinox. Coordinates of the object at mid-day on 1st January 2000, ICRS reference frame.
	kECT_equJ2050				=	3,	//*	J2050 equator/equinox, ICRS reference frame.
	kECT_equB1950				=	4,	//*	B1950 equinox, FK4 reference frame.
};

//*****************************************************************************
enum TYPE_PierSide
{
	kPierSide_pierEast		=	0,	//*	Normal pointing state - Mount on the East side of pier (looking West)
	kPierSide_pierUnknown	=	-1,	//*	Unknown or indeterminate.
	kPierSide_pierWest		=	1,	//*	Through the pole pointing state - Mount on the West side of pier (looking East)
};

//*****************************************************************************
enum TYPE_DriveRates
{
	kDriveRate_driveSidereal	=	0,	//*	Sidereal tracking rate (15.041 arcseconds per second).
	kDriveRate_driveLunar		=	1,	//*	Lunar tracking rate (14.685 arcseconds per second).
	kDriveRate_driveSolar		=	2,	//*	Solar tracking rate (15.0 arcseconds per second).
	kDriveRate_driveKing		=	3,	//*	King tracking rate (15.0369 arcseconds per second).
};


//*****************************************************************************
enum TYPE_SensorType
{
	kSensorType_Monochrome	=	0,	//*	Camera produces monochrome array with no Bayer encoding
	kSensorType_Color		=	1,	//*	Camera produces color image directly, requiring not Bayer decoding
	kSensorType_RGGB		=	2,	//*	Camera produces RGGB encoded Bayer array images
	kSensorType_CMYG		=	3,	//*	Camera produces CMYG encoded Bayer array images
	kSensorType_CMYG2		=	4,	//*	Camera produces CMYG2 encoded Bayer array images
	kSensorType_LRGB		=	5,	//*	Camera produces Kodak TRUESENSE Bayer LRGB array images
};


#endif // _ALPACA_DEFS_H_





//*****************************************************************************
//*	Mar  6,	2020	<MLS> Created alpaca_defs.h
//*	Sep  8,	2020	<MLS> Added TYPE_ASCOM_STATUS to ASCOM error return codes
//*	Dec  6,	2020	<MLS> Version V0.3.8-beta
//*	Dec 10,	2020	<MLS> Version V0.3.9-beta
//*	Jan 24,	2021	<MLS> Major improvement in ASCOM property definition
//*	Jan 24,	2021	<MLS> Added TYPE_TelescopeProperties structure
//*	Jan 24,	2021	<MLS> Added TYPE_DomeProperties structure
//*	Jan 24,	2021	<MLS> Added TYPE_FocuserProperties structure
//*	Jan 24,	2021	<MLS> Version V0.4.0-beta
//*	Jan 25,	2021	<MLS> Added TYPE_CameraProperties structure
//*	Jan 25,	2021	<MLS> Added TYPE_RotatorProperties structure
//*	Jan 30,	2021	<MLS> Added TYPE_ImageArray structure
//*****************************************************************************
//*	These are for my comment extraction program that sorts comments by date.
//*	Jan  1,	2019	-----------------------------------------------------------
//*	Jan  1,	2020	-----------------------------------------------------------
//*	Jan  1,	2021	-----------------------------------------------------------
//*****************************************************************************
//#include	"alpaca_defs.h"



#ifndef	_ALPACA_DEFS_H_
#define	_ALPACA_DEFS_H_

#ifndef _SYS_TIME_H
	#include	<sys/time.h>
#endif

#ifndef	_TIME_H
	#include	<time.h>
#endif

#ifndef _STDINT_H
	#include	<stdint.h>
#endif

#define	kApplicationName	"AlpacaPi"
#define	kVersionString		"V0.4.0-beta"
#define	kBuildNumber		90


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

#define	kMaxSensorNameLen		32
#define	kMaxReadOutModes		5

//**************************************************************************************
typedef struct
{
	char	mode[64];
} READOUTMODE;


//*****************************************************************************
typedef struct
{
	//*	ASCOM variables (properties)
	//*	the ones commented out with //+ need to be implemented.....
//+	short					BayerOffsetX;			//*	The X offset of the Bayer matrix.
//+	short					BayerOffsetY;			//*	The Y offset of the Bayer matrix.
	int						BinX;					//*	The binning factor for the X axis.
	int						BinY;					//*	The binning factor for the Y axis.
	int						CameraXsize;			//*	The width of the CCD camera chip.
	int						CameraYsize;			//*	The height of the CCD camera chip.
//+	TYPE_ALPACA_CAMERASTATE	CameraState;			//*	the camera operational state.
	int						CanAbortExposure;		//*	Indicates whether the camera can abort exposures.
	bool					CanAsymmetricBin;		//*	Indicates whether the camera supports asymmetric binning
	bool					CanFastReadout;			//*	Indicates whether the camera has a fast readout mode.
	bool					CanGetCoolerPower;		//*	Indicates whether the camera's cooler power setting can be read.
	bool					CanPulseGuide;			//*	Indicates whether this camera supports pulse guiding
	bool					Cansetccdtemperature;	//*	Indicates whether this camera supports setting the CCD temperature
	bool					CanStopExposure;		//*	Returns a flag indicating whether this camera can stop an exposure that is in progress
	double					CCDtemperature;			//*	Returns the current CCD temperature
	bool					CoolerOn;				//*	Returns the current cooler on/off state.
//+	double					CoolerPower;			//*	Returns the present cooler power level
	double					ElectronsPerADU;		//*	Returns the gain of the camera
	double					ExposureMax_seconds;	//*	Returns the maximum exposure time supported by StartExposure.
	long					ExposureMax_us;			//*	micro-seconds
	double					ExposureMin_seconds;	//*	Returns the Minimium exposure time
	long					ExposureMin_us;			//*	micro-seconds
	double					ExposureResolution;		//*	The smallest increment in exposure time supported by StartExposure.

//+	bool					FastReadout;			//*	Returns whenther Fast Readout Mode is enabled.
//+	double					FullWellCapacity;		//*	Reports the full well capacity of the camera
	int						Gain;					//*	Returns the camera's gain
	int						GainMax;				//*	Maximum Gain value of that this camera supports
	int						GainMin;				//*	Minimum Gain value of that this camera supports
//+	???						Gains;					//*	List of Gain names supported by the camera
	bool					HasShutter;				//*	Indicates whether the camera has a mechanical shutter
//+	double					HeatSinkTemperature;	//*	Returns the current heat sink temperature.
	bool					ImageReady;				//*	Indicates that an image is ready to be downloaded
	bool					IsPulseGuiding;			//*	Indicates that the camera is pulse guideing.



	//==========================================================================================
	//*	information about the last exposure
	//*	we need to record a bunch of stuff in case they get changed before
	//*	the image gets downloaded
	uint32_t				Lastexposure_duration_us;	//*	stored in microseconds, ASCOM wants seconds, convert on the fly
														//*	Reported as a string
	struct timeval			Lastexposure_StartTime;		//*	time exposure or video was started for frame rate calculations
	struct timeval			Lastexposure_EndTime;		//*	NON-ALPACA----time last exposure ended
//+	TYPE_IMAGE_ROI_Info		LastExposure_ROIinfo;


//+	???						MaxADU;						//*	Camera's maximum ADU value
	int						MaxbinX;					//*	Maximum binning for the camera X axis
	int						MaxbinY;					//*	Maximum binning for the camera Y axis

	//===================================
	//*	subframe information
	int						NumX;						//*	The current subframe width
	int						NumY;						//*	The current subframe height


	int						StartX;					//*	The current subframe X axis start position
	int						StartY;					//*	The current subframe Y axis start position


//+	int						Offset;					//*	The camera's offset
//+	int						OffsetMax;				//*	Maximum offset value of that this camera supports
//+	int						OffsetMin;				//*	Minimum offset value of that this camera supports
//+	int						Offsets;				//*	List of offset names supported by the camera
//+	int						PercentCompleted;		//*	Indicates percentage completeness of the current operation
	double					PixelSizeX;				//*	the pixel size of the camera, unit is um. (microns) such as 5.6um
	double					PixelSizeY;				//*	the pixel size of the camera, unit is um. (microns) such as 5.6um


	double					SetCCDTemperature;		//*	The current camera cooler setpoint in degrees Celsius.

//	currently ReadoutMode is implemented at execution time
	int						ReadOutMode;			//*	Indicates the canera's readout mode as an index into the array ReadoutModes
//+	??						ReadOutModes;			//*	List of available readout modes
	READOUTMODE				ReadOutModes[kMaxReadOutModes];

	char					SensorName[kMaxSensorNameLen];	//	Sensor name
//+	TYPE_SensorType			SensorType;						//*	Type of information returned by the the camera sensor (monochrome or colour)

} TYPE_CameraProperties;

//*****************************************************************************
typedef struct
{
	//*	The rate of motion (deg/sec) about the specified axis
	double	Minimum;
	double	Maximum;
} TYPE_AxisRates;

//*****************************************************************************
//*	these are the exact properties from ASCOM Telescope V3
//*****************************************************************************
typedef struct
{
	TYPE_AlignmentModes				AlginmentMode;
	double							Altitude;
	double							ApertureArea;
	double							ApertureDiameter;
	bool							AtHome;
	bool							AtPark;
	TYPE_AxisRates					AxisRates[3];	//*	there are 3 possible axis
	double							Azimuth;
	bool							CanFindHome;
	bool							CanMoveAxis;
	bool							CanPark;
	bool							CanPulseGuide;
	bool							CanSetDeclinationRate;
	bool							CanSetGuideRates;
	bool							CanSetPark;
	bool							CanSetPierSide;
	bool							CanSetRightAscensionRate;
	bool							CanSetTracking;
	bool							CanSlew;
	bool							CanSlewAltAz;
	bool							CanSlewAltAzAsync;
	bool							CanSlewAsync;
	bool							CanSync;
	bool							CanSyncAltAz;
	bool							CanUnpark;
	bool							TargetDec_HasBeenSet;
	double							Declination;				//*	degrees
	double							DeclinationRate;
	bool							TargetRA_HasBeenSet;
	double							RightAscension;				//*	hours
	double							RightAscensionRate;
	bool							DoesRefraction;
	TYPE_EquatorialCoordinateType	EquatorialSystem;
	double							FocalLength;
	double							GuideRateDeclination;
	double							GuideRateRightAscension;
	bool							IsPulseGuiding;
	TYPE_PierSide					SideOfPier;
	double							SiderealTime;
	double							SiteElevation;
	double							SiteLatitude;
	double							SiteLongitude;
	bool							Slewing;
	short							SlewSettleTime;
	double							TargetDeclination;
	double							TargetRightAscension;
	bool							Tracking;
	TYPE_DriveRates					TrackingRate;
	double							TrackingRates;
	double							UTCDate;


} TYPE_TelescopeProperties;


//*****************************************************************************
typedef struct
{
	double			Altitude;			//*	Degrees;
	bool			AtHome;
	bool			AtPark;
	double			Azimuth;			//*	Degrees
	bool			CanFindHome;
	bool			CanPark;
	bool			CanSetAltitude;
	bool			CanSetAzimuth;
	bool			CanSetPark;
	bool			CanSetShutter;
	bool			CanSlave;
	bool			CanSyncAzimuth;
	int32_t			ShutterStatus;
	bool			Slaved;
	bool			Slewing;


} TYPE_DomeProperties;


//*****************************************************************************
typedef struct
{


	bool			Absolute;			//	Indicates whether the focuser is capable of absolute position.
	bool			IsMoving;			//	Indicates whether the focuser is currently moving.
	int32_t			MaxIncrement;		//	Returns the focuser's maximum increment size.
	int32_t			MaxStep;			//	Returns the focuser's maximum step size.
	int32_t			Position;			//	Returns the focuser's current position.
	double			StepSize;			//	Returns the focuser's step size.
	bool			TempComp;			//	Retrieves the state of temperature compensation mode
	bool			TempCompAvailable;	//	Indicates whether the focuser has temperature compensation.
	double			Temperature;		//	Returns the focuser's current temperature.

} TYPE_FocuserProperties;

//*****************************************************************************
typedef struct
{
	bool	CanReverse;			//*	Indicates whether the Rotator supports the Reverse method.
	bool	IsMoving;			//*	Indicates whether the rotator is currently moving
	double	MechanicalPosition;	//*	This returns the raw mechanical position of the rotator in degrees.
	double	Position;			//*	Current instantaneous Rotator position, allowing for any sync offset, in degrees.
	bool	Reverse;			//*	Sets or Returns the rotator’s Reverse state.
	double	StepSize;			//*	The minimum StepSize, in degrees.
	double	TargetPosition;		//*	The destination position angle for Move() and MoveAbsolute().
} TYPE_RotatorProperties;


#if 0
//*	not finished, havent started using this yet
#define	kMaxSwitchNameLen		32
#define	kMaxSwitchDescLen		64
//*****************************************************************************
typedef struct
{
	char	switchName[kMaxSwitchNameLen];
	char	switchDesciption[kMaxSwitchDescLen];

} TYPE_SwitchInfo;

//*****************************************************************************
typedef struct
{
	int						MaxSwitch;
	TYPE_SwitchDescription	SwitchTable[kMaxSwitchCnt];

} TYPE_SwitchProperties;

#endif


//*****************************************************************************
typedef struct
{
	int32_t		RedValue;
	int32_t		GrnValue;
	int32_t		BluValue;


} TYPE_ImageArray;


#endif // _ALPACA_DEFS_H_





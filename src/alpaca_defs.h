//*****************************************************************************
//*	These are for my comment extraction program that sorts comments by date.
//*	Jan  1,	2019	-----------------------------------------------------------
//*	Jan  1,	2020	-----------------------------------------------------------
//*	Mar  6,	2020	<MLS> Created alpaca_defs.h
//*	Sep  8,	2020	<MLS> Added TYPE_ASCOM_STATUS to ASCOM error return codes
//*	Dec  6,	2020	<MLS> Version V0.3.8-beta
//*	Dec 10,	2020	<MLS> Version V0.3.9-beta
//*	Jan  1,	2021	-----------------------------------------------------------
//*	Jan 24,	2021	<MLS> Major improvement in ASCOM property definition
//*	Jan 24,	2021	<MLS> Added TYPE_TelescopeProperties structure
//*	Jan 24,	2021	<MLS> Added TYPE_DomeProperties structure
//*	Jan 24,	2021	<MLS> Added TYPE_FocuserProperties structure
//*	Jan 24,	2021	<MLS> Version V0.4.0-beta
//*	Jan 25,	2021	<MLS> Added TYPE_CameraProperties structure
//*	Jan 25,	2021	<MLS> Added TYPE_RotatorProperties structure
//*	Jan 30,	2021	<MLS> Added TYPE_ImageArray structure
//*	Feb  7,	2021	<MLS> Added TYPE_CommonProperties
//*	Feb 12,	2021	<MLS> Added TYPE_CoverCalibrationProperties
//*	Mar  1,	2021	<MLS> Added TYPE_ObsConditionProperties
//*	Mar 21,	2021	<MLS> Added TYPE_FilterWheelProperties
//*	Sep 15,	2021	<MLS> Version V0.4.2-beta
//*	Oct 13,	2021	<MLS> Build 121
//*	Dec  6,	2021	<MLS> Build 122
//*	Dec 11,	2021	<MLS> Build 123
//*	Dec 14,	2021	<MLS> Started on binary image stuff, added header structure
//*	Dec 16,	2021	<MLS> Version V0.4.3-beta
//*	Dec 16,	2021	<MLS> Build 124
//*	Dec 20,	2021	<MLS> Build 125
//*	Jan  1,	2022	-----------------------------------------------------------
//*	Jan  1,	2022	<MLS> Version V0.4.4-beta
//*	Jan  1,	2022	<MLS> Build 126
//*	Jan 17,	2022	<MLS> Build 127
//*	Jan 28,	2022	<MLS> Build 128
//*	Feb 12,	2022	<MLS> Build 129
//*	Feb 18,	2022	<MLS> Version V0.5.0-beta
//*	Feb 18,	2022	<MLS> Build 130
//*	Mar 11,	2022	<MLS> Build 131
//*	Mar 22,	2022	<MLS> Build 132
//*	Mar 25,	2022	<MLS> Build 134
//*	Mar 25,	2022	<MLS> Build 135 - starting on QSI
//*	Apr 13,	2022	<MLS> Build 136
//*	Apr 17,	2022	<MLS> Build 137
//*	May  2,	2022	<MLS> Build 138
//*	May  6,	2022	<MLS> Build 139
//*	May 11,	2022	<MLS> Build 140
//*	May 15,	2022	<MLS> Build 141
//*	May 17,	2022	<MLS> Build 142
//*	May 19,	2022	<MLS> Build 143
//*	May 23,	2022	<MLS> Changed syntax of enum typedefs to make RNS compiler happy
//*	May 19,	2022	<MLS> Build 144
//*	May 30,	2022	<MLS> Build 145
//*	Jun  7,	2022	<MLS> Version V0.5.1-beta
//*	Jun  7,	2022	<MLS> Build 146
//*	Jul  6,	2022	<MLS> Build 147
//*	Aug 15,	2022	<MLS> Build 148
//*	Aug 30,	2022	<MLS> Build 149
//*	Sep 11,	2022	<MLS> Build 150
//*	Sep 26,	2022	<MLS> Added structure TYPE_Sensor for ObservingConditions
//*	Sep 29,	2022	<MLS> Build 151
//*	Oct  5,	2022	<MLS> Build 152
//*	Oct 16,	2022	<MLS> Build 153
//*	Oct 21,	2022	<MLS> Build 154
//*	Nov  5,	2022	<MLS> Build 155
//*	Nov 27,	2022	<MLS> Build 156
//*	Nov 30,	2022	<MLS> Build 157
//*	Dec 18,	2022	<MLS> Build 158
//*	Jan  1,	2023	-----------------------------------------------------------
//*	Feb 20,	2023	<MLS> Build 159
//*	Feb 27,	2023	<MLS> Build 160
//*	Mar  2,	2023	<MLS> Build 161
//*	Mar  7,	2023	<MLS> Build 162
//*	Mar 30,	2023	<MLS> Build 163
//*	May  9,	2023	<MLS> Changed TYPE_Sensor to TYPE_InstSensor to avoid confusion
//*	May 26,	2023	<MLS> Build 164
//*	May 26,	2023	<MLS> Version V0.7.0
//*	May 27,	2023	<MLS> Added SavedImageCnt to Camera Properties
//*	Jun  9,	2023	<MLS> Build 165
//*	Jun 18,	2023	<MLS> Added Connecting property to common properties
//*	Jun 18,	2023	<MLS> Added CalibratorChanging & CoverMoving to calibrator properties
//*	Jun 18,	2023	<MLS> Version V0.7.1
//*	Jun 18,	2023	<MLS> Build 166
//*	Jul  2,	2023	<MLS> Build 167
//*	Jul 16,	2023	<MLS> Build 168
//*	Jul 18,	2023	<MLS> Build 169
//*	Sep  7,	2023	<MLS> Build 170
//*	Sep 25,	2023	<MLS> Build 171	working on TOPENS ROR driver
//*	Sep 28,	2023	<MLS> Build 172
//*	Nov  1,	2023	<MLS> Pushed Build 172
//*	Nov  1,	2023	<MLS> Build 173
//*	Jan  1,	2024	-----------------------------------------------------------
//*	Feb 10,	2024	<MLS> Version V0.7.2
//*	Feb 10,	2024	<MLS> Build 174
//*	Feb 10,	2024	<MLS> Pushed Build 174
//*	Mar 22,	2024	<MLS> Build 175
//*	Apr 15,	2024	<MLS> Build 176
//*	Apr 29,	2024	<MLS> Build 177
//*	May 15,	2024	<MLS> Added TYPE_GuideDirections enums
//*	May 26,	2024	<MLS> Build 178
//*	Jun 16,	2024	<MLS> Build 179 - pushed to github
//*	Jun 25,	2024	<MLS> Build 180 - pushed to github
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


#define	kApplicationName		"AlpacaPi"
#define	kVersionString			"V0.7.2"
#define	kBuildNumber			180
#define kAlpacaDiscoveryPORT	32227
#define kAlpacaPiDefaultPORT	6800


#if defined(__ARM_ARCH) && !defined(__arm__)
	#define __arm__
#endif


#ifdef _USE_OPENCV_
	#include	<opencv2/opencv.hpp>
	#include	<opencv2/core.hpp>

	#ifndef _USE_OPENCV_CPP_
		#include	<opencv2/highgui/highgui_c.h>
	#endif // _USE_OPENCV_CPP_

	#if (CV_MAJOR_VERSION <= 3)
		#define _ENABLE_CVFONT_
//		#warning (CV_MAJOR_VERSION <= 3)
	#endif
	#if (CV_MAJOR_VERSION == 4)
//		#warning (CV_MAJOR_VERSION == 4)
	#endif
#endif // _USE_OPENCV_

//*****************************************************************************
//*	enum for the various driver types
//*	ordered alphabetically, no particular order required
//*****************************************************************************
typedef enum	//	TYPE_DEVICETYPE
{
	kDeviceType_undefined	=	-1,
	kDeviceType_Camera,
	kDeviceType_CoverCalibrator,
	kDeviceType_Dome,
	kDeviceType_Filterwheel,
	kDeviceType_Focuser,
	kDeviceType_Management,
	kDeviceType_Observingconditions,
	kDeviceType_Rotator,
	kDeviceType_SafetyMonitor,
	kDeviceType_Switch,
	kDeviceType_Telescope,

	//*	extras defined by MLS
	kDeviceType_Multicam,
	kDeviceType_Shutter,
	kDeviceType_SlitTracker,
	kDeviceType_Spectrograph,

	//*	these are window types, not actually device types
	kDeviceType_SkyTravel,
	kDeviceType_Startup,

	kDeviceType_last

} TYPE_DEVICETYPE;


//*****************************************************************************
//*	defined by the ascom standards
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
//*	defined by the ascom standards
//*	https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__shutterstatus
//*****************************************************************************
typedef enum
{
	kShutterStatus_Unknown	=	-1,		//*	not part of the alpaca standard

	kShutterStatus_Open		=	0,
	kShutterStatus_Closed	=	1,
	kShutterStatus_Opening	=	2,
	kShutterStatus_Closing	=	3,
	kShutterStatus_Error	=	4

} TYPE_ShutterStatus;


//*****************************************************************************
typedef enum
{
	kASCOM_Err_Success					=	0,
	kASCOM_Err_NotImplemented			=	0x400,		//	1024
	kASCOM_Err_PropertyNotImplemented	=	0x400,		//*	use this one
	kASCOM_Err_MethodNotImplemented		=	0x400,		//*	or this one
	kASCOM_Err_InvalidValue				=	0x401,		//	1025
	kASCOM_Err_ValueNotSet				=	0x402,		//	1026
	kASCOM_Err_NotConnected				=	0x407,		//	1031
	kASCOM_Err_InvalidWhileParked		=	0x408,		//	1032
	kASCOM_Err_InvalidWhileSlaved		=	0x409,		//	1033
	kASCOM_Err_SettingsProviderError	=	0x40A,		//	1034
	kASCOM_Err_InvalidOperation			=	0x40B,		//	1035
	kASCOM_Err_ActionNotImplemented		=	0x40C,
	kASCOM_Err_NotInCacheException		=	0x40D,
	kASCOM_Err_UnspecifiedError			=	0x4FF,		//	1279

	//*	0x0500 to 0x0fff are driver defined.
	kASCOM_Err_NotSupported				=	0x500,
	kASCOM_Err_FailedToTakePicture		=	0x501,
	kASCOM_Err_CameraDriverErr			=	0x502,
	kASCOM_Err_CameraBusy				=	0x503,
	kASCOM_Err_DataFailure				=	0x504,
	kASCOM_Err_Unknown					=	0x505,
	kASCOM_Err_InternalError			=	0x506,


	kASCOM_Err_FailedUnknown,

	kASCOM_Err_last

} TYPE_ASCOM_STATUS;

//*****************************************************************************
typedef enum
{
	kCalibrator_NotPresent	=	0,	//*	This device does not have a calibration capability
	kCalibrator_Off			=	1,	//*	The calibrator is off
	kCalibrator_NotReady	=	2,	//*	The calibrator is stabilizing or is not yet in the commanded state
	kCalibrator_Ready		=	3,	//*	The calibrator is ready for use
	kCalibrator_Unknown		=	4,	//*	The calibrator state is unknown
	kCalibrator_Error		=	5	//*	The calibrator encountered an error when changing state
} CalibratorStatus;


//*****************************************************************************
typedef enum
{
	kCover_NotPresent	=	0,	//*	This device does not have a cover that can be closed independently
	kCover_Closed		=	1,	//*	The cover is closed
	kCover_Moving		=	2,	//*	The cover is moving to a new position
	kCover_Open			=	3,	//*	The cover is open
	kCover_Unknown		=	4,	//*	The state of the cover is unknown
	kCover_Error		=	5	//*	The device encountered an error when changing state
} CoverStatus;

//*****************************************************************************
//*	https://ascom-standards.org/Help/Developer/html/T_ASCOM_DeviceInterface_AlignmentModes.htm
//*****************************************************************************
typedef enum
{
	kAlignmentMode_algAltAz			=	0,	//*	Altitude-Azimuth alignment.
	kAlignmentMode_algPolar			=	1,	//*	Polar (equatorial) mount other than German equatorial.
	kAlignmentMode_algGermanPolar	=	2,	//*	German equatorial mount.
} TYPE_AlignmentModes;

//*****************************************************************************
typedef enum
{
	kECT_equOther				=	0,	//*	Custom or unknown equinox and/or reference frame.
	kECT_equTopocentric			=	1,	//*	Topocentric coordinates.
										//*	Coordinates of the object at the current date having allowed for annual aberration, precession and nutation.
										//*	This is the most common coordinate type for amateur telescopes.
	kECT_equJ2000				=	2,	//*	J2000 equator/equinox. Coordinates of the object at mid-day on 1st January 2000, ICRS reference frame.
	kECT_equJ2050				=	3,	//*	J2050 equator/equinox, ICRS reference frame.
	kECT_equB1950				=	4	//*	B1950 equinox, FK4 reference frame.
} TYPE_EquatorialCoordinateType;

//*****************************************************************************
typedef enum
{
	kPierSide_NotAvailable	=	-2,	//*	NOT PART OF ASCOM
	kPierSide_pierUnknown	=	-1,	//*	Unknown or indeterminate.
	kPierSide_pierEast		=	0,	//*	Normal pointing state - Mount on the East side of pier (looking West)
	kPierSide_pierWest		=	1	//*	Through the pole pointing state - Mount on the West side of pier (looking East)
} TYPE_PierSide;

//*****************************************************************************
//*	these are ASCOM defined values
//*	https://ascom-standards.org/Help/Platform/html/T_ASCOM_DeviceInterface_DriveRates.htm
//*****************************************************************************
typedef enum
{
	kDriveRate_driveSidereal	=	0,	//*	Sidereal tracking rate (15.041 arcseconds per second).
	kDriveRate_driveLunar		=	1,	//*	Lunar tracking rate (14.685 arcseconds per second).
	kDriveRate_driveSolar		=	2,	//*	Solar tracking rate (15.0 arcseconds per second).
	kDriveRate_driveKing		=	3,	//*	King tracking rate (15.0369 arcseconds per second).
	kDriveRate_Count
} TYPE_DriveRates;

//*****************************************************************************
//*	https://ascom-standards.org/Help/Platform/html/T_ASCOM_DeviceInterface_SensorType.htm
//*****************************************************************************
typedef enum
{
	kSensorType_Monochrome	=	0,	//*	Camera produces monochrome array with no Bayer encoding
	kSensorType_Color		=	1,	//*	Camera produces color image directly, requiring not Bayer decoding
	kSensorType_RGGB		=	2,	//*	Camera produces RGGB encoded Bayer array images
	kSensorType_CMYG		=	3,	//*	Camera produces CMYG encoded Bayer array images
	kSensorType_CMYG2		=	4,	//*	Camera produces CMYG2 encoded Bayer array images
	kSensorType_LRGB		=	5	//*	Camera produces Kodak TRUESENSE Bayer LRGB array images
} TYPE_SensorType;



#define	kMaxSensorNameLen		32
#define	kMaxReadOutModes		5


#define	kImgTypeStrMaxLen	64
//**************************************************************************************
typedef struct	//	READOUTMODE
{
	bool	valid;
	int		internalImageType;		//*	this is my INTERNAL image mode, having nothing to do with Alpaca
									//*	it uses TYPE_IMAGE_TYPE enums (see cameradriver.h)
	char	modeStr[kImgTypeStrMaxLen];
} READOUTMODE;


//*****************************************************************************
#define	kCommonPropMaxStrLen	256
typedef struct	//	TYPE_CommonProperties
{
	//*	ASCOM common properties
	bool	Connected;
	bool	Connecting;
	char	Description[kCommonPropMaxStrLen];
	char	DriverInfo[kCommonPropMaxStrLen];
	char	DriverVersion[kCommonPropMaxStrLen];
	int		InterfaceVersion;
	char	Name[kCommonPropMaxStrLen];

} TYPE_CommonProperties;



//*****************************************************************************
//*	these are the exact properties from ASCOM Camera V3
//*****************************************************************************
typedef struct	//	TYPE_CameraProperties
{
	//*	ASCOM variables (properties)
	//*	the ones commented out with //+ need to be implemented.....
	int						BayerOffsetX;			//*	The X offset of the Bayer matrix.
	int						BayerOffsetY;			//*	The Y offset of the Bayer matrix.
	int						BinX;					//*	The binning factor for the X axis.
	int						BinY;					//*	The binning factor for the Y axis.
	int						CameraXsize;			//*	The width of the CCD camera chip.
	int						CameraYsize;			//*	The height of the CCD camera chip.
	TYPE_ALPACA_CAMERASTATE	CameraState;			//*	the camera operational state.
	bool					CanAbortExposure;		//*	Indicates whether the camera can abort exposures.
	bool					CanAsymmetricBin;		//*	Indicates whether the camera supports asymmetric binning
	bool					CanFastReadout;			//*	Indicates whether the camera has a fast readout mode.
	bool					CanGetCoolerPower;		//*	Indicates whether the camera's cooler power setting can be read.
	bool					CanPulseGuide;			//*	Indicates whether this camera supports pulse guiding
	bool					CanSetCCDtemperature;	//*	Indicates whether this camera supports setting the CCD temperature
	bool					CanStopExposure;		//*	Returns a flag indicating whether this camera can stop an exposure that is in progress
	double					CCDtemperature;			//*	Returns the current CCD temperature
	bool					CoolerOn;				//*	Returns the current cooler on/off state.
	double					CoolerPower;			//*	Returns the present cooler power level
	double					ElectronsPerADU;		//*	Returns the gain of the camera
	double					ExposureMax_seconds;	//*	Returns the maximum exposure time supported by StartExposure.
	long					ExposureMax_us;			//*	micro-seconds
	double					ExposureMin_seconds;	//*	Returns the Minimium exposure time
	long					ExposureMin_us;			//*	micro-seconds
	double					ExposureResolution;		//*	The smallest increment in exposure time supported by StartExposure.

	bool					FastReadout;			//*	Returns whether Fast Readout Mode is enabled.
	double					FullWellCapacity;		//*	Reports the full well capacity of the camera
	int						Gain;					//*	Returns the camera's gain
	int						GainMax;				//*	Maximum Gain value of that this camera supports
	int						GainMin;				//*	Minimum Gain value of that this camera supports
//+	???						Gains;					//*	List of Gain names supported by the camera
	bool					HasShutter;				//*	Indicates whether the camera has a mechanical shutter
	double					HeatSinkTemperature;	//*	Returns the current heat sink temperature.
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


	int						MaxADU;					//*	Camera's maximum ADU value
	int						MaxbinX;				//*	Maximum binning for the camera X axis
	int						MaxbinY;				//*	Maximum binning for the camera Y axis

	//===================================
	//*	sub-frame information
	int						NumX;					//*	The current subframe width
	int						NumY;					//*	The current subframe height

	int						Offset;					//*	The camera's offset
	int						OffsetMax;				//*	Maximum offset value of that this camera supports
	int						OffsetMin;				//*	Minimum offset value of that this camera supports
//?	int						Offsets;				//*	List of offset names supported by the camera
	int						PercentCompleted;		//*	Indicates percentage completeness of the current operation
	double					PixelSizeX;				//*	the pixel size of the camera, unit is um. (microns) such as 5.6um
	double					PixelSizeY;				//*	the pixel size of the camera, unit is um. (microns) such as 5.6um

	//*	currently ReadoutMode is implemented at execution time
	int						ReadOutMode;					//*	Indicates the camera's readout mode as an index into the array ReadoutModes
	READOUTMODE				ReadOutModes[kMaxReadOutModes];	//*	List of available readout modes

	double					SetCCDTemperature;		//*	The current camera cooler setpoint in degrees Celsius.

	char					SensorName[kMaxSensorNameLen];	//	Sensor name
	TYPE_SensorType			SensorType;						//*	Type of information returned by the the camera sensor (monochrome or colour)

	int						StartX;					//*	The current subframe X axis start position
	int						StartY;					//*	The current subframe Y axis start position
	double					SubExposureDuration;	//*	Camera's sub-exposure interval

	//=======================================================================
	//=======================================================================
	//*	Extra stuff, not defined by Alpaca standard
	int						FlipMode;
	int						SavedImageCnt;					//*	replaces cNumFramesSaved
	char					FPGAversion[48];
	char					ProductionDate[48];


} TYPE_CameraProperties;



//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*	Filter wheel properties
#define	kMaxFWnameLen	32
//*****************************************************************************
typedef struct	//	TYPE_FilterName
{
	char	FilterName[kMaxFWnameLen];
} TYPE_FilterName;

#define	kMaxFiltersPerWheel	10

//*****************************************************************************
typedef struct	//	TYPE_FilterWheelProperties
{
	int				FocusOffsets[kMaxFiltersPerWheel];	//	Focus offset of each filter in the wheel
	TYPE_FilterName	Names[kMaxFiltersPerWheel];			//	Name of each filter in the wheel
	int				Position;							//	Sets or returns the current filter wheel position

	//*	this is NOT a standard ASCOM/ALPACA property
	bool			IsMoving;
} TYPE_FilterWheelProperties;

//*****************************************************************************
typedef enum
{
	kAxis_RA		=	0,	//*	Primary axis (e.g., Right Ascension or Azimuth).
	kAxis_DEC		=	1,	//*	Secondary axis (e.g., Declination or Altitude).
	kAxis_Tertiary	=	2	//*	Tertiary axis (e.g. image rotator/de-rotator).
} TYPE_Axis;


//*****************************************************************************
typedef struct	//	TYPE_AxisRates
{
	//*	The rate of motion (deg/sec) about the specified axis
	double	Minimum;
	double	Maximum;
} TYPE_AxisRates;

//*****************************************************************************
typedef struct	//	TYPE_CoverCalibrationProperties
{
	int					Brightness;
	CalibratorStatus	CalibratorState;
	CoverStatus			CoverState;
	int					MaxBrightness;
	//*	new properties as of June 18, 2023
	//*	for the new DeviceState
	bool				CalibratorReady;
	bool				CoverMoving;

	//*	extras by MLS
	double				Aperture;		//*	percentage value
	bool				CanSetAperture;

} 	TYPE_CoverCalibrationProperties;


//*****************************************************************************
typedef enum	//	TYPE_GuideDirections
{
	kGuide_North	=	0,	//*	North (+ declination/altitude).
	kGuide_South	=	1,	//*	South (- declination/altitude).
	kGuide_East		=	2,	//*	East (+ right ascension/azimuth).
	kGuide_West		=	3,	//*	West (- right ascension/azimuth)
	kGuide_last
} TYPE_GuideDirections;


//*****************************************************************************
//*	these are the exact properties from ASCOM Telescope V3
//*****************************************************************************
typedef struct	//	TYPE_TelescopeProperties
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
	bool							CanMoveAxis[3];
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
	double							Declination;				//*	degrees
	double							DeclinationRate;
	TYPE_PierSide					DestinationSideOfPier;
	bool							driverSupportsRefraction;	//*	NON-alpaca
	bool							DoesRefraction;
	double							RightAscension;				//*	hours
	double							RightAscensionRate;
	TYPE_EquatorialCoordinateType	EquatorialSystem;
	double							FocalLength;
	double							GuideRateDeclination;
	double							GuideRateRightAscension;
	bool							IsPulseGuiding;
	TYPE_PierSide					SideOfPier;
	double							SiderealTime;
	double							SiteElevation;				//*	meters
	double							SiteLatitude;				//*	degrees
	double							SiteLongitude;				//*	degrees
	bool							Slewing;
	short							SlewSettleTime;
	double							TargetDeclination;
	double							TargetRightAscension;
	bool							TargetDec_HasBeenSet;	//*	non-Alpaca
	bool							TargetRA_HasBeenSet;	//*	non-Alpaca
	bool							Tracking;
	TYPE_DriveRates					TrackingRate;
//	double							TrackingRates;
////	double							UTCDate;	//*	this is the real time date so we dont keep it around


	//*	extras NOT defined by ASCOM
	bool							hourAngleIsValid;
	double							HourAngle;			//*	Hour angle 0 -> 23.99999
	double							HourAngle_deg;
	TYPE_PierSide					PhysicalSideOfPier;
	double							IMU_Roll;
	double							IMU_Pitch;
	double							IMU_Yaw;
} TYPE_TelescopeProperties;

void	DumpTelescopeDriverStruct(TYPE_TelescopeProperties *telescopeDriver);

//*****************************************************************************
//*	Dome properties as described by ASCOM
//*****************************************************************************
typedef struct	//	TYPE_DomeProperties
{
	double				Altitude;			//*	Degrees;
	bool				AtHome;
	bool				AtPark;
	double				Azimuth;			//*	Degrees
	bool				CanFindHome;
	bool				CanPark;
	bool				CanSetAltitude;
	bool				CanSetAzimuth;
	bool				CanSetPark;
	bool				CanSetShutter;
	bool				CanSlave;
	bool				CanSyncAzimuth;
	TYPE_ShutterStatus	ShutterStatus;
	bool				Slaved;
	bool				Slewing;
} TYPE_DomeProperties;


//*****************************************************************************
//*	Focuser properties
//*****************************************************************************
typedef struct	//	TYPE_FocuserProperties
{
	bool			Absolute;			//	Indicates whether the focuser is capable of absolute position.
	bool			IsMoving;			//	Indicates whether the focuser is currently moving.
	int32_t			MaxIncrement;		//	Returns the focuser's maximum increment size.
	int32_t			MaxStep;			//	Returns the focuser's maximum step value.
	int32_t			Position;			//	Returns the focuser's current position.
	double			StepSize;			//	Returns the focuser's step size.
	bool			TempComp;			//	Retrieves the state of temperature compensation mode
	bool			TempCompAvailable;	//	Indicates whether the focuser has temperature compensation.
	double			Temperature_DegC;	//	Returns the focuser's current temperature. (deg C)

} TYPE_FocuserProperties;

//*****************************************************************************
//*	Rotator properties
//*****************************************************************************
typedef struct	//	TYPE_RotatorProperties
{
	bool	CanReverse;			//*	Indicates whether the Rotator supports the Reverse method.
	bool	IsMoving;			//*	Indicates whether the rotator is currently moving
	double	MechanicalPosition;	//*	This returns the raw mechanical position of the rotator in degrees.
	double	Position;			//*	Current instantaneous Rotator position, allowing for any sync offset, in degrees.
	bool	Reverse;			//*	Sets or Returns the rotator’s Reverse state.
	double	StepSize;			//*	The minimum StepSize, in degrees.
	double	SyncOffset;			//*	Once the Sync method has been called and the sync offset determined,
								//*	both the MoveAbsolute(Single) method and the Position property must function in synced coordinates rather than mechanical coordinates. The sync offset must persist across driver starts and device reboots.

	double	TargetPosition;		//*	The destination position angle for Move() and MoveAbsolute(). (in degrees)
} TYPE_RotatorProperties;

//*****************************************************************************
//*	Instrument Sensor
//*	this is for observing conditions and spectrograph
//*****************************************************************************
typedef struct	//	TYPE_InstSensor
{
	bool		IsSupported;
	bool		ValidData;
	double		Value;
	int			ValueInteger;
	char		Description[128];
	char		Info[128];
	uint32_t	LastRead;
} TYPE_InstSensor;

//*****************************************************************************
typedef struct	//	TYPE_ObsConditionProperties
{
	TYPE_InstSensor		Averageperiod;		//*	Returns the time period over which observations will be averaged
	TYPE_InstSensor		CloudCover;			//*	Returns the amount of sky obscured by cloud
	TYPE_InstSensor		DewPoint;			//*	Returns the atmospheric dew point at the observatory
	TYPE_InstSensor		Humidity;			//*	Returns the atmospheric humidity at the observatory
	TYPE_InstSensor		Pressure;			//*	Returns the atmospheric pressure at the observatory. hectoPascals
	TYPE_InstSensor		RainRate;			//*	Returns the rain rate at the observatory.
	TYPE_InstSensor		SkyBrightness;		//*	Returns the sky brightness at the observatory
	TYPE_InstSensor		SkyQuality;			//*	Returns the sky quality at the observatory
	TYPE_InstSensor		SkyTemperature;		//*	Returns the sky temperature at the observatory
	TYPE_InstSensor		StarFWHM;			//*	Returns the seeing at the observatory
	TYPE_InstSensor		Temperature;		//*	Returns the temperature at the observatory
	TYPE_InstSensor		WindDirection;		//*	Returns the wind direction at the observatory
	TYPE_InstSensor		WindGust;			//*	Returns the peak 3 second wind gust at the observatory over the last 2 minutes
	TYPE_InstSensor		WindSpeed;			//*	Returns the wind speed at the observatory.

//	double	sensordescription,		//*	Return a sensor description
//	double	timesincelastupdate,	//*	Return the time since the sensor value was last updated

} TYPE_ObsConditionProperties;



//*****************************************************************************
//*	not finished
#define	kMaxSwitchCnt			16
#define	kMaxSwitchNameLen		32
#define	kMaxSwitchDescLen		128

////*****************************************************************************
//typedef struct	//	TYPE_SwitchInfo
//{
//	char	switchName[kMaxSwitchNameLen];
//	char	switchDescription[kMaxSwitchDescLen];
//
//} TYPE_SwitchInfo;
//**************************************************************************************
typedef struct
{
	char	switchName[kMaxSwitchNameLen];
	char	switchDescription[kMaxSwitchDescLen];
	bool	canWrite;
	bool	switchState;
	double	minswitchvalue;
	double	maxswitchvalue;
	double	switchvalue;

} TYPE_SwitchInfo;
#define	kMaxSwitches	32

//*****************************************************************************
typedef struct	//	TYPE_SwitchProperties
{
	int					MaxSwitch;
	TYPE_SwitchInfo		SwitchTable[kMaxSwitchCnt];

} TYPE_SwitchProperties;

//*****************************************************************************
typedef struct	//	TYPE_ImageArray
{
	int32_t		RedValue;
	int32_t		GrnValue;
	int32_t		BluValue;
} TYPE_ImageArray;


#define	kMaxAlpacaDeviceCnt	128


//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*	Proposed binary image structure
//	Version 0.3 5-December 2021
//*****************************************************************************
//*****************************************************************************
typedef struct	//	TYPE_BinaryImageHdr
{
	//Metadata Structure
	//The following structure describes the returned data
	int32_t		MetadataVersion;			// Bytes 0..3 - Metadata version = 1
	int32_t		ErrorNumber;				// Bytes 4..7 - Alpaca error number or zero for success
	uint32_t	ClientTransactionID;		// Bytes 8..11 - Client's transaction ID
	uint32_t	ServerTransactionID;		// Bytes 12..15 - Device's transaction ID
	int32_t		DataStart;					// Bytes 16..19 - Offset of the start of the data bytes = 36 for version 1
	int32_t		ImageElementType;			// Bytes 20..23 - Element type of the source image array
	int32_t		TransmissionElementType;	// Bytes 24..27 - Element type as sent over the network
	int32_t		Rank;						// Bytes 28..31 - Image array rank
	int32_t		Dimension1;					// Bytes 32..35 - Length of image array first dimension
	int32_t		Dimension2;					// Bytes 36..39 - Length of image array second dimension
	int32_t		Dimension3;					// Bytes 40..43 - Length of image array third dimension (0 for 2D array)

} TYPE_BinaryImageHdr;

//*****************************************************************************
enum
{
	kAlpacaImageData_Unknown	=	0, // 0 to 3 are the values already used in the Alpaca standard
	kAlpacaImageData_Int16		=	1,
	kAlpacaImageData_Int32		=	2,
	kAlpacaImageData_Double		=	3,
	kAlpacaImageData_Single		=	4,	// 4 to 9 are an extension to include other numeric types
	kAlpacaImageData_Decimal	=	5,
	kAlpacaImageData_Byte		=	6,
	kAlpacaImageData_Int64		=	7,
	kAlpacaImageData_UInt16		=	8
};


//*****************************************************************************
//*	slit tracker properties, (NOT defined by ASCOM)
//*****************************************************************************
typedef struct	//	TYPE_SlittrackerProperties
{
	char	DomeAddress[48];
	bool	TrackingEnabled;
} TYPE_SlittrackerProperties;


//*****************************************************************************
//*	this is for the temperature log used in both drivers and controllers
//*	extra, not part of ASCOM/Alpaca specs
#define	kTemperatureLogEntries	(24 * 60)




#endif // _ALPACA_DEFS_H_





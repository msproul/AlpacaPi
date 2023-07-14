//*****************************************************************************
//*	command table for Alpaca covercalib
//*		This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created covercalib_AlpacaCmds.cpp
//*****************************************************************************

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _COVERCALIB_ALPACA_CMDS_H_
	#include	"covercalib_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gCalibrationCmdTable[]	=
{


	{	"brightness",			kCmd_Calibration_brightness,		kCmdType_GET	},	//*	Returns the current calibrator brightness
	{	"calibratorstate",		kCmd_Calibration_calibratorstate,	kCmdType_GET	},	//*	Returns the state of the calibration device
	{	"coverstate",			kCmd_Calibration_coverstate,		kCmdType_GET	},	//*	Returns the state of the device cover"
	{	"calibratoroff",		kCmd_Calibration_calibratoroff,		kCmdType_PUT	},	//*	Turns the calibrator off
	{	"calibratoron",			kCmd_Calibration_calibratoron,		kCmdType_PUT	},	//*	Turns the calibrator on at the specified brightness
	{	"closecover",			kCmd_Calibration_closecover,		kCmdType_PUT	},	//*	Initiates cover closing
	{	"haltcover",			kCmd_Calibration_haltcover,			kCmdType_PUT	},	//*	Stops any cover movement that may be in progress
	{	"maxbrightness",		kCmd_Calibration_maxbrightness,		kCmdType_GET	},	//*	Returns the calibrator's maximum Brightness value.
	{	"opencover",			kCmd_Calibration_opencover,			kCmdType_PUT	},	//*	Initiates cover opening

	{	"CalibratorReady",		kCmd_Calibration_CalibratorReady,	kCmdType_GET	},	//*	Initiates cover opening
	{	"CoverMoving",			kCmd_Calibration_CoverMoving,		kCmdType_GET	},	//*	Initiates cover opening

	//*	added by MLS
	{	"--extras",			kCmd_Calibration_Extras,			kCmdType_GET	},
	{	"aperture",			kCmd_Calibration_aperture,			kCmdType_BOTH	},
	{	"canadjustaperture",kCmd_Calibration_canadjustaperture,	kCmdType_GET	},
	{	"readall",			kCmd_Calibration_readall,			kCmdType_GET	},

	{	"",						-1,	0x00	}
};

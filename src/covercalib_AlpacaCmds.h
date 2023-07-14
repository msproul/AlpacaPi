//*****************************************************************************
//*	Jun 28,	2023	<MLS> Created covercalib_AlpacaCmds.h
//*****************************************************************************
//#include	"covercalib_AlpacaCmds.h"

#ifndef _COVERCALIB_ALPACA_CMDS_H_
#define _COVERCALIB_ALPACA_CMDS_H_

extern TYPE_CmdEntry	gCalibrationCmdTable[];

//*****************************************************************************
//	cover calibration
//*****************************************************************************
enum
{
	kCmd_Calibration_brightness,			//*	Returns the current calibrator brightness
	kCmd_Calibration_calibratorstate,		//*	Returns the state of the calibration device
	kCmd_Calibration_coverstate,			//*	Returns the state of the device cover"
	kCmd_Calibration_maxbrightness,			//*	Returns the calibrator's maximum Brightness value.
	kCmd_Calibration_calibratoroff,			//*	Turns the calibrator off
	kCmd_Calibration_calibratoron,			//*	Turns the calibrator on at the specified brightness
	kCmd_Calibration_closecover,			//*	Initiates cover closing
	kCmd_Calibration_haltcover,				//*	Stops any cover movement that may be in progress
	kCmd_Calibration_opencover,				//*	Initiates cover opening

	kCmd_Calibration_CalibratorReady,
	kCmd_Calibration_CoverMoving,

	//*	added by MLS
	kCmd_Calibration_Extras,
	kCmd_Calibration_aperture,				//*	GET/PUT aperture opening in percentage
	kCmd_Calibration_canadjustaperture,		//*	true if adjustable aperture is available
	kCmd_Calibration_readall,

	kCmd_Calibration_last
};

#endif // _COVERCALIB_ALPACA_CMDS_H_


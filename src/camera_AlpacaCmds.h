//*****************************************************************************
//*	camera_AlpacaCmds.h
//*****************************************************************************
//*	Jun 30,	2023	<MLS> Created camera_AlpacaCmds.h
//*****************************************************************************
//#include	"camera_AlpacaCmds.h"

#ifndef _CAMERA_ALPACA_CMDS_H_
#define	_CAMERA_ALPACA_CMDS_H_

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

extern	TYPE_CmdEntry	gCameraCmdTable[];
extern	TYPE_CmdEntry	gCameraExtrasTable[];


//*****************************************************************************
//*	Camera commands
enum
{
	kCmd_Camera_bayeroffsetX	=	0,	//*	Returns the X offset of the Bayer matrix.
	kCmd_Camera_bayeroffsetY,			//*	Returns the Y offset of the Bayer matrix.
	kCmd_Camera_binX,					//*	Returns the binning factor for the X axis.
										//*	Sets the binning factor for the X axis.
	kCmd_Camera_binY,					//*	Returns the binning factor for the Y axis.
										//*	Sets the binning factor for the Y axis.
	kCmd_Camera_camerastate,			//*	Returns the camera operational state.
	kCmd_Camera_cameraxsize,			//*	Returns the width of the CCD camera chip.
	kCmd_Camera_cameraysize,			//*	Returns the height of the CCD camera chip.
	kCmd_Camera_canabortexposure,		//*	Indicates whether the camera can abort exposures.
	kCmd_Camera_canasymmetricbin,		//*	Indicates whether the camera supports asymmetric binning
	kCmd_Camera_canfastreadout,			//*	Indicates whether the camera has a fast readout mode.
	kCmd_Camera_cangetcoolerpower,		//*	Indicates whether the camera's cooler power setting can be read.
	kCmd_Camera_canpulseguide,			//*	Returns a flag indicating whether this camera supports pulse guiding
	kCmd_Camera_cansetccdtemperature,	//*	Returns a flag indicating whether this camera supports setting the CCD temperature
	kCmd_Camera_canstopexposure,		//*	Returns a flag indicating whether this camera can stop an exposure that is in progress
	kCmd_Camera_ccdtemperature,			//*	Returns the current CCD temperature
	kCmd_Camera_cooleron,				//*	Returns the current cooler on/off state.
										//*	Turns the camera cooler on and off
	kCmd_Camera_CoolerPower,			//*	Returns the present cooler power level
	kCmd_Camera_electronsperadu,		//*	Returns the gain of the camera
	kCmd_Camera_exposuremax,			//*	Returns the maximum exposure time supported by StartExposure.
	kCmd_Camera_exposuremin,			//*	Returns the Minimium exposure time
	kCmd_Camera_exposureresolution,		//*	Returns the smallest increment in exposure time supported by StartExposure.
	kCmd_Camera_fastreadout,			//*	Returns whether Fast Readout Mode is enabled.
										//*	Sets whether Fast Readout Mode is enabled.
	kCmd_Camera_FullWellCapacity,		//*	Reports the full well capacity of the camera
	kCmd_Camera_gain,					//*	Returns the camera's gain
										//*	Sets the camera's gain.
	kCmd_Camera_gainmax,				//*	Maximum value of Gain
	kCmd_Camera_gainmin,				//*	Minimum value of Gain
	kCmd_Camera_gains,					//*	Gains supported by the camera
	kCmd_Camera_hasshutter,				//*	Indicates whether the camera has a mechanical shutter
	kCmd_Camera_heatsinktemperature,	//*	Returns the current heat sink temperature.
	kCmd_Camera_imagearray,				//*	Returns an array of integers containing the exposure pixel values
	kCmd_Camera_imagearrayvariant,		//*	Returns an array of int containing the exposure pixel values
	kCmd_Camera_imageready,				//*	Indicates that an image is ready to be downloaded
	kCmd_Camera_IsPulseGuiding,			//*	Indicates that the camera is pulse guideing.
	kCmd_Camera_lastexposureduration,	//*	Duration of the last exposure
	kCmd_Camera_lastexposurestarttime,	//*	Start time of the last exposure in FITS standard format.
	kCmd_Camera_maxadu,					//*	Camera's maximum ADU value
	kCmd_Camera_maxbinX,				//*	Maximum binning for the camera X axis
	kCmd_Camera_maxbinY,				//*	Maximum binning for the camera Y axis
	kCmd_Camera_numX,					//*	Returns the current subframe width
										//*	Sets the current subframe width
	kCmd_Camera_numY,					//*	Returns the current subframe height
										//*	Sets the current subframe height
	kCmd_Camera_offset,					//*	Returns the camera's offset
										//*	Sets the camera's offset.
	kCmd_Camera_offsetmax,				//*	Returns the maximum value of offset.
	kCmd_Camera_offsetmin,				//*	Returns the Minimum value of offset.
	kCmd_Camera_offsets,				//*	Returns List of offset names supported by the camera
	kCmd_Camera_percentcompleted,		//*	Indicates percentage completeness of the current operation
	kCmd_Camera_PixelSizeX,				//*	Width of CCD chip pixels (microns)
	kCmd_Camera_PixelSizeY,				//*	Height of CCD chip pixels (microns)
	kCmd_Camera_readoutmode,			//*	Indicates the canera's readout mode as an index into the array ReadoutModes
										//*	Set the camera's readout mode
	kCmd_Camera_readoutmodes,			//*	List of available readout modes
	kCmd_Camera_sensorname,				//*	Sensor name
	kCmd_Camera_sensortype,				//*	Type of information returned by the the camera sensor (monochrome or colour)
	kCmd_Camera_setccdtemperature,		//*	Returns the current camera cooler setpoint in degrees Celsius.
										//*	Set the camera's cooler setpoint (degrees Celsius).
	kCmd_Camera_startX,					//*	Return the current subframe X axis start position
										//*	Sets the current subframe X axis start position
	kCmd_Camera_startY,					//*	Return the current subframe Y axis start position
										//*	Sets the current subframe Y axis start position
	kCmd_Camera_abortexposure,			//*	Aborts the current exposure
	kCmd_Camera_pulseguide,				//*	Pulse guide in the specified direction for the specified time.
	kCmd_Camera_startexposure,			//*	Starts an exposure
	kCmd_Camera_stopexposure,			//*	Stops the current exposure
	kCmd_Camera_subexposureduration,	//*	Camera's sub-exposure interval

	//=================================================================
	//*	commands added that are not part of Alpaca
	kCmd_Camera_Extras,

	//*	commands borrowed from the telescope device
	kCmd_Camera_ApertureArea,			//*	Returns the telescope's aperture.
	kCmd_Camera_ApertureDiameter,		//*	Returns the telescope's effective aperture.
	kCmd_Camera_FocalLength,			//*	Returns the telescope's focal length in meters.


	kCmd_Camera_autoexposure,
	kCmd_Camera_displayimage,

	kCmd_Camera_ExposureTime,
	kCmd_Camera_filelist,
	kCmd_Camera_filenameoptions,
	kCmd_Camera_fitsheader,
	kCmd_Camera_flip,
	kCmd_Camera_framerate,
	kCmd_Camera_livemode,
	kCmd_Camera_rgbarray,
	kCmd_Camera_settelescopeinfo,
	kCmd_Camera_saveallimages,
	kCmd_Camera_saveasFITS,
	kCmd_Camera_saveasJPEG,
	kCmd_Camera_saveasPNG,
	kCmd_Camera_saveasRAW,
	kCmd_Camera_savedimages,
	kCmd_Camera_savenextimage,
	kCmd_Camera_startsequence,
	kCmd_Camera_startvideo,
	kCmd_Camera_stopvideo,

	//*	keep this one last for consistency with other drivers
	kCmd_Camera_readall,
	kCmd_Camera_last

};

//*****************************************************************************
enum
{
	kCmd_Camera_BackGroundColor	=	(kCmd_Camera_last + 1),
	kCmd_Camera_FileNamePrefix,
	kCmd_Camera_FileNameSuffix,
	kCmd_Camera_FilenameRoot,
	kCmd_Camera_FilenameIncludeFilter,
	kCmd_Camera_FilenameIncludeCamera,
	kCmd_Camera_FilenameIncludeSerialnum,
	kCmd_Camera_FilenameIncludeRefid,
	kCmd_Camera_FreeDisk_Gigabytes,
	kCmd_Camera_REFID,

};

#endif // _CAMERA_ALPACA_CMDS_H_

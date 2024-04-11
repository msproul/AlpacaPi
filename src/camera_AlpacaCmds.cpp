//*****************************************************************************
//*	command table for Alpaca camera
//*		This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created camera_AlpacaCmds.cpp
//*****************************************************************************


#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _CAMERA_ALPACA_CMDS_H_
	#include	"camera_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gCameraCmdTable[]	=
{
	{	"bayeroffsetx",				kCmd_Camera_bayeroffsetX,			kCmdType_GET	},
	{	"bayeroffsety",				kCmd_Camera_bayeroffsetY,			kCmdType_GET	},
	{	"binx",						kCmd_Camera_binX,					kCmdType_BOTH	},
	{	"biny",						kCmd_Camera_binY,					kCmdType_BOTH	},
	{	"camerastate",				kCmd_Camera_camerastate,			kCmdType_GET	},
	{	"cameraxsize",				kCmd_Camera_cameraxsize,			kCmdType_GET	},
	{	"cameraysize",				kCmd_Camera_cameraysize,			kCmdType_GET	},
	{	"canabortexposure",			kCmd_Camera_canabortexposure,		kCmdType_GET	},
	{	"canasymmetricbin",			kCmd_Camera_canasymmetricbin,		kCmdType_GET	},
	{	"canfastreadout",			kCmd_Camera_canfastreadout,			kCmdType_GET	},
	{	"cangetcoolerpower",		kCmd_Camera_cangetcoolerpower,		kCmdType_GET	},
	{	"canpulseguide",			kCmd_Camera_canpulseguide,			kCmdType_GET	},
	{	"cansetccdtemperature",		kCmd_Camera_cansetccdtemperature,	kCmdType_GET	},
	{	"canstopexposure",			kCmd_Camera_canstopexposure,		kCmdType_GET	},
	{	"ccdtemperature",			kCmd_Camera_ccdtemperature,			kCmdType_GET	},
	{	"cooleron",					kCmd_Camera_cooleron,				kCmdType_BOTH	},
	{	"coolerpower",				kCmd_Camera_CoolerPower,			kCmdType_GET	},
	{	"electronsperadu",			kCmd_Camera_electronsperadu,		kCmdType_GET	},
	{	"exposuremax",				kCmd_Camera_exposuremax,			kCmdType_GET	},
	{	"exposuremin",				kCmd_Camera_exposuremin,			kCmdType_GET	},
	{	"exposureresolution",		kCmd_Camera_exposureresolution,		kCmdType_GET	},
	{	"fastreadout",				kCmd_Camera_fastreadout,			kCmdType_BOTH	},
	{	"fullwellcapacity",			kCmd_Camera_FullWellCapacity,		kCmdType_GET	},
	{	"gain",						kCmd_Camera_gain,					kCmdType_BOTH	},
	{	"gainmax",					kCmd_Camera_gainmax,				kCmdType_GET	},
	{	"gainmin",					kCmd_Camera_gainmin,				kCmdType_GET	},
	{	"gains",					kCmd_Camera_gains,					kCmdType_GET	},
	{	"hasshutter",				kCmd_Camera_hasshutter,				kCmdType_GET	},
	{	"heatsinktemperature",		kCmd_Camera_heatsinktemperature,	kCmdType_GET	},
	{	"imagearray",				kCmd_Camera_imagearray,				kCmdType_GET	},
	{	"imagearrayvariant",		kCmd_Camera_imagearrayvariant,		kCmdType_GET	},
	{	"imageready",				kCmd_Camera_imageready,				kCmdType_GET	},
	{	"ispulseguiding",			kCmd_Camera_IsPulseGuiding,			kCmdType_GET	},
	{	"lastexposureduration",		kCmd_Camera_lastexposureduration,	kCmdType_GET	},
	{	"lastexposurestarttime",	kCmd_Camera_lastexposurestarttime,	kCmdType_GET	},
	{	"maxadu",					kCmd_Camera_maxadu,					kCmdType_GET	},
	{	"maxbinx",					kCmd_Camera_maxbinX,				kCmdType_GET	},
	{	"maxbiny",					kCmd_Camera_maxbinY,				kCmdType_GET	},
	{	"numx",						kCmd_Camera_numX,					kCmdType_BOTH	},
	{	"numy",						kCmd_Camera_numY,					kCmdType_BOTH	},
	{	"offset",					kCmd_Camera_offset,					kCmdType_BOTH	},
	{	"offsetmax",				kCmd_Camera_offsetmax,				kCmdType_GET	},
	{	"offsetmin",				kCmd_Camera_offsetmin,				kCmdType_GET	},
	{	"offsets",					kCmd_Camera_offsets,				kCmdType_GET	},
	{	"percentcompleted",			kCmd_Camera_percentcompleted,		kCmdType_GET	},
	{	"pixelsizex",				kCmd_Camera_PixelSizeX,				kCmdType_GET	},
	{	"pixelsizey",				kCmd_Camera_PixelSizeY,				kCmdType_GET	},
	{	"readoutmode",				kCmd_Camera_readoutmode,			kCmdType_BOTH	},
	{	"readoutmodes",				kCmd_Camera_readoutmodes,			kCmdType_GET	},
	{	"sensorname",				kCmd_Camera_sensorname,				kCmdType_GET	},
	{	"sensortype",				kCmd_Camera_sensortype,				kCmdType_GET	},
	{	"setccdtemperature",		kCmd_Camera_setccdtemperature,		kCmdType_BOTH	},
	{	"startx",					kCmd_Camera_startX,					kCmdType_BOTH	},
	{	"starty",					kCmd_Camera_startY,					kCmdType_BOTH	},
	{	"abortexposure",			kCmd_Camera_abortexposure,			kCmdType_PUT	},
	{	"pulseguide",				kCmd_Camera_pulseguide,				kCmdType_PUT	},
	{	"startexposure",			kCmd_Camera_startexposure,			kCmdType_PUT	},
	{	"stopexposure",				kCmd_Camera_stopexposure,			kCmdType_PUT	},
	{	"subexposureduration",		kCmd_Camera_subexposureduration,	kCmdType_BOTH	},

//#ifdef _INCLUDE_ALPACA_EXTRAS_
	//*	items added by MLS
	{	"--extras",					kCmd_Camera_Extras,					kCmdType_GET	},


	{	"autoexposure",				kCmd_Camera_autoexposure,			kCmdType_BOTH	},
	{	"displayimage",				kCmd_Camera_displayimage,			kCmdType_BOTH	},
	{	"exposuretime",				kCmd_Camera_ExposureTime,			kCmdType_BOTH	},
#ifdef _ENABLE_FITS_
	{	"fitsheader",				kCmd_Camera_fitsheader,				kCmdType_GET	},
#endif
	{	"filelist",					kCmd_Camera_filelist,				kCmdType_GET	},
	{	"filenameoptions",			kCmd_Camera_filenameoptions,		kCmdType_PUT	},
	{	"flip",						kCmd_Camera_flip,					kCmdType_BOTH	},
	{	"framerate",				kCmd_Camera_framerate,				kCmdType_GET	},
	{	"livemode",					kCmd_Camera_livemode,				kCmdType_BOTH	},
	{	"rgbarray",					kCmd_Camera_rgbarray,				kCmdType_GET	},
	{	"saveallimages",			kCmd_Camera_saveallimages,			kCmdType_BOTH	},

	{	"saveasfits",				kCmd_Camera_saveasFITS,				kCmdType_BOTH	},
	{	"saveasjpeg",				kCmd_Camera_saveasJPEG,				kCmdType_BOTH	},
	{	"saveaspng",				kCmd_Camera_saveasPNG,				kCmdType_BOTH	},
	{	"saveasraw",				kCmd_Camera_saveasRAW,				kCmdType_BOTH	},

	{	"savedimages",				kCmd_Camera_savedimages,			kCmdType_GET	},
	{	"savenextimage",			kCmd_Camera_savenextimage,			kCmdType_PUT	},
	{	"settelescopeinfo",			kCmd_Camera_settelescopeinfo,		kCmdType_PUT	},
	{	"startsequence",			kCmd_Camera_startsequence,			kCmdType_PUT	},
	{	"startvideo",				kCmd_Camera_startvideo,				kCmdType_PUT	},
	{	"stopvideo",				kCmd_Camera_stopvideo,				kCmdType_PUT	},

	{	"readall",					kCmd_Camera_readall,				kCmdType_GET	},
//#endif // _INCLUDE_ALPACA_EXTRAS_

	{	"",							-1,									0x00			}
};

//		NOT HANDLED: SENSORTYPESTR, Monochrome
//		NOT HANDLED: EXPOSURESTATE, Idle
//		NOT HANDLED: STEPSIZE, 5
//		NOT HANDLED: FRAMES-READ, 0
//		NOT HANDLED: FOCUSERINFOVALID, true
//		NOT HANDLED: ROTATORINFOVALID, false
//		NOT HANDLED: FILTERWHEELINFOVALID, false
//		NOT HANDLED: OBJECT, unknown
//		NOT HANDLED: VIDEOFRAMES, 0
//		NOT HANDLED: APERTUREAREA, 0.000326851300
//		NOT HANDLED: APERTUREDIAMETER, 0.020400000000
//		NOT HANDLED: FOCALLENGTH, 0.100000000000
//		NOT HANDLED: IMAGE-MODE, Single
//		NOT HANDLED: INTERNALCAMERASTATE, Idle
//		NOT HANDLED: ERRORLOGGING, false
//		NOT HANDLED: CONFORMLOGGING, false
//		NOT HANDLED: PLATFORM, Raspberry Pi 4 Model B Rev 1.2 (64 bit)
//		NOT HANDLED: CPUINFO, Arm8-V8 w/NEON
//		NOT HANDLED: OPERATINGSYSTEM, Debian GNU/Linux 11 (bullseye)
//		NOT HANDLED: BOGOMIPS, 108.000000000000
//		NOT HANDLED: CPUTEMP_DEGC, 55.991000000000
//		NOT HANDLED: CPUTEMP_DEGF, 132.783800000000
//		NOT HANDLED: UPTIME_SECS, 841989
//		NOT HANDLED: UPTIME_DAYS, 9 days 17:53:09
//		NOT HANDLED: TOTALRAM_MEGABYTES, 3795
//		NOT HANDLED: FREERAM_MEGABYTES, 2024
//		NOT HANDLED: USBFS_MEMORY_MB, 200
//		NOT HANDLED: CRUSAGE.RU_UTIME.TV_SEC, 475
//		NOT HANDLED: CRUSAGE.RU_STIME.TV_SEC, 1794
//		NOT HANDLED: PERCENTCPU, 0
//		notHandledCnt	= 30

//*****************************************************************************
//*	extra stuff in read all that are not actual commands
//*****************************************************************************
TYPE_CmdEntry	gCameraExtrasTable[]	=
{

	{	"backGroundColor",				kCmd_Camera_BackGroundColor,			kCmdType_GET	},
	{	"fileNamePrefix",				kCmd_Camera_FileNamePrefix,				kCmdType_GET	},
	{	"fileNameSuffix",				kCmd_Camera_FileNameSuffix,				kCmdType_GET	},
	{	"filenameRoot",					kCmd_Camera_FilenameRoot,				kCmdType_GET	},
	{	"filename_IncludeFILTER",		kCmd_Camera_FilenameIncludeFilter,		kCmdType_GET	},
	{	"filename_IncludeCAMERA",		kCmd_Camera_FilenameIncludeCamera,		kCmdType_GET	},
	{	"filename_IncludeSERIALNUM",	kCmd_Camera_FilenameIncludeSerialnum,	kCmdType_GET	},
	{	"filename_IncludeREFID",		kCmd_Camera_FilenameIncludeRefid,		kCmdType_GET	},
	{	"freeDisk_Gigabytes",			kCmd_Camera_FreeDisk_Gigabytes,			kCmdType_GET	},
	{	"REFID",						kCmd_Camera_REFID,						kCmdType_GET	},
//	{	"foo",				foo,			kCmdType_GET	},

	{	"",							-1,									0x00			}
};

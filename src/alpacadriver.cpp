//**************************************************************************
//*	Name:			alpacadriver.cpp
//*
//*	Author:			Mark Sproul (C) 2019, 2020
//*					msproul@skychariot.com
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
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*
//*		https://ascom-standards.org/Help/Developer/html/P_ASCOM_DriverAccess_Camera_ReadoutModes.htm
//*
//*		https://www.astro.louisville.edu/software/xmccd/archive/xmccd-4.1/xmccd-4.1e/docs/cfitsio/cfitsio.pdf
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul msproul@skychariot.com
//*****************************************************************************
//*	Apr  5,	2019	<MLS> Attended lecture by Bob Denny introducing Alpaca protocol
//*	Apr  9,	2019	<MLS> Created alpaca_driver.c
//*	Apr 12,	2019	<MLS> Parsing of the get/put command working
//*	Apr 12,	2019	<MLS> Added _ENABLE_CAMERA_
//*	Apr 12,	2019	<MLS> Added _ENABLE_DOME_
//*	Apr 12,	2019	<MLS> Added _ENABLE_FILTERWHEEL_
//*	Apr 12,	2019	<MLS> Added _ENABLE_FOCUSER_
//*	Apr 12,	2019	<MLS> Added _ENABLE_ROTATOR_
//*	Apr 12,	2019	<MLS> Added _ENABLE_TELESCOPE_
//*	Apr 12,	2019	<MLS> Started on Dome control
//*	Apr 12,	2019	<MLS> Added TYPE_CmdEntry structure
//*	Apr 14,	2019	<MLS> Added TYPE_DeviceTable structure
//*	Apr 14,	2019	<MLS> Added Common commands table
//*	Apr 14,	2019	<MLS> JSON response working
//*	Apr 15,	2019	<MLS> Added SocketWriteData()
//*	Apr 15,	2019	<MLS> Added SendSupportedActions()
//*	Apr 16,	2019	<MLS> Breaking up AlpacaCallback() to reduce complexity
//*	Apr 16,	2019	<MLS> Added ProcessGetPutRequest()
//*	Apr 18,	2019	<MLS> Downloaded ZWO Filter Wheel library
//*	May  1,	2019	<MLS> Switched to buffered Json output
//*	May  6,	2019	<MLS> Added threads
//*	May  6,	2019	<MLS> Added _ENABLE_OBSERVINGCONDITIONS_
//*	May  6,	2019	<MLS> Added _ENABLE_SAFETYMONITOR_
//*	May  7,	2019	<MLS> Added observingconditions parsing
//*	May 10,	2019	<MLS> Added GetTimeString()
//*	May 11,	2019	<MLS> Added _ENABLE_SWITCH_
//*	May 14,	2019	<MLS> Added GenerateHTMLcmdLinkTable()
//*	May 21,	2019	<MLS> Added logging
//*	May 22,	2019	<MLS> Added logging HTML output
//*	Aug 30,	2019	<MLS> Started on alpaca driver base class
//*	Oct  4,	2019	<MLS> cDeviceNum is now set correctly on class creation
//*	Oct  9,	2019	<MLS> Added ProcessCmdLineArgs()
//*	Oct 20,	2019	<MLS> Added processing for "setup"
//*	Oct 20,	2019	<MLS> Added processing for "management"
//*	Oct 20,	2019	<MLS> Added discovery protocol via another thread
//*	Nov  8,	2019	<MLS> Added GetKeyWordArgument()
//*	Nov  8,	2019	<MLS> Added millis()
//*	Nov 20,	2019	<MLS> Added CountDevicesByType()
//*	Nov 25,	2019	<MLS> Updated web output to support .png files
//*	Nov 27,	2019	<MLS> Added ProcessManagementRequest()
//*	Nov 27,	2019	<MLS> Added ToLowerStr() & StripTrailingSpaces()
//*	Dec  1,	2019	<MLS> Management commands are no longer logged
//*	Dec  6,	2019	<MLS> Spent the day at Naval Academy working with Jeff Larsen
//*	Dec 20,	2019	<MLS> Added version info to web output
//*	Dec 29,	2019	<MLS> Added -t cmd line option to specify the default telescope config
//*	Jan  1,	2020	<MLS> Added ability to set title for web page,
//*	Jan  1,	2020	<MLS> Moved OutputHTMLrowData() to base class
//*	Jan  3,	2020	<MLS> Added InitObsConditionGloblas()
//*	Jan 14,	2020	<MLS> Added Check_udev_rulesFile()
//*	Jan 18,	2020	<MLS> Added Read_OSreleaseVersion()
//*	Jan 19,	2020	<MLS> Added Read_CpuInfo()
//*	Jan 26,	2020	<MLS> Added _ENABLE_DISCOVERY_QUERRY_
//*	Feb  3,	2020	<MLS> Added gPlatformString
//*	Feb 10,	2020	<MLS> Added ExtractArgValue()
//*	Feb 18,	2020	<MLS> Added -q (quiet) -v (verbose default) options to command line
//*	Feb 22,	2020	<MLS> Changed command logging to only when there are errors
//*	Mar 27,	2020	<MLS> Added _ENABLE_QHY_
//*	Mar 28,	2020	<MLS> Removed device number from constructor, it is determined internally
//*	Mar 29,	2020	<MLS> Changed my alpaca port to 6800
//*	Mar 30,	2020	<MLS> Started working on passing Alpaca/ASCOM Conform testing
//*	Mar 31,	2020	<MLS> Added _DEBUG_CONFORM_
//*	Apr  2,	2020	<MLS> Rewrote ParseHTMLdataIntoReqStruct()
//*	Apr  3,	2020	<MLS> Added Command statistics RecordCmdStats()
//*	Apr  3,	2020	<MLS> Added OutputHTML_CmdStats()
//*	Apr  3,	2020	<MLS> Added GetCmdNameFromMyCmdTable() to all drivers
//*	Apr 20,	2020	<MLS> Added Get_Readall_Common()
//*	Apr 27,	2020	<MLS> Added Get_Readall_CPUstats()
//*	Apr 27,	2020	<MLS> Added CPU temp to readall output
//*	May  2,	2020	<MLS> Added Slit Tracker device
//*	May 19,	2020	<MLS> Added bogomips to cpustats
//*	Jun 15,	2020	<MLS> Added DumpRequestStructure()  (it got lost somewhere along the way)
//*	Jul  7,	2020	<MLS> Added compile flag _ENABLE_WIRING_PI_
//*	Jul 16,	2020	<MLS> Compiling and running on 64bit Raspberry Pi OS
//*	Jul 20,	2020	<MLS> Fixed bug in GetKeyWordArgument(), returned garbage if no argument
//*	Jul 20,	2020	<MLS> Added Put_Connected(), AlpacaConnect() and AlpacaDisConnect()
//*	Jul 20,	2020	<MLS> Added "exit" command for clean shutdown of sony camera
//*	Sep  1,	2020	<MLS> Re-organized the case statements in all of the ProcessCommand() functions
//*	Nov 30,	2020	<MLS> All (TYPE_ASCOM_STATUS) type casts have been removed
//*	Dec  3,	2020	<MLS> First release of source code to outside
//*	Dec 12,	2020	<MLS> Started github repository https://github.com/msproul/AlpacaPi
//*	Dec 28,	2020	<MLS> Finished making all Alpaca error messages uniform
//*	Jan 10,	2021	<MLS> Changed SendSupportedActions() to Get_SupportedActions()
//*	Jan 10,	2021	<MLS> Pushed build 74 up to github
//*	Mar 19,	2021	<MLS> Added coma checking in numeric string in GetKeyWordArgument()
//*	Apr  1,	2021	<MLS> LiveWindow mode will be available for all drivers
//*	Apr  1,	2021	<MLS> Added Put_LiveWindow()
//*	Apr  1,	2021	<MLS> Added UpdateLiveWindow()
//*	Apr  8,	2021	<MLS> Added GetMyHostName()
//*	Jun 19,	2021	<MLS> Added _ENABLE_LIVE_CONTROLLER_
//*	Aug 20,	2021	<MLS> Started testing with Peter's cross platform CONFORM
//*	Sep  2,	2021	<MLS> Started adding bandwidth statistics
//*	Sep  2,	2021	<MLS> Added BandWidthStatsInit()
//*	Dec 13,	2021	<MLS> Started working on watchdog time out processing
//*	Dec 13,	2021	<MLS> Added CheckWatchDogTimeout() & WatchDog_TimeOut()
//*	Dec 18,	2021	<MLS> Started using pmccabe to check routine complexity
//*	Dec 29,	2021	<MLS> Fixed argument buffer overflow bug in GetKeyWordArgument()
//*	Mar  2,	2022	<MLS> Updated Connected command
//*	Apr  7,	2022	<MLS> Added _ENABLE_QSI_
//*	Apr 13,	2022	<MLS> Added CreateDriverObjects()
//*	Apr 17,	2022	<MLS> Added run time option flag gSimulateCameraImage
//*	May  5,	2022	<MLS> Added cVerboseDebug so an individual driver can be more verbose
//*	Jul  3,	2022	<MLS> Added HTTP header for JPEG output (was removed for some reason)
//*****************************************************************************
//*	to install code blocks 20
//*	Step 1: sudo add-apt-repository ppa:codeblocks-devs/release
//*	Step 2: sudo apt-get update
//*	Step 3: sudo apt-get install codeblocks codeblocks-contrib
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	<sys/time.h>
#include	<sys/resource.h>
#include	<time.h>
#include	<gnu/libc-version.h>


#ifdef _ENABLE_FITS_
	#ifndef _FITSIO_H
		#include <fitsio.h>
	#endif // _FITSIO_H
#endif // _ENABLE_FITS_
#ifdef _ENABLE_JPEGLIB_
	#include	<jpeglib.h>
#endif


#include <pthread.h>

//#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"JsonResponse.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"eventlogging.h"
#include	"socket_listen.h"
#include	"discoverythread.h"
#include	"html_common.h"
#include	"observatory_settings.h"
#include	"obsconditions_globals.h"
#include	"cpu_stats.h"

//#define _DEBUG_CONFORM_


int			gAlpacaListenPort			=	kAlpacaPiDefaultPORT;
uint32_t	gClientID					=	0;
uint32_t	gClientTransactionID		=	0;
uint32_t	gServerTransactionID		=	0;		//*	we are the server, we will increment this each time a transaction occurs
bool		gErrorLogging				=	false;	//*	write errors to log file if true
bool		gConformLogging				=	false;	//*	log all commands to log file to match up with Conform
bool		gImageDownloadInProgress	=	false;
char		gHostName[48]				=	"";

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_ASI_)
	#include	"cameradriver_ASI.h"
#endif
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_ATIK_)
	#include	"cameradriver_ATIK.h"
#endif
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_TOUP_)
	#include	"cameradriver_TOUP.h"
#endif
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)
	#include	"cameradriver_QHY.h"
#endif
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QSI_)
	#include	"cameradriver_QSI.h"
#endif
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FLIR_)
	#include	"cameradriver_FLIR.h"
#endif
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_SONY_)
	#include	"cameradriver_SONY.h"
#endif
#if defined(_ENABLE_CAMERA_) && defined(_SIMULATE_CAMERA_)
	#include	"cameradriver_sim.h"
#endif


#ifdef	_ENABLE_DOME_
//	#include	"domedriver.h"
	#include	"domedriver_rpi.h"
#endif
#ifdef	_ENABLE_ROR_
	#include	"domedriver_ror_rpi.h"
#endif


#if defined(_ENABLE_FILTERWHEEL_) || defined(_ENABLE_FILTERWHEEL_ZWO_)
	#include	"filterwheeldriver.h"
	#include	"filterwheeldriver_ZWO.h"
#endif

#ifdef _ENABLE_FILTERWHEEL_ATIK_
	#include	"filterwheeldriver.h"
	#include	"filterwheeldriver_ATIK.h"
#endif

#ifdef _ENABLE_FOCUSER_
	#include	"focuserdriver.h"
	#include	"focuserdriver_nc.h"
#endif
#ifdef _ENABLE_MULTICAM_
	#include	"multicam.h"
#endif
#ifdef _ENABLE_SWITCH_
	#include	"switchdriver.h"
	#include	"switchdriver_rpi.h"
#endif
#ifdef _ENABLE_OBSERVINGCONDITIONS_
	#include	"obsconditionsdriver.h"
#endif
#if defined(_ENABLE_OBSERVINGCONDITIONS_) && defined(__arm__)
	#include	"obsconditionsdriver_rpi.h"
#endif

#ifdef _ENABLE_CALIBRATION_
	#include	"calibrationdriver.h"
	#include	"calibrationdriver_rpi.h"
#endif // _ENABLE_CALIBRATION_

#ifdef _ENABLE_TELESCOPE_
	#include	"telescopedriver.h"
	#include	"telescopedriver_lx200.h"
	#include	"telescopedriver_comm.h"
#endif


#ifdef _ENABLE_SHUTTER_
	#include	"shutterdriver_arduino.h"
#endif

#ifdef _ENABLE_SLIT_TRACKER_
	#include	"slittracker.h"
#endif

#ifdef _ENABLE_IMU_
	#include "imu_lib.h"
#endif

#if defined(__arm__) && defined(_ENABLE_WIRING_PI_)
	#include <wiringPi.h>
#endif


#include	"managementdriver.h"


AlpacaDriver		*gAlpacaDeviceList[kMaxDevices];
bool				gKeepRunning				=	true;
int					gDeviceCnt					=	0;
bool				gLiveView					=	false;
bool				gAutoExposure				=	false;
bool				gDisplayImage				=	false;
bool				gSimulateCameraImage		=	false;
bool				gVerbose					=	true;
bool				gDebugDiscovery				=	false;
bool				gObservatorySettingsOK		=	false;
const char			gValueString[]				=	"Value";
char				gDefaultTelescopeRefID[kDefaultRefIdMaxLen]	=	"";
char				gWebTitle[80]				=	"AlpacaPi";

char				gFullVersionString[128];

#ifdef _ENABLE_BANDWIDTH_LOGGING_
	int				gTimeUnitsSinceTopOfHour	=	0;
#endif // _ENABLE_BANDWIDTH_LOGGING_

#ifdef _ENABLE_LIVE_CONTROLLER_
	static void	HandleContollerWindow(AlpacaDriver *alpacaObjPtr);
#endif // _ENABLE_LIVE_CONTROLLER_

//*****************************************************************************
static void	InitDeviceList(void)
{
int		ii;

	for (ii=0; ii<kMaxDevices; ii++)
	{
		gAlpacaDeviceList[ii]	=	NULL;
	}
	gDeviceCnt	=	0;
}


//*****************************************************************************
const TYPE_CmdEntry	gCommonCmdTable[]	=
{

	{	"action",				kCmd_Common_action,				kCmdType_PUT	},
	{	"commandblind",			kCmd_Common_commandblind,		kCmdType_PUT	},
	{	"commandbool",			kCmd_Common_commandbool,		kCmdType_PUT	},
	{	"commandstring",		kCmd_Common_commandstring,		kCmdType_PUT	},
	{	"connected",			kCmd_Common_connected,			kCmdType_BOTH	},
	{	"description",			kCmd_Common_description,		kCmdType_GET	},
	{	"driverinfo",			kCmd_Common_driverinfo,			kCmdType_GET	},
	{	"driverversion",		kCmd_Common_driverversion,		kCmdType_GET	},
	{	"interfaceversion",		kCmd_Common_interfaceversion,	kCmdType_GET	},
	{	"name",					kCmd_Common_name,				kCmdType_GET	},
	{	"supportedactions",		kCmd_Common_supportedactions,	kCmdType_GET	},

	//*	extras added by MLS
	{	"--extras",				kCmd_Common_Extras,				kCmdType_GET	},
	{	"livewindow",			kCmd_Common_LiveWindow,			kCmdType_PUT	},

	//*	make sure to update kCommonCmdCnt if any commands are added

#ifdef _INCLUDE_EXIT_COMMAND_
	//*	the exit command was implemented for a special case application, it is not intended
	//*	to be used in the normal astronomy community
	{	"exit",					kCmd_Common_exit,				kCmdType_GET	},
#endif // _INCLUDE_EXIT_COMMAND_

	{	"",						-1,	0x00	}
};



//*****************************************************************************
bool	GetCmdNameFromTable(const int cmdNumber, char *comandName, const TYPE_CmdEntry *cmdTable, char *getPut)
{
int		iii;
bool	foundIt;

	strcpy(comandName, "????");
	foundIt	=	false;
	iii		=	0;
	while ((foundIt == false) && (cmdTable[iii].commandName[0] > 0x20))
	{
		if (cmdNumber == cmdTable[iii].enumValue)
		{
			strcpy(comandName, cmdTable[iii].commandName);
			*getPut	=	cmdTable[iii].get_put;
			foundIt	=	true;
		}
		iii++;
	}
	return(foundIt);
}


//*****************************************************************************
AlpacaDriver::AlpacaDriver(TYPE_DEVICETYPE argDeviceType)
{
int		alpacaDeviceNum;
int		iii;

//	CONSOLE_DEBUG("---------------------------------------");
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, argDeviceType);


	//*	set the common property defaults
	memset((void *)&cCommonProp, 0, sizeof(TYPE_CommonProperties));
	cCommonProp.InterfaceVersion	=	1;
	strcpy(cCommonProp.Name,			"unknown");
	sprintf(cCommonProp.DriverVersion, "%s Build %d", kVersionString, kBuildNumber);
	cCommonProp.Connected		=	false;

	cVerboseDebug				=	false;
	cMagicCookie				=	kMagicCookieValue;
	cDeviceModel[0]				=	0;
	cDeviceManufacturer[0]		=	0;
	cDeviceManufAbrev[0]		=	0;
	cDeviceSerialNum[0]			=	0;
	cDeviceVersion[0]			=	0;
	cDeviceFirmwareVersStr[0]	=	0;
	cTotalCmdsProcessed			=	0;
	cTotalCmdErrors				=	0;
	cLastUpdate_milliSecs		=	0;

	cTotalBytesRcvd				=	0;
	cTotalBytesSent				=	0;

	cUniqueID.part1				=	'ALPA';				//*	4 byte manufacturer code
	cUniqueID.part2				=	kBuildNumber;		//*	software version number
	cUniqueID.part3				=	1;					//*	model number
	cUniqueID.part4				=	0;
	cUniqueID.part5				=	0;					//*	serial number

	//========================================
	//*	discovery stuff
	cDiscoveryThreadActive		=	false;
	cDiscoveryThreadID			=	0;
	cBroadcastSocket			=	-1;
	cDiscoveryCount				=	0;

#ifdef _USE_OPENCV_
	//========================================
	//*	live window stuff
	cLiveController				=	NULL;
#endif // _USE_OPENCV_
	//========================================
	//*	Watchdog timer stuff
	cTimeOfLastValidCmd			=	time(NULL);		//*	these need to be set or it will do a shutdown before it even starts
	cTimeOfLastWatchDogCheck	=	time(NULL);
	cWatchDogTimeOut_Minutes	=	5;				//*	default timeout, can be overridden

	//==========================================================================================
	//*	add the device to the list
	cDeviceType	=	argDeviceType;
	//*	we have to figure out which index this devices is for this device type
	alpacaDeviceNum	=	0;
	for (iii=0; iii<gDeviceCnt; iii++)
	{
		//*	check to see if there are any other devices of this type
		if (argDeviceType == gAlpacaDeviceList[iii]->cDeviceType)
		{
			alpacaDeviceNum++;
		}
	}
	cDeviceNum		=	alpacaDeviceNum;
//	CONSOLE_DEBUG_W_NUM("cDeviceNum\t=", cDeviceNum);


	if (gDeviceCnt < kMaxDevices)
	{
		gAlpacaDeviceList[gDeviceCnt]	=	this;
		gDeviceCnt++;
	}
	else
	{
		CONSOLE_DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		CONSOLE_DEBUG("Exceed the maximum number of device objects");
		CONSOLE_DEBUG_W_NUM("Current max is (kMaxDevices)\t=", kMaxDevices);
		CONSOLE_ABORT(__FUNCTION__);
	}

	//*	command statistics
	for (iii=0; iii<kCommonCmdCnt; iii++)
	{
		memset(&cCommonCmdStats[iii], 0, sizeof(TYPE_CMD_STATS));
	}
	for (iii=0; iii<kDeviceCmdCnt; iii++)
	{
		memset(&cDeviceCmdStats[iii], 0, sizeof(TYPE_CMD_STATS));
	}
	GetAlpacaName(argDeviceType, cAlpacaName);
	LogEvent(	cAlpacaName,
				"Created",
				NULL,
				kASCOM_Err_Success,
				"");

#ifdef _ENABLE_BANDWIDTH_LOGGING_
	BandWidthStatsInit();

#endif // _ENABLE_BANDWIDTH_LOGGING_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
AlpacaDriver::~AlpacaDriver( void )
{
int	ii;

	cMagicCookie	=	0;

	CONSOLE_DEBUG(__FUNCTION__);
	//*	remove this device from the list
	for (ii=0; ii<kMaxDevices; ii++)
	{
		if (gAlpacaDeviceList[ii] == this)
		{
			gAlpacaDeviceList[ii]	=	NULL;
		}
	}
}

//**************************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

	CONSOLE_DEBUG("We should not be here, this routine needs to be over-ridden");
	//*	do nothing, this routine should be overridden
	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	return(alpacaErrCode);

}

//**************************************************************************************
TYPE_ASCOM_STATUS		AlpacaDriver::ProcessCommand_Common(	TYPE_GetPutRequestData	*reqData,
																const int				cmdEnum,
																char					*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
int					mySocket;

//	CONSOLE_DEBUG(__FUNCTION__);

	mySocket		=	reqData->socket;
	alpacaErrCode	=	kASCOM_Err_Success;
	switch(cmdEnum)
	{
		//========================================================================================
		//*	Common commands
		//========================================================================================
		case kCmd_Common_action:			//*	Invokes the specified device-specific action.
		case kCmd_Common_commandblind:		//*	Transmits an arbitrary string to the device
		case kCmd_Common_commandbool:		//*	Transmits an arbitrary string to the device and returns a boolean value from the device.
		case kCmd_Common_commandstring:		//*	Transmits an arbitrary string to the device and returns a string value from the device.
			JsonResponse_Add_String(mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									gValueString,
									"",
									INCLUDE_COMMA);
			break;

		case kCmd_Common_connected:			//*	GET--Retrieves the connected state of the device
											//*	PUT--Sets the connected state of the device
//			CONSOLE_DEBUG_W_STR("command=", "kCmd_Common_connected");
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_Connected(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Connected(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Common_description:		//*	Device description
			alpacaErrCode	=	Get_Description(reqData, alpacaErrMsg, gValueString);
			break;


		case kCmd_Common_driverinfo:		//*	Device driver description
			alpacaErrCode	=	Get_Driverinfo(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Common_driverversion:		//*	Driver Version
			alpacaErrCode	=	Get_Driverversion(reqData, alpacaErrMsg, gValueString);
			break;

		//*	The ASCOM Device interface version number that this device supports.
		case kCmd_Common_interfaceversion:
			alpacaErrCode	=	Get_Interfaceversion(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Common_name:				//*	Device name
			alpacaErrCode	=	Get_Name(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, NULL);
			break;

#ifdef _INCLUDE_EXIT_COMMAND_
		//*	the exit command was implemented for a special case application, it is not intended
		//*	to be used in the normal astronomy community
		case kCmd_Common_exit:
			gKeepRunning	=	false;
			break;
#endif // _INCLUDE_EXIT_COMMAND_

		case kCmd_Common_LiveWindow:
			alpacaErrCode	=	Put_LiveWindow(reqData, alpacaErrMsg);
			break;


		default:
			alpacaErrCode	=	kASCOM_Err_InvalidOperation;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Unrecognized command");
			CONSOLE_DEBUG(alpacaErrMsg);
			strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
			break;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Connected(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cCommonProp.Connected,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Put_Connected(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundKeyWord;
bool				connectFlag;
char				argumentString[32];

	CONSOLE_DEBUG(__FUNCTION__);

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Connected",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		connectFlag	=	IsTrueFalse(argumentString);
		if (connectFlag)
		{
			AlpacaConnect();
			LogEvent(	reqData->deviceType,
						"Connect",
						NULL,
						kASCOM_Err_Success,
						"");
			cCommonProp.Connected		=	true;
		}
		else
		{
			AlpacaDisConnect();
			LogEvent(	reqData->deviceType,
						"Dis-Connect",
						NULL,
						kASCOM_Err_Success,
						"");
			cCommonProp.Connected		=	false;
		}
	}
	else
	{
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Connected' not found");
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							gValueString,
							true,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
bool	AlpacaDriver::AlpacaConnect(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	return(false);
}

//*****************************************************************************
bool	AlpacaDriver::AlpacaDisConnect(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	return(false);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Description(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

#ifdef _DEBUG_CONFORM_
//	CONSOLE_DEBUG_W_LONG("description length\t=", strlen(cCommonProp.Description))
//	CONSOLE_DEBUG_W_STR("cCommonProp.Description\t=", cCommonProp.Description)
#endif // _DEBUG_CONFORM_

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cCommonProp.Description,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Driverinfo(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							"AlpacaPi C++ Open source by Mark Sproul, msproul@skychariot.com",
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Driverversion(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cCommonProp.DriverVersion,
							INCLUDE_COMMA);

//?	if (strlen(cDriverversionStr) > 0)
//?	{
//?		JsonResponse_Add_String(reqData->socket,
//?								reqData->jsonTextBuffer,
//?								kMaxJsonBuffLen,
//?								"libraryVersion",
//?								cDriverversionStr,
//?								INCLUDE_COMMA);
//?	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Interfaceversion(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cCommonProp.InterfaceVersion,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Name(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("cCommonProp.Name\t=", cCommonProp.Name)
	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cCommonProp.Name,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Readall_Common(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	Get_Connected(			reqData, alpacaErrMsg, "connected");
	Get_Description(		reqData, alpacaErrMsg, "description");
	Get_Driverinfo(			reqData, alpacaErrMsg, "driverinfo");
	Get_Driverversion(		reqData, alpacaErrMsg, "driverversion");
	Get_Interfaceversion(	reqData, alpacaErrMsg, "interfaceversion");
	Get_Name(				reqData, alpacaErrMsg, "name");

//	CONSOLE_DEBUG("exit");
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Readall_CPUstats(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
double		cpuTemp_DegC;
double		cpuTemp_DegF;
uint32_t	upTime;
char		textBuff[128];
int			upTime_Seconds;
int			upTime_Minutes;
int			upTime_Hours;
int			upTime_Days;
int			ram_Megabytes;
double		freeDiskSpace_Gigs;
bool		hasUSBfs;

//	CONSOLE_DEBUG(__FUNCTION__);
	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"platform",
							gPlatformString,
							INCLUDE_COMMA);

	if (gBogoMipsValue > 1.0)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"bogomips",
								gBogoMipsValue,
								INCLUDE_COMMA);
	}
	//====================================================
	cpuTemp_DegC	=	CPUstats_GetTemperature(NULL);
	cpuTemp_DegF	=	((cpuTemp_DegC * (9.0/5.0)) + 32);
	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"cpuTemp_DegC",
							cpuTemp_DegC,
							INCLUDE_COMMA);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"cpuTemp_DegF",
							cpuTemp_DegF,
							INCLUDE_COMMA);

	//====================================================
	upTime	=	CPUstats_GetUptime();

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"uptime_secs",
							upTime,
							INCLUDE_COMMA);

	upTime_Days		=	upTime / (24 * 60 * 60);
	upTime_Hours	=	(upTime / (60 * 60)) - (upTime_Days * 24);
	upTime_Minutes	=	(upTime / 60) - (upTime_Days * 24 * 60)- (upTime_Hours * 60);
	upTime_Seconds	=	upTime % 60;
	sprintf(textBuff, "%d days %02d:%02d:%02d",	upTime_Days,
												upTime_Hours,
												upTime_Minutes,
												upTime_Seconds);
	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"uptime_days",
							textBuff,
							INCLUDE_COMMA);

	//====================================================
	ram_Megabytes	=	CPUstats_GetTotalRam();
	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"totalRam_Megabytes",
							ram_Megabytes,
							INCLUDE_COMMA);
	//====================================================
	ram_Megabytes	=	CPUstats_GetFreeRam();
	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"freeRam_Megabytes",
							ram_Megabytes,
							INCLUDE_COMMA);

	//====================================================
	freeDiskSpace_Gigs	=	CPUstats_GetFreeDiskSpace("/") / 1024.0;
//	CONSOLE_DEBUG_W_DBL("freeDiskSpace_Gigs\t=", freeDiskSpace_Gigs);

	JsonResponse_Add_Double(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"freeDisk_Gigabytes",
							freeDiskSpace_Gigs,
							INCLUDE_COMMA);

	hasUSBfs	=	ReadUSBfsMemorySetting(textBuff);
	if (hasUSBfs)
	{
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"usbfs_memory_mb",
								textBuff,
								INCLUDE_COMMA);
	}

	return(kASCOM_Err_Success);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_SupportedActions(TYPE_GetPutRequestData *reqData, const TYPE_CmdEntry *theCmdTable)
{
int		iii;
char	lineBuffer[256];
int		mySocketFD;

	mySocketFD	=	reqData->socket;

	JsonResponse_Add_RawText(mySocketFD,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"\t\"Value\": \r\n\t[\r\n");

	//*	fist list the common commands
	iii		=	0;
	while (gCommonCmdTable[iii].commandName[0] != 0)
	{
		strcpy(lineBuffer, "\t\t\"");
		strcat(lineBuffer, gCommonCmdTable[iii].commandName);
		strcat(lineBuffer, "\"");
		strcat(lineBuffer, ",");
		strcat(lineBuffer, "\r\n");

		JsonResponse_Add_RawText(mySocketFD,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								lineBuffer);
		iii++;
	}

	//*	output the commands for the specific device
	if (theCmdTable != NULL)
	{
		iii		=	0;
		while (theCmdTable[iii].commandName[0] != 0)
		{
			//*	dont print out the entry if it begins with "-"
			if (theCmdTable[iii].commandName[0] != '-')
			{
				strcpy(lineBuffer, "\t\t\"");
				strcat(lineBuffer, theCmdTable[iii].commandName);
				strcat(lineBuffer, "\"");
				//*	do we need a comma?  The last one does not get a comma
				if (theCmdTable[iii+1].commandName[0] != 0)
				{
					strcat(lineBuffer, ",");
				}
				strcat(lineBuffer, "\r\n");

				JsonResponse_Add_RawText(mySocketFD,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										lineBuffer);
			}
			iii++;
		}
	}
	JsonResponse_Add_RawText(mySocketFD,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"\t],\r\n");
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Put_LiveWindow(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
#ifdef _USE_OPENCV_
bool				foundKeyWord;
char				argumentString[32];
bool				liveWindowFlg;

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Live",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		liveWindowFlg	=	IsTrueFalse(argumentString);
		if (liveWindowFlg)
		{
			alpacaErrCode	=	OpenLiveWindow(alpacaErrMsg);
		}
		else
		{
			alpacaErrCode	=	CloseLiveWindow(alpacaErrMsg);
		}
	}
	else
	{
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Live' not found");
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		CONSOLE_DEBUG(alpacaErrMsg);
	}
#endif // _USE_OPENCV_
	return(alpacaErrCode);

}

#ifdef _USE_OPENCV_
//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::OpenLiveWindow(char *alpacaErrMsg)
{
	//*	this is intended to be over-ridden by the sub-class
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "LiveWindow not implemented");
	return(kASCOM_Err_MethodNotImplemented);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::CloseLiveWindow(char *alpacaErrMsg)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (cLiveController != NULL)
	{
		cLiveController->cKeepRunning	=	false;
	}
	cLiveController	=	NULL;	//*	added 2/14/2022
	return(kASCOM_Err_Success);
}

//*****************************************************************************
void	AlpacaDriver::UpdateLiveWindow(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	//*	this is intended to be over-ridden by the sub-class
}
#endif // _USE_OPENCV_


//*****************************************************************************
void	AlpacaDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	AlpacaDriver::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
bool	AlpacaDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
	CONSOLE_DEBUG("This function should be over-ridden");
//	GetCmdNameFromMyCmdTable(cmdNumber, cmdName, gCommonCmdTable, getPut);
	strcpy(comandName, "----");
	CONSOLE_ABORT(__FUNCTION__);
	return(false);

}

//*****************************************************************************
static void	GenerateCmdStatsEntry(	const char	*cmdName,
									const char	getPutIndicator,
									const int	connCnt,
									const int	getCnt,
									const int	putCnt,
									const int	errorCnt,
									char		*lineBuffer)
{
char	getBuff[64];
char	putBuff[64];
bool	getPutErr;

	if (strncmp(cmdName, "--extra", 7) == 0)
	{
		strcpy(lineBuffer, "<TR><TD COLSPAN=5><CENTER>--Extra non-Alpaca--</TD></TR>");
	}
	else
	{
		getPutErr	=	false;
		//*	Get count
		if ((getPutIndicator == 'G') || (getPutIndicator == 'B'))
		{
			sprintf(getBuff, "<TD><CENTER>%d</TD>", getCnt);
		}
		else if (getCnt > 0)
		{
			//*	this is an error,
			sprintf(getBuff, "<TD><CENTER><FONT COLOR=RED>%d</TD>", getCnt);
			getPutErr	=	true;
		}
		else
		{
			strcpy(getBuff, "<TD><CENTER>--</TD>");
		}

		//*	Put count
		if ((getPutIndicator == 'P') || (getPutIndicator == 'B'))
		{
			sprintf(putBuff, "<TD><CENTER>%d</TD>", putCnt);
		}
		else if (putCnt > 0)
		{
			//*	this is an error,
			sprintf(getBuff, "<TD><CENTER><FONT COLOR=RED>%d</TD>", putCnt);
			getPutErr	=	true;
		}
		else
		{
			strcpy(putBuff, "<TD><CENTER>--</TD>");
		}
		sprintf(lineBuffer,	"<TR><TD>%s</TD><TD><CENTER>%d</TD>%s%s<TD><CENTER>%d</TD>",
								cmdName,
								connCnt,
								getBuff,
								putBuff,
								errorCnt);
	#define	_SHOW_GETPUT_ERRORS
	#ifdef _SHOW_GETPUT_ERRORS
		if (getPutErr)
		{
			strcat(lineBuffer, "<TD><FONT COLOR=RED><<-Error</TD>\r\n");
		}
		strcat(lineBuffer, "</TR>\r\n");
	#endif // _SHOW_GETPUT_ERRORS
	}
}

//*****************************************************************************
void	AlpacaDriver::OutputHTML_CmdStats(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[512];
//char	getBuff[64];
//char	putBuff[64];
int		mySocketFD;
int		iii;
char	cmdName[32];
bool	foundIt;
int		total_Conn;
int		total_Get;
int		total_Put;
int		total_Errors;
char	getPutIndicator;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cCommonProp.Name);

	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");

	sprintf(lineBuffer, "%s<BR>\r\n", cCommonProp.Name);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	sprintf(lineBuffer,	"<TR><TH>%s</TH><TH>%s</TH><TH>%s</TH><TH>%s</TH><TH>%s</TH></TR>\r\n",
							"Command",
							"Total #",
							"Get #",
							"Put #",
							"Errors");

	SocketWriteData(mySocketFD,	lineBuffer);
	total_Conn		=	0;
	total_Get		=	0;
	total_Put		=	0;
	total_Errors	=	0;

	//===============================================================
	//*	first do the common commands
	strcpy(cmdName, "???");
	for (iii=0; iii<kCommonCmdCnt; iii++)
	{
		//*	get the info about this command
		foundIt	=	GetCmdNameFromTable((kCmd_Common_action + iii),
										cmdName,
										gCommonCmdTable,
										&getPutIndicator);
		if (foundIt)
		{
			GenerateCmdStatsEntry(	cmdName,
									getPutIndicator,
									cCommonCmdStats[iii].connCnt,
									cCommonCmdStats[iii].getCnt,
									cCommonCmdStats[iii].putCnt,
									cCommonCmdStats[iii].errorCnt,
									lineBuffer);

			SocketWriteData(mySocketFD,	lineBuffer);

			total_Conn		+=	cCommonCmdStats[iii].connCnt;
			total_Get		+=	cCommonCmdStats[iii].getCnt;
			total_Put		+=	cCommonCmdStats[iii].putCnt;
			total_Errors	+=	cCommonCmdStats[iii].errorCnt;
		}
	}

	SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=5><HR></TD></TR>\r\n");
	SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=5><CENTER><B>Device Specific Commands</TD></TR>\r\n");

	//===============================================================
	//*	now do the commands for this device
	for (iii=0; iii<kDeviceCmdCnt; iii++)
	{
		//*	get the info about this command
		foundIt	=	GetCmdNameFromMyCmdTable(iii, cmdName, &getPutIndicator);
		if (foundIt)
		{
			GenerateCmdStatsEntry(	cmdName,
									getPutIndicator,
									cDeviceCmdStats[iii].connCnt,
									cDeviceCmdStats[iii].getCnt,
									cDeviceCmdStats[iii].putCnt,
									cDeviceCmdStats[iii].errorCnt,
									lineBuffer);

			SocketWriteData(mySocketFD,	lineBuffer);

			total_Conn		+=	cDeviceCmdStats[iii].connCnt;
			total_Get		+=	cDeviceCmdStats[iii].getCnt;
			total_Put		+=	cDeviceCmdStats[iii].putCnt;
			total_Errors	+=	cDeviceCmdStats[iii].errorCnt;
		}
	}
	SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=5><HR></TD></TR>\r\n");
	sprintf(lineBuffer,	"<TR><TH>%s</TH><TH>%d</TH><TH>%d</TH><TH>%d</TH><TH>%d</TH></TR>\r\n",
							"Totals",
							total_Conn,
							total_Get,
							total_Put,
							total_Errors);
	SocketWriteData(mySocketFD,	lineBuffer);


	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");


#ifdef _ENABLE_BANDWIDTH_LOGGING_
	//----------------------------------------------------------------------------------
	//*	BandWidth Statistics
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>Band width statistics</H2>\r\n");
	SocketWriteData(mySocketFD,	"<BR><B>Per minute, over the last hour</B>\r\n");
	SocketWriteData(mySocketFD,	"<BR>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	sprintf(lineBuffer,	"<TR><TH>%s</TH><TH>%s</TH><TH>%s</TH><TH>%s</TH></TR>\r\n",
							"Time Unit",
							"Cmds Rcvd",
							"Bytes Rcvd",
							"Bytes Sent");
	SocketWriteData(mySocketFD,	lineBuffer);
	for (iii=0; iii<kMaxBandWidthSamples; iii++)
	{

		sprintf(lineBuffer,	"<TR><TD>%d</TD>\r\n", iii);
		SocketWriteData(mySocketFD,	lineBuffer);

		sprintf(lineBuffer,	"<TD>%d</TD><TD>%d</TD><TD>%d</TD>\r\n",
								cBW_CmdsReceived[iii],
								cBW_BytesReceived[iii],
								cBW_BytesSent[iii]);
		SocketWriteData(mySocketFD,	lineBuffer);

		SocketWriteData(mySocketFD,	"</TR>\r\n");
	}



	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");
#endif // _ENABLE_BANDWIDTH_LOGGING_

}


//*****************************************************************************
void	AlpacaDriver::OutputHTMLrowData(int socketFD, const char *string1, const char *string2)
{
	SocketWriteData(socketFD,	"<TR>\r\n\t<TD>");
	SocketWriteData(socketFD,	string1);
	SocketWriteData(socketFD,	"</TD>\r\n\t<TD>");
	SocketWriteData(socketFD,	string2);
	SocketWriteData(socketFD,	"</TD>\r\n</TR>\r\n");

}


//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	AlpacaDriver::RunStateMachine(void)
{
	//*	5 * 1000 * 1000 means you might not get called again for 5 seconds
	//*	you might get called earlier
	return(5 * 1000 * 1000);
}


//*****************************************************************************
void	AlpacaDriver::RecordCmdStats(int cmdNum, char getput, TYPE_ASCOM_STATUS alpacaErrCode)
{
int		tblIdx;

	//*	check for common command index ( > 1000)
	if (cmdNum >= kCmd_Common_action)
	{
		tblIdx	=	cmdNum - kCmd_Common_action;
		if ((tblIdx >= 0) && (tblIdx < kCommonCmdCnt))
		{
			cCommonCmdStats[tblIdx].connCnt++;
			if (getput == 'G')
			{
				cCommonCmdStats[tblIdx].getCnt++;
			}
			else if (getput == 'P')
			{
				cCommonCmdStats[tblIdx].putCnt++;
			}

			if (alpacaErrCode != 0)
			{
				cCommonCmdStats[tblIdx].errorCnt++;
			}
		}
	}
	else if ((cmdNum >= 0) && (cmdNum < kDeviceCmdCnt))
	{
		cDeviceCmdStats[cmdNum].connCnt++;
		if (getput == 'G')
		{
			cDeviceCmdStats[cmdNum].getCnt++;
		}
		else if (getput == 'P')
		{
			cDeviceCmdStats[cmdNum].putCnt++;
		}

		if (alpacaErrCode != 0)
		{
			cDeviceCmdStats[cmdNum].errorCnt++;
		}
	}
	else
	{
		CONSOLE_DEBUG("Command index out of bounds");
	}
}



#pragma mark -

#ifdef _ENABLE_BANDWIDTH_LOGGING_
//*****************************************************************************
void	AlpacaDriver::BandWidthStatsInit(void)
{
int	iii;

	for (iii=0; iii<kMaxBandWidthSamples; iii++)
	{
		cBW_CmdsReceived[iii]	=	0;
		cBW_BytesReceived[iii]	=	0;
		cBW_BytesSent[iii]		=	0;
	}
	gTimeUnitsSinceTopOfHour	=	0;
}

#endif // _ENABLE_BANDWIDTH_LOGGING_

#pragma mark -

//*****************************************************************************
int	SocketWriteData(const int socket, const char *dataBuffer)
{
int		bufferLen;
int		bytesWritten;

#ifdef _DEBUG_CONFORM_
//	CONSOLE_DEBUG_W_STR("socket>\t", dataBuffer);
#endif // _DEBUG_CONFORM_

	bufferLen		=	strlen(dataBuffer);
	bytesWritten	=	write(socket, dataBuffer, bufferLen);
	if (bytesWritten < 0)
	{
	//	fprintf(stderr, "ERROR writing to socket");
		CONSOLE_ERROR("ERROR writing to socket");
		CONSOLE_ERROR_W_NUM("bytesWritten\t=", bytesWritten);
	}
	return(bytesWritten);
}



//*****************************************************************************
static void	StrcpyToEOL(char *newString, const char *oldString, const int maxLen)
{
int		ii;

	ii	=	0;
	while ((oldString[ii] >= 0x20) && (ii < maxLen))
	{
		newString[ii]	=	oldString[ii];
		ii++;
	}
	if (ii < maxLen)
	{
		newString[ii]	=	0;
	}
	else
	{
		newString[maxLen-1]	=	0;
	}

}

//*****************************************************************************
void	GetTimeString(char *timeString)
{
time_t		currentTime;
struct tm	*linuxTime;

	if (timeString != NULL)
	{
		timeString[0]	=	0;
		currentTime		=	time(NULL);
		if (currentTime != -1)
		{
			linuxTime		=	localtime(&currentTime);
			sprintf(timeString, "%d/%d/%d %02d:%02d:%02d",
									(1 + linuxTime->tm_mon),
									linuxTime->tm_mday,
									(1900 + linuxTime->tm_year),
									linuxTime->tm_hour,
									linuxTime->tm_min,
									linuxTime->tm_sec);
		}
	}
}

//*****************************************************************************
const char	gBadResponse400[]	=
{
	"HTTP/1.0 400 Bad Request\r\n"
	"Server: AkamaiGHost\r\n"
	"Mime-Version: 1.0\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 207\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<!DOCTYPE html>\r\n"
	"<HTML><HEAD>\r\n"
	"<TITLE>Invalid URL</TITLE>\r\n"
	"</HEAD><BODY>\r\n"
	"<H1>Invalid URL</H1>\r\n"
	"The requested URL , is invalid.<p>\r\n"
//	"Reference&#32;&#35;9&#46;d0fb4317&#46;1555413904&#46;e81982\r\n"
	"</BODY></HTML>\r\n"
};

//*****************************************************************************
const char	gHtmlHeader[]	=
{
	"HTTP/1.0 200 \r\n"
//	"Server: alpaca\r\n"
//	"Mime-Version: 1.0\r\n"
	"Content-Type: text/html\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<!DOCTYPE html>\r\n"
	"<HTML><HEAD>\r\n"



};

#define _USE_BLACK_HTML_
//*****************************************************************************
const char	gHtmlNightMode[]	=
{
#ifdef _USE_BLACK_HTML_
//	"<BODY bgcolor=#111111>\r\n"
	"<BODY bgcolor=#000000>\r\n"
	"<FONT COLOR=white>\r\n"
	"<BODY link=red>\r\n"
	"<BODY vlink=pink>\r\n"
#else
//	"<BODY bgcolor=#111111>\r\n"
	"<BODY bgcolor=#40098E>\r\n"
//	"<BODY bgcolor=#222222>\r\n"
	"<FONT COLOR=white>\r\n"
	"<BODY link=red>\r\n"
//	"<BODY vlink=pink>\r\n"
	"<BODY vlink=green>\r\n"
#endif

};

#pragma mark -
//*****************************************************************************
static void	SendHtmlResponse(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
char	separaterLine[]	=	"<HR SIZE=4 COLOR=RED>\r\n";
//char	separaterLine[]	=	"<HR SIZE=4 COLOR=BLUE>\r\n";
int		mySocketFD;
int		ii;


	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;

		SocketWriteData(mySocketFD,	gHtmlHeader);

		sprintf(lineBuffer, "<TITLE>%s</TITLE>\r\n", gWebTitle);

		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	gHtmlNightMode);
		SocketWriteData(mySocketFD,	"</HEAD><BODY>\r\n<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H1>Alpaca device driver Web server</H1>\r\n");
		sprintf(lineBuffer, "<H3>%s</H3>\r\n", gWebTitle);

		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		OutPutObservatoryInfoHTML(mySocketFD);

		SocketWriteData(mySocketFD,	"<A HREF=../log target=log>Click here for log</A><BR>\r\n");
		SocketWriteData(mySocketFD,	"<A HREF=../stats target=log>Click here for stats</A><BR>\r\n");

	#ifdef	_ENABLE_CAMERA_
		SocketWriteData(mySocketFD,	"Camera support is enabled<BR>\r\n");
	#endif

	#ifdef	_ENABLE_DOME_
		SocketWriteData(mySocketFD,	"Dome support is enabled<BR>\r\n");
	#endif
	#ifdef	_ENABLE_ROR_
		SocketWriteData(mySocketFD,	"ROR support is enabled<BR>\r\n");
	#endif

#if defined(_ENABLE_FILTERWHEEL_) || defined(_ENABLE_FILTERWHEEL_ZWO_) || defined(_ENABLE_FILTERWHEEL_ATIK_)
		SocketWriteData(mySocketFD,	"Filterwheel support is enabled<BR>\r\n");
	#endif

	#ifdef	_ENABLE_FOCUSER_
		SocketWriteData(mySocketFD,	"Focuser support is enabled<BR>\r\n");
	#endif

	#ifdef	_ENABLE_ROTATOR_
		SocketWriteData(mySocketFD,	"Rotator support is enabled<BR>\r\n");
	#endif

	#ifdef	_ENABLE_TELESCOPE_
		SocketWriteData(mySocketFD,	"Telescope support is enabled<BR>\r\n");
	#endif

	#ifdef	_ENABLE_OBSERVINGCONDITIONS_
		SocketWriteData(mySocketFD,	"Observingconditions support is enabled<BR>\r\n");
	#endif
	#ifdef _ENABLE_CALIBRATION_
		SocketWriteData(mySocketFD,	"CoverCalibration support is enabled<BR>\r\n");
	#endif // _ENABLE_CALIBRATION_

	#ifdef	_ENABLE_SAFETYMONITOR_
		SocketWriteData(mySocketFD,	"SafetyMonitor support is enabled<BR>\r\n");
	#endif

	#ifdef	_ENABLE_SWITCH_
		SocketWriteData(mySocketFD,	"Switch support is enabled<BR>\r\n");
	#endif
	#ifdef _ENABLE_MULTICAM_
		SocketWriteData(mySocketFD,	"Multicam is enabled<BR>\r\n");
	#endif
	#ifdef _ENABLE_SHUTTER_
		SocketWriteData(mySocketFD,	"Shutter is enabled<BR>\r\n");
	#endif
	#ifdef _ENABLE_SLIT_TRACKER_
		SocketWriteData(mySocketFD,	"Slit Tracker is enabled<BR>\r\n");
	#endif // _ENABLE_SLIT_TRACKER_

		SocketWriteData(mySocketFD,	separaterLine);
		//=============================================================================
		//*	print out a table of active devices
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");


		SocketWriteData(mySocketFD,	"The following devices are configured on this Remote Server:<P>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=2>\r\n");
		//*	do the header row
		SocketWriteData(mySocketFD,	"\t<TR>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device Type</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device Number</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device Name</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Description</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Cmds / Errs</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t</TR>\r\n");

		for (ii=0; ii<gDeviceCnt; ii++)
		{
			if (gAlpacaDeviceList[ii] != NULL)
			{
				SocketWriteData(mySocketFD,	"\t<TR>\r\n");

				SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
					SocketWriteData(mySocketFD,	gAlpacaDeviceList[ii]->cAlpacaName);
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				sprintf(lineBuffer, "<TD><CENTER>%d</TD>\r\n", gAlpacaDeviceList[ii]->cDeviceNum);
				SocketWriteData(mySocketFD,	lineBuffer);

				SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
					SocketWriteData(mySocketFD,	gAlpacaDeviceList[ii]->cCommonProp.Name);
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
					SocketWriteData(mySocketFD,	gAlpacaDeviceList[ii]->cCommonProp.Description);
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				sprintf(lineBuffer, "<TD><CENTER>%d/%d</TD>\r\n",
											gAlpacaDeviceList[ii]->cTotalCmdsProcessed,
											gAlpacaDeviceList[ii]->cTotalCmdErrors);
				SocketWriteData(mySocketFD,	lineBuffer);


				SocketWriteData(mySocketFD,	"\t</TR>\r\n");

			}
		}

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		//**********************************************************
		//*	Output the html for each device
		for (ii=0; ii<gDeviceCnt; ii++)
		{
//			CONSOLE_DEBUG_W_STR(__FUNCTION__, gAlpacaDeviceList[ii]->cCommonProp.Name);
			if (gAlpacaDeviceList[ii] != NULL)
			{
				SocketWriteData(mySocketFD,	separaterLine);
				gAlpacaDeviceList[ii]->OutputHTML(reqData);
				gAlpacaDeviceList[ii]->OutputHTML_Part2(reqData);
			}
		}

		//**********************************************************
		SocketWriteData(mySocketFD,	separaterLine);

		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H3>Versions</H3>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

		//*	this software
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>AlpacaDriver</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%s</TD>\r\n", kVersionString);
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>Build #</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%d</TD>\r\n", kBuildNumber);
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");


		//*	OS version
		if (strlen(gOsReleaseString) > 0)
		{
			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>OS Version</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%s</TD>\r\n", gOsReleaseString);
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");
		}

		//*	cpu we are running on
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>cpu</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%s</TD>\r\n", gCpuInfoString);
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		//*	gcc version
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>gcc</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%s</TD>\r\n", __VERSION__);
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		//*	glib version
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>libc version</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%s</TD>\r\n", gnu_get_libc_version());
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

#if defined(_ENABLE_WIRING_PI_) && defined(__arm__) && defined(__WIRING_PI_H__)
		//*	wiringPi version
		int		wiringPi_verMajor;
		int		wiringPi_verMinor;
		char	wiringPi_VerString[32];


		wiringPiVersion(&wiringPi_verMajor, &wiringPi_verMinor);
		sprintf(wiringPi_VerString, "%d.%d", wiringPi_verMajor, wiringPi_verMinor);
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>wiringPi version</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%s</TD>\r\n", wiringPi_VerString);
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");
#elif defined(_ENABLE_WIRING_PI_) || defined(__WIRING_PI_H__)
	#warning "Somethings wrong, wiringPi is enabled"
#endif


#ifdef _ENABLE_FITS_
		//*	cfitsio version
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>FITS (cfitsio)</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%d.%d</TD>\r\n", CFITSIO_MAJOR, CFITSIO_MINOR);
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");
#endif // _ENABLE_FITS_

#ifdef _USE_OPENCV_
		//*	openCV version
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>OpenCV</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%s</TD>\r\n", CV_VERSION);
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");
#endif

#ifdef _ENABLE_JPEGLIB_
		//*	jpeg lib version
		SocketWriteData(mySocketFD,	"<TR>\r\n");
			SocketWriteData(mySocketFD,	"<TD>libjpeg</TD>\r\n");
			sprintf(lineBuffer,	"<TD>%d</TD>\r\n", JPEG_LIB_VERSION);
			SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TR>\r\n");
#endif



		if ((sizeof(long) != 4) || (sizeof(int*) != 4))
		{
			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>sizeof(char)</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%ld</TD>\r\n", (long)sizeof(char));
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");

			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>sizeof(short)</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%ld</TD>\r\n", (long)sizeof(short));
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");

			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>sizeof(int)</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%ld</TD>\r\n", (long)sizeof(int));
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");

			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>sizeof(float)</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%ld</TD>\r\n", (long)sizeof(float));
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");

			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>sizeof(long)</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%ld</TD>\r\n", (long)sizeof(long));
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");

			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>sizeof(ptr)</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%ld</TD>\r\n", (long)sizeof(int*));
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");

			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>sizeof(double)</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%ld</TD>\r\n", (long)sizeof(double));
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");

			SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"<TD>sizeof(long double)</TD>\r\n");
				sprintf(lineBuffer,	"<TD>%ld</TD>\r\n", (long)sizeof(long double));
				SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");
		}

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		//**********************************************************
		SocketWriteData(mySocketFD,	separaterLine);
		SocketWriteData(mySocketFD,	"<H3>Links</H3>\r\n");
		SocketWriteData(mySocketFD,	"<UL>\r\n");

		SocketWriteData(mySocketFD,	"\t<LI><A HREF=https://ascom-standards.org/Developer/Alpaca.htm target=link>https://ascom-standards.org/Developer/Alpaca.htm</A>\r\n");
		SocketWriteData(mySocketFD,	"\t<LI><A HREF=https://ascom-standards.org/api/ target=link>https://ascom-standards.org/api/</A>\r\n");
//		SocketWriteData(mySocketFD,	"\t<LI><A HREF=https://agenaastro.com/zwo-astronomy-cameras-buyers-guide.html target=link>https://agenaastro.com/zwo-astronomy-cameras-buyers-guide.html</A>\r\n");
		SocketWriteData(mySocketFD,	"\t<LI><A HREF=https://agenaastro.com/articles/zwo-astronomy-cameras-buyers-guide.html target=link>https://agenaastro.com/articles/zwo-astronomy-cameras-buyers-guide.html</A>\r\n");
//		SocketWriteData(mySocketFD,	"\t<LI><A HREF=https://agenaastro.com/articles/guides/zwo-buyers-guide.html target=link>https://agenaastro.com/articles/guides/zwo-buyers-guide.html</A>\r\n");

		SocketWriteData(mySocketFD,	"\t<LI><A HREF=https://astronomy-imaging-camera.com/software-drivers target=link>https://astronomy-imaging-camera.com/software-drivers</A>\r\n");

		SocketWriteData(mySocketFD,	"</UL>\r\n");


		//**********************************************************
		SocketWriteData(mySocketFD,	separaterLine);
		SocketWriteData(mySocketFD,	"Compiled on ");
		SocketWriteData(mySocketFD,	__DATE__);
		SocketWriteData(mySocketFD,	"\r\n<BR>");
		SocketWriteData(mySocketFD,	"C++ version\r\n<BR>");
		SocketWriteData(mySocketFD,	"(C) 2020-21 by Mark Sproul msproul@skychariot.com\r\n<BR>");

		SocketWriteData(mySocketFD,	"</BODY></HTML>\r\n");
	}
	else
	{
	//	CONSOLE_DEBUG("reqData is NULL");
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//*****************************************************************************
static void	SendHtmlStats(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
char	separaterLine[]	=	"<HR SIZE=4 COLOR=RED>\r\n";
//char	separaterLine[]	=	"<HR SIZE=4 COLOR=BLUE>\r\n";
int		mySocketFD;
int		ii;

	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;
		SocketWriteData(mySocketFD,	gHtmlHeader);
//		SocketWriteData(mySocketFD,	gHtmlNightMode);
		sprintf(lineBuffer, "<TITLE>%s</TITLE>\r\n", gWebTitle);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</HEAD><BODY>\r\n<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H1>Alpaca device driver Web server</H1>\r\n");
		sprintf(lineBuffer, "<H3>%s</H3>\r\n", gWebTitle);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		OutPutObservatoryInfoHTML(mySocketFD);

		for (ii=0; ii<gDeviceCnt; ii++)
		{
			if (gAlpacaDeviceList[ii] != NULL)
			{
				SocketWriteData(mySocketFD,	separaterLine);
				gAlpacaDeviceList[ii]->OutputHTML_CmdStats(reqData);
			}
		}


		SocketWriteData(mySocketFD,	separaterLine);
		SocketWriteData(mySocketFD,	"Compiled on ");
		SocketWriteData(mySocketFD,	__DATE__);
		SocketWriteData(mySocketFD,	"\r\n<BR>");
		SocketWriteData(mySocketFD,	"C++ version\r\n<BR>");
		SocketWriteData(mySocketFD,	"(C) 2020-21 by Mark Sproul msproul@skychariot.com\r\n<BR>");

		SocketWriteData(mySocketFD,	"</BODY></HTML>\r\n");
	}
	else
	{
	//	CONSOLE_DEBUG("reqData is NULL");
	}
}


//*****************************************************************************
void	GenerateHTMLcmdLinkTable(	int			socketFD,
									const char	*deviceName,
									const int	deviceNum,
									const TYPE_CmdEntry *cmdTable)
{
int		ii;
char	lineBuffer[256];

//	CONSOLE_DEBUG_W_STR("deviceName\t=", deviceName);
//	CONSOLE_DEBUG_W_NUM("deviceNum\t=", deviceNum);
	//*	now generate links to all of the commands
	SocketWriteData(socketFD,	"\r\n<UL>\r\n");
	ii	=	0;
	while (gCommonCmdTable[ii].commandName[0] != 0)
	{
		sprintf(lineBuffer,	"\t<LI><A HREF=../api/v1/%s/%d/%s target=cmd>%s</A>\r\n",
									deviceName,
									deviceNum,
									gCommonCmdTable[ii].commandName,
									gCommonCmdTable[ii].commandName);
		SocketWriteData(socketFD,	lineBuffer);
		ii++;
	}

	SocketWriteData(socketFD,	"<P>\r\n");

	ii	=	0;
	while (cmdTable[ii].commandName[0] != 0)
	{
		sprintf(lineBuffer,	"\t<LI><A HREF=../api/v1/%s/%d/%s target=cmd>%s</A>\r\n",
									deviceName,
									deviceNum,
									cmdTable[ii].commandName,
									cmdTable[ii].commandName);
		SocketWriteData(socketFD,	lineBuffer);
//		CONSOLE_DEBUG(lineBuffer);
		ii++;
	}
	SocketWriteData(socketFD,	"</UL>\r\n");
}

//*****************************************************************************
const char	gJpegHeader[]	=
{
	"HTTP/1.0 200 ok\r\n"
//	"Server: alpaca\r\n"
	"Mime-Version: 1.0\r\n"
	"Content-Type: image/jpeg\r\n"
	"Connection: close\r\n"
	"\r\n"

};

//*****************************************************************************
static void	SendJpegResponse(int socket, char *jpegFileName)
{
FILE			*filePointer;
int				numRead;
int				bytesWritten;
int				totalBytesWritten;
bool			keepGoing;
char			dataBuffer[1024];
char			myJpegFileName[128];
char			*myFilenamePtr;

//	CONSOLE_DEBUG(__FUNCTION__);

	SocketWriteData(socket,	gJpegHeader);

	if (jpegFileName != NULL)
	{
//		CONSOLE_DEBUG_W_STR("jpegFileName=", jpegFileName);

		myFilenamePtr	=	jpegFileName;
		if (*myFilenamePtr == '/')
		{
			myFilenamePtr++;
		}
		strncpy(myJpegFileName, myFilenamePtr, 100);

		myFilenamePtr	=	strstr(myJpegFileName, ".jpg");
		if (myFilenamePtr != NULL)
		{
			myFilenamePtr[4]	=	0;
		}
		myFilenamePtr	=	strstr(myJpegFileName, ".png");
		if (myFilenamePtr != NULL)
		{
			myFilenamePtr[4]	=	0;
		}
	}
	else
	{
		strcpy(myJpegFileName, "image.jpg");
	}
//	CONSOLE_DEBUG_W_STR("myJpegFileName=", myJpegFileName);

	filePointer	=	fopen(myJpegFileName, "r");
	if (filePointer != NULL)
	{
//		CONSOLE_DEBUG_W_STR("File is open:", myJpegFileName);
		totalBytesWritten	=	0;
		keepGoing			=	true;
		while (keepGoing)
		{
			numRead	=	fread(dataBuffer, 1, 1000, filePointer);
			if ((numRead > 0) || (feof(filePointer)))
			{
//				CONSOLE_DEBUG_W_NUM("numRead=", numRead);
				bytesWritten		=	write(socket, dataBuffer, numRead);
				totalBytesWritten	+=	bytesWritten;
			}
			else
			{
				keepGoing	=	false;
			}

			if (feof(filePointer))
			{
				keepGoing	=	false;
			}
		}
		fclose(filePointer);
//		CONSOLE_DEBUG_W_STR("File is closed:", myJpegFileName);
//		CONSOLE_DEBUG_W_NUM("totalBytesWritten\t=",	totalBytesWritten);
	}
	else
	{
		CONSOLE_DEBUG("Failed to open file");
	}
//	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
static TYPE_ASCOM_STATUS	ProcessAlpacaAPIrequest(TYPE_GetPutRequestData	*reqData,
													char					*parseChrPtr,
													long					byteCount)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
//int				alpacaVerNum;
int					deviceTypeEnum;
int					cc;
int					iii;
int					slen;
int					cmdBuffLen;
char				deviceStringBuffer[kDevStrLen];
char				*slashPtr;
char				*httpPtr;
bool				deviceFound;
char				argumentString[64];
bool				foundKeyWord;

#ifdef _DEBUG_CONFORM_
//	CONSOLE_DEBUG("/api/ found");
//	CONSOLE_DEBUG_W_STR("parseChrPtr\t=", parseChrPtr);
#endif // _DEBUG_CONFORM_

	deviceTypeEnum	=	-1;
	//*	so far so good, now get the version
	parseChrPtr	+=	5;
	if (*parseChrPtr == 'v')
	{
		parseChrPtr++;
	}
	if (isdigit(*parseChrPtr))
	{
//		alpacaVerNum	=	atoi(parseChrPtr);
//		CONSOLE_DEBUG_W_NUM("Alpaca version\t=", alpacaVerNum);

		//*	now skip over the version number
		while ((*parseChrPtr != '/') && (*parseChrPtr > 0))
		{
			parseChrPtr++;
		}
		if (*parseChrPtr == '/')
		{
			parseChrPtr++;
		}

		//*	we should now be to what really matters
#ifdef _DEBUG_CONFORM_
		CONSOLE_DEBUG_W_STR("Device data=", parseChrPtr);
#endif // _DEBUG_CONFORM_
		cc	=	0;
		while ((*parseChrPtr >= 0x20) && (cc < kDevStrLen))
		{
			deviceStringBuffer[cc++]	=	*parseChrPtr;
			deviceStringBuffer[cc]		=	0;

			parseChrPtr++;
		}

		strncpy(reqData->deviceType, deviceStringBuffer, 60);
		reqData->deviceType[59]	=	0;
		slashPtr	=	strchr(reqData->deviceType, '/');
		if (slashPtr != NULL)
		{
			*slashPtr	=	0;
		}


		//*	now get the device number
		slashPtr	=	strchr(deviceStringBuffer, '/');
		if (slashPtr != NULL)
		{
			slashPtr++;
			reqData->deviceNumber	=	atoi(slashPtr);

			//*	now skip the device number
			while (isdigit(*slashPtr))
			{
				slashPtr++;
			}
			if (*slashPtr == '/')
			{
				slashPtr++;
			}
			StrcpyToEOL(reqData->cmdBuffer, slashPtr, kDevStrLen);
			httpPtr	=	strstr(reqData->cmdBuffer, "HTTP");
			if (httpPtr != NULL)
			{
				*httpPtr	=	0;
				httpPtr--;
				while (*httpPtr <= 0x20)
				{
					*httpPtr	=	0;
					httpPtr--;
				}
			}

			//*	check the command string make sure its terminated properly
			slen	=	strlen(reqData->cmdBuffer);
			for (iii=0; iii<slen; iii++)
			{
				if ((reqData->cmdBuffer[iii] == '%') || (reqData->cmdBuffer[iii] == '"'))
				{
					reqData->cmdBuffer[iii]	=	0;
					break;
				}
			}
		}

#ifdef _DEBUG_CONFORM_
		CONSOLE_DEBUG_W_STR("Device type\t=",	reqData->deviceType);
//		DumpRequestStructure(__FUNCTION__, reqData);
#endif // _DEBUG_CONFORM_

		//	curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/camera/0/cooleron"
		//			-H  "accept: application/json"
		//			-H  "Content-Type: application/x-www-form-urlencoded"
		//			-d "CoolerOn=true&ClientID=223&ClientTransactionID=45"
		//	filterwheel/0/interfaceversion?ClientTransactionID=2&ClientID=12498 HTTP/1.1

		//*	Check for client ID
		foundKeyWord	=	GetKeyWordArgument(reqData->contentData, "ClientID", argumentString, 31);
		if (foundKeyWord)
		{
			gClientID	=	atoi(argumentString);
		}
#ifdef _DEBUG_CONFORM_
		else
		{
//			CONSOLE_DEBUG("gClientID NOT FOUND");
		}
#endif // _DEBUG_CONFORM_

		//*	Check for ClientTransactionID
		foundKeyWord	=	GetKeyWordArgument(reqData->contentData, "ClientTransactionID", argumentString, 31);
		if (foundKeyWord)
		{
			gClientTransactionID	=	atoi(argumentString);
		}
#ifdef _DEBUG_CONFORM_
		else
		{
//			CONSOLE_DEBUG("gClientTransactionID NOT FOUND");
		}
		CONSOLE_DEBUG_W_NUM("gClientID\t=", gClientID);
		CONSOLE_DEBUG_W_NUM("gClientTransactionID\t=", gClientTransactionID);

#endif // _DEBUG_CONFORM_


		//*	extract out the command itself for easier processing by the handlers
		cmdBuffLen		=	strlen(reqData->cmdBuffer);
		cc				=	0;
		while (	(reqData->cmdBuffer[cc] > 0x20) &&
				(reqData->cmdBuffer[cc] != '&') &&
				(reqData->cmdBuffer[cc] != '?') &&
				(cc < cmdBuffLen ))
		{
			reqData->deviceCommand[cc]	=	reqData->cmdBuffer[cc];
			cc++;
			reqData->deviceCommand[cc]	=	0;
		}

		//*******************************************
		//*	now do something with the data
		deviceTypeEnum	=	FindDeviceTypeByString(reqData->deviceType);
		deviceFound	=	false;
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
			#ifdef _DEBUG_CONFORM_
			//	CONSOLE_DEBUG_W_NUM("gAlpacaDeviceList[iii]->cDeviceType\t=", gAlpacaDeviceList[iii]->cDeviceType);
			//	CONSOLE_DEBUG_W_NUM("gAlpacaDeviceList[iii]->cDeviceNum\t=", gAlpacaDeviceList[iii]->cDeviceNum);
			#endif // _DEBUG_CONFORM_

				if ((gAlpacaDeviceList[iii]->cDeviceType == deviceTypeEnum) &&
					(gAlpacaDeviceList[iii]->cDeviceNum == reqData->deviceNumber))
				{
					deviceFound		=	true;

					gAlpacaDeviceList[iii]->cBytesWrittenForThisCmd	=	0;
					alpacaErrCode	=	gAlpacaDeviceList[iii]->ProcessCommand(reqData);
					if (alpacaErrCode == kASCOM_Err_Success)
					{
						//*	record the time of the last successful command
						//*	this is for watch dog timing
						gAlpacaDeviceList[iii]->cTimeOfLastValidCmd	=	time(NULL);
					}
					else
					{
						gAlpacaDeviceList[iii]->cTotalCmdErrors++;
					}
					gAlpacaDeviceList[iii]->cTotalCmdsProcessed++;
					gAlpacaDeviceList[iii]->cTotalBytesRcvd	+=	byteCount;

					reqData->alpacaErrCode	=	alpacaErrCode;
					//*	are we conform logging
					if (gConformLogging)
					{
						LogToDisk(kLog_Conform, reqData);
					}
					//*	are we error loggin
					if ((alpacaErrCode != 0) && gErrorLogging)
					{
						LogToDisk(kLog_Error, reqData);
					}
#ifdef _ENABLE_BANDWIDTH_LOGGING_
					//*	this is for network stats
					if (gTimeUnitsSinceTopOfHour < kMaxBandWidthSamples)
					{
						gAlpacaDeviceList[iii]->cBW_CmdsReceived[gTimeUnitsSinceTopOfHour]	+=	1;
						gAlpacaDeviceList[iii]->cBW_BytesReceived[gTimeUnitsSinceTopOfHour]	+=	byteCount;
						gAlpacaDeviceList[iii]->cBW_BytesSent[gTimeUnitsSinceTopOfHour]		+=	gAlpacaDeviceList[iii]->cBytesWrittenForThisCmd;
					}
#endif // _ENABLE_BANDWIDTH_LOGGING_

					break;
				}
			}
		}

		if (deviceFound == false)
		{
			CONSOLE_DEBUG_W_STR("Device not found: Device type\t=", reqData->deviceType);
			SocketWriteData(reqData->socket,	gBadResponse400);
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
		}
	}
	else
	{
#ifdef _DEBUG_CONFORM_
		CONSOLE_DEBUG_W_STR("kASCOM_Err_InvalidValue", parseChrPtr);
#endif // _DEBUG_CONFORM_
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
	}


#ifdef _DEBUG_CONFORM_
	//*	log everything
#else
	if ((alpacaErrCode != 0) && (alpacaErrCode != kASCOM_Err_NotSupported))
#endif // _DEBUG_CONFORM_
	{
	char	myCommandString[256];
	char	*quesMkPtr;

		strcpy(myCommandString, reqData->cmdBuffer);
		quesMkPtr	=	strchr(myCommandString, '?');
		if (quesMkPtr != NULL)
		{
			*quesMkPtr	=	0;
		}

		//*	log the event
		LogEvent(	reqData->deviceType,
					myCommandString,
					NULL,
					alpacaErrCode,
					reqData->alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
static TYPE_ASCOM_STATUS	ProcessManagementRequest(TYPE_GetPutRequestData	*reqData,
													char					*parseChrPtr,
													long					byteCount)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
int					iii;
int					parseDataLen;
int					cmdStrLen;

	if (parseChrPtr != NULL)
	{
		if (strncmp(parseChrPtr, "/management/", 12) == 0)
		{
			parseChrPtr	+=	12;
		}
		if (*parseChrPtr == 'v')
		{
			parseChrPtr++;
			while (*parseChrPtr != '/')
			{
				parseChrPtr++;
			}
		}
		if (*parseChrPtr == '/')
		{
			parseChrPtr++;
		}

		parseDataLen	=	strlen(parseChrPtr);
		cmdStrLen		=	0;
		for (iii=0; iii<parseDataLen; iii++)
		{
			cmdStrLen++;
			if (parseChrPtr[iii] <= 0x20)
			{
				break;
			}
		}
		if (cmdStrLen < kMaxCommandLen)
		{
			strncpy(reqData->deviceCommand, parseChrPtr, cmdStrLen);
			reqData->deviceCommand[cmdStrLen]	=	0;
			StripTrailingSpaces(reqData->deviceCommand);
		}
//		CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=", reqData->deviceCommand);
//		CONSOLE_DEBUG_W_NUM("cmdStrLent=", cmdStrLen);
	}

	for (iii=0; iii<gDeviceCnt; iii++)
	{
		if (gAlpacaDeviceList[iii] != NULL)
		{
			if (gAlpacaDeviceList[iii]->cDeviceType == kDeviceType_Management)
			{
				alpacaErrCode	=	gAlpacaDeviceList[iii]->ProcessCommand(reqData);
				gAlpacaDeviceList[iii]->cTotalCmdsProcessed++;
				if (alpacaErrCode!= kASCOM_Err_Success)
				{
					gAlpacaDeviceList[iii]->cTotalCmdErrors++;
				}
#ifdef _ENABLE_BANDWIDTH_LOGGING_
				//*	this is for network stats
				if (gTimeUnitsSinceTopOfHour < kMaxBandWidthSamples)
				{
					gAlpacaDeviceList[iii]->cBW_CmdsReceived[gTimeUnitsSinceTopOfHour]	+=	1;
					gAlpacaDeviceList[iii]->cBW_BytesReceived[gTimeUnitsSinceTopOfHour]	+=	byteCount;
		//-			gAlpacaDeviceList[iii]->cBW_BytesSent[gTimeUnitsSinceTopOfHour];
				}
#endif // _ENABLE_BANDWIDTH_LOGGING_
				break;
			}
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*
//*	PUT /api/v1/filterwheel/0/connected HTTP/1.1
//*	BinarySerialisation: true
//*	Authorization: Basic Og==
//*	Accept: application/json, text/json, text/x-json, text/javascript, application/xml, text/xml
//*	User-Agent: RestSharp/106.6.9.0
//*	Content-Type: application/x-www-form-urlencoded
//*	Host: 192.168.1.164:6800
//*	Content-Length: 53
//*	Accept-Encoding: gzip, deflate
//*	Connection: Keep-Alive
//*
//*	ClientTransactionID=31&ClientID=18194&Connected=False
//*****************************************************************************
static void	ParseHTMLdataIntoReqStruct(const char *htmlData, TYPE_GetPutRequestData	*reqData)
{
char			*contentLenPtr;
char			lineBuff[1024];
char			firstLine[1024];
unsigned int	iii;
unsigned int	ccc;
unsigned int	sLen;
int				lineCnt;
char			theChar;
bool			isContent;
bool			isFirstLine;
char			*queMrkPtr;

//	CONSOLE_DEBUG(__FUNCTION__);
	if ((htmlData != NULL) && (reqData != NULL))
	{
		if (strncasecmp(htmlData, "GET", 3) == 0)
		{
			reqData->get_putIndicator	=	'G';
		}
		else 	if (strncasecmp(htmlData, "PUT", 3) == 0)
		{
			reqData->get_putIndicator	=	'P';
		}
		else
		{
			CONSOLE_DEBUG("GET/PUT NOT SPECIFIED!!!!!!!");
			reqData->get_putIndicator	=	'?';
		}

		//*	extract the HTTP command
		ccc	=	0;
		while ((htmlData[ccc] >= 0x20) && (ccc < (kHTTPbufLen - 2)))
		{
			reqData->httpCmdString[ccc]	=	htmlData[ccc];
			ccc++;
		}
		reqData->httpCmdString[ccc]	=	0;

		sLen		=	strlen(htmlData);
//		CONSOLE_DEBUG_W_NUM("htmlData length\t=", sLen);
//		CONSOLE_DEBUG_W_NUM("sizeof(lineBuff)\t=", sizeof(lineBuff));


		//*	keep a copy of the entire thing
		strcpy(reqData->htmlData, htmlData);


		//*	go through the entire data and treat them as separate lines of text
		lineCnt		=	0;
		isContent	=	false;
		ccc			=	0;
		theChar		=	0;
		iii			=	0;
		isFirstLine	=	true;
		while (iii <= sLen)
		{
			theChar		=	htmlData[iii];
			if ((theChar >= 0x20) || (theChar == 0x09))
			{
				if (ccc < sizeof(lineBuff))
				{
					lineBuff[ccc]	=	theChar;
					ccc++;
				}
			}
			else
			{
				lineCnt++;
				lineBuff[ccc]	=	0;
				ccc				=	0;
//				CONSOLE_DEBUG_W_STR("lineBuff\t=", lineBuff);
				if (isFirstLine)
				{
					strcpy(firstLine, lineBuff);
					isFirstLine	=	false;
				}
				//*	now lets see if this is anything we care about
				if (strlen(lineBuff) > 0)
				{
					if (isContent)
					{
						strcat(reqData->contentData, lineBuff);
						strcat(reqData->contentData, "\r\n");
					}
					else
					{
						if (strncasecmp(lineBuff, "Content-Length", 14) == 0)
						{
							contentLenPtr	=	lineBuff;
							contentLenPtr	+=	15;
							while (*contentLenPtr == 0x20)
							{
								contentLenPtr++;
							}
							reqData->contentLength	=	atoi(contentLenPtr);
					#ifdef _DEBUG_CONFORM_
							CONSOLE_DEBUG("Content-Length: was found");
					#endif // _DEBUG_CONFORM_

						}
					}
				}
				else
				{
					isContent	=	true;
				}

				//*	check the next char, make sure we dont process CR and LF
				if ((theChar == 0x0d) && (htmlData[iii+1] == 0x0a))
				{
					//*	skip the line feed
					iii++;
				}
				else if ((theChar == 0x0a) && (htmlData[iii+1] == 0x0d))
				{
					//*	skip the cr
					iii++;
				}
			}

			iii++;

		}
		if (reqData->get_putIndicator == 'G')
		{
			//*	the get data is in a different location
			queMrkPtr	=	strchr(firstLine, '?');
			if (queMrkPtr != NULL)
			{
				queMrkPtr++;
				strcpy(reqData->contentData, queMrkPtr);
			}
		}
	#ifdef _DEBUG_CONFORM_
		CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
	#endif // _DEBUG_CONFORM_
	}
	else
	{
		CONSOLE_DEBUG("args are NULL");
	}
}

//*****************************************************************************
static int	ProcessGetPutRequest(const int socket, char *htmlData, long byteCount)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_InternalError;
char					*parseChrPtr;
TYPE_GetPutRequestData	reqData;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG("=========================================================================================");
	CONSOLE_DEBUG("GET/PUT");
	CONSOLE_DEBUG_W_STR("htmlData\t=", htmlData);
#endif // _DEBUG_CONFORM_

#ifdef _ENABLE_BANDWIDTH_LOGGING_
int	previousUnitsSinceTopOfHour;
int	iii;

	previousUnitsSinceTopOfHour	=	gTimeUnitsSinceTopOfHour;
	gTimeUnitsSinceTopOfHour	=	(time(NULL) / 60) % kMaxBandWidthSamples;
//	CONSOLE_DEBUG_W_NUM("gTimeUnitsSinceTopOfHour\t=", gTimeUnitsSinceTopOfHour);
	//*	check to see if it changed
	if (gTimeUnitsSinceTopOfHour != previousUnitsSinceTopOfHour)
	{
		//*	reset all of the counters to zero
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
				gAlpacaDeviceList[iii]->cBW_CmdsReceived[gTimeUnitsSinceTopOfHour]	=	0;
				gAlpacaDeviceList[iii]->cBW_BytesReceived[gTimeUnitsSinceTopOfHour]	=	0;
				gAlpacaDeviceList[iii]->cBW_BytesSent[gTimeUnitsSinceTopOfHour]		=	0;
			}
		}
	}
#endif // _ENABLE_BANDWIDTH_LOGGING_
	memset(&reqData, 0, sizeof(TYPE_GetPutRequestData));
	//*	the TYPE_GetPutRequestData simplifies parsing and passing of the
	//*	parsed data to subroutines
	reqData.socket				=	socket;
	reqData.get_putIndicator	=	htmlData[0];
//	DumpRequestStructure(__FUNCTION__, &reqData);

	ParseHTMLdataIntoReqStruct(htmlData, &reqData);
//	CONSOLE_DEBUG(reqData.httpCmdString);
//	if (strstr(htmlData, "connected") != NULL)
//	{
//		CONSOLE_DEBUG(htmlData);
//	}
	parseChrPtr			=	(char *)htmlData;
	parseChrPtr			+=	3;
	while (*parseChrPtr == 0x20)
	{
		parseChrPtr++;
	}

#ifdef _DEBUG_CONFORM_
//	CONSOLE_DEBUG_W_STR("Alpaca data=", parseChrPtr);
#endif // _DEBUG_CONFORM_

	//-------------------------------------------------------------------
	//*	standard ALPACA api call
	if (strncmp(parseChrPtr, "/api/", 5) == 0)
	{
		//*	if we are here, we are guaranteed to have either GET or PUT, so no need to check
		alpacaErrCode	=	ProcessAlpacaAPIrequest(&reqData, parseChrPtr, byteCount);
	}
	//-------------------------------------------------------------------
	//*	standard ALPACA setup
	else if ((strncmp(parseChrPtr, "/setup", 6) == 0) || (strncmp(parseChrPtr, "/web", 4) == 0))
	{
		SendHtmlResponse(&reqData);
	}
	//-------------------------------------------------------------------
	//*	standard ALPACA management
	else if (strncmp(parseChrPtr, "/management", 11) == 0)
	{
		alpacaErrCode	=	ProcessManagementRequest(&reqData, parseChrPtr, byteCount);
	}
	//-------------------------------------------------------------------
	//*	extra log interface
	else if (strncmp(parseChrPtr, "/log", 4) == 0)
	{
		SendHtmlLog(socket);
	}
	//-------------------------------------------------------------------
	//*	Stats interface
	else if (strncmp(parseChrPtr, "/stats", 6) == 0)
	{
		SendHtmlStats(&reqData);
	}
	//-------------------------------------------------------------------
	else if (strncmp(parseChrPtr, "/favicon.ico", 12) == 0)
	{
		//*	do nothing, this is my web browser sends this
//		CONSOLE_DEBUG("Ignored");
	}
	//-------------------------------------------------------------------
	else if (strncmp(parseChrPtr, "/image.jpg", 10) == 0)
	{
		CONSOLE_DEBUG("image.jpg");
		SendJpegResponse(socket, NULL);
	}
	//-------------------------------------------------------------------
	else if (strstr(parseChrPtr, ".jpg") != NULL)
	{
		CONSOLE_DEBUG(".....jpg");
		SendJpegResponse(socket, parseChrPtr);
	}
	//-------------------------------------------------------------------
	else if (strstr(parseChrPtr, ".png") != NULL)
	{
		SendJpegResponse(socket, parseChrPtr);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Incomplete alpaca command\t=",	htmlData);
		SocketWriteData(socket,	gBadResponse400);
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(alpacaErrCode);
}

//*****************************************************************************
//*	this function is called from the socket handler with the received data
//*	It will parse through the data checking all of the normal alpaca requirements
//*	and then call the appropriate function based on the device type
//*****************************************************************************
static int AlpacaCallback(const int socket, char *htmlData, long byteCount)
{
int		returnCode	=	-1;

//	CONSOLE_DEBUG("Timing Start----------------------");
//	SETUP_TIMING();

	//*	we are looking for GET or PUT
	if (strncmp(htmlData, "GET /favicon.ico", 16) == 0)
	{
		//*	do nothing
	}
	else if ((strncmp(htmlData, "GET", 3) == 0) || (strncmp(htmlData, "PUT", 3) == 0))
	{
//		CONSOLE_DEBUG("Calling ProcessGetPutRequest");
		returnCode	=	ProcessGetPutRequest(socket, htmlData, byteCount);
	}
	else if (byteCount > 0)
	{
		CONSOLE_DEBUG_W_STR("Invalid HTML get/put command\t=",	htmlData);
		CONSOLE_DEBUG_W_LONG("byteCount\t=",	byteCount);
	}

	gServerTransactionID++;	//*	we are the "server"

//	DEBUG_TIMING(__FUNCTION__);

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(returnCode);
}


//*****************************************************************************
static void	*ListenThread(void *arg)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SocketListen_SetCallback(&AlpacaCallback);

	SocketListen_Init(gAlpacaListenPort);

	while (1)
	{
		SocketListen_Poll();
	}
	return(NULL);
}


//*****************************************************************************
static void	PrintHelp(const char *appName)
{
	printf("usage: %s [-<option>]\r\n", appName);
	printf("\t%-12s\t%s\r\n",	"a",			"Auto exposure");
	printf("\t%-12s\t%s\r\n",	"c",			"Conform logging, log ALL commands to disk");
	printf("\t%-12s\t%s\r\n",	"d",			"Display images as they are taken");
	printf("\t%-12s\t%s\r\n",	"e",			"Error logging, log errors commands to disk");
	printf("\t%-12s\t%s\r\n",	"h",			"This help message");
	printf("\t%-12s\t%s\r\n",	"l",			"Live mode");
	printf("\t%-12s\t%s\r\n",	"p <port>",		"what port to use (default 6800)");
	printf("\t%-12s\t%s\r\n",	"q",			"quiet (less console messages)");
	printf("\t%-12s\t%s\r\n",	"s",			"Simulate camera image (ATIK, QHY and QSI only at present)");
	printf("\t%-12s\t%s\r\n",	"t <profile>",	"Which telescope profile to use");
	printf("\t%-12s\t%s\r\n",	"v",			"verbose (more console messages default)");
}

//*****************************************************************************
static void	ProcessCmdLineArgs(int argc, char **argv)
{
int		ii;
char	theChar;
int		newListenPort;

	for (ii=1; ii<argc; ii++)
	{
		if (argv[ii][0] == '-')
		{
			theChar	=	argv[ii][1];
			switch(theChar)
			{
				//	"-a" means auto adjust exposure
				case 'a':
					gAutoExposure	=	true;
					break;

				//	-c means Conform logging
				case 'c':
					gConformLogging	=	true;	//*	log all commands to log file to match up with Conform
					break;

				//	-e means Error logging
				case 'e':
					gErrorLogging	=	true;	//*	write errors to log file if true
					break;

				//	"-d" means display image after download from camera
				case 'd':
					gDisplayImage	=	true;
					break;

				//	"-h" means print help
				case 'h':
					PrintHelp(argv[0]);
					exit(0);	//*	help message
					break;

				//	"-l" means live view
				case 'l':
				#ifdef _USE_OPENCV_
					gLiveView	=	true;
				#else
					CONSOLE_DEBUG("Cannot do live mode without opencv");
				#endif
					break;

				//	-p specifies a port
				case 'p':
					ii++;
					newListenPort		=	atoi(argv[ii]);
					if ((newListenPort > 1024) && (newListenPort <= 65535))
					{
						gAlpacaListenPort	=	newListenPort;
					}
					else
					{
						CONSOLE_DEBUG("Invalid listen port specified");
						CONSOLE_ABORT(__FUNCTION__);
					}
					break;

				//	"-q" means quiet
				case 'q':
					gVerbose	=	false;
					break;

				//	"-s" means Simulate image
				case 's':
					gSimulateCameraImage	=	true;
					break;

				//*	"-t" means which telescope profile to use
				//*	either -tNEWT16 or -t Newt16
				case 't':
//					CONSOLE_DEBUG_W_STR("argv[ii]\t=", argv[ii]);
					if (strlen(argv[ii]) > 2)
					{
						strncpy(gDefaultTelescopeRefID, &argv[ii][2], (kDefaultRefIdMaxLen - 2));
						gDefaultTelescopeRefID[kDefaultRefIdMaxLen - 1]	=	0;
					}
					else if (argc > (ii+1))
					{
						ii++;
						strncpy(gDefaultTelescopeRefID, argv[ii], (kDefaultRefIdMaxLen - 2));
						gDefaultTelescopeRefID[kDefaultRefIdMaxLen - 1]	=	0;
					}
//					CONSOLE_DEBUG_W_STR("gDefaultTelescopeRefID\t=", gDefaultTelescopeRefID);
					break;

				//	"-v" means verbose
				case 'v':
					gVerbose	=	true;
					break;
			}
		}
	}
}

//*****************************************************************************
static void	GetMyHostName(void)
{
FILE		*filePointer;
char		*strPtr;
int			sLen;
int			iii;

	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen("hostname.txt", "r");
	if (filePointer != NULL)
	{
		strPtr	=	fgets(gHostName, sizeof(gHostName), filePointer);
		if (strPtr == NULL)
		{
			CONSOLE_DEBUG("fgets returned error");
		}
		sLen	=	strlen(gHostName);
		if (sLen > 0)
		{
			for (iii=0; iii<sLen; iii++)
			{
				if (gHostName[iii] < 0x20)
				{
					gHostName[iii]	=	0;
					break;
				}
			}
		}

		fclose(filePointer);
	}
}

//*****************************************************************************
//*	this should be over-ridden if needed
//*****************************************************************************
void	AlpacaDriver::WatchDog_TimeOut(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cCommonProp.Name);
}


//*****************************************************************************
//*	this checks to see how long ago a VALID command was sent to the driver
//*	if the time is greater than the specified time (in Minutes),
//*	then WatchDog_TimeOut() will be called.
//*	It is up to WatchDog_TimeOut() to do what is needed to shut this device down.
//*
//*	For example:
//*		Dome:		Close the shutter
//*		Telescope:	Stop tracking
//*****************************************************************************
void	AlpacaDriver::CheckWatchDogTimeout(void)
{
time_t		currentTimeEpoch;
time_t		deltaSeconds;

//	CONSOLE_DEBUG(__FUNCTION__);
	//*	we only want to check this once a minute
	currentTimeEpoch	=	time(NULL);
	deltaSeconds		=	currentTimeEpoch - cTimeOfLastWatchDogCheck;

	if (deltaSeconds >= 60)
	{
		//*	now check to see how long it has been since a valid command
		deltaSeconds		=	currentTimeEpoch - cTimeOfLastValidCmd;
		if (deltaSeconds > (cWatchDogTimeOut_Minutes * 60))
		{
			WatchDog_TimeOut();
		}

		cTimeOfLastWatchDogCheck	=	time(NULL);
	}
}

//*****************************************************************************
void	AlpacaDriver::DumpCommonProperties(const char *callingFunctionName)
{
char	titleLine[128];


	CONSOLE_DEBUG(		"*************************************************************");
	sprintf(titleLine,	"******************** Alpaca device properties ***************");
	CONSOLE_DEBUG(titleLine);
	sprintf(titleLine,	"************* Called from: %-20s *************", callingFunctionName);
	CONSOLE_DEBUG(titleLine);
	CONSOLE_DEBUG(		"*************************************************************");


	CONSOLE_DEBUG_W_NUM(	"cDeviceType                    \t=",	cDeviceType);
	CONSOLE_DEBUG_W_NUM(	"cDeviceNum                     \t=",	cDeviceNum);
	CONSOLE_DEBUG_W_BOOL(	"cCommonProp.Connected          \t=",	cCommonProp.Connected);
	CONSOLE_DEBUG_W_STR(	"cCommonProp.Description        \t=",	cCommonProp.Description);
	CONSOLE_DEBUG_W_STR(	"cCommonProp.DriverInfo         \t=",	cCommonProp.DriverInfo);
	CONSOLE_DEBUG_W_STR(	"cCommonProp.DriverVersion      \t=",	cCommonProp.DriverVersion);
	CONSOLE_DEBUG_W_NUM(	"cCommonProp.InterfaceVersion   \t=",	cCommonProp.InterfaceVersion);
	CONSOLE_DEBUG_W_STR(	"cCommonProp.Name               \t=",	cCommonProp.Name);
}

//*****************************************************************************
static void	CreateDriverObjects()
{
//*********************************************************
//*	Cameras
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_ATIK_)
	CreateATIK_CameraObjects();
#endif

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_ASI_)
	CreateASI_CameraObjects();
#endif
//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_TOUP_)
	CreateTOUP_CameraObjects();
#endif
//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)
	CreateQHY_CameraObjects();
#endif
//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QSI_)
	CreateQSI_CameraObjects();
#endif
//-----------------------------------------------------------
//#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FLIR_) && (__GNUC__ > 5)
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FLIR_)
	CreateFLIR_CameraObjects();
#endif


//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_SONY_)
	CreateSONY_CameraObjects();
#endif

#if defined(_ENABLE_CAMERA_) && defined(_SIMULATE_CAMERA_)
	CreateSimulator_CameraObjects();
#endif

//*********************************************************
//*	Multicam
#ifdef _ENABLE_MULTICAM_
	cameraCnt	=	CountDevicesByType(kDeviceType_Camera);
	if (cameraCnt > 1)
	{
		CreateMultiCamObject();
	}
#endif

//*********************************************************
//*	Focuser
#ifdef _ENABLE_FOCUSER_
	CreateFocuserNiteCrawlerObjects();
#endif

//*********************************************************
//*	Filter wheel
#ifdef _ENABLE_FILTERWHEEL_ZWO_
	CreateZWOFilterWheelObjects();
#endif
#ifdef _ENABLE_FILTERWHEEL_ATIK_
	CreateATIKFilterWheelObjects();
#endif

//*********************************************************
//*	Dome
#ifdef _ENABLE_DOME_
	CreateDomeObjectsRPi();
#endif
//*********************************************************
//*	Shutter
#ifdef _ENABLE_SHUTTER_
//	CreateShutterObjects();
	CreateShuterArduinoObjects();
#endif
#ifdef	_ENABLE_ROR_
	CreateDomeObjectsROR();
#endif // _ENABLE_ROR_
//*********************************************************
//*	Switch
#if defined(_ENABLE_SWITCH_) && defined(__arm__)
	CreateSwitchObjectsRPi();
#endif	//	_ENABLE_SWITCH_

//*********************************************************
//*	Observing conditions
#if defined(_ENABLE_OBSERVINGCONDITIONS_) && defined(__arm__)
	CreateObsConditionRpiObjects();
#elif defined(_ENABLE_OBSERVINGCONDITIONS_)
//	CreateObsConditionObjects();
#endif


#ifdef _ENABLE_CALIBRATION_
//	CreateCalibrationObjects();
	CreateCalibrationObjectsRPi();
#endif // _ENABLE_CALIBRATION_

//*********************************************************
//*	Telescope
#ifdef _ENABLE_TELESCOPE_
	CreateTelescopeObjects();
#endif // _ENABLE_TELESCOPE_


//*********************************************************
//*	Slit tacker
#ifdef _ENABLE_SLIT_TRACKER_
	CreateSlitTrackerObjects();
#endif // _ENABLE_SLIT_TRACKER_


	//*********************************************************
	//*	Management
	CreateManagementObject();


}


static	int32_t	gMainLoopCntr	=	0;
//*****************************************************************************
int	main(int argc, char **argv)
{
pthread_t		threadID;
int				threadErr;
uint32_t		delayTime_microSecs;
uint32_t		delayTimeForThisTask;
int				iii;
int				cameraCnt;
int				ram_Megabytes;
double			freeDiskSpace_Gigs;

#if defined(_ENABLE_FITS_) || defined(_ENABLE_JPEGLIB_)
	char			lineBuffer[64];
#endif
//struct rlimit	myRlimit;
//int				errorCode;

	printf("AlpacaPi driver\r\n");
	sprintf(gFullVersionString, "%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(gFullVersionString);
//	CONSOLE_DEBUG_W_STR("gcc version:", __VERSION__);

	AddLibraryVersion("software", "gcc", __VERSION__);
	AddLibraryVersion("software", "libc", gnu_get_libc_version());

	ram_Megabytes	=	CPUstats_GetTotalRam();
	CONSOLE_DEBUG_W_NUM("totalRam_Megabytes\t=", ram_Megabytes);

	ram_Megabytes	=	CPUstats_GetFreeRam();
	CONSOLE_DEBUG_W_NUM("freeRam_Megabytes\t=", ram_Megabytes);

	freeDiskSpace_Gigs	=	CPUstats_GetFreeDiskSpace("/") / 1024.0;
	CONSOLE_DEBUG_W_DBL("freeDiskSpace_Gigs\t=", freeDiskSpace_Gigs);

	GetMyHostName();


#ifdef _ENABLE_FITS_
	//*	cfitsio version
	sprintf(lineBuffer,	"%d.%d", CFITSIO_MAJOR, CFITSIO_MINOR);
	AddLibraryVersion("software", "cfitsio", lineBuffer);
	CONSOLE_DEBUG_W_STR("cfitsio version\t=", lineBuffer);
#endif // _ENABLE_FITS_

#ifdef _USE_OPENCV_
	//*	openCV version
	AddLibraryVersion("software", "opencv", CV_VERSION);
	CONSOLE_DEBUG_W_STR("opencv version\t=", CV_VERSION);
#endif

#ifdef _ENABLE_JPEGLIB_
	//*	jpeg lib version
	sprintf(lineBuffer,	"%d", JPEG_LIB_VERSION);
	AddLibraryVersion("software", "libjpeg", lineBuffer);
	CONSOLE_DEBUG_W_STR("libjpeg version\t=", lineBuffer);
#endif

	//*	check resoure limits
//	errorCode	=	getrlimit(RLIMIT_NOFILE, &myRlimit);
//	CONSOLE_DEBUG_W_NUM("getrlimit returned errorCode\t=",	errorCode);
//	CONSOLE_DEBUG_W_NUM("myRlimit.rlim_cur\t=",	myRlimit.rlim_cur);
//	CONSOLE_DEBUG_W_NUM("myRlimit.rlim_max\t=",	myRlimit.rlim_max);

	strcpy(gDefaultTelescopeRefID,	"");
	strcpy(gPlatformString,			"");

	strcpy(gWebTitle, "Alpaca Driver");

	CPUstats_ReadOSreleaseVersion();
	CPUstats_ReadInfo();

	CONSOLE_DEBUG_W_STR("CPU info  \t=",	gCpuInfoString);
	CONSOLE_DEBUG_W_STR("OS Release\t=",	gOsReleaseString);
	CONSOLE_DEBUG_W_STR("Platform  \t=",	gPlatformString);


	InitObsConditionGloblas();
	ProcessCmdLineArgs(argc, argv);

//	CONSOLE_DEBUG_W_INT32("sizeof(int)\t=",		(long)sizeof(int));
//	CONSOLE_DEBUG_W_INT32("sizeof(long)\t=",	(long)sizeof(long));

	InitDeviceList();

	LogEvent(	"AlpacaPi",
				NULL,
				NULL,
				kASCOM_Err_Success,
				"System Started");

	LogEvent(	"AlpacaPi",
				NULL,
				NULL,
				kASCOM_Err_Success,
				gFullVersionString);

	ObservatorySettings_Init();
	gObservatorySettingsOK	=	ObservatorySettings_ReadFile();


	CreateDriverObjects();


	//*********************************************************
	StartDiscoveryListenThread(gAlpacaListenPort);
#ifdef _JETSON_
	StartExtraListenThread(4520);
#endif

//#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FITS_) && defined(_ENABLE_DISCOVERY_QUERRY_)
#if defined(_ENABLE_CAMERA_)
	cameraCnt	=	CountDevicesByType(kDeviceType_Camera);
	CONSOLE_DEBUG_W_NUM("cameraCnt=", cameraCnt);
	//*	for now, we dont need this on all devices
	if (cameraCnt > 0)
	{
//		StartDiscoveryQuerryThread();
	}
#endif


	threadErr	=	pthread_create(&threadID, NULL, &ListenThread, NULL);
	if (threadErr != 0)
	{
		CONSOLE_DEBUG_W_NUM("threadErr=", threadErr);
	}

#ifdef _ENABLE_IMU_
	IMU_Init();
#endif

	//========================================================================================
	gKeepRunning	=	true;
	while (gKeepRunning)
	{
		gMainLoopCntr++;
		delayTime_microSecs	=	(1000000 / 2);		//*	default to 1/2 second
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			//==================================================================================
			//*	Run state machines for enabled device.
			//*	Not all devices have state machines to run
			if (gAlpacaDeviceList[iii] != NULL)
			{
				delayTimeForThisTask	=	gAlpacaDeviceList[iii]->RunStateMachine();

				//*	figure out what is the minimum delay time we have
				if (delayTimeForThisTask < delayTime_microSecs)
				{
					delayTime_microSecs	=	delayTimeForThisTask;
				}

			#ifdef _ENABLE_LIVE_CONTROLLER_
				//==================================================================================
				//*	live window
				if (gAlpacaDeviceList[iii]->cLiveController != NULL)
				{
					HandleContollerWindow(gAlpacaDeviceList[iii]);

					//*	if we have an active live window,
					//*	we want to be able to give it more time by waiting less time
					delayTime_microSecs	=	10;
				}
			#endif // _ENABLE_LIVE_CONTROLLER_

				gAlpacaDeviceList[iii]->CheckWatchDogTimeout();
			}
		}
		if (delayTime_microSecs < 10)
		{
			delayTime_microSecs	=	10;
		}
//		CONSOLE_DEBUG_W_INT32("delayTime_microSecs\t=", delayTime_microSecs);
		usleep(delayTime_microSecs);
	}


	//*	the program has been told to quit, go through and delete the objects
	for (iii=0; iii<kMaxDevices; iii++)
	{
		if (gAlpacaDeviceList[iii] != NULL)
		{
			CONSOLE_DEBUG_W_STR("Deleting ", gAlpacaDeviceList[iii]->cCommonProp.Name);
			delete gAlpacaDeviceList[iii];
		}
	}

	return(0);
}


#ifdef _ENABLE_LIVE_CONTROLLER_
//*****************************************************************************
static	void HandleContollerWindow(AlpacaDriver *alpacaObjPtr)
{
Controller	*myController;
int			keyPressed;

#ifdef _USE_OPENCV_
#warning "Under test 4/11/2022"
	//ProcessControllerWindows();
#if (CV_MAJOR_VERSION >= 3)
	keyPressed	=	cv::waitKeyEx(5);
#else
	keyPressed	=	cvWaitKey(5);
#endif
	if (keyPressed > 0)
	{
		Controller_HandleKeyDown(keyPressed);
	}

	myController	=	alpacaObjPtr->cLiveController;
	if (myController != NULL)
	{
		myController->HandleWindow();
//		cv::waitKey(100);

		if (myController->cKeepRunning == false)
		{
			CONSOLE_DEBUG(__FUNCTION__);
			delete myController;


			alpacaObjPtr->cLiveController	=	NULL;

			CONSOLE_DEBUG(__FUNCTION__);
		}
	}
#endif

}
#endif // _ENABLE_LIVE_CONTROLLER_

#pragma mark -
#pragma mark Helper functions

static uint32_t	gSystemStartSecs = 0;


//*****************************************************************************
uint32_t	Calc_millisFromTimeStruct(struct timeval *theTimeStruct)
{
uint32_t	elapsedSecs;
uint32_t	milliSecs;

	elapsedSecs	=	theTimeStruct->tv_sec - gSystemStartSecs;
	milliSecs	=	(elapsedSecs * 1000) + (theTimeStruct->tv_usec / 1000);
	return(milliSecs);
}



//*****************************************************************************
//*	returns -1 if not found
//*****************************************************************************
int	FindCmdFromTable(const char *theCmd, const TYPE_CmdEntry *theCmdTable, int *cmdType)
{
int		iii;
int		cmdEnumValue;

	cmdEnumValue	=	-1;
	iii				=	0;
	while ((theCmdTable[iii].commandName[0] != 0) && (cmdEnumValue < 0))
	{
		if (strcasecmp(theCmd, theCmdTable[iii].commandName) == 0)
		{
			cmdEnumValue	=	theCmdTable[iii].enumValue;
		}
		iii++;
	}

	//*	if we haven't found the command, look it up in the common table
	if (cmdEnumValue < 0)
	{
		iii				=	0;
		while ((gCommonCmdTable[iii].commandName[0] != 0) && (cmdEnumValue < 0))
		{
			if (strcasecmp(theCmd, gCommonCmdTable[iii].commandName) == 0)
			{
				cmdEnumValue	=	gCommonCmdTable[iii].enumValue;
			}
			iii++;
		}
	}
	return(cmdEnumValue);
}



//*****************************************************************************
//*	This finds the unique keyword in the data string.
//*	the keyword must be terminated with a "=" in order to return
//*	a valid argument.
//*	The method is a little slow but it insures non-ambiguity.
//*	For example, "Duration=" and "Duration1="
//*
//*	argIsNumeric should be set to kArgumentIsNumeric/TRUE
//*		IF the argument is a floating point number,
//*		this allows for European strings with commas instead of periods
//*****************************************************************************
bool	GetKeyWordArgument(	const char	*dataSource,
							const char	*keyword,
							char		*argument,
							const int	maxArgLen,
							const bool	argIsNumeric)
{
int		dataSrcLen;
int		iii;
int		jjj;
bool	foundKeyWord;
char	myKeyWord[256];
char	myArgString[256];
int		myArgLength;
int		ccc;
char	theChar;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("dataSource\t=", dataSource);
	CONSOLE_DEBUG_W_STR("keyword\t=", keyword);
#endif // _DEBUG_CONFORM_


	foundKeyWord	=	false;
	if ((dataSource != NULL) && (keyword != NULL) && (argument != NULL))
	{
		//*	this steps through the string looking for keywords
		//*	Once the keyword is found, it MUST be followed by an "="
		argument[0]	=	0;
		dataSrcLen	=	strlen(dataSource);
		iii			=	0;
		ccc			=	0;
		while ((foundKeyWord == false) && (iii <= dataSrcLen))
		{
			theChar	=	dataSource[iii];
//-			CONSOLE_DEBUG_W_HEX("theChar\t=", theChar);
			if ((theChar == '=') || (theChar == '&') || (theChar < 0x20))
			{
				//*	we have a keyword, lets see what it is
				myKeyWord[ccc]		=	0;

				//*	now extract the argument
				if (dataSource[iii] == '=')
				{
					iii			+=	1;			//*	skip the "="
				}
				jjj				=	0;
				myArgString[0]	=	0;
				//*	leave room for the null termination
				while ((dataSource[iii] >= 0x20) && (dataSource[iii] != '&') && (jjj < (maxArgLen - 2)))
				{
					myArgString[jjj]	=	dataSource[iii];
					myArgString[jjj+1]	=	0;
					iii++;
					jjj++;
				}
			#ifdef _DEBUG_CONFORM_
				CONSOLE_DEBUG_W_STR("myKeyWord\t\t=", myKeyWord);
				CONSOLE_DEBUG_W_STR("myArgString\t=", myArgString);
			#endif // _DEBUG_CONFORM_

				if (strcasecmp(myKeyWord, keyword) == 0)
				{
					foundKeyWord	=	true;
					//==================================================================
					//*	in order to handle the comma char as a decimal point for Europe
					if (argIsNumeric)
					{
						myArgLength	=	strlen(myArgString);
						for (jjj=0; jjj<myArgLength; jjj++)
						{
							//*	check for comma
							if (myArgString[jjj] == ',')
							{
								myArgString[jjj]	=	'.';	//*	replace with period
							}
						}
					}
					//==================================================================
//					CONSOLE_DEBUG_W_NUM("maxArgLen\t=", maxArgLen);
//					CONSOLE_DEBUG_W_STR("myArgString\t=", myArgString);
					strcpy(argument, myArgString);
				}
				ccc	=	0;
			}
			else
			{
				myKeyWord[ccc]		=	theChar;
				myKeyWord[ccc+1]	=	0;
				ccc++;
			}
			iii++;
		}

	}
	else
	{
		CONSOLE_DEBUG("1 of the 3 arguments is null");
		foundKeyWord	=	false;
	}
#ifdef _DEBUG_CONFORM_
//	if (foundKeyWord)
//	{
//		CONSOLE_DEBUG("We found what we are looking for");
//		CONSOLE_DEBUG_W_STR("myKeyWord\t\t=", myKeyWord);
//		CONSOLE_DEBUG_W_STR("argument\t\t=", argument);
//	}
#endif // _DEBUG_CONFORM_
	return(foundKeyWord);
}

//**************************************************************************************
//*	Count devices by type
//**************************************************************************************
int	CountDevicesByType(const int deviceType)
{
int		deviceCount;
int		ii;

	deviceCount	=	0;
	for (ii=0; ii<gDeviceCnt; ii++)
	{
		if (gAlpacaDeviceList[ii] != NULL)
		{
			if (gAlpacaDeviceList[ii]->cDeviceType == deviceType)
			{
				deviceCount++;
			}
		}
	}
	return(deviceCount);
}


//**************************************************************************************
//*	returns the number of active filter wheels.
int	GetFilterWheelCnt(void)
{
int		filterWhlCnt;

	filterWhlCnt	=	CountDevicesByType(kDeviceType_Filterwheel);
	return(filterWhlCnt);
}


//*****************************************************************************
AlpacaDriver	*FindDeviceByType(const int deviceType)
{
int				ii;
AlpacaDriver	*devicePtr;

	devicePtr	=	NULL;
	for (ii=0; ii<gDeviceCnt; ii++)
	{
		if (gAlpacaDeviceList[ii] != NULL)
		{
			//*	make sure the object is valid
			if (gAlpacaDeviceList[ii]->cMagicCookie == kMagicCookieValue)
			{
				if (gAlpacaDeviceList[ii]->cDeviceType == deviceType)
				{
					devicePtr	=	gAlpacaDeviceList[ii];
				}
			}
		}
	}
	return(devicePtr);

}

//*****************************************************************************
void	ToLowerStr(char *theString)
{
int		ii;

	ii	=	0;
	while (theString[ii] > 0)
	{
		theString[ii]	=	tolower(theString[ii]);
		ii++;
	}
}

//*****************************************************************************
void	StripTrailingSpaces(char *theString)
{
int		ii;
int		sLen;

	sLen	=	strlen(theString);
	ii		=	sLen - 1;
	while (ii > 0)
	{
		if (theString[ii] <= 0x20)
		{
			theString[ii]	=	0;
		}
		else
		{
			break;
		}
		ii--;
	}
}


//**************************************************************************************
bool	Check_udev_rulesFile(const char *rulesFileName)
{
char		rulesFilePath[256];
FILE		*filePointer;
bool		rulesFileOK;

//	/lib/udev/rules.d/99-atik.rules

	rulesFileOK	=	false;

	strcpy(rulesFilePath, "/lib/udev/rules.d/");
	strcat(rulesFilePath, rulesFileName);
	filePointer	=	fopen(rulesFilePath, "r");
	if (filePointer != NULL)
	{
		rulesFileOK	=	true;

		fclose(filePointer);
	}

	//*	if we didnt find it, look in /etc/udev/rules.d/
	if (rulesFileOK == false)
	{
		strcpy(rulesFilePath, "/etc/udev/rules.d/");
		strcat(rulesFilePath, rulesFileName);

		filePointer	=	fopen(rulesFilePath, "r");
		if (filePointer != NULL)
		{
			rulesFileOK	=	true;

			fclose(filePointer);
		}
	}

	//*	if we still didnt find it and the name does not start with "99", check that
	if (rulesFileOK == false)
	{
		if (rulesFileName[0] != '9')
		{
			strcpy(rulesFilePath, "/lib/udev/rules.d/");
			strcat(rulesFilePath, "99-");
			strcat(rulesFilePath, rulesFileName);
			filePointer	=	fopen(rulesFilePath, "r");
			if (filePointer != NULL)
			{
				rulesFileOK	=	true;

				fclose(filePointer);
			}
		}
	}
	return(rulesFileOK);
}

//**************************************************************************
void			GetAlpacaName(TYPE_DEVICETYPE deviceType, char *alpacaName)
{
	switch(deviceType)
	{
		case kDeviceType_Camera:				strcpy(alpacaName, "Camera");			break;
		case kDeviceType_CoverCalibrator:		strcpy(alpacaName, "CoverCalibrator");	break;
		case kDeviceType_Dome:					strcpy(alpacaName, "Dome");				break;
		case kDeviceType_Filterwheel:			strcpy(alpacaName, "Filterwheel");		break;
		case kDeviceType_Focuser:				strcpy(alpacaName, "Focuser");			break;
		case kDeviceType_Management:			strcpy(alpacaName, "Management");		break;
		case kDeviceType_Observingconditions:	strcpy(alpacaName, "Observingconditions");	break;
		case kDeviceType_Rotator:				strcpy(alpacaName, "Rotator");			break;
		case kDeviceType_SafetyMonitor:			strcpy(alpacaName, "SafetyMonitor");	break;
		case kDeviceType_Switch:				strcpy(alpacaName, "Switch");			break;
		case kDeviceType_Telescope:				strcpy(alpacaName, "Telescope");		break;

		//*	extras - non Alpaca standard
		case kDeviceType_Multicam:				strcpy(alpacaName, "Multicam");			break;
//		case kDeviceType_RemoteDiscovery:		strcpy(alpacaName, "RemoteDiscovery");	break;
		case kDeviceType_Shutter:				strcpy(alpacaName, "Shutter");			break;
		case kDeviceType_SlitTracker:			strcpy(alpacaName, "SlitTracker");		break;
		default:								strcpy(alpacaName, "unknown");			break;
	}
}

#pragma mark -

TYPE_OBS_GLOBALS	gEnvData;

//**************************************************************************
void	InitObsConditionGloblas(void)
{
	memset(&gEnvData, 0, sizeof(TYPE_OBS_GLOBALS));
	gEnvData.siteDataValid	=	false;
	gEnvData.domeDataValid	=	false;

}

//**************************************************************************
void	DumpRequestStructure(const char *functionName, TYPE_GetPutRequestData	*reqData)
{
	printf("%s\r\n", functionName);
	printf("Dev#=%d\tG/P=%c\r\n",	reqData->deviceNumber, reqData->get_putIndicator);
	printf("HTML\t=%s\r\n",			reqData->htmlData);
	printf("cmdBuffer\t=%s\r\n",	reqData->cmdBuffer);
	printf("contentData\t=%s\r\n",	reqData->contentData);

	printf("\r\n");
}

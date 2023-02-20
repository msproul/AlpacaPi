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
//*	Re-distribution of this source code must retain this copyright notice.
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
//*	Aug 28,	2022	<MLS> Now recognize the "POST" command, ignore for now.
//*	Sep 13,	2022	<MLS> Added _ENABLE_PHASEONE_
//*	Sep 18,	2022	<MLS> Added ComputeCPUusage()
//*	Sep 19,	2022	<MLS> Working on CPU usage per active driver
//*	Oct 13,	2022	<MLS> Added UpdateProperties()
//*	Oct 14,	2022	<MLS> Added OutputHTML_DriverDocs()
//*	Oct 14,	2022	<MLS> Added OutputCommadTable()
//*	Oct 16,	2022	<MLS> Added temperaturelog command
//*	Nov 29,	2022	<MLS> Lots of work on UUID to make them unique
//*	Dec  1,	2022	<MLS> Added OutputHTML_Form() for testing
//*	Dec  1,	2022	<MLS> Added ProcessAlpacaSETUPrequest()
//*	Dec  2,	2022	<MLS> Added ParseAlpacaRequest()
//*	Dec  2,	2022	<MLS> Total re-write of Alpaca command parsing code
//*	Dec  3,	2022	<MLS> Updated all camera drivers to return # of devices created
//*	Dec  3,	2022	<MLS> Added ipAddressString to AlpacaCallback()
//*	Dec  7,	2022	<MLS> Added LogRequest()
//*	Dec 13,	2022	<MLS> Updated ascom/alpaca URL links in SendHtml_MainPage()
//*	Dec 24,	2022	<MLS> Added User-Agent counts to stats web page
//*****************************************************************************
//*	to install code blocks 20
//*	Step 1: sudo add-apt-repository ppa:codeblocks-devs/release
//*	Step 2: sudo apt-get update
//*	Step 3: sudo apt-get install codeblocks codeblocks-contrib
//*****************************************************************************
//		 getrusage() - get resource usage

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
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_PHASEONE_)
	#include	"cameradriver_PhaseOne.h"
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
	#ifdef _ENABLE_CALIBRATION_ALNITAK_
		#include	"calibration_Alnitak.h"
	#endif
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
#ifdef _ENABLE_SPECTROGRAPH_
	#include	"spectrodriver.h"
	#include	"spectrodrvr_usis.h"
	#include	"filterwheeldriver_usis.h"
	#include	"focuserdriver_USIS.h"
#endif

#if defined(__arm__) && defined(_ENABLE_WIRING_PI_)
	#include <wiringPi.h>
#endif


#include	"managementdriver.h"


AlpacaDriver	*gAlpacaDeviceList[kMaxDevices];
bool			gKeepRunning								=	true;
int				gDeviceCnt									=	0;
bool			gLiveView									=	false;
bool			gAutoExposure								=	false;
bool			gDisplayImage								=	false;
bool			gSimulateCameraImage						=	false;
bool			gVerbose									=	false;
bool			gDebugDiscovery								=	false;
bool			gObservatorySettingsOK						=	false;
const char		gValueString[]								=	"Value";
char			gDefaultTelescopeRefID[kDefaultRefIdMaxLen]	=	"";
char			gWebTitle[80]								=	"AlpacaPi";
char			gFullVersionString[128]						=	"";
int				gAlpacaListenPort							=	kAlpacaPiDefaultPORT;
uint32_t		gClientID									=	0;
uint32_t		gClientTransactionID						=	0;
uint32_t		gServerTransactionID						=	0;		//*	we are the server, we will increment this each time a transaction occurs
bool			gErrorLogging								=	false;	//*	write errors to log file if true
bool			gConformLogging								=	false;	//*	log all commands to log file to match up with Conform
bool			gImageDownloadInProgress					=	false;
char			gHostName[48]								=	"";
char			gUserAgentAlpacaPiStr[80]					=	"";
int				gUserAgentCounters[kHTTPclient_last];


#ifdef _ENABLE_BANDWIDTH_LOGGING_
	int				gTimeUnitsSinceTopOfHour	=	0;
#endif // _ENABLE_BANDWIDTH_LOGGING_

#ifdef _ENABLE_LIVE_CONTROLLER_
	static void	HandleContollerWindow(AlpacaDriver *alpacaObjPtr);
#endif // _ENABLE_LIVE_CONTROLLER_


static void	OutputHTML_Form(TYPE_GetPutRequestData *reqData);


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
	{	"temperaturelog",		kCmd_Common_TemperatureLog,		kCmdType_GET	},



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

	cRunStartupOperations		=	true;
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
	cDriverCmdTablePtr			=	NULL;
	cTotalBytesRcvd				=	0;
	cTotalBytesSent				=	0;

	cUUID.part1					=	'ALPA';				//*	4 byte manufacturer code
	cUUID.part2					=	kBuildNumber;		//*	software version number
	cUUID.part3					=	1;					//*	model number
	cUUID.part4					=	gDeviceCnt;
	cUUID.part5					=	random();			//*	serial number

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

	//========================================
	//*	CPU usage information
	cAccumilatedNanoSecs		=	0;
	cTotalNanoSeconds			=	0;
	cTotalMilliSeconds			=	0;

	//========================================
	//*	Setup support
	cDriverSupportsSetup		=	false;


	//========================================
	//*	Temperature logging
	TemperatureLog_Init();

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
	for (iii=0; iii<kCmd_Common_last; iii++)
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

	CONSOLE_DEBUG(__FUNCTION__);

	cMagicCookie	=	0;

	//*	remove this device from the list
	for (ii=0; ii<kMaxDevices; ii++)
	{
		if (gAlpacaDeviceList[ii] == this)
		{
			gAlpacaDeviceList[ii]	=	NULL;
		}
	}
}

//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	AlpacaDriver::RunStateMachine(void)
{
uint32_t			currentMilliSecs;
uint32_t			deltaMilliSecs;
int32_t				delayMicroSeconds;


	//*	5 * 1000 * 1000 means you might not get called again for 5 seconds
	//*	you might get called earlier
	delayMicroSeconds	=	5 *1000 * 1000;
	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cLastUpdate_milliSecs;
	if (deltaMilliSecs > 5000)
	{
		UpdateProperties();

		cLastUpdate_milliSecs	=	currentMilliSecs;
	}

	return(delayMicroSeconds);
}

//*****************************************************************************
//*	return value 0 = OK
//*****************************************************************************
int	AlpacaDriver::UpdateProperties(void)
{
int			returnCode;

	returnCode	=	0;

	return(returnCode);
}

//**************************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

	//*	do nothing, this routine should be overridden
	CONSOLE_DEBUG_W_STR("We should not be here, this routine needs to be over-ridden:", cAlpacaName);
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

		case kCmd_Common_TemperatureLog:
			cHttpHeaderSent	=	true;
			alpacaErrCode	=	Get_TemperatureLog(reqData, alpacaErrMsg, gValueString);
			break;

		default:
			alpacaErrCode	=	kASCOM_Err_InvalidOperation;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Unrecognized command");
			CONSOLE_DEBUG(alpacaErrMsg);
			CONSOLE_DEBUG_W_STR("deviceCommand\t=", reqData->deviceCommand);

			strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
			DumpRequestStructure(__FUNCTION__, reqData);
//			CONSOLE_ABORT(__FUNCTION__);
			break;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Connected(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
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

//	CONSOLE_DEBUG(__FUNCTION__);
	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Connected",
											argumentString,
											(sizeof(argumentString) -1));
	CONSOLE_DEBUG_W_STR(__FUNCTION__, argumentString);
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
int			runTimeSeconds;
int			percentCPU;

//	CONSOLE_DEBUG(__FUNCTION__);
	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"platform",
							gPlatformString,
							INCLUDE_COMMA);

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"cpuinfo",
							gCpuInfoString,
							INCLUDE_COMMA);

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"operatingsystem",
							gOsReleaseString,
							INCLUDE_COMMA);

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"version",
							gFullVersionString,
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

	//====================================================
	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"cRusage.ru_utime.tv_sec",
							cRusage.ru_utime.tv_sec,
							INCLUDE_COMMA);
	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"cRusage.ru_stime.tv_sec",
							cRusage.ru_stime.tv_sec,
							INCLUDE_COMMA);
	runTimeSeconds	=	millis() / 1000;
	if (runTimeSeconds > 0)
	{
		percentCPU	=	(100 * (cRusage.ru_utime.tv_sec + cRusage.ru_stime.tv_sec)) / runTimeSeconds;
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"percentCPU",
								percentCPU,
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
void	AlpacaDriver::OutputCommadTable(int mySocketFD, const char *title, const TYPE_CmdEntry *commandTable)
{
int		iii;
char	lineBuff[512];
char	cmdArgumentStr[256];
bool	useAlternateColor;
bool	foundArgFlag;

	useAlternateColor	=	false;
	sprintf(lineBuff,	"<TR><TD COLSPAN=4><B><CENTER>%s</B></TD></TR>", title);
	SocketWriteData(mySocketFD,	lineBuff);
	iii	=	0;
	while (commandTable[iii].enumValue >= 0)
	{
		if (commandTable[iii].commandName[0] == '-')
		{
			SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=4><HR></TD></TR>\r\n");
			useAlternateColor	=	true;
		}
		else
		{
			if (useAlternateColor)
			{
				sprintf(lineBuff,	"<TR><TD><FONT COLOR=#ff00ff>%s</TD>",	commandTable[iii].commandName);
			}
			else
			{
				sprintf(lineBuff,	"<TR><TD>%s</TD>",	commandTable[iii].commandName);
			}
			SocketWriteData(mySocketFD,	lineBuff);
			if ((commandTable[iii].get_put == kCmdType_GET) || (commandTable[iii].get_put == kCmdType_BOTH))
			{
				SocketWriteData(mySocketFD,	"<TD><FONT COLOR=GREEN><CENTER>GET</TD>");
			}
			else
			{
				SocketWriteData(mySocketFD,	"<TD></TD>");
			}
			if ((commandTable[iii].get_put == kCmdType_PUT) || (commandTable[iii].get_put == kCmdType_BOTH))
			{
				SocketWriteData(mySocketFD,	"<TD><FONT COLOR=RED><CENTER>PUT</TD>");
			}
			else
			{
				SocketWriteData(mySocketFD,	"<TD></TD>");
			}

			foundArgFlag	=	GetCommandArgumentString(commandTable[iii].enumValue, cmdArgumentStr);
			if (foundArgFlag == false)
			{
				AlpacaDriver::GetCommandArgumentString(commandTable[iii].enumValue, cmdArgumentStr);
			}
			sprintf(lineBuff,	"<TD>%s</TD>",	cmdArgumentStr);
			SocketWriteData(mySocketFD,	lineBuff);

			SocketWriteData(mySocketFD,	"</TD>\r\n");
		}
		iii++;
	}
}

//*****************************************************************************
//*	the purpose of this routine is for self documenting code
//*	it should return the parameter options for the command specified
//*
//*	return TRUE if found, false otherwise
//*****************************************************************************
bool	AlpacaDriver::GetCommandArgumentString(const int cmdENum, char *agumentString)
{
bool	foundFlag	=	true;

	//*	this needs to be over-ridden
	switch(cmdENum)
	{
		case kCmd_Common_action:					//*	Invokes the specified device-specific action.
		case kCmd_Common_commandblind:				//*	Transmits an arbitrary string to the device
		case kCmd_Common_commandbool:				//*	Transmits an arbitrary string to the device and returns a boolean value from the device.
		case kCmd_Common_commandstring:				//*	Transmits an arbitrary string to the device and returns a string value from the device.
		case kCmd_Common_connected:					//*	GET--Retrieves the connected state of the device
		case kCmd_Common_description:				//*	Device description
		case kCmd_Common_driverinfo:				//*	Device driver description
		case kCmd_Common_driverversion:				//*	Driver Version
		case kCmd_Common_interfaceversion:			//*	The ASCOM Device interface version number that this device supports.
		case kCmd_Common_name:						//*	Device name
		case kCmd_Common_supportedactions:			//*	Returns the list of action names supported by this driver.
			strcpy(agumentString, "");
			break;

#ifdef _INCLUDE_EXIT_COMMAND_
		case kCmd_Common_exit:						strcpy(agumentString, "-none-, causes driver to terminate");	break;
#endif
		case kCmd_Common_Extras:
		case kCmd_Common_LiveWindow:				strcpy(agumentString, "Live=BOOL");	break;

		case kCmd_Common_TemperatureLog:
			strcpy(agumentString, "returns 24 hour temperature log (24 * 60) entries");	break;
			break;

		default:
			strcpy(agumentString, "");
			foundFlag	=	false;
			break;
	}
	return(foundFlag);
}

//*****************************************************************************
//*	the purpose of this routine is to provide self documenting code
//*	in the form of a command table for the device type
//*****************************************************************************
void	AlpacaDriver::OutputHTML_DriverDocs(TYPE_GetPutRequestData *reqData)
{
int		mySocketFD;
char	lineBuff[128];

//	CONSOLE_DEBUG(__FUNCTION__);
	mySocketFD	=	reqData->socket;

	if (cDriverCmdTablePtr != NULL)
	{
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

		sprintf(lineBuff,	"<TR><TD COLSPAN=4><CENTER><H2>%s</H2></TD></TR>",	cAlpacaName);
		SocketWriteData(mySocketFD,	lineBuff);

		SocketWriteData(mySocketFD,	"<TR><TH>command</TH><TH COLSPAN=2><CENTER>GET / PUT</TH><TH>Alpaca data sting</TH></TR>");

		//-----------------------------------------------
		//*	first do the common commands
		OutputCommadTable(mySocketFD,	"Common Commands",			gCommonCmdTable);
		//-----------------------------------------------
		//*	Now do the device specific commands
		OutputCommadTable(mySocketFD,	"Device specific Commands", cDriverCmdTablePtr);

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	}
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
	for (iii=0; iii<kCmd_Common_last; iii++)
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
void	AlpacaDriver::RecordCmdStats(int cmdNum, char getput, TYPE_ASCOM_STATUS alpacaErrCode)
{
int		tblIdx;

	//*	check for common command index ( > 1000)
	if (cmdNum >= kCmd_Common_action)
	{
		tblIdx	=	cmdNum - kCmd_Common_action;
		if ((tblIdx >= 0) && (tblIdx < kCmd_Common_last))
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

////*****************************************************************************
//static void	StrcpyToEOL(char *newString, const char *oldString, const int maxLen)
//{
//int		ii;
//
//	ii	=	0;
//	while ((oldString[ii] >= 0x20) && (ii < maxLen))
//	{
//		newString[ii]	=	oldString[ii];
//		ii++;
//	}
//	if (ii < maxLen)
//	{
//		newString[ii]	=	0;
//	}
//	else
//	{
//		newString[maxLen-1]	=	0;
//	}
//}

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
	"User-Agent: AlpacaPi\r\n"
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
static void	SendHtml_TopLevel(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
int		mySocketFD;

		mySocketFD	=	reqData->socket;

		SocketWriteData(mySocketFD,	gHtmlHeader);

		sprintf(lineBuffer, "<TITLE>%s-%s</TITLE>\r\n", kApplicationName, kVersionString);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	gHtmlNightMode);

		SocketWriteData(mySocketFD,	"</HEAD><BODY>\r\n<CENTER>\r\n");
		sprintf(lineBuffer, "<H1>%s<BR>%s Build #%d </H1>\r\n", kApplicationName, kVersionString, kBuildNumber);
		SocketWriteData(mySocketFD,	lineBuffer);

		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		SocketWriteData(mySocketFD,	"<FONT SIZE=30>\r\n");
		SocketWriteData(mySocketFD,	"<UL>\r\n");

		SocketWriteData(mySocketFD,	"<LI><A HREF=setup>AlpacaPi Settings for this server</A>\r\n");
		SocketWriteData(mySocketFD,	"<P>\r\n");

		SocketWriteData(mySocketFD,	"<LI><A HREF=https://msproul.github.io/AlpacaPi/ target=github>AlpacaPi Documentation on github</A>\r\n");

		SocketWriteData(mySocketFD,	"<LI><A HREF=https://github.com/msproul/AlpacaPi target=github>Download AlpacaPi from github</A>\r\n");



		SocketWriteData(mySocketFD,	"</UL>\r\n");
		SocketWriteData(mySocketFD,	"</FONT>\r\n");

		SocketWriteData(mySocketFD,	"<P>\r\n");

		sprintf(lineBuffer, "Your IP address is %s\r\n", reqData->clientIPaddr);
		SocketWriteData(mySocketFD,	lineBuffer);
}


//*****************************************************************************
static const char	*gURLlist[]	=
{
		"https://ascom-standards.org/AlpacaDeveloper/Index.htm",
		"https://ascom-standards.org/api/",
		"https://astronomy-imaging-camera.com/software-drivers",

//		"https://agenaastro.com/zwo-astronomy-cameras-buyers-guide.html",
//		"https://agenaastro.com/articles/guides/zwo-buyers-guide.html",
		"https://agenaastro.com/articles/zwo-astronomy-cameras-buyers-guide.html",


		""
};

//*****************************************************************************
static void	SendHtml_MainPage(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
char	separaterLine[]	=	"<HR SIZE=4 COLOR=RED>\r\n";
//char	separaterLine[]	=	"<HR SIZE=4 COLOR=BLUE>\r\n";
int		mySocketFD;
int		iii;


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
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Setup</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device Type</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device Number</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device Name</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Description</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Cmds / Errs</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>CPU (ms)</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>CPU (nano-secs)</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t</TR>\r\n");

		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
				SocketWriteData(mySocketFD,	"\t<TR>\r\n");

				//*	is SETUP supported
				SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
				if (gAlpacaDeviceList[iii]->cDriverSupportsSetup)
				{
					//*	https://ascom-standards.org/api/?urls.primaryName=ASCOM%20Alpaca%20Management%20API#/
					//*		/setup/v1/{device_type}/{device_number}/setup

					sprintf(lineBuffer,	"<A HREF=/setup/v1/%s/%d/setup target=%s>Setup</A>",
												gAlpacaDeviceList[iii]->cAlpacaName,
												gAlpacaDeviceList[iii]->cDeviceNum,
												gAlpacaDeviceList[iii]->cAlpacaName);
					SocketWriteData(mySocketFD,	lineBuffer);
				}
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
					SocketWriteData(mySocketFD,	gAlpacaDeviceList[iii]->cAlpacaName);
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				sprintf(lineBuffer, "<TD><CENTER>%d</TD>\r\n", gAlpacaDeviceList[iii]->cDeviceNum);
				SocketWriteData(mySocketFD,	lineBuffer);

				SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
					SocketWriteData(mySocketFD,	gAlpacaDeviceList[iii]->cCommonProp.Name);
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
					SocketWriteData(mySocketFD,	gAlpacaDeviceList[iii]->cCommonProp.Description);
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				sprintf(lineBuffer, "<TD><CENTER>%d/%d</TD>\r\n",
											gAlpacaDeviceList[iii]->cTotalCmdsProcessed,
											gAlpacaDeviceList[iii]->cTotalCmdErrors);
				SocketWriteData(mySocketFD,	lineBuffer);

				//*	cpu usage, this may get moved to a different page later
				sprintf(lineBuffer, "<TD><CENTER>%lu</TD>\r\n", gAlpacaDeviceList[iii]->cTotalMilliSeconds);
				SocketWriteData(mySocketFD,	lineBuffer);

				sprintf(lineBuffer, "<TD><CENTER>%lu</TD>\r\n", gAlpacaDeviceList[iii]->cTotalNanoSeconds);
				SocketWriteData(mySocketFD,	lineBuffer);

				SocketWriteData(mySocketFD,	"\t</TR>\r\n");

			}
		}

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		//**********************************************************
		//*	Output the html for each device
		for (iii=0; iii<gDeviceCnt; iii++)
		{
//			CONSOLE_DEBUG_W_STR(__FUNCTION__, gAlpacaDeviceList[iii]->cCommonProp.Name);
			if (gAlpacaDeviceList[iii] != NULL)
			{
				SocketWriteData(mySocketFD,	separaterLine);
				gAlpacaDeviceList[iii]->OutputHTML(reqData);
				gAlpacaDeviceList[iii]->OutputHTML_Part2(reqData);
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

		//*	print out the list of URLS
		iii	=	0;
		while (strlen(gURLlist[iii]) > 0)
		{
			sprintf(lineBuffer,	"\t<LI><A HREF=%s target=link>%s</A>\r\n",	gURLlist[iii], gURLlist[iii]);
			SocketWriteData(mySocketFD,	lineBuffer);
//			CONSOLE_DEBUG(lineBuffer);
			iii++;
		}

		SocketWriteData(mySocketFD,	"</UL>\r\n");

		//**********************************************************
		SocketWriteData(mySocketFD,	separaterLine);
		SocketWriteData(mySocketFD,	"Compiled on ");
		SocketWriteData(mySocketFD,	__DATE__);
		SocketWriteData(mySocketFD,	"\r\n<BR>");
		SocketWriteData(mySocketFD,	"Written in C/C++\r\n<BR>");
		SocketWriteData(mySocketFD,	"(C) 2019-2023 by Mark Sproul msproul@skychariot.com\r\n<BR>");

		SocketWriteData(mySocketFD,	"</BODY></HTML>\r\n");
	}
	else
	{
	//	CONSOLE_DEBUG("reqData is NULL");
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//*****************************************************************************
const char	*gUserAgentNames[]	=
{
	"NotSpecified",
	"AlpacaPi",
	"ASCOM_RestSharp",
	"ConfomU",
	"Curl",
	"Mozilla",
	"NotRecognized"
};


//*****************************************************************************
static void	SendHtmlStats(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
char	separaterLine[]	=	"<HR SIZE=4 COLOR=RED>\r\n";
int		mySocketFD;
int		iii;

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

		//====================================================
		//*	output the request counts by user agent
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H3>Requests by User-Agent</H3>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
		for (iii=0; iii<kHTTPclient_last; iii++)
		{
			SocketWriteData(mySocketFD,	"<TR>\r\n");
			sprintf(lineBuffer, "<TD>%s</TD><TD>%d</TD>", gUserAgentNames[iii], gUserAgentCounters[iii]);
			SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TR>\r\n");
		}
		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");


		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
				SocketWriteData(mySocketFD,	separaterLine);
				gAlpacaDeviceList[iii]->OutputHTML_CmdStats(reqData);
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
static char	gDocsIntro[]	=
{
	"<P>\r\n"
	"This page documents the arguments for the Alpaca commands.\r\n"
	"The purpose of this page is to document the extra stuff that I have added to Alpaca in AlpacaPi.\r\n"
	"Most of the standard commands are not documented here because full documentation can be found on the Alpaca/ASCOM web site.\r\n"
	"Commands that are <FONT COlOR=#ff00ff>magenta</FONT COLOR> are the ones that have been added and\r\n"
	"are not part of the Alpaca standard definition.\r\n"
//	"foo\r\n"
//	"foo\r\n"
	"<P>\r\n"
};

//*****************************************************************************
static void	OutputHTML_DriverDocs(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
char	separaterLine[]	=	"<HR SIZE=4 COLOR=RED>\r\n";
int		mySocketFD;
int		iii;

	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;
		SocketWriteData(mySocketFD,	gHtmlHeader);
		sprintf(lineBuffer, "<TITLE>%s</TITLE>\r\n", gWebTitle);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</HEAD><BODY>\r\n<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H1>Alpaca device driver Web server</H1>\r\n");
		sprintf(lineBuffer, "<H3>%s</H3>\r\n", gWebTitle);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		OutPutObservatoryInfoHTML(mySocketFD);

		SocketWriteData(mySocketFD,	gDocsIntro);

		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
				SocketWriteData(mySocketFD,	separaterLine);
				gAlpacaDeviceList[iii]->OutputHTML_DriverDocs(reqData);
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
int		iii;
char	lineBuffer[256];

//	CONSOLE_DEBUG_W_STR("deviceName\t=", deviceName);
//	CONSOLE_DEBUG_W_NUM("deviceNum\t=", deviceNum);
	//*	now generate links to all of the commands
	SocketWriteData(socketFD,	"\r\n<UL>\r\n");
	iii	=	0;
	while (gCommonCmdTable[iii].commandName[0] != 0)
	{
		sprintf(lineBuffer,	"\t<LI><A HREF=../api/v1/%s/%d/%s target=cmd>%s</A>\r\n",
									deviceName,
									deviceNum,
									gCommonCmdTable[iii].commandName,
									gCommonCmdTable[iii].commandName);
		SocketWriteData(socketFD,	lineBuffer);
		iii++;
	}

	SocketWriteData(socketFD,	"<P>\r\n");

	iii	=	0;
	while (cmdTable[iii].commandName[0] != 0)
	{
		sprintf(lineBuffer,	"\t<LI><A HREF=../api/v1/%s/%d/%s target=cmd>%s</A>\r\n",
									deviceName,
									deviceNum,
									cmdTable[iii].commandName,
									cmdTable[iii].commandName);
		SocketWriteData(socketFD,	lineBuffer);
//		CONSOLE_DEBUG(lineBuffer);
		iii++;
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
static void	SendJpegResponse(int socket, const char *jpegFileName)
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

		myFilenamePtr	=	(char *)jpegFileName;
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
													long					byteCount)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
int					deviceTypeEnum;
int					iii;
bool				deviceFound;

//	CONSOLE_DEBUG(__FUNCTION__);
//	DumpRequestStructure(__FUNCTION__, reqData);

	//*******************************************
	//*	now do something with the data
	deviceTypeEnum	=	FindDeviceTypeByString(reqData->deviceType);
	deviceFound		=	false;
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
				gAlpacaDeviceList[iii]->cHttpHeaderSent			=	false;


//				CONSOLE_DEBUG("Calling ProcessCommand() ---------------------------------------------");
//				CONSOLE_DEBUG_W_STR("cAlpacaName         \t=",	gAlpacaDeviceList[iii]->cAlpacaName);
//				CONSOLE_DEBUG_W_STR("deviceCommand       \t=",	reqData->deviceCommand);
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
				//*	are we error logging
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
static TYPE_ASCOM_STATUS	ProcessManagementRequest(	TYPE_GetPutRequestData	*reqData,
														long					byteCount)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
int					iii;

//	CONSOLE_DEBUG("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
//	CONSOLE_DEBUG(__FUNCTION__);
//	DumpRequestStructure(__FUNCTION__, reqData);

	//*	step thru the list looking for management devices
	for (iii=0; iii<gDeviceCnt; iii++)
	{
		if (gAlpacaDeviceList[iii] != NULL)
		{
			if (gAlpacaDeviceList[iii]->cDeviceType == kDeviceType_Management)
			{
				gAlpacaDeviceList[iii]->cHttpHeaderSent			=	false;
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
static TYPE_ASCOM_STATUS	ProcessAlpacaSETUPrequest(	TYPE_GetPutRequestData	*reqData,
														long					byteCount)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
int					iii;
int					deviceTypeEnum;
bool				deviceFound;

	CONSOLE_DEBUG("/setup/ found");
//	CONSOLE_DEBUG_W_STR("httpCmdString\t=", reqData->httpCmdString);

	//*	check to see if its
	//		http://192.168.2.171:32323/setup/
	//	or
	//		http://192.168.2.171:32323/setup/v1/dome/0/setup


	if ((reqData->alpacaVersion != 1) || (strlen(reqData->deviceCommand) <= 0))
	{
		SendHtml_MainPage(reqData);
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
//		DumpRequestStructure(__FUNCTION__, reqData);

		deviceTypeEnum	=	FindDeviceTypeByString(reqData->deviceType);
		deviceFound		=	false;
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
				if ((gAlpacaDeviceList[iii]->cDeviceType == deviceTypeEnum) &&
					(gAlpacaDeviceList[iii]->cDeviceNum == reqData->deviceNumber))
				{
					deviceFound		=	true;

//					CONSOLE_DEBUG("Calling Setup_ProcessCommand() ---------------------------------------------");
//					CONSOLE_DEBUG_W_STR("cAlpacaName         \t=",	gAlpacaDeviceList[iii]->cAlpacaName);
//					CONSOLE_DEBUG_W_STR("deviceCommand       \t=",	reqData->deviceCommand);
					gAlpacaDeviceList[iii]->Setup_ProcessCommand(reqData);
					break;
				}
			}
		}
		if (deviceFound)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
	}
	return(alpacaErrCode);
}

static	FILE	*gIPlogFilePointer		=	NULL;
static	bool	gIPlogNeedsToBeOpened	=	true;
static	long	gIPlogWriteCount		=	0;
static	short	gCurrentDayOfMonth		=	-1;
//*****************************************************************************
static void	LogRequest(TYPE_GetPutRequestData	*reqData)
{
char		lineBuff[512];
char		datestring[64];
time_t		currentTime;
struct tm	*linuxTime;
int			bytesWritten;
char		getPutStr[16];
char		logFilename[64];
int			returnCode;

//	CONSOLE_DEBUG(__FUNCTION__);
//2022/12/08 08:19:15	10.6.0.3          	Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:106.0) Gecko/20100101 Firefox/106.0	GET /setup/v1/camera/0/setup

	gIPlogWriteCount++;

	currentTime		=	time(NULL);
	if (currentTime != -1)
	{
		linuxTime		=	localtime(&currentTime);
		sprintf(datestring, "%d/%02d/%02d %02d:%02d:%02d",	(1900 + linuxTime->tm_year),
															(1 + linuxTime->tm_mon),
															linuxTime->tm_mday,
															linuxTime->tm_hour,
															linuxTime->tm_min,
															linuxTime->tm_sec);
	}
	else
	{
		strcpy(datestring, "date-unknown");
	}
	if (reqData->get_putIndicator == 'G')
	{
		strcpy(getPutStr, "GET");
	}
	else if (reqData->get_putIndicator == 'P')
	{
		strcpy(getPutStr, "PUT");
	}
	else
	{
		strcpy(getPutStr, "xxx");
	}

	if (gIPlogNeedsToBeOpened)
	{
		//*	create a log filename with today's date
		linuxTime		=	localtime(&currentTime);
		sprintf(logFilename, "requestlog-%d-%02d-%02d.txt",	(1900 + linuxTime->tm_year),
															(1 + linuxTime->tm_mon),
															linuxTime->tm_mday);
		CONSOLE_DEBUG("-------------------------------------------------------------------------");
		CONSOLE_DEBUG_W_STR("Open log file:", logFilename);
		gIPlogFilePointer		=	fopen(logFilename, "a");
		gIPlogNeedsToBeOpened	=	false;
		gCurrentDayOfMonth		=	linuxTime->tm_mday;

		//*	record the fact that we opened the log file
		sprintf(lineBuff,	"%-18s\tLog file opened --------------------------------------------------------\r\n", datestring);
		bytesWritten	=	fprintf(gIPlogFilePointer,	"%s", lineBuff);
	}


	sprintf(lineBuff,	"%-18s\t%-18s\t%s\t%s %s\r\n",
						datestring,
						reqData->clientIPaddr,
						reqData->httpUserAgent,
						getPutStr,
						reqData->cmdBuffer);
//	CONSOLE_DEBUG(lineBuff);


	if (gIPlogFilePointer != NULL)
	{
		bytesWritten	=	fprintf(gIPlogFilePointer, "%s", lineBuff);
		if (bytesWritten >= 0)
		{
			fflush(gIPlogFilePointer);
		}
		else
		{
			CONSOLE_DEBUG("Error writing to logfile");
			fclose(gIPlogFilePointer);
			gIPlogFilePointer		=	NULL;
			gIPlogNeedsToBeOpened	=	true;
		}

		//*	when ever the day changes, close the file and open a new one
		if (linuxTime->tm_mday != gCurrentDayOfMonth)
		{
			CONSOLE_DEBUG("Closing log file");
			returnCode	=	fclose(gIPlogFilePointer);
			if (returnCode != 0)
			{
				CONSOLE_DEBUG("Error closing requestlog")
			}
			gIPlogFilePointer		=	NULL;
			gIPlogNeedsToBeOpened	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG("Error: gIPlogFilePointer is NULL");
	}
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
char			*userAgentPtr;
//int				htmlDataLen;
int				contentDataLen;

//	CONSOLE_DEBUG(__FUNCTION__);

	if ((htmlData != NULL) && (reqData != NULL))
	{
//		htmlDataLen	=	strlen(htmlData);

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

		//========================================================================
		//*	check for user agent
		userAgentPtr	=	strcasestr((char *)htmlData, "User-Agent:");
		if (userAgentPtr != NULL)
		{
			//*	extract the "User-Agent:"
			userAgentPtr	+=	11;
			while (*userAgentPtr == 0x20)	//*	skip spaces
			{
				userAgentPtr++;
			}
			ccc	=	0;
			while ((userAgentPtr[ccc] >= 0x20) && (ccc < (kUserAgentLen - 2)))
			{
				reqData->httpUserAgent[ccc]	=	userAgentPtr[ccc];
				ccc++;
			}
			reqData->httpUserAgent[ccc]	=	0;

			//===========================================================================
			//*	check to see who is talking to us so we can keep count.....................
			if (strncasecmp(reqData->httpUserAgent, "AlpacaPi", 8) == 0)
			{
				reqData->clientIs_AlpacaPi	=	true;
				reqData->cHTTPclientType	=   kHTTPclient_AlpacaPi;
			}
			else if (strncasecmp(reqData->httpUserAgent, "ConformU", 8) == 0)
			{
				reqData->clientIs_ConformU	=	true;
				reqData->cHTTPclientType	=   kHTTPclient_ConfomU;
			}
			else if (strncasecmp(reqData->httpUserAgent, "curl", 4) == 0)
			{
				reqData->cHTTPclientType	=   kHTTPclient_Curl;
			}
			else if (strncasecmp(reqData->httpUserAgent, "Mozilla", 7) == 0)
			{
				reqData->cHTTPclientType	=   kHTTPclient_Mozilla;
				CONSOLE_DEBUG_W_STR("User-Agent:\t=", reqData->httpUserAgent);
			}
			else if (strncasecmp(reqData->httpUserAgent, "RestSharp", 9) == 0)
			{
				reqData->cHTTPclientType	=   kHTTPclient_ASCOM_RestSharp;
			}
			else
			{
				reqData->cHTTPclientType	=   kHTTPclient_NotRecognized;
				CONSOLE_DEBUG_W_STR("User-Agent:\t=", reqData->httpUserAgent);
			}
		}
		else
		{
			reqData->cHTTPclientType	=   kHTTPclient_NotSpecified;
			strcpy(reqData->httpUserAgent, "not-specified");
//			CONSOLE_DEBUG("'User-Agent:' not found!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//			CONSOLE_DEBUG_W_STR("htmlData:\r\n", htmlData);
		}

		if ((reqData->cHTTPclientType >= 0) && (reqData->cHTTPclientType < kHTTPclient_last))
		{
			gUserAgentCounters[reqData->cHTTPclientType]++;
		}

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
				if (ccc < (sizeof(lineBuff) - 1))
				{
					lineBuff[ccc++]	=	theChar;
					lineBuff[ccc]	=	0;
				}
				else
				{
					CONSOLE_DEBUG_W_STR("Buffer overflow:", lineBuff);
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
						contentDataLen	=	strlen(reqData->contentData);
						if (contentDataLen + strlen(lineBuff) < (kContentDataLen -2))
						{
							strcat(reqData->contentData, lineBuff);
						//	strcat(reqData->contentData, "\r\n");
						}
						else
						{
							CONSOLE_DEBUG_W_STR("contentData overflow:", lineBuff);
						}
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
		CONSOLE_ABORT(__FUNCTION__);
	}
}


//*****************************************************************************
typedef enum
{
	kRequestType_Invalid	=	-1,
	kRequestType_API		=	0,
	kRequestType_Docs,
	kRequestType_Log,
	kRequestType_Managment,
	kRequestType_Setup,
	kRequestType_Stats,
	kRequestType_Web,
	kRequestType_TopLevel,

	kRequestType_Form,

	kRequestType_last
} TYPE_REQUEST_TYPE;

//*****************************************************************************
typedef struct
{
	char				RequestString[16];
	TYPE_REQUEST_TYPE	RequstType;
} TYPE_Request;


//*****************************************************************************
static TYPE_Request	gRequestType[]	=
{
	{	"api",			kRequestType_API		},
	{	"docs",			kRequestType_Docs		},
	{	"log",			kRequestType_Log		},
	{	"management",	kRequestType_Managment	},
	{	"setup",		kRequestType_Setup		},
	{	"stats",		kRequestType_Stats		},
	{	"web",			kRequestType_Web		},

	{	"form",			kRequestType_Form		},

	{	"",				kRequestType_Invalid	},
	{	"",				kRequestType_Invalid	},

};

//*****************************************************************************
//*	returns ENUM of request type
//*****************************************************************************
TYPE_REQUEST_TYPE	FindRequestType(char *requestTypeString)
{
TYPE_REQUEST_TYPE	requestType;
int					iii;
int					commandLen;

	//*	figure out the base type of the request (see enum list above)
	requestType	=	kRequestType_Invalid;
	if (strlen(requestTypeString) > 0)
	{
		iii			=	0;
		while ((requestType < 0) && (iii < kRequestType_last))
		{
			commandLen	=	strlen(gRequestType[iii].RequestString);
			if (strncasecmp(requestTypeString, gRequestType[iii].RequestString, commandLen) == 0)
			{
				requestType	=	gRequestType[iii].RequstType;
			}
			iii++;
		}
	}
	else
	{
		requestType	=	kRequestType_TopLevel;
	}
	return(requestType);
}

//*****************************************************************************
//*	Parse the full Alpaca request, trying to cover all variants
//*	returns ENUM of request type
//*****************************************************************************
static int	ParseAlpacaRequest(TYPE_GetPutRequestData *reqData)
{
int		requestType;
int		iii;
int		ccc;
bool	foundKeyWord;
int		slashCounter;
char	sLen;
char	theChar;
char	argumentString[256]			=	"";
char	myRequestTypeString[64]		=	"";
char	myAlpacaVersionString[64]	=	"";
char	myDeviceString[64]			=	"";
char	myDeviceNumString[64]		=	"";
char	myDeviceCmdString[256]		=	"";
int		cmdBuffLen;

//	CONSOLE_DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//	CONSOLE_DEBUG(__FUNCTION__);
//	DumpRequestStructure(__FUNCTION__, reqData);
//	PUT /api/v1/camera/0/exposuretime HTTP/1.1

	//*	copy the full command over
	ccc		=	0;
	iii		=	4;
	sLen	=	strlen(reqData->httpCmdString);
	while ((reqData->httpCmdString[iii] > 0x20) && (iii< sLen) && (ccc < (kMaxCommandLen - 1)))
	{
		reqData->cmdBuffer[ccc++]	=	reqData->httpCmdString[iii++];
	}
	reqData->cmdBuffer[ccc]	=	0;


	slashCounter		=	0;
	ccc					=	0;
	iii					=	4;	//*	start after "GET " so that we can ignore the first space
	argumentString[0]	=	0;
	theChar				=	0;
	while ((slashCounter < 6) && (iii < sLen) && (theChar != 0x20))
	{
		theChar	=	reqData->httpCmdString[iii++];
		if ((theChar == '/') || (theChar == 0x20))
		{
			argumentString[ccc]		=	0;
			switch(slashCounter)
			{
				case 1:
					//*	the request type, i.e. api, or setup (see list above)
					strcpy(myRequestTypeString, argumentString);
					break;
				case 2:
					strcpy(myAlpacaVersionString, argumentString);
					break;
				case 3:
					strcpy(myDeviceString, argumentString);
					break;
				case 4:
					strcpy(myDeviceNumString, argumentString);
					break;
				case 5:
					strcpy(myDeviceCmdString, argumentString);
					break;
			}
			ccc					=	0;
			argumentString[0]	=	0;
			slashCounter++;
		}
		else
		{
			argumentString[ccc++]	=	theChar;
			argumentString[ccc]		=	0;
		}
	}

	//---------------------------------------------------
	if (slashCounter >= 3)
	{
//		CONSOLE_DEBUG_W_STR("myAlpacaVersionString\t=",	myAlpacaVersionString);

		if (myAlpacaVersionString[0] == 'v')
		{
			reqData->alpacaVersion		=	myAlpacaVersionString[1] & 0x0f;
		}
//		CONSOLE_DEBUG_W_NUM("reqData->alpacaVersion\t=",	reqData->alpacaVersion);
	}

	if (slashCounter > 5)
	{
		strcpy(reqData->deviceType,		myDeviceString);
		reqData->deviceNumber		=	atoi(myDeviceNumString);

		//*	extract out the command itself for easier processing by the handlers
		cmdBuffLen		=	strlen(myDeviceCmdString);
		ccc				=	0;
		while (	(myDeviceCmdString[ccc] > 0x20) &&
				(myDeviceCmdString[ccc] != '&') &&
				(myDeviceCmdString[ccc] != '?') &&
				(ccc < cmdBuffLen ))
		{
			reqData->deviceCommand[ccc]	=	myDeviceCmdString[ccc];
			ccc++;
		}
		reqData->deviceCommand[ccc]	=	0;
	}
	else
	{
		strcpy(reqData->deviceType,		"unknown");
		reqData->deviceCommand[0]	=	0;
	}

	//*	figure out the base type of the request (see enum list above)
	requestType					=	FindRequestType(myRequestTypeString);
	reqData->requestTypeEnum	=	requestType;

	//	/management/v1/configureddevices
	//	/management/apiversions
	if (requestType == kRequestType_Managment)
	{
//		CONSOLE_DEBUG_W_NUM("slashCounter         \t=",	slashCounter);
//		CONSOLE_DEBUG_W_STR("myRequestTypeString  \t=",	myRequestTypeString);
//		CONSOLE_DEBUG_W_STR("myAlpacaVersionString\t=",	myAlpacaVersionString);
//		CONSOLE_DEBUG_W_STR("myDeviceString       \t=",	myDeviceString);
//		CONSOLE_DEBUG_W_STR("myDeviceNumString    \t=",	myDeviceNumString);
//		CONSOLE_DEBUG_W_STR("myDeviceCmdString    \t=",	myDeviceCmdString);

		if ((strlen(myAlpacaVersionString) > 3) && (strlen(myDeviceString) == 0))
		{
			strcpy(reqData->deviceCommand, myAlpacaVersionString);
		}
		else
		{
			strcpy(reqData->deviceCommand, myDeviceString);
		}
//		DumpRequestStructure(__FUNCTION__, reqData);
	}

	//------------------------------------------------------------------
	//*	check contentData for "HTTP...." at the end
	sLen	=	strlen(reqData->contentData);
	iii		=	sLen -1;
	while ((iii > 1) && (reqData->contentData[iii] != 0x20))
	{
		iii--;
	}
	iii++;
	if (strncmp(&reqData->contentData[iii], "HTTP", 4) == 0)
	{
		reqData->contentData[iii-1]	=	0;
	}

	//------------------------------------------------------------------
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
//		CONSOLE_DEBUG("gClientTransactionID NOT FOUND");
	}
	CONSOLE_DEBUG_W_NUM("gClientID\t=", gClientID);
	CONSOLE_DEBUG_W_NUM("gClientTransactionID\t=", gClientTransactionID);

#endif // _DEBUG_CONFORM_

//	DumpRequestStructure(__FUNCTION__, reqData);

	return(requestType);
}

//*****************************************************************************
static int	ProcessGetPutRequest(const int socket, char *htmlData, long byteCount, const char *ipAddressString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_InternalError;
char					*parseChrPtr;
TYPE_GetPutRequestData	reqData;
int						requestType;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG("=========================================================================================");
	CONSOLE_DEBUG("GET/PUT");
	CONSOLE_DEBUG_W_STR("htmlData\t=", htmlData);
#else
//	CONSOLE_DEBUG(__FUNCTION__);
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
	reqData.requestTypeEnum		=	-1;
	reqData.deviceNumber		=	-1;
	strcpy(reqData.clientIPaddr, ipAddressString);

	ParseHTMLdataIntoReqStruct(htmlData, &reqData);

	requestType	=	ParseAlpacaRequest(&reqData);
	LogRequest(&reqData);

	parseChrPtr			=	htmlData;
	parseChrPtr			+=	3;
	while (*parseChrPtr == 0x20)
	{
		parseChrPtr++;
	}

//	DumpRequestStructure(__FUNCTION__, &reqData);

	alpacaErrCode	=	kASCOM_Err_Success;
	switch(requestType)
	{
		//*	standard ALPACA api call
		case kRequestType_API:
			alpacaErrCode	=	ProcessAlpacaAPIrequest(&reqData, byteCount);
			break;

		//*	extra self docoumenting
		case kRequestType_Docs:
			OutputHTML_DriverDocs(&reqData);
			break;

		//*	extra - logging data
		case kRequestType_Log:
			SendHtmlLog(socket);
			break;

		//*	standard ALPACA management
		case kRequestType_Managment:
			alpacaErrCode	=	ProcessManagementRequest(&reqData, byteCount);
			break;

		//*	standard ALPACA setup
		case kRequestType_Setup:
			alpacaErrCode	=	ProcessAlpacaSETUPrequest(&reqData, byteCount);
			break;

		//*	extra - stats
		case kRequestType_Stats:
			SendHtmlStats(&reqData);
			break;

		case kRequestType_Web:
			SendHtml_MainPage(&reqData);
			break;

		case kRequestType_TopLevel:
			SendHtml_TopLevel(&reqData);
			break;


		//*	this is for testing, will be deleted later
		case kRequestType_Form:
			OutputHTML_Form(&reqData);
			break;



		default:
			DumpRequestStructure(__FUNCTION__, &reqData);
//			CONSOLE_DEBUG_W_STR("parseChrPtr\t=", parseChrPtr);
			//-------------------------------------------------------------------
			if (strncasecmp(parseChrPtr,	"/favicon.ico", 12) == 0)
			{
				CONSOLE_DEBUG("favicon.ico");
				//*	do nothing, this is my web browser sends this
				//		CONSOLE_DEBUG("Ignored");
				SendJpegResponse(socket, "favicon.ico");
			}
			//-------------------------------------------------------------------
			else if (strncasecmp(parseChrPtr,	"/image.jpg", 10) == 0)
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
			break;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
static void	ProcessPostCommand(const int socket)
{
char	postResponse[2048];
int		bytesWritten;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(postResponse,	"HTTP/1.1 200 OK\r\n");
	strcat(postResponse,	"Date: Tue, 06 Sep 2022 00:34:52 GMT\r\n");
	strcat(postResponse,	"/Server: Apache/2.4.41 (Ubuntu)\r\n");
	strcat(postResponse,	"Content-Type: text/plain\r\n");
	strcat(postResponse,	"X-Frame-Options: DENY\r\n");
	strcat(postResponse,	"Vary: Cookie,Accept-Encoding\r\n");
//	strcat(postResponse,	"Content-Length: 126\r\n");
	strcat(postResponse,	"X-Content-Type-Options: nosniff\r\n");
	strcat(postResponse,	"Referrer-Policy: same-origin\r\n");
	strcat(postResponse,	"Cross-Origin-Opener-Policy: same-origin\r\n");
//	strcat(postResponse,	"Set-Cookie: csrftoken=T7AkBsNUMW0a6FjxDw2bDtMBFlExEWfXO7VXrawwSX6F704cS2zPOst8QHT7LtWh; expires=Tue, 05 Sep 2023 00:34:52 GMT; Max-Age=31449600; Path=/; SameSite=Lax
	strcat(postResponse,	"Connection: close\r\n");

	strcat(postResponse,	"{\"status\": \"success\", \"message\": \"authenticated user: xyzzy@gmail.com\", \"session\": \"i56kn2jy8gdn1lgq865e2fdo9uht2eak\"}\r\n");

	CONSOLE_DEBUG_W_STR("Sending data:", postResponse);

	bytesWritten	=	SocketWriteData(socket,	postResponse);
	CONSOLE_DEBUG_W_NUM("bytesWritten\t=",	bytesWritten);
}


//*****************************************************************************
//*	this function is called from the socket handler with the received data
//*	It will parse through the data checking all of the normal alpaca requirements
//*	and then call the appropriate function based on the device type
//*****************************************************************************
static int AlpacaCallback(const int socket, char *htmlData, long byteCount,  const char *ipAddressString)
{
int		returnCode	=	-1;

//	CONSOLE_DEBUG("Timing Start----------------------");
//	SETUP_TIMING();

	if (strstr(htmlData, "favicon.ico") != NULL)
	{
		CONSOLE_DEBUG("favicon.ico request !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}


//	if (strncmp(htmlData, "GET /favicon.ico", 16) == 0)
//	{
//		//*	do nothing
//	}
//	else
	if ((strncmp(htmlData, "GET", 3) == 0) || (strncmp(htmlData, "PUT", 3) == 0))
	{
//		CONSOLE_DEBUG("Calling ProcessGetPutRequest");
		returnCode	=	ProcessGetPutRequest(socket, htmlData, byteCount, ipAddressString);
	}
	else if (strncmp(htmlData, "POST", 4) == 0)
	{
		CONSOLE_DEBUG("POST command not supported (yet)");
		CONSOLE_DEBUG_W_STR("htmlData\t=",	htmlData);
		CONSOLE_DEBUG_W_LONG("byteCount\t=",	byteCount);
		ProcessPostCommand(socket);
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
	printf("\t%-12s\t%s\r\n",	"-a",			"Auto exposure");
	printf("\t%-12s\t%s\r\n",	"-c",			"Conform logging, log ALL commands to disk");
	printf("\t%-12s\t%s\r\n",	"-d",			"Display images as they are taken");
	printf("\t%-12s\t%s\r\n",	"-e",			"Error logging, log errors commands to disk");
	printf("\t%-12s\t%s\r\n",	"-h",			"This help message");
	printf("\t%-12s\t%s\r\n",	"-l",			"Live mode");
	printf("\t%-12s\t%s\r\n",	"-p <port>",	"what port to use (default 6800)");
	printf("\t%-12s\t%s\r\n",	"-q",			"quiet (less console messages)");
	printf("\t%-12s\t%s\r\n",	"-s",			"Simulate camera image (ATIK, QHY and QSI only at present)");
	printf("\t%-12s\t%s\r\n",	"-t <profile>",	"Which telescope profile to use");
	printf("\t%-12s\t%s\r\n",	"-v",			"verbose (more console messages default)");
}

//*****************************************************************************
static void	ProcessCmdLineArgs(int argc, char **argv)
{
int		iii;
char	theChar;
int		newListenPort;

	for (iii=1; iii<argc; iii++)
	{
		if (argv[iii][0] == '-')
		{
			theChar	=	argv[iii][1];
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
					iii++;
					newListenPort		=	atoi(argv[iii]);
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
					CONSOLE_DEBUG("gSimulateCameraImage = true");
					break;

				//*	"-t" means which telescope profile to use
				//*	either -tNEWT16 or -t Newt16
				case 't':
					CONSOLE_DEBUG_W_STR("argv[iii]\t=", argv[iii]);
					if (strlen(argv[iii]) > 2)
					{
						strncpy(gDefaultTelescopeRefID, &argv[iii][2], (kDefaultRefIdMaxLen - 2));
						gDefaultTelescopeRefID[kDefaultRefIdMaxLen - 1]	=	0;
					}
					else if (argc > (iii+1))
					{
						iii++;
						strncpy(gDefaultTelescopeRefID, argv[iii], (kDefaultRefIdMaxLen - 2));
						gDefaultTelescopeRefID[kDefaultRefIdMaxLen - 1]	=	0;
					}
					CONSOLE_DEBUG_W_STR("gDefaultTelescopeRefID\t=", gDefaultTelescopeRefID);
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
	CONSOLE_DEBUG(		"******************** Alpaca device properties ***************");
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
void	AlpacaDriver::ComputeCPUusage(void)
{

	getrusage(RUSAGE_SELF, &cRusage);

}


//*****************************************************************************
static void	CreateDriverObjects()
{
int		cameraCnt	=	0;


//*********************************************************
//*	Cameras
//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_ATIK_)
	cameraCnt	+=	CreateATIK_CameraObjects();
#endif

//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_ASI_)
	cameraCnt	+=	CreateASI_CameraObjects();
#endif
//-----------------------------------------------------------
//#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FLIR_) && (__GNUC__ > 5)
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FLIR_)
	cameraCnt	+=	CreateFLIR_CameraObjects();
#endif
//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_PHASEONE_)
	cameraCnt	+=	CreatePhaseOne_CameraObjects();
#endif

//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QHY_)
	cameraCnt	+=	CreateQHY_CameraObjects();
#endif
//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_QSI_)
	cameraCnt	+=	CreateQSI_CameraObjects();
#endif
//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_TOUP_)
	cameraCnt	+=	CreateTOUP_CameraObjects();
#endif

//-----------------------------------------------------------
#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_SONY_)
	CreateSONY_CameraObjects();
#endif

#if defined(_ENABLE_CAMERA_) && defined(_SIMULATE_CAMERA_)
	cameraCnt	+=	CreateSimulator_CameraObjects();
#endif

	CONSOLE_DEBUG_W_NUM("total cameras created\t=", cameraCnt);

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
#ifdef _ENABLE_FOCUSER_MOONLITE_
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


#if _ENABLE_CALIBRATION_
	#ifdef _ENABLE_CALIBRATION_RPI_
		CreateCalibrationObjectsRPi();
	#endif
	#ifdef _ENABLE_CALIBRATION_ALNITAK_
		CreateCalibrationObjectsAlnitak();
	#endif
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
#ifdef _ENABLE_SPECTROGRAPH_
	CreateSpectrographObjects();
	CreateFilterWheelObjects_USIS();
	CreateFocuserObjects_USUS();
#endif

	//*********************************************************
	//*	Management
	CreateManagementObject();


}


//static	int32_t	gMainLoopCntr;
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
int32_t			mainLoopCntr;
uint64_t		startNanoSecs;
uint64_t		endNanoSecs;
uint64_t		deltaNanoSecs;
time_t			currentTime;
struct tm		*linuxTime;


#if defined(_ENABLE_FITS_) || defined(_ENABLE_JPEGLIB_)
	char			lineBuffer[64];
#endif
//struct rlimit	myRlimit;
//int				errorCode;

	printf("AlpacaPi driver\r\n");
	sprintf(gFullVersionString,		"%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);
	sprintf(gUserAgentAlpacaPiStr,	"User-Agent: AlpacaPi/%s-Build-%d\r\n", kVersionString,  kBuildNumber);

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
	#if (CV_MAJOR_VERSION >= 4)
		AddLibraryVersion("software", "opencv", cv::getVersionString().c_str());
		CONSOLE_DEBUG_W_STR("opencv version (library)\t=", cv::getVersionString().c_str());
	#else
		AddLibraryVersion("software", "opencv", CV_VERSION);
	#endif
	CONSOLE_DEBUG_W_STR("opencv version (include)\t=", CV_VERSION);
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

	if (gVerbose)
	{
		CONSOLE_DEBUG_W_STR("CPU info  \t=",	gCpuInfoString);
		CONSOLE_DEBUG_W_STR("OS Release\t=",	gOsReleaseString);
		CONSOLE_DEBUG_W_STR("Platform  \t=",	gPlatformString);
	}

	//-----------------------------------------------------
	currentTime		=	time(NULL);
	if (currentTime != -1)
	{
	unsigned int	randomSeed;

		linuxTime		=	localtime(&currentTime);
		randomSeed	=	linuxTime->tm_hour * linuxTime->tm_min * linuxTime->tm_sec;
		srandom(randomSeed);
	}

	InitObsConditionGloblas();
	ProcessCmdLineArgs(argc, argv);

//	CONSOLE_DEBUG_W_INT32("sizeof(int)\t=",		(long)sizeof(int));
//	CONSOLE_DEBUG_W_INT32("sizeof(long)\t=",	(long)sizeof(long));

	for (iii=0; iii<kHTTPclient_last; iii++)
	{
		gUserAgentCounters[iii]	=	0;
	}
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

	gObservatorySettingsOK	=	ObservatorySettings_ReadFile();

#ifdef _ENABLE_IMU_
	IMU_Init();
	IMU_Print_Calibration();
	IMU_SetDebug(false);
#endif


	//--------------------------------------------------------
	//*	create the various driver objects
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


	//========================================================================================
	gKeepRunning	=	true;
	mainLoopCntr	=	0;
	while (gKeepRunning)
	{
		mainLoopCntr++;
		delayTime_microSecs	=	(1000000 / 2);		//*	default to 1/2 second
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			//==================================================================================
			//*	Run state machines for enabled device.
			//*	Not all devices have state machines to run
			if (gAlpacaDeviceList[iii] != NULL)
			{
				//*	this helps verify that it is a valid object and nothing is corrupted
				if (gAlpacaDeviceList[iii]->cMagicCookie == kMagicCookieValue)
				{
					startNanoSecs			=	MSecTimer_getNanoSecs();

					delayTimeForThisTask	=	gAlpacaDeviceList[iii]->RunStateMachine();
					endNanoSecs				=	MSecTimer_getNanoSecs();
					deltaNanoSecs			=	endNanoSecs - startNanoSecs;

					gAlpacaDeviceList[iii]->cTotalNanoSeconds		+=	deltaNanoSecs;
					gAlpacaDeviceList[iii]->cAccumilatedNanoSecs	+=	deltaNanoSecs;
					if (gAlpacaDeviceList[iii]->cAccumilatedNanoSecs > 1000000)
					{
						gAlpacaDeviceList[iii]->cAccumilatedNanoSecs	-=	1000000;
						gAlpacaDeviceList[iii]->cTotalMilliSeconds++;
					}

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
						delayTime_microSecs	=	50;
					}
				#endif // _ENABLE_LIVE_CONTROLLER_

					//*	we dont need to do these every time through
					if ((mainLoopCntr % 5) == 0)
					{
						gAlpacaDeviceList[iii]->CheckWatchDogTimeout();
						gAlpacaDeviceList[iii]->ComputeCPUusage();
					}

					//*	put a little delay in between each device
					usleep(10);
				}
				else
				{
					CONSOLE_DEBUG("Magic cookie is bad");
				}
			}
		}
		if (delayTime_microSecs < 50)
		{
			delayTime_microSecs	=	50;
		}
		usleep(delayTime_microSecs);
	}
	CONSOLE_DEBUG_W_BOOL("gKeepRunning\t=", gKeepRunning);
	CONSOLE_DEBUG("Shutting down");

	//*	the program has been told to quit, go through and delete the objects
	for (iii=0; iii<kMaxDevices; iii++)
	{
		if (gAlpacaDeviceList[iii] != NULL)
		{
			CONSOLE_DEBUG_W_STR("Deleting ", gAlpacaDeviceList[iii]->cCommonProp.Name);
			delete gAlpacaDeviceList[iii];
		}
	}

	CONSOLE_DEBUG("Clean exit");
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
		//		while ((dataSource[iii] >= 0x20) && (dataSource[iii] != '&') && (jjj < (maxArgLen - 2)))
				while ((dataSource[iii] > 0x20) && (dataSource[iii] != '&') && (jjj < (maxArgLen - 2)))
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
AlpacaDriver	*FindDeviceByType(const int deviceType, const int alpacaDevNum)
{
int				iii;
AlpacaDriver	*devicePtr;

	devicePtr	=	NULL;
	for (iii=0; iii<gDeviceCnt; iii++)
	{
		if (gAlpacaDeviceList[iii] != NULL)
		{
			//*	make sure the object is valid
			if (gAlpacaDeviceList[iii]->cMagicCookie == kMagicCookieValue)
			{
				if (gAlpacaDeviceList[iii]->cDeviceType == deviceType)
				{
					//*	was a specific device number specified
					if (alpacaDevNum >= 0)
					{
						if (gAlpacaDeviceList[iii]->cDeviceNum == alpacaDevNum)
						{
							CONSOLE_DEBUG("Found matching device with matching device number");
							devicePtr	=	gAlpacaDeviceList[iii];
						}
					}
					else
					{
						devicePtr	=	gAlpacaDeviceList[iii];
					}
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
		case kDeviceType_Shutter:				strcpy(alpacaName, "Shutter");			break;
		case kDeviceType_SlitTracker:			strcpy(alpacaName, "SlitTracker");		break;
		case kDeviceType_Spectrograph:			strcpy(alpacaName, "Spectrograph");		break;

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
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "*********************************");
	CONSOLE_DEBUG_W_STR(	"called from         \t=",	functionName);
	CONSOLE_DEBUG_W_STR(	"httpUserAgent       \t=",	reqData->httpUserAgent);
	CONSOLE_DEBUG_W_NUM(	"cHTTPclientType     \t=",	reqData->cHTTPclientType);
	CONSOLE_DEBUG_W_BOOL(	"clientIs_AlpacaPi   \t=",	reqData->clientIs_AlpacaPi);
	CONSOLE_DEBUG_W_BOOL(	"clientIs_ConformU   \t=",	reqData->clientIs_ConformU);

	CONSOLE_DEBUG_W_STR(	"httpCmdString       \t=",	reqData->httpCmdString);
	CONSOLE_DEBUG_W_NUM(	"requestTypeEnum     \t=",	reqData->requestTypeEnum);
	CONSOLE_DEBUG_W_NUM(	"Alpaca Version      \t=",	reqData->alpacaVersion);

	CONSOLE_DEBUG_W_STR(	"deviceType          \t=",	reqData->deviceType);
	CONSOLE_DEBUG_W_NUM(	"Device Number       \t=",	reqData->deviceNumber);
	CONSOLE_DEBUG_W_STR(	"cmdBuffer           \t=",	reqData->cmdBuffer);
	CONSOLE_DEBUG_W_STR(	"deviceCommand       \t=",	reqData->deviceCommand);
	CONSOLE_DEBUG_W_STR(	"contentData         \t=",	reqData->contentData);
	CONSOLE_DEBUG_W_NUM(	"gClientID           \t=",	gClientID);
	CONSOLE_DEBUG_W_NUM(	"gClientTransactionID\t=",	gClientTransactionID);
	CONSOLE_DEBUG_W_NUM(	"gServerTransactionID\t=",	gServerTransactionID);


//	printf("Dev#=%d\tG/P=%c\r\n",	reqData->deviceNumber, reqData->get_putIndicator);
//	printf("HTML\t=%s\r\n",			reqData->htmlData);

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "--------------exit---------------");
}



//**************************************************************************
//*	https://www.w3schools.com/html/html_forms.asp
//**************************************************************************
static void	OutputHTML_Form(TYPE_GetPutRequestData *reqData)
{
int		mySocketFD;

	mySocketFD	=	reqData->socket;
	SocketWriteData(mySocketFD,	gHtmlHeader);

	SocketWriteData(mySocketFD,	"<!DOCTYPE html>\r\n");
	SocketWriteData(mySocketFD,	"<HTML>\r\n");
	SocketWriteData(mySocketFD,	"<TITLE>Form Testing</TITLE>\r\n");
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H1>Form Testing</H1>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<form action=\"/action_page.php\" target=\"_blank\">\r\n");
	SocketWriteData(mySocketFD,	"  <label for=\"fname\">First name:</label><br>\r\n");
	SocketWriteData(mySocketFD,	"  <input type=\"text\" id=\"fname\" name=\"fname\" value=\"John\"><br>\r\n");
	SocketWriteData(mySocketFD,	"  <label for=\"lname\">Last name:</label><br>\r\n");
	SocketWriteData(mySocketFD,	"  <input type=\"text\" id=\"lname\" name=\"lname\" value=\"Doe\"><br><br>\r\n");
	SocketWriteData(mySocketFD,	"  <input type=\"submit\" value=\"Submit\">\r\n");
	SocketWriteData(mySocketFD,	"</form>\r\n");
}

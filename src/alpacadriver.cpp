//**************************************************************************
//*	Name:			alpacadriver.cpp
//*
//*	Author:			Mark Sproul (C) 2019-2023
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
//*	Apr  9,	2019	<MLS> Created alpacadriver.c
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
//*	Aug 30,	2019	<MLS> Switching to C++
//*	Aug 30,	2019	<MLS> Started on alpaca driver base class
//*	Oct  4,	2019	<MLS> cAlpacaDeviceNum is now set correctly on class creation
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
//*	Feb 25,	2023	<MLS> Added SendHtml_CompiledInfo()
//*	Mar  2,	2023	<MLS> Re-arranged creation of driver objects
//*	Mar  3,	2023	<MLS> Added ProcessAlpacaCommand()
//*	Mar 11,	2023	<MLS> Added kRequestType_HTML()
//*	Mar 12,	2023	<MLS> Added OutputHTML_html()
//*	Mar 12,	2023	<MLS> Added SendFileToSocket()
//*	Mar 12,	2023	<MLS> Can now serve standard HTML files from folder "html"
//*	May  3,	2023	<MLS> Fixed jpg/png image requests to look in "docs" folder if not found
//*	May  7,	2023	<MLS> Added comment argument to GetCommandArgumentString
//*	May  7,	2023	<MLS> Added OuputCommandTableNotes()
//*	Jun 13,	2023	<MLS> IMU support now allows failure to initialize, i.e. cable disconnected
//*	Jun 18,	2023	<MLS> Peter Simpson published new commands for better async operation
//*	Jun 18,	2023	<MLS> https://download.ascom-standards.org/AsyncInterfaceChanges-Proposal1.pdf
//*	Jun 18,	2023	<MLS> Started implementing new commands
//*	Jun 18,	2023	<MLS> Added Get_DeviceState()
//*	Jun 18,	2023	<MLS> Moved all "connect" related routines to alpacadriverConnect.cpp
//*	Jun 18,	2023	<MLS> Added DeviceState_Add_Content()
//*	Jun 18,	2023	<MLS> Added log event if IMU fails to initialize
//*	Jul 14,	2023	<MLS> Added OutputHTML_CmdTable()
//*	Jul 17,	2023	<MLS> Added Get_WatchDogEnabled()
//*	Jul 17,	2023	<MLS> Made WatchDog timer optional and defaults to OFF
//*	Jul 18,	2023	<MLS> Added watchdog table to main web page
//*	Jul 22,	2023	<MLS> Added Get_WatchDogTimeout()
//*	Sep  6,	2023	<MLS> Added all the device version info to Get_Readall_Common()
//*	Sep 12,	2023	<MLS> Added OutputHTML_ClassDocs()
//*	Sep 13,	2023	<MLS> Fixed HTML header bug when sending PNG file SendJpegResponse()
//*	Sep 20,	2023	<MLS> Adding optional thread to driver base class
//*	Nov  1,	2023	<MLS> Fixed git hub issue #29 - issues with
//*	Nov  1,	2023	<MLS> /management/v1/description?ClientID=1&ClientTransactionID=2
//*	Nov 26,	2023	<MLS> Removed links form HTML cmd table for cmds starting with "-"
//*	Apr 10,	2024	<MLS> Started on global GPS support
//*	Apr 22,	2024	<MLS> Started working on multi-language support
//*	Apr 25,	2024	<MLS> Successfully got JavaScript to send HTTP PUT command
//*	Apr 25,	2024	<MLS> Added ProcessOptionsCommand()
//*	Apr 28,	2024	<MLS> Fixed ProcessGetPutRequest() to properly handle image requests
//*	May 17,	2024	<MLS> Added features to allow returning http error codes like 400
//*	May 18,	2024	<MLS> Added _DEBUG_MANAGEMENT_
//*	May 18,	2024	<MLS> Fixed cSendJSONresponse bug, not initialized to true
//*****************************************************************************
//*	to install code blocks 20
//*	Step 1: sudo add-apt-repository ppa:codeblocks-devs/release
//*	Step 2: sudo apt-get update
//*	Step 3: sudo apt-get install codeblocks codeblocks-contrib
//*****************************************************************************
//		 getrusage() - get resource usage
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	<sys/time.h>
#include	<sys/resource.h>
#include	<time.h>
#include	<gnu/libc-version.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<unistd.h>

#ifdef _ENABLE_FITS_
	#ifndef _FITSIO_H
		#include <fitsio.h>
	#endif // _FITSIO_H
#endif // _ENABLE_FITS_
#ifdef _ENABLE_JPEGLIB_
	#include	<jpeglib.h>
#endif


#include <pthread.h>

#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"JsonResponse.h"
#include	"alpacadriver.h"
#include	"alpacadriver_gps.h"
#include	"alpacadriver_helper.h"
#include	"eventlogging.h"
#include	"socket_listen.h"
#include	"discoverythread.h"
#include	"html_common.h"
#include	"observatory_settings.h"
#include	"obsconditions_globals.h"
#include	"cpu_stats.h"

//#define _DEBUG_CONFORM_
//#define	_SHOW_HTTP_DATA_

//#define	_DEBUG_HTML_
//#define _DEBUG_MANAGEMENT_


#ifdef _ENABLE_CAMERA_
	#include	"cameradriver.h"
#endif

#ifdef	_ENABLE_DOME_
	#include	"domedriver.h"
#endif


#ifdef	_ENABLE_FILTERWHEEL_
	#include	"filterwheeldriver.h"
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

#ifdef _ENABLE_CALIBRATION_
	#include	"calibrationdriver.h"
#endif

#ifdef _ENABLE_TELESCOPE_
	#include	"telescopedriver.h"
#endif

#ifdef _ENABLE_SHUTTER_
	#include	"shutterdriver_arduino.h"
#endif

#ifdef _ENABLE_SLIT_TRACKER_
	#include	"slittracker.h"
#endif

#ifdef _ENABLE_IMU_
	#include "imu_lib.h"
	bool	gIMUisOnLine	=	false;
#endif
#ifdef _ENABLE_SPECTROGRAPH_
	#include	"spectrodriver.h"
#endif

#ifdef _ENABLE_GLOBAL_GPS_
	#include	"gps_data.h"
#endif

#if defined(__arm__) && defined(_ENABLE_WIRING_PI_)
	#include <wiringPi.h>
#endif


#include	"managementdriver.h"
#include	"common_AlpacaCmds.h"
#include	"common_AlpacaCmds.cpp"


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
int				gAlpacaListenPort							=	kAlpacaPiDefaultPORT;	//*	6800 is the default
uint32_t		gClientID									=	1;
uint32_t		gServerTransactionID						=	1;		//*	we are the server, we will increment this each time a transaction occurs
bool			gErrorLogging								=	false;	//*	write errors to log file if true
bool			gConformLogging								=	false;	//*	log all commands to log file to match up with Conform
bool			gImageDownloadInProgress					=	false;
char			gHostName[48]								=	"";
char			gUserAgentAlpacaPiStr[80]					=	"";
int				gUserAgentCounters[kHTTPclient_last];
int				gHTTP_OptionsRequestCnt						=	0;

#ifdef _ENABLE_BANDWIDTH_LOGGING_
	int				gTimeUnitsSinceTopOfHour	=	0;
#endif // _ENABLE_BANDWIDTH_LOGGING_

#ifdef _ENABLE_LIVE_CONTROLLER_
	static void	HandleContollerWindow(AlpacaDriver *alpacaObjPtr);
#endif // _ENABLE_LIVE_CONTROLLER_


static void	OutputHTML_Form(TYPE_GetPutRequestData *reqData);
static void	OutputHTML_html(TYPE_GetPutRequestData *reqData);
static void	SendHtml_CompiledInfo(const int socketFD);


//*****************************************************************************
static void	InitDeviceList(void)
{
int		iii;

	for (iii=0; iii<kMaxDevices; iii++)
	{
		gAlpacaDeviceList[iii]	=	NULL;
	}
	gDeviceCnt	=	0;
}

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

	GetDeviceTypeFromEnum(argDeviceType, cAlpacaDeviceString);
	CONSOLE_DEBUG_W_STR("cAlpacaDeviceString\t=", cAlpacaDeviceString);
	ToLowerStr(cAlpacaDeviceString);
	CONSOLE_DEBUG_W_STR("cAlpacaDeviceString\t=", cAlpacaDeviceString);

	strcpy(cCommonProp.Name,		cAlpacaDeviceString);
	sprintf(cCommonProp.DriverVersion, "%s Build %d", kVersionString, kBuildNumber);
	cCommonProp.Connected		=	false;
	cCommonProp.Connecting		=	false;

	cDeleteMe					=	false;
	cRunStartupOperations		=	true;
	cVerboseDebug				=	false;
	cSendJSONresponse			=	true;
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
	cWatchDogEnabled			=	false;
	cWatchDogTimeOut_Minutes	=	5;				//*	default timeout, can be overridden
	cTimeOfLastValidCmd			=	time(NULL);		//*	these need to be set or it will do a shutdown before it even starts
	cTimeOfLastWatchDogCheck	=	time(NULL);
	strcpy(cWatchDogTimeOutAction, "-not defined-");

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
	cAlpacaDeviceNum		=	alpacaDeviceNum;
//	CONSOLE_DEBUG_W_NUM("cAlpacaDeviceNum\t=", cAlpacaDeviceNum);


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

	//-----------------------------------------------------------------
	cDriverThreadIsActive		=	false;
	cDriverThreadKeepRunning	=	false;
	cDriverThreadID				=	0;

#ifdef _ENABLE_BANDWIDTH_LOGGING_
	BandWidthStatsInit();
#endif // _ENABLE_BANDWIDTH_LOGGING_
	CONSOLE_DEBUG_W_BOOL("cSendJSONresponse\t=", cSendJSONresponse);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
AlpacaDriver::~AlpacaDriver( void )
{
int	iii;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cDriverThreadIsActive || cDriverThreadKeepRunning)
	{
		cDriverThreadKeepRunning	=	false;
		usleep(500 * 1000);	//*	give the thread time to quit
	}

	cMagicCookie				=	0;
	//*	remove this device from the list
	for (iii=0; iii<kMaxDevices; iii++)
	{
		if (gAlpacaDeviceList[iii] == this)
		{
			gAlpacaDeviceList[iii]	=	NULL;
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
char				tempString[64];

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

		//--------------------------------------------------------------------------------------------------------
		//*	New commands as of Jun 18, 2023
		case kCmd_Common_Connect:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Connect(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Connect does not allow GET");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
			break;

		case kCmd_Common_Connecting:
			alpacaErrCode	=	Get_Connecting(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Common_Disconnect:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Disconnect(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidOperation;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Disconnect does not allow GET");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
			break;

		case kCmd_Common_DeviceState:
			alpacaErrCode	=	Get_DeviceState(reqData, alpacaErrMsg);
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

		case kCmd_Common_SetupDialog:
			CONSOLE_DEBUG("kCmd_Common_SetupDialog");
			Setup_ProcessCommand(reqData);
			cSendJSONresponse	=	false;
			alpacaErrCode		=	kASCOM_Err_Success;
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
			strcpy(tempString,	"Unrecognized command:");
			strcat(tempString,	reqData->deviceCommand);

			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, tempString);
			CONSOLE_DEBUG(alpacaErrMsg);
			CONSOLE_DEBUG_W_STR("deviceCommand\t=", reqData->deviceCommand);

			reqData->httpRetCode	=	400;
			strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
			DumpRequestStructure(__FUNCTION__, reqData);
//			CONSOLE_ABORT(__FUNCTION__);
			break;
	}
	return(alpacaErrCode);
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
TYPE_ASCOM_STATUS	AlpacaDriver::Get_WatchDogEnabled(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cWatchDogEnabled,
							INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_WatchDogTimeout(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cWatchDogTimeOut_Minutes,
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
	Get_WatchDogEnabled(	reqData, alpacaErrMsg, "watchdogenabled");
	Get_WatchDogTimeout(	reqData, alpacaErrMsg, "watchdogtimeout");

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Alpaca-Name",
									cAlpacaName,
									INCLUDE_COMMA);
	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"DeviceModel",
									cDeviceModel,
									INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"DeviceManufacturer",
									cDeviceManufacturer,
									INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"DeviceSerialNum",
									cDeviceSerialNum,
									INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"DeviceVersion",
									cDeviceVersion,
									INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"DeviceFirmwareVersStr",
									cDeviceFirmwareVersStr,
									INCLUDE_COMMA);



//	CONSOLE_DEBUG("exit");
	return(kASCOM_Err_Success);
}


//*****************************************************************************
//{
//	"Value":[
//				{"Name":"CameraState","Value":0},
//				{"Name":"CCDTemperature","Value":10},
//				{"Name":"CoolerPower","Value":0},
//				{"Name":"HeatSinkTemperature","Value":10},
//				{"Name":"ImageReady","Value":false},
//				{"Name":"IsPulseGuiding","Value":false},
//				{"Name":"PercentCompleted","Value":0},
//				{"Name":"TimeStamp","Value":"2023-06-14T11:17:50.0Z"}
//			],
//	"ClientTransactionID":123,
//	"ServerTransactionID":456,
//	"ErrorNumber":0,
//	"ErrorMessage":""
//}

//*****************************************************************************
//*	returns the number of true if implemented
//*	this is so the Get_DeviceState knows if it is finished
//*****************************************************************************
bool	AlpacaDriver::DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen)
{
	CONSOLE_DEBUG("This should be over-loaded");
	return(false);
}


//*****************************************************************************
void	AlpacaDriver::DeviceState_Add_Bool(	const int		socketFD,
											char			*jsonTextBuffer,
											const int		maxLen,
											const char		*name,
											const bool		boolValue,
											const bool		includeComa)
{
char	jsonLineBuff[128];

	sprintf(jsonLineBuff, "\t\t\t{\"Name\":\"%s\",\"Value\":%s}", name, (boolValue ? "true" : "false"));
	if (includeComa)
	{
		strcat(jsonLineBuff, ",");
	}
//	CONSOLE_DEBUG(jsonLineBuff);
	strcat(jsonLineBuff, "\r\n");
	JsonResponse_Add_RawText(socketFD, jsonTextBuffer, maxLen, jsonLineBuff);
}

//*****************************************************************************
void	AlpacaDriver::DeviceState_Add_Dbl(const int socketFD, char *jsonTextBuffer, const int maxLen, const char *name, const double dblValue, const bool includeComa)
{
char	jsonLineBuff[128];

	sprintf(jsonLineBuff, "\t\t\t{\"Name\":\"%s\",\"Value\":%f}", name, dblValue);
	if (includeComa)
	{
		strcat(jsonLineBuff, ",");
	}
//	CONSOLE_DEBUG(jsonLineBuff);
	strcat(jsonLineBuff, "\r\n");
	JsonResponse_Add_RawText(socketFD, jsonTextBuffer, maxLen, jsonLineBuff);
}

//*****************************************************************************
void	AlpacaDriver::DeviceState_Add_Int(const int socketFD, char *jsonTextBuffer, const int maxLen, const char *name, const int intValue, const bool includeComa)
{
char	jsonLineBuff[128];

	sprintf(jsonLineBuff, "\t\t\t{\"Name\":\"%s\",\"Value\":%d}", name, intValue);
	if (includeComa)
	{
		strcat(jsonLineBuff, ",");
	}
//	CONSOLE_DEBUG(jsonLineBuff);
	strcat(jsonLineBuff, "\r\n");
	JsonResponse_Add_RawText(socketFD, jsonTextBuffer, maxLen, jsonLineBuff);
}

//*****************************************************************************
void	AlpacaDriver::DeviceState_Add_Str(	const int		socketFD,
											char			*jsonTextBuffer,
											const int		maxLen,
											const char		*name,
											const char		*valueStr,
											const bool		includeComa)
{
char	jsonLineBuff[128];

	sprintf(jsonLineBuff, "\t\t\t{\"Name\":\"%s\",\"Value\":\"%s\"}", name, valueStr);
	if (includeComa)
	{
		strcat(jsonLineBuff, ",");
	}
//	CONSOLE_DEBUG(jsonLineBuff);
	strcat(jsonLineBuff, "\r\n");
	JsonResponse_Add_RawText(socketFD, jsonTextBuffer, maxLen, jsonLineBuff);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_DeviceState(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
char				timeStampString[64];
struct timeval		currentTime;		//*	time exposure or video was started for frame rate calculations
bool				contentFinished;

//	CONSOLE_DEBUG(__FUNCTION__);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_ArrayStart(	reqData->socket,
																reqData->jsonTextBuffer,
																kMaxJsonBuffLen,
																gValueString);
	strcat(reqData->jsonTextBuffer, "\r\n");	//*	make it look pretty

	//*	now let the driver add it's specific information
	contentFinished	=	DeviceState_Add_Content(reqData->socket, reqData->jsonTextBuffer, kMaxJsonBuffLen);

	gettimeofday(&currentTime, NULL);
	FormatTimeStringISO8601(&currentTime, timeStampString);
	DeviceState_Add_Str(reqData->socket,
						reqData->jsonTextBuffer,
						kMaxJsonBuffLen,
						"TimeStamp",
						timeStampString,
						false);


	cBytesWrittenForThisCmd	+=	JsonResponse_Add_ArrayEnd(		reqData->socket,
																reqData->jsonTextBuffer,
																kMaxJsonBuffLen,
																INCLUDE_COMMA);
	if (contentFinished)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "DeviceState not finished");
	}
	return(alpacaErrCode);
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
							"hardware",
							gHardwareString,
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
	int	usbMemoryMB;

		usbMemoryMB	=	atoi(textBuff);
		JsonResponse_Add_Int32(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"usbfs_memory_mb",
								usbMemoryMB,
								INCLUDE_COMMA);
//		JsonResponse_Add_String(reqData->socket,
//								reqData->jsonTextBuffer,
//								kMaxJsonBuffLen,
//								"usbfs_memory_mb",
//								textBuff,
//								INCLUDE_COMMA);
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
void	AlpacaDriver::OutputHTML_CmdTable(TYPE_GetPutRequestData *reqData)
{
	if (cDriverCmdTablePtr != NULL)
	{
		//*	now generate links to all of the commands
		GenerateHTMLcmdLinkTable(reqData->socket, cAlpacaDeviceString, 0, cDriverCmdTablePtr);
	}
}

//*****************************************************************************
void	AlpacaDriver::OutputCommadTable(int mySocketFD, const char *title, const TYPE_CmdEntry *commandTable)
{
int		iii;
char	lineBuff[1024];
char	cmdArgumentStr[256];
char	commentString[256];
bool	useAlternateColor;
bool	foundArgFlag;

	useAlternateColor	=	false;
	sprintf(lineBuff,	"<TR><TD COLSPAN=5><B><CENTER>%s</B></TD></TR>", title);
	SocketWriteData(mySocketFD,	lineBuff);
	iii	=	0;
	while (commandTable[iii].enumValue >= 0)
	{
		if (commandTable[iii].commandName[0] == '-')
		{
			SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=5><HR></TD></TR>\r\n");
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

			cmdArgumentStr[0]	=	0;
			commentString[0]	=	0;

			foundArgFlag	=	GetCommandArgumentString(commandTable[iii].enumValue, cmdArgumentStr, commentString);
			if (foundArgFlag == false)
			{
				AlpacaDriver::GetCommandArgumentString(commandTable[iii].enumValue, cmdArgumentStr, commentString);
			}
			sprintf(lineBuff,	"<TD>%s</TD>\r\n<TD>%s</TD>\r\n", cmdArgumentStr, commentString);
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
bool	AlpacaDriver::GetCommandArgumentString(const int cmdENum, char *agumentString, char *commentString)
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

		sprintf(lineBuff,	"<TR><TD COLSPAN=5><CENTER><H2>%s</H2></TD></TR>",	cAlpacaName);
		SocketWriteData(mySocketFD,	lineBuff);

		SocketWriteData(mySocketFD,	"<TR><TH>command</TH><TH COLSPAN=2><CENTER>GET / PUT</TH><TH>Alpaca data sting</TH><TH>Comments</TH></TR>");

		//-----------------------------------------------
		//*	first do the common commands
		OutputCommadTable(mySocketFD,	"Common Commands",			gCommonCmdTable);
		//-----------------------------------------------
		//*	Now do the device specific commands
		OutputCommadTable(mySocketFD,	"Device specific Commands", cDriverCmdTablePtr);

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		OuputCommandTableNotes(mySocketFD);
	}
}

//*****************************************************************************
void	AlpacaDriver::OuputCommandTableNotes(int mySocketFD)
{
	//*	do nothing, can be over-ridden
}

//*****************************************************************************
bool	AlpacaDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
	CONSOLE_DEBUG_W_STR("This function should be over-ridden", cAlpacaName);
	CONSOLE_DEBUG_W_STR("This function should be over-ridden", cDeviceModel);
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
		CONSOLE_DEBUG_W_NUM("cmdNum       \t=", cmdNum);
		CONSOLE_DEBUG_W_STR("getput       \t=", ((getput == 'G') ? "GET" : "PUT"));
		CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
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
	"HTTP/1.0 404 Bad Request\r\n"
//	"Server: AkamaiGHost\r\n"
	"Server: AlpacaPi\r\n"
	"Mime-Version: 1.0\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 207\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<!DOCTYPE html>\r\n"
	"<HTML lang=\"en\">\r\n"
	"<HEAD>\r\n"
	"<TITLE>404 Not Found</TITLE>\r\n"
	"</HEAD>\r\n"
	"<BODY>\r\n"
	"<H1>Invalid URL</H1>\r\n"
	"The requested URL , is invalid.<p>\r\n"
//	"Reference&#32;&#35;9&#46;d0fb4317&#46;1555413904&#46;e81982\r\n"
	"</BODY>\r\n"
	"</HTML>\r\n"
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
	"Access-Control-Allow-Origin: *\r\n"
	"\r\n"
	"<!DOCTYPE html>\r\n"
	"<HTML lang=\"en\">\r\n"
	"<HEAD>\r\n"



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
char		lineBuffer[256];
int			mySocketFD;
struct stat	fileStatus;
int			returnCode;

		mySocketFD	=	reqData->socket;

		SocketWriteData(mySocketFD,	gHtmlHeader);

		sprintf(lineBuffer, "<TITLE>%s-%s</TITLE>\r\n", kApplicationName, kVersionString);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	gHtmlNightMode);

		SocketWriteData(mySocketFD,	"</HEAD><BODY>\r\n<CENTER>\r\n");
		sprintf(lineBuffer, "<H1>%s<BR>%s Build #%d </H1>\r\n", kApplicationName, kVersionString, kBuildNumber);
		SocketWriteData(mySocketFD,	lineBuffer);

		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		SocketWriteData(mySocketFD,	"<FONT SIZE=5>\r\n");
		SocketWriteData(mySocketFD,	"<UL>\r\n");

		SocketWriteData(mySocketFD,	"<LI><A HREF=setup>AlpacaPi Settings for this server</A>\r\n");
#ifdef _ENABLE_GLOBAL_GPS_
		if (gEnableGlobalGPS)
		{
			SocketWriteData(mySocketFD,	"<LI><A HREF=gps>GPS Statistics for GPS connected to this server</A>\r\n");
		}
#endif // _ENABLE_GLOBAL_GPS_
		SocketWriteData(mySocketFD,	"<P>\r\n");

		//*	check to see if the docs folder and index file are present
		returnCode	=	stat("docs/index.html", &fileStatus);		//*	fstat - check for existence of file
		if (returnCode == 0)
		{
			SocketWriteData(mySocketFD,
				"<LI><A HREF=docs/index.html target=github>AlpacaPi Documentation on this server</A>\r\n");
		}
//		else
//		{
//			CONSOLE_DEBUG("docs/index.html NOT found");
//		}
		SocketWriteData(mySocketFD,
				"<LI><A HREF=https://msproul.github.io/AlpacaPi/ target=github>AlpacaPi Documentation on github</A>\r\n");

		SocketWriteData(mySocketFD,
				"<LI><A HREF=https://github.com/msproul/AlpacaPi target=github>Download AlpacaPi from github</A>\r\n");



		SocketWriteData(mySocketFD,	"</UL>\r\n");
		SocketWriteData(mySocketFD,	"</FONT>\r\n");

		SocketWriteData(mySocketFD,	"<P>\r\n");

		sprintf(lineBuffer, "Your IP address is %s\r\n", reqData->clientIPaddr);
		SocketWriteData(mySocketFD,	lineBuffer);
}


//*****************************************************************************
static const char	*gURLlist[]	=
{
		"https://github.com/msproul/AlpacaPi",
		"https://msproul.github.io/AlpacaPi/",
		"http://www.skychariot.com/dome/",
		"https://ascom-standards.org/AlpacaDeveloper/Index.htm",
		"https://ascom-standards.org/api/",

//		"https://astronomy-imaging-camera.com/software-drivers",

//		"https://agenaastro.com/zwo-astronomy-cameras-buyers-guide.html",
//		"https://agenaastro.com/articles/guides/zwo-buyers-guide.html",
//		"https://agenaastro.com/articles/zwo-astronomy-cameras-buyers-guide.html",


		""
};

//*****************************************************************************
static void	SendSeparateLine(const int socketFD)
{
	SocketWriteData(socketFD,	"<HR SIZE=4 COLOR=RED>\r\n");
}

//*****************************************************************************
static const char	gWatchDogHelpMsg[]	=
{
	"<P>"
	"The watchdog timeout is triggered when no commands are received by the "
	"driver for the time period specifed. "
	"The primary intent of this is to go safe-mode if communications is lost to the controlling system. "
	"The only drivers currently utilizing this feature are Dome_RPI and coverCalibration "
	"The dome watchdog can be disabled in the dome setup page. "
	"<P>"

};

//*****************************************************************************
static const char	gLanguageSelection[]	=
{
//	"<label for=\"language\">Language:</label>"
//	"<select name=\"Language\" id=\"language\">"
//	"<option value=\"english\">English</option>"
//	"<option value=\"spanish\">Spanish</option>"
//	"<option value=\"chinese\">Chinese</option>"
//	"</select>"

	"<div id=\"google_translate_element\"></div>\n"
	"<script type=\"text/javascript\">\n"
	"	function googleTranslateElementInit()\n"
	"	{\n"
	"		new google.translate.TranslateElement(\n"
	"			{pageLanguage: 'en'},\n"
	"			'google_translate_element');\n"
	"	}\n"
	"</script>\n"

	"<script type=\"text/javascript\"\n"
	"	src=\"https://translate.google.com/translate_a/element.js?cb=googleTranslateElementInit\">\n"
	"</script>\n"
};

//*****************************************************************************
static void	SendHtml_MainPage(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
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
#ifdef _INCLUDE_MULTI_LANGUAGE_SUPPORT_
		SocketWriteData(mySocketFD,	gLanguageSelection);
#endif // _INCLUDE_MULTI_LANGUAGE_SUPPORT_

		OutPutObservatoryInfoHTML(mySocketFD);

		SocketWriteData(mySocketFD,	"<A HREF=../log target=log>Click here for log</A><BR>\r\n");
		SocketWriteData(mySocketFD,	"<A HREF=../stats target=log>Click here for stats</A><BR>\r\n");

	#ifdef	_ENABLE_CAMERA_
		SocketWriteData(mySocketFD,	"Camera support is enabled<BR>\r\n");
	#endif

	#ifdef	_ENABLE_DOME_
		SocketWriteData(mySocketFD,	"Dome support is enabled<BR>\r\n");
	#endif
	#ifdef	_ENABLE_DOME_ROR_
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

		SendSeparateLine(mySocketFD);
		//=============================================================================
		//*	print out a table of active devices
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");


		SocketWriteData(mySocketFD,	"The following devices are configured on this Remote Server:<P>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=2>\r\n");
		//*	do the header row
		SocketWriteData(mySocketFD,	"\t<TR>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Setup</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device Type</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Dev Num</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device Name</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Description</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Cmds / Errs</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>CPU (ms)</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>CPU (nano-secs)</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t</TR>\r\n");

		//------------------------------------------------------------------------
		//*	output the main device grid listing
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
												gAlpacaDeviceList[iii]->cAlpacaDeviceString,
												gAlpacaDeviceList[iii]->cAlpacaDeviceNum,
												gAlpacaDeviceList[iii]->cAlpacaName);
					SocketWriteData(mySocketFD,	lineBuffer);
				}
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
					SocketWriteData(mySocketFD,	gAlpacaDeviceList[iii]->cAlpacaName);
				SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

				sprintf(lineBuffer, "<TD><CENTER>%d</TD>\r\n", gAlpacaDeviceList[iii]->cAlpacaDeviceNum);
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
			#if (__LONG_LONG_WIDTH__ == 64)
				sprintf(lineBuffer, "<TD><CENTER>%lu</TD>\r\n", gAlpacaDeviceList[iii]->cTotalNanoSeconds);
			#else
				sprintf(lineBuffer, "<TD><CENTER>%lu</TD>\r\n", gAlpacaDeviceList[iii]->cTotalNanoSeconds);
			#endif
				SocketWriteData(mySocketFD,	lineBuffer);

				SocketWriteData(mySocketFD,	"\t</TR>\r\n");

			}
		}

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		//-------------------------------------------------------------
		//*	output html showing watchdog status
		SocketWriteData(mySocketFD,	"<P>\r\n");
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"WatchDog settings<P>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=2>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Device</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Watchdog</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Timeout (minutes)</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t\t<TH><FONT COLOR=yellow>Timeout Action</TH>\r\n");
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
				if (gAlpacaDeviceList[iii]->cDeviceType != kDeviceType_Management)
				{
					SocketWriteData(mySocketFD,	"\t<TR>\r\n");
					SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
						SocketWriteData(mySocketFD,	gAlpacaDeviceList[iii]->cAlpacaName);
					SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");

					//*	is Watchdog enabled
					if (gAlpacaDeviceList[iii]->cWatchDogEnabled)
					{
						SocketWriteData(mySocketFD,	"\t\t<TD><FONT COLOR=GREEN>Enabled</TD>\r\n");
					}
					else
					{
						SocketWriteData(mySocketFD,	"\t\t<TD><FONT COLOR=RED>Disabled</TD>\r\n");
					}
					sprintf(lineBuffer, "<TD><CENTER>%d</TD>\r\n",
												gAlpacaDeviceList[iii]->cWatchDogTimeOut_Minutes);
					SocketWriteData(mySocketFD,	lineBuffer);
					SocketWriteData(mySocketFD,	"\t\t<TD>\r\n");
						SocketWriteData(mySocketFD,	gAlpacaDeviceList[iii]->cWatchDogTimeOutAction);
					SocketWriteData(mySocketFD,	"\t\t</TD>\r\n");


					SocketWriteData(mySocketFD,	"\t</TR>\r\n");
				}
			}
		}

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
		SocketWriteData(mySocketFD,	gWatchDogHelpMsg);


		//-------------------------------------------------------------
		//*	Output the html for each device
		for (iii=0; iii<gDeviceCnt; iii++)
		{
//			CONSOLE_DEBUG_W_STR(__FUNCTION__, gAlpacaDeviceList[iii]->cCommonProp.Name);
			if (gAlpacaDeviceList[iii] != NULL)
			{
				SendSeparateLine(mySocketFD);
				gAlpacaDeviceList[iii]->OutputHTML(reqData);
				gAlpacaDeviceList[iii]->OutputHTML_Part2(reqData);
				gAlpacaDeviceList[iii]->OutputHTML_CmdTable(reqData);
			}
		}

		//**********************************************************
		SendSeparateLine(mySocketFD);

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
		SendSeparateLine(mySocketFD);
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
		SendSeparateLine(mySocketFD);
		SendHtml_CompiledInfo(mySocketFD);

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
static void	SendHtml_CompiledInfo(const int socketFD)
{
	SocketWriteData(socketFD,	"Compiled on ");
	SocketWriteData(socketFD,	__DATE__);
	SocketWriteData(socketFD,	"\r\n<BR>");
	SocketWriteData(socketFD,	"Written in C/C++\r\n<BR>");
	SocketWriteData(socketFD,	"(C) 2019-2023 by Mark Sproul msproul@skychariot.com\r\n<BR>");
}

//*****************************************************************************
static void	SendHtml_Stats(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
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
				SendSeparateLine(mySocketFD);
				gAlpacaDeviceList[iii]->OutputHTML_CmdStats(reqData);
			}
		}


		SendSeparateLine(mySocketFD);
		SendHtml_CompiledInfo(mySocketFD);

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
				SendSeparateLine(mySocketFD);
				gAlpacaDeviceList[iii]->OutputHTML_DriverDocs(reqData);
			}
		}

		SendSeparateLine(mySocketFD);
		SendHtml_CompiledInfo(mySocketFD);

		SocketWriteData(mySocketFD,	"</BODY></HTML>\r\n");
	}
	else
	{
	//	CONSOLE_DEBUG("reqData is NULL");
	}
}

//*****************************************************************************
static void	OutputHTML_ClassSize(int socketFD, const char *className, size_t classSize)
{
char	lineBuffer[256];
long	deltaSize;
long	myClassSize;

	deltaSize	=	classSize - sizeof(AlpacaDriver);
	myClassSize	=	classSize;
	if (deltaSize > 0)
	{
		sprintf(lineBuffer,	"<TR><TD>%s</TD><TD><CENTER>%ld</TD><TD><CENTER>%ld</TD></TR>\r\n",	className, myClassSize, deltaSize);
	}
	else
	{
		sprintf(lineBuffer,	"<TR><TD>%s</TD><TD><CENTER>%ld</TD></TR>\r\n",	className, myClassSize);
	}
	SocketWriteData(socketFD,	lineBuffer);

}

//*****************************************************************************
static void	OutputHTML_ClassDocs(TYPE_GetPutRequestData *reqData)
{
char	lineBuffer[256];
int		mySocketFD;

	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;
		SocketWriteData(mySocketFD,	gHtmlHeader);
		sprintf(lineBuffer, "<TITLE>%s</TITLE>\r\n", gWebTitle);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	gHtmlNightMode);
		SocketWriteData(mySocketFD,	"</HEAD><BODY>\r\n<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H1>Alpaca device driver Web server</H1>\r\n");
		sprintf(lineBuffer, "<H3>%s</H3>\r\n", "AlpacaPi Class object size");
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");


		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

		SocketWriteData(mySocketFD,	"<TR><TH>Class Name</TH><TH>Size</TH><TH>Delta</TH></TD>\r\n");

		//----------------------------------------
		OutputHTML_ClassSize(mySocketFD, "AlpacaDriver (base class)",	sizeof(AlpacaDriver));
#ifdef _ENABLE_CALIBRATION_
		OutputHTML_ClassSize(mySocketFD, "CalibrationDriver",	sizeof(CalibrationDriver));
#endif
#ifdef _ENABLE_CAMERA_
		OutputHTML_ClassSize(mySocketFD, "CameraDriver",		sizeof(CameraDriver));
#endif
#ifdef	_ENABLE_DOME_
		OutputHTML_ClassSize(mySocketFD, "DomeDriver",			sizeof(DomeDriver));
#endif
#ifdef	_ENABLE_FILTERWHEEL_
		OutputHTML_ClassSize(mySocketFD, "FilterwheelDriver",	sizeof(FilterwheelDriver));
#endif
#ifdef _ENABLE_FOCUSER_
		OutputHTML_ClassSize(mySocketFD, "FocuserDriver",		sizeof(FocuserDriver));
#endif
#ifdef _ENABLE_MULTICAM_
		OutputHTML_ClassSize(mySocketFD, "MultiCam",			sizeof(MultiCam));
#endif
#ifdef _ENABLE_OBSERVINGCONDITIONS_
		OutputHTML_ClassSize(mySocketFD, "ObsConditionsDriver",	sizeof(ObsConditionsDriver));
#endif
#ifdef _ENABLE_SWITCH_
		OutputHTML_ClassSize(mySocketFD, "SwitchDriver",		sizeof(SwitchDriver));
#endif
#ifdef _ENABLE_TELESCOPE_
		OutputHTML_ClassSize(mySocketFD, "TelescopeDriver",		sizeof(TelescopeDriver));
#endif
#ifdef _ENABLE_SHUTTER_
		OutputHTML_ClassSize(mySocketFD, "ShutterDriver",		sizeof(ShutterDriver));
#endif
#ifdef _ENABLE_SLIT_TRACKER_
		OutputHTML_ClassSize(mySocketFD, "SlitTrackerDriver",	sizeof(SlitTrackerDriver));
#endif
#ifdef _ENABLE_SPECTROGRAPH_
		OutputHTML_ClassSize(mySocketFD, "SpectrographDriver",	sizeof(SpectrographDriver));
#endif

		SocketWriteData(mySocketFD,	"<TR><TH>Other structures</TH><TH>Size</TH></TD>\r\n");

		OutputHTML_ClassSize(mySocketFD, "TYPE_CommonProperties",			sizeof(TYPE_CommonProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_CameraProperties",			sizeof(TYPE_CameraProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_CoverCalibrationProperties",	sizeof(TYPE_CoverCalibrationProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_FilterWheelProperties",		sizeof(TYPE_FilterWheelProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_FocuserProperties",			sizeof(TYPE_FocuserProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_RotatorProperties",			sizeof(TYPE_RotatorProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_ObsConditionProperties",		sizeof(TYPE_ObsConditionProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_DomeProperties",				sizeof(TYPE_DomeProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_SwitchProperties",			sizeof(TYPE_SwitchProperties));
		OutputHTML_ClassSize(mySocketFD, "TYPE_TelescopeProperties",		sizeof(TYPE_TelescopeProperties));

#ifdef _ENABLE_CAMERA_
		OutputHTML_ClassSize(mySocketFD, "TYPE_QHY_GPSdata",				sizeof(TYPE_QHY_GPSdata));
#endif
		OutputHTML_ClassSize(mySocketFD, "TYPE_BinaryImageHdr",				sizeof(TYPE_BinaryImageHdr));

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		SendSeparateLine(mySocketFD);
		SendHtml_CompiledInfo(mySocketFD);

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
		if ((gCommonCmdTable[iii].get_put == kCmdType_PUT) || (gCommonCmdTable[iii].commandName[0] == '-'))
		{
			sprintf(lineBuffer,	"\t<LI>%s\r\n", gCommonCmdTable[iii].commandName);
		}
		else
		{
			sprintf(lineBuffer,	"\t<LI><A HREF=../api/v1/%s/%d/%s target=cmd>%s</A>\r\n",
										deviceName,
										deviceNum,
										gCommonCmdTable[iii].commandName,
										gCommonCmdTable[iii].commandName);
		}
		SocketWriteData(socketFD,	lineBuffer);
		iii++;
	}

	SocketWriteData(socketFD,	"<P>\r\n");
	//----------------------------------------------------------------
	//*	now do the device specific command table
	iii	=	0;
	while (cmdTable[iii].commandName[0] != 0)
	{
		if ((cmdTable[iii].get_put ==  kCmdType_PUT) || (cmdTable[iii].commandName[0] == '-'))
		{
			sprintf(lineBuffer,	"\t<LI>%s\r\n", cmdTable[iii].commandName);
		}
		else
		{
			sprintf(lineBuffer,	"\t<LI><A HREF=../api/v1/%s/%d/%s target=cmd>%s</A>\r\n",
										deviceName,
										deviceNum,
										cmdTable[iii].commandName,
										cmdTable[iii].commandName);
		}
		SocketWriteData(socketFD,	lineBuffer);
//		CONSOLE_DEBUG(lineBuffer);
		iii++;
	}
	SocketWriteData(socketFD,	"</UL>\r\n");
}

//*****************************************************************************
const char	gHTML_HeaderJpeg[]	=
{
	"HTTP/1.0 200 ok\r\n"
//	"Server: alpaca\r\n"
	"Mime-Version: 1.0\r\n"
	"Content-Type: image/jpeg\r\n"
	"Connection: close\r\n"
	"\r\n"

};


//*****************************************************************************
const char	gHTML_HeaderPNG[]	=
{
	"HTTP/1.0 200 ok\r\n"
//	"Server: alpaca\r\n"
	"Mime-Version: 1.0\r\n"
	"Content-Type: image/png\r\n"
	"Connection: close\r\n"
	"\r\n"

};

//*****************************************************************************
//*	Open and sends fileName to socket as a binary stream
//*	returns bytes sent
//*****************************************************************************
static int	SendFileToSocket(int socket, const char *fileName)
{
FILE			*filePointer;
int				numRead;
int				retCode;
int				bytesWritten;
int				totalBytesWritten;
bool			keepGoing;
char			dataBuffer[1600];

	CONSOLE_DEBUG_W_STR("Sending file:", fileName);
	totalBytesWritten	=	0;
	filePointer	=	fopen(fileName, "r");
	if (filePointer != NULL)
	{
//		CONSOLE_DEBUG_W_STR("File is open:", fileName);
		keepGoing			=	true;
		while (keepGoing)
		{
			numRead	=	fread(dataBuffer, 1, 1500, filePointer);
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
		retCode	=	fclose(filePointer);
		if (retCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("fclose(filePointer) failed with rc\t=",	retCode);

		}
//		CONSOLE_DEBUG_W_STR("File is closed   \t=", fileName);
//		CONSOLE_DEBUG_W_NUM("totalBytesWritten\t=",	totalBytesWritten);
	}
	else
	{
		CONSOLE_DEBUG("Failed to open file");
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(totalBytesWritten);
}

//*****************************************************************************
static void	StrcpyUntilChar(char *destString, const char *srcString, const char stopChar, const int maxLen)
{
int		iii;

	iii	=	0;
	while ((srcString[iii] != stopChar) && (srcString[iii] > 0) && (iii < (maxLen - 1)))
	{
		destString[iii]	=	srcString[iii];
		iii++;
	}
	destString[iii]	=	0;
}

//*****************************************************************************
static void	SendJpegResponse(int socket, const char *jpegFileName)
{
int				totalBytesWritten;
char			myJpegFileName[256];
char			altJpegFileName[256];
char			*myFilenamePtr;
struct stat		fileStatus;
int				returnCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (strcasestr(jpegFileName, ".png") != NULL)
	{
//		CONSOLE_DEBUG("Sending PNG file!!!!!!!!!!!!!!");
		SocketWriteData(socket,	gHTML_HeaderPNG);
	}
	else
	{
//		CONSOLE_DEBUG("Sending JPEG file!!!!!!!!!!!!!!");
		SocketWriteData(socket,	gHTML_HeaderJpeg);
	}
	if (jpegFileName != NULL)
	{
//		CONSOLE_DEBUG_W_STR("jpegFileName\t=", jpegFileName);
		myFilenamePtr	=	(char *)jpegFileName;
		if (*myFilenamePtr == '/')
		{
			myFilenamePtr++;
		}
		StrcpyUntilChar(myJpegFileName, myFilenamePtr, 0x20, sizeof(myJpegFileName));
		//*	check to see if the file is present
		returnCode	=	stat(myJpegFileName, &fileStatus);
		if (returnCode != 0)
		{
			//*	ok, the file is not there, lets look in docs
			StrcpyUntilChar(altJpegFileName, jpegFileName, 0x20, sizeof(altJpegFileName));

			CONSOLE_DEBUG("Looking for file in 'docs' folder");
			strcpy(myJpegFileName, "docs");
			strcat(myJpegFileName, altJpegFileName);
			CONSOLE_DEBUG_W_STR("myJpegFileName\t=", myJpegFileName);
			returnCode	=	stat(myJpegFileName, &fileStatus);
			if ((returnCode == 0) && (fileStatus.st_mode & S_IFREG))
			{
				CONSOLE_DEBUG("File found")
			}
		}

		//------------------------------------------------------------------
		//*	make sure there is nothing past the extension
//		CONSOLE_DEBUG_W_STR("myJpegFileName=", myJpegFileName);
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
//		CONSOLE_DEBUG_W_STR("myJpegFileName=", myJpegFileName);
	}
	else
	{
		strcpy(myJpegFileName, "image.jpg");
	}
//	CONSOLE_DEBUG_W_STR("myJpegFileName=", myJpegFileName);
	totalBytesWritten	=	SendFileToSocket(socket, myJpegFileName);
	if (totalBytesWritten <= 0)
	{
		CONSOLE_DEBUG_W_STR("Failed to send file:", myJpegFileName);
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//*****************************************************************************
static TYPE_ASCOM_STATUS	ProcessAlpacaCommand(	AlpacaDriver			*alpacaDevice,
													TYPE_GetPutRequestData	*reqData,
													long					byteCount)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;

	if ((alpacaDevice != NULL) && (reqData != NULL))
	{

		alpacaDevice->cBytesWrittenForThisCmd	=	0;
		alpacaDevice->cHttpHeaderSent			=	false;

//		CONSOLE_DEBUG("Calling ProcessCommand() ---------------------------------------------");
//		CONSOLE_DEBUG_W_STR("cAlpacaName         \t=",	alpacaDevice->cAlpacaName);
//		CONSOLE_DEBUG_W_STR("deviceCommand       \t=",	reqData->deviceCommand);
		alpacaDevice->cSendJSONresponse	=	true;
		alpacaErrCode					=	alpacaDevice->ProcessCommand(reqData);
		if (alpacaErrCode == kASCOM_Err_Success)
		{
			//*	record the time of the last successful command
			//*	this is for watch dog timing
			alpacaDevice->cTimeOfLastValidCmd	=	time(NULL);
		}
		else
		{
			alpacaDevice->cTotalCmdErrors++;
		}
		alpacaDevice->cTotalCmdsProcessed++;
		alpacaDevice->cTotalBytesRcvd	+=	byteCount;

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
			alpacaDevice->cBW_CmdsReceived[gTimeUnitsSinceTopOfHour]	+=	1;
			alpacaDevice->cBW_BytesReceived[gTimeUnitsSinceTopOfHour]	+=	byteCount;
			alpacaDevice->cBW_BytesSent[gTimeUnitsSinceTopOfHour]		+=	alpacaDevice->cBytesWrittenForThisCmd;
		}
#endif // _ENABLE_BANDWIDTH_LOGGING_
	}

	return(alpacaErrCode);
}

//*****************************************************************************
static TYPE_ASCOM_STATUS	ProcessAlpacaAPIrequest(TYPE_GetPutRequestData	*reqData,
													long					byteCount)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
int					deviceTypeEnum;
int					iii;
bool				deviceFound;

#ifdef _DEBUG_MANAGEMENT_
	CONSOLE_DEBUG(__FUNCTION__);
#endif // _DEBUG_MANAGEMENT_

//	CONSOLE_DEBUG(__FUNCTION__);
//	if (strncmp(reqData->httpCmdString, "PUT", 3) == 0)
//	{
//		CONSOLE_DEBUG(__FUNCTION__);
//		DumpRequestStructure(__FUNCTION__, reqData);
//	}

	//*******************************************
	//*	now do something with the data
	deviceTypeEnum	=	FindDeviceTypeByStringLowerCase(reqData->deviceType);
	deviceFound		=	false;
	if (deviceTypeEnum != kDeviceType_undefined)
	{
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
			#ifdef _DEBUG_CONFORM_
			//	CONSOLE_DEBUG_W_NUM("gAlpacaDeviceList[iii]->cDeviceType\t=", gAlpacaDeviceList[iii]->cDeviceType);
			//	CONSOLE_DEBUG_W_NUM("gAlpacaDeviceList[iii]->cAlpacaDeviceNum\t=", gAlpacaDeviceList[iii]->cAlpacaDeviceNum);
			#endif // _DEBUG_CONFORM_

				if ((gAlpacaDeviceList[iii]->cDeviceType == deviceTypeEnum) &&
					(gAlpacaDeviceList[iii]->cAlpacaDeviceNum == reqData->deviceNumber))
				{
					deviceFound		=	true;
					alpacaErrCode	=	ProcessAlpacaCommand(gAlpacaDeviceList[iii], reqData, byteCount);

					break;
				}
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
					__FUNCTION__,
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

//	CONSOLE_DEBUG("/setup/ found");
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
					(gAlpacaDeviceList[iii]->cAlpacaDeviceNum == reqData->deviceNumber))
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
		CONSOLE_DEBUG("localtime() failed!!!!!!!!!!!!");
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
		currentTime		=	time(NULL);
		linuxTime		=	localtime(&currentTime);
		CONSOLE_DEBUG_W_NUM("tm_mday", linuxTime->tm_mday);
		sprintf(logFilename, "requestlog-%d-%d-%02d-%02d.txt",	gAlpacaListenPort,
																(1900 + linuxTime->tm_year),
																(1 + linuxTime->tm_mon),
																linuxTime->tm_mday);
//		CONSOLE_DEBUG("-------------------------------------------------------------------------");
		CONSOLE_DEBUG_W_STR("Open log file:", logFilename);
//		SETUP_TIMING();
		gIPlogFilePointer		=	fopen(logFilename, "a");
		gIPlogNeedsToBeOpened	=	false;
		gCurrentDayOfMonth		=	linuxTime->tm_mday;

		//*	record the fact that we opened the log file
		sprintf(lineBuff,	"%-18s\tLog file opened --------------------------------------------------------\r\n", datestring);
		bytesWritten	=	fprintf(gIPlogFilePointer,	"%s", lineBuff);
//		DEBUG_TIMING("Time to open log file (ms)\t=");
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
			CONSOLE_DEBUG_W_NUM("gCurrentDayOfMonth\t=",	gCurrentDayOfMonth);
			CONSOLE_DEBUG_W_NUM("tm_mday           \t=",	linuxTime->tm_mday);
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
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
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

#ifdef _DEBUG_HTML_
	CONSOLE_DEBUG(__FUNCTION__);
#endif

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
#ifdef _DEBUG_HTML_
		CONSOLE_DEBUG_W_NUM("htmlData length\t=", sLen);
		CONSOLE_DEBUG_W_SIZE("sizeof(lineBuff)\t=", sizeof(lineBuff));
#endif

		//*	keep a copy of the entire thing
		strcpy(reqData->htmlData, htmlData);

		//========================================================================
		//*	check for user agent
		reqData->cHTTPclientType	=   kHTTPclient_NotSpecified;
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
			//*	special case because "ConformUniversal/2.1.0+23787.e8effdcb04975452b0e7e529b87bcb851920d57a"
			//*	is too long for the log file
			if (strncmp(reqData->httpUserAgent, "ConformU", 8) == 0)
			{
			char	*plusPtr;

				plusPtr	=	strchr(reqData->httpUserAgent, '+');
				if (plusPtr != NULL)
				{
					*plusPtr	=	0;
				}
			}
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
			else if (strncasecmp(reqData->httpUserAgent, "Conform", 7) == 0)
			{
				reqData->clientIs_Conform	=	true;
				reqData->cHTTPclientType	=   kHTTPclient_ConfomU;
			}
			else if (strncasecmp(reqData->httpUserAgent, "curl", 4) == 0)
			{
				reqData->cHTTPclientType	=   kHTTPclient_Curl;
			}
			else if (strncasecmp(reqData->httpUserAgent, "Mozilla", 7) == 0)
			{
				reqData->cHTTPclientType	=   kHTTPclient_Mozilla;
//				CONSOLE_DEBUG_W_STR("User-Agent:\t=", reqData->httpUserAgent);
			}
			else if (strncasecmp(reqData->httpUserAgent, "RestSharp", 9) == 0)
			{
				reqData->cHTTPclientType	=   kHTTPclient_ASCOM_RestSharp;
				reqData->clientIs_Conform	=	true;
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
		}
		//-----------------------------------------------------------
		//*	bump the counters
		if ((reqData->cHTTPclientType >= 0) && (reqData->cHTTPclientType < kHTTPclient_last))
		{
			gUserAgentCounters[reqData->cHTTPclientType]++;
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("reqData->cHTTPclientType out of range", reqData->cHTTPclientType);
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
#ifdef _DEBUG_HTML_
//	DumpRequestStructure(__FUNCTION__, reqData);
#endif
}


//*****************************************************************************
typedef enum
{
	kRequestType_Invalid	=	-1,
	kRequestType_API		=	0,
	kRequestType_Docs,
	kRequestType_ClassDocs,
	kRequestType_DriverDocs,
	kRequestType_Log,
	kRequestType_Managment,
	kRequestType_Setup,
	kRequestType_Stats,
	kRequestType_Web,
	kRequestType_GPS,
	kRequestType_TopLevel,
	kRequestType_HTML,

	kRequestType_Form,

	kRequestType_last
} TYPE_REQUEST_TYPE;

//*****************************************************************************
typedef struct	//	TYPE_Request
{
	char				RequestString[16];
	TYPE_REQUEST_TYPE	RequstType;
} TYPE_Request;


//*****************************************************************************
static TYPE_Request	gRequestType[]	=
{
	{	"api",			kRequestType_API		},
	{	"docs",			kRequestType_Docs		},
	{	"classdocs",	kRequestType_ClassDocs	},
	{	"driverdocs",	kRequestType_DriverDocs	},
	{	"log",			kRequestType_Log		},
	{	"management",	kRequestType_Managment	},
	{	"setup",		kRequestType_Setup		},
	{	"stats",		kRequestType_Stats		},
	{	"web",			kRequestType_Web		},
	{	"gps",			kRequestType_GPS		},

	{	"form",			kRequestType_Form		},
	{	"html",			kRequestType_HTML		},

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

#ifdef _DEBUG_HTML_
	CONSOLE_DEBUG_W_STR("requestTypeString\t=", requestTypeString);
#endif

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
#ifdef _DEBUG_HTML_
	CONSOLE_DEBUG_W_NUM("requestType      \t=", requestType);
#endif
	return(requestType);
}

//*****************************************************************************
//*	Parse the full Alpaca request, trying to cover all variants
//*	returns ENUM of request type
//*****************************************************************************
static int	ParseAlpacaRequest(TYPE_GetPutRequestData *reqData)
{
TYPE_REQUEST_TYPE	requestType;
int					iii;
int					ccc;
bool				foundKeyWord;
int					slashCounter;
char				sLen;
char				theChar;
char				argumentString[256]			=	"";
char				myRequestTypeString[64]		=	"";
char				myAlpacaVersionString[64]	=	"";
char				myDeviceString[64]			=	"";
char				myDeviceNumString[64]		=	"";
char				myDeviceCmdString[256]		=	"";
int					cmdBuffLen;
char				*delimPtr;

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

	//CONSOLE_DEBUG_W_NUM("slashCounter\t=",	slashCounter);
	//---------------------------------------------------
	if (slashCounter >= 3)
	{
		//CONSOLE_DEBUG_W_STR("myAlpacaVersionString\t=",	myAlpacaVersionString);

		if (myAlpacaVersionString[0] == 'v')
		{
			reqData->alpacaVersion		=	myAlpacaVersionString[1] & 0x0f;
		}
		//CONSOLE_DEBUG_W_NUM("reqData->alpacaVersion\t=",	reqData->alpacaVersion);
		strcpy(reqData->deviceType,		myDeviceString);
	}

	if (slashCounter >= 4)
	{
//		CONSOLE_DEBUG("Extracting command string");
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
//		CONSOLE_DEBUG_W_NUM("cmdBuffLen            \t=",	cmdBuffLen);
//		CONSOLE_DEBUG_W_STR("myDeviceCmdString     \t=",	myDeviceCmdString);
//		CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=",	reqData->deviceCommand);

	}

	if (slashCounter > 5)
	{
		//*	check for valid device number,  CONFORMU throws -1 and "A"
		if (isdigit(myDeviceNumString[0]))
		{
			reqData->deviceNumber		=	atoi(myDeviceNumString);
		}
		else
		{
			//*	the device number is not a valid number
			CONSOLE_DEBUG("=====================================================");
			CONSOLE_DEBUG_W_STR("Device number string is invalid!!!", myDeviceNumString);
			reqData->deviceNumber	=	-1;
		}
	}
//	else if ((slashCounter >= 3) && (strncasecmp(myDeviceCmdString, "/management", 11) == 0))
	else if (slashCounter >= 3)
	{
		//	/management/v1/configureddevices
//		CONSOLE_DEBUG("=====================================================");
//		CONSOLE_DEBUG_W_NUM("Slash count >= 3", slashCounter);
//		CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=",	reqData->deviceCommand);
//		CONSOLE_DEBUG_W_STR("myDeviceCmdString     \t=",	myDeviceCmdString);
//		CONSOLE_DEBUG_W_NUM("slashCounter          \t=",	slashCounter);
//		CONSOLE_DEBUG_W_STR("myRequestTypeString   \t=",	myRequestTypeString);
//		CONSOLE_DEBUG_W_STR("myAlpacaVersionString \t=",	myAlpacaVersionString);
//		CONSOLE_DEBUG_W_STR("myDeviceString        \t=",	myDeviceString);
//		CONSOLE_DEBUG_W_STR("myDeviceNumString     \t=",	myDeviceNumString);
//		CONSOLE_DEBUG_W_STR("myDeviceCmdString     \t=",	myDeviceCmdString);
//		CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=",	reqData->deviceCommand);
	}
	else
	{
		strcpy(reqData->deviceType,		"unknown");
		reqData->deviceCommand[0]	=	0;
//		CONSOLE_DEBUG_W_STR("Unknown device type:", reqData->cmdBuffer);
	}

	//*	figure out the base type of the request (see enum list above)
	requestType					=	FindRequestType(myRequestTypeString);
	reqData->requestTypeEnum	=	requestType;

	//	/management/v1/configureddevices
	//	/management/apiversions
	if (requestType == kRequestType_Managment)
	{
#ifdef _DEBUG_MANAGEMENT_
		CONSOLE_DEBUG_W_NUM("slashCounter          \t=",	slashCounter);
		CONSOLE_DEBUG_W_STR("myRequestTypeString   \t=",	myRequestTypeString);
		CONSOLE_DEBUG_W_STR("myAlpacaVersionString \t=",	myAlpacaVersionString);
		CONSOLE_DEBUG_W_STR("myDeviceString        \t=",	myDeviceString);
		CONSOLE_DEBUG_W_STR("myDeviceNumString     \t=",	myDeviceNumString);
		CONSOLE_DEBUG_W_STR("myDeviceCmdString     \t=",	myDeviceCmdString);
		CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=",	reqData->deviceCommand);
#endif // _DEBUG_MANAGEMENT_

		if ((strlen(myAlpacaVersionString) > 3) && (strlen(myDeviceString) == 0))
		{
			strcpy(reqData->deviceCommand, myAlpacaVersionString);
		}
		else
		{
			strcpy(reqData->deviceCommand, myDeviceString);
		}
		//------------------------------------------------
		//*	https://github.com/msproul/AlpacaPi/issues
		//*	issue #29
		//*	look for delimiter characters
		delimPtr	=	strchr(reqData->deviceCommand, '?');
		if (delimPtr != NULL)
		{
			*delimPtr	=	0;
		}
		delimPtr	=	strchr(reqData->deviceCommand, '&');
		if (delimPtr != NULL)
		{
			*delimPtr	=	0;
		}
		delimPtr	=	strchr(reqData->deviceCommand, 0x20);
		if (delimPtr != NULL)
		{
			*delimPtr	=	0;
		}
//		CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=",	reqData->deviceCommand);
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
	reqData->ClientTransactionID	=	67890;	//*	set the default (fake out conformU)
	reqData->ClientTransactionID	=	1234;	//*	set the default (fake out conformU)
	foundKeyWord	=	GetKeyWordArgument(reqData->contentData, "ClientTransactionID", argumentString, 31, kIgnoreCase);
	if (foundKeyWord)
	{
	int	myClientTransactionID;

		//*	ConformU likes to play games and send negative numbers text strings, it expects a 0 in this case
		myClientTransactionID			=	atoi(argumentString);
		if (myClientTransactionID < 0)
		{
			myClientTransactionID	=	0;
		}
		reqData->ClientTransactionID	=	myClientTransactionID;
		strcpy(reqData->ClientTransactionIDstr,	argumentString);
	}
#ifdef _DEBUG_CONFORM_
	else
	{
//		CONSOLE_DEBUG("ClientTransactionID NOT FOUND");
	}
	CONSOLE_DEBUG_W_NUM("gClientID\t=", gClientID);

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
	reqData.httpRetCode			=	200;
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

//	if (requestType != kRequestType_API)
//	{
//		DumpRequestStructure(__FUNCTION__, &reqData);
//	}

#ifdef _DEBUG_MANAGEMENT_
	CONSOLE_DEBUG_W_NUM("requestType\t=", requestType);
#endif // _DEBUG_MANAGEMENT_
	alpacaErrCode	=	kASCOM_Err_Success;
	switch(requestType)
	{
		//*	standard ALPACA api call
		case kRequestType_API:
			//*	Mar  3,	2023	<MLS> Make CONFORMU happy, check for valid device number
			if (reqData.deviceNumber >= 0)
			{
				alpacaErrCode	=	ProcessAlpacaAPIrequest(&reqData, byteCount);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("Invalid device number\t=",	reqData.deviceNumber);
				DumpRequestStructure(__FUNCTION__, &reqData);
				SocketWriteData(socket,	gBadResponse400);
			}
			break;

		//*	statistics on class structure size
		case kRequestType_ClassDocs:
			OutputHTML_ClassDocs(&reqData);
			break;

		//*	extra self documentation
		case kRequestType_DriverDocs:
			OutputHTML_DriverDocs(&reqData);
			break;

		//*	extra - logging data
		case kRequestType_Log:
			SendHtmlLog(socket);
			break;

		//*	standard ALPACA management
		case kRequestType_Managment:
//			CONSOLE_DEBUG(__FUNCTION__);
			alpacaErrCode	=	ProcessManagementRequest(&reqData, byteCount);
			break;

		//*	standard ALPACA setup
		case kRequestType_Setup:
			alpacaErrCode	=	ProcessAlpacaSETUPrequest(&reqData, byteCount);
			break;

		//*	extra - stats
		case kRequestType_Stats:
			SendHtml_Stats(&reqData);
			break;

		case kRequestType_Web:
			SendHtml_MainPage(&reqData);
			break;

		case kRequestType_GPS:
			SendHtml_GPS(&reqData);
			break;

		case kRequestType_TopLevel:
			SendHtml_TopLevel(&reqData);
			break;

		//*	this outputs a real HTML file from folder html
		case kRequestType_HTML:
		case kRequestType_Docs:
			OutputHTML_html(&reqData);
			break;

		//*	this is for testing, will be deleted later
		case kRequestType_Form:
			OutputHTML_Form(&reqData);
			break;


		default:
//			CONSOLE_DEBUG_W_STR("parseChrPtr\t=", parseChrPtr);
			//-------------------------------------------------------------------
			if (strncasecmp(parseChrPtr,	"/favicon.ico", 12) == 0)
			{
//				CONSOLE_DEBUG("favicon.ico");
				SendJpegResponse(socket, "favicon.ico");
			}
			//-------------------------------------------------------------------
			else if (strncasecmp(parseChrPtr,	"/image.jpg", 10) == 0)
			{
//				CONSOLE_DEBUG("image.jpg");
				SendJpegResponse(socket, NULL);
			}
			//-------------------------------------------------------------------
			else if (strstr(parseChrPtr, ".jpg") != NULL)
			{
//				CONSOLE_DEBUG(".....jpg");
				SendJpegResponse(socket, parseChrPtr);
			}
			//-------------------------------------------------------------------
			else if (strstr(parseChrPtr, ".png") != NULL)
			{
//				CONSOLE_DEBUG(".....png");
				SendJpegResponse(socket, parseChrPtr);
			}
			else
			{
//				CONSOLE_DEBUG_W_STR("Unknown http request\t=",	htmlData);
				CONSOLE_DEBUG_W_STR("parseChrPtr\t=", parseChrPtr);
				DumpRequestStructure(__FUNCTION__, &reqData);
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
static void	ProcessOptionsCommand(const int socket)
{
char	optionsResponse[2048];
int		bytesWritten;

	CONSOLE_DEBUG(__FUNCTION__);
	gHTTP_OptionsRequestCnt++;

	strcpy(optionsResponse,	"HTTP/1.0 200 OK\r\n");
	strcat(optionsResponse,	"Content-Type: text/plain\r\n");
	strcat(optionsResponse,	"Allow: OPTIONS, GET, PUT, HEAD, POST\r\n");
	strcat(optionsResponse,	"Access-Control-Allow-Origin: *\r\n");
	strcat(optionsResponse,	"Access-Control-Allow-Headers: *\r\n");
	strcat(optionsResponse,	"Access-Control-Allow-Methods: GET, PUT, POST\r\n");
	strcat(optionsResponse,	"Connection: close\r\n");

//	CONSOLE_DEBUG_W_STR("Sending data:\r\n", optionsResponse);

	bytesWritten	=	SocketWriteData(socket,	optionsResponse);
	if (bytesWritten <= 0)
	{
		CONSOLE_DEBUG_W_NUM("bytesWritten\t=",	bytesWritten);
	}
}



//*****************************************************************************
//*	this function is called from the socket handler with the received data
//*	It will parse through the data checking all of the normal alpaca requirements
//*	and then call the appropriate function based on the device type
//*****************************************************************************
static int AlpacaCallback(const int socket, char *htmlData, long byteCount,  const char *ipAddressString)
{
int		returnCode	=	-1;

#ifdef _SHOW_HTTP_DATA_
CONSOLE_DEBUG_W_STR("\r\n", htmlData);

#endif

//	CONSOLE_DEBUG("Timing Start----------------------");
//	SETUP_TIMING();

//	if (strstr(htmlData, "favicon.ico") != NULL)
//	{
//		CONSOLE_DEBUG("favicon.ico request !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//	}


//	if (strncmp(htmlData, "GET /favicon.ico", 16) == 0)
//	{
//		//*	do nothing
//	}
//	else
	if ((strncmp(htmlData, "GET", 3) == 0) || (strncmp(htmlData, "PUT", 3) == 0))
	{
//		CONSOLE_DEBUG("Calling ProcessGetPutRequest");
		returnCode	=	ProcessGetPutRequest(socket, htmlData, byteCount, ipAddressString);
		gServerTransactionID++;	//*	we are the "server"
	}
	else if (strncmp(htmlData, "POST", 4) == 0)
	{
		ProcessPostCommand(socket);
		gServerTransactionID++;	//*	we are the "server"
	}
	else if (strncmp(htmlData, "OPTIONS", 7) == 0)
	{
		ProcessOptionsCommand(socket);
		gServerTransactionID++;	//*	we are the "server"
	}
	else if (byteCount > 0)
	{
		CONSOLE_DEBUG_W_STR("Invalid HTML get/put command\t=\r\n",	htmlData);
		CONSOLE_DEBUG_W_LONG("byteCount\t=",	byteCount);
	}


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
	printf("\t%-20s\t%s\r\n",	"-a",				"Auto exposure");
	printf("\t%-20s\t%s\r\n",	"-c",				"Conform logging, log ALL commands to disk");
	printf("\t%-20s\t%s\r\n",	"-d",				"Display images as they are taken");
	printf("\t%-20s\t%s\r\n",	"-e",				"Error logging, log errors commands to disk");
#ifdef _ENABLE_GLOBAL_GPS_
	printf("\t%-20s\t%s\r\n",	"-g",				"Enable GPS via serial port (/dev/ttyS0)");
	printf("\t%-20s\t%s\r\n",	"-g9",				"Sets baud rate to 9600 (default)");
	printf("\t%-20s\t%s\r\n",	"-g4",				"Sets baud rate to 4800");
	printf("\t%-20s\t%s\r\n",	"-g4/dev/ttyUSB0",	"Sets baud rate to 4800 and port to /dev/ttyUSB0");
#else
	printf("\t%-20s\t%s\r\n",	"-g...",			"GPS support not enabled in this build");
#endif
	printf("\t%-20s\t%s\r\n",	"-h",				"This help message");
	printf("\t%-20s\t%s\r\n",	"-l",				"Live mode");
	printf("\t%-20s\t%s\r\n",	"-p <port>",		"what port to use (default 6800)");
	printf("\t%-20s\t%s\r\n",	"-q",				"quiet (less console messages)");
	printf("\t%-20s\t%s\r\n",	"-s",				"Simulate camera image");
	printf("\t%-20s\t%s\r\n",	"-t <profile>",		"Which telescope profile to use");
	printf("\t%-20s\t%s\r\n",	"-v",				"verbose (more console messages default)");
}

#ifdef _ENABLE_GLOBAL_GPS_
//*****************************************************************************
static void	ProcessGPScmdArgs(char *gpsCmdLineOptions)
{
char	*slashPtr;

	gEnableGlobalGPS	=	true;
	if (isdigit(gpsCmdLineOptions[2]))
	{
		gGlobalGPSbaudrate	=	gpsCmdLineOptions[2];
	}
	slashPtr	=	strchr(gpsCmdLineOptions, '/');
	if (slashPtr != NULL)
	{
		strcpy(gGlobalGPSpath, slashPtr);
	}

}
#endif // _ENABLE_GLOBAL_GPS_

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

		#ifdef _ENABLE_GLOBAL_GPS_
				//	-g	means enable local GPS
				case 'g':
					ProcessGPScmdArgs(argv[iii]);
					break;
		#endif // _ENABLE_GLOBAL_GPS_

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
	if (cWatchDogEnabled)
	{
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
	CONSOLE_DEBUG_W_NUM(	"cAlpacaDeviceNum               \t=",	cAlpacaDeviceNum);
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
	SETUP_TIMING();

//------------------------------------------------------
//*	Cameras
#ifdef _ENABLE_CAMERA_
int		cameraCnt	=	0;
	cameraCnt	=	CreateCameraObjects();
	CONSOLE_DEBUG_W_NUM("total cameras created\t=", cameraCnt);
	DEBUG_TIMING("Time to create camera objects (ms)=");
#endif


//------------------------------------------------------
//*	Multicam
#ifdef _ENABLE_MULTICAM_
	cameraCnt	=	CountDevicesByType(kDeviceType_Camera);
	if (cameraCnt > 1)
	{
		CreateMultiCamObject();
	}
	DEBUG_TIMING("Time to create multicam objects (ms)=");
#endif

//------------------------------------------------------
#if _ENABLE_CALIBRATION_
	CreateCalibrationObjects();
	DEBUG_TIMING("Time to create calibration objects (ms)=");
#endif // _ENABLE_CALIBRATION_

//------------------------------------------------------
//*	Dome
#ifdef _ENABLE_DOME_
	CreateDomeObjects();
	DEBUG_TIMING("Time to create dome objects (ms)=");
#endif

//------------------------------------------------------
//*	Filter wheel
#ifdef _ENABLE_FILTERWHEEL_
	CreateFilterWheelObjects();
	DEBUG_TIMING("Time to create filterhweel objects (ms)=");
#endif

//------------------------------------------------------
//*	Focuser
#ifdef _ENABLE_FOCUSER_
	CreateFocuserObjects();
	DEBUG_TIMING("Time to create focuser objects (ms)=");
#endif

//------------------------------------------------------
//*	Observing conditions
#if defined(_ENABLE_OBSERVINGCONDITIONS_)
	CreateObsConditionObjects();
//	CreateObsConditionRpiObjects();
	DEBUG_TIMING("Time to create obs-condition objects (ms)=");
#endif

//------------------------------------------------------
//*	rotator objects
#ifdef _ENABLE_ROTATOR_
	CreateRotatorObjects();
	DEBUG_TIMING("Time to create rotator objects (ms)=");
#endif


//------------------------------------------------------
//*	Shutter
#ifdef _ENABLE_SHUTTER_
//	CreateShutterObjects();
	CreateShuterArduinoObjects();
	DEBUG_TIMING("Time to create shutter objects (ms)=");
#endif

//------------------------------------------------------
//*	Slit tacker
#ifdef _ENABLE_SLIT_TRACKER_
	CreateSlitTrackerObjects();
	DEBUG_TIMING("Time to create slit-tracker objects (ms)=");
#endif // _ENABLE_SLIT_TRACKER_

//------------------------------------------------------
//*	Switch
#if defined(_ENABLE_SWITCH_)
	CreateSwitchObjects();
	DEBUG_TIMING("Time to create switch objects (ms)=");
#endif

//------------------------------------------------------
//*	Telescope
#ifdef _ENABLE_TELESCOPE_
	CreateTelescopeObjects();
	DEBUG_TIMING("Time to create telescope objects (ms)=");
#endif // _ENABLE_TELESCOPE_

//------------------------------------------------------
#ifdef _ENABLE_SPECTROGRAPH_
	CreateSpectrographObjects();
	DEBUG_TIMING("Time to create spectrograph objects (ms)=");
#endif

//------------------------------------------------------
	//*	Management
	CreateManagementObject();

#ifdef _ENABLE_CAMERA_SIMULATOR_
	strcpy(gWebTitle, "AlpacaPi Simulator");
#endif
	DEBUG_TIMING("Time to create driver objects (ms)=");

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
int				ram_Megabytes;
double			freeDiskSpace_Gigs;
int32_t			mainLoopCntr;
uint64_t		startNanoSecs;
uint64_t		endNanoSecs;
uint64_t		deltaNanoSecs;
time_t			currentTime;
struct tm		*linuxTime;
#if defined(_ENABLE_CAMERA_)
	int			cameraCnt;
#endif
#if defined(_ENABLE_FITS_) || defined(_ENABLE_JPEGLIB_)
	char			lineBuffer[64];
#endif
//struct rlimit	myRlimit;
//int				errorCode;

	SETUP_TIMING();

//	while (1)
//	{
//		sleep(1);
//		DEBUG_TIMING(__FUNCTION__);
//	}


	printf("AlpacaPi driver\r\n");
	sprintf(gFullVersionString,		"%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);
	sprintf(gUserAgentAlpacaPiStr,	"User-Agent: AlpacaPi/%s-Build-%d\r\n", kVersionString,  kBuildNumber);

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(gFullVersionString);

//	CONSOLE_DEBUG_W_SIZE("sizeof(AlpacaDriver)\t=",	sizeof(AlpacaDriver));
//	CONSOLE_DEBUG_W_SIZE("sizeof(CameraDriver)\t=",	sizeof(CameraDriver));
//	CONSOLE_DEBUG_W_SIZE("sizeof(TYPE_GPSdata)\t=",	sizeof(TYPE_GPSdata));
//	CONSOLE_ABORT(__FUNCTION__);

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

	DEBUG_TIMING("Timing step 1");
	gObservatorySettingsOK	=	ObservatorySettings_ReadFile();

#ifdef _ENABLE_IMU_
int	imu_ReturnCode;
	CONSOLE_DEBUG("Calling IMU_Init()");
	imu_ReturnCode	=	IMU_Init();
	if (imu_ReturnCode == 0)
	{
		IMU_Print_Calibration();
		IMU_SetDebug(false);
		gIMUisOnLine	=	true;
	}
	else
	{
		CONSOLE_DEBUG("IMU_Init() failed, now disabled");
		LogEvent(	"AlpacaPi",
					"IMU",
					NULL,
					kASCOM_Err_Success,
					"IMU Failed to initialize");
	}
#endif
#ifdef _ENABLE_GLOBAL_GPS_
	if (gEnableGlobalGPS)
	{
		CONSOLE_DEBUG("Starting GPS thread");
		GPS_StartThread(gGlobalGPSpath, gGlobalGPSbaudrate);
	}
#endif

	DEBUG_TIMING("Timing step 2:");
	//--------------------------------------------------------
	//*	create the various driver objects
	CreateDriverObjects();
	DEBUG_TIMING("Timing step 3:");

	//*********************************************************
	StartDiscoveryListenThread(gAlpacaListenPort);
	DEBUG_TIMING("Timing step 4:");
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

	DEBUG_TIMING("Timing step 5:");


	//========================================================================================
	CONSOLE_DEBUG("Starting main loop -----------------------------------------");
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
//					CONSOLE_DEBUG(gAlpacaDeviceList[iii]->cAlpacaDeviceString);
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
					if ((mainLoopCntr % 10) == 0)
					{
						gAlpacaDeviceList[iii]->CheckWatchDogTimeout();
						gAlpacaDeviceList[iii]->ComputeCPUusage();
					}

					//==================================================================================
					//*	does the device driver need to be deleted
					//*	this occurs when the RESTART command is issued, NON-ALPACA
					if (gAlpacaDeviceList[iii]->cDeleteMe)
					{
						delete gAlpacaDeviceList[iii];
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
//		CONSOLE_DEBUG_W_INT32("Delay microsecs\t=", delayTime_microSecs);
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
//	#warning "Under test 4/11/2022"
//	//ProcessControllerWindows();
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
			if (cmdType != NULL)
			{
				*cmdType	=	theCmdTable[iii].get_put;
			}
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
				if (cmdType != NULL)
				{
					*cmdType	=	theCmdTable[iii].get_put;
				}
			}
			iii++;
		}
	}
	return(cmdEnumValue);
}
#include	<ctype.h>
//*****************************************************************************
static void	GenerateInvertedCase(const char *charStr, char *invertedStr)
{
int	sLen;
int	iii;

	sLen	=	strlen(charStr);
	for (iii=0; iii<sLen; iii++)
	{
		if (isupper(charStr[iii]))
		{
			invertedStr[iii]	=	tolower(charStr[iii]);
		}
		else if (islower(charStr[iii]))
		{
			invertedStr[iii]	=	toupper(charStr[iii]);
		}
		else
		{
			invertedStr[iii]	=	charStr[iii];
		}
	}
	invertedStr[iii]	=	0;
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
							const bool	ingoreCase,
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
char	invertedCaseKeyWord[64];
#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("dataSource\t=", dataSource);
	CONSOLE_DEBUG_W_STR("keyword   \t=", keyword);
#endif // _DEBUG_CONFORM_


	foundKeyWord	=	false;
	if ((dataSource != NULL) && (keyword != NULL) && (argument != NULL))
	{
		GenerateInvertedCase(keyword, invertedCaseKeyWord);

		//*	this steps through the string looking for keywords
		//*	Once the keyword is found, it MUST be followed by an "="
		dataSrcLen	=	strlen(dataSource);
		if (dataSrcLen > 0)
		{
			argument[0]	=	0;
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

					//*	conformU wants us accept any case on GET and strict case on PUT
					if ((strcmp(myKeyWord, keyword) == 0) ||
						((strcasecmp(myKeyWord, keyword) == 0) && ingoreCase))
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
						if (gAlpacaDeviceList[iii]->cAlpacaDeviceNum == alpacaDevNum)
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
	CONSOLE_DEBUG("*******************************************");
	CONSOLE_DEBUG_W_STR(	"Called from           \t=",	functionName);
	CONSOLE_DEBUG_W_STR(	"httpUserAgent         \t=",	reqData->httpUserAgent);
	CONSOLE_DEBUG_W_NUM(	"cHTTPclientType       \t=",	reqData->cHTTPclientType);
	CONSOLE_DEBUG_W_BOOL(	"clientIs_AlpacaPi     \t=",	reqData->clientIs_AlpacaPi);
	CONSOLE_DEBUG_W_BOOL(	"clientIs_ConformU     \t=",	reqData->clientIs_ConformU);

	CONSOLE_DEBUG_W_STR(	"httpCmdString         \t=",	reqData->httpCmdString);
	CONSOLE_DEBUG_W_NUM(	"requestTypeEnum       \t=",	reqData->requestTypeEnum);
	CONSOLE_DEBUG_W_NUM(	"Alpaca Version        \t=",	reqData->alpacaVersion);

	CONSOLE_DEBUG_W_STR(	"deviceType            \t=",	reqData->deviceType);
	CONSOLE_DEBUG_W_NUM(	"Device Number         \t=",	reqData->deviceNumber);
	CONSOLE_DEBUG_W_STR(	"cmdBuffer             \t=",	reqData->cmdBuffer);
	CONSOLE_DEBUG_W_STR(	"deviceCommand         \t=",	reqData->deviceCommand);
	CONSOLE_DEBUG_W_STR(	"contentData           \t=",	reqData->contentData);
	CONSOLE_DEBUG_W_STR(	"ClientTransactionIDstr\t=",	reqData->ClientTransactionIDstr);
	CONSOLE_DEBUG_W_NUM(	"ClientTransactionID   \t=",	reqData->ClientTransactionID);
	CONSOLE_DEBUG_W_NUM(	"gClientID             \t=",	gClientID);
	CONSOLE_DEBUG_W_NUM(	"gServerTransactionID  \t=",	gServerTransactionID);
	CONSOLE_DEBUG_W_NUM(	"httpRetCode           \t=",	reqData->httpRetCode);


//	printf("Dev#=%d\tG/P=%c\r\n",	reqData->deviceNumber, reqData->get_putIndicator);
	printf("HTML\t=%s\r\n",			reqData->htmlData);

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
	SocketWriteData(mySocketFD,	"<HTML lang=\"en\">\r\n");
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

//**************************************************************************************
static void	ExtractFileExtension(const char *fileName, char *extension)
{
int		fnLength;
int		ccc;

	fnLength	=	strlen(fileName);
	ccc			=	fnLength;
	while ((fileName[ccc] != '.') && (ccc > 1))
	{
		ccc--;
	}
	strcpy(extension, &fileName[ccc]);
}


//**************************************************************************
//*	look for the specified file in the "html" folder
static void	OutputHTML_html(TYPE_GetPutRequestData *reqData)
{
char		filePath[256];
char		altFilePath[256];
char		fileExtension[128];
char		*slashPtr;
int			mySocketFD;
int			slashCnt;
int			ccc;
struct stat	fileStatus;
int			returnCode;

//	CONSOLE_DEBUG(__FUNCTION__);
//	DumpRequestStructure(__FUNCTION__, reqData);
	mySocketFD	=	reqData->socket;

	slashPtr	=	strchr(reqData->httpCmdString, '/');
	if (slashPtr != NULL)
	{
		//*	built the file path
		slashPtr++;	//*	skip past the "/"
		ccc	=	0;
		while ((*slashPtr > 0x20) && (ccc < 100))
		{
			filePath[ccc++]	=	*slashPtr;
			slashPtr++;
		}
		filePath[ccc]	=	0;
		CONSOLE_DEBUG_W_STR("filePath\t=", filePath);

		slashCnt	=	CountCharsInString(filePath, '/');
		if ((slashCnt == 0) && (strcmp(filePath, "docs") == 0))
		{
			strcat(filePath, "/index.html");
		}
		CONSOLE_DEBUG_W_STR("filePath\t=", filePath);

		//*	fstat - check for existence of file
		returnCode	=	stat(filePath, &fileStatus);
		CONSOLE_DEBUG_W_HEX("fileStatus.st_mode\t=", fileStatus.st_mode);
		if (returnCode != 0)
		{
			//*	ok, the file is not there, lets look in docs
			strcpy(altFilePath, "docs");
			strcat(altFilePath, filePath);
			CONSOLE_DEBUG_W_STR("altFilePath\t=", altFilePath);
			returnCode	=	stat(filePath, &fileStatus);
			if ((returnCode == 0) && (fileStatus.st_mode & S_IFREG))
			{
				strcpy(filePath, altFilePath);
			}
		}
		CONSOLE_DEBUG_W_STR("filePath\t=", filePath);
		if (returnCode == 0)
		{
			if (fileStatus.st_mode & S_IFREG)
			{
				CONSOLE_DEBUG("File is S_IFREG (regular file)");
			}
			if (fileStatus.st_mode & S_IFDIR)
			{
			char	lastChar;

				CONSOLE_DEBUG("File is S_IFDIR (Directory)");

				lastChar	=	filePath[strlen(filePath) - 1];
				if (lastChar == '/')
				{
					strcat(filePath, "index.html");
				}
			}

			CONSOLE_DEBUG("File is OK");

			//*	determine the extension
			ExtractFileExtension(filePath, fileExtension);
			CONSOLE_DEBUG_W_STR("fileExtension\t=", fileExtension);
			if (strcasecmp(fileExtension, ".jpg") == 0)
			{
				SendJpegResponse(mySocketFD, filePath);
			}
			else if (strcasecmp(fileExtension, ".png") == 0)
			{
				SendJpegResponse(mySocketFD, filePath);
			}
			else
			{
				//*	send the file to the socket
				SocketWriteData(mySocketFD,	gHtmlHeader);
				SendFileToSocket(mySocketFD, filePath);
			}
		}
		else
		{
			SocketWriteData(mySocketFD,	"File not found\r\n");
		}
	}
	else
	{
		SocketWriteData(mySocketFD,	"File not found\r\n");
	}
}



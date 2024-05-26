//*****************************************************************************
//*	Name:			alpacadriver.h
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Aug 30,	2019	<MLS> Started on alpaca driver base class
//*	Jan 17,	2020	<MLS> Added magic cookie for object validation
//*	Jan 17,	2020	<MLS> Moved device strings to parent class, reduced duplication
//*	Jan 26,	2020	<MLS> Added kCmd_Common_interfaceversion
//*	Jan 30,	2020	<MLS> Added macros SETUP_TIMING(), START_TIMING, DEBUG_TIMING()
//*	Feb 22,	2020	<MLS> Added UniqueID
//*	Jul 24,	2020	<MLS> gcc on 64 bit Raspberry pi does not have __arm__ defined
//*	Sep  1,	2020	<MLS> Added _INCLUDE_EXIT_COMMAND_
//*	Dec  5,	2020	<MLS> Added cDriverVersion so that different drivers can have different versions
//*	Dec 11,	2020	<MLS> Added GENERATE_ALPACAPI_ERRMSG() macro to make error messages consistent
//*	Feb 11,	2021	<MLS> Deleted cDriverVersion, use cCommonProp.InterfaceVersion
//*	Sep  2,	2021	<MLS> Added _ENABLE_BANDWIDTH_LOGGING_
//*	Nov 28,	2022	<MLS> Added cLastDeviceErrMsg
//*	Sep 20,	2023	<MLS> Moved camera read thread to base class
//*	Apr 29,	2024	<MLS> Added cSendJSONresponse to handle setupdialog
//*****************************************************************************
//#include	"alpacadriver.h"

#ifndef _ALPACA_DRIVER_H_
#define	_ALPACA_DRIVER_H_

#include	<time.h>

#ifndef _STDINT_H
	#include	<stdint.h>
#endif

#ifndef	_UNISTD_H
	#include	<unistd.h>
#endif

#ifndef _PTHREAD_H
	#include	<pthread.h>
#endif

#ifndef _ARPA_INET_H
	#include	<arpa/inet.h>
#endif

#ifndef _SYS_TIME_H
	#include	<sys/time.h>
#endif
#ifndef _SYS_RESOURCE_H
	#include	<sys/resource.h>
#endif

//=============================================================================
#ifdef _USE_OPENCV_
	#include	<opencv2/opencv.hpp>
	#include	<opencv2/core.hpp>
	#if (CV_MAJOR_VERSION >= 3)
		#include	<opencv2/videoio.hpp>
	#endif

	#ifndef _USE_OPENCV_CPP_
		#include	"opencv2/highgui/highgui_c.h"
		#include	"opencv2/imgproc/imgproc_c.h"
		#include	"opencv2/core/version.hpp"

		#if (CV_MAJOR_VERSION >= 3)
			#include	"opencv2/imgproc/imgproc.hpp"
		#endif
	#endif // _USE_OPENCV_CPP_
#endif // _USE_OPENCV_




#ifndef _REQUESTDATA_H_
	#include	"RequestData.h"
#endif

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _GPS_DATA_H_
	#include	"gps_data.h"
#endif



#ifdef _USE_OPENCV_
	#ifndef	_CONTROLLER_H_
		#include	"controller.h"
	#endif
#endif


#if defined(__ARM_ARCH) && !defined(__arm__)
	#define __arm__
#endif // defined

#define	kMaxDevices			30

//#define _DEBUG_CONFORM_
#define		_ENABLE_BANDWIDTH_LOGGING_

#ifndef _COMMON_ALPACA_CMDS_H_
	#include	"common_AlpacaCmds.h"
#endif



//*****************************************************************************
//*	UUID / GUID
//*	https://en.wikipedia.org/wiki/Universally_unique_identifier
//*	128 bit number
//*	This is my own concoction of a UUID, not from any standard
//*****************************************************************************
typedef struct	//	TYPE_UniqueID
{
	uint32_t	part1;		//*	4 byte manufacturer code
	uint32_t	part2;		//*	software version number
	uint32_t	part3;		//*	model number
	uint32_t	part4;		//*	tbd
	uint32_t	part5;		//*	serial number

} TYPE_UniqueID;

//*****************************************************************************
#define	kDeviceCmdCnt	100
typedef struct	//	TYPE_CMD_STATS
{
	int		connCnt;
	int		getCnt;
	int		putCnt;
	int		errorCnt;

} TYPE_CMD_STATS;


#define	kMagicCookieValue	0x55AA7777

#define	kDeviceModelStrLen		64
#define	kDeviceVersionStrLen	64
#define	kDeviceSerialNumStrLen	64


//**************************************************************************************
enum DeviceConnectionType
{
	kDevCon_Ethernet,
	kDevCon_USB,
	kDevCon_Serial,
	kDevCon_Custom

};


//**************************************************************************************
class AlpacaDriver
{
	public:

		//alpaca
		// Construction
		//
						AlpacaDriver(TYPE_DEVICETYPE argDeviceType);
		virtual			~AlpacaDriver(void);

		virtual	int32_t	RunStateMachine(void);	//*	returns delay time in micro-seconds
		virtual int		UpdateProperties(void);


		virtual	bool	AlpacaConnect(void);	//*	Connect and Disconnect names conflicted with other libraries
		virtual	bool	AlpacaDisConnect(void);

		virtual	TYPE_ASCOM_STATUS		ProcessCommand(			TYPE_GetPutRequestData *reqData);
				TYPE_ASCOM_STATUS		ProcessCommand_Common(	TYPE_GetPutRequestData *reqData, const int cmdEnum, char *alpacaErrMsg);

				TYPE_ASCOM_STATUS		Get_Connected(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Put_Connected(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS		Get_Description(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Driverinfo(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Driverversion(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Interfaceversion(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Name(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_WatchDogEnabled(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_WatchDogTimeout(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

				TYPE_ASCOM_STATUS		Get_SupportedActions(TYPE_GetPutRequestData *reqData, const TYPE_CmdEntry *theCmdTable);


				//*	New commands as of Jun 18, 2023
				TYPE_ASCOM_STATUS		Get_Connecting(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Put_Connect(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS		Put_Disconnect(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS		Get_DeviceState(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


				TYPE_ASCOM_STATUS		Put_LiveWindow(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

				TYPE_ASCOM_STATUS		Get_Readall_Common(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS		Get_Readall_CPUstats(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		virtual	bool	DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen);
				void	DeviceState_Add_Bool(const int socketFD, char *jsonTextBuffer, const int maxLen, const char *name, const bool boolValue, const bool includeComa=true);
				void	DeviceState_Add_Dbl(const int socketFD, char *jsonTextBuffer, const int maxLen, const char *name, const double dblValue, const bool includeComa=true);
				void	DeviceState_Add_Int(const int socketFD, char *jsonTextBuffer, const int maxLen, const char *name, const int intValue, const bool includeComa=true);
				void	DeviceState_Add_Str(const int socketFD, char *jsonTextBuffer, const int maxLen, const char *name, const char *valueStr, const bool includeComa=true);


		virtual	void	OutputHTML(				TYPE_GetPutRequestData *reqData);
		virtual	void	OutputHTML_Part2(		TYPE_GetPutRequestData *reqData);
				void	OutputHTML_CmdTable(	TYPE_GetPutRequestData *reqData);
				void	OutputHTML_DriverDocs(	TYPE_GetPutRequestData *reqData);
				void	OutputCommadTable(int mySocketFD, const char *title, const TYPE_CmdEntry *commandTable);
		virtual bool	GetCommandArgumentString(const int cmdNumber, char *agumentString, char *commentString);
		virtual bool	GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);
		virtual	void	OuputCommandTableNotes(int mySocketFD);

				void	OutputHTMLrowData(int socketFD, const char *string1, const char *string2);
				void	OutputHTML_CmdStats(	TYPE_GetPutRequestData *reqData);

				TYPE_ASCOM_STATUS		SendSupportedActions(TYPE_GetPutRequestData *reqData, const TYPE_CmdEntry *theCmdTable);
				void					DumpCommonProperties(const char *callingFunctionName);

				//*	class variables start with "c"
				bool					cDeleteMe;
				//*	start with the Alpaca properties
				TYPE_CommonProperties	cCommonProp;
				const TYPE_CmdEntry		*cDriverCmdTablePtr;

				bool				cSendJSONresponse;		//*	False for setupdialog and camera binary data
				bool				cHttpHeaderSent;
				bool				cRunStartupOperations;
				bool				cVerboseDebug;
				uint32_t			cMagicCookie;			//*	used to validate objects
				TYPE_DEVICETYPE		cDeviceType;
				char				cAlpacaDeviceString[32];
				int					cAlpacaDeviceNum;		//*	device index for alpaca
				uint32_t			cLastUpdate_milliSecs;


				char				cAlpacaName[32];
				char				cDeviceModel[kDeviceModelStrLen];
				char				cDeviceManufacturer[64];
				char				cDeviceManufAbrev[16];
				char				cDeviceSerialNum[kDeviceSerialNumStrLen];
				char				cDeviceVersion[kDeviceVersionStrLen];
				char				cDeviceFirmwareVersStr[64];
				TYPE_UniqueID		cUUID;

				char				cLastDeviceErrMsg[128];

				//=========================================================
				//*	bandwidth statistics
				unsigned long		cTotalBytesRcvd;
				unsigned long		cTotalBytesSent;

#ifdef _ENABLE_BANDWIDTH_LOGGING_
				void				BandWidthStatsInit(void);
	//	#define	kSecsPerHour			(60 * 60)
		#define	kMaxBandWidthSamples	(60 * 1)
				int					cBW_CmdsReceived[kMaxBandWidthSamples];
				int					cBW_BytesReceived[kMaxBandWidthSamples];
				int					cBW_BytesSent[kMaxBandWidthSamples];
				int					cBytesWrittenForThisCmd;
#endif // _ENABLE_BANDWIDTH_LOGGING_

				//=========================================================
				//*	command statistics
				int					cTotalCmdsProcessed;
				int					cTotalCmdErrors;
				void				RecordCmdStats(int cmdNum, char getput, TYPE_ASCOM_STATUS alpacaErrCode);

				TYPE_CMD_STATS		cCommonCmdStats[kCmd_Common_last];
				TYPE_CMD_STATS		cDeviceCmdStats[kDeviceCmdCnt];

				//=========================================================
				//*	discovery routines, allow a device to look for other devices
				bool					SendDiscoveryQuery(void);
				void					HandleDiscoveryResponse(void);
				void					QueryConfiguredDevices(	struct sockaddr_in *deviceAddress,
																const int			ipPortNumber);
				void					ReadExternalIPlist(void);

		virtual	void					ProcessDiscovery(		struct sockaddr_in	*deviceAddress,
																const int			ipPortNumber,
																const char			*deviceType,
																const int			deviceNumber);

				struct sockaddr_in		cServer_addr;
				bool					cDiscoveryThreadActive;
				pthread_t				cDiscoveryThreadID;
				int						cBroadcastSocket;
				int						cDiscoveryCount;		//*	how many times have we done the discovery request

		//-------------------------------------------------------------------------
		//*	Watchdog timer stuff
	private:
				time_t					cTimeOfLastWatchDogCheck;
	protected:
		virtual	void					WatchDog_TimeOut(void);
	public:
				bool					cWatchDogEnabled;		//*	defaults to FALSE
				int						cWatchDogTimeOut_Minutes;
				void					CheckWatchDogTimeout(void);
				time_t					cTimeOfLastValidCmd;
				char					cWatchDogTimeOutAction[64];
		//-------------------------------------------------------------------------
		//*	CPU usage information
				uint64_t				cAccumilatedNanoSecs;
				uint64_t				cTotalNanoSeconds;
				uint64_t				cTotalMilliSeconds;
		//*	cpu usage statistics
				void					ComputeCPUusage(void);
				struct rusage			cRusage;

		//-------------------------------------------------------------------------
		//*	Temperature logging
				void				TemperatureLog_Init(void);
				void				TemperatureLog_SetDescription(const char *description);
				void				TemperatureLog_AddEntry(const double temperatureEntry);
				TYPE_ASCOM_STATUS	Get_TemperatureLog(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				char				cTempLogDescription[32];
				double				cTemperatureLog[kTemperatureLogEntries + 10];
				uint32_t			cLastTempUpdate_Secs;



	#ifdef _USE_OPENCV_
		//-------------------------------------------------------------------------
		//*	live controller window
		virtual	TYPE_ASCOM_STATUS		OpenLiveWindow(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS		CloseLiveWindow(char *alpacaErrMsg);
		virtual	void					UpdateLiveWindow(void);
				Controller				*cLiveController;
	#endif // _USE_OPENCV_




		//-------------------------------------------------------------------------
		//*	this is for the setup function
				bool					cDriverSupportsSetup;
				bool					Setup_ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	bool					Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString);
				int						Setup_Save(TYPE_GetPutRequestData *reqData);
		virtual void					Setup_SaveInit(void);
		virtual void					Setup_SaveFinish(void);
		virtual	bool					Setup_ProcessKeyword(const char *keyword, const char *valueString);
				void					Setup_OutputCheckBox(	const int	socketFD,
																const char	*name,
																const char	*displayedName,
																const bool	checked);
				void					Setup_OutputRadioBtn(	const int	socketFD,
																const char	*groupName,
																const char	*name,
																const char	*displayedName,
																const bool	checked);
				void					Setup_OutputNumberBox(	const int	socketFD,
																const char	*name,
																const char	*displayedName,
																const int	value,
																const int	format=0);


		//-------------------------------------------------------------------------
		//*	this is for driver thread
				void				RunThread(void);
	protected:
				void				StartDriverThread(void);
				void				StopDriverThread(void);
		virtual	void				RunThread_Startup(void);
		virtual	void				RunThread_Loop(void);
				bool				cDriverThreadIsActive;
				bool				cDriverThreadKeepRunning;
				long				cDriverThreadLoopCnt;
				pthread_t			cDriverThreadID;


};

//**************************************************************************************
enum
{
	kLog_Error	=	0,
	kLog_Conform

};

#define	kDefaultRefIdMaxLen	16

extern	AlpacaDriver	*gAlpacaDeviceList[];
extern	int				gDeviceCnt;
extern	bool			gLiveView;
extern	bool			gAutoExposure;
extern	bool			gDisplayImage;
extern	bool			gSimulateCameraImage;
extern	bool			gVerbose;
extern	bool			gDebugDiscovery;
extern	bool			gObservatorySettingsOK;
extern	const char		gValueString[];
extern	bool			gImageDownloadInProgress;
extern	bool			gIMUisOnLine;

extern	uint32_t		gServerTransactionID;
extern	char			gDefaultTelescopeRefID[kDefaultRefIdMaxLen];


extern	bool			gErrorLogging;		//*	write errors to log file if true
extern	bool			gConformLogging;	//*	log all commands to log file to match up with Conform
extern	char			gFullVersionString[];
extern	char			gHostName[];
extern	const char		gHtmlHeader[];

#ifdef __cplusplus
	extern "C" {
#endif
int				FindCmdFromTable(const char *theCmd, const TYPE_CmdEntry *theCmdTable, int *cmdType);
void			GenerateHTMLcmdLinkTable(int socketFD, const char *deviceName, const int deviceNum, const TYPE_CmdEntry *cmdTable);
int				GetFilterWheelCnt(void);
int				CountDevicesByType(const int deviceType);
AlpacaDriver	*FindDeviceByType(const int deviceType, const int alpacaDevNum=-1);
bool			GetCmdNameFromTable(const int cmdNumber, char *comandName, const TYPE_CmdEntry *cmdTable, char *getPut);
void			LogToDisk(const int whichLogFile, TYPE_GetPutRequestData *reqData);
void			GetAlpacaName(TYPE_DEVICETYPE deviceType, char *alpacaName);



//*****************************************************************************

//*	this is a macro to help make all error messages consistent
#define	GENERATE_ALPACAPI_ERRMSG(buffer,errmsg)		\
		sprintf(buffer, "AlpacaPi:%s: %s: %d", errmsg, __FUNCTION__, __LINE__);


#ifdef __cplusplus
}
#endif


#endif	//	_ALPACA_DRIVER_H_


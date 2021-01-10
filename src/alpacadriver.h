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
//*****************************************************************************
//#include	"alpacadriver.h"

#ifndef _ALPACA_DRIVER_H_
#define	_ALPACA_DRIVER_H_

#include	<time.h>

#ifndef _STDINT_H
	#include	<stdint.h>
#endif // _STDINT_H

#ifndef	_UNISTD_H
	#include	<unistd.h>
#endif // _UNISTD_H

#ifndef _PTHREAD_H
	#include	<pthread.h>
#endif // _PTHREAD_H
#ifndef _ARPA_INET_H
	#include	<arpa/inet.h>
#endif // _ARPA_INET_H

#ifndef _REQUESTDATA_H_
	#include	"RequestData.h"
#endif // _REQUESTDATA_H_


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif // _ALPACA_DEFS_H_

#if defined(__ARM_ARCH) && !defined(__arm__)
//	#warning "__arm__ was not defined"
	#define __arm__
#endif // defined

#define	kMaxDevices			20

//#define _DEBUG_CONFORM_

//*****************************************************************************
//*	common commands
enum
{
	//*	these enums start at 1000 to stay out of the way of the device specific commands.
	//*	this allows the driver to have a single switch statement for all the implemented commands

	kCmd_Common_action			=	1000,	//*	Invokes the specified device-specific action.
	kCmd_Common_commandblind,				//*	Transmits an arbitrary string to the device
	kCmd_Common_commandbool,				//*	Transmits an arbitrary string to the device and returns a boolean value from the device.
	kCmd_Common_commandstring,				//*	Transmits an arbitrary string to the device and returns a string value from the device.
	kCmd_Common_connected,					//*	GET--Retrieves the connected state of the device
											//*	PUT--Sets the connected state of the device
	kCmd_Common_description,				//*	Device description
	kCmd_Common_driverinfo,					//*	Device driver description
	kCmd_Common_driverversion,				//*	Driver Version
	kCmd_Common_interfaceversion,			//*	The ASCOM Device interface version number that this device supports.
	kCmd_Common_name,						//*	Device name
	kCmd_Common_supportedactions,			//*	Returns the list of action names supported by this driver.

#ifdef _INCLUDE_EXIT_COMMAND_
	//*	Added by MLS 7/20/2020
	kCmd_Common_exit,
#endif // _INCLUDE_EXIT_COMMAND_

	kCmd_Common_last
};

//*****************************************************************************
enum
{
	kCmdType_PUT	=	'P',
	kCmdType_GET	=	'G',
	kCmdType_BOTH	=	'B'
};

//*****************************************************************************
#define	kMaxCmdLen	32
typedef struct
{
	char		commandName[kMaxCmdLen];
	int16_t		enumValue;
	char		get_put;

} TYPE_CmdEntry;

//*****************************************************************************
#define	kMaxDeviceLen	32
typedef struct
{
	char	deviceType[kMaxDeviceLen];
	int		enumValue;

} TYPE_DeviceTable;


//*****************************************************************************
//*	UUID / GUID
//*	https://en.wikipedia.org/wiki/Universally_unique_identifier
//*	128 bit number
//*	This is my own concoction of a UUID, not from any standard
typedef struct
{
	uint32_t	part1;		//*	4 byte manufacturer code
	uint32_t	part2;		//*	software version number
	uint32_t	part3;		//*	model number
	uint32_t	part4;		//*	tbd
	uint32_t	part5;		//*	serial number

} TYPE_UniqueID;

//*****************************************************************************
#define	kCommonCmdCnt	12
#define	kDeviceCmdCnt	100
typedef struct
{
	int		connCnt;
	int		getCnt;
	int		putCnt;
	int		errorCnt;

} TYPE_CMD_STATS;



//*****************************************************************************
typedef enum
{
	kDeviceType_Management	=	0,
	kDeviceType_Camera,
	kDeviceType_Dome,
	kDeviceType_Filterwheel,
	kDeviceType_Focuser,
	kDeviceType_Rotator,
	kDeviceType_Telescope,
	kDeviceType_Observingconditions,
	kDeviceType_SafetyMonitor,
	kDeviceType_Shutter,
	kDeviceType_Switch,
	kDeviceType_Multicam,
	kDeviceType_SlitTracker,
	kDeviceType_CoverCalibrator,

	kDeviceType_last

} TYPE_DEVICETYPE;

#define	kMagicCookieValue	0x55AA7777

#define	kDeviceModelStrLen		64
#define	kDeviceVersionStrLen	64
#define	kDeviceSerialNumStrLen	64


//**************************************************************************************
class AlpacaDriver
{
	public:

		//
		// Construction
		//
				AlpacaDriver(TYPE_DEVICETYPE argDeviceType);
		virtual	~AlpacaDriver(void);

		virtual	TYPE_ASCOM_STATUS		ProcessCommand(			TYPE_GetPutRequestData *reqData);
				TYPE_ASCOM_STATUS		ProcessCommand_Common(	TYPE_GetPutRequestData *reqData, const int cmdEnum, char *alpacaErrMsg);

				TYPE_ASCOM_STATUS		Get_Connected(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Put_Connected(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS		Get_Description(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Driverinfo(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Driverversion(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Interfaceversion(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Name(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

				TYPE_ASCOM_STATUS		Get_SupportedActions(TYPE_GetPutRequestData *reqData, const TYPE_CmdEntry *theCmdTable);

				TYPE_ASCOM_STATUS		Get_Readall_Common(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS		Get_Readall_CPUstats(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


						//*	Connect and Disconnect conflicted with other libraries
		virtual	void	AlpacaConnect(void);
		virtual	void	AlpacaDisConnect(void);

		virtual	void	OutputHTML(				TYPE_GetPutRequestData *reqData);
		virtual	void	OutputHTML_Part2(		TYPE_GetPutRequestData *reqData);
		virtual	int32_t	RunStateMachine(void);

				void	OutputHTMLrowData(int socketFD, const char *string1, const char *string2);
				void	OutputHTML_CmdStats(	TYPE_GetPutRequestData *reqData);

				TYPE_ASCOM_STATUS		SendSupportedActions(TYPE_GetPutRequestData *reqData, const TYPE_CmdEntry *theCmdTable);

				//*	class variables start with "c"

				//*	start with the Alpaca properties
				char				cDeviceDescription[64];
				//					cDriverInfo				//	is hard coded in alpacadriver.cpp
				char				cDriverversionStr[64];


				uint32_t			cMagicCookie;		//*	used to validate objects
				TYPE_DEVICETYPE		cDeviceType;
				int					cDeviceNum;			//*	device index for alpaca
				int					cDriverVersion;
				char				cDeviceName[128];
				char				cDeviceModel[kDeviceModelStrLen];
				char				cDeviceManufacturer[64];
				char				cDeviceManufAbrev[8];
				char				cDeviceSerialNum[kDeviceSerialNumStrLen];
				char				cDeviceVersion[kDeviceVersionStrLen];
				char				cDeviceFirmwareVersStr[64];
				TYPE_UniqueID		cUniqueID;



				//=========================================================
				//*	command statistics
				void				RecordCmdStats(int cmdNum, char getput, TYPE_ASCOM_STATUS alpacaErrCode);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

				bool				cDeviceConnected;		//*	normally always true
				TYPE_CMD_STATS		cCommonCMdStats[kCommonCmdCnt];
				TYPE_CMD_STATS		cDeviceCMdStats[kDeviceCmdCnt];

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
extern	bool			gVerbose;
extern	const char		gValueString[];

extern	uint32_t		gClientTransactionID;
extern	uint32_t		gServerTransactionID;
extern	char			gDefaultTelescopeRefID[kDefaultRefIdMaxLen];
extern	char			gOsReleaseString[];
extern	char			gCpuInfoString[];
extern	char			gPlatformString[];
extern	bool			gErrorLogging;		//*	write errors to log file if true
extern	bool			gConformLogging;	//*	log all commands to log file to match up with Conform
extern	char			gFullVersionString[];

#ifdef __cplusplus
	extern "C" {
#endif
int				FindCmdFromTable(const char *theCmd, const TYPE_CmdEntry *theCmdTable, int *cmdType);
void			GenerateHTMLcmdLinkTable(int socketFD, const char *deviceName, const int deviceNum, const TYPE_CmdEntry *cmdTable);
int				GetFilterWheelCnt(void);
int				CountDevicesByType(const int deviceType);
AlpacaDriver	*FindDeviceByType(const int deviceType);
void			GetDeviceTypeFromEnum(const int deviceEnum, char *deviceTypeString);
bool			GetCmdNameFromTable(const int cmdNumber, char *comandName, const TYPE_CmdEntry *cmdTable, char *getPut);
void			LogToDisk(const int whichLogFile, TYPE_GetPutRequestData *reqData);



//*****************************************************************************
#ifdef _ENABLE_CONSOLE_DEBUG_
//	#define	_DEBUG_TIMING_
#endif
#if defined(_DEBUG_TIMING_) && defined(_ENABLE_CONSOLE_DEBUG_) && !defined(SETUP_TIMING)

	#define		SETUP_TIMING()					\
		uint32_t		tStartMillisecs;		\
		uint32_t		tStopMillisecs;			\
		uint32_t		tDeltaMillisecs;		\
		tStartMillisecs	=	millis();


	#define		START_TIMING()								\
					tStartMillisecs	=	millis();


	//*	this macro calculates delta milliseconds for timing testing
	#define		DEBUG_TIMING(string)									\
				tStopMillisecs	=	millis();							\
				tDeltaMillisecs	=	tStopMillisecs - tStartMillisecs;	\
				CONSOLE_DEBUG_W_NUM(string,	tDeltaMillisecs);

#else
	#define		SETUP_TIMING()
	#define		START_TIMING()
	#define		DEBUG_TIMING(string)
#endif

//*	this is a macro to help make all error messages consistent
#define	GENERATE_ALPACAPI_ERRMSG(buffer,errmsg)		sprintf(buffer, "AlpacaPi: %s. L#%d", errmsg, __LINE__);


#ifdef __cplusplus
}
#endif


#endif	//	_ALPACA_DRIVER_H_

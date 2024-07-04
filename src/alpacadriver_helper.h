//**************************************************************************
//#include	"alpacadriver_helper.h"

#ifndef _ALPACA_HELPER_H_
#define	_ALPACA_HELPER_H_

#ifndef _SYS_TIME_H
	#include	<sys/time.h>
#endif

#ifndef _STDINT_H
	#include	<stdint.h>
#endif

#if defined(__ARM_ARCH) && !defined(__arm__)
	#define __arm__
#endif

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

//*****************************************************************************
#define	kMaxDeviceLen	32
typedef struct	//	TYPE_DeviceTable
{
	char				deviceTypeStr[kMaxDeviceLen];
	TYPE_DEVICETYPE		enumValue;

} TYPE_DeviceTable;

//*****************************************************************************
#define	kMaxLookupName	32
typedef struct	//	TYPE_LookupTable
{
	char	keyword[kMaxLookupName];
	int		enumValue;

} TYPE_LookupTable;


//*****************************************************************************
enum
{
	kCmdType_PUT	=	'P',
	kCmdType_GET	=	'G',
	kCmdType_BOTH	=	'B'
};

//*****************************************************************************
#define	kMaxCmdLen	32
typedef struct	//	TYPE_CmdEntry
{
	char		commandName[kMaxCmdLen];
	int16_t		enumValue;
	char		get_put;

} TYPE_CmdEntry;



#ifdef __cplusplus
	extern "C" {
#endif


//*****************************************************************************
//*	Helper functions
#define	kArgumentIsNumeric	true
#define	kRequireCase		false
#define	kIgnoreCase			true
bool			GetKeyWordArgument(	const char	*dataSource,
									const char	*keyword,
									char		*argument,
									const int	maxArgLen,
									const bool	ingoreCase=false,
									const bool	argIsNumeric=false);



int				SocketWriteData(const int socket, const char *dataBuffer);
void			ToLowerStr(char *theString);
void			StripTrailingSpaces(char *theString);
uint32_t		Calc_millisFromTimeStruct(struct timeval *theTimeStruct);
bool			Check_udev_rulesFile(const char *rulesFileName);
void			LoadAlpacaImage(void);
void			AddLibraryVersion(const char *device, const char *manuf, const char *versionString);
TYPE_DEVICETYPE	FindDeviceTypeByString(const char *deviceTypeStr);
TYPE_DEVICETYPE	FindDeviceTypeByStringLowerCase(const char *deviceTypeStr);

void			GetBinaryElementTypeString(const int elementType, char *typeString);
void			GetDeviceTypeFromEnum(const int deviceEnum, char *deviceTypeString);
void			GetDomeShutterStatusString(const int status, char *statusString);

int				StartExtraListenThread(int exposureListenPort);
void			DumpObservingconditionsProp(TYPE_ObsConditionProperties	*obsCondProp, const char *functionName);
void			DumpCoverCalibProp(TYPE_CoverCalibrationProperties	*coverCalibProp, const char *callingFunctionName);
int				SendFileToSocket(int socket, const char *fileName);

extern	char		gWebTitle[];

int	LookupStringInTable(const char *lookupString, TYPE_LookupTable *lookupTable);
int	LookupStringInCmdTable(const char *lookupString, TYPE_CmdEntry *commandTable);

#define	DEGREES_F(x)	((x * (9.0/5.0) ) + 32.0)


#ifdef __cplusplus
}
#endif

#endif // _ALPACA_HELPER_H_

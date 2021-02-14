//**************************************************************************
//#include	"alpacadriver_helper.h"

#ifndef _ALPACA_HELPER_H_
#define	_ALPACA_HELPER_H_


#ifndef _SYS_TIME_H
	#include	<sys/time.h>
#endif // _SYS_TIME_H

#ifndef _STDINT_H
	#include	<stdint.h>
#endif // _STDINT_H


#if defined(__ARM_ARCH) && !defined(__arm__)
	#define __arm__
#endif


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif


//*****************************************************************************
#define	kMaxDeviceLen	32
typedef struct
{
	char				deviceType[kMaxDeviceLen];
	TYPE_DEVICETYPE		enumValue;

} TYPE_DeviceTable;

#ifdef __cplusplus
	extern "C" {
#endif


//*****************************************************************************
//*	Helper functions
void			FormatTimeString(time_t *time, char *timeString);
int				SocketWriteData(const int socket, const char *dataBuffer);
bool			GetKeyWordArgument(const char *dataSource, const char *keyword, char *argument, int maxArgLen);
bool			IsTrueFalse(const char *trueFalseString);
void			ToLowerStr(char *theString);
void			StripTrailingSpaces(char *theString);
uint32_t		Calc_millisFromTimeStruct(struct timeval *theTimeStruct);
bool			Check_udev_rulesFile(const char *rulesFileName);
void			LoadAlpacaImage(void);
void			AddLibraryVersion(const char *device, const char *manuf, const char *versionString);
void			FormatTimeStringISO8601(struct timeval *tv, char *timeString);
TYPE_DEVICETYPE	FindDeviceTypeByString(const char *deviceTypeStr);
void			GetDeviceTypeFromEnum(const int deviceEnum, char *deviceTypeString);

extern	char		gWebTitle[];


#if !defined(__arm__) || defined(_INCLUDE_MILLIS_)
	uint32_t	millis(void);
#endif	//	!defined(__arm__) || defined(_INCLUDE_MILLIS_)


#define	DEGREES_F(x)	((x * (9.0/5.0) ) + 32.0)


#ifdef __cplusplus
}
#endif

#endif // _ALPACA_HELPER_H_

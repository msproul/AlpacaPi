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
#define	kArgumentIsNumeric	true
bool			GetKeyWordArgument(	const char	*dataSource,
									const char	*keyword,
									char		*argument,
									const int	maxArgLen,
									const bool	argIsNumeric=false);



int				SocketWriteData(const int socket, const char *dataBuffer);
void			ToLowerStr(char *theString);
void			StripTrailingSpaces(char *theString);
uint32_t		Calc_millisFromTimeStruct(struct timeval *theTimeStruct);
bool			Check_udev_rulesFile(const char *rulesFileName);
void			LoadAlpacaImage(void);
void			AddLibraryVersion(const char *device, const char *manuf, const char *versionString);
TYPE_DEVICETYPE	FindDeviceTypeByString(const char *deviceTypeStr);
void			GetDeviceTypeFromEnum(const int deviceEnum, char *deviceTypeString);
int				StartExtraListenThread(int exposureListenPort);
void			GetBinaryElementTypeString(const int elementType, char *typeString);

extern	char		gWebTitle[];


#define	DEGREES_F(x)	((x * (9.0/5.0) ) + 32.0)


#ifdef __cplusplus
}
#endif

#endif // _ALPACA_HELPER_H_

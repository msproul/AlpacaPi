//**************************************************************************

//#include	"alpacadriver_helper.h"


#ifndef _SYS_TIME_H
	#include	<sys/time.h>
#endif // _SYS_TIME_H

#ifndef _STDINT_H
	#include	<stdint.h>
#endif // _STDINT_H

#ifdef __cplusplus
	extern "C" {
#endif



//*	Helper functions
void		FormatTimeString(time_t *time, char *timeString);
int			SocketWriteData(const int socket, const char *dataBuffer);
bool		GetKeyWordArgument(const char *dataSource, const char *keyword, char *argument, int maxArgLen);
bool		IsTrueFalse(const char *trueFalseString);
void		ToLowerStr(char *theString);
void		StripTrailingSpaces(char *theString);
uint32_t	Calc_millisFromTimeStruct(struct timeval *theTimeStruct);
bool		Check_udev_rulesFile(const char *rulesFileName);
void		LoadAlpacaImage(void);
void		AddLibraryVersion(const char *device, const char *manuf, const char *versionString);
void		FormatTimeStringISO8601(struct timeval *tv, char *timeString);

extern	char		gWebTitle[];


#if !defined(__arm__) || defined(_INCLUDE_MILLIS_)
	uint32_t	millis(void);
#endif	//	!defined(__arm__) || defined(_INCLUDE_MILLIS_)


#define	DEGREES_F(x)	((x * (9.0/5.0) ) + 32.0)


#ifdef __cplusplus
}
#endif


//**************************************************************************************
//#include	"helper_functions.h"

#ifndef _HELPER_FUNCTIONS_H_
#define	_HELPER_FUNCTIONS_H_

#ifndef _STDIO_H
   #include <stdio.h>
#endif

#ifndef _STDINT_H
	#include	<stdint.h>
#endif

#ifndef _SYS_TIME_H
	#include	<sys/time.h>
#endif // _SYS_TIME_H

 #ifndef	_TIME_H
	#include <time.h>
#endif

#define	DEG_MIN_SEC(degrees, minutes, seconds)	((degrees) + ((minutes* 1.0) / 60.0) + (seconds / 3600.0))
#define	DEGREES(radians)	((radians) * (180.0 / M_PI))
#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))



#ifdef __cplusplus
	extern "C" {
#endif

bool		IsTrueFalse(const char *trueFalseString);

void		FormatHHMMSS(const double argDegreeValue, char *timeString, bool includeSign);
void		FormatHHMMSSdd(const double argDegreeValue, char *timeString, bool includeSign);
void		FormatTimeString_time_t(time_t *time, char *timeString);
void		FormatTimeString(struct timeval *tv, char *timeString);
void		FormatTimeString_Local(struct timeval *tv, char *timeString);
void		FormatDateTimeString_Local(struct timeval *tv, char *timeString);
void		FormatTimeString_TM(struct tm *timeStruct, char *timeString);
//-void		FormatTimeString(time_t *time, char *timeString);
void		FormatTimeStringISO8601(struct timeval *tv, char *timeString);

uint32_t	millis(void);
int			CountLinesInFile(FILE *filePointer);
int			CelestObjDeclinationQsortProc(const void *e1, const void *e2);


#ifdef __cplusplus
}
#endif

#define	DISPOSEPTR_IF_INUSE(thePtr)	\
	if (thePtr != NULL)				\
	{								\
		free(thePtr);				\
		thePtr	=	NULL;			\
	}


#endif // _HELPER_FUNCTIONS_H_

//#include	"julianTime.h"

#ifndef	_STDLIB_H
	#include	<stdlib.h>
#endif // _STDLIB_H

#ifndef _SYS_TIME_H
	#include	<sys/time.h>
#endif // _SYS_TIME_H

double	Julian_CalcDate(struct timeval *timeStruct);
double	Julian_GetCurrentDate(void);
double	Julian_CalcFromDate(const int month, const int day, const int year);
void	Julian_GetUTCfromJulian(const double	julianDateTime, struct timeval *timeStruct);

double	Julian_ConvertFromMJD(const double mjdDate);

double	Julian_CalcMJD(struct timeval *timeStruct);
double	Julian_CalcMJDFromDate(const int month, const int day, const int year);
void	Julian_GetUTCfromMJD(const double	mjdDateTime, struct timeval *timeStruct);

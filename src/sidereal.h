//*****************************************************************************
//#include	"sidereal.h"


#ifndef	_TIME_H
	#include	<time.h>
#endif


#ifdef __cplusplus
	extern "C" {
#endif

void	CalcSiderealTime(	struct tm	*utcTime,
							struct tm	*siderealTime,
							double		argLongitude);


double	CalcSiderealTime_dbl(struct tm	*utcTime, double argLongitude);

#ifdef __cplusplus
}
#endif

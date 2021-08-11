//**************************************************************************************
//#include	"helper_functions.h"

#ifndef _HELPER_FUNCTIONS_H_
#define	_HELPER_FUNCTIONS_H_


#ifndef _STDINT_H
	#include	<stdint.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

bool		IsTrueFalse(const char *trueFalseString);
void		FormatHHMMSS(const double argDegreeValue, char *timeString, bool includeSign);
uint32_t	millis(void);
void		Goto_RA_DEC(double argRA_radians, double argDecl_radians);


#ifdef __cplusplus
}
#endif


#endif // _HELPER_FUNCTIONS_H_

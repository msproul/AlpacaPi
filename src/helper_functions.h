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
void		FormatHHMMSSdd(const double argDegreeValue, char *timeString, bool includeSign);
uint32_t	millis(void);
int			CountLinesInFile(FILE *filePointer);


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

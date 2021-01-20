//#include	"StarCatalogHelper.h"


#ifndef _SKY_STRUCTS_
	#include	"SkyStruc.h"
#endif

#ifdef __cplusplus
	extern "C" {
#endif

void	strncpyZero(char *destString, const char *sourceString, short numChars);

long	ParseLongFromString(char *lineBuff, short firstChar, short strLen);
long	ParseCharValueFromString(char *lineBuff, short firstChar, short strLen);
double	ParseFloatFromString(char *lineBuff, short firstChar, short strLen);


#ifdef __cplusplus
}
#endif

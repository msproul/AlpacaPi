//************************************************************************
//*	StarCatalogHelper.c
//************************************************************************


#include	<stdlib.h>
#include	<string.h>

//*	MLS Libraries

#include	"StarCatalogHelper.h"

//**************************************************************************************
void	strncpyZero(char *destString, const char *sourceString, short numChars)
{
	strncpy(destString,sourceString,numChars);
	destString[numChars]	=	0;
}


//************************************************************************
long	ParseLongFromString(char *lineBuff, short firstChar, short strLen)
{
long	theValue;
char	tempStr[64];

	strncpyZero(tempStr, &lineBuff[firstChar], strLen);

	theValue	=	atoi(tempStr);
	return(theValue);
}

//************************************************************************
double	ParseFloatFromString(char *lineBuff, short firstChar, short strLen)
{
double	theValue;
char	tempStr[64];

	strncpyZero(tempStr, &lineBuff[firstChar], strLen);

	theValue	=	atof(tempStr);
	return(theValue);
}


//************************************************************************
long	ParseCharValueFromString(char *lineBuff, short firstChar, short strLen)
{
long	theValue;
short	ii;
char	tempStr[64];

	strncpyZero(tempStr, &lineBuff[firstChar], strLen);

	theValue	=	0x20202020;
	for (ii=0; ii<strLen; ii++)
	{
		theValue	=	(theValue << 8);
		theValue	+=	tempStr[ii] & 0x00ff;
	}
	return(theValue);
}

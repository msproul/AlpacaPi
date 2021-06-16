//*****************************************************************************
//*	Apr  1,	2021	<MLS> Created helper_functions.c
//*****************************************************************************

#include	<stdbool.h>
#include	<strings.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<stdint.h>
#include	<sys/time.h>

#include	"helper_functions.h"

//*****************************************************************************
bool	IsTrueFalse(const char *trueFalseString)
{
bool	trueFalseFlag;

	if (strcasecmp(trueFalseString, "true") == 0)
	{
		trueFalseFlag	=	true;
	}
	else
	{
		trueFalseFlag	=	false;
	}
	return(trueFalseFlag);
}

//*****************************************************************************
//*	Right Ascension is never negitive (0->24) and therefore does not need a sign
//*****************************************************************************
void	FormatHHMMSS(const double argDegreeValue, char *timeString, bool includeSign)
{
double	myDegreeValue;
double	minutes_dbl;
double	seconds_dbl;
int		degrees;
int		minutes;
int		seconds;
char	signChar;

	signChar		=	'+';
	myDegreeValue	=	argDegreeValue;
	if (myDegreeValue < 0)
	{
		myDegreeValue	=	-argDegreeValue;
		signChar		=	'-';
	}
	degrees		=	myDegreeValue;
	minutes_dbl	=	myDegreeValue - (1.0 * degrees);
	minutes		=	minutes_dbl * 60.0;
	seconds_dbl	=	(minutes_dbl * 60) - (1.0 * minutes);
	seconds		=	seconds_dbl * 60;;

	if (includeSign)
	{
		sprintf(timeString, "%c%02d:%02d:%02d", signChar, degrees, minutes, seconds);
	}
	else
	{
		sprintf(timeString, "%02d:%02d:%02d", degrees, minutes, seconds);
	}
}


#if !defined(__arm__) || defined(_INCLUDE_MILLIS_)

static uint32_t	gSystemStartSecs = 0;

//*****************************************************************************
uint32_t	millis(void)
{
uint32_t	elapsedSecs;
uint32_t	milliSecs;
struct timeval	currentTime;

	gettimeofday(&currentTime, NULL);

	if (gSystemStartSecs == 0)
	{
		gSystemStartSecs	=	currentTime.tv_sec;
	}
	elapsedSecs	=	currentTime.tv_sec - gSystemStartSecs;
	milliSecs	=	(elapsedSecs * 1000) + (currentTime.tv_usec / 1000);
	return(milliSecs);
}
#endif	//	!defined(__arm__) || defined(_INCLUDE_MILLIS_)

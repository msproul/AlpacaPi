//*****************************************************************************
//*	Apr  1,	2021	<MLS> Created helper_functions.c
//*	Nov  8,	2021	<MLS> Added FormatHHMMSSdd()
//*****************************************************************************

#include	<math.h>
#include	<stdbool.h>
//#include	<strings.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<stdint.h>
#include	<sys/time.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


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
//*	returns sign char "+" or "-"
//*****************************************************************************
static char	CalcHHMMSS(	const double	argDegreeValue,
						int				*arg_degrees_int,
						int				*arg_minutes_int,
						int				*arg_seconds_int,
						int				*arg_seconds_dbl)
{
double	myDegreeValue;
double	minutes_dbl;
double	seconds_dbl;
int		degrees_int;
int		minutes_int;
int		seconds_int;
char	signChar;

	signChar		=	'+';
	myDegreeValue	=	argDegreeValue;
	if (myDegreeValue < 0.0)
	{
		myDegreeValue	=	-argDegreeValue;
		signChar		=	'-';
	}
	degrees_int	=	myDegreeValue;
	minutes_dbl	=	myDegreeValue - floor(myDegreeValue);
	minutes_int	=	minutes_dbl * 60.0;

	//*	for seconds, get rid of the degrees
	seconds_dbl	=	myDegreeValue - floor(myDegreeValue);
	seconds_dbl	-=	((1.0 * minutes_int) / 60.0);
	seconds_dbl	=	seconds_dbl * 3600.0;
	seconds_int	=	seconds_dbl;


	*arg_degrees_int	=	degrees_int;
	*arg_minutes_int	=	minutes_int;
	*arg_seconds_int	=	seconds_int;
	*arg_seconds_dbl	=	seconds_dbl;

	return(signChar);
}

//*****************************************************************************
//*	Right Ascension is never negative (0->24) and therefore does not need a sign
//*****************************************************************************
void	FormatHHMMSS(const double argDegreeValue, char *timeString, bool includeSign)
{
double	myDegreeValue;
double	minutes_dbl;
double	seconds_dbl;
int		degrees_int;
int		minutes_int;
int		seconds_int;
char	signChar;

	signChar		=	'+';
	myDegreeValue	=	argDegreeValue;
	if (myDegreeValue < 0.0)
	{
		myDegreeValue	=	-argDegreeValue;
		signChar		=	'-';
	}
	degrees_int	=	myDegreeValue;
	minutes_dbl	=	myDegreeValue - floor(myDegreeValue);
	minutes_int	=	minutes_dbl * 60.0;
	seconds_dbl	=	(minutes_dbl * 60) - (1.0 * minutes_int);
	seconds_int	=	seconds_dbl * 60;

	if (includeSign)
	{
		sprintf(timeString, "%c%02d:%02d:%02d", signChar, degrees_int, minutes_int, seconds_int);
	}
	else
	{
		sprintf(timeString, "%02d:%02d:%02d", degrees_int, minutes_int, seconds_int);
	}
}

//*****************************************************************************
//*	Right Ascension is never negative (0->24) and therefore does not need a sign
//*****************************************************************************
void	FormatHHMMSSdd(const double argDegreeValue, char *timeString, bool includeSign)
{
double	myDegreeValue;
double	minutes_dbl;
double	seconds_dbl;
int		degrees_int;
int		minutes_int;
//int		seconds_int;
char	signChar;

//	CONSOLE_DEBUG("--------------------------");
	signChar		=	'+';
	myDegreeValue	=	argDegreeValue;
	if (myDegreeValue < 0.0)
	{
		myDegreeValue	=	-argDegreeValue;
		signChar		=	'-';
	}
	degrees_int	=	myDegreeValue;
	minutes_dbl	=	myDegreeValue - floor(myDegreeValue);
	minutes_int	=	minutes_dbl * 60.0;
//	CONSOLE_DEBUG_W_DBL("myDegreeValue\t=",	myDegreeValue);
//	CONSOLE_DEBUG_W_NUM("degrees_int\t=",	degrees_int);
//	CONSOLE_DEBUG_W_NUM("minutes_int\t=",	minutes_int);

	//*	for seconds, get rid of the degrees
	seconds_dbl	=	myDegreeValue - floor(myDegreeValue);
//	CONSOLE_DEBUG_W_DBL("seconds_dbl\t=",	seconds_dbl);
	seconds_dbl	-=	((1.0 * minutes_int) / 60.0);
	seconds_dbl	=	seconds_dbl * 3600.0;

//	CONSOLE_DEBUG_W_DBL("minutes_dbl\t=",	minutes_dbl);
//	CONSOLE_DEBUG_W_DBL("seconds_dbl\t=",	seconds_dbl);

	if (includeSign)
	{
		sprintf(timeString, "%c%02d:%02d:%05.2f", signChar, degrees_int, minutes_int, seconds_dbl);
	}
	else
	{
		sprintf(timeString, "%02d:%02d:%05.2f", degrees_int, minutes_int, seconds_dbl);
	}
//	CONSOLE_DEBUG(timeString);
//	CONSOLE_ABORT(__FUNCTION__);

}

//************************************************************************
int	CountLinesInFile(FILE *filePointer)
{
char	lineBuff[2048];
int		linesRead;

	linesRead	=	0;
	while (fgets(lineBuff, 2000, filePointer))
	{
		if (strlen(lineBuff) > 0)
		{
			linesRead++;
		}
	}
	fseek(filePointer, 0, SEEK_SET);
	return(linesRead);
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

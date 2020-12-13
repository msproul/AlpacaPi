//**************************************************************************************
//*	moonphase.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 29,	2020	<MLS> Created moonphase.c
//*	Nov 29,	2020	<MLS> Added CalcDaysSinceNewMoon()
//*	Nov 29,	2020	<MLS> Added CalcMoonIllumination()
//*	Dec  1,	2020	<MLS> Added GetMoonPhase(), GetCurrentMoonPhase()
//*****************************************************************************

#include	<stdlib.h>
#include	<time.h>
#include	<sys/time.h>


#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"moonphase.h"
#include	"julianTime.h"

//https://github.com/signetica/MoonRise/blob/master/MoonRise.cpp


/*****************************************************************************
https://www.subsystems.us/uploads/9/8/9/4/98948044/moonphase.pdf


www.subsystems.us
©2017 Subsystems www.subystems.us
Calculate the Moon Phase
Since early times, the moon has been an important object to mark time and
celebrate seasons.
It is pretty easy to estimate the phase of the moon based on the relatively small variations in the orbits
of the Earth and moon. The New Moon (when the sun does not illuminate any of the moon’s surface we
see on Earth) repeats every 29.53 days. This is different than its period to revolve around the Earth
(27.32 days). The difference is mainly due to the Earth moving in its orbit around the sun. The Earth
revolves around 360 degrees in about 365 days. So we move really close to 1 degree a day in our orbit.
To make up that distance and get in the same location in the sky, the moon needs about another hour
each day. That is why if you observe the moon at the same time each day, it will not be in the same
location in the sky.
The easiest way to calculate the current phase of the moon is to compare it to a known time when it
was new, and determine how many cycles it has passed through. We can do this by finding the number
of days from a known New Moon and then dividing by the lunar period.
On 1/6/2000 at 12:24:01, the moon was New. If we know how many days have elapsed since that time,
we can know how many lunar cycles we have seen. Let’s calculate this using Julian Day numbers.
Julian day numbers are a system of counting days since a specific day (January 1, 4713 BC). Entering the
above date of the New Moon in an online calculator gives the value of 2451549.5. Now we need to get
the current date in Julian Day number format. Let’s calculate it.
	1) Express the date as Y = year, M = Month, D = day.
	2) If the month is January or February, subtract 1 from the year and add 12 to the month.
	3) With a calculator, do the following calculations:
		a. A = Y/100 and then record the integer part A = _______________
		b. B = A/4 and then record the integer part B = _______________
		c. C = 2-A+B C = _______________
		d. E = 365.25 x (Y+4716) record the integer part E = _______________
		e. F = 30.6001 x (M+1) record the integer part F = _______________
		f. JD = C+D+E+F-1524.5 JD = _______________
Now that we have the Julian day, let’s calculate the days since the last new moon:
Day since New = JD - 2451549.5

If we divide this by the period, we will have how many new moons there have been:
New Moons = Days since New / 29.53
If we drop the whole number, the decimal represents the fraction of a cycle that the moon is currently
in. Multiply the fraction by 29.53 and you will uncover how many days you are into the moon’s cycle.

Let’s do a sample calculation:
Let’s calculate the phase of the moon on 3/1/2017:
	1) Express the date as Y = 2017, M = 3, D = 1.
	2) Since the month March (M=3), we don’t need to adjust the values.
	3) With a calculator, do the following calculations:
		a. A = Y/100 and then record the integer part A = 20
		b. B = A/4 and then record the integer part B = 5
		c. C = 2-A+B C = -13
		d. E = 365.25 x (Y+4716) record the integer part E = 2459228
		e. F = 30.6001 x (M+1) record the integer part F = 122
		f. JD = C+D+E+F-1524.5 JD = 2457813.5

Now that we have the Julian day, let’s calculate the days since the last new moon:
Day since New = 2457813.5 - 2451549.5 = 6264 days
If we divide this by the period, we will have how many new moons there have been:
New Moons = 6264 / 29.53 = 212.123 cycles
Now, multiply the fractional part by 29.53:
Days into cycle = 0.123 x 29.53 = 3.63 days since New Moon
Our simple calculation above doesn’t account for a few factors in the moon phase so it is not as
accurate as more rigorous calculations, but it is close enough to get a good idea of the status of the
moon. We also didn’t adjust for the time of day.
The age of the moon is the number of days since the most recent New Moon. Here is what that day
translates to:

*****************************************************************************
*/

//https://en.wikipedia.org/wiki/Orbit_of_the_Moon
//#define	kMoonPeriod	29.53
#define	kMoonPeriod	29.530588
//**************************************************************************************
double	CalcJD(const int month, const int day, const int year)
{
int		aaa;
int		bbb;
int		ccc;
int		eee;
int		fff;
double	julianDate;

//		a. A = Y/100				and then record the integer part A = _______________
//		b. B = A/4					and then record the integer part B = _______________
//		c. C = 2-A+B				C = _______________
//		d. E = 365.25 x (Y+4716)	record the integer part E = _______________
//		e. F = 30.6001 x (M+1)		record the integer part F = _______________
//		f. JD = C+D+E+F-1524.5		JD = _______________

	aaa			=	year / 100;
	bbb			=	aaa / 4;
	ccc			=	2 - aaa + bbb;
	eee			=	365.25 * (year + 4716);
	fff			=	30.6001 * (month + 1);
	julianDate	=	ccc + day + eee + fff - 1524.5;

	return(julianDate);
}


//**************************************************************************************
//*	pass a value of zero for the year to get the current value
//**************************************************************************************
double	CalcDaysSinceNewMoon(const int month, const int day, const int year)
{
double	julianDate;
double	daysSinceNewMoon;
double	newMoons;
double	moonAge;

	if (year > 0)
	{
//		julianDate		=	CalcJD(month, day, year);
//		printf("jd=%f\r\n",			julianDate);
		julianDate		=	Julian_CalcFromDate(month, day, year);
//		printf("jd=%f\r\n",			julianDate);
	}
	else
	{
		//*	if the year is 0, calculate for now
		julianDate		=	Julian_GetCurrentDate();
	}

	//*	new moon on 1/6/2000 at 12:24:01 gives a julian date of 2451549.5.
	//															2451550.0166782406

	//*	https://www.timeanddate.com/moon/phases/usa/los-angeles?year=2019
	//*	new moon on 12/25/2019 at 9:13 PST
	//*	new moon on 12/25/2019 at 17:13 UTC
	//*	http://www.csgnetwork.com/juliandatetime.html
	//*	gives a value of 2458843.17569

	if (julianDate > 2458843.17569)
	{
		//*	new moon on 12/25/2019 at 15:13 UTC
		daysSinceNewMoon	=	julianDate - 2458843.17569;
	}
	else
	{
		//*	new moon on 1/6/2000 at 12:24:01 gives a julian date of 2451549.5.
	//	daysSinceNewMoon	=	julianDate - 2451549.5;
		daysSinceNewMoon	=	julianDate - 2451550.0166782406;
		printf("old\r\n");
	}
	newMoons			=	daysSinceNewMoon / kMoonPeriod;
	moonAge				=	(newMoons - floor(newMoons)) * kMoonPeriod;


//	CONSOLE_DEBUG_W_DBL("jd\t=%f",	julianDate);
//	printf("daysSinceNewMoon\t=%f\r\n",	daysSinceNewMoon);
//	printf("newMoons\t\t=%f\r\n",		newMoons);

	return(moonAge);
}


//**************************************************************************************
//*	https://www.physicsforums.com/threads/illuminated-fraction-of-the-moon.515983/
//**************************************************************************************
double	CalcMoonIllumination(const int month, const int day, const int year)
{
double	moonAge_Days;
double	moonAge_Prct;
double	phaseAngle_Rad;
double	phaseAngle_Deg;
double	illumination;
double	daysSinceLastFullMoon;

	moonAge_Days	=	CalcDaysSinceNewMoon(month, day, year);

	//	The phase angle Es is t/T * 360, where t is the elapsed time since the last full moon
	//	and T is the period (elapsed time between full moons, about 29.5 days). For example,
	//	Es is 0 at full moon, 90o at first quarter, 180o at new moon, etc.

	//	Source https://www.physicsforums.com/threads/illuminated-fraction-of-the-moon.515983/
	daysSinceLastFullMoon	=	moonAge_Days + (kMoonPeriod / 2);
	while (daysSinceLastFullMoon > kMoonPeriod)
	{
		daysSinceLastFullMoon	-=	kMoonPeriod;
	}
	moonAge_Prct			=	daysSinceLastFullMoon / kMoonPeriod;
	phaseAngle_Rad			=	moonAge_Prct * 2 * M_PI;
	phaseAngle_Deg			=	moonAge_Prct * 360.0;

	illumination			=	100 * 0.5 * ( 1 + cos(phaseAngle_Rad));

//	printf("daysSinceLastFullMoon\t\t=%f\r\n",	daysSinceLastFullMoon);

//	printf("moonAge_Prct\t\t=%f\r\n",	moonAge_Prct);
//	printf("phaseAngle_Rad\t\t=%f\r\n",	phaseAngle_Rad);
//	printf("phaseAngle_Deg\t\t=%f\r\n",	phaseAngle_Deg);
//	printf("illumination\t\t=%f\r\n",	illumination);

	return(illumination);

}

//**************************************************************************************
void	GetMoonPhase(const double illumination, const double moonAge_Days, char *moonPhaseStr)
{
int		moonAge_int;

	if (illumination < 0.2)
	{
		strcpy(moonPhaseStr, "New Moon");
	}
	else if (illumination > 99.8)
	{
		strcpy(moonPhaseStr, "Full Moon");
	}
	else if ((illumination > 99.5) && (moonAge_Days > 14.5))
	{
		strcpy(moonPhaseStr, "Full Moon");
	}
	else
	{
	//	moonAge_int	=	round(moonAge_Days);
		moonAge_int	=	floor(moonAge_Days);
		switch(moonAge_int)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:		strcpy(moonPhaseStr, "Waxing Crescent");break;

			case 7:		strcpy(moonPhaseStr, "First Quarter");	break;

			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
						strcpy(moonPhaseStr, "Waxing Gibbous");	break;

			case 14:	strcpy(moonPhaseStr, "Full Moon");		break;

			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:	strcpy(moonPhaseStr, "Waning Gibbous");	break;

			case 22:	strcpy(moonPhaseStr, "Last Quarter");	break;

			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:	strcpy(moonPhaseStr, "Waning Crescent");	break;

			default:
				strcpy(moonPhaseStr, "Huh?");
				printf("illumination=%f\tmoonAge_Days=%f\tage_int=%d\r\n", illumination, moonAge_Days, moonAge_int);
				break;
		}
	}
}


//**************************************************************************************
void	GetCurrentMoonPhase(char *moonPhaseStr)
{
double		illumination;
double		moonAge_Days;
time_t		currentTime;
struct tm	*linuxTime;


	currentTime		=	time(NULL);
	if (currentTime != -1)
	{
		linuxTime		=	gmtime(&currentTime);
		illumination	=	CalcMoonIllumination(	(1 + linuxTime->tm_mon),
													linuxTime->tm_mday,
													(1900 + linuxTime->tm_year));

		moonAge_Days	=	CalcDaysSinceNewMoon(	(1 + linuxTime->tm_mon),
													linuxTime->tm_mday,
													(1900 + linuxTime->tm_year));
		GetMoonPhase(illumination, moonAge_Days, moonPhaseStr);
	}
	else
	{
		strcpy(moonPhaseStr, "Error");
	}
}

#ifndef _ALPACA_PI_

//**************************************************************************************
void	TestJulianDate(void)
{
int		month	=	11;
int		day		=	24;
int		year	=	2020;
double	julianDate_jlib;

	for (month = 1; month<=12; month++)
	{
		for (day = 1; day<=30; day++)
		{
			julianDate_jlib	=	Julian_CalcFromDate(month, day, year);
			printf("%2d/%2d/%4d\tJulian Date=%f\r\n",
										month,
										day,
										year,
										julianDate_jlib);
		}
	}
}

//**************************************************************************************
int main(int argc, char *argv[])
{
double	moonAge_Days;
double	illumination;
int		month	=	11;
int		day		=	24;
int		year	=	2020;
double	julianDate_CalcJD;
double	julianDate_jlib;
char	moonPhaseStr[64];

//	moonAge_Days	=	CalcDaysSinceNewMoon(month, day, year);
//	printf("moonAge\t\t\t=%f\r\n",		moonAge_Days);

//	illumination	=	CalcMoonIllumination(month, day, year);
//	printf("illumination\t\t\t=%f\r\n",		illumination);

//	TestJulianDate();

	for (month = 1; month<=12; month++)
	{
		for (day = 1; day<=30; day++)
		{
			moonAge_Days	=	CalcDaysSinceNewMoon(month, day, year);
			illumination	=	CalcMoonIllumination(month, day, year);

			GetMoonPhase(illumination, moonAge_Days, moonPhaseStr);
			printf("%2d/%2d/%4d\tage=%4.1f\tillumination\t=%7.3f\t=%2.0f\t%s\r\n",
										month,
										day,
										year,
										moonAge_Days,
										illumination,
										illumination,
										moonPhaseStr);
		}
	}

	julianDate_CalcJD	=	CalcJD(12, 1, 2020);
	julianDate_jlib		=	Julian_GetCurrentDate();

	printf("CalcJD=%f\tjlib=%f\r\n", julianDate_CalcJD, julianDate_jlib);
	printf("-----------------------\r\n");
}
#endif


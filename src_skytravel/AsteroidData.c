//*****************************************************************************
//*	AsteroidData.c
//*		https://asteroid.lowell.edu/main/astorb/
//*
//*		https://create.arduino.cc/projecthub/30506/calculation-of-right-ascension-and-declination-402218
//*		https://paulsite.com/calculation_of_right_ascension_and_declination/
//*		http://people.tamu.edu/~kevinkrisciunas/ra_dec_sun_2021.html
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Dec 27,	2021	<MLS> Created AsteroidData.c
//*	Dec 29,	2021	<MLS> Added ReadAsteroidDataAstorb()
//*	Dec 29,	2021	<MLS> With the help of Clif, we have the code working, now lets test it.
//*	Dec 30,	2021	<MLS> Added UpdateAsteroidEphemeris()
//*	Dec 31,	2021	<MLS> Got asteroid calculations 95% correct (matching Lowell)
//*	Jan  1,	2022	<MLS> Added ReadAsteroidData()
//*	Jan  1,	2022	<MLS> Added ReadAsteroidDataMPC()
//*	Jan  1,	2022	<MLS> Added support for Minor Planet Center asteroid data
//*	Jan  3,	2022	<MLS> Tested asteroid display on Raspberry-Pi, works, but slow
//*	Jan 24,	2022	<MLS> Asteroids without magnitude now default to 10
//*****************************************************************************

#ifdef _ENABLE_ASTERIODS_


#include	<time.h>
#include	<stdbool.h>
#include	<math.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	"julianTime.h"
#include	"helper_functions.h"

#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))
#define	DEGREES(radians)	((radians) * (180.0 / M_PI))

#define	DEG_MIN_SEC(degrees, minutes, seconds)	((degrees) + ((minutes* 1.0) / 60.0) + (seconds / 3600.0))


//*	MLS Libraries
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"AsteroidData.h"

//************************************************************************
static void	StripLeadingSpaces(char *theString)
{
int		iii;
int		ccc;

	ccc		=	0;
	iii		=	0;
	while (theString[iii] != 0)
	{
		if ((ccc == 0) && (theString[iii] == 0x20))
		{
			//*	do nothing
		}
		else
		{
			theString[ccc++]	=	theString[iii];
		}
		iii++;
	}
	theString[ccc]	=	0;
}

//*****************************************************************************
void	StripTrailingSpaces(char *theString)
{
int		ii;
int		sLen;

	sLen	=	strlen(theString);
	ii		=	sLen - 1;
	while (ii > 0)
	{
		if (theString[ii] <= 0x20)
		{
			theString[ii]	=	0;
		}
		else
		{
			break;
		}
		ii--;
	}
}

//**************************************************************************
static void	ExtractTextField(	const char	*theLine,
								const int	startIdx,
								const int	fieldLen,
								char		*outputText,
								bool		removeLeadingSpaces)
{
int		iii;
int		ccc;

	iii	=	startIdx;
	ccc	=	0;
	while (ccc < fieldLen)
	{
		outputText[ccc]	=	theLine[iii];
		ccc++;
		iii++;
	}
	outputText[ccc]	=	0;

//	CONSOLE_DEBUG_W_STR("outputText:", outputText);

	if (removeLeadingSpaces)
	{
		StripLeadingSpaces(outputText);
	}
}



//************************************************************************
//*			The Asteroid Orbital Elements Database
//*				Lowell Minor Planet Services
//*		https://asteroid.lowell.edu/main/astorb/
//************************************************************************
//(1)    (2)                (3)             (4)    (5)  (6)  (7)   (8)
//     1 Ceres              E. Bowell        3.34  0.12 0.72 913.0 G?
//  1693 Hertzsprung        E. Bowell       10.97  0.15 0.74  39.5 C
//         0         0         0         0         0         0         0
//         1         2         3         4         5         6         7
//1234567890123456789012345678901234567890123456789012345678901234567890
//
//   (9)                   (10) (11)  (12)     (13)       (14)       (15)
//   0   0   0   0   0   0 56959 4750 19960427  80.477333  71.802404  80
//   0   0   0   0   0   0 20972   25 19960427 322.276332 234.698906  70
//         0         0         1         1         1         1         1
//         8         9         0         1         2         3         4
//1234567890123456789012345678901234567890123456789012345678901234567890
//
//        (16)      (17)       (18)        (19)     (20)     (21)    (22)
//.659857 10.600303 0.07604100   2.76788714 19960414 2.3E-02  1.4E-04 19
//.393559 11.942428 0.27460300   2.79629204 19950513 9.0E-01  7.9E-03 19
//         1         1         1         1         1         2         2
//         5         6         7         8         9         0         1
//1234567890123456789012345678901234567890123456789012345678901234567890
//
//      (23)             (24)             (25)
//960416 2.7E-02 19960530 3.1E-02 20040111 3.1E-02 20040111
//960416 1.2E+00 19960610 1.3E+00 20010812 9.0E-01 20010813
//         2         2         2         2         2
//         2         3         4         5         6
//123456789012345678901234567890123456789012345678901234567

//(1) 	Asteroid number (blank if unnumbered).
//(2) 	Name or preliminary designation.
//(3) 	Orbit computer.
//(4) 	Absolute magnitude H, mag [see E. Bowell et al., pp. 549-554, in "Asteroids II", R. P. Binzel et al. (eds.), The University of Arizona Press, Tucson, 1989 and more recent Minor Planet Circulars]. Note that H may be given to 2 decimal places (e.g., 13.41), 1 decimal place (13.4) or as an integer (13), depending on its estimated accuracy. H is given to two decimal places for all unnumbered asteroids, even though it may be very poorly known.
//(5) 	Slope parameter G ( ibid.).
//(6) 	Color index B-V, mag (blank if unknown; see E. F. Tedesco, pp. 1090-1138, op. cit. ).
//(7) 	IRAS diameter, km (blank if unknown; see E. F. Tedesco et al., pp. 1151-1161, op.cit.).
//(8) 	IRAS Taxonomic classification (blank if unknown; ibid.).
//(9) 	Six integer codes (see table of explanation below). Note that not all codes have been correctly computed.
//(10) 	Orbital arc, days, spanned by observations used in orbit computation.
//(11) 	Number of observations used in orbit computation.
//(12) 	Epoch of osculation, yyyymmdd (TDT). The epoch is the Julian date ending in 00.5 nearest the date the file was created. Thus, as the file is updated, epochs will succeed each other at 100-day intervals on or after Julian dates ending in 50.5 (19980328, 19980706, 19981014, 19990122,...)
//(13) 	Mean anomaly, deg.
//(14) 	Argument of perihelion, deg (J2000.0).
//(15) 	Longitude of ascending node, deg (J2000.0).
//(16) 	Inclination, deg (J2000.0).
//(17) 	Eccentricity.
//(18) 	Semimajor axis, AU.
//(19) 	Date of orbit computation, yymmdd (MST, = UTC - 7 hr).
//(20) 	Absolute value of the current 1-σ ephemeris uncertainty (CEU), arcsec.
//(21) 	Rate of change of CEU, arcsec/day.
//(22) 	Date of CEU, yyyymmdd (0 hr UT).
//(23) 	Next peak ephemeris uncertainty (PEU), arcsec, from date of CEU, and date of its occurrence, yyyymmdd.
//(24) 	Greatest PEU, arcsec, in 10 years from date of CEU, and date of its occurrence, yyyymmdd.
//(25) 	Greatest PEU, arcsec, in 10 years from date of next PEU, and date of its occurrence, yyyymmdd, if two observations (of accuracy equal to that of the observations currently included in the orbit--typically ± 1 arcsec) were to be made on the date of the next PEU [parameter (23)].
//
//************************************************************************


//**************************************************************************
//*	The equations used in this code came from the formulas described in chapter 7
//*	of this book
//*		[MAA] "Math for Amateur Astronomers" by James H. Fox
//*			published by "The Astronomical League"
//*			copyright 1982, 1988 by James H. Fox
//*			1992 printing
//**************************************************************************


static bool	gEnableDebug	=	false;

//**************************************************************************
static double	KeplersEquations_CalcE(int loopMax, double MMM_radians, double eccentricity)
{
double	myEEE;
int		loopCnt;
double	previousEEE;
double	deltaEEE;

	if (gEnableDebug)
	{
		CONSOLE_DEBUG_W_DBL("MMM_radians\t=",		MMM_radians);
		CONSOLE_DEBUG_W_DBL("eccentricity\t=",		eccentricity);
	}
	//*	[MAA] Equation (7.3) page 31
	loopCnt		=	0;
	myEEE		=	MMM_radians;	//*	initial value
	deltaEEE	=	1;
	while ((loopCnt < loopMax) && (deltaEEE > 0.00000001))
	{
		previousEEE	=	myEEE;
		myEEE		=	MMM_radians + (eccentricity * sin(myEEE));
		if (gEnableDebug)
		{
			CONSOLE_DEBUG_W_DBL("myEEE\t=",			DEGREES(myEEE));
		}
		deltaEEE	=	fabs(myEEE - previousEEE);
		loopCnt++;
	}
	return(myEEE);
}


//**************************************************************************
//*	this calculates the data that does not change and only needs to be called once
//**************************************************************************
static void	ComputeAsteroidData_Static(TYPE_Asteroid *asteroidData)
{
double	omegaL_argOfPerihelion_rad;		//*	omegaL means lower case omega
double	omegaU_longitude_rad;				//*	omegaU means upper case omega
double	alpha1;
double	alpha2;
double	beta1;
double	beta2;
double	gama1;
double	gama2;
double	ecliptic_angle;			//*	degrees
double	ecliptic_angle_rad;		//*	radians
double	inclination_rad;

char	yearString[16];
char	monthString[16];
char	dateString[16];
int		ccc;
int		epochMonth;
int		epochDay;
int		epochYear;

	//============================================================
	//*	Compute the orbital period from the Equation  a^3 = T^2
	//*		where a	= semi-Major axis  (expressed in AU)
	//*		T = period (expressed in years)
	//*	http://astronomy.nmsu.edu/nicole/teaching/ASTR505/lectures/lecture08/slide13.html
	//============================================================
	asteroidData->Period				=	sqrt(asteroidData->SemimajorAxis * asteroidData->SemimajorAxis * asteroidData->SemimajorAxis);
	asteroidData->PeriodDays			=	asteroidData->Period * 365.256;
	if (asteroidData->n0_dailyMotion_deg == 0.0)
	{
		//*	the MPC data has this value, the Lowell data does not
		asteroidData->n0_dailyMotion_deg	=	360.0 / asteroidData->PeriodDays;
	}

	//	ε = 23°26′21.45″ − 46.815″ T − 0.0006″ T2 + 0.00181″ T3
//	ecliptic_angle			=	DEG_MIN_SEC(23, 26, 44.8);
#ifdef _INCLUDE_ASTEROID_MAIN_
	//*	from example problem [MAA] 7.2
	ecliptic_angle			=	DEG_MIN_SEC(23, 26, 33.12);
#else
	ecliptic_angle			=	DEG_MIN_SEC(23, 26, 21.45);
#endif
	ecliptic_angle_rad		=	RADIANS(ecliptic_angle);
	inclination_rad			=	RADIANS(asteroidData->Inclination);

	omegaL_argOfPerihelion_rad	=	RADIANS(asteroidData->ArgOfPerihelion);
	omegaU_longitude_rad		=	RADIANS(asteroidData->Longitude);

	//*	[MAA] Equation (7.8) page 33
	alpha1					=	sin(omegaU_longitude_rad) * sin(omegaL_argOfPerihelion_rad);
	alpha2					=	sin(omegaU_longitude_rad) * cos(omegaL_argOfPerihelion_rad);

	beta1					=	cos(omegaU_longitude_rad) * sin(omegaL_argOfPerihelion_rad);
	beta2					=	cos(omegaU_longitude_rad) * cos(omegaL_argOfPerihelion_rad);

	gama1					=	sin(inclination_rad) * sin(omegaL_argOfPerihelion_rad);
	gama2					=	sin(inclination_rad) * cos(omegaL_argOfPerihelion_rad);


	//*	calculate the P coordinates
	//*	[MAA] Equation (7.9) page 33
	asteroidData->Px	=	beta2 - (alpha1 * cos(inclination_rad));

	asteroidData->Py	=	(alpha2 + (beta1 * cos(inclination_rad))) * cos(ecliptic_angle_rad);
	asteroidData->Py	-=	gama1 * sin(ecliptic_angle_rad);

	asteroidData->Pz	=	(alpha2 + (beta1 * cos(inclination_rad))) * sin(ecliptic_angle_rad);
	asteroidData->Pz	+=	gama1 * cos(ecliptic_angle_rad);

	//*	now for the Q coordinates
	asteroidData->Qx	=	-beta1 - (alpha2 * cos(inclination_rad));

	asteroidData->Qy	=	(-alpha1 + (beta2 * cos(inclination_rad))) * cos(ecliptic_angle_rad);
	asteroidData->Qy	-=	gama2 * sin(ecliptic_angle_rad);

	asteroidData->Qz	=	(-alpha1 + (beta2 * cos(inclination_rad))) * sin(ecliptic_angle_rad);
	asteroidData->Qz	+=	gama2 * cos(ecliptic_angle_rad);

	//*	now figure out the julian dates of stuff

	strncpy(yearString,	asteroidData->EpochOfOsculation, 4);
	yearString[4]	=	0;

	ccc	=	4;
	monthString[0]	=	asteroidData->EpochOfOsculation[ccc++];
	monthString[1]	=	asteroidData->EpochOfOsculation[ccc++];
	monthString[2]	=	0;

	dateString[0]	=	asteroidData->EpochOfOsculation[ccc++];
	dateString[1]	=	asteroidData->EpochOfOsculation[ccc++];
	dateString[2]	=	0;

	epochMonth	=	atoi(monthString);
	epochDay	=	atoi(dateString);
	epochYear	=	atoi(yearString);

//	CONSOLE_DEBUG_W_NUM("epochYear\t\t=",			epochYear);
//	CONSOLE_DEBUG_W_NUM("epochMonth\t\t=",			epochMonth);
//	CONSOLE_DEBUG_W_NUM("epochDay\t\t=",			epochDay);

	asteroidData->epochJulian	=	Julian_CalcFromDate(epochMonth, epochDay, epochYear);

	asteroidData->StarData.realMagnitude	=	asteroidData->AbsoluteMagnitude;

}

//**************************************************************************
//*	this calculates the data that needs to be updated often
//**************************************************************************
static void	ComputeAsteroidData_Dynamic(	TYPE_Asteroid	*asteroidData,
											const double	targetJulianDate,
											const bool		validSunData,
											const double	sun_rtAscen,	//*	radians
											const double	sun_decl,		//*	radians
											const double	sun_dist)		//*	AU
{
double	actualAnomaly_M;
double	deltaDays;

double	eccentricAnomaly_E;		//*	a.k.a E
double	xxx;
double	yyy;
double	zzz;
double	cosE_minus_e;		//*	value of "(cos(E) - e)"
double	a_cosE_minus_e;		//*	value of "a (cos(E) - e)"
double	a_sqrt_1_minus_e_sqrd;
double	sinE;
double	rrr;
double	eee_sqrd;

double	sunXXX;
double	sunYYY;
double	sunZZZ;
double	xi;
double	eta;
double	zeta;
double	deltaEarth;


	asteroidData->targetJulian	=	targetJulianDate;


	//*	calculate the # of days offset from ephoch
	deltaDays	=	asteroidData->targetJulian - asteroidData->epochJulian;

	//*	calculate the actual Anomaly
	//*	[MAA] Equation (7.10) page 33
	actualAnomaly_M	=	asteroidData->MeanAnomaly + (asteroidData->n0_dailyMotion_deg * deltaDays);

	if (gEnableDebug)
	{
		CONSOLE_DEBUG_W_DBL("n0_dailyMotion_deg\t=",	asteroidData->n0_dailyMotion_deg);
		CONSOLE_DEBUG_W_DBL("deltaDays\t\t=",				deltaDays);
		CONSOLE_DEBUG_W_DBL("actualAnomaly_M\t=",		actualAnomaly_M);
	}

	//*	[MAA] Equation (7.3) page 31
	eccentricAnomaly_E	=	KeplersEquations_CalcE(50, RADIANS(actualAnomaly_M), (asteroidData->Eccentricity));


	//=========================================================
	//*	[MAA] Equation (7.11) page 33
	//*	first we are going to create some intermediate results to simplify and speed up the math
	//*	Also, this follows example 7.2 on page 35
	//*
	//*	the first term: a Px  (cos E -e)
	//*	change to: a cos(E -e) Px
	cosE_minus_e			=	cos(eccentricAnomaly_E) - asteroidData->Eccentricity;
	a_cosE_minus_e			=	asteroidData->SemimajorAxis * cosE_minus_e;
	eee_sqrd				=	asteroidData->Eccentricity * asteroidData->Eccentricity;
	a_sqrt_1_minus_e_sqrd	=	asteroidData->SemimajorAxis * sqrt(1.0 - eee_sqrd);
	sinE					=	sin(eccentricAnomaly_E);

	xxx	=	a_cosE_minus_e * asteroidData->Px;
	xxx	+=	a_sqrt_1_minus_e_sqrd * asteroidData->Qx * sinE;

	yyy	=	a_cosE_minus_e * asteroidData->Py;
	yyy	+=	a_sqrt_1_minus_e_sqrd * asteroidData->Qy * sinE;

	zzz	=	a_cosE_minus_e * asteroidData->Pz;
	zzz	+=	a_sqrt_1_minus_e_sqrd * asteroidData->Qz * sinE;

	//*	[MAA] Equation (7.12) page 33
	rrr	=	sqrt((xxx * xxx) + (yyy * yyy) + (zzz * zzz));
	if (gEnableDebug)
	{
		CONSOLE_DEBUG_W_DBL("cosE_minus_e\t\t=",		cosE_minus_e);
		CONSOLE_DEBUG_W_DBL("a_cosE_minus_e\t\t=",	a_cosE_minus_e);
		CONSOLE_DEBUG_W_DBL("a_sqrt_1_minus_e_sqrd\t=",	a_sqrt_1_minus_e_sqrd);
		CONSOLE_DEBUG_W_DBL("sinE\t\t\t=",				sinE);

		CONSOLE_DEBUG_W_DBL("sinE*a_sqrt_1_minus_e_sqrd\t=",	sinE * a_sqrt_1_minus_e_sqrd);

		CONSOLE_DEBUG_W_DBL("xxx\t\t\t\t=",		xxx);
		CONSOLE_DEBUG_W_DBL("yyy\t\t\t\t=",		yyy);
		CONSOLE_DEBUG_W_DBL("zzz\t\t\t\t=",		zzz);
		CONSOLE_DEBUG_W_DBL("rrr\t\t\t\t=",		rrr);
	}

	//*	we need to calculate the heliocentric to geocentric offsets
	if (validSunData)
	{
		//*	[MAA] Equation (7.20) page 34
		sunXXX	=	sun_dist * cos(sun_rtAscen) * cos(sun_decl);
		sunYYY	=	sun_dist * sin(sun_rtAscen) * cos(sun_decl);
		sunZZZ	=	sun_dist * sin(sun_decl);

#ifdef _INCLUDE_ASTEROID_MAIN_
//		if (asteroidData->AsteroidNumber == 1)
//		{
//			CONSOLE_DEBUG_W_DBL("sun_rtAscen\t=",	DEGREES(sun_rtAscen)/15);
//			CONSOLE_DEBUG_W_DBL("sun_decl   \t=",	DEGREES(sun_decl));
//			CONSOLE_DEBUG_W_DBL("sun_dist   \t=",	sun_dist);
//			CONSOLE_DEBUG_W_DBL("sunXXX     \t=",	sunXXX);
//			CONSOLE_DEBUG_W_DBL("sunYYY     \t=",	sunYYY);
//			CONSOLE_DEBUG_W_DBL("sunZZZ    \t=",	sunZZZ);
//		}
#endif // _INCLUDE_ASTEROID_MAIN_
	}
	else
	{
		//*	we have to have something
		//*	from the book example, [MAA] page 36, it gives these values for the date Oct 4, 1977
		sunXXX	=	-0.9829983;
		sunYYY	=	-0.1697915;
		sunZZZ	=	-0.0736272;
	}

	//*	[MAA] Equation (7.18) page 34
	xi		=	xxx + sunXXX;
	eta		=	yyy + sunYYY;
	zeta	=	zzz + sunZZZ;

	//*	[MAA] Equation (7.19) page 34
	deltaEarth	=	sqrt((xi * xi) + (eta * eta) + (zeta * zeta));


	//*	[MAA] Equation (7.21) page 34
	asteroidData->StarData.decl		=	asin(zeta / deltaEarth);

	//*	[MAA] Equation (7.22) page 34
	asteroidData->StarData.ra		=	atan2(eta, xi);
	//*	[MAA] page 35, top paragraph says to do this, it DOES NOT WORK
	//*	it puts all of the asteroids on the same half
//	if (xi < 0.0)
//	{
//		asteroidData->StarData.ra	+=	M_PI;
//	}
//	else if (eta < 0.0)
//	{
//		asteroidData->StarData.ra	+=	2 * M_PI;
//	}


	//*	copy the data over to the original value as well (not really used, but just in case)
	asteroidData->StarData.org_ra	=	asteroidData->StarData.ra;
	asteroidData->StarData.org_decl	=	asteroidData->StarData.decl;

	//*	keep a copy for debugging.
	asteroidData->sunXXX	=	sunXXX;
	asteroidData->sunYYY	=	sunYYY;
	asteroidData->sunZZZ	=	sunZZZ;
	asteroidData->delta_AUfromEarth	=	deltaEarth;

}

//**************************************************************************
static bool	ParseOneLineAstorbData(const char *lineBuff, TYPE_Asteroid *asteroidData)
{
bool	validData;
char	argString[64];

	validData	=	false;
	if (strlen(lineBuff) > 100)
	{
		validData	=	true;
		asteroidData->StarData.dataSrc			=	kDataSrc_Asteroids;

		//(1) 	Asteroid number (blank if unnumbered).
		ExtractTextField(lineBuff,	(1-1),	6,	argString, true);
		asteroidData->AsteroidNumber	=	atoi(argString);
		asteroidData->StarData.id		=	asteroidData->AsteroidNumber;

		//(2) 	Name or preliminary designation.
		ExtractTextField(lineBuff,	(8-1),	18,	asteroidData->AsteroidName, true);
		StripTrailingSpaces(asteroidData->AsteroidName);

		strcpy(asteroidData->StarData.longName, asteroidData->AsteroidName);

		//(3) 	Orbit computer.
		//(4) 	Absolute magnitude H, mag [see E. Bowell et al., pp. 549-554, in "Asteroids II", R. P. Binzel et al. (eds.), The University of Arizona Press, Tucson, 1989 and more recent Minor Planet Circulars]. Note that H may be given to 2 decimal places (e.g., 13.41), 1 decimal place (13.4) or as an integer (13), depending on its estimated accuracy. H is given to two decimal places for all unnumbered asteroids, even though it may be very poorly known.
		ExtractTextField(lineBuff,	(43 - 1),	5,	argString, true);
		if (strlen(argString) > 0)
		{
			asteroidData->AbsoluteMagnitude	=	atof(argString);
		}
		else
		{
			asteroidData->AbsoluteMagnitude	=	10;
		}

		//(5) 	Slope parameter G ( ibid.).
		ExtractTextField(lineBuff,	(50 - 1),	5,	argString, true);
		asteroidData->Slope	=	atof(argString);

		//(6) 	Color index B-V, mag (blank if unknown; see E. F. Tedesco, pp. 1090-1138, op. cit. ).
		//(7) 	IRAS diameter, km (blank if unknown; see E. F. Tedesco et al., pp. 1151-1161, op.cit.).
		//(8) 	IRAS Taxonomic classification (blank if unknown; ibid.).
		//(9) 	Six integer codes (see table of explanation below). Note that not all codes have been correctly computed.
		//(10) 	Orbital arc, days, spanned by observations used in orbit computation.
		//(11) 	Number of observations used in orbit computation.

		//(12) 	Epoch of osculation, yyyymmdd (TDT). The epoch is the Julian date ending in 00.5 nearest the date the file was created.
		//			Thus, as the file is updated, epochs will succeed each other at 100-day intervals on or after Julian dates ending in 50.5 (19980328, 19980706, 19981014, 19990122,...)
		ExtractTextField(lineBuff,	(107 - 1),	8,	asteroidData->EpochOfOsculation, true);

		//(13) 	Mean anomaly, deg.
		ExtractTextField(lineBuff,	(116 - 1),	10,	argString, true);
		asteroidData->MeanAnomaly	=	atof(argString);

		//(14) 	Argument of perihelion, deg (J2000.0).
		ExtractTextField(lineBuff,	(127 - 1),	10,	argString, true);
		asteroidData->ArgOfPerihelion	=	atof(argString);

		//(15) 	Longitude of ascending node, deg (J2000.0).
		ExtractTextField(lineBuff,	(138 - 1),	10,	argString, true);
		asteroidData->Longitude	=	atof(argString);

		//(16) 	Inclination, deg (J2000.0).
		ExtractTextField(lineBuff,	(149 - 1),	9,	argString, true);
		asteroidData->Inclination	=	atof(argString);

		//(17) 	Eccentricity.
		ExtractTextField(lineBuff,	(159 - 1),	10,	argString, true);
		asteroidData->Eccentricity	=	atof(argString);

		//(18) 	Semimajor axis, AU.
		ExtractTextField(lineBuff,	(170 - 1),	12,	argString, true);
		asteroidData->SemimajorAxis	=	atof(argString);

		//(19) 	Date of orbit computation, yymmdd (MST, = UTC - 7 hr).

		//(20) 	Absolute value of the current 1-σ ephemeris uncertainty (CEU), arcsec.
		//*	(CEU = current ephemeris uncertainty)
		ExtractTextField(lineBuff,	(191 - 1),	9,	argString, true);
		asteroidData->AbsValueCEU	=	atof(argString);


		//(21) 	Rate of change of CEU, arcsec/day.
		ExtractTextField(lineBuff,	(200 - 1),	8,	argString, true);
		asteroidData->RateOfChangeCEU	=	atof(argString);


		//(22) 	Date of CEU, yyyymmdd (0 hr UT).
		ExtractTextField(lineBuff,	(208 - 1),	9,	asteroidData->DateOfCEU, true);

		//(23) 	Next peak ephemeris uncertainty (PEU), arcsec, from date of CEU, and date of its occurrence, yyyymmdd.
		//(24) 	Greatest PEU, arcsec, in 10 years from date of CEU, and date of its occurrence, yyyymmdd.
		//(25) 	Greatest PEU, arcsec, in 10 years from date of next PEU, and date of its occurrence, yyyymmdd, if two observations (of accuracy equal to that of the observations currently included in the orbit--typically ± 1 arcsec) were to be made on the date of the next PEU [parameter (23)].

	}

	return(validData);
}

#define		kMaxAsteroids	(1161751 + 100)

//**************************************************************************
//*	returns the pointer to the new array
//**************************************************************************
static TYPE_Asteroid	*ReadAsteroidDataAstorb(const char *filePath, long *asteroidCount)
{
FILE				*filePointer;
char				lineBuff[2048];
char				currentName[128];
char				myFilePath[128];
int					lineLength;
int					linesRead;
bool				validData;
int					maxLineLength;
int					asteroidIdx;
TYPE_Asteroid		asteroidData;
TYPE_Asteroid		*asteroidArray;
double				targetJulian;

	CONSOLE_DEBUG(__FUNCTION__);

	asteroidArray	=	NULL;
	if (filePath != NULL)
	{
		strcpy(myFilePath, filePath);
	}
	else
	{
		strcpy(myFilePath, "asteroids");
		strcat(myFilePath, "/");
		strcat(myFilePath, "astorb.dat");
	}
	asteroidIdx		=	0;
	linesRead		=	0;
	maxLineLength	=	0;
	filePointer		=	fopen(myFilePath, "r");
	if (filePointer != NULL)
	{
//		CONSOLE_DEBUG("File Open");

		asteroidArray	=	(TYPE_Asteroid *)calloc(kMaxAsteroids, sizeof(TYPE_Asteroid));
		if (asteroidArray != NULL)
		{

			strcpy(currentName, "");
			targetJulian	=	Julian_GetCurrentDate();

			linesRead		=	0;
			while (fgets(lineBuff, 2000, filePointer) && (linesRead < kMaxAsteroids))
	//		while (fgets(lineBuff, 2000, filePointer) && (linesRead < 2))
			{
				linesRead++;
				lineLength	=	strlen(lineBuff);
				if (lineLength > maxLineLength)
				{
					maxLineLength	=	lineLength;
				}
				//*	get rid of trailing CR/LF and spaces
				while ((lineBuff[lineLength - 1] <= 0x20) && (lineLength > 1))
				{
					lineBuff[lineLength - 1]	=	0;
					lineLength	=	strlen(lineBuff);
				}
				if (lineLength > 100)
				{
					memset(&asteroidData, 0, sizeof(TYPE_Asteroid));
					validData	=	ParseOneLineAstorbData(lineBuff, &asteroidData);

					if (validData)
					{
						//*	calculate the asteroid RA/DEC stuff
						ComputeAsteroidData_Static(&asteroidData);
						ComputeAsteroidData_Dynamic(&asteroidData, targetJulian, false, 0.0, 0.0, 0.0);
						if (asteroidIdx < kMaxAsteroids)
						{
							asteroidArray[asteroidIdx]	=	asteroidData;
							asteroidIdx++;
						}
					}
					else
					{

					}
				}
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open file:", filePath);
	}
	CONSOLE_DEBUG_W_NUM("linesRead:", linesRead);
	CONSOLE_DEBUG_W_NUM("maxLineLength:", maxLineLength);

	*asteroidCount	=	asteroidIdx;
	return(asteroidArray);
}

//*****************************************************************************
void			UpdateAsteroidEphemeris(	TYPE_Asteroid	*asteroidData,
											const long		asteriodCnt,
											const double	targetJulianDate,	//*	julian Date
											const double	sun_rtAscen,		//*	radians
											const double	sun_decl,			//*	radians
											const double	sun_dist)			//*	AU
{
int				iii;

//	CONSOLE_DEBUG_W_LONG("asteriodCnt\t=",	asteriodCnt);

	for (iii=0; iii<asteriodCnt; iii++)
	{
		ComputeAsteroidData_Dynamic(&asteroidData[iii],
									targetJulianDate,
									true,
									sun_rtAscen,	//*	radians
									sun_decl,		//*	radians
									sun_dist);		//*	AU
	}
}

//**************************************************************************
static void	DumpAsteroidData(TYPE_Asteroid *asteroidData)
{
	CONSOLE_DEBUG_W_NUM("AsteroidNumber\t=",	asteroidData->AsteroidNumber);
	CONSOLE_DEBUG_W_STR("AsteroidName\t=",		asteroidData->AsteroidName);
	CONSOLE_DEBUG_W_DBL("AbsoluteMag\t=",		asteroidData->AbsoluteMagnitude);
//	CONSOLE_DEBUG_W_DBL("Slope\t\t=",			asteroidData->Slope);
	CONSOLE_DEBUG_W_STR("EpochOfOsculation\t=",	asteroidData->EpochOfOsculation);
	CONSOLE_DEBUG_W_DBL("MeanAnomaly\t=",		asteroidData->MeanAnomaly);
	CONSOLE_DEBUG_W_DBL("ArgOfPerihelion\t=",	asteroidData->ArgOfPerihelion);
	CONSOLE_DEBUG_W_DBL("Longitude  \t=",		asteroidData->Longitude);
	CONSOLE_DEBUG_W_DBL("Inclination\t=",		asteroidData->Inclination);
	CONSOLE_DEBUG_W_DBL("Eccentricity\t=",		asteroidData->Eccentricity);
	CONSOLE_DEBUG_W_DBL("SemimajorAxis\t=",		asteroidData->SemimajorAxis);
//	CONSOLE_DEBUG_W_DBL("AbsValueCEU\t=",		asteroidData->AbsValueCEU);
//	CONSOLE_DEBUG_W_DBL("RateOfChangeCEU\t=",	asteroidData->RateOfChangeCEU);
//	CONSOLE_DEBUG_W_STR("DateOfCEU\t\t=",		asteroidData->DateOfCEU);
}

//**************************************************************************
static void	DumpAsteroidCalcs(TYPE_Asteroid *asteroidData)
{
char			timeString[64];
struct timeval 	timeStruct;

	CONSOLE_DEBUG_W_DBL("Px\t\t\t=",			asteroidData->Px);
	CONSOLE_DEBUG_W_DBL("Py\t\t\t=",			asteroidData->Py);
	CONSOLE_DEBUG_W_DBL("Pz\t\t\t=",			asteroidData->Pz);
	CONSOLE_DEBUG_W_DBL("Qx\t\t\t=",			asteroidData->Qx);
	CONSOLE_DEBUG_W_DBL("Qy\t\t\t=",			asteroidData->Qy);
	CONSOLE_DEBUG_W_DBL("Qz\t\t\t=",			asteroidData->Qz);

	CONSOLE_DEBUG_W_DBL("Period\t\t=",			asteroidData->Period);
	CONSOLE_DEBUG_W_DBL("PeriodDays\t\t=",		asteroidData->PeriodDays);
	CONSOLE_DEBUG_W_DBL("n0_dailyMotion\t=",	asteroidData->n0_dailyMotion_deg);
	CONSOLE_DEBUG_W_DBL("epochJulian\t=",		asteroidData->epochJulian);

	CONSOLE_DEBUG_W_DBL("sunXXX\t\t=",			asteroidData->sunXXX);
	CONSOLE_DEBUG_W_DBL("sunYYY\t\t=",			asteroidData->sunYYY);
	CONSOLE_DEBUG_W_DBL("sunZZZ\t\t=",			asteroidData->sunZZZ);
	CONSOLE_DEBUG_W_DBL("delta_AUfromEarth\t=",	asteroidData->delta_AUfromEarth);

	Julian_GetUTCfromJulian(asteroidData->epochJulian, &timeStruct);
	FormatTimeStringISO8601(&timeStruct, timeString);
	CONSOLE_DEBUG_W_STR("epochJulian\t=",		timeString);

}



static int	gDebugCounter	=	0;

//*****************************************************************************
void	PrintAsteroidEphemeris(	TYPE_Asteroid	*asteroidData,
								const double	targetJulianDate,	//*	julian Date
								const double	sun_rtAscen,		//*	radians
								const double	sun_decl,			//*	radians
								const double	sun_dist)			//*	AU
{
double			myJulianDate;
int				iii;
char			ra_string[32];
char			dec_string[32];
char			timeString[64];
struct timeval 	timeStruct;
double			delta_RA;		//*	arc secs / minute
double			delta_DEC;		//*	arc secs / minute
double			previousRA;
double			previousDEC;
int				outputLimit;

	printf("*****************************************\r\n");
	printf("Ephemeris for %s\t#%d\r\n", asteroidData->AsteroidName, gDebugCounter++);
	printf("targetJulianDate\t=%f\r\n", targetJulianDate);

	FormatHHMMSSdd((DEGREES(sun_rtAscen) / 15),	ra_string, false);
	FormatHHMMSSdd(DEGREES(sun_decl),			dec_string, true);

	printf("sun_rtAscen\t\t=%s\r\n",	ra_string);
	printf("sun_decl\t\t=%s\r\n",		dec_string);
	printf("sun_dist\t\t=%f\r\n",		sun_dist);

#ifdef _INCLUDE_ASTEROID_MAIN_
	outputLimit	=	2;
#else
	outputLimit	=	24;
#endif
	myJulianDate	=	targetJulianDate;
	previousRA		=	0.0;
	previousDEC		=	0.0;
	for (iii =0; iii<outputLimit; iii++)
	{
		ComputeAsteroidData_Dynamic(	asteroidData,
										myJulianDate,
										true,
										sun_rtAscen,
										sun_decl,
										sun_dist);

		FormatHHMMSSdd((DEGREES(asteroidData->StarData.ra) / 15),	ra_string, false);
		FormatHHMMSSdd((DEGREES(asteroidData->StarData.decl)),		dec_string, true);

		Julian_GetUTCfromJulian(myJulianDate, &timeStruct);
		FormatTimeStringISO8601(&timeStruct, timeString);

		printf("J=%f\tUTC=%s\tRA=%s\tDEC=%s",	myJulianDate,
												timeString,
												ra_string,
												dec_string);

		if (iii >  0)
		{
			//*	divide by 60 is so that it is change per minute
			delta_RA		=	(asteroidData->StarData.ra - previousRA) / 60.0;
			delta_DEC		=	(asteroidData->StarData.decl - previousDEC) / 60.0;
			printf("\tdeltaRA=%f", DEGREES(delta_RA) * 3600);
			printf("\tdeltaDEC=%f", DEGREES(delta_DEC) * 3600);
		}

		printf("\r\n");
		previousRA		=	asteroidData->StarData.ra;
		previousDEC		=	asteroidData->StarData.decl;

		//*	advance 1 hour
		myJulianDate	+=	(1.0/ 24.0);

	}
//	DumpAsteroidData(asteroidData);
//	DumpAsteroidCalcs(asteroidData);
}

//*****************************************************************************
//*	https://www.minorplanetcenter.net/iau/info/MPOrbitFormat.html
//*****************************************************************************
//Des'n     H     G   Epoch     M        Peri.      Node       Incl.       e            n           a        Reference #Obs #Opp    Arc    rms  Perts   Computer
//
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
//00001    3.54  0.15 K221L 291.37563   73.63703   80.26858   10.58769  0.0785011  0.21424745   2.7660431  0 MPO656898  7297 122 1801-2021 0.63 M-v 30k Pan        0000      (1) Ceres              20211019
//*****************************************************************************
//   Columns   F77    Use
//
//    1 -   7  a7     Number or provisional designation
//                      (in packed form)
//    9 -  13  f5.2   Absolute magnitude, H
//   15 -  19  f5.2   Slope parameter, G
//
//   21 -  25  a5     Epoch (in packed form, .0 TT)
//   27 -  35  f9.5   Mean anomaly at the epoch, in degrees
//
//   38 -  46  f9.5   Argument of perihelion, J2000.0 (degrees)
//   49 -  57  f9.5   Longitude of the ascending node, J2000.0
//                      (degrees)
//   60 -  68  f9.5   Inclination to the ecliptic, J2000.0 (degrees)
//
//   71 -  79  f9.7   Orbital eccentricity
//   81 -  91  f11.8  Mean daily motion (degrees per day)
//   93 - 103  f11.7  Semimajor axis (AU)
//
//  106        i1     Uncertainty parameter, U
//          or a1     If this column contains `E' it indicates
//                    that the orbital eccentricity was assumed.
//                    For one-opposition orbits this column can
//                    also contain `D' if a double (or multiple)
//                    designation is involved or `F' if an e-assumed
//                    double (or multiple) designation is involved.
//
//  108 - 116  a9     Reference
//  118 - 122  i5     Number of observations
//  124 - 126  i3     Number of oppositions
//
//     For multiple-opposition orbits:
//     128 - 131  i4     Year of first observation
//     132        a1     '-'
//     133 - 136  i4     Year of last observation
//
//     For single-opposition orbits:
//     128 - 131  i4     Arc length (days)
//     133 - 136  a4     'days'
//
//  138 - 141  f4.2   r.m.s residual (")
//  143 - 145  a3     Coarse indicator of perturbers
//                    (blank if unperturbed one-opposition object)
//  147 - 149  a3     Precise indicator of perturbers
//                    (blank if unperturbed one-opposition object)
//  151 - 160  a10    Computer name
//
//There may sometimes be additional information beyond column 160
//as follows:
//
//  162 - 165  z4.4   4-hexdigit flags
//
//                    This information has been updated 2014 July 16, for files
//                    created after 18:40 UTC on that day.  Classification of
//                    distant-orbit types will resume after we ingest data from
//                    an outside collaborator.
//
//                    The bottom 6 bits (bits 0 to 5) are used to encode
//                    a value representing the orbit type (other
//                    values are undefined):
//
//                     Value
//                        1  Atira
//                        2  Aten
//                        3  Apollo
//                        4  Amor
//                        5  Object with q < 1.665 AU
//                        6  Hungaria
//                        7  Unused or internal MPC use only
//                        8  Hilda
//                        9  Jupiter Trojan
//                       10  Distant object
//
//                    Additional information is conveyed by
//                    adding in the following bit values:
//
//               Bit  Value
//                 6     64  Unused or internal MPC use only
//                 7    128  Unused or internal MPC use only
//                 8    256  Unused or internal MPC use only
//                 9    512  Unused or internal MPC use only
//                10   1024  Unused or internal MPC use only
//                11   2048  Object is NEO
//                12   4096  Object is 1-km (or larger) NEO
//                13   8192  1-opposition object seen at
//                           earlier opposition
//                14  16384  Critical list numbered object
//                15  32768  Object is PHA
//
//                    Note that the orbit classification is
//                      based on cuts in osculating element
//                      space and is not 100% reliable.
//
//                    Note also that certain of the flags
//                      are for internal MPC use and are
//                      not documented.
//
//  167 - 194  a      Readable designation
//
//  195 - 202  i8     Date of last observation included in
//                      orbit solution (YYYYMMDD format)
//

//**************************************************************************
//07482   16.56  0.15 K221L 337.27014   47.47662  117.87792   33.47909  0.3296520  0.62915877   1.3488466  0 MPO656989   735  16 1974-2021 0.73 M-v 3Ek Pan        9803   (7482) 1994 PC1           20211014
//         0         0         0         0         0         0         0         0         0         1         1         1         1         1         1         1         1         1         1         2         2         2
//         1         2         3         4         5         6         7         8         9         0         1         2         3         4         5         6         7         8         9         0
//123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
//**************************************************************************

//**************************************************************************
//*	https://www.minorplanetcenter.net/iau/info/PackedDates.html
//**************************************************************************
//Packed Dates
//Dates of the form YYYYMMDD may be packed into five characters to conserve space.
//
//The first two digits of the year are packed into a single character in column 1 (I = 18, J = 19, K = 20). Columns 2-3 contain the last two digits of the year. Column 4 contains the month and column 5 contains the day, coded as detailed below:
//
//   Month     Day      Character         Day      Character
//                     in Col 4 or 5              in Col 4 or 5
//   Jan.       1           1             17           H
//   Feb.       2           2             18           I
//   Mar.       3           3             19           J
//   Apr.       4           4             20           K
//   May        5           5             21           L
//   June       6           6             22           M
//   July       7           7             23           N
//   Aug.       8           8             24           O
//   Sept.      9           9             25           P
//   Oct.      10           A             26           Q
//   Nov.      11           B             27           R
//   Dec.      12           C             28           S
//             13           D             29           T
//             14           E             30           U
//             15           F             31           V
//             16           G
//
//Examples:
//
//   1996 Jan. 1    = J9611
//   1996 Jan. 10   = J961A
//   1996 Sept.30   = J969U
//   1996 Oct. 1    = J96A1
//   2001 Oct. 22   = K01AM
//
//This system can be extended to dates with non-integral days. The decimal fraction of the day is simply appended to the five characters defined above.
//
//Examples:
//
//   1998 Jan. 18.73     = J981I73
//   2001 Oct. 22.138303 = K01AM138303
//**************************************************************************


//**************************************************************************
static void	UnPackMPCepochDate(char *packedstr, char *unpackedStr)
{
int		ccc;
int		month;
int		day;
char	yearString[8];
	ccc	=	0;
	switch (packedstr[0])
	{
		case 'I':
			yearString[ccc++]	=	'1';
			yearString[ccc++]	=	'8';
			break;
		case 'J':
			yearString[ccc++]	=	'1';
			yearString[ccc++]	=	'9';
			break;
		case 'K':
			yearString[ccc++]	=	'2';
			yearString[ccc++]	=	'0';
			break;
		default:
			yearString[ccc++]	=	'?';
			yearString[ccc++]	=	'?';
			break;
	}
	yearString[ccc++]	=	packedstr[1];
	yearString[ccc++]	=	packedstr[2];
	yearString[ccc++]	=	0;

	month	=	packedstr[3] & 0x0f;
	if (packedstr[3] >= 'A')
	{
		month	+=	9;
	}
	//*	the data is hexidecimal
	if (isdigit(packedstr[4]))
	{

		day	=	packedstr[4] & 0x0f;
	}
	else
	{
		day	=	(packedstr[4] & 0x1f) + 9;
	}
	sprintf(unpackedStr, "%4s%02d%02d", yearString, month, day);
}

//**************************************************************************
static bool	ParseOneLineMPCasteroidData(const char *lineBuff, TYPE_Asteroid *asteroidData)
{
bool	validData;
char	argString[64];
char	unpackedString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(lineBuff);
	validData	=	false;
	if (strlen(lineBuff) > 100)
	{
//		CONSOLE_DEBUG("OK");
		validData	=	true;
		asteroidData->StarData.dataSrc			=	kDataSrc_Asteroids;

		//    1 -   7  a7     Number or provisional designation
		//                      (in packed form)
		ExtractTextField(lineBuff,	(1-1),	7,	argString, true);
		asteroidData->AsteroidNumber	=	atoi(argString);
		asteroidData->StarData.id		=	asteroidData->AsteroidNumber;

		//    9 -  13  f5.2   Absolute magnitude, H
		ExtractTextField(lineBuff,	(9 - 1),	5,	argString, true);
		if (strlen(argString) > 0)
		{
			asteroidData->AbsoluteMagnitude	=	atof(argString);
		}
		else
		{
			asteroidData->AbsoluteMagnitude	=	10;
		}
//		if (asteroidData->AbsoluteMagnitude < 1)
//		{
//			CONSOLE_DEBUG_W_STR("AbsoluteMagnitude\t=",	argString);
//			CONSOLE_DEBUG_W_DBL("AbsoluteMagnitude\t=",	asteroidData->AbsoluteMagnitude);
//		}

		//   15 -  19  f5.2   Slope parameter, G
		ExtractTextField(lineBuff,	(15 - 1),	5,	argString, true);
		asteroidData->Slope	=	atof(argString);

		//   21 -  25  a5     Epoch (in packed form, .0 TT)
		ExtractTextField(lineBuff,	(21 - 1),	5,	argString, true);
		UnPackMPCepochDate(argString, unpackedString);
		strcpy(asteroidData->EpochOfOsculation, unpackedString);
#ifdef _INCLUDE_ASTEROID_MAIN_
//		CONSOLE_DEBUG_W_STR("EpochOfOsculation (packed)", argString);
//		CONSOLE_DEBUG_W_STR("EpochOfOsculation (unpacked)", unpackedString);
#endif

		//   27 -  35  f9.5   Mean anomaly at the epoch, in degrees
		ExtractTextField(lineBuff,	(27 - 1),	9,	argString, true);
		asteroidData->MeanAnomaly	=	atof(argString);

		//   38 -  46  f9.5   Argument of perihelion, J2000.0 (degrees)
		ExtractTextField(lineBuff,	(38 - 1),	9,	argString, true);
		asteroidData->ArgOfPerihelion	=	atof(argString);

		//   49 -  57  f9.5   Longitude of the ascending node, J2000.0 (degrees)
		ExtractTextField(lineBuff,	(49 - 1),	9,	argString, true);
		asteroidData->Longitude	=	atof(argString);

		//   60 -  68  f9.5   Inclination to the ecliptic, J2000.0 (degrees)
		ExtractTextField(lineBuff,	(60 - 1),	9,	argString, true);
		asteroidData->Inclination	=	atof(argString);

		//   71 -  79  f9.7   Orbital eccentricity
		ExtractTextField(lineBuff,	(71 - 1),	9,	argString, true);
		asteroidData->Eccentricity	=	atof(argString);


		//   81 -  91  f11.8  Mean daily motion (degrees per day)
		ExtractTextField(lineBuff,	(81 - 1),	11,	argString, true);
//		CONSOLE_DEBUG_W_STR("Mean daily motion:", argString);
		asteroidData->n0_dailyMotion_deg	=	atof(argString);

		//   93 - 103  f11.7  Semimajor axis (AU)
		ExtractTextField(lineBuff,	(93 - 1),	11,	argString, true);
		asteroidData->SemimajorAxis	=	atof(argString);

//  106        i1     Uncertainty parameter, U
//          or a1     If this column contains `E' it indicates
//                    that the orbital eccentricity was assumed.
//                    For one-opposition orbits this column can
//                    also contain `D' if a double (or multiple)
//                    designation is involved or `F' if an e-assumed
//                    double (or multiple) designation is involved.
//
//  108 - 116  a9     Reference
//  118 - 122  i5     Number of observations
//  124 - 126  i3     Number of oppositions
//
//     For multiple-opposition orbits:
//     128 - 131  i4     Year of first observation
//     132        a1     '-'
//     133 - 136  i4     Year of last observation
//
//     For single-opposition orbits:
//     128 - 131  i4     Arc length (days)
//     133 - 136  a4     'days'
//
//  138 - 141  f4.2   r.m.s residual (")
//  143 - 145  a3     Coarse indicator of perturbers
//                    (blank if unperturbed one-opposition object)
//  147 - 149  a3     Precise indicator of perturbers
//                    (blank if unperturbed one-opposition object)
//  151 - 160  a10    Computer name
//
//There may sometimes be additional information beyond column 160
//as follows:
//
//  162 - 165  z4.4   4-hexdigit flags
//
//                    This information has been updated 2014 July 16, for files
//                    created after 18:40 UTC on that day.  Classification of
//                    distant-orbit types will resume after we ingest data from
//                    an outside collaborator.
//
//                    The bottom 6 bits (bits 0 to 5) are used to encode
//                    a value representing the orbit type (other
//                    values are undefined):
//
//                     Value
//                        1  Atira
//                        2  Aten
//                        3  Apollo
//                        4  Amor
//                        5  Object with q < 1.665 AU
//                        6  Hungaria
//                        7  Unused or internal MPC use only
//                        8  Hilda
//                        9  Jupiter Trojan
//                       10  Distant object
//
//                    Additional information is conveyed by
//                    adding in the following bit values:
//
//               Bit  Value
//                 6     64  Unused or internal MPC use only
//                 7    128  Unused or internal MPC use only
//                 8    256  Unused or internal MPC use only
//                 9    512  Unused or internal MPC use only
//                10   1024  Unused or internal MPC use only
//                11   2048  Object is NEO
//                12   4096  Object is 1-km (or larger) NEO
//                13   8192  1-opposition object seen at
//                           earlier opposition
//                14  16384  Critical list numbered object
//                15  32768  Object is PHA
//
//                    Note that the orbit classification is
//                      based on cuts in osculating element
//                      space and is not 100% reliable.
//
//                    Note also that certain of the flags
//                      are for internal MPC use and are
//                      not documented.
//
		//  167 - 194  a      Readable designation
		ExtractTextField(lineBuff,	(167-1),	28,	argString, true);
//		if (argString[0] == '(')
//		{
//		char	*namePtr;
//			namePtr	=	strchr(argString, 0x20);
//			if (namePtr != NULL)
//			{
//				namePtr++;
//				strcpy(asteroidData->AsteroidName, namePtr);
//			}
//			else
//			{
//				strcpy(asteroidData->AsteroidName, argString);
//			}
//		}
//		else
		{
			strcpy(asteroidData->AsteroidName, argString);
		}
		StripTrailingSpaces(asteroidData->AsteroidName);
		strcpy(asteroidData->StarData.longName, asteroidData->AsteroidName);

//  195 - 202  i8     Date of last observation included in


	}
//	if (asteroidData->StarData.id == 7482)
//	{
//		asteroidData->AbsoluteMagnitude			=	2;
//		asteroidData->StarData.realMagnitude	=	2;
//		CONSOLE_DEBUG_W_STR("AsteroidName", asteroidData->AsteroidName);
//		DumpAsteroidData(asteroidData);
//		CONSOLE_ABORT(__FUNCTION__);
//	}

	return(validData);
}

//**************************************************************************
//*	returns the pointer to the new array
//**************************************************************************
static TYPE_Asteroid	*ReadAsteroidDataMPC(const char *filePath, long *asteroidCount)
{
FILE				*filePointer;
char				lineBuff[2048];
char				currentName[128];
char				myFilePath[128];
int					lineLength;
int					linesRead;
bool				validData;
int					maxLineLength;
int					asteroidIdx;
TYPE_Asteroid		asteroidData;
TYPE_Asteroid		*asteroidArray;
double				targetJulian;

	CONSOLE_DEBUG(__FUNCTION__);

	asteroidArray	=	NULL;
	if (filePath != NULL)
	{
		strcpy(myFilePath, filePath);
	}
	else
	{
		strcpy(myFilePath, "asteroids");
		strcat(myFilePath, "/");
		strcat(myFilePath, "MPCORB.DAT");
	}
	asteroidIdx		=	0;
	linesRead		=	0;
	maxLineLength	=	0;
	filePointer		=	fopen(myFilePath, "r");
	if (filePointer != NULL)
	{
//		CONSOLE_DEBUG("File Open");

		asteroidArray	=	(TYPE_Asteroid *)calloc(kMaxAsteroids, sizeof(TYPE_Asteroid));
		if (asteroidArray != NULL)
		{

			strcpy(currentName, "");
			targetJulian	=	Julian_GetCurrentDate();

			linesRead		=	0;

			//------------------------------------------
			//*	the MPC data file has a a text header of about 50 lines
			//*	the last line of the header is all "----"
			while (fgets(lineBuff, 2000, filePointer) && (linesRead < 60))
			{
				linesRead++;
				if (lineBuff[0] == '-')
				{
					break;
				}
			}
//			CONSOLE_DEBUG_W_NUM("linesRead:", linesRead);
			while (fgets(lineBuff, 2000, filePointer) && (linesRead < kMaxAsteroids))
			{
				linesRead++;
				lineLength	=	strlen(lineBuff);
				if (lineLength > maxLineLength)
				{
					maxLineLength	=	lineLength;
				}
				//*	get rid of trailing CR/LF and spaces
				while ((lineBuff[lineLength - 1] <= 0x20) && (lineLength > 1))
				{
					lineBuff[lineLength - 1]	=	0;
					lineLength	=	strlen(lineBuff);
				}
				if (lineLength > 100)
				{
					memset(&asteroidData, 0, sizeof(TYPE_Asteroid));
					validData	=	ParseOneLineMPCasteroidData(lineBuff, &asteroidData);

					if (validData)
					{
						//*	calculate the asteroid RA/DEC stuff
						ComputeAsteroidData_Static(&asteroidData);
						ComputeAsteroidData_Dynamic(&asteroidData, targetJulian, false, 0.0, 0.0, 0.0);

					#ifdef _INCLUDE_ASTEROID_MAIN_
						DumpAsteroidData(&asteroidData);
						DumpAsteroidCalcs(&asteroidData);
					#endif
						if (asteroidIdx < kMaxAsteroids)
						{
							asteroidArray[asteroidIdx]	=	asteroidData;
							asteroidIdx++;
						}
					}
					else
					{

					}
				}
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open file:", filePath);
	}
	CONSOLE_DEBUG_W_NUM("linesRead:", linesRead);
	CONSOLE_DEBUG_W_NUM("maxLineLength:", maxLineLength);

	*asteroidCount	=	asteroidIdx;
	return(asteroidArray);
}

//*****************************************************************************
//*	we have 2 different files that we support
//*		Lowell
//*		Minor Planets Center
//*
//*	The name of that database used is returned
//*****************************************************************************
TYPE_Asteroid	*ReadAsteroidData(	const char	*filePath,
									long		*asteroidCount,
									char		*databaseUsed)
{
TYPE_Asteroid	*asteroidData;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(databaseUsed, "None");
	asteroidData	=	NULL;
	*asteroidCount	=	0;
	//*	first lets try the Minor Planet Center data
	asteroidData	=	ReadAsteroidDataMPC(NULL, asteroidCount);
	if (asteroidData != NULL)
	{
		strcpy(databaseUsed, "MPC");
	}
	else
	{
		//*	that didnt work, lets try Lowell
		asteroidData	=	ReadAsteroidDataAstorb(NULL, asteroidCount);
		if (asteroidData != NULL)
		{
			strcpy(databaseUsed, "Lowell");
		}
	}

	CONSOLE_DEBUG_W_LONG("asteroidCount\t=", *asteroidCount);

	CONSOLE_DEBUG_W_LONG("asteroidCount\t=", *asteroidCount);

	return(asteroidData);
}

#endif // _ENABLE_ASTERIODS_

#ifdef _INCLUDE_ASTEROID_MAIN_

//*****************************************************************************
void	RunExample7_2(void)
{
TYPE_Asteroid	asteroidData;
double			targetJulian;

	CONSOLE_DEBUG("-----------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);

	gEnableDebug	=	true;
	targetJulian	=	Julian_CalcFromDate(10, 4, 1977);

	memset(&asteroidData, 0, sizeof(TYPE_Asteroid));
	asteroidData.StarData;

	//*	values from astorb.dat
	asteroidData.AsteroidNumber		=	1;
	strcpy(asteroidData.AsteroidName, "Gehreis 3");
	strcpy(asteroidData.EpochOfOsculation, "19770423");
	asteroidData.AbsoluteMagnitude	=	1.0;
//	asteroidData.Slope;
	asteroidData.MeanAnomaly		=	0.0;
	asteroidData.ArgOfPerihelion	=	231.4871;
	asteroidData.Longitude			=	242.5518;
	asteroidData.Inclination		=	1.1012;;
	asteroidData.Eccentricity		=	0.15186;
	asteroidData.SemimajorAxis		=	4.037397;
//	asteroidData.AbsValueCEU;		//*	arc seconds		(CEU = current ephemeris uncertainty)
//	asteroidData.RateOfChangeCEU;	//*	arcsec/day
//	asteroidData.DateOfCEU[16];		//*	yyyymmdd (0 hr UT).

double	sun_rtAscen		=	RADIANS(15 * DEG_MIN_SEC(12, 39, 10.4));
double	degrees			=	DEG_MIN_SEC(4, 13, 2.2);
double	sun_decl		=	RADIANS(-degrees);
double	sun_dist		=	0.983313;

	ComputeAsteroidData_Static(&asteroidData);
	asteroidData.epochJulian		=	Julian_ConvertFromMJD(43256.2676);
	asteroidData.n0_dailyMotion_deg	=	0.1214932;

	PrintAsteroidEphemeris(	&asteroidData,
					targetJulian,		//*	julian Date
					sun_rtAscen,		//*	radians
					sun_decl,			//*	radians
					sun_dist);
}

//*****************************************************************************
int main(int argc, char *argv[])
{
char			filePath[64];
long			asteroidCount;
TYPE_Asteroid	*asteroidData;
double			targetJulianDate;	//*	julian Date
double			sun_rtAscen;		//*	radians
double			sun_decl;			//*	radians
double			sun_dist;
double			degrees;
double			hours;
double			ecliptic_angle;

	strcpy(filePath, "asteroids");
	strcat(filePath, "/");
	strcat(filePath, "astorb.dat");
//	asteroidCount	=	0;
//	asteroidData	=	ReadAstorbDataFile(filePath, &asteroidCount);
//	CONSOLE_DEBUG_W_LONG("asteroidCount\t=", asteroidCount);

	asteroidData	=	ReadAsteroidDataMPC(NULL, &asteroidCount);
	CONSOLE_DEBUG_W_LONG("asteroidCount\t=", asteroidCount);


	targetJulianDate	=	Julian_GetCurrentDate();
	hours				=	DEG_MIN_SEC(18, 41, 23.46);
	sun_rtAscen			=	RADIANS(hours * 15);
	degrees				=	DEG_MIN_SEC(23, 05, 46.99);
	sun_decl			=	RADIANS(-degrees);
	sun_dist			=	0.983325;

//	if (asteroidData != NULL)
//	{
//		PrintAsteroidEphemeris(	asteroidData,
//						targetJulianDate,	//*	julian Date
//						sun_rtAscen,		//*	radians
//						sun_decl,			//*	radians
//						sun_dist);
//	}
//
//	RunExample7_2();


	ecliptic_angle			=	DEG_MIN_SEC(23, 26, 21.45);
	CONSOLE_DEBUG_W_DBL("ecliptic_angle_deg\t=", ecliptic_angle);
	CONSOLE_DEBUG_W_DBL("ecliptic_angle_RAD\t=", RADIANS(ecliptic_angle));

	return(0);
}

#endif

//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jun 10,	2024	<MLS> Created KeplerEquations.cpp
//*	Jun 10,	2024	<MLS> Found keps for Pluto
//*	Jun 12,	2024	<MLS> Added ReadAsteroidElementsData()
//*	Jun 14,	2024	<MLS> Added ReadTransNeptunianData()
//*****************************************************************************
//*	this is based of of code found at
//*		https://github.com/mayakraft/Kepler
//*		https://ssd.jpl.nasa.gov/sb/elem_tables.html
//*****************************************************************************
#include	<math.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"KeplerEquations.h"

int	ReadAsteroidElementsData(void);
int	ReadTransNeptunianData(void);

TYPE_KeplerData	*gAsteriodData	=	NULL;
int				gAsteroidCount	=	0;

TYPE_KeplerData	*gTransNeptunianData	=	NULL;
int				gTransNeptunianCount	=	0;


TYPE_KeplerData	gPlanetKepData[20];

//*****************************************************************************
//https://ssd.jpl.nasa.gov/planets/approx_pos.html
//
//               a              e               I                L            long.peri.      long.node.
//           au, au/Cy     rad, rad/Cy     deg, deg/Cy      deg, deg/Cy      deg, deg/Cy     deg, deg/Cy
//-----------------------------------------------------------------------------------------------------------
//Mercury   0.38709927      0.20563593      7.00497902      252.25032350     77.45779628     48.33076593
//          0.00000037      0.00001906     -0.00594749   149472.67411175      0.16047689     -0.12534081
//Venus     0.72333566      0.00677672      3.39467605      181.97909950    131.60246718     76.67984255
//          0.00000390     -0.00004107     -0.00078890    58517.81538729      0.00268329     -0.27769418
//EM Bary   1.00000261      0.01671123     -0.00001531      100.46457166    102.93768193      0.0
//          0.00000562     -0.00004392     -0.01294668    35999.37244981      0.32327364      0.0
//Mars      1.52371034      0.09339410      1.84969142       -4.55343205    -23.94362959     49.55953891
//          0.00001847      0.00007882     -0.00813131    19140.30268499      0.44441088     -0.29257343
//Jupiter   5.20288700      0.04838624      1.30439695       34.39644051     14.72847983    100.47390909
//         -0.00011607     -0.00013253     -0.00183714     3034.74612775      0.21252668      0.20469106
//Saturn    9.53667594      0.05386179      2.48599187       49.95424423     92.59887831    113.66242448
//         -0.00125060     -0.00050991      0.00193609     1222.49362201     -0.41897216     -0.28867794
//Uranus   19.18916464      0.04725744      0.77263783      313.23810451    170.95427630     74.01692503
//         -0.00196176     -0.00004397     -0.00242939      428.48202785      0.40805281      0.04240589
//Neptune  30.06992276      0.00859048      1.77004347      -55.12002969     44.96476227    131.78422574
//          0.00026291      0.00005105      0.00035372      218.45945325     -0.32241464     -0.00508664
//------------------------------------------------------------------------------------------------------
//EM Bary = Earth/Moon Barycenter

//**************************************************************************************
static void	SetKeplerElements(		TYPE_KeplerData *kepData,
									const char	*name,
									double		a,				//*	au
									double		e,				//*	rad
									double		I,				//*	deg
									double		L,				//*	deg
									double		omega_bar,		//*	deg
									double		Omega,			//*	deg

									double		a_dot,			//*	au / century
									double		e_dot,			//*	rad / century
									double		I_dot,			//*	deg / century
									double		L_dot,			//*	deg / century
									double		omega_bar_dot,	//*	deg / century
									double		Omega_dot)		//*	deg / century
{
	strcpy(kepData->name, name);
	kepData->elem_a				=	a;
	kepData->elem_e				=	e;
	kepData->elem_I				=	I;
	kepData->elem_L				=	L;
	kepData->elem_omega_bar		=	omega_bar;
	kepData->elem_Omeaga		=	Omega;
	kepData->elem_a_dot			=	a_dot;
	kepData->elem_e_dot			=	e_dot;
	kepData->elem_I_dot			=	I_dot;
	kepData->elem_L_dot			=	L_dot;
	kepData->elem_omega_bar_dot	=	omega_bar_dot;
	kepData->elem_Omeaga_dot	=	Omega_dot;

}

//**************************************************************************************
void	Kepler_InitPlanets(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	SetKeplerElements(&gPlanetKepData[kPlanet_Mercury],
			"Mercury",
			0.38709927,		0.20563593,		7.00497902,		252.25032350,		77.45779628,	48.33076593,
			0.00000037,		0.00001906,		-0.00594749,	149472.67411175,	0.16047689,		-0.12534081);

	SetKeplerElements(&gPlanetKepData[kPlanet_Venus],
			"Venus",
			0.72333566,		0.00677672,		3.39467605,		181.97909950,		131.60246718,	76.67984255,
			0.00000390,		-0.00004107,	-0.00078890,	58517.81538729,		0.00268329,		-0.27769418);
	SetKeplerElements(&gPlanetKepData[kPlanet_Earth],
			"Earth",
			1.00000261,		0.01671123,		-0.00001531,	100.46457166,		102.93768193,	0.0,
			0.00000562,		-0.00004392,	-0.01294668,	35999.37244981,		0.32327364,		0.0);
	SetKeplerElements(&gPlanetKepData[kPlanet_Mars],
			"Mars",
			1.52371034,		0.09339410,		1.84969142,		-4.55343205,		-23.94362959,	49.55953891,
			0.00001847,		0.00007882,		-0.00813131,	19140.30268499,		0.44441088,		-0.29257343);
	SetKeplerElements(&gPlanetKepData[kPlanet_Jupiter],
			"Jupiter",
			5.20288700,		0.04838624,		1.30439695,		34.39644051,		14.72847983,	100.47390909,
			-0.00011607,	-0.00013253,	-0.00183714,	3034.74612775,		0.21252668,		0.20469106);
	SetKeplerElements(&gPlanetKepData[kPlanet_Saturn],
			"Saturn",
			9.53667594,		0.05386179,		2.48599187,		49.95424423,		92.59887831,	113.66242448,
			-0.00125060,	-0.00050991,	0.00193609,		1222.49362201,		-0.41897216,	-0.28867794);
	SetKeplerElements(&gPlanetKepData[kPlanet_Uranus],
			"Uranus",
			19.18916464,	0.04725744,		0.77263783,		313.23810451,		170.95427630,	74.01692503,
			-0.00196176,	-0.00004397,	-0.00242939,	428.48202785,		0.40805281,		0.04240589);
	SetKeplerElements(&gPlanetKepData[kPlanet_Neptune],
			"Neptune",
			30.06992276,	0.00859048,		1.77004347,		-55.12002969,		44.96476227,	131.78422574,
			0.00026291,		0.00005105,		0.00035372,		218.45945325,		-0.32241464,	-0.00508664);


	//	https://nssdc.gsfc.nasa.gov/planetary/factsheet/plutofact.html
	//	Pluto Mean Orbital Elements (J2000)
	//
	//	Semimajor axis (AU)                 39.48168677
	//	Orbital eccentricity                 0.24880766
	//	Orbital inclination (deg)           17.14175
	//	Longitude of ascending node (deg)  110.30347
	//	Longitude of perihelion (deg)      224.06676
	//	Mean longitude (deg)               238.92881
//
//	SetKeplerElements(&gPlanetKepData[kPlanet_Pluto],
//			"Pluto",
//			//a				e			I			L (mean longitude)	long.peri/OmBar	long.node/OMEGA.
//			39.48168677,	0.24880766,	17.14175,	238.92881,			224.06676,		110.30347,
//			0.0,			0.0,		0.0,		0.0,				0.0,			0.0);

//	SetKeplerElements(&gPlanetKepData[kPlanet_Haumea],
//			"Haumea",
//			//a				e			I			L (mean longitude)	long.peri/OmBar	long.node/OMEGA.
//			43.1,			0.19,		28.0,		0.0,				0.0,			0.0,
//			0.0,			0.0,		0.0,		0.0,				0.0,			0.0);



	//*	set the orbit periods
	//*	https://space-facts.com/orbital-periods-planets/
	//	Mercury: 87.97 days (0.2 years)
	//	Venus : 224.70 days (0.6 years)
	//	Earth: 365.26 days(1 year)
	//	Mars: 686.98 days(1.9 years)
	//	Jupiter: 4,332.82 days (11.9 years)
	//	Saturn: 10,755.70 days (29.5 years)
	//	Uranus: 30,687.15 days (84 years)
	//	Neptune: 60,190.03 days (164.8 years)
	gPlanetKepData[kPlanet_Mercury].oribit_days	=	87.97;		//(0.2 years)
	gPlanetKepData[kPlanet_Venus].oribit_days	=	224.70;		//days (0.6 years)
	gPlanetKepData[kPlanet_Earth].oribit_days	=	365.26;		//days(1 year)
	gPlanetKepData[kPlanet_Mars].oribit_days	=	686.98;		//days(1.9 years)
	gPlanetKepData[kPlanet_Jupiter].oribit_days	=	4332.82;	//days (11.9 years)
	gPlanetKepData[kPlanet_Saturn].oribit_days	=	10755.70;	//days (29.5 years)
	gPlanetKepData[kPlanet_Uranus].oribit_days	=	30687.15;	//days (84 years)
	gPlanetKepData[kPlanet_Neptune].oribit_days	=	60190.03;	//days (164.8 years)

	//----------------------------------------------
	//*	testing
int		iii;
double	date	=	0.138767; // mid November 2013
double x, y, z;

	printf("mid November 2013\n");
	for (iii = 0; iii < kPlanet_Last; iii++)
	{
		Kepler_CalculatePlanetLocation(&gPlanetKepData[iii], date, &x, &y, &z);
		printf("%s\t%d°\t(x:%7.3f  y:%7.3f  z:%7.3f)\n",
				gPlanetKepData[iii].name, (int)(atan2(y,x)/M_PI*180), x, y, z );
	}

	gAsteroidCount			=	ReadAsteroidElementsData();
	gTransNeptunianCount	=	ReadTransNeptunianData();
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
double KeplersEquation(double E_degrees, double M, double e_degrees)
{
double	deltaM;
double	deltaE;

	deltaM	=	M - ( E_degrees - RADIANS(e_degrees) * sin(RADIANS(E_degrees)));
	deltaE	=	deltaM / (1.0 - e_degrees * cos(RADIANS(E_degrees)));

	return(E_degrees + deltaE);
}

//*****************************************************************************
void Kepler_CalculatePlanetLocation(TYPE_KeplerData	*objectData,
									double			time,
									double			*planet_x,
									double			*planet_y,
									double			*planet_z)
{
int		iii;
double	a;
double	e;
double	I;
double	L;
double	omega_bar;
double	OMEGA;
double	omega;
double	M;
double	xx0;
double	yy0;
double	E;

//	CONSOLE_DEBUG(__FUNCTION__);

	//-------------------------------------------------------
	// step 1
	// compute the value of each of that planet's six elements
//	double a = _elements[6*planet+0] + _rates[6*planet+0]*time;	// (au) semi_major_axis
//	double e = _elements[6*planet+1] + _rates[6*planet+1]*time;	//  ( ) eccentricity
//	double I = _elements[6*planet+2] + _rates[6*planet+2]*time;	//  (°) inclination
//	double L = _elements[6*planet+3] + _rates[6*planet+3]*time;	//  (°) mean_longitude
//	double omega_bar = _elements[6*planet+4] + _rates[6*planet+4]*time;	//  (°) longitude_of_periapsis
//	double OMEGA = _elements[6*planet+5] + _rates[6*planet+5]*time;	//  (°) longitude_of_the_ascending_node

	a			=	objectData->elem_a + (objectData->elem_a_dot * time);	// (au) semi_major_axis
	e			=	objectData->elem_e + (objectData->elem_e_dot * time);	//  ( ) eccentricity
	I			=	objectData->elem_I + (objectData->elem_I_dot * time);	//  (°) inclination
	L			=	objectData->elem_L + (objectData->elem_L_dot * time);	//  (°) mean_longitude
	omega_bar	=	objectData->elem_omega_bar + (objectData->elem_omega_bar_dot * time);	//  (°) longitude_of_periapsis
	OMEGA		=	objectData->elem_Omeaga + (objectData->elem_Omeaga_dot * time);	//  (°) longitude_of_the_ascending_node

	objectData->a	=	a;
	objectData->e	=	e;
	objectData->I	=	I;
	objectData->L	=	L;
	//-------------------------------------------------------
	// step 2
	// compute the argument of perihelion, omega, and the mean anomaly, M
	omega	=	omega_bar - OMEGA;
	M		=	L - omega_bar;

	//-------------------------------------------------------
	// step 3a
	// modulus the mean anomaly so that -180° ≤ M ≤ +180°
	while (M > 180)
	{
		M	-=	360.0;  // in degrees
	}
	//-------------------------------------------------------
	// step 3b
	// obtain the eccentric anomaly, E, from the solution of Kepler's equation
	//   M = E - e*sinE
	//   where e* = 180/πe = 57.29578e
	E	=	M + RADIANS(e) * sin(RADIANS(M));  // E0
	for (iii = 0; iii < 5; iii++)
	{
		// iterate for precision, 10^(-6) degrees is sufficient
		E	=	KeplersEquation(E, M, e);
	}
	objectData->E			=	E;
	//-------------------------------------------------------
	// step 4
	// compute the planet's heliocentric coordinates in its orbital plane, r', with the x'-axis aligned from the focus to the perihelion
	omega	=	RADIANS(omega);
	E		=	RADIANS(E);
	I		=	RADIANS(I);
	OMEGA	=	RADIANS(OMEGA);
	xx0		=	a * (cos(E) - e);
	yy0		=	a * sqrt((1.0 - e * e)) * sin(E);
	//-------------------------------------------------------
	// step 5
	// compute the coordinates in the J2000 ecliptic plane, with the x-axis aligned toward the equinox:

	objectData->planet_x	=	( cos(omega)*cos(OMEGA) - sin(omega)*sin(OMEGA)*cos(I) )*xx0 + ( -sin(omega)*cos(OMEGA) - cos(omega)*sin(OMEGA)*cos(I) )*yy0;
	objectData->planet_y	=	( cos(omega)*sin(OMEGA) + sin(omega)*cos(OMEGA)*cos(I) )*xx0 + ( -sin(omega)*sin(OMEGA) + cos(omega)*cos(OMEGA)*cos(I) )*yy0;
	objectData->planet_z	=	(						  sin(omega)*sin(I)			   )*xx0 + (			 cos(omega)*sin(I)			 )*yy0;

	*planet_x	=	objectData->planet_x;
	*planet_y	=	objectData->planet_y;
	*planet_z	=	objectData->planet_z;
}



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

//*****************************************************************************
static void	DumpAsteroidElements(TYPE_KeplerData *asteroidData)
{
	printf("//>%6d",	asteroidData->idNumber);
	printf(" %-18s",	asteroidData->name);
	printf("%5d",		asteroidData->epoch);
	printf("%11.7f",	asteroidData->elem_a);
	printf("%11.8f",	asteroidData->elem_e);
	printf("%10.5f",	asteroidData->elem_I);
	printf("%10.5f",	asteroidData->elem_omega_bar);
	printf("%10.5f",	asteroidData->elem_Omeaga);
	printf("%12.7f",	0.0);
	printf("%6.2f",		asteroidData->magnitude);

	printf("%8.4f ",	asteroidData->elem_q);
	printf("%8.4f ",	asteroidData->elem_Q);

	printf("\r\n");

}

// Num   Name              Epoch      a          e        i         w        Node        M         H     G   Ref
//*     1 Ceres             60400  2.7670775 0.07902732  10.58759  73.36091  80.25353 102.9507232  3.34  0.12 JPL 48
//>     1 Ceres             60400  2.7670775 0.07902732  10.58759  73.36091  80.25353   0.0000000  3.34


//*****************************************************************************
//*	https://ssd.jpl.nasa.gov/sb/elem_tables.html
//*****************************************************************************

//*****************************************************************************
// Num   Name              Epoch      a          e        i         w        Node        M         H     G   Ref
//------ ----------------- ----- ---------- ---------- --------- --------- --------- ----------- ----- ----- ----------
//     1 Ceres             60400  2.7670775 0.07902732  10.58759  73.36091  80.25353 102.9507232  3.34  0.12 JPL 48
//     2 Pallas            60400  2.7704235 0.23033990  34.92284 310.88724 172.91472  83.3272450  4.12  0.11 JPL 64
//     3 Juno              60400  2.6696851 0.25618681  12.98975 247.77598 169.84066  82.1865266  5.18  0.32 JPL 138
//123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
//0        1         2         3         4         5         6         7         8         9         0         1
//*
//*		w		=	lower case omega
//*		Node	=	UPPER case omega
//*		M		=
//*****************************************************************************
//*	https://phys.libretexts.org/Bookshelves/Astronomy__Cosmology/Celestial_Mechanics_(Tatum)/10%3A_Computation_of_an_Ephemeris/10.07%3A_Calculating_the_Position_of_a_Comet_or_Asteroid
//*
//	For example, according to the Minor Planet Center, the osculating elements for the minor planet
//	(1) Ceres for the epoch of osculation t0=2002 May 6.0 TT
//
//	are as follows:
//
//	a	=	2.7664122 AU	Ω	=	80.48632
//	e	=	0.0791158		ω	=	73.98440
//	i	=	10.58347		M0	=	189.27500
//*****************************************************************************
bool	ParseOneLineElementsData(char *lineBuff, TYPE_KeplerData *asteroidData)
{
bool	validData;
char	textfield[32];

	validData	=	false;
	if (isdigit(lineBuff[5]))
	{
		validData	=	true;

		//*	Num
		ExtractTextField(lineBuff,	(1-1),	6,	textfield, true);
		asteroidData->idNumber	=	atoi(textfield);

		//*	Name
		ExtractTextField(lineBuff,	(8-1),	18,	textfield, true);
		strcpy(asteroidData->name,	textfield);

		//*	Epoch
		ExtractTextField(lineBuff,	(26-1),	5,	textfield, true);
		asteroidData->epoch	=	atoi(textfield);

		//*	a
		ExtractTextField(lineBuff,	(33-1),	9,	textfield, true);
		asteroidData->elem_a	=	atof(textfield);
		//*	e
		ExtractTextField(lineBuff,	(43-1),	10,	textfield, true);
		asteroidData->elem_e	=	atof(textfield);
		//*	i
		ExtractTextField(lineBuff,	(54-1),	9,	textfield, true);
		asteroidData->elem_I	=	atof(textfield);

		//*	w
		ExtractTextField(lineBuff,	(64-1),	9,	textfield, true);
//		asteroidData->elem_omega_bar	=	atof(textfield);
		asteroidData->elem_Omeaga	=	atof(textfield);

		//*	Node
		ExtractTextField(lineBuff,	(74-1),	9,	textfield, true);
//		asteroidData->elem_Omeaga	=	atof(textfield);
		asteroidData->elem_omega_bar	=	atof(textfield);

		//*	M
		ExtractTextField(lineBuff,	(84-1),	11,	textfield, true);
		asteroidData->elem_L	=	atof(textfield);

		//*	H
		ExtractTextField(lineBuff,	(96-1),	5,	textfield, true);
		asteroidData->magnitude	=	atof(textfield);
		//*	G
		//*	Ref
	}
	return(validData);
}


#define	kAsteroidElemntsFilename	"ELEMENTS.NUMBR"
#define	kMaxAsterioidCnt			700000

//*****************************************************************************
int	ReadAsteroidElementsData(void)
{
FILE				*filePointer;
char				lineBuff[2048];
char				myFilePath[128];
int					lineLength;
int					linesRead;
bool				validData;
int					maxLineLength;
int					asteroidIdx;
TYPE_KeplerData		asteroidData;

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(myFilePath, kAsteroidElemntsFilename);
	asteroidIdx		=	0;
	linesRead		=	0;
	maxLineLength	=	0;
	filePointer		=	fopen(myFilePath, "r");
	if (filePointer != NULL)
	{
		gAsteriodData	=	(TYPE_KeplerData *)calloc(kMaxAsterioidCnt, sizeof(TYPE_KeplerData));
		if (gAsteriodData != NULL)
		{
			while (fgets(lineBuff, 2000, filePointer) && (linesRead < kMaxAsterioidCnt))
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
					memset(&asteroidData, 0, sizeof(TYPE_KeplerData));
					validData	=	ParseOneLineElementsData(lineBuff, &asteroidData);

					if (validData)
					{
					double	date	=	0.138767; // mid November 2013
					double x, y, z;
						Kepler_CalculatePlanetLocation(&asteroidData, date, &x, &y, &z);

						if (asteroidIdx < 5)
						{
							DumpAsteroidElements(&asteroidData);
						}

						if (asteroidIdx < kMaxAsterioidCnt)
						{
							gAsteriodData[asteroidIdx]	=	asteroidData;
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
		CONSOLE_DEBUG_W_STR("Failed to open file:", myFilePath);
	}
	CONSOLE_DEBUG_W_NUM("linesRead  \t=", linesRead);
	CONSOLE_DEBUG_W_NUM("asteroidIdx\t=", asteroidIdx);

//	CONSOLE_ABORT(__FUNCTION__);
	return(asteroidIdx);
}

#define	kTransNeptunianFilename	"trans-Neptunian.txt"
#define	kMaxTransNeptunianCount	5500

int	gNameCount			=	0;
//*****************************************************************************
//#  number/name                       provisional    dynamical          a        e        q           Q         i    diameter albedo  assum.   B-R  taxon  density  known add'l components   discovery
//#                                    designation      class           (AU)              (AU)        (AU)     (deg)    (km)           albedo   mag  type   (g/cm^3) (diameter in km)         (year mon)
//#-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//#
//#objects which should be designated as planets, currently designated as dwarf planets or overdue to be:
//#
// (134340) Pluto                                    plutino           39.453    0.247   29.721      49.185   17.16   2375    0.720           1.34  BBb    1.86  MULT: 1211, 10, 39, 12, 42  1930  1
//  (87269)                           2000 OO67      Centaur          667.838    0.969   20.831    1314.845   20.06     83?       ?  (0.058)  1.69  RR                                       2000  7
// (471143) Dziewanna                 2010 EK139     res 2:7           68.731    0.528   32.472     104.990   29.48    470    0.250                                                          2010  3
//123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
//0        1         2         3         4         5         6         7         8         9         0         1

//*****************************************************************************
bool	ParseOneLineTransNeptuneData(char *lineBuff, TYPE_KeplerData *objectData)
{
bool	validData;
char	textfield[128];
int		nameLen;

	validData	=	false;
	if (lineBuff[0] != '#')
	{
		validData	=	true;

		//*	number
		ExtractTextField(lineBuff,	(1-1),	32,	textfield, true);
		objectData->idNumber	=	atoi(textfield);


		//*	name
		ExtractTextField(lineBuff,	(11-1),	18,	textfield, true);
		nameLen	=	strlen(textfield);
		if (nameLen > 0)
		{
			strcpy(objectData->name,	textfield);
			gNameCount++;
		}

//		//*	Epoch
//		ExtractTextField(lineBuff,	(26-1),	5,	textfield, true);
//		objectData->epoch	=	atoi(textfield);

		//*	a
		ExtractTextField(lineBuff,	(69-1),	6,	textfield, true);
		objectData->elem_a	=	atof(textfield);
		//*	e
		ExtractTextField(lineBuff,	(80-1),	5,	textfield, true);
		objectData->elem_e	=	atof(textfield);
		//*	i
		ExtractTextField(lineBuff,	(109-1),	5,	textfield, true);
		objectData->elem_I	=	atof(textfield);

//		//*	w
//		ExtractTextField(lineBuff,	(64-1),	9,	textfield, true);
////		objectData->elem_omega_bar	=	atof(textfield);
//		objectData->elem_Omeaga	=	atof(textfield);
//
//		//*	Node
//		ExtractTextField(lineBuff,	(74-1),	9,	textfield, true);
////		objectData->elem_Omeaga	=	atof(textfield);
//		objectData->elem_omega_bar	=	atof(textfield);
//
//		//*	M
//		ExtractTextField(lineBuff,	(84-1),	11,	textfield, true);
//		objectData->elem_L	=	atof(textfield);
//
//		//*	H
//		ExtractTextField(lineBuff,	(96-1),	5,	textfield, true);
//		objectData->magnitude	=	atof(textfield);
//		//*	G
//		//*	Ref

		//*	q
		ExtractTextField(lineBuff,	(87-1),	7,	textfield, true);
		objectData->elem_q	=	atof(textfield);

		//*	Q
		ExtractTextField(lineBuff,	(99-1),	8,	textfield, true);
		objectData->elem_Q	=	atof(textfield);
	}
	return(validData);
}

//*****************************************************************************
int	ReadTransNeptunianData(void)
{
FILE				*filePointer;
char				lineBuff[2048];
char				myFilePath[128];
int					lineLength;
int					linesRead;
bool				validData;
int					maxLineLength;
int					objectIdx;
TYPE_KeplerData		objectData;

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(myFilePath, kTransNeptunianFilename);
	objectIdx		=	0;
	linesRead		=	0;
	maxLineLength	=	0;
	filePointer		=	fopen(myFilePath, "r");
	if (filePointer != NULL)
	{
		gTransNeptunianData	=	(TYPE_KeplerData *)calloc(kMaxTransNeptunianCount, sizeof(TYPE_KeplerData));
		if (gTransNeptunianData != NULL)
		{
			while (fgets(lineBuff, 2000, filePointer) &&
					(linesRead < kMaxAsterioidCnt) &&
					(objectIdx < 5))
			{
				linesRead++;
				lineLength	=	strlen(lineBuff);
				if (lineLength > maxLineLength)
				{
					maxLineLength	=	lineLength;
				}
				//*	get rid of trailing CR/LF and spaces
				while	((lineBuff[lineLength - 1] <= 0x20) && (lineLength > 1))
				{
					lineBuff[lineLength - 1]	=	0;
					lineLength	=	strlen(lineBuff);
				}
				if (lineLength > 100)
				{
					memset(&objectData, 0, sizeof(TYPE_KeplerData));
					validData	=	ParseOneLineTransNeptuneData(lineBuff, &objectData);

					if ((validData && (objectIdx < 5)) ||
						(validData && (objectData.elem_a < 200.0) && (strlen(objectData.name) > 0))
						)
					{
					double	date	=	0.138767; // mid November 2013
					double x, y, z;
						Kepler_CalculatePlanetLocation(&objectData, date, &x, &y, &z);

						if (objectIdx < 5)
						{
							DumpAsteroidElements(&objectData);
						}

						if (objectIdx < kMaxTransNeptunianCount)
						{
							gTransNeptunianData[objectIdx]	=	objectData;
							objectIdx++;
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
		CONSOLE_DEBUG_W_STR("Failed to open file:", myFilePath);
	}
	CONSOLE_DEBUG_W_NUM("linesRead \t=", linesRead);
	CONSOLE_DEBUG_W_NUM("objectIdx \t=", objectIdx);
	CONSOLE_DEBUG_W_NUM("gNameCount\t=", gNameCount);

//	CONSOLE_ABORT(__FUNCTION__);
	return(objectIdx);
}


//**************************************************************************


//#include	"AsteroidData.h"



#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif


//**************************************************************************
typedef struct
{
	//*	to make it easier to draw
	TYPE_CelestData	StarData;

	//*	values from astorb.dat
	int		AsteroidNumber;
	char	AsteroidName[32];
	char	EpochOfOsculation[32];			//	Epoch of osculation
	double	AbsoluteMagnitude;
	double	Slope;
	double	MeanAnomaly;
	double	ArgOfPerihelion;
	double	Longitude;
	double	Inclination;
	double	Eccentricity;
	double	SemimajorAxis;
	double	AbsValueCEU;		//*	arc seconds		(CEU = current ephemeris uncertainty)
	double	RateOfChangeCEU;	//*	arcsec/day
	char	DateOfCEU[16];		//*	yyyymmdd (0 hr UT).
	//----------------------------------------------------
	//*	computed values
	double	Period;
	double	PeriodDays;
	double	n0_dailyMotion_deg;
	double	epochJulian;
	double	targetJulian;

//	double	epochMJD;			//*	Modified Julian Date
//	double	targetMJD;

	double	Px;
	double	Py;
	double	Pz;
	double	Qx;
	double	Qy;
	double	Qz;

	//*	for debugging
	double	sunXXX;
	double	sunYYY;
	double	sunZZZ;
	double	delta_AUfromEarth;

} TYPE_Asteroid;


#ifdef __cplusplus
	extern "C" {
#endif

TYPE_Asteroid	*ReadAsteroidData(	const char	*filePath,
									long		*asteroidCount,
									char		*databaseUsed);
void			UpdateAsteroidEphemeris(	TYPE_Asteroid	*asteroidData,
											const long		asteriodCnt,
											const double	targetJulianDate,	//*	julian Date
											const double	sun_rtAscen,		//*	radians
											const double	sun_decl,			//*	radians
											const double	sun_dist);			//*	AU

void	PrintAsteroidEphemeris(	TYPE_Asteroid *asteroidData,
								const double	targetJulianDate,	//*	julian Date
								const double	sun_rtAscen,		//*	radians
								const double	sun_decl,			//*	radians
								const double	sun_dist);			//*	AU

extern	TYPE_Asteroid		*gAsteroidPtr;
extern	long				gAsteroidCnt;
extern	char				gAsteroidDatbase[];

#ifdef __cplusplus
}
#endif

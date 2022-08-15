//*****************************************************************************
//*	this is the star structure format used by SkyTravel
//*	SkyTravel was originally written by Clif Ashcraft and Frank Covits
//*	for the Commodore 64
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Dec 31,	2020	<MLS> had to change char to int8_t to keep Raspberry-Pi g++ compiler happy
//*	Apr 17,	2021	<MLS> Increased short name length from 8 to 12
//*	Jul  9,	2021	<MLS> Finished converting all start data to globals
//*	Oct 23,	2021	<MLS> Added spectralClass to star structure (O,B,A,F,G,K,M)
//*	Aug  2,	2022	<MLS> Added kDataSrc_SAO
//*****************************************************************************
//#include	"SkyStruc.h"


#ifndef _SKY_STRUCTS_H_
#define	_SKY_STRUCTS_H_

#ifndef _STDINT_H
	#include	<stdint.h>
#endif // _STDINT_H


#if PRAGMA_ALIGN_SUPPORTED
	#pragma options align=mac68k
#elif __INTEL__
	#pragma options align=reset
#endif
void	ConvertRadiansToDegMinSec(	double	radianValue,
									short	*argDegrees,
									short	*argMinutes,
									double	*argSeconds);

//*****************************************************************************
//*	this is the on-disk format for Clif and Franks object database
//*	fortunately the FLOAT format is the same for PPC/68K as for intel.
//*	The bytes have to be swapped for big endian machines
//*****************************************************************************
typedef struct
{
	float			decl;
	float			ra;
	unsigned short	magn;
	unsigned short	id;
}	TYPE_CelestDataDisk;


#if PRAGMA_ALIGN_SUPPORTED
	#pragma options align=reset
#elif __INTEL__
	#pragma options align=reset
#endif

//*****************************************************************************
enum
{
	kDataSrc_Orginal	=	0,	//*	Frank and Clif's orginal data file
	kDataSrc_Planets,
	kDataSrc_Zodiac,
	kDataSrc_YaleBrightStar,
	kDataSrc_NGC2000,
	kDataSrc_NGC2000IC,
	kDataSrc_HubbleGSC,
	kDataSrc_Hipparcos,

	kDataSrc_Messier,
	kDataSrc_Draper,

	kDataSrc_HYG,

	kDataSrc_Special,
	kDataSrc_PolarAlignCenter,
	kDataSrc_AAVSOalert,

	kDataSrc_GAIA_SQL,
	kDataSrc_Asteroids,
	kDataSrc_OpenNGC,

	kDataSrc_SAO,

	kDataSrc_Unknown,

	kDataSrc_last
};

#define	kShortNameMax	12
#define	kLongNameMax	32


//*****************************************************************************
typedef struct
{
	long			id;
	unsigned short	magn;
	char			dataSrc;		//*	see enum table above
	char			spectralClass;	//*	O,B,A,F,G,K,M
	char			variability;	//*	Variability Flag: 0=not variable, 1=variable, 2=uncertain
	long			type;			//*	NGC = 3 char field
	short			curXX;			//*	current plot location on the screen
	short			curYY;			//*	this is used for finding the object the cursor is pointing to
	double			realMagnitude;
	double			ra;				//*	these values are in RADIANS
	double			decl;
	double			org_ra;
	double			org_decl;
//	double			properMotion;	//	https://en.wikipedia.org/wiki/Proper_motion
//	double			maxSizeArcMinutes;
	double			parallax;
	char			shortName[kShortNameMax];
	char			longName[kLongNameMax];
}	TYPE_CelestData;

//*****************************************************************************
typedef struct
{
	char			dataSrc;		//*	see enum table above
	long			id;
	char			name[64];
	char			miscInfo[64];

}	TYPE_ObjectInfo;


//*****************************************************************************
typedef struct
{
	char	 		shortName[6];
	char	 		name[32];
	short			indexIntoConstStarTable;
	short			starsInConstelation;
}	TYPE_Constelation;



//*****************************************************************************
typedef struct
{
	double	aside;
	double	bside;
	double	cside;
	double	alpha;
	double	beta;
	double	gamma;
}	TYPE_SpherTrig;


//*****************************************************************************
typedef struct
{
	double	latitude;
	double	longitude;
}	TYPE_LatLon;



//*****************************************************************************
typedef struct
{
	int				year;				//*	greenwich date
	int				day;
	int				month;
	int				hour;
	int				min;
	int				sec;

	int				local_year;
	int				local_day;
	int				local_month;
	int				local_hour;
	int				timezone;

	int				dyear;				//*	for display
	int				dday;
	int				dmonth;
	int				dhour;

	int				timeinc;

	//*	in the original code, these were individual bit fields, we dont need to be
	//*	that paranoid about memory any more
	unsigned char	calflag;			//*	0=auto, 1=julian, anything else=gregorian
	bool			precflag;			//*	=1 means force a precession
	bool			strflag;			//*	1= year 2000 data is present
	bool			starDataModified;
	bool			negflag;			//*	1= time decrements 0 = time increments
	bool			local_time_flag;	//*	1=use local time 0=use gmt
	bool			dstflag;			//*	1=use daylight savings

	double			fgmt;				//*	floating point form of gmt
	double			fSiderealTime;		//*	sidereal time
	double			fJulianDay;			//*	the floating point form of the julian day
	double			daysTillEpoch2000;	//*	days til epoch 2000
	double			cdte;				//*	(corrected) days til epoch 2000
	double			cent;				//*	centuries from epoch 1900
	double			ccent;				//*	(corrected) centuries from epoch 1900

	double			timeOfLastPrec;		//*	last jd time precession was done
	double			delprc;				//*	the delta jd which triggers precession

	//*	added by MLS Dec 21, 1999
	short			clockMode;				//*	see enum below
	short			screenUpdateInterval;	//*	seconds
	short			simulationClockFactor;

}	TYPE_Time;

//*****************************************************************************
enum
{
	kClockMode_None	=	0,
	kClockMode_RealTime,
	kClockMode_Simulate,

	kClockMode_Last
};

//*****************************************************************************
typedef struct
{
	int				xpos;
	int				ypos;
	unsigned int	idword;
} xy_struct;

//*****************************************************************************
typedef struct
{
	double		sun_elev;				//*	for sky color
	double		sun_az;

	double		mon_geo_elev;			//*	geo-centric coords
	double		mon_geo_az;

	double		mon_topo_ra;			//*	topographic coords
	double		mon_topo_decl;
	double		mon_topo_elev;
	double		mon_topo_az;

	double		mon_topo_dist;			//*	observer to moon distance

	double		smang;					//*	moon position angle of solar eclipse cusps
	double		smdist;					//*	actual angular distance between sun-moon
	double		smdmax;					//*	sun-moon angular distance for solar eclipse to happen

	double		earth_shadow_radius;	//*	in radians
	double		emdist;					//*	actual angular distance between earth shadow-moon
	double		emdmax;					//*	earth shadow-moon angular distance for lunar eclipse to happen

	double		emang;					//*	like position angle of lunar eclipse

	unsigned int	solar_ecl_flag:1;	//*	for solar eclipse
	unsigned int	lunar_ecl_flag:1;	//*	for lunar eclipse

}	sun_moon_struct;				//*	special data for moon and sun

//*	the structures for the series evaluator

//*	the number of terms for the series  evaluator

#define		MONN	7
#define		SUNN	7
#define		MERN	10
#define		VENN	9
#define		MARN	8
#define		JUPN	7
#define		SATN	7
#define		URAN	9
#define		NEPN	10
#define		PLUN	4

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[MONN];
}	mon_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[SUNN];
}	sun_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[MERN];
}	mer_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[VENN];
}	ven_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[MARN];
}	mar_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[JUPN];
}	jup_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[SATN];
}	sat_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[URAN];
}	ura_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[NEPN];
}	nep_series_struct;

//*****************************************************************************
typedef struct
{
	float	coeff;
	int8_t	pwr_of_t;
	int8_t	sin_cos;
	int8_t	trig_arg[PLUN];
}	plu_series_struct;

//*****************************************************************************
typedef struct
{
	float		map_lat;		//*	latitude of object
	float		map_long;		//*	longitude of object
	float		map_tol;		//*	lat/long tolerance
	double		jd_begin;		//*	start date
	double		jd_end;			//*	end date
	float		map_az;			//*	map azimuth
}	map_token_struct;

//*	data structure for each planet

//*****************************************************************************
typedef struct
{
	double			decl;		//*	in radians
	double			ra;			//*	in radians
	double			dist;		//*	distance from earth in AU, in earth radii for Moon

	double			decl_t0;
	double			ra_t0;
	double			dist_t0;

	double			decl_slope;	//*	in radians per day
	double			ra_slope;
	double			dist_slope;

	double			dte0;		//*	the dte for which pseries compute was done
	double			delta_dte;	//*	the delta t over which interpolation is ok

	double			radius;		//*	apparent radius in radians
	double			v,u,w;
	int				magn;
	unsigned int	phase_angle;
	unsigned int	position_angle;
	unsigned char	color_index;
	unsigned char	shape_index;
}	planet_struct;

#define	kPlanetObjectCnt	10


enum{ST_STAR,ST_DEEP,ST_NAME,ST_ALWAYS};
enum{MON,SUN,MER,VEN,MAR,JUP,SAT,URA,NEP,PLU,GEOMON};

//*****************************************************************************
enum
{
//		WHITE,
//		BLACK,
//		RED,
//		GREEN,
//		BLUE,
//		MAGENTA,
//		LIGHTMAGENTA,
//		CYAN,
//		BROWN,
//		YELLOW,
//		LIGHTGRAY,
//		DARKGRAY,

		USERBIT_LINE	=	20,
		SOLID_LINE,
		NORM_WIDTH,
		THICK_WIDTH,
		SOLID_FILL,

		last
};

#ifdef __cplusplus
	extern "C" {
#endif

long	ReadStarData(TYPE_CelestData **objptr,unsigned int *sort_index, TYPE_Time *timePtr);


extern	TYPE_CelestData		*gStarDataPtr;
extern	long				gStarCount;

extern	TYPE_CelestData		*gYaleStarDataPtr;
extern	long				gYaleStarCount;

extern	TYPE_Constelation	*gConstelations;
extern	short				gConstelationCount;

extern	TYPE_CelestData		*gConstStarPtr;
extern	long				gConstStarCount;

//*	Hipparcos
extern	TYPE_CelestData		*gHipObjectPtr;
extern	long				gHipObjectCount;

//*	Messier
extern	TYPE_CelestData		*gMessierObjectPtr;
extern	long				gMessierObjectCount;

//*	HYG
extern	TYPE_CelestData		*gHYGObjectPtr;
extern	long				gHYGObjectCount;

//*	Henry Draper
extern	TYPE_CelestData		*gDraperObjectPtr;
extern	long				gDraperObjectCount;

//*	special objects for local use.
extern	TYPE_CelestData		*gSpecialObjectPtr;
extern	long				gSpecialObjectCount;

//*	Polar Alignment center points
extern	TYPE_CelestData		*gPolarAlignObjectPtr;
extern	long				gPolarAlignObjectCount;

//*	AAVSO alert list
extern	TYPE_CelestData		*gAAVSOalertsPtr;
extern	long				gAAVSOalertsCnt;

//*	Gaia star catalog
extern	TYPE_CelestData		*gGaiaObjectPtr;
extern	long				gGaiaObjectCnt;

extern	TYPE_CelestData		*gNGCobjectPtr;
extern	long				gNGCobjectCount;

extern	TYPE_CelestData		*gSAOobjectPtr;
extern	long				gSAOobjectCount;


extern	char				gNGCDatbase[];


void		Center_RA_DEC(double argRA_radians, double argDecl_radians);
void		Center_CelestralObject(TYPE_CelestData *starObject);


#ifdef __cplusplus
}
#endif


#endif // _SKY_STRUCTS_H_

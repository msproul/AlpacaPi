//*****************************************************************************
//*	ConstellationData.h
//*	Constellation outlines based on data from
//*		https://www.iau.org/public/themes/constellations/
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  5,	2021	<MLS> Created ConstellationData.h
//*****************************************************************************
//#include	"ConstellationData.h"

#ifndef _CONSTELLATION_DATA_H_
#define	_CONSTELLATION_DATA_H_

#ifndef _SKY_STRUCTS_
	#include	"SkyStruc.h"
#endif


//*****************************************************************************
typedef struct
{
	double	rtAscension;
	double	declination;
}	TYPE_RaDec;

#define	kMaxConstPointCnt	55

//*****************************************************************************
typedef struct
{
	char		shortName[8];
	char		longName[32];
	double		rtAscension;		//*	these are the points for the center of the outline
	double		declination;
	TYPE_RaDec	path[kMaxConstPointCnt];

} TYPE_ConstOutline;

#define	kConstOutlineCnt	92
#define	kMaxConstVecotrPts	40


//*****************************************************************************
typedef struct
{
	bool		foundFlag;
	bool		moveFlag;
	int			hippIdNumber;
	double		rtAscension;
	double		declination;

} TYPE_HippStar;


//*****************************************************************************
typedef struct
{
	char			longName[32];
	double			rtAscension;		//*	these are the points for the center of the outline
	double			declination;
	TYPE_HippStar	hippStars[kMaxConstVecotrPts];
	int				starCount;

} TYPE_ConstVector;



#ifdef __cplusplus
	extern "C" {
#endif

TYPE_ConstOutline	*ReadConstellationOutlines(const char *filePath, int *objectCount);
TYPE_ConstVector	*ReadConstellationVectors(const char *directoryPath, int *objectCount);
void				SetHipparcosDataPointers(TYPE_CelestData *theHippDataPtr, long theHippObjectCnt);


#ifdef __cplusplus
}
#endif

#endif // _CONSTELLATION_DATA_H_

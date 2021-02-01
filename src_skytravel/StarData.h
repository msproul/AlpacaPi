//*****************************************************************************
//#include	"StarData.h"


#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))
#define	DEGREES(radians)	((radians) * (180.0 / M_PI))


#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_



//#define		kSkyTravelDataDirectory	"skytravel_data"

#ifdef __cplusplus
	extern "C" {
#endif



TYPE_CelestData		*GetDefaultStarData(long *objectCount, TYPE_Time *timePtr);
long				GetFileSize(const char *filePath);

TYPE_CelestData		*ReadTSCfile(const char *filePath, int dataSource, long *objectCount);
void				DumpCelestDataStruct(const char *functionName, TYPE_CelestData *objectStruct);


#ifdef __cplusplus
}
#endif

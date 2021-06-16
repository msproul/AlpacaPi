//*****************************************************************************
//#include	"HipparcosCatalog.h"

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef _SKY_STRUCTS_
	#include	"SkyStruc.h"
#endif

TYPE_CelestData	*ReadHipparcosStarCatalog(long *starCount);
int	ReadCommonStarNames(TYPE_CelestData	*hipStarData, long hipStarCount);



#ifdef __cplusplus
}
#endif




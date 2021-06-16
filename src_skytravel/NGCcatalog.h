//#include	"NGCcatalog.h"


#ifndef _SKY_STRUCTS_
	#include	"SkyStruc.h"
#endif

#ifdef __cplusplus
	extern "C" {
#endif

TYPE_CelestData	*ReadNGCStarCatalog(long *starCount);
bool			GetObjectDescription(TYPE_CelestData *objectPtr, char *returnText, short maxTexLen);


#ifdef __cplusplus
}
#endif

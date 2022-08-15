//*****************************************************************************
//*	SAO_stardata.h
//*****************************************************************************
//#include	"SAO_stardata.h"



#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif


#ifdef __cplusplus
	extern "C" {
#endif

TYPE_CelestData	*SAO_ReadFile(long *starCount);

#ifdef __cplusplus
}
#endif

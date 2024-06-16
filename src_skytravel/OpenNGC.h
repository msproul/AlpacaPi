//*****************************************************************************
//#include	"OpenNGC.h"

#ifndef	_OPEN_NGC_H_
#define	_OPEN_NGC_H_

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_

#ifndef	_OUTLINE_DATA_H_
	#include	"outlinedata.h"
#endif


#ifdef __cplusplus
	extern "C" {
#endif



TYPE_CelestData		*Read_OpenNGC_StarCatalog(long *starCount);
TYPE_OutlineData	*Read_OpenNGC_Outline_catgen(long *pointCount);

extern	TYPE_OutlineData	*gOpenNGC_outlines;
extern	long				gOpenNGC_outlineCnt;


#ifdef __cplusplus
}
#endif



#endif // _OPEN_NGC_H_

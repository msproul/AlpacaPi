//*****************************************************************************
//#include	"OpenNGC.h"

#ifndef	_OPEN_NGC_H_
#define	_OPEN_NGC_H_

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_


#ifdef __cplusplus
	extern "C" {
#endif

//************************************************************************
typedef struct
{
		double	ra_rad;
		double	decl_rad;
		int		flag;
} TYPE_OpenNGCoutline;



TYPE_CelestData		*Read_OpenNGC_StarCatalog(long *starCount);
TYPE_OpenNGCoutline	*Read_OpenNGC_Outline_catgen(long *pointCount);

extern	TYPE_OpenNGCoutline	*gOpenNGC_outlines;
extern	long				gOpenNGC_outlineCnt;


#ifdef __cplusplus
}
#endif



#endif // _OPEN_NGC_H_

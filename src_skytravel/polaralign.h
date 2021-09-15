//*****************************************************************************


#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_

int		ComputeCenterFromStarList(	TYPE_CelestData	*starList,
									int				totalStars,
									char			fistCharOfName,
									TYPE_CelestData	*centerPointArray,
									int				maxCenterPtCnt);

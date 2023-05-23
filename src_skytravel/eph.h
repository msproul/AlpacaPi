//*****************************************************************************

#ifndef _EPH_H_
#define _EPH_H_

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif


#ifdef __cplusplus
	extern "C" {
#endif

//*	sas -> side.angle.side
//*	sss -> side.side.side.


void eph(TYPE_SkyTime*,TYPE_LatLon*,planet_struct*,sun_moon_struct*);
void sphsas(TYPE_SpherTrig *sphptr);
void sphsss(TYPE_SpherTrig *sphptr);

#ifdef __cplusplus
}
#endif

#endif // _EPH_H_

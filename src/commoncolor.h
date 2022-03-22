//**************************************************************************************
//#include	"commoncolor.h"


#ifndef _COMMON_COLOR_H_
#define _COMMON_COLOR_H_

#ifndef _GCC_STDINT_H
	#include	<stdint.h>
#endif


#ifdef __cplusplus
	extern "C" {
#endif


//**************************************************************************************
typedef struct
{
	uint16_t	red;
	uint16_t	grn;
	uint16_t	blu;
} RGBcolor;

void	GetDefaultColors(	const char	overRideChar,
							const char	*windowName,
							RGBcolor	*bgColor,
							RGBcolor	*txColor);
#ifdef __cplusplus
}
#endif


#endif // _COMMON_COLOR_H_

//*****************************************************************************
//*	GaiaData.h
//*		https://gea.esac.esa.int/archive/
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Nov 10,	2021	<MLS> Created GaiaData.h
//*****************************************************************************
//#include	"GaiaData.h"

#error	"GAIA data from disk no longer supported"

#if 0
#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif

TYPE_CelestData	*ReadGaiaExtractedList(long *gaiaObjCnt);

TYPE_CelestData	*ReadGaiaDataDirectory(	const char	*dirName,
										double		magnitudeLimit,
										long		*objectCount);
#endif

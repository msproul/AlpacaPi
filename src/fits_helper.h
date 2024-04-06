//*****************************************************************************
//#include	"fits_helper.h"

#ifndef _FITS_HELPER_H_
#define	_FITS_HELPER_H_

//*****************************************************************************
enum
{
	kFitsKeyword_dummy		=	0,
	kFitsKeyword_ApertureDiam,
	kFitsKeyword_Camera,
	kFitsKeyword_CCDTEMP,
	kFitsKeyword_Date,
	kFitsKeyword_EXPTIME,
	kFitsKeyword_Filter,
	kFitsKeyword_FocalLength,
	kFitsKeyword_FRatio,
	kFitsKeyword_Gain,
	kFitsKeyword_Location,
	kFitsKeyword_MoonAge,
	kFitsKeyword_MoonIllumination,
	kFitsKeyword_MoonPhase,
	kFitsKeyword_Object,
	kFitsKeyword_Observer,
	kFitsKeyword_Observatory,
	kFitsKeyword_Telescope,
	kFitsKeyword_TimeUTC,
	kFitsKeyword_TimeLocal,
	kFitsKeyword_WebSite,

	kFitsKeyword_last
};



int	FITS_FindKeyWordEnum(const char *fitsKeyword);

//*****************************************************************************
typedef struct
{
	char	fitsLine[88];
	bool	lineSelected;
} TYPE_FitsHdrLine;

#define	kMaxFitsHdrLines	200

#endif // _FITS_HELPER_H_


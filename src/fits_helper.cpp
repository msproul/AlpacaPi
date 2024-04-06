//*****************************************************************************
//*		fits_helper.cpp		(c) 2024 by Mark Sproul
//*
//*
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 17,	2024	<MLS> Created fits_helper.cpp
//*****************************************************************************

#include	"alpacadriver_helper.h"
#include	"fits_helper.h"


//*****************************************************************************
TYPE_LookupTable	gFitsLookUpTable[]	=
{
	{	"APTDIA",		kFitsKeyword_ApertureDiam		},
	{	"CAMERA",		kFitsKeyword_Camera				},
	{	"CCD-TEMP",		kFitsKeyword_CCDTEMP			},
	{	"Date",			kFitsKeyword_Date				},
	{	"EXPTIME",		kFitsKeyword_EXPTIME			},
	{	"FILTER",		kFitsKeyword_Filter				},
	{	"FOCALLEN",		kFitsKeyword_FocalLength		},
	{	"FRATIO",		kFitsKeyword_FRatio				},
	{	"Gain",			kFitsKeyword_Gain				},
	{	"LOCATION",		kFitsKeyword_Location			},
	{	"MOONAGE",		kFitsKeyword_MoonAge			},
	{	"MOONILUM",		kFitsKeyword_MoonIllumination	},
	{	"MOONPHAS",		kFitsKeyword_MoonPhase			},
	{	"OBJECT",		kFitsKeyword_Object				},
	{	"OBSERVER",		kFitsKeyword_Observer			},
	{	"OBSERVAT",		kFitsKeyword_Observatory		},
	{	"TELESCOP",		kFitsKeyword_Telescope			},
	{	"DATE-OBS",		kFitsKeyword_TimeUTC			},
	{	"TimeLocal",	kFitsKeyword_TimeLocal			},
	{	"WEBSITE",		kFitsKeyword_WebSite			},

	{	"",				-1								}
};


//*****************************************************************************
int	FITS_FindKeyWordEnum(const char *fitsKeyword)
{
int		fitsEnum;
char	myKeyWord[12];
int		ccc;

//	CONSOLE_DEBUG(__FUNCTION__);
	ccc	=	0;
	while ((fitsKeyword[ccc] > 0x20) && (fitsKeyword[ccc] != '=') && (ccc < 8))
	{
		myKeyWord[ccc]	=	fitsKeyword[ccc];
		ccc++;
	}
	myKeyWord[ccc]	=	0;
	fitsEnum	=	LookupStringInTable(myKeyWord, gFitsLookUpTable);
	return(fitsEnum);
}


//**************************************************************************
//*	Name:			obsconditions_globals.h
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:
//*
//*		This is for weather related information that may be popluated by
//*		different means.
//*		For example:
//*			Local weather station attached to the device
//*			Infomation obtained via network querry to another alpaca devices
//*		This data is then available for anything that needs it.
//*			FITS files
//*			Determining if things need to be shut down for safety
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jan  2,	2020	<MLS> Created obsconditions_globals.h
//**************************************************************************
//#include	"obsconditions_globals.h"

#ifndef _OBSCONDITION_GLOBALS_H_
#define	_OBSCONDITION_GLOBALS_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif
#ifndef _SYS_TIME_H
	#include <sys/time.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

//**************************************************************************
typedef struct
{
	bool			siteDataValid;
	char			siteDataSource[64];
	struct timeval	siteLastUpdate;			//*	Time of last env data update
	bool			siteTemperature_valid;
	double			siteTemperature_degC;
	double			sitePressure_kPa;
	double			siteHumidity;

	bool			domeDataValid;
	char			domeDataSource[64];
	struct timeval	domeLastUpdate;			//*	Time of last env data update
	bool			domeTemperature_valid;
	double			domeTemperature_degC;
	double			domePressure_kPa;
	double			domeHumidity;


} TYPE_OBS_GLOBALS;

extern	TYPE_OBS_GLOBALS	gEnvData;

void	InitObsConditionGloblas(void);


#ifdef __cplusplus
}
#endif



#endif	//	_OBSCONDITION_GLOBALS_H_
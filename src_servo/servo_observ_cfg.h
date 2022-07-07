//******************************************************************************
//*	Name:			servo_observ_cfg.h
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description: Reads/parses the telescope mount location config files
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++ and led by Mark Sproul
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.
//*	You must determine the suitability of this source code for your use.
//*
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*	<RNS>	=	Ron N Story
//*****************************************************************************
//*	Jun 28,	2022	<RNS> Initial version of this file
//*	Jul  3,	2022	<RNS> renamed default file to servo_observatory.cfg
//*	Jul  3,	2022	<RNS> Moved enum of config files to .c file
//****************************************************************************
//#include	"servo_observ_cfg.h"

#ifndef _SERVO_OBSERV_CFG_H_
#define _SERVO_OBSERV_CFG_H_

#ifndef	_INCLUDED_SERVO_STD_DEFS_
	#include	"servo_std_defs.h"
#endif

// Default name for the telescope location config file
#define kOBSERV_CFG_FILE "servo_observatory.cfg"

//*****************************************************************************
// list of defined tokens in config file and array to keep track of discovery
enum
{
	EPOCH	=	0,
	EPOCH_JD,
	LATITUDE,
	LONGITUDE,
	ELEVATION,
	TEMPERATURE,
	PRESSURE,
	SITE,

	OBS_CFG_LAST
};	// of enum

//*****************************************************************************
typedef struct
{
	double	baseEpoch;
	double	baseJd;
	double	lat;
	double	lon;
	double	elev;
	double	temp;
	double	press;
	char	site[kMAX_STR_LEN];
} TYPE_OBSERV_CFG;

extern TYPE_OBSERV_CFG	gServoObservCfg;
extern TYPE_CFG_ITEM	gObservConfigArray[];

#ifdef __cplusplus
	extern "C" {
#endif

		int			Servo_read_observ_cfg(const char *observCfgFile);
		bool		Servo_check_observ_cfg(void);

		//*	get routines
		double		Servo_get_lat(void);
		double		Servo_get_lon(void);
		double		Servo_get_elev(void);
		const char	*Servo_get_site(void);
		double		Servo_get_temperature(void);
		double		Servo_get_pressure(void);

		//*	set routines
		void		Servo_set_lat(double newLatValue);
		void		Servo_set_lon(double newLonValue);
		void		Servo_set_elev(double newElevValue);
		void		Servo_set_temperature(double newTempValue);
		void		Servo_set_pressure(double newPressureValue);

		void		Print_observ_configuration(void);

#ifdef __cplusplus
}
#endif

#endif // _SERVO_OBSERV_CFG_H_

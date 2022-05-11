//*****************************************************************************
//*	Name:			servo_time.h
//*
//*	Author:			Ron Story (C) 2022
//*
//*	Description:    Include file for servo time, coordinates,location functions
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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*	<RNS>	=	Ron N Story
//*****************************************************************************
//*	Apr 25,	2022	<RNS> Created servo_time.h using cproto
//*	Apr 29,	2022	<RNS> added local typedefs and fixed macros back to functions
//*	May  6,	2022	<RNS> add Time_str_to_upper()
//*****************************************************************************
// $ cproto servo_time.c

#define kLOCAL_CFG_FILE "servo_location.cfg"

typedef struct local_cfg_t
{
	double	baseEpoch;
	double	baseJd;
	double	lat;
	double	lon;
	double	elev;
	double	temp;
	double	press;
	char	site[kMAX_STR_LEN];
}	localCfg, *localCfgPtr;

extern localCfg	gServoLocalCfg;

void		Time_str_to_upper(char *in);
void		Time_deci_days_to_hours(double *day);
void		Time_deci_hours_to_deg(double *hours);
void		Time_deci_deg_to_hours(double *deg);
long double	Time_get_systime(void);
long double	Time_systime_to_jd(void);
long double	Time_jd_to_sid(long double jd);
long double	Time_sid_to_lst(long double sid, double lon);
void		Time_check_hms(double *hms);
void		Time_deci_hours_to_hms(double *value);
void		Time_hms_hours_to_deci(double *value);
void		Time_ra_dec_to_alt_azi(double ra, double dec, long double sid, double lat, double *alt, double *azi);
double		Time_calc_field_rotation(double alt, double azi, double lat);
double		Time_calc_refraction(double alt, double temp, double press);
int			Time_read_local_cfg(localCfgPtr local, char *localCfgFile);

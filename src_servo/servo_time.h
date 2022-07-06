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
//*	May 14,	2022	<RNS> added Time_ascii_maybe_hms_tof()
//*	May 15,	2022	<RNS> added Time_normalize_ functions
//*	May 15,	2022	<RNS> added alt-azi tracking rates function
//*	May 15,	2022	<RNS> added Time_alt_azi_to_ra_dec() function
//*	May 16,	2022	<RNS> privatized the gServoLocalCfg global, added get_ field calls
//*	May 26,	2022	<MLS> Made gServoLocalCfg global so it can be accessed by AlpacaPi
//*	May 28,	2022	<RNS> Renamed _jd_to_sid() to _js_to_gmst()
//*	May 29,	2022	<RNS> Renamed _sid_to_lst() to _gmst_to_lst() for consistancy
//*	Jun 28,	2022	<RNS> Moved location type/data/routines to new _local_cfg.h
//*****************************************************************************
// $ cproto servo_time.c


#ifdef __cplusplus
extern "C"
{
#endif

	// void		Time_deci_days_to_hours(double *day);
	void		Time_deci_hours_to_deg(double *hours);
	void		Time_deci_deg_to_hours(double *deg);
	void		Time_normalize_HA(double *ha);
	void		Time_normalize_hours(long double *hours);
	void		Time_normalize_RA(double *ra);
	void		Time_str_to_upper(char *in);
	double		Time_ascii_maybe_HMS_tof(char *token);
	long double	Time_get_systime(void);
	long double	Time_systime_to_jd(void);
	long double	Time_jd_to_gmst(long double jd);
	long double	Time_gmst_to_lst(long double sid, double lon);
	void		Time_check_hms(double *hms);
	void		Time_deci_hours_to_hms(double *value);
	void		Time_hms_hours_to_deci(double *value);
	void		Time_ra_dec_to_alt_azi(double ra, double dec, long double lst, double lat, double *alt, double *azi);
	void		Time_alt_azi_to_ra_dec(double alt, double azi, long double lst, double lat, double *ra, double *dec);
	int			Time_calc_alt_azi_tracking(double alt, double azi, double lat, double *rateAlt, double *rateAzi);
	double		Time_calc_field_rotation(double alt, double azi, double lat);
	double		Time_calc_refraction(double alt, double temp, double press);
	int			Time_read_local_cfg(const char *localCfgFile);
#ifdef __cplusplus
}
#endif

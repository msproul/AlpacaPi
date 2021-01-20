//**********************************************************************
//	various functions to update time/date
//**********************************************************************

#include	<math.h>
#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"SkyStruc.h"

#include	"SkyTravelConstants.h"
#include	"SkyTravelTimeRoutines.h"


//* return the last day of the current month
//**********************************************************************
unsigned char Lastday(TYPE_Time *timeptr)
{
unsigned char	lastd[12]	= {31,28,31,30,31,30,31,31,30,31,31,31};
int				yr,flg		=	0;	//* flg=0 means default gregorian

	yr	=	timeptr->year;

	if (timeptr->month==2)	//* Feb. check
	{
		if (timeptr->calflag==1)
		{
			flg++;
		}
		if (timeptr->calflag==0)	//* auto
		{
			if (timeptr->year<1582)
			{
				flg++;	//* jul
			}
			if (timeptr->year==1582)
			{
				if (timeptr->month<10)
				{
					flg++;	//*jul
				}
				if (timeptr->month==10)
				{
					if (timeptr->day<15) flg++;	//* jul
				}
			}
		}

		if (!flg && (((!(yr&3)) && (!(yr%100)) && ((yr%400)!=0))))
		{
			lastd[1]	=	29;	//* leap year test
		}
		else if (!(yr&3))
		{
			lastd[1]	=	29;
		}
	}
	return(lastd[timeptr->month-1]);
}

//**********************************************************************
void Add_year(TYPE_Time *timeptr,int delta)
{
	timeptr->year	+=	delta;
	if (timeptr->day > Lastday(timeptr))
	{
		timeptr->day	=	Lastday(timeptr);
	}
}

//**********************************************************************
void Add_month(TYPE_Time *timeptr)
{
	timeptr->month++;
	if (timeptr->month>12)
	{
		timeptr->month	=	1;
		Add_year(timeptr,1);
	}
	else if (timeptr->day > Lastday(timeptr))
	{
		timeptr->day	=	Lastday(timeptr);
	}
}

//**********************************************************************
void Add_day(TYPE_Time *timeptr)
{
	timeptr->day++;
	if (timeptr->day > Lastday(timeptr))
	{
		timeptr->day	=	1;
		Add_month(timeptr);
	}
}

//**********************************************************************
void Add_hour(TYPE_Time *timeptr)
{
	timeptr->hour++;
	if (timeptr->hour>23)
	{
		timeptr->hour	-=	24;
		Add_day(timeptr);
	}
}

//**********************************************************************
void Add_min(TYPE_Time *timeptr)
{
timeptr->min++;
	if (timeptr->min>59)
	{
		timeptr->min	-=	60;
		Add_hour(timeptr);
	}
}

//**********************************************************************
void Add_sec(TYPE_Time *timeptr)
{
	timeptr->sec++;
	if (timeptr->sec>59)
	{
		timeptr->sec	-=	60;
		Add_min(timeptr);
	}
}

//**********************************************************************
void Sub_year(TYPE_Time *timeptr,int delta)
{
	timeptr->year	-=	delta;
	if (timeptr->day > Lastday(timeptr))
	{
		timeptr->day	=	Lastday(timeptr);
	}
}

//**********************************************************************
void Sub_month(TYPE_Time *timeptr)
{
	timeptr->month--;
	if (timeptr->month<1)
	{
		timeptr->month	=	12;
		Add_year(timeptr,-1);
	}
	else if (timeptr->day > Lastday(timeptr))
	{
		timeptr->day	=	Lastday(timeptr);
	}
}


//**********************************************************************
void Sub_day(TYPE_Time *timeptr)
{
	timeptr->day--;
	if (timeptr->day<1)
	{
		Sub_month(timeptr);
		timeptr->day	=	Lastday(timeptr);
	}
}

//**********************************************************************
void Sub_hour(TYPE_Time *timeptr)
{
	timeptr->hour--;
	if (timeptr->hour<0)
	{
		timeptr->hour	+=	24;
		Sub_day(timeptr);
	}
}

//**********************************************************************
void Sub_min(TYPE_Time *timeptr)
{
	timeptr->min--;
	if (timeptr->min<0)
	{
		timeptr->min	+=	60;
		Sub_hour(timeptr);
	}
}

//**********************************************************************
void Sub_sec(TYPE_Time *timeptr)
{
	timeptr->sec--;
	if (timeptr->sec<0)
	{
		timeptr->sec	+=	60;
		Sub_min(timeptr);
	}
}

//**********************************************************************
void Add_local_year(TYPE_Time *timeptr, int delta)
{
	timeptr->local_year	+=	delta;
	if (timeptr->local_day > Lastday(timeptr))
	{
		timeptr->local_day	=	Lastday(timeptr);
	}
}

//**********************************************************************
void Add_local_month(TYPE_Time *timeptr)
{
	timeptr->local_month++;
	if (timeptr->local_month>12)
	{
		timeptr->local_month	=	1;
		Add_local_year(timeptr,1);
	}
	else if (timeptr->local_day > Lastday(timeptr))
	{
		timeptr->local_day	=	Lastday(timeptr);
	}
}

//**********************************************************************
void Add_local_day(TYPE_Time *timeptr)
{
	timeptr->local_day++;
	if (timeptr->local_day > Lastday(timeptr))
	{
		timeptr->local_day	=	1;
		Add_local_month(timeptr);
	}
}

//**********************************************************************
void Sub_local_month(TYPE_Time *timeptr)
{
	timeptr->local_month--;
	if (timeptr->local_month<1)
	{
		timeptr->local_month	=	12;
		Add_local_year(timeptr,-1);
	}
	else if (timeptr->local_day > Lastday(timeptr))
	{
		timeptr->local_day	=	Lastday(timeptr);
	}
}

//**********************************************************************
void Sub_local_day(TYPE_Time *timeptr)
{
	timeptr->local_day--;
	if (timeptr->local_day<1)
	{
		Sub_local_month(timeptr);
		timeptr->local_day	=	Lastday(timeptr);
	}
}


//**************************************************************************
//* TIMEZONE(as a 0.0-1.0 fraction) = (24*.LONG+.5) IF >12 ZONE = ZONE -24
//**************************************************************************
void	Compute_Timezone(TYPE_LatLon *locptr, TYPE_Time *timeptr)
{
double dtemp;

	dtemp	=	locptr->longitude;
	if (dtemp < 0.)
	{
		dtemp	+=	kTWOPI;
	}
	dtemp				/=	kTWOPI;			//* convert to 0.0-1.0 fraction


	timeptr->timezone	=	(24.0 * dtemp + 0.5);
//	CONSOLE_DEBUG_W_DBL("dtemp\t=", dtemp);
//	CONSOLE_DEBUG_W_NUM("timezone\t=", timeptr->timezone);

	if (timeptr->timezone > 12)
	{
		timeptr->timezone	-=	24;
	}
//	CONSOLE_DEBUG_W_NUM("timezone\t=", timeptr->timezone);
}

//*****************************************************************
//* return the julian day from the gregorian calendar date
//* using van flandern equation
static	long	Gregorian(TYPE_Time *timeptr)
{
long	jd;
long	yr,mo,day;

	yr	=	timeptr->year;
	mo	=	timeptr->month;
	day	=	timeptr->day;
//	jd	=	367 * (long int)yr-(7*((long int)yr + (mo + 9)/12)/4)-(3*((yr+(mo-9)/7)/100+1)/4)+((275*mo)/9)+day+1721029;
	jd	=	367 * yr - (7*(yr + (mo + 9)/12)/4) - (3*((yr + (mo-9)/7)/100 + 1)/4) + ((275*mo)/9) + day + 1721029;
	return(jd);
}

//*****************************************************************
//* return the julian day from the julian calendar date
static	long	Julian(TYPE_Time *timeptr)
{
int		a,b;
long	jd;
int		yr,mo;

	yr	=	timeptr->year;
	mo	=	timeptr->month;

	if (timeptr->month<3)
	{
		yr	-=	1;
		mo	+=	12;
	}

	a	=	yr / 4;
	b	=	FM * (mo + 1);
	jd	=	a + YR365 * ((long int)yr - 1860) + b + timeptr->day - 105;
	return(jd + JDOFS);
}


//*****************************************************************************
//	Do all the calendric and time stuff
//*****************************************************************************
void	CalanendarTime(TYPE_Time *timeptr)	//* compute dte and cent
{
int		flg	=	0;	//* 0 = gregorian calendar default, >0 = julian calendar
double	dtemp; 		//* floating point accumulator
double	hr,min,sec;

//	CONSOLE_DEBUG(__FUNCTION__);

	hr	=	timeptr->hour;
	min	=	timeptr->min;
	sec	=	timeptr->sec;

	timeptr->fgmt	=	(hr + (min + (sec/60.))/60.)/24.;

	//* note that if calflag>1 gregorian is used since flg remains 0
	if (timeptr->calflag == 1)
	{
		flg++;	//* force julian
	}
	if (timeptr->calflag==0)	//* auto, ergo decide on gregorian vs. julian
	{
		if (timeptr->year<1582)
		{
			flg++;	//* jul
		}
		if (timeptr->year == 1582)
		{
			if (timeptr->month<10)
			{
				flg++;	//*	jul
			}
			if (timeptr->month == 10)
			{
				if (timeptr->day<15)
				{
					flg++;	//* jul
				}
			}
		}
	}

	if (!flg)
	{
		timeptr->fJulianDay	=	Gregorian(timeptr) - .5 + timeptr->fgmt;
	}
	else
	{
		timeptr->fJulianDay	=	Julian(timeptr) - .5 + timeptr->fgmt;
	}

	timeptr->daysTillEpoch2000	=	timeptr->fJulianDay - F2000;				//*	days to epoch 2000
	timeptr->cent				=	(timeptr->daysTillEpoch2000 / FCENT) + 1.;	//*	centuries from epoch 1900
	dtemp						=	timeptr->fgmt + STCA + timeptr->cent * (STCB + (STCC * timeptr->cent));	//* sidereal time
	timeptr->fSiderealTime		=	dtemp - floor(dtemp);						//* just the fractional part

	//* corrections start here
	//*
	//*CORRECTION TO 'CENT' IS MADE HERE
	//*SO THAT EPHEMERIS DOESN'T DRIFT OVER
	//*TIME PERIODS >> 300 YEARS

	//*	timeptr->cent	=	FDGF * sin(timeptr->cent / FDGF);

	//*CORRECTION OF GMT (UT) TO EPHEMERIS TIME (ET)

	//	timeptr->dte+	=	ETA + (timeptr->cent * (ETB + ETC * timeptr->cent));
	//	timeptr->cent	=	1.0 + (timeptr->dte / FCENT);

	//* corrections end here
}

//**********************************************************************
void Local_Time(TYPE_Time *timeptr)
{
	timeptr->local_year		=	timeptr->year;
	timeptr->local_month	=	timeptr->month;
	timeptr->local_day		=	timeptr->day;
	timeptr->local_hour		=	timeptr->hour + timeptr->timezone;

	if (timeptr->dstflag)
	{
		timeptr->local_hour++;	//* daylight savings time
	}
	if (timeptr->timezone < 0)
	{
		if (timeptr->local_hour < 0)
		{
			timeptr->local_hour	+=	24;
			Sub_local_day(timeptr);
		}
	}

	if (timeptr->timezone > 0)
	{
		if (timeptr->local_hour > 23)
		{
			timeptr->local_hour	-=	24;
			Add_local_day(timeptr);
		}
	}
	timeptr->dyear	=	timeptr->year;
	timeptr->dmonth	=	timeptr->month;
	timeptr->dday	=	timeptr->day;
	timeptr->dhour	=	timeptr->hour;

	if (timeptr->local_time_flag)
	{
		timeptr->dyear	=	timeptr->local_year;
		timeptr->dmonth	=	timeptr->local_month;
		timeptr->dday	=	timeptr->local_day;
		timeptr->dhour	=	timeptr->local_hour;
	}
}

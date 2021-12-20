//*****************************************************************************
//*	eph.c
//*	uses Van Flandern paper on low-precision formula for
//*	planetary positions - The Astrophysical Journal
//*	Supplemental Series, 41:391-411, 1979 Nov
//*****************************************************************************
//*	Edit history
//*****************************************************************************
//*	Written by Frank Covets and Clif Ashcraft
//*	May  2,	1996	<MLS> (Mark Sproul) Starting on Sky Travel for Frank and Clif
//*	Nov 20,	1999	<MLS> Minor formatting cleanup to improve readability
//*	Jan  2,	2021	<MLS> More formatting cleanup
//*****************************************************************************
//*	sas -> side.angle.side
//*	sss -> side.side.side.
//*****************************************************************************

#include	<math.h>
#include	<stdlib.h>

#include	"SkyStruc.h"
#include	"SkyTravelConstants.h"
//#include	"view.h"
#include	"eph.h"


static	void	pseries(double *primary_arg, double dte, int planet, planet_struct *planptr);


//*****************************************************************************
//* sphsas - given sides b,c and angle alpha, compute side c
//*	sas -> side.angle.side
//*****************************************************************************
void	sphsas(TYPE_SpherTrig *sphptr)
{
	sphptr->aside	=	acos(	cos(sphptr->bside - sphptr->cside) -
								sin(sphptr->bside) * sin(sphptr->cside) * (1. - cos(sphptr->alpha)));
}

//**************************************************************************
//* sphsss - given sides a,b,c compute gamma (angle opp. side c)
//*	sss -> side.side.side.
//**************************************************************************
void	sphsss(TYPE_SpherTrig *sphptr)
{
	if (sphptr->aside > kEPSILON)
	{
		sphptr->gamma	=	asin(sin(sphptr->cside) * sin(sphptr->alpha) / sin(sphptr->aside));
	}
	else
	{
		sphptr->gamma	=	0.0;
	}
	//* supplement gamma if cos(c)<cos(b)*cos(a)

	if (cos(sphptr->cside) < cos(sphptr->bside) * cos(sphptr->aside))
	{
		sphptr->gamma	=	PI - sphptr->gamma;
	}
}



//*****************************************************************************
//void eph(time_struct *timeptr,locn_struct *locptr,planet_struct **planptr,sun_moon_struct *sunmonptr)
//*****************************************************************************
void	eph(	TYPE_Time		*timeptr,
				TYPE_LatLon		*locptr,
				planet_struct	*planptr,
				sun_moon_struct	*sunmonptr)
{
int				iii;
double			dte;
double			delta_dte;
double			ra;
double			decl;
double			dist;
double			delta_ra;
double			emr0;					//* earth moon distance ratio
double			dtemp;
double			umbra_radius;
double			primary_arg[33];		//* primary argument in radians
TYPE_SpherTrig	sphptr;

	dte		=	timeptr->daysTillEpoch2000;

	for (iii=PLU; iii>=0; iii--)
	{
		delta_dte	=	dte - planptr[iii].dte0;
		if (fabs(delta_dte)>planptr[iii].delta_dte)
		{
			//* compute the pseries for dte + delta dte

			pseries(primary_arg, dte + planptr[iii].delta_dte, iii, &planptr[iii]);	//* update series for t+delta t
			ra		=	planptr[iii].ra;		//* hang on to the coords for t=t+delta t
			decl	=	planptr[iii].decl;
			dist	=	planptr[iii].dist;

			//* compute the pseries for dte

			pseries(primary_arg, dte, iii, &planptr[iii]);	//* update series for time t

			//* save the new dte, ra,decl and dist

			planptr[iii].ra_t0		=	planptr[iii].ra;
			planptr[iii].decl_t0	=	planptr[iii].decl;
			planptr[iii].dist_t0	=	planptr[iii].dist;
			planptr[iii].dte0		=	dte;					//* reset t0

			//* save the new slopes = (param(t+delta t) -parm(t))/(delta t)

			delta_ra	=	ra-planptr[iii].ra_t0;

			if (delta_ra>PI)
			{
				delta_ra	-=	kTWOPI;				//* check for wraparound
			}
			if (delta_ra<-PI)
			{
				delta_ra	+=	kTWOPI;
			}
			planptr[iii].ra_slope	=	delta_ra/planptr[iii].delta_dte;
			planptr[iii].decl_slope	=	(decl-planptr[iii].decl_t0) / planptr[iii].delta_dte;
			planptr[iii].dist_slope	=	(dist-planptr[iii].dist_t0) / planptr[iii].delta_dte;
		}
		else	//* do interpolation param=param(t0)+slope*(t-t0)
		{
			planptr[iii].ra		=	planptr[iii].ra_t0+(planptr[iii].ra_slope*delta_dte);
			planptr[iii].decl	=	planptr[iii].decl_t0+(planptr[iii].decl_slope*delta_dte);
			planptr[iii].dist	=	planptr[iii].dist_t0+(planptr[iii].dist_slope*delta_dte);
		}
	}

	//* special computes for Sun and Moon
	//* need sun elev for day/night/twilight/night sky colors

	sunmonptr->lunar_ecl_flag	=	false;	//* defaults
	sunmonptr->solar_ecl_flag	=	false;

	sphptr.alpha	=	kTWOPI * timeptr->fSiderealTime - planptr[SUN].ra + locptr->longitude;
	sphptr.bside	=	kHALFPI - locptr->latitude;
	sphptr.cside	=	kHALFPI - planptr[SUN].decl;
	sphsas(&sphptr);
	sphsss(&sphptr);
	sunmonptr->sun_elev	=	kHALFPI - sphptr.aside;
	sunmonptr->sun_az	=	sphptr.gamma;

	//* compute moon topographic coords
	//* topo-elev = geoelev-asin(cos(geoelev)/sqrt(1.+(emr0*emr0)-(2.*emr0*sin(geoelev))))
	//* where emr0=moon distance/earth radius
	//* then convert delta elev to ra/decl

	emr0			=	planptr[MON].dist;
	sphptr.alpha	=	kTWOPI * timeptr->fSiderealTime - planptr[MON].ra + locptr->longitude;
	sphptr.cside	=	kHALFPI - locptr->latitude;
	sphptr.bside	=	kHALFPI - planptr[MON].decl;
	sphsas(&sphptr);
	sphsss(&sphptr);
	sunmonptr->mon_geo_elev		=	kHALFPI - sphptr.aside;
	sunmonptr->mon_geo_az		=	sphptr.gamma;
	sunmonptr->mon_topo_dist	=	sqrt(1.0 + (emr0 * emr0) - (2.0 * emr0 * sin(sunmonptr->mon_geo_elev)));

	dtemp						=	asin(cos(sunmonptr->mon_geo_elev)/sunmonptr->mon_topo_dist);
	sunmonptr->mon_topo_decl	=	planptr[MON].decl - (dtemp*cos(sphptr.gamma));
	sunmonptr->mon_topo_ra		=	planptr[MON].ra - (dtemp*sin(sphptr.gamma)/sin(sphptr.bside));

	planptr[MON].radius		=	asin(MOON_RADIUS / (EARTH_RADIUS * sunmonptr->mon_topo_dist));
	planptr[SUN].radius		=	asin(SUN_RADIUS / (AU * planptr[SUN].dist));

	//*	now do solar and lunar eclipse comps

	//*	max angular sun-moon dist for solar eclipse

	sunmonptr->smdmax	=	planptr[MON].radius+planptr[SUN].radius;

	//* umbral shadow radius in radians

	dtemp							=	emr0*(SUN_RADIUS-EARTH_RADIUS)/(AU*planptr[SUN].dist);
	umbra_radius					=	atan((1.-dtemp) / sunmonptr->mon_topo_dist);
	sunmonptr->earth_shadow_radius	=	umbra_radius;

	//*	umbra_radius=(sunmonptr->earth_shadow_radius=asin((1./emr0)-((SUN_RADIUS-EARTH_RADIUS)/(planptr[SUN]->dist*AU))));

	//* max angular earthshadow-moon dist for lunar eclipse

	sunmonptr->emdmax	=	planptr[MON].radius+umbra_radius;

	//* check for solar eclipse

	sphptr.alpha	=	sunmonptr->mon_topo_ra - planptr[SUN].ra;	//*moon-sun delta ra
	sphptr.bside	=	kHALFPI - planptr[SUN].decl;				//* sun co-decl
	sphptr.cside	=	kHALFPI - sunmonptr->mon_topo_decl;			//* moon co-decl
	sphsas(&sphptr);
	sphsss(&sphptr);
	sunmonptr->smang	=	sphptr.gamma;

	//* sun-moon distance (in radians)
	if ((sunmonptr->smdist=sphptr.aside) < sunmonptr->smdmax)
	{
		sunmonptr->solar_ecl_flag	=	true;		//* solar eclipse possible!!
	}
	//*	else check for lunar eclipse by computing angular distance between
	//*	anti-sun and geo_moon
	else
	{
		sphptr.alpha	=	PI+planptr[SUN].ra-planptr[MON].ra;		//* antisun-geomoon ra
		sphptr.bside	=	kHALFPI - planptr[MON].decl;			//* moon geo co-decl
		sphptr.cside	=	kHALFPI + planptr[SUN].decl;			//* antisun co-decl
		sphsas(&sphptr);
		if ((sunmonptr->emdist	=	sphptr.aside)<sunmonptr->emdmax)
		{
			sunmonptr->lunar_ecl_flag	=	true;		//* lunar eclipse possible!!
		}
	}

	//* dummy magnitudes (until magphs is implemented)
	planptr[MER].magn	=	5;
	planptr[VEN].magn	=	6;
	planptr[MAR].magn	=	5;
	planptr[JUP].magn	=	5;
	planptr[SAT].magn	=	4;
	planptr[URA].magn	=	1;
	planptr[NEP].magn	=	0;
	planptr[PLU].magn	=	0;

}

//*****************************************************************************
//* the primary argument parameters from Van Flandern
//**************************************************************************
static	void	uparg(double *primary_arg,double dte)
{
double ipart;
//*	note args 6 and 30 are blank
double	intercept_arg[33]=
	{
	.606434,		//*(01)Moon	Lm
	.374897,		//*(02)		Gm
	.259091,		//*(03)		Fm
	.827362,		//*(04)		D
	.347343,		//*(05)		Wm

	.000000,		//*(06)

	.779072,		//*(07)Sun	Ls
	.993126,		//*(08)		Gs

	.700695,		//*(09)Mer	L1
	.485541,		//*(10)		G1
	.566441,		//*(11)		F1

	.505498,		//*(12)Ven	L2
	.140023,		//*(13)		G2
	.292498,		//*(14)		F2

	.987353,		//*(15)Mar	L4
	.053856,		//*(16)		G4
	.849694,		//*(17)		F4

	.089608,		//*(18)Jup	L5
	.056531,		//*(19)		G5
	.814794,		//*(20)		F5

	.133295,		//*(21)Sat	L6
	.882987,		//*(22)		G6
	.821218,		//*(23)		F6

	.870169,		//*(24)Ura	L7
	.400589,		//*(25)		G7
	.664614,		//*(26)		F7

	.846912,		//*(27)Nep	L8
	.725368,		//*(28)		G8
	.480856,		//*(29)		F8

	.000000,		//*(30)

	.663854,		//*(31)Plu	L9
	.041020,		//*(32)		G9
	.357355 		//*(33)		F9
	};

//*	note args 6 and 30 are blank
double	slope_arg[33]=
	{
	.03660110129,	//*0(01)Moon	Lm
	.03629164709,	//*1(02)		Gm
	.03674819520,	//*2(03)		Fm
	.03386319198,	//*3(04)		D
	-.00014709391,	//*4(05)		Wm

	.000000,		//*(06)

	.00273790931,	//*5(07)Sun	Ls
	.00273777850,	//*6(08)	Gs

	.01136771400,	//*7(09)Mer	L1
	.01136759566,	//*8(10)	G1
	.01136762384,	//*9(11)	F1

	.00445046867,	//*10(12)Ven	L2
	.00445036173,	//*11(13)		G2
	.00445040017,	//*12(14)		F2

	.00145575328,	//*13(15)Mar	L4
	.00145561327,	//*14(16)		G4
	.00145569465,	//*15(17)		F4

	.00023080893,	//*16(18)Jup	L5
	.00023080893,	//*17(19)		G5
	.00023080893,	//*18(20)		F5

	.00009294371,	//*19(21)Sat	L6
	.00009294371,	//*20(22)		G6
	.00009294371,	//*21(23)		F6

	.00003269438,	//*22(24)Ura	L7
	.00003269438,	//*23(25)		G7
	.00003265562,	//*24(26)		F7

	.00001672092,	//*25(27)Nep	L8
	.00001672092,	//*26(28)		G8
	.00001663715,	//*27(29)		F8

	.000000,	//*(30)

	.00001115482,	//*28(31)Plu	L9
	.00001104864,	//*29(32)		G9
	.00001104864	//*30(33)		F9
	};

int index;

	for	(index=0;index<33;index++)	//* !! note - in radians !!
	{
		primary_arg[index]	=	kTWOPI * modf(intercept_arg[index]+dte*slope_arg[index],&ipart);
	}
}

//*****************************************************************************
//*	compute the vuw series for the Moon
//**************************************************************************
static	void	update_mon_series(double *primary_arg, double dte, planet_struct *planptr)
{
double			cent,sum,prod;
double			vuw[3];
int				jj,kk,ll;
int				ii;
mon_series_struct *serptr;

	//* the v,u, and w series data for moon

	#include	"MONSER.DTA"

	uparg(primary_arg, dte);

	cent	=	(dte/FCENT)+1.;

	//* compute the vuw-series

	for(ll=0;ll<3;ll++) vuw[ll]	=	0.;

	for(ll=0;ll<3;ll++)
	{
		ii=0;

		switch(ll)
		{
			case 0: serptr	=	&mon_v_data[ii];	break;
			case 1: serptr	=	&mon_u_data[ii];	break;
			case 2: serptr	=	&mon_w_data[ii];	break;
		}

		do
		{
			sum	=	0.;
			for (jj=0;jj<MONN;jj++)
			{
				kk	=	serptr->trig_arg[jj];
				if (kk != 0)
				{
					sum	+=	kk * primary_arg[mon_prim_arg_index[jj]];
				}
			}
			if (serptr->sin_cos=='s') sum	=	sin(sum);
				else sum	=	cos(sum);
			prod	=	sum;
			kk		=	serptr->pwr_of_t;

			while (kk-- != 0) prod	*=	cent;	//*	the time power

			prod	*=	serptr->coeff;
			vuw[ll]	+=	prod;
			ii++;
			switch(ll)
			{
				case 0: serptr	=	&mon_v_data[ii];break;
				case 1: serptr	=	&mon_u_data[ii];break;
				case 2: serptr	=	&mon_w_data[ii];break;
			}
		} while (serptr->coeff != 0);
	}

	planptr->v	=	vuw[0];
	planptr->u	=	vuw[1];
	planptr->w	=	vuw[2];
}

//*****************************************************************************
//* compute the vuw series for the Sun
//**************************************************************************
static	void	update_sun_series(double *primary_arg, double dte, planet_struct *planptr)
{
double	cent,sum,prod;
double	vuw[3];
int		j,k,l;
int		ii	=0;
sun_series_struct *serptr;

	//* the v,u, and w series data for sun

	#include	"SUNSER.DTA"

	uparg(primary_arg, dte);
	cent	=	(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0; l<3; l++)
	{
		vuw[l]	=	0.;
	}

	for(l=0; l<3; l++)
	{
		ii	=	0;

		switch(l)
		{
			case 0: serptr	=	&sun_v_data[ii];break;
			case 1: serptr	=	&sun_u_data[ii];break;
			case 2: serptr	=	&sun_w_data[ii];break;
		}
		do
		{
			sum	=	0.;
			for(j=0;j<SUNN;j++)
			{
				k	=	serptr->trig_arg[j];
				if (k!=0) sum	+=	k*primary_arg[sun_prim_arg_index[j]];
			}
			if (serptr->sin_cos=='s')
			{
				sum	=	sin(sum);
			}
			else
			{
				sum	=	cos(sum);
			}
			prod	=	sum;
			k		=	serptr->pwr_of_t;
			while (k--!=0)
			{
				prod	*=	cent;	//* the time power
			}

			prod	*=	serptr->coeff;
			vuw[l]	+=	prod;
			ii++;
			switch(l)
			{
				case 0: serptr	=	&sun_v_data[ii];break;
				case 1: serptr	=	&sun_u_data[ii];break;
				case 2: serptr	=	&sun_w_data[ii];break;
			}
		} while (serptr->coeff!=0);
	}

	planptr->v	=	vuw[0];
	planptr->u	=	vuw[1];
	planptr->w	=	vuw[2];
}

//*****************************************************************************
//* compute the vuw series for Mercury
//**************************************************************************
static	void	update_mer_series(double *primary_arg, double dte, planet_struct *planptr)
{
double	cent,sum,prod;
double	vuw[3];
int		j,k,l;
int		i=0;
mer_series_struct *serptr;

	//* the v,u, and w series data for mercury

	#include	"MERSER.DTA"

	uparg(primary_arg,dte);
	cent	=	(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0;l<3;l++) vuw[l]=0.;

	for(l=0;l<3;l++)
	{
		i	=	0;

		switch(l)
		{
			case 0: serptr	=	&mer_v_data[i];break;
			case 1: serptr	=	&mer_u_data[i];break;
			case 2: serptr	=	&mer_w_data[i];break;
		}

		do
		{
			sum	=	0.;
			for(j=0;j<MERN;j++)
			{
				k	=	serptr->trig_arg[j];
				if (k!=0) sum+=k*primary_arg[mer_prim_arg_index[j]];
			}
			if (serptr->sin_cos=='s') sum	=	sin(sum);
				else sum	=	cos(sum);
			prod	=	sum;
			k		=	serptr->pwr_of_t;
			while (k--!=0) prod	*=	cent;	//* the time power
			prod	*=	serptr->coeff;
			vuw[l]	+=	prod;
			i++;
			switch(l)
			{
				case 0: serptr	=	&mer_v_data[i];break;
				case 1: serptr	=	&mer_u_data[i];break;
				case 2: serptr	=	&mer_w_data[i];break;
			}
		} while (serptr->coeff!=0);
	}

	planptr->v	=	vuw[0];
	planptr->u	=	vuw[1];
	planptr->w	=	vuw[2];
}

//*****************************************************************************
//* compute the vuw series for Venus
//**************************************************************************
static	void	update_ven_series(double *primary_arg, double dte, planet_struct *planptr)
{
double	cent,sum,prod;
double	vuw[3];
int		j,k,l;
int		i=0;
ven_series_struct *serptr;

	//* the v,u, and w series data for venus

	#include	"VENSER.DTA"

	uparg(primary_arg, dte);
	cent=(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0;l<3;l++) vuw[l]=0.;

	for(l=0;l<3;l++)
	{
		i=0;

		switch(l)
		{
			case 0: serptr=&ven_v_data[i];break;
			case 1: serptr=&ven_u_data[i];break;
			case 2: serptr=&ven_w_data[i];break;
		}

		do
		{
			sum=0.;
			for(j=0;j<VENN;j++)
			{
				k	=	serptr->trig_arg[j];
				if (k!=0) sum	+=	k*primary_arg[ven_prim_arg_index[j]];
			}
			if (serptr->sin_cos=='s') sum=sin(sum);
				else sum=cos(sum);
			prod	=	sum;
			k		=	serptr->pwr_of_t;
			while (k--!=0) prod*=cent;		//* the time power
			prod	*=	serptr->coeff;
			vuw[l]	+=	prod;
			i++;
			switch(l)
			{
				case 0: serptr=&ven_v_data[i];break;
				case 1: serptr=&ven_u_data[i];break;
				case 2: serptr=&ven_w_data[i];break;
			}
		} while (serptr->coeff!=0);
	}

	planptr->v=vuw[0];
	planptr->u=vuw[1];
	planptr->w=vuw[2];
}

//*****************************************************************************
//* compute the vuw series for Mars
//**************************************************************************
static	void	update_mar_series(double *primary_arg, double dte, planet_struct *planptr)
{
double	cent,sum,prod;
double	vuw[3];
int		j,k,l;
int		i=0;
mar_series_struct *serptr;

	//* the v,u, and w series data for mars

	#include	"MARSER.DTA"

	uparg(primary_arg, dte);
	cent=(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0;l<3;l++) vuw[l]=0.;

	for(l=0;l<3;l++)
	{
		i=0;

		switch(l)
		{
			case 0: serptr=&mar_v_data[i];break;
			case 1: serptr=&mar_u_data[i];break;
			case 2: serptr=&mar_w_data[i];break;
		}

		do
		{
			sum=0.;
			for(j=0;j<MARN;j++)
			{
				k	=	serptr->trig_arg[j];
				if (k!=0) sum	+=	k * primary_arg[mar_prim_arg_index[j]];
			}
			if (serptr->sin_cos=='s')
			{
				sum	=	sin(sum);
			}
			else
			{
				sum	=	cos(sum);
			}
			prod	=	sum;
			k		=	serptr->pwr_of_t;
			while (k--!=0) prod*=cent;	//* the time power
			prod	*=	serptr->coeff;
			vuw[l]	+=	prod;
			i++;
			switch(l)
			{
				case 0: serptr=&mar_v_data[i];break;
				case 1: serptr=&mar_u_data[i];break;
				case 2: serptr=&mar_w_data[i];break;
			}
		} while (serptr->coeff!=0);
	}

	planptr->v	=	vuw[0];
	planptr->u	=	vuw[1];
	planptr->w	=	vuw[2];
}

//*****************************************************************************
//* compute the vuw series for Jupiter
//**************************************************************************
static	void	update_jup_series(double *primary_arg, double dte, planet_struct *planptr)
{
double cent,sum,prod;
double vuw[3];
int j,k,l;
int i=0;
jup_series_struct *serptr;

	//* the v,u, and w series data for jupiter

	#include	"JUPSER.DTA"

	uparg(primary_arg, dte);
	cent=(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0;l<3;l++) vuw[l]=0.;

	for(l=0;l<3;l++)
	{
		i	=	0;

		switch(l)
		{
			case 0: serptr=&jup_v_data[i];break;
			case 1: serptr=&jup_u_data[i];break;
			case 2: serptr=&jup_w_data[i];break;
		}

		do
		{
			sum=0.;
			for(j=0;j<JUPN;j++)
			{
				k	=	serptr->trig_arg[j];
				if (k!=0) sum	+=	k * primary_arg[jup_prim_arg_index[j]];
			}
			if (serptr->sin_cos=='s')
			{
				sum	=	sin(sum);
			}
			else
			{
				sum	=	cos(sum);
			}
			prod	=	sum;
			k		=	serptr->pwr_of_t;
			while (k--!=0) prod*=cent;	//* the time power
			prod	*=	serptr->coeff;
			vuw[l]	+=	prod;
			i++;
			switch(l)
			{
				case 0: serptr=&jup_v_data[i];break;
				case 1: serptr=&jup_u_data[i];break;
				case 2: serptr=&jup_w_data[i];break;
			}
		} while (serptr->coeff!=0);
	}

	planptr->v=vuw[0];
	planptr->u=vuw[1];
	planptr->w=vuw[2];
}

//*****************************************************************************
//* compute the vuw series for Saturn
//**************************************************************************
static	void	update_sat_series(double *primary_arg, double dte, planet_struct *planptr)
{
double cent,sum,prod;
double vuw[3];
int j,k,l;
int i=0;
sat_series_struct *serptr;

	//* the v,u, and w series data for saturn

	#include	"SATSER.DTA"

	uparg(primary_arg,dte);

	cent=(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0;l<3;l++) vuw[l]=0.;

	for(l=0;l<3;l++)
	{
		i	=	0;

		switch(l)
		{
			case 0: serptr=&sat_v_data[i];break;
			case 1: serptr=&sat_u_data[i];break;
			case 2: serptr=&sat_w_data[i];break;
		}

		do
		{
			sum	=	0.;
			for(j=0;j<SATN;j++)
			{
				k	=	serptr->trig_arg[j];
				if (k!=0) sum+=k*primary_arg[sat_prim_arg_index[j]];
			}
			if (serptr->sin_cos == 's') sum=sin(sum);
				else sum=cos(sum);

			prod	=	sum;
			k		=	serptr->pwr_of_t;
			while (k--!=0) prod	*=	cent;	//* the time power
			prod	*=	serptr->coeff;
			vuw[l]	+=	prod;
			i++;
			switch(l)
			{
				case 0: serptr=&sat_v_data[i];break;
				case 1: serptr=&sat_u_data[i];break;
				case 2: serptr=&sat_w_data[i];break;
			}
		} while (serptr->coeff!=0);
	}

	planptr->v=vuw[0];
	planptr->u=vuw[1];
	planptr->w=vuw[2];
}

//*****************************************************************************
//* compute the vuw series for Uranus
//**************************************************************************
static	void	update_ura_series(double *primary_arg, double dte, planet_struct *planptr)
{
double cent,sum,prod;
double vuw[3];
int j,k,l;
int i=0;
ura_series_struct *serptr;

	//* the v,u, and w series data for uranus

	#include	"URASER.DTA"

	uparg(primary_arg, dte);
	cent=(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0;l<3;l++) vuw[l]=0.;

	for(l=0;l<3;l++)
	{
		i=0;

		switch(l)
		{
			case 0: serptr=&ura_v_data[i];break;
			case 1: serptr=&ura_u_data[i];break;
			case 2: serptr=&ura_w_data[i];break;
		}

		do
		{
			sum=0.;
			for(j=0;j<URAN;j++)
				{
				k=serptr->trig_arg[j];
				if (k!=0) sum+=k*primary_arg[ura_prim_arg_index[j]];
				}
			if (serptr->sin_cos=='s') sum=sin(sum);
				else sum=cos(sum);
			prod	=	sum;
			k		=	serptr->pwr_of_t;
			while (k--!=0) prod*=cent;	//* the time power
			prod	*=	serptr->coeff;
			vuw[l]	+=	prod;
			i++;
			switch(l)
			{
				case 0: serptr=&ura_v_data[i];break;
				case 1: serptr=&ura_u_data[i];break;
				case 2: serptr=&ura_w_data[i];break;
			}
		} while (serptr->coeff!=0);
	}

	planptr->v	=	vuw[0];
	planptr->u	=	vuw[1];
	planptr->w	=	vuw[2];
}

//*****************************************************************************
//* compute the vuw series for Neptune
//**************************************************************************
static	void	update_nep_series(double *primary_arg, double dte, planet_struct *planptr)
{
double	cent,sum,prod;
double	vuw[3];
int		j,k,l;
int		i=0;
nep_series_struct *serptr;

	//* the v,u, and w series data for neptune

	#include	"NEPSER.DTA"

	uparg(primary_arg, dte);
	cent=(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0;l<3;l++) vuw[l]=0.;

	for(l=0;l<3;l++)
	{
		i=0;

		switch(l)
		{
			case 0: serptr=&nep_v_data[i];break;
			case 1: serptr=&nep_u_data[i];break;
			case 2: serptr=&nep_w_data[i];break;
		}

		do
			{
			sum=0.;
			for(j=0;j<NEPN;j++)
			{
				k=serptr->trig_arg[j];
				if (k!=0) sum+=k*primary_arg[nep_prim_arg_index[j]];
			}
			if (serptr->sin_cos == 's') sum	=	sin(sum);
				else sum	=	cos(sum);
			prod	=	sum;
			k		=	serptr->pwr_of_t;
			while (k--!=0) prod*=cent;	//* the time power
			prod	*=	serptr->coeff;
			vuw[l]	+=	prod;
			i++;
			switch(l)
			{
				case 0: serptr=&nep_v_data[i];break;
				case 1: serptr=&nep_u_data[i];break;
				case 2: serptr=&nep_w_data[i];break;
			}
		} while (serptr->coeff!=0);
	}

	planptr->v=vuw[0];
	planptr->u=vuw[1];
	planptr->w=vuw[2];
}

//*****************************************************************************
//* compute the vuw series for Pluto
//**************************************************************************
static	void	update_plu_series(double *primary_arg, double dte, planet_struct *planptr)
{
double	cent,sum,prod;
double	vuw[3];
int		j,k,l;
int		i=0;
plu_series_struct	*serptr;

	//* the v,u, and w series data for pluto

	#include	"PLUSER.DTA"

	uparg(primary_arg, dte);
	cent=(dte/FCENT)+1.;

	//* compute the vuw-series

	for(l=0;l<3;l++) vuw[l]=0.;

	for(l=0;l<3;l++)
	{
	i=0;

	switch(l)
		{
		case 0: serptr=&plu_v_data[i];break;
		case 1: serptr=&plu_u_data[i];break;
		case 2: serptr=&plu_w_data[i];break;
		}

	do
		{
		sum=0.;
		for(j=0;j<PLUN;j++)
		{
			k=serptr->trig_arg[j];
			if (k!=0) sum+=k*primary_arg[plu_prim_arg_index[j]];
		}
		if (serptr->sin_cos=='s') sum=sin(sum);
			else sum=cos(sum);
		prod=sum;
		k=serptr->pwr_of_t;
		while (k--!=0) prod*=cent;	//* the time power
		prod*=serptr->coeff;
		vuw[l]+=prod;
		i++;
		switch(l)
			{
			case 0: serptr=&plu_v_data[i];break;
			case 1: serptr=&plu_u_data[i];break;
			case 2: serptr=&plu_w_data[i];break;
			}
		}
	while (serptr->coeff!=0);
	}

	planptr->v	=	vuw[0];
	planptr->u	=	vuw[1];
	planptr->w	=	vuw[2];
}

#if 0
//*****************************************************************************
void printra(double ra)
{
double	hr,min,sec;
double	conv	=	12.0 / PI;	//* converts radians to hours

	hr	=	ra * conv;
	if (hr < 0)
	{
		hr	=	24.0+hr;
	{
	min	=	60.*modf(hr,&min);
	sec	=	60.*modf(min,&sec);
	printf("%2dh %2dm %2ds\t",(int)hr,(int)min,(int)sec);
	return;
}
#endif

//*****************************************************************************
//*	update series
//**************************************************************************
static	void	pseries(double *primary_arg, double dte, int planet, planet_struct *planptr)
{
//* scale factor for dist
double scale_factor[10]=
		{
		60.40974,
		1.00021,
		1.07693,
		1.23437,
		1.83094,
		5.30693,
		9.61711,
		19.24877,
		30.08900,
		41.32680
		};

	//*	the primary_arg term is Lm for Moon, Ls for the Sun,Mer, and Ven and Lp for
	//*	the exterior planets, so use the following index table


	unsigned char argindex[10]={0,6,6,6,14,17,20,23,26,30};

	switch(planet)
	{
		case MON:update_mon_series(primary_arg,dte,planptr);break;
		case SUN:update_sun_series(primary_arg,dte,planptr);break;
		case MER:update_mer_series(primary_arg,dte,planptr);break;
		case VEN:update_ven_series(primary_arg,dte,planptr);break;
		case MAR:update_mar_series(primary_arg,dte,planptr);break;
		case JUP:update_jup_series(primary_arg,dte,planptr);break;
		case SAT:update_sat_series(primary_arg,dte,planptr);break;
		case URA:update_ura_series(primary_arg,dte,planptr);break;
		case NEP:update_nep_series(primary_arg,dte,planptr);break;
		case PLU:update_plu_series(primary_arg,dte,planptr);break;
		default: break;
	}
	planptr->ra		=	primary_arg[argindex[planet]]+asin(planptr->w/sqrt(planptr->u-(planptr->v*planptr->v)));	//* right ascension
	planptr->decl	=	asin(planptr->v/sqrt(planptr->u));			//* decl
	planptr->dist	=	scale_factor[planet] * sqrt(planptr->u);	//* dist
}



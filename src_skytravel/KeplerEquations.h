//*****************************************************************************
//#include	"KeplerEquations.h"

#ifndef _KEPLER_EQUATIONS_H_
#define _KEPLER_EQUATIONS_H_

#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))

//*****************************************************************************
enum
{
	kPlanet_Mercury	=	0,
	kPlanet_Venus,
	kPlanet_Earth,
	kPlanet_Mars,
	kPlanet_Jupiter,
	kPlanet_Saturn,
	kPlanet_Uranus,
	kPlanet_Neptune,
//	kPlanet_Pluto,

//	kPlanet_Haumea,

	kPlanet_Last
};
//*****************************************************************************
typedef struct
{
	char		name[24];
	int			idNumber;
	double		magnitude;
	int			epoch;
	double		elem_a;				//*	au
	double		elem_a_dot;			//*	au / century
	double		elem_e;				//*	rad
	double		elem_e_dot;			//*	rad / century
	double		elem_I;				//*	deg
	double		elem_I_dot;			//*	deg / century
	double		elem_L;				//*	deg
	double		elem_L_dot;			//*	deg / century
	double		elem_omega_bar;		//*	deg
	double		elem_omega_bar_dot;	//*	deg / century
	double		elem_Omeaga;		//*	deg
	double		elem_Omeaga_dot;	//*	deg / century

	double		a;				//*	au
	double		e;				//*	rad
	double		I;				//*	deg
	double		L;				//*	deg
	double		omega_bar;		//*	deg
	double		Omeaga;			//*	deg

	double		E;

	double		planet_x;
	double		planet_y;
	double		planet_z;

	double		oribit_days;

	double		elem_q;		//*	both in AU
	double		elem_Q;

} TYPE_KeplerData;

extern	TYPE_KeplerData	gPlanetKepData[];

extern	TYPE_KeplerData	*gAsteriodData;
extern	int				gAsteroidCount;

extern	TYPE_KeplerData	*gTransNeptunianData;
extern	int				gTransNeptunianCount;

void	Kepler_InitPlanets(void);
void	Kepler_CalculatePlanetLocation(TYPE_KeplerData	*objectData,
										double			time,
										double			*planet_x,
										double			*planet_y,
										double			*planet_z);


#endif // _KEPLER_EQUATIONS_H_

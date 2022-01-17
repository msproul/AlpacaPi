//*********************************************************************
//*	Oct 23,	2021	<MLS> Added .dispMagnitude option
//*********************************************************************

//#include	"SkyDisplayStruct.h"




#define	_SKY_DISPLAY_H_

//*********************************************************************
typedef enum
{
	kSpecialDisp_Off	=	0,
	kSpecialDisp_All,
	kSpecialDisp_Arcs_w_CentVect,
	kSpecialDisp_Arcs_noLabels,
	kSpecialDisp_ArcsOnly,

	kSpecialDisp_Last

} TYPE_SPECIAL_DISP;

//*********************************************************************
//*	these variables are a separate structure so that the entire thing
//*	can be passed to various routines without having to pass the entire window object
typedef struct
{

		//*	overlays
		bool				dispGrid;			//*	Grid on/off
		bool				dispHorizon_line;	//*	Horizon line on/off
		bool				dispEquator_line;	//*	equator line on/off
		bool				ecliptic_line;
		bool				dispEarth;			//*	earth lines on/off

		bool				dispTelescope;		//*	telescope pointing area on/off
		bool				dispDomeSlit;		//*	dome slit opening


		//*	data
		bool				dispDefaultData;		//*	this is the original data file
		bool				dispLines;				//*	Constellation lines on/off
		bool				dispNames;
		bool				dispSymbols;			//*	Symbols on/off
		bool				dispNGC;				//*	NGC objects on/off
		bool				dispHIP;				//*	Hipparcos objects on/off
		bool				dispDeep;				//*	Deep sky objects on/off
		bool				dispYale;				//*	Deep Yale catalog on/off
		//*	added by <MLS>
		bool				dispMessier;			//*	Messier on/off
		bool				dispConstOutlines;		//*	Constellation outlines on/off
		bool				dispConstellations;		//*	Constellation vectors on/off
		bool				dispCommonStarNames;	//*	Common star names on/off
		bool				dispHYG_all;
		bool				dispDraper;				//*	Henry Draper Catalog
		bool				dispAAVSOalerts;		//*	AAVSO TargetTool Alerts

		bool				dispGaia;				//*	Gaia Catalog
		bool				dispAsteroids;			//*	Lowell Asteroid catalog

		unsigned char		dispSpecialObjects;		//*	Special.txt objects, this is an enum

} TYPE_SkyDispOptions;


//*********************************************************************
//*	these variables are a separate structure so that the entire thing
//*	can be passed to various routines without having to pass the entire window object
typedef struct
{
		bool				dispTeleScopeOutline;
		bool				dispTeleScopeCrossHairs;
		bool				dispFindScopeOutline;
		bool				dispFindScopeCrossHairs;

} TYPE_TeleDispOptions;


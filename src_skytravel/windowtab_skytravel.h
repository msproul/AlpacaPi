//*****************************************************************************
//#include	"windowtab_skytravel.h"



#ifndef	_WINDOWTAB_SKYTRAVEL_H_
#define	_WINDOWTAB_SKYTRAVEL_H_

#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _CONTROLLER_H_

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_
#ifndef _CONSTELLATION_DATA_H_
	#include	"ConstellationData.h"
#endif // _CONSTELLATION_DATA_H_

#include	"commoncolor.h"

#include	"SkyDisplayStruct.h"


//*****************************************************************************
enum
{
	kSkyTravel_Title	=	0,
	kSkyTravel_Logo,

	kSkyTravel_Display,
	kSkyTravel_Data,

	//*	these are data controls
	kSkyTravel_Btn_DeepSky,
	kSkyTravel_Btn_Names,
	kSkyTravel_Btn_Lines,
	kSkyTravel_Btn_ConstOutline,
	kSkyTravel_Btn_Constellations,
	kSkyTravel_Btn_NGC,
	kSkyTravel_Btn_Messier,
	kSkyTravel_Btn_YaleCat,
	kSkyTravel_Btn_Hipparcos,

	//*	these are display controls
	kSkyTravel_Btn_Reset,
	kSkyTravel_Btn_Chart,
	kSkyTravel_Btn_Earth,
	kSkyTravel_Btn_Grid,
	kSkyTravel_Btn_Equator,
	kSkyTravel_Btn_Ecliptic,
	kSkyTravel_Btn_NightMode,
	kSkyTravel_Btn_Symbols,
	kSkyTravel_Btn_TscopeDisp,


	kSkyTravel_Btn_Plus,
	kSkyTravel_Btn_Minus,
	kSkyTravel_Btn_ZoomLevel,

	kSkyTravel_DomeIndicator,
	kSkyTravel_TelescopeIndicator,

	//*	this MUST be the first one after the buttons
	kSkyTravel_MsgTextBox,


	kSkyTravel_HelpTextBox,


	kSkyTravel_ConnLX200,
	kSkyTravel_SyncLX200,
	kSkyTravel_LX200_RA_DEC,

	kSkyTravel_Search_Text,
	kSkyTravel_Search_Btn,


	kSkyTravel_NightSky,

	kSkyTravel_last
};


//**************************************************************************************
class WindowTabSkyTravel: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSkyTravel(	const int	xSize,
									const int	ySize,
									CvScalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabSkyTravel(void);

		virtual	void	RunBackgroundTasks(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgitIdx);
		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(const int buttonIdx);
		virtual void	ProcessMouseEvent(const int widgitIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDown(const int widgitIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonUp(const int widgitIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDragged(const int widgitIdx, const int event, const int xxx, const int yyy, const int flags);

				void	UpdateButtonStatus(void);
		//--------------------------------------------------------------
		//*	SkyTravel stuff
				void	SetSkyDisplaySize(short xSize, short ySize);
				void	SetCurrentTime(void);
				bool	Precess(void);
				bool	Precess(TYPE_CelestData	*celestObjPtr,
								long			celestObjCount,
								bool			sortFlag,
								bool			forcePrecession);
				void	DrawSkyAll(void);
				void	ResetView(void);
				short	Set_Skycolor(	planet_struct	*planptr,
										sun_moon_struct	*sunmonptr,
										bool			earthFlag);
				void	PlotObjectsByDataSource(TYPE_CelestData	*objectptr, long maxObjects);
				long	Search_and_plot(TYPE_CelestData	*objectptr, long maxObjects);
				void	DrawObjectByShape(int xcoord, int ycoord, int shape, int magn);

				void 	DrawStar_shape(short xcoord, short ycoord, short index);
				void	DrawVector(	short	color,
									short	xx,
									short	yy,
									short	scale,
									const char	*shape_data);

				void	ConvertLatLonToRaDec(TYPE_LatLon *locptr, TYPE_Time *timeptr);
				void	DrawWindowOverlays(void);
				void	DrawCompass(void);
				void	DrawGrid(short theSkyColor);

				void	DrawEcliptic(void);
				void	DrawHorizon(void);
				void	DrawHorizontalArc(double elevAngle, double startAz, double endAz);
				void	DrawVerticalArc(double azimuthAngle, double startElev, double endElev);
				int		DrawGreatCircle(double declinationAngle, bool rainbow=false);
				int		DrawNorthSouthLine(double rightAscen);

				void	PlotSkyObjects(	TYPE_CelestData	*objectptr,
										const char		**name,
										const char		**shapes,
										long			objCnt);
				void	BuildConstellationData(void);
				void	DrawConstellationLines(void);

				void	DrawConstellationOutLines(void);
				void	DrawConstellationVectors(void);

				void	DrawFeet(void);
				void	DrawTelescopeReticle(int screenXX, int screenYY);
				void	DrawDomeSlit(void);
				void	CenterOnDomeSlit(void);

				void	FindAltAz(		TYPE_LatLon *locptr, TYPE_Time *timeptr);
				void	Compute_cursor(	TYPE_Time	*timeptr, TYPE_LatLon	*locptr);
				void	DrawCursorLocationInfo(void);
				void	FindObjectNearCursor(void);
				bool	GetXYfromRA_Decl(double argRA_radians, double argDecl_radians, short *xx, short *yy);
				bool	GetXYfromAz_Elev(double azimuth_radians, double elev_radians, short *xx, short *yy);

				void	ForceReDrawSky(void);

				void	SearchSkyObjects(char *objectName);

				//*	this routine draws cute little easter eggs along the horizon,
				void	MapTokens(TYPE_Time *timeptr, TYPE_LatLon *locptr);


		//*	I am using openCV ROI for the drawing so these are always zero
		int					cWorkSpaceLeftOffset;
		int					cWorkSpaceTopOffset;

		//*	still need the offset for doing cursor calculations
		int					cCursorOffsetY;

		bool				cNightMode;
		unsigned short		cTrack;		//*	0=no tracking, 1=track cursor, 2,3 etc. means track planet
		TYPE_SkyDispOptions	cDispOptions;
		bool				cChart;				//*	Chart on/off
		bool				cFindFlag;
//-		bool				inform;
		bool				cLunarEclipseFlag;			//*	Lunar Eclipse flag
//-		bool				updflag;

		int					cMagmin;
		int					cView_index;		//*	1,2,4,8,16,32,64
		double				cView_angle;		//*	in radians
		double				cXfactor;
		double				cYfactor;
		double				cGamang;
		double				cChart_gamma;
		double				cRadmax;

		double				cElev0;				//*	values for current center of screen
		double				cAz0;
		double				cRa0;
		double				cDecl0;

		double				cElev;				//*	only used for Find
		double				cAz;				//*	only used for Find
		double				cRa;
		double				cDecl;

		//*	current cursor values (where is the user cursor pointing)
//-		bool				csr_valid;
		double				cCursor_elev;
		double				cCursor_az;
		double				cCursor_ra;
		double				cCursor_decl;

		int					cCsrx;
		int					cCsry;
		int					cSavedMouseClick_X;	//*	these are for left mouse drag operation
		int					cSavedMouseClick_Y;
		bool				cMouseDragInProgress;

		long				cInform_dist;
		long				cInform_id;
		char				cInform_name[256];

		double				cRamax;				//*	max delta ra for search
		double				cDecmax;
		double				cDecmin;

		int					cWind_x0;			//*	center of field in pixels, relative to wind_ulx,_uly
		int					cWind_y0;
		int					wind_ulx;
		int					wind_uly;
		int					cWind_width;
		int					cWind_height;

		double				sun_radius;			//*	in radians
		double				moon_radius;		//*	in radians
		double				phase_angle;		//*	for crescent moon
		double				position_angle;
		double				earth_shadow_radius;

		double				mon_geo_ra;
		double				mon_geo_decl;

		//*	end view_struct
		short				cCurrentSkyColor;
		RGBcolor			cSkyRGBvalue;



		short				currentForeColor;

		TYPE_Time			cCurrentTime;
		TYPE_LatLon			cCurrLatLon;

		//-----------------------------------------------------------
		//*	star information

		double				cDisplayedMagnitudeLimit;

		TYPE_CelestData		*cStarDataPtr;
		long				cStarCount;

		TYPE_CelestData		*cYaleStarDataPtr;
		long				cYaleStarCount;

		TYPE_Constelation	*constelations;
		short				constelationCount;
		TYPE_CelestData		*constStarPtr;
		long				constStarCount;

		TYPE_CelestData		*cNGCobjectPtr;
		long				cNGCobjectCount;

		//*	Hipparcos
		TYPE_CelestData		*cHipObjectPtr;
		long				cHipObjectCount;

		//*	Hipparcos
		TYPE_CelestData		*cTSCmessierOjbectPtr;
		long				cTSCmessierOjbectCount;



		sun_moon_struct		sunMonStruct;
		planet_struct		planetStruct[kPlanetObjectCnt];
		TYPE_CelestData		cPlanets[kPlanetObjectCnt];


		TYPE_ConstOutline	*cConstOutlinePtr;
		int					cConstOutlineCount;

		//*	Constellation vectors
		TYPE_ConstVector	*cConstVecotrPtr;
		int					cConstVectorCnt;


		//--------------------------------------------------------------
		//*	Telescope tracking stuff
		bool					cTrackTelescope;
		TYPE_TeleDispOptions	cTelescopeDisplayOptions;

		int					cDebugCounter;
};


#endif // _WINDOWTAB_SKYTRAVEL_H_


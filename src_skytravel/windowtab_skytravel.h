//*****************************************************************************
//#include	"windowtab_skytravel.h"



#ifndef	_WINDOWTAB_SKYTRAVEL_H_
#define	_WINDOWTAB_SKYTRAVEL_H_

#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif

#ifndef _CONSTELLATION_DATA_H_
	#include	"ConstellationData.h"
#endif

#ifndef _CAMERA_FOV_H_
	#include	"cameraFOV.h"
#endif

#include	"commoncolor.h"

#include	"SkyDisplayStruct.h"

#define	_ENABLE_HYG_


//*****************************************************************************
enum
{
	kSkyTravel_Title	=	0,
	kSkyTravel_Logo,

	kSkyTravel_Display,
	kSkyTravel_Data,

	//*	these are data controls
	kSkyTravel_Btn_First,
	kSkyTravel_Btn_OrigDatabase	=	kSkyTravel_Btn_First,		//*	the original SkyTravel database for C-64
	kSkyTravel_Btn_Lines,

	kSkyTravel_Btn_DeepSky,
	kSkyTravel_Btn_Names,
	kSkyTravel_Btn_CommonStarNames,
	kSkyTravel_Btn_ConstOutline,
	kSkyTravel_Btn_Constellations,
	kSkyTravel_Btn_NGC,
	kSkyTravel_Btn_Messier,
	kSkyTravel_Btn_YaleCat,
	kSkyTravel_Btn_Hipparcos,
	kSkyTravel_Btn_Draper,
	kSkyTravel_Btn_AAVSOalerts,
	kSkyTravel_Btn_Gaia,

	kSkyTravel_Btn_MagnitudeDisp,

	kSkyTravel_UTCtime,


	//*	these are display controls
	kSkyTravel_Btn_Reset,
	kSkyTravel_Btn_AutoAdvTime,
	kSkyTravel_Btn_Chart,
	kSkyTravel_Btn_Earth,
	kSkyTravel_Btn_Grid,
	kSkyTravel_Btn_Equator,
	kSkyTravel_Btn_Ecliptic,
	kSkyTravel_Btn_NightMode,
	kSkyTravel_Btn_Symbols,
	kSkyTravel_Btn_AllMagnitudes,
	kSkyTravel_Btn_TscopeDisp,


	kSkyTravel_Btn_Plus,
	kSkyTravel_Btn_Minus,
	kSkyTravel_Btn_ZoomLevel,
	kSkyTravel_Telescope_ViewAngle,
	kSkyTravel_DisplayedStarCnt,

	kSkyTravel_DomeIndicator,
	kSkyTravel_TelescopeIndicator,

	//*	this MUST be the first one after the buttons
	kSkyTravel_MsgTextBox,


	kSkyTravel_CursorInfoTextBox,
//	kSkyTravel_HelpTextBox,

	kSkyTravel_Telescope_RA_DEC,
	kSkyTravel_Telescope_Sync,
	kSkyTravel_Telescope_GoTo,


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
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);
		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual	void	ProcessDoubleClick_RtBtn(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags);

		virtual void	ProcessMouseEvent(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDown(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonUp(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDragged(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseWheelMoved(const int widgetIdx, const int event, const int xxx, const int yyy, const int wheelMovement);

				bool	ProcessSingleCharCmd(const int cmdChar);
				void	UpdateButtonStatus(void);
				void	UpdateViewAngleDisplay(void);

		//--------------------------------------------------------------
		//*	Telescope control stuff
				bool	SyncTelescopeToCenter(void);
				bool	SlewTelescopeToCenter(void);

				bool	SendAlpacaCmdToTelescope(	const char		*theCommand,
													const char		*dataString,
													SJP_Parser_t	*jsonParser);

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

				void	SetView_Angle(const double newViewAngle_radians);


				void	ZoomViewAngle(const int direction);
				void	SetMinimumViewAngle(const double minimumViewAngle);
				void	SetMaximumViewAngle(const double maximumViewAngle);
				void	ResetView(void);
				short	Set_Skycolor(	planet_struct	*planptr,
										sun_moon_struct	*sunmonptr,
										bool			earthFlag);
				void	PlotObjectsByDataSource(bool			enabled,
												TYPE_CelestData	*objectptr,
												long			maxObjects);
				long	Search_and_plot(TYPE_CelestData	*objectptr, long maxObjects, bool dataIsSorted=true);
				void	DrawObjectByShape(int xcoord, int ycoord, int shape, int magn);

				void	DrawStarFancy(	const int		xcoord,
										const int		ycoord,
										TYPE_CelestData	*theStar,
										int				textColor,
										double			viewAngle_LabelDisplay,
										double			viewAngle_InfoDisplay);
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
				int		DrawGreatCircle(const double declinationAngle_rad, const bool forceNumberDraw=false);
				int		DrawNorthSouthLine(double rightAscen);
				void	DrawScale(void);

				void	DrawMoon(	TYPE_SpherTrig	sphptr,
									TYPE_CelestData	*objectptr,
									const char		**shapes,
									short			xcoord,
									short			ycoord
								);

				void	PlotSkyObjects(	TYPE_CelestData	*objectptr,
										const char		**name,
										const char		**shapes,
										long			objCnt);
				void	BuildConstellationData(void);
				void	DrawConstellationLines(void);

				void	DrawConstellationOutLines(void);
				void	DrawConstellationVectors(void);
				void	DrawConstellationNameByViewAngle(	const int pt_XX,
															const int pt_YY,
															const char *theString);

				void	DrawCommonStarNames(void);

				void	DrawFeet(void);
				void	DrawTelescopeReticle(int screenXX, int screenYY);
				int		DrawTelescopeFOV(void);
				bool	DrawTelescopeFOV(TYPE_CameraFOV *fovPtr, short	telescopeXX, short telescopeYY);
				void	DrawDomeSlit(void);
				void	CenterOnDomeSlit(void);

				void	FindAltAz(		TYPE_LatLon *locptr, TYPE_Time *timeptr);
				void	Compute_cursor(	TYPE_Time	*timeptr, TYPE_LatLon	*locptr);
				void	DrawCursorLocationInfo(void);
				void	FindObjectNearCursor(void);
				bool	GetXYfromRA_Decl(double argRA_radians, double argDecl_radians, short *xx, short *yy);
				bool	GetXYfromAz_Elev(double azimuth_radians, double elev_radians, short *xx, short *yy);

				void	ForceReDrawSky(void);

				void	Center_RA_DEC(double argRA_radians, double argDecl_radians);
				void	Center_CelestralObject(TYPE_CelestData *starObject);
				void	SetAAVSOdisplayFlag(const bool newAAVSOdisplayState);

				void	DrawPolarAlignmentCircles(TYPE_CelestData *polarAlignCenters, long polarAlignCnt);
				void	DrawPolarAlignmentCenterVector(TYPE_CelestData *polarAlignCenters, long polarAlignCnt);
				//*	this routine draws cute little easter eggs along the horizon,
				void	MapTokens(TYPE_Time *timeptr, TYPE_LatLon *locptr);

				//=====================================================================
				//*	Stuff for searching
				void	SearchSkyObjects(char *objectName);
				bool	SearchSkyObjectsDataListByNumber(	TYPE_CelestData *starDataPtr,
															long			starCount,
															int				dataSource,
															char			*namePrefix,
															char			*searchString);
				bool	SearchSkyObjectsDataListByShortName(TYPE_CelestData *starDataPtr,
															long			starCount,
															int				dataSource,
															char			*searchString);
				bool	SearchSkyObjectsDataListByLongName(	TYPE_CelestData *starDataPtr,
															long			starCount,
															int				dataSource,
															char			*searchString);
				bool	SearchSkyObjectsConstellations(const char *searchString);
				bool	SearchSkyObjectsConstOutlines(const char *searchString);

				bool	cFoundSomething;
				double	cFound_newRA;
				double	cFound_newDec;
				char	cFoundName[64];
				char	cFoundDatabase[64];



		uint32_t			cLastUpdateTime_ms;
		uint32_t			cLastClockUpdateTime_ms;

		uint32_t			cLastRemoteImageUpdate_ms;

		//*	I am using openCV ROI for the drawing so these are always zero
		int					cWorkSpaceLeftOffset;
		int					cWorkSpaceTopOffset;

		//*	still need the offset for doing cursor calculations
		int					cCursorOffsetY;

		bool				cAutoAdvanceTime;
		bool				cNightMode;
		unsigned short		cTrack;				//*	0=no tracking, 1=track cursor, 2,3 etc. means track planet
		TYPE_SkyDispOptions	cDispOptions;
		bool				cChartMode;				//*	Chart on/off
		bool				cFindFlag;
//-		bool				inform;
		bool				cLunarEclipseFlag;	//*	Lunar Eclipse flag
//-		bool				updflag;

		int					cDisplayedStarCount;	//*	number of currently displayed stars

		int					cMagmin;
		int					cView_index;		//*	1,2,4,8,16,32,64
		double				cView_angle;		//*	in radians
		double				cXfactor;			//*	pixels per radian
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

		double				cSun_radius;		//*	in radians
		double				cMoon_radius;		//*	in radians
		double				cPhase_angle;		//*	for crescent moon
		double				cPosition_angle;
		double				cEarth_shadow_radius;

		double				mon_geo_ra;
		double				mon_geo_decl;

		//*	end view_struct
		short				cCurrentSkyColor;
		RGBcolor			cSkyRGBvalue;



		short				currentForeColor;

		TYPE_Time			cCurrentTime;
		TYPE_LatLon			cCurrLatLon;

//-		double				cDisplayedMagnitudeLimit;

		sun_moon_struct		cSunMonStruct;
		planet_struct		cPlanetStruct[kPlanetObjectCnt];
		TYPE_CelestData		cPlanets[kPlanetObjectCnt];

		//--------------------------------------------------------------
		//*	Telescope tracking stuff
		bool					cTrackTelescope;
		TYPE_TeleDispOptions	cTelescopeDisplayOptions;

		int						cDebugCounter;

		//--------------------------------------------------------------
		//*	Camera FOV stuff
		void			SetCameraFOVptr(TYPE_CameraFOV	*cameraFOVarrayPtr);
		TYPE_CameraFOV	*cCameraFOVarrayPtr;
};


#endif // _WINDOWTAB_SKYTRAVEL_H_


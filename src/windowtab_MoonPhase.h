//*****************************************************************************
//#include	"windowtab_MoonPhase.h"

#ifndef	_WINDOWTAB_MOONPHASE_H_
#define	_WINDOWTAB_MOONPHASE_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

#ifndef _NASA_MOONPHASE_H_
	#include	"NASA_moonphase.h"
#endif

//*****************************************************************************
enum
{
	kMoonPhase_Title	=	0,


//	kMoonPhase_GraphTitle

	kMoonPhase_EnablePhaseBtn,
	kMoonPhase_EnableAgeBtn,
	kMoonPhase_EnableDiamBtn,
	kMoonPhase_EnableDistBtn,
	kMoonPhase_EnableAxisBtn,
	kMoonPhase_EnableELatBtn,
	kMoonPhase_EnableELonBtn,

	kMoonPhase_EnableRA_Btn,
	kMoonPhase_EnableDEC_Btn,


//	kMoonPhase_YearLftBtn,
	kMoonPhase_YearText,
//	kMoonPhase_YearRgtBtn,
//	kMoonPhase_YearOutLine,

	kMoonPhase_HelpBtn,

	kMoonPhase_MoonPhaseGraph,
//	kMoonPhase_DisplayYear
//	kMoonPhase_DisplayMonth

	kMoonPhase_Curr_PhaseBlockTitle,
	kMoonPhase_Curr_DateLbl,
	kMoonPhase_Curr_DateValue,
	kMoonPhase_Curr_TimeLbl,
	kMoonPhase_Curr_TimeValue,
	kMoonPhase_Curr_PhaseNameLbl,
	kMoonPhase_Curr_PhaseNameValue,
	kMoonPhase_Curr_PhasePercentLbl,
	kMoonPhase_Curr_PhasePercentValue,
	kMoonPhase_Curr_AgeLbl,
	kMoonPhase_Curr_AgeValue,
	kMoonPhase_Curr_DiameterLbl,
	kMoonPhase_Curr_DiameterValue,
	kMoonPhase_Curr_DistanceLbl,
	kMoonPhase_Curr_DistanceValue,
	kMoonPhase_Curr_PolarAxisLbl,
	kMoonPhase_Curr_PolarAxisValue,

	kMoonPhase_Curr_RA_DECLbl,
	kMoonPhase_Curr_RA_DECValue,

	kMoonPhase_Curr_ELatELonLbl,
	kMoonPhase_Curr_ELatELonValue,

	kMoonPhase_Curr_OutLineBox,

	kMoonPhase_TodayButton,

	kMoonPhase_MoonBox,
	kMoonPhase_MoonBoxGraphicOverlay,
	kMoonPhase_MoonFileName,
	kMoonPhase_OverlayChkBox,

	kMoonPhase_Disp_PhaseBlockTitle,
	kMoonPhase_Disp_DateLbl,
	kMoonPhase_Disp_DateValue,
	kMoonPhase_Disp_TimeLbl,
	kMoonPhase_Disp_TimeValue,
	kMoonPhase_Disp_PhaseNameLbl,
	kMoonPhase_Disp_PhaseNameValue,
	kMoonPhase_Disp_PhasePercentLbl,
	kMoonPhase_Disp_PhasePercentValue,
	kMoonPhase_Disp_AgeLbl,
	kMoonPhase_Disp_AgeValue,
	kMoonPhase_Disp_DiameterLbl,
	kMoonPhase_Disp_DiameterValue,
	kMoonPhase_Disp_DistanceLbl,
	kMoonPhase_Disp_DistanceValue,
	kMoonPhase_Disp_PolarAxisLbl,
	kMoonPhase_Disp_PolarAxisValue,

	kMoonPhase_Disp_RA_DECLbl,
	kMoonPhase_Disp_RA_DECValue,

	kMoonPhase_Disp_ELatELonLbl,
	kMoonPhase_Disp_ELatELonValue,

	kMoonPhase_Disp_OutLineBox,

	kMoonPhase_RunButton,

	kMoonPhase_StartDownLoadBtn,
	kMoonPhase_TotalImgCnt,
	kMoonPhase_DownLoadStatus,

	kMoonPhase_AlpacaDev_Total,
	kMoonPhase_AlpacaLogo,
	kMoonPhase_last
};


//**************************************************************************************
class WindowTabMoonPhase: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabMoonPhase(const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabMoonPhase(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ActivateWindow(void);
		virtual	void	RunWindowBackgroundTasks(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual void	ProcessMouseEvent(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
//		virtual void	ProcessMouseLeftButtonDown(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
//		virtual void	ProcessMouseLeftButtonUp(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
//		virtual void	ProcessMouseLeftButtonDragged(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseWheelMoved(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags);
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_

				int		SetupPhaseInfoBlock(int		xLoc_start,
											int		yLoc_start,
											int		titleIdx,
											int		fistLblIdx,
											int		outlineIdx);
				void	DrawMoonPhaseGraph(TYPE_WIDGET *theWidget);
				void	DrawMoonWidget(TYPE_WIDGET *theWidget);
				void	DrawMoonGraphic(void);
				void	UpdateButtons(void);

				void	HandleMouseMovedInGraph(	TYPE_WIDGET *theWidget,
													const int	box_XXX,
													const int	box_YYY);

				void	HandleMouseClickedInGraph(	TYPE_WIDGET *theWidget,
													const int	box_XXX,
													const int	box_YYY);


				bool			UpdateCurrentMoonPhase(TYPE_MoonPhase *moonPhaseInfo);
				void			UpdateMoonPhaseTables(void);
				void			UpdateMoonImage(int year, int month, int day, int hour);

				int				cPhaseIndex_Now;
				int				cPhaseIndex_Displayed;

				int				cPhaseStepValue;
				bool			cPhaseSimulationEnabled;
				bool			cDisplayCurrentPhase;
				TYPE_MoonPhase	cCurrentMoonPhaseInfo;
				TYPE_MoonPhase	cDisplayedMoonPhaseInfo;
				cv::Rect		cMoonDisplayRect;

				int				cGraphColor_Phase;
				int				cGraphColor_Age;
				int				cGraphColor_Diam;
				int				cGraphColor_Dist;
				int				cGraphColor_AxisA;
				int				cGraphColor_ELat;
				int				cGraphColor_ELon;

				int				cGraphColor_RA;
				int				cGraphColor_DEC;


				int				cGraphColor_Current;
				int				cGraphColor_Displayed;

				bool			cEnableGraph_Phase;
				bool			cEnableGraph_Diam;
				bool			cEnableGraph_Dist;
				bool			cEnableGraph_Age;
				bool			cEnableGraph_AxisA;
				bool			cEnableGraph_ELat;
				bool			cEnableGraph_ELon;

				bool			cEnableGraph_RA;
				bool			cEnableGraph_DEC;
				bool			cEnableOverlay;
				//*	moon image stuff
				cv::Mat			*cMoonOpenCVimage;
				cv::Mat			*cDisplayedImage;
				int				cMoonDisplaySize;
				int				cMoonImageYear;
				int				cMoonImageIndex;	//*	this is the number of hours since Jan 1st
				char			cMoonImageName[32];
				char			cMoonImagePath[256];
				int				cMoonPhaseImageCnt;


};




#endif // _WINDOWTAB_MOONPHASE_H_

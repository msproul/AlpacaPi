//*****************************************************************************
//#include	"windowtab_solarsystem.h"



#ifndef	_WINDOWTAB_SOLAR_SYSTEM_H_
#define	_WINDOWTAB_SOLAR_SYSTEM_H_

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif


#ifndef _KEPLER_EQUATIONS_H_
	#include	"KeplerEquations.h"
#endif

//*****************************************************************************
enum
{
//	kSolarSystem_Title	=	0,
	kSolarSystem_DisplayBox,

	kSolarSystem_Btn_First,
	kSolarSystem_Btn_Reset	=	kSolarSystem_Btn_First,
	kSolarSystem_Btn_AutoAdvTime,
	kSolarSystem_Btn_DispAsteriods,
	kSolarSystem_Btn_DispAUscale,
	kSolarSystem_Btn_DispKuiperBelt,
	kSolarSystem_Btn_DispOrbit,
	kSolarSystem_Btn_DispCrossHairs,
	kSolarSystem_UTCtime,
	kSolarSystem_AsteroidMagLimit,
//	kSolarSystem_Btn_Last,

	kSolarSystem_MsgTextBox,



	kSolarSystem_last
};

#define	kMaxLogEnries	300

//**************************************************************************************
class WindowTabSolarSystem: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSolarSystem(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor);
		virtual	~WindowTabSolarSystem(void);

		virtual	void	SetupWindowControls(void);
//-		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_
		virtual	void	ActivateWindow(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual void	ProcessMouseWheelMoved(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags);

				void	DrawSolarSystem(TYPE_WIDGET *graphWidget);
				void	DrawCrossHairs(void);
				void	DrawAstronomicalScale(void);
				void	DrawAsteriods(void);
				void	DrawKuiperBelt(void);
				void	DrawOrbits(TYPE_KeplerData *objectData, int objectCount, const int color=W_WHITE);

				void	UpdateButtonStatus(void);

				double	cScaleFactor;
				int		cCenter_X;
				int		cCenter_Y;
				int		cDisplayTop;
				int		cDisplayLeft;
				int		cDisplayWidth;
				int		cDisplayHeight;

				bool	cDislayAsteriods;
				bool	cDislayAUscale;
				bool	cDislayCrossHairs;
				bool	cDislayKuiperBelt;
				bool	cDislayOrbit;
				double	cAsteroidMagLimit;


};


#endif // _WINDOWTAB_SOLAR_SYSTEM_H_

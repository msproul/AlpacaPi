//*****************************************************************************
//#include	"windowtab_dome.h"



#ifndef	_WINDOWTAB_DOME_H_
#define	_WINDOWTAB_DOME_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H_

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif // _CONTROLLER_H_


//*****************************************************************************
enum
{
	kDomeBox_Title	=	0,

	kDomeBox_Connected,

	kDomeBox_CurPosLabel,
	kDomeBox_CurPosition,

	kDomeBox_AzimLabel,
	kDomeBox_Azimuth,

	kDomeBox_AltitudeLabel,
	kDomeBox_Altitude,

	kDomeBox_ShutterLabel,
	kDomeBox_ShutterStatus,

	kDomeBox_SlavedLabel,
	kDomeBox_SlavedStatus,


	kDomeBox_GoHome,
	kDomeBox_GoPark,

	kDomeBox_Minus20,
	kDomeBox_Minus10,
	kDomeBox_Minus5,
	kDomeBox_Minus1,

	kDomeBox_Plus1,
	kDomeBox_Plus5,
	kDomeBox_Plus10,
	kDomeBox_Plus20,

	kDomeBox_Stop,

	//*	these are all extras
	kDomeBox_GoLeft,
	kDomeBox_GoRight,
	kDomeBox_SlowLeft,
	kDomeBox_SlowRight,
	kDomeBox_BumpLeft,
	kDomeBox_BumpRight,
	//*	end of extras


	kDomeBox_ToggleSlaveMode,

	kDomeBox_OpenShutter,
	kDomeBox_CloseShutter,
	kDomeBox_StopShutter,

	kDomeBox_LastCmdString,

//	kDomeBox_AlpacaLogo,
	kDomeBox_AlpacaErrorMsg,
	kDomeBox_IPaddr,
	kDomeBox_Readall,
#ifdef _ENABLE_SKYTRAVEL_
	kDomeBox_Outline,

	kDomeBox_DomeGraphic,
	kDomeBox_North,
	kDomeBox_South,
	kDomeBox_East,
	kDomeBox_West,

#endif

	kDomeBox_last
};


//**************************************************************************************
class WindowTabDome: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabDome(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName,
								const bool	parentIsSkyTravel = false);
		virtual	~WindowTabDome(void);

		virtual	void	SetupWindowControls(void);
	#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
				void	DrawDomeGraphic(cv::Mat *openCV_Image, TYPE_WIDGET *theWidget);
	#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
				void	DrawDomeGraphic(IplImage *openCV_Image, TYPE_WIDGET *theWidget);
	#endif // _USE_OPENCV_CPP_

		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual	void	UpdateControls(void);

				void	SetDomePropertiesPtr(TYPE_DomeProperties *domePropPtr);

				void	UpdateDomeAzimuth(const double newAzimuth);

				void	ResetKnownCommands(void);
				bool	SendAlpacaCmdToDome(	const char		*theCommand,
												const char		*dataString,
												SJP_Parser_t	*jsonParser = NULL);
				void	ToggleSlaveMode(void);

				void	SendShutterCommand(const char *shutterCmd);
				void	MoveDomeByAmount(const double moveAmount);

				bool	cParentIsSkyTravel;

				TYPE_DomeProperties	*cDomePropPtr;
};


#endif // _WINDOWTAB_DOME_H_

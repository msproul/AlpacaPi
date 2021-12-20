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

	kDomeBox_ErrorMsg,

	kDomeBox_LastCmdString,

	kDomeBox_AlpacaLogo,
	kDomeBox_AlpacaDrvrVersion,
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
									CvScalar	backGrndColor,
									const char	*windowName,
									const bool	parentIsSkyTravel = false);
		virtual	~WindowTabDome(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
		virtual	void	UpdateControls(void);

				void	SetDomePropertiesPtr(TYPE_DomeProperties *domePropPtr);

				void	UpdateDomeAzimuth(const double newAzimuth);
				void	DrawDomeGraphic(IplImage *openCV_Image, TYPE_WIDGET *theWidget);

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

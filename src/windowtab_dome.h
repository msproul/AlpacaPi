//*****************************************************************************
//#include	"windowtab_dome.h"



#ifndef	_WINDOWTAB_DOME_H_
#define	_WINDOWTAB_DOME_H_

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif // _CONTROLLER_H_


//*****************************************************************************
enum
{
	kDomeBox_Title	=	0,

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

	kDomeBox_Stop,

	kDomeBox_GoLeft,
	kDomeBox_GoRight,
	kDomeBox_SlowLeft,
	kDomeBox_SlowRight,
	kDomeBox_BumpLeft,
	kDomeBox_BumpRight,

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
	kDomeBox_Rescan,

#endif

	kDomeBox_last
};

#define	kMaxSwitchControlBoxes	14

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
									const char	*windowName);
		virtual	~WindowTabDome(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgitIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);

				bool	SendAlpacaCmdToDome(	const char		*theCommand,
												const char		*dataString,
												SJP_Parser_t	*jsonParser = NULL);
				void	ToggleSlaveMode(void);

				void	SendShutterCommand(const char *shutterCmd);
};


#endif // _WINDOWTAB_DOME_H_

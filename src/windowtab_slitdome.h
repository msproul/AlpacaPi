//*****************************************************************************
//#include	"windowtab_slitdome.h"



#ifndef	_WINDOWTAB_SLIT_DOME_H_
#define	_WINDOWTAB_SLIT_DOME_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kSlitDome_Title	=	0,

	kSlitDome_DomeInfoLabel,
	kSlitDome_DomeIPaddr,
	kSlitDome_DomeAlpacaPort,
	kSlitDome_DomeDevNum,

	kSlitDome_DomeName,
	kSlitDome_DomeDescription,

	kSlitDome_DomeShutterLbl,
	kSlitDome_DomeShutter,

	kSlitDome_DomePositionLbl,
	kSlitDome_DomePosition,

	kSlitDome_DomeAzimuthLbl,
	kSlitDome_DomeAzimuth,

	kSlitDome_DomeGetDataBtn,
	kSlitDome_DomeOutline,

	kSlitDome_DomeEnableData,
	kSlitDome_DomeEnableTracking,

	kSlitDome_LastCmdString,
	kSlitDome_AlpacaLogo,
	kSlitDome_AlpacaErrorMsg,
	kSlitDome_IPaddr,
	kSlitDome_Readall,
	kSlitDome_DeviceState,


	kSlitDome_last
};



//**************************************************************************************
class WindowTabSlitDome: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSlitDome(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabSlitDome(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);

				void	SetDomePropertiesPtr(TYPE_DomeProperties *domePropPtr);
//				void	UpdateButtons(const int widgetBtnIdx, const bool newState);

				void	GetDomeData(void);

				bool		cEnableAutomaticDomeUpdates;
				TYPE_SlittrackerProperties	cSlitProp;

				TYPE_DomeProperties			*cDomePropPtr;

};


#endif // _WINDOWTAB_SLIT_DOME_H_


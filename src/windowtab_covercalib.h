//*****************************************************************************
//#include	"windowtab_covercalib.h"



#ifndef	_WINDOWTAB_COVERCALIB_H_
#define	_WINDOWTAB_COVERCALIB_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif


//*****************************************************************************
enum
{
	kCoverCalib_Title	=	0,
	kCoverCalib_Connected,


	kCoverCalib_Cover_Label,
	kCoverCalib_Cover_State,
	kCoverCalib_Cover_Open,
	kCoverCalib_Cover_Close,
	kCoverCalib_Cover_Halt,
	kCoverCalib_Cover_Outline,


	kCoverCalib_Brightness_Label,
	kCoverCalib_Brightness_Value,
	kCoverCalib_Brightness_Slider,
	kCoverCalib_Brightness_Off,

	kCoverCalib_Minus50,
	kCoverCalib_Minus10,
	kCoverCalib_Minus5,
	kCoverCalib_Minus1,

	kCoverCalib_Plus1,
	kCoverCalib_Plus5,
	kCoverCalib_Plus10,
	kCoverCalib_Plus50,

	kCoverCalib_Brightness_Outline,


	kCoverCalib_LastCmdString,
	kCoverCalib_AlpacaLogo,

	kCoverCalib_AlpacaErrorMsg,
	kCoverCalib_IPaddr,
	kCoverCalib_Readall,
	kCoverCalib_DeviceSelect,


	kCoverCalib_last



};



#define	kCoverCalib_LastOption	kCoverCalib_BtnSingle

//**************************************************************************************
class WindowTabCoverCalib: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabCoverCalib(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabCoverCalib(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual void	ProcessMouseLeftButtonUp(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual	void	UpdateSliderValue(const int widgetIdx, double newSliderValue);

				void	SetBrightness(const int newBrightnessValue);
				void	AdjustBrightness(const int howMuch);
				void	SetCoverCalibPropPtr(TYPE_CoverCalibrationProperties *argPropPtr);

				TYPE_CoverCalibrationProperties	*cCoverCalibrationPropPtr;

				uint32_t	cLastBrightnewssUpdate_Millis;

};


#endif // _WINDOWTAB_COVERCALIB_H_

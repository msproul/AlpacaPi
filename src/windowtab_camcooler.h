//*****************************************************************************
//#include	"windowtab_camcooler.h"

#ifndef	_WINDOWTAB_CAMCOOLER_H_
#define	_WINDOWTAB_CAMCOOLER_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kCamCooler_Title	=	0,

//	kCamCooler_ControllerVersion,

	//-------------------------------
	kCamCooler_CoolerChkBox,
	kCamCooler_Temp_Label,
	kCamCooler_Temperature,
	kCamCooler_Temperature_unused,

	kCamCooler_TargetTempLBL,
	kCamCooler_TargetTemp,
	kCamCooler_TargetTempInput,
	kCamCooler_TargetTempSetBtn,
	kCamCooler_TempGraph,

	kCamCooler_PowerGraph,

	kCamCooler_TempOutline,

	kCamCooler_LastCmdString,
	kCamCooler_AlpacaLogo,
	kCamCooler_AlpacaErrorMsg,
	kCamCooler_IPaddr,
	kCamCooler_Readall,
	kCamCooler_DeviceSelect,


	kCamCooler_last
};


//**************************************************************************************
class WindowTabCamCooler: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabCamCooler(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabCamCooler(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ActivateWindow(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);



				void	SetTempartueDisplayEnable(bool enabled);
				void	SetCCDTargetTemperature(void);
				void	ToggleCooler(void);
				void	SetTemperatureGraphPtrs(double *arrayPtr, int arrayCnt);
};



#endif // _WINDOWTAB_CAMCOOLER_H_





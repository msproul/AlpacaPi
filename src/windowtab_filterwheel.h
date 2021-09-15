//*****************************************************************************
//#include	"windowtab_filterwheel.h"



#ifndef	_WINDOWTAB_FILTERWHEEL_H_
#define	_WINDOWTAB_FILTERWHEEL_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kFilterWheel_Title	=	0,
	kFilterWheel_Connected,

	kFilterWheel_Name,
	kFilterWheel_FilterCircle,

	kFilterWheel_LastCmdString,
	kFilterWheel_AlpacaLogo,
	kFilterWheel_AlpacaDrvrVersion,
	kFilterWheel_IPaddr,
	kFilterWheel_Readall,


	kFilterWheel_last

};

#ifndef kSensorValueCnt
	#define		kSensorValueCnt	12
#endif // kSensorValueCnt


//**************************************************************************************
class WindowTabFilterWheel: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabFilterWheel(	const int	xSize,
										const int	ySize,
										CvScalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabFilterWheel(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);

				void	SetFilterWheelPropPtr(TYPE_FilterWheelProperties *fwProperties);
				void	SetPositonCount(int positionCount);
				void	DrawFilterWheel(IplImage *openCV_Image, TYPE_WIDGET *theWidget);

				int		cPositionCount;
				CvPoint	cFilterCirleCenterPt[kMaxFiltersPerWheel];

				TYPE_FilterWheelProperties	*cFilterWheelPropPtr;
};


#endif // _WINDOWTAB_FILTERWHEEL_H_


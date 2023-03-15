//*****************************************************************************
//#include	"windowtab_alpacaUnit.h"

#ifndef	_WINDOWTAB_ALPACAUNIT_H_
#define	_WINDOWTAB_ALPACAUNIT_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kAlpacaUnit_Title	=	0,


	kAlpacaUnit_CPUtempGraph,

	kAlpacaUnit_Legend_CPU,
	kAlpacaUnit_Legend_Camera,
	kAlpacaUnit_Legend_Focuser,

	kAlpacaUnit_AlpacaDev_01,
	kAlpacaUnit_AlpacaDev_02,
	kAlpacaUnit_AlpacaDev_03,
	kAlpacaUnit_AlpacaDev_04,
	kAlpacaUnit_AlpacaDev_05,
	kAlpacaUnit_AlpacaDev_06,
	kAlpacaUnit_AlpacaDev_07,
	kAlpacaUnit_AlpacaDev_08,
	kAlpacaUnit_AlpacaDev_09,
	kAlpacaUnit_AlpacaDev_10,
	kAlpacaUnit_AlpacaDev_11,
	kAlpacaUnit_AlpacaDev_12,
	kAlpacaUnit_AlpacaDev_13,
	kAlpacaUnit_Outline,

//	kAlpacaUnit_DeviceList,

	kAlpacaUnit_CPUinfo,

	kAlpacaUnit_AlpacaLogo,


	kAlpacaUnit_last
};


//**************************************************************************************
class WindowAlpacaUnit: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowAlpacaUnit(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowAlpacaUnit(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
//		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
//											const int	event,
//											const int	xxx,
//											const int	yyy,
//											const int	flags);

#ifdef _USE_OPENCV_CPP_
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_

		cv::Scalar		cColor_CPU;
		cv::Scalar		cColor_Camera;
		cv::Scalar		cColor_Focuser;


};



#endif // _WINDOWTAB_ALPACAUNIT_H_





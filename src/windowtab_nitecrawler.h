//*****************************************************************************
//#include	"windowtab_nitecrawler.h"


#ifndef	_WINDOWTAB_NITECRAWLER_H_
#define	_WINDOWTAB_NITECRAWLER_H_

#include	"windowtab.h"


//*****************************************************************************
enum
{
	kNiteCrawlerTab_logo	=	0,
	kNiteCrawlerTab_Model,

	kNiteCrawlerTab_focValue,
	kNiteCrawlerTab_rotValue,

	kNiteCrawlerTab_focDesired,
	kNiteCrawlerTab_rotDesired,

	kNiteCrawlerTab_rotLabel,
	kNiteCrawlerTab_focLabel,

	kNiteCrawlerTab_RotatorCompass,
	kNiteCrawlerTab_Temperature,
	kNiteCrawlerTab_Voltage,
	kNiteCrawlerTab_SwitchIN,
	kNiteCrawlerTab_SwitchROT,
	kNiteCrawlerTab_SwitchOUT,
	kNiteCrawlerTab_SwitchLbl,
	kNiteCrawlerTab_StopMotors,

	kNiteCrawlerTab_RotationJog,
	kNiteCrawlerTab_FocusJog,

	//*	these MUST be in this order
	kNiteCrawlerTab_Home,
	kNiteCrawlerTab_Zero,
	kNiteCrawlerTab_Rot_m1,
	kNiteCrawlerTab_Rot_p1,
	kNiteCrawlerTab_Foc_m1,
	kNiteCrawlerTab_Foc_p1,

	kNiteCrawlerTab_Rot_m10,
	kNiteCrawlerTab_Rot_p10,
	kNiteCrawlerTab_Foc_m10,
	kNiteCrawlerTab_Foc_p10,

	kNiteCrawlerTab_Rot_m100,
	kNiteCrawlerTab_Rot_p100,
	kNiteCrawlerTab_Foc_m100,
	kNiteCrawlerTab_Foc_p100,

	kNiteCrawlerTab_Rot_m1000,
	kNiteCrawlerTab_Rot_p1000,
	kNiteCrawlerTab_Foc_m1000,
	kNiteCrawlerTab_Foc_p1000,

	kNiteCrawlerTab_Rot_m5000,
	kNiteCrawlerTab_Rot_p5000,
	kNiteCrawlerTab_Foc_m5000,
	kNiteCrawlerTab_Foc_p5000,
	//*	end of order restriction

	kNiteCrawlerTab_AlpacaLogo,

	kNiteCrawlerTab_IPaddr,
	kNiteCrawlerTab_Readall,
	kNiteCrawlerTab_Connect,


	kNiteCrawlerTab_last
};


//*****************************************************************************
//*	nite crawler and regular focuser use the same windowtab widget indexes
//*****************************************************************************

//#define	kSwitchColorOff	CV_RGB(0x7A, 0x39, 0x02)
#define	kSwitchColorOff	CV_RGB(0x80, 0x00, 0x00)

#define	kLogoHeight	120

//**************************************************************************************
enum
{
	kNCcomMode_Alpaca	=	0,
	kNCcomMode_USB,

	kNCcomMode_last
};

//**************************************************************************************
class WindowTabNitecrawler: public WindowTab
{
	public:
		//
		// Construction
		//
		//		WindowTabNitecrawler(const int xSize, const int ySize);
				WindowTabNitecrawler(	const int	xSize,
									const int	ySize,
									CvScalar	backGrndColor,
									const int	comMode,
									const char	*windowName=NULL);
		virtual	~WindowTabNitecrawler(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(const int buttonIdx);

		int			cComMode;
		int			cRotatorPosition;
		int			cRotatorDesiredPos;
		int			cStepsPerRev;			//*	steps per revolution

};

#endif // _WINDOWTAB_NITECRAWLER_H_


//*****************************************************************************
//#include	"windowtab_ml_single.h"


#ifndef	_WINDOWTAB_MOONLITE_SINGLE_H_
#define	_WINDOWTAB_MOONLITE_SINGLE_H_

#include	"windowtab.h"


//*****************************************************************************
enum
{
	kMLsingle_logo	=	0,
	kMLsingle_Model,
	kMLsingle_Connected,

	kMLsingle_Temperature,



	//*	these MUST be in this order
	kMLsingle_focValue,
	kMLsingle_focDesired,
//	kMLsingle_GOTO,

	kMLsingle_Store,
	kMLsingle_SP01,				//*	Store Point 1
	kMLsingle_SP02,
	kMLsingle_SP03,
	kMLsingle_SP04,
	kMLsingle_SP05,
	kMLsingle_SP06,
	kMLsingle_SP07,
	kMLsingle_SP08,
	kMLsingle_SP09,
//	kMLsingle_SP10,

	kMLsingle_Foc_p5000,
	kMLsingle_Foc_p1000,
	kMLsingle_Foc_p100,
	kMLsingle_Foc_p1,

	kMLsingle_Foc_m1,
	kMLsingle_Foc_m10,
	kMLsingle_Foc_p10,
	kMLsingle_Foc_m100,
	kMLsingle_Foc_m1000,
	kMLsingle_Foc_m5000,
	//*	end of order restriction

	kMLsingle_GOTO_outline,


//*	right hand side
	kMLsingle_CurPosLabel,
	kMLsingle_NewPosLabel,
	kMLsingle_StopMotors,

	kMLsingle_Home,


	kMLsingle_LastCmdString,
	kMLsingle_AlpacaLogo,
	kMLsingle_AlpacaDrvrVersion,
	kMLsingle_IPaddr,
	kMLsingle_Readall,
	kMLsingle_Connect,


	kMLsingle_last
};

//#define	kSwitchColorOff	CV_RGB(0x7A, 0x39, 0x02)
#define	kSwitchColorOff	CV_RGB(0x80, 0x00, 0x00)

#define	kLogoHeight	120

#define	kStorePtCnt	10
//**************************************************************************************
class WindowTabMLsingle: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabMLsingle(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const int	comMode,
									const int	focuserType,
									const char	*windowName=NULL);
		virtual	~WindowTabMLsingle(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(const int buttonIdx);

				int		cComMode;
				int		cFocuserType;

//-				int		cButtonNumValues[10]	=	{5000, 1000, 100, 10, 1, -1, -10, -100, -1000, -5000};
				int		cButtonNumValues[10];
				int		cPreviousBtnClicked;
				int		cStorePoints[kStorePtCnt];

};

#endif // _WINDOWTAB_MOONLITE_SINGLE_H_


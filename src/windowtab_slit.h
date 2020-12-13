//*****************************************************************************
//#include	"windowtab_slit.h"



#ifndef	_WINDOWTAB_SLIT_H_
#define	_WINDOWTAB_SLIT_H_


//*****************************************************************************
enum
{
	kSlitTracker_Title	=	0,

	kSlitTracker_SlitClockFace,
	kSlitTracker_LastUpdate,

	kSlitTracker_RadioBtnSlit,
	kSlitTracker_RadioBtnCalib,
	kSlitTracker_RadioBtnOutline,

	kSlitTracker_LogDataCheckBox,

	kSlitTracker_RemoteAddress,


	kSlitTracker_AlpacaLogo,
	kSlitTracker_AlpacaDrvrVersion,
	kSlitTracker_IPaddr,
	kSlitTracker_Readall,


	kSlitTracker_last
};

//**************************************************************************************
enum
{
	kClockDisplay_Slit	=	0,
	kClockDisplay_Calib
};

#define	kMaxDotColors	6
//**************************************************************************************
class WindowTabSlitTracker: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSlitTracker(	const int	xSize,
										const int	ySize,
										CvScalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabSlitTracker(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgitIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx);

				void	DrawClockFace(IplImage *openCV_Image, TYPE_WIDGET *theWidget);
				void	UpdateClockRadioBtns(void);
				void	ToggleLogData(void);


				int		cClockDisplayMode;

				int			cUpdateColorIdx;
				CvScalar	cDotColor[kMaxDotColors];

};


#endif // _WINDOWTAB_SLIT_H_

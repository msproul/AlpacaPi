//*****************************************************************************
//#include	"windowtab_fitsheader.h"

#ifndef	_WINDOWTAB_FITSHEADER_H_
#define	_WINDOWTAB_FITSHEADER_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

#ifndef _FITS_HELPER_H_
	#include	"fits_helper.h"
#endif

//*****************************************************************************
enum
{
	kFitsHeader_Title	=	0,
	kFitsHeader_FileName,

	kFitsHeader_Line_01,
	kFitsHeader_Line_02,
	kFitsHeader_Line_03,
	kFitsHeader_Line_04,
	kFitsHeader_Line_05,
	kFitsHeader_Line_06,
	kFitsHeader_Line_07,
	kFitsHeader_Line_08,
	kFitsHeader_Line_09,
	kFitsHeader_Line_10,
	kFitsHeader_Line_11,
	kFitsHeader_Line_12,
	kFitsHeader_Line_13,
	kFitsHeader_Line_14,
	kFitsHeader_Line_15,
	kFitsHeader_Line_16,
	kFitsHeader_Line_17,
	kFitsHeader_Line_18,
	kFitsHeader_Line_19,
	kFitsHeader_Line_20,
	kFitsHeader_Line_21,
	kFitsHeader_Line_22,
	kFitsHeader_Line_23,
	kFitsHeader_Line_24,
	kFitsHeader_Line_25,
	kFitsHeader_Line_26,
	kFitsHeader_Line_27,
	kFitsHeader_Line_28,
	kFitsHeader_Line_29,
	kFitsHeader_Line_30,
	kFitsHeader_Line_31,
	kFitsHeader_Line_32,
	kFitsHeader_Line_33,
	kFitsHeader_Line_34,
	kFitsHeader_Line_35,
	kFitsHeader_Line_36,
	kFitsHeader_Line_37,
	kFitsHeader_Line_38,
//	kFitsHeader_Line_39,

	kFitsHeader_Line_Last,

	kFitsHeader_AlpacaLogo,
	kFitsHeader_last
};

//*****************************************************************************
enum
{
	kGraphMode_Raw	=	0,
	kGraphMode_Avg5,
};

#define	kCpuColorCnt	12
//**************************************************************************************
typedef struct	//	TYPE_CPU_SORT
{
	char		cpuName[32];
	double		cpuTemp;
	cv::Scalar	cpuColor;

} TYPE_CPU_SORT;
#define	kMaxCPUs	32

//**************************************************************************************
class WindowTabFITSheader: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabFITSheader(const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabFITSheader(void);

		virtual	void	SetupWindowControls(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual void	ProcessMouseEvent(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
//		virtual void	ProcessMouseLeftButtonDown(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
//		virtual void	ProcessMouseLeftButtonUp(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
//		virtual void	ProcessMouseLeftButtonDragged(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseWheelMoved(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags);
		virtual	void	ProcessLineSelect(int widgetIdx);
				void	SetNewSelectedDevice(int deviceIndex);

		virtual	void	UpdateOnScreenWidgetList(void);

				void	UpdateButtons(void);

				void	SetFitsHeaderData(TYPE_FitsHdrLine *fitsHeaderDataPtr, int fitsHeaderDataCnt);
				TYPE_FitsHdrLine	*cFitsHeaderText;
				int					cFitsLineCount;

};




#endif // _WINDOWTAB_FITSHEADER_H_

//*****************************************************************************
//#include	"windowtab_iplist.h"

#ifndef	_WINDOWTAB_IP_LIST_H_
#define	_WINDOWTAB_IP_LIST_H_

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kIPaddrList_Title	=	0,

	kIPaddrList_DiscoveryThrdStatus,
	kIPaddrList_DiscoveryThrdStop,
	kIPaddrList_DiscoveryThrdReStart,
	kIPaddrList_DiscoveryClear,
	kIPaddrList_DiscoveryReScan,

	kIPaddrList_StatusMsg,
	kIPaddrList_TemperatureGraph,
	kIPaddrList_SortedCPUList,
	kIPaddrList_SortedTempList,
	kIPaddrList_SortedOutline,
	kIPaddrList_TempModeRaw,
	kIPaddrList_TempModeAvg,
	kIPaddrList_ExportCSV,

	kIPaddrList_ClmTitle1,
	kIPaddrList_ClmTitle2,
	kIPaddrList_ClmTitle3,
	kIPaddrList_ClmTitle4,
	kIPaddrList_ClmTitle5,
	kIPaddrList_ClmTitle6,
	kIPaddrList_ClmTitle7,
	kIPaddrList_ClmTitle8,
	kIPaddrList_ClmTitle9,
	kIPaddrList_ClmTitle10,
	kIPaddrList_ClmOutline,

	kIPaddrList_AlpacaDev_01,
	kIPaddrList_AlpacaDev_02,
	kIPaddrList_AlpacaDev_03,
	kIPaddrList_AlpacaDev_04,
	kIPaddrList_AlpacaDev_05,
	kIPaddrList_AlpacaDev_06,
	kIPaddrList_AlpacaDev_07,
	kIPaddrList_AlpacaDev_08,
	kIPaddrList_AlpacaDev_09,
	kIPaddrList_AlpacaDev_10,
	kIPaddrList_AlpacaDev_11,
	kIPaddrList_AlpacaDev_12,
	kIPaddrList_AlpacaDev_13,
	kIPaddrList_AlpacaDev_14,
	kIPaddrList_AlpacaDev_15,
	kIPaddrList_AlpacaDev_16,
	kIPaddrList_AlpacaDev_17,
	kIPaddrList_AlpacaDev_18,
	kIPaddrList_AlpacaDev_19,
	kIPaddrList_AlpacaDev_20,
	kIPaddrList_AlpacaDev_21,
	kIPaddrList_AlpacaDev_22,
//	kIPaddrList_AlpacaDev_23,
//	kIPaddrList_AlpacaDev_24,
//	kIPaddrList_AlpacaDev_25,
//	kIPaddrList_AlpacaDev_26,
//	kIPaddrList_AlpacaDev_27,
//	kIPaddrList_AlpacaDev_28,
//	kIPaddrList_AlpacaDev_29,
//	kIPaddrList_AlpacaDev_30,
//	kIPaddrList_AlpacaDev_31,
//	kIPaddrList_AlpacaDev_32,
//	kIPaddrList_AlpacaDev_33,
//	kIPaddrList_AlpacaDev_34,
//	kIPaddrList_AlpacaDev_35,
//	kIPaddrList_AlpacaDev_36,
//	kIPaddrList_AlpacaDev_37,
//	kIPaddrList_AlpacaDev_38,
//	kIPaddrList_AlpacaDev_39,

	kIPaddrList_AlpacaDev_Last,

	kIPaddrList_AlpacaDev_Total,
	kIPaddrList_AlpacaLogo,
	kIPaddrList_last
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
class WindowTabIPList: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabIPList(const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabIPList(void);

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
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_
		virtual	void	UpdateOnScreenWidgetList(void);

				void	DrawCpuTempGraph(TYPE_WIDGET *theWidget);

				void	ClearIPaddrList(void);

				void	UpdateButtons(void);
				void	ExportCSV(void);

				void	HandleMouseMovedInGraph(	TYPE_WIDGET *theWidget,
													const int	box_XXX,
													const int	box_YYY);

				int				cPrevAlpacaDevCnt;

				cv::Scalar		cCPUcolors[kCpuColorCnt];

				int				cGraphMode;

				TYPE_CPU_SORT	cCPU_NameList[kMaxCPUs];
				int				cValidCPUtempCount;

};




#endif // _WINDOWTAB_IP_LIST_H_

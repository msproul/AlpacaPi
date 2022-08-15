//**************************************************************************************
//#include	"windowtab_cpustats.h"

#ifndef	_WINDOWTAB_CPU_STATS_H_
#define	_WINDOWTAB_CPU_STATS_H_



#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kCpuStats_Title	=	0,

	kCpuStats_ClmTitle1,
	kCpuStats_ClmTitle2,
	kCpuStats_ClmTitle3,
	kCpuStats_ClmTitle4,
	kCpuStats_ClmTitle5,
	kCpuStats_ClmTitle6,

	kCpuStats_Obj_01,
	kCpuStats_Obj_02,
	kCpuStats_Obj_03,
	kCpuStats_Obj_04,
	kCpuStats_Obj_05,
	kCpuStats_Obj_06,
	kCpuStats_Obj_07,
	kCpuStats_Obj_08,
	kCpuStats_Obj_09,
	kCpuStats_Obj_10,
	kCpuStats_Obj_11,
	kCpuStats_Obj_12,
	kCpuStats_Obj_13,
	kCpuStats_Obj_14,
	kCpuStats_Obj_15,
	kCpuStats_Obj_16,
	kCpuStats_Obj_17,
	kCpuStats_Obj_18,
	kCpuStats_Obj_19,
	kCpuStats_Obj_20,
	kCpuStats_Obj_21,
	kCpuStats_Obj_22,
	kCpuStats_Obj_23,
	kCpuStats_Obj_24,
	kCpuStats_Obj_25,
	kCpuStats_Obj_26,
	kCpuStats_Obj_27,
	kCpuStats_Obj_28,
	kCpuStats_Obj_29,
	kCpuStats_Obj_30,
	kCpuStats_Obj_31,
	kCpuStats_Obj_32,
	kCpuStats_Obj_33,
	kCpuStats_Obj_34,

	kCpuStats_Obj_Last	=	kCpuStats_Obj_34,

	kCpuStats_ScrollBar,
	kCpuStats_AlpacaLogo,

	kCpuStats_last
};

//**************************************************************************************
class WindowTabCpuStats: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabCpuStats(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabCpuStats(void);

		virtual	void	SetupWindowControls(const char *argWindowName);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual void	ProcessMouseWheelMoved(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags);
		virtual	void	ActivateWindow(void);

				void	UpdateOnScreenWidgetList(void);

				int				cSortColumn;
				int				cFirstLineIdx;

};


#endif // _WINDOWTAB_CPU_STATS_H_


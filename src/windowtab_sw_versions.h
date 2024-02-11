//*****************************************************************************
//#include	"windowtab_sw_versions.h"

#ifndef	_WINDOWTAB_SW_VERSIONS_H_
#define	_WINDOWTAB_SW_VERSIONS_H_

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kSWversionsList_Title	=	0,


	kSWversionsList_StatusMsg,

	kSWversionsList_ClmTitle1,
	kSWversionsList_ClmTitle2,
	kSWversionsList_ClmTitle3,
	kSWversionsList_ClmTitle4,
	kSWversionsList_ClmTitle5,
	kSWversionsList_ClmTitle6,
	kSWversionsList_ClmTitle7,
	kSWversionsList_ClmTitle8,
	kSWversionsList_ClmTitle9,
	kSWversionsList_ClmTitle10,
	kSWversionsList_ClmOutline,

	kSWversionsList_AlpacaDev_01,
	kSWversionsList_AlpacaDev_02,
	kSWversionsList_AlpacaDev_03,
	kSWversionsList_AlpacaDev_04,
	kSWversionsList_AlpacaDev_05,
	kSWversionsList_AlpacaDev_06,
	kSWversionsList_AlpacaDev_07,
	kSWversionsList_AlpacaDev_08,
	kSWversionsList_AlpacaDev_09,
	kSWversionsList_AlpacaDev_10,
	kSWversionsList_AlpacaDev_11,
	kSWversionsList_AlpacaDev_12,
	kSWversionsList_AlpacaDev_13,
	kSWversionsList_AlpacaDev_14,
	kSWversionsList_AlpacaDev_15,
	kSWversionsList_AlpacaDev_16,
	kSWversionsList_AlpacaDev_17,
	kSWversionsList_AlpacaDev_18,
	kSWversionsList_AlpacaDev_19,
	kSWversionsList_AlpacaDev_20,
	kSWversionsList_AlpacaDev_21,
	kSWversionsList_AlpacaDev_22,
//	kSWversionsList_AlpacaDev_23,
//	kSWversionsList_AlpacaDev_24,
//	kSWversionsList_AlpacaDev_25,
//	kSWversionsList_AlpacaDev_26,
//	kSWversionsList_AlpacaDev_27,
//	kSWversionsList_AlpacaDev_28,
//	kSWversionsList_AlpacaDev_29,
//	kSWversionsList_AlpacaDev_30,
//	kSWversionsList_AlpacaDev_31,
//	kSWversionsList_AlpacaDev_32,
//	kSWversionsList_AlpacaDev_33,
//	kSWversionsList_AlpacaDev_34,
//	kSWversionsList_AlpacaDev_35,
//	kSWversionsList_AlpacaDev_36,
//	kSWversionsList_AlpacaDev_37,
//	kSWversionsList_AlpacaDev_38,
//	kSWversionsList_AlpacaDev_39,

	kSWversionsList_AlpacaDev_Last,

	kSWversionsList_AlpacaDev_Total,
	kSWversionsList_AlpacaLogo,
	kSWversionsList_last
};


//**************************************************************************************
class WindowTabSwVersions: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSwVersions(const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabSwVersions(void);

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

				void	DrawCpuTempGraph(TYPE_WIDGET *theWidget);

				void	ClearIPaddrList(void);

				void	UpdateButtons(void);
				void	ExportCSV(void);

				void	HandleMouseMovedInGraph(	TYPE_WIDGET *theWidget,
													const int	box_XXX,
													const int	box_YYY);

				int				cPrevAlpacaDevCnt;


};




#endif // _WINDOWTAB_SW_VERSIONS_H_

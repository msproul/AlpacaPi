//**************************************************************************************
//#include	"windowtab_aavsolist.h"

#ifndef	_WINDOWTAB_AAVSO_LIST_H_
#define	_WINDOWTAB_AAVSO_LIST_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kAAVSOlist_Title	=	0,

	kAAVSOlist_ClmTitle1,
	kAAVSOlist_ClmTitle2,
	kAAVSOlist_ClmTitle3,
	kAAVSOlist_ClmTitle4,
	kAAVSOlist_ClmTitle5,

	kAAVSOlist_Obj_01,
	kAAVSOlist_Obj_02,
	kAAVSOlist_Obj_03,
	kAAVSOlist_Obj_04,
	kAAVSOlist_Obj_05,
	kAAVSOlist_Obj_06,
	kAAVSOlist_Obj_07,
	kAAVSOlist_Obj_08,
	kAAVSOlist_Obj_09,
	kAAVSOlist_Obj_10,
	kAAVSOlist_Obj_11,
	kAAVSOlist_Obj_12,
	kAAVSOlist_Obj_13,
	kAAVSOlist_Obj_14,
	kAAVSOlist_Obj_15,
	kAAVSOlist_Obj_16,
	kAAVSOlist_Obj_17,
	kAAVSOlist_Obj_18,
	kAAVSOlist_Obj_19,
	kAAVSOlist_Obj_20,
	kAAVSOlist_Obj_21,
	kAAVSOlist_Obj_22,
	kAAVSOlist_Obj_23,
	kAAVSOlist_Obj_24,
	kAAVSOlist_Obj_25,
	kAAVSOlist_Obj_26,
	kAAVSOlist_Obj_27,
	kAAVSOlist_Obj_28,
	kAAVSOlist_Obj_29,
	kAAVSOlist_Obj_30,
	kAAVSOlist_Obj_31,
	kAAVSOlist_Obj_32,
	kAAVSOlist_Obj_33,
	kAAVSOlist_Obj_34,

	kAAVSOlist_Obj_Last	=	kAAVSOlist_Obj_34,

	kAAVSOlist_ScrollBar,

	kAAVSOlist_Obj_Total,

	kAAVSOlist_AlpacaLogo,

	kAAVSOlist_last
};

//**************************************************************************************
class WindowTabAAVSOlist: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabAAVSOlist(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabAAVSOlist(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual void	ProcessMouseWheelMoved(const int widgetIdx, const int event, const int xxx, const int yyy, const int wheelMovement);
		virtual	void	UpdateSliderValue(	const int widgetIdx, double newSliderValue);

				void	UpdateSettings(void);
				void	UpdateOnScreenWidgetList(void);
				void	UpdateSortOrder(void);

				int				cSortColumn;
				int				cFirstLineIdx;
};


#endif // _WINDOWTAB_AAVSO_LIST_H_

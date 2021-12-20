//**************************************************************************************
//#include	"windowtab_starlist.h"

#ifndef	_WINDOWTAB_STAR_LIST_H_
#define	_WINDOWTAB_STAR_LIST_H_

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kStarList_Title	=	0,

	kStarList_ClmTitle1,
	kStarList_ClmTitle2,
	kStarList_ClmTitle3,
	kStarList_ClmTitle4,
	kStarList_ClmTitle5,
	kStarList_ClmTitle6,

	kStarList_Obj_01,
	kStarList_Obj_02,
	kStarList_Obj_03,
	kStarList_Obj_04,
	kStarList_Obj_05,
	kStarList_Obj_06,
	kStarList_Obj_07,
	kStarList_Obj_08,
	kStarList_Obj_09,
	kStarList_Obj_10,
	kStarList_Obj_11,
	kStarList_Obj_12,
	kStarList_Obj_13,
	kStarList_Obj_14,
	kStarList_Obj_15,
	kStarList_Obj_16,
	kStarList_Obj_17,
	kStarList_Obj_18,
	kStarList_Obj_19,
	kStarList_Obj_20,
	kStarList_Obj_21,
	kStarList_Obj_22,
	kStarList_Obj_23,
	kStarList_Obj_24,
	kStarList_Obj_25,
	kStarList_Obj_26,
	kStarList_Obj_27,
	kStarList_Obj_28,
	kStarList_Obj_29,
	kStarList_Obj_30,
	kStarList_Obj_31,
	kStarList_Obj_32,
	kStarList_Obj_33,
	kStarList_Obj_34,

	kStarList_Obj_Last	=	kStarList_Obj_34,

	kStarList_ScrollBar,
	kStarList_Obj_Total,
	kStarList_ExportCSV,
	kStarList_AlpacaLogo,

	kStarList_last
};

//**************************************************************************************
class WindowTabStarList: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabStarList(	const int	xSize,
									const int	ySize,
									CvScalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabStarList(void);

		virtual	void	SetupWindowControls(const char *argWindowName);
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

				void	SetStarDataPointers(TYPE_CelestData *argStarList, int argStarListCount);
				void	SetColumnOneTitle(const char *clmOneTitle);

				int				cSortColumn;
				int				cFirstLineIdx;

				TYPE_CelestData	*cStarListPtr;
				int				cStarListCount;
				int				cDataSource;
};


#endif // _WINDOWTAB_STAR_LIST_H_


//**************************************************************************************
//#include	"windowtab_constList.h"



#ifndef	_WINDOWTAB_CONSTELLATION_LIST_H_
#define	_WINDOWTAB_CONSTELLATION_LIST_H_


#include	"ConstellationData.h"


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kConstList_Title	=	0,

	kConstList_ClmTitle1,
	kConstList_ClmTitle2,
	kConstList_ClmTitle3,
	kConstList_ClmTitle4,
	kConstList_ClmTitle5,
	kConstList_ClmTitle6,

	kConstList_Obj_01,
	kConstList_Obj_02,
	kConstList_Obj_03,
	kConstList_Obj_04,
	kConstList_Obj_05,
	kConstList_Obj_06,
	kConstList_Obj_07,
	kConstList_Obj_08,
	kConstList_Obj_09,
	kConstList_Obj_10,
	kConstList_Obj_11,
	kConstList_Obj_12,
	kConstList_Obj_13,
	kConstList_Obj_14,
	kConstList_Obj_15,
	kConstList_Obj_16,
	kConstList_Obj_17,
	kConstList_Obj_18,
	kConstList_Obj_19,
	kConstList_Obj_20,
	kConstList_Obj_21,
	kConstList_Obj_22,
	kConstList_Obj_23,
	kConstList_Obj_24,
	kConstList_Obj_25,
	kConstList_Obj_26,
	kConstList_Obj_27,
	kConstList_Obj_28,
	kConstList_Obj_29,
	kConstList_Obj_30,
	kConstList_Obj_31,
	kConstList_Obj_32,
	kConstList_Obj_33,
	kConstList_Obj_34,

	kConstList_Obj_Last	=	kConstList_Obj_34,

	kConstList_ScrollBar,
	kConstList_Obj_Total,
//	kConstList_ExportCSV,
	kConstList_AlpacaLogo,

	kConstList_last
};

//**************************************************************************************
class WindowTabConstellationList: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabConstellationList(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName);
		virtual	~WindowTabConstellationList(void);

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
		virtual	void	UpdateSliderValue(	const int widgetIdx, double newSliderValue);

				void	UpdateSettings(void);
				void	UpdateOnScreenWidgetList(void);
				void	UpdateSortOrder(void);

				void	SetStarDataPointers(TYPE_ConstVector *argConstellationPtr, int argConstellationCount);
				void	SetColumnOneTitle(const char *clmOneTitle);


				TYPE_ConstVector		*cConstellationsPtr;
				int						cConstellationCnt;
};


#endif // _WINDOWTAB_CONSTELLATION_LIST_H_


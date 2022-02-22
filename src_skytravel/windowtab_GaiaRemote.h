//**************************************************************************************
//#include	"windowtab_GaiaRemote.h"

#ifndef	_WINDOWTAB_GAIAREMOTE_H_
#define	_WINDOWTAB_GAIAREMOTE_H_

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kGaiaRemoteList_Title	=	0,

	kGaiaRemoteList_ClmTitle1,
	kGaiaRemoteList_ClmTitle2,
	kGaiaRemoteList_ClmTitle3,
	kGaiaRemoteList_ClmTitle4,
	kGaiaRemoteList_ClmTitle5,
	kGaiaRemoteList_ClmTitle6,
	kGaiaRemoteList_ClmTitle7,
	kGaiaRemoteList_ClmTitle8,
	kGaiaRemoteList_ClmTitle9,
	kGaiaRemoteList_ClmTitle10,

	kGaiaRemoteList_Obj_01,
	kGaiaRemoteList_Obj_02,
	kGaiaRemoteList_Obj_03,
	kGaiaRemoteList_Obj_04,
	kGaiaRemoteList_Obj_05,
	kGaiaRemoteList_Obj_06,
	kGaiaRemoteList_Obj_07,
	kGaiaRemoteList_Obj_08,
	kGaiaRemoteList_Obj_09,
	kGaiaRemoteList_Obj_10,
	kGaiaRemoteList_Obj_11,
	kGaiaRemoteList_Obj_12,
	kGaiaRemoteList_Obj_13,
	kGaiaRemoteList_Obj_14,
	kGaiaRemoteList_Obj_15,
	kGaiaRemoteList_Obj_16,
	kGaiaRemoteList_Obj_17,
	kGaiaRemoteList_Obj_18,
	kGaiaRemoteList_Obj_19,
	kGaiaRemoteList_Obj_20,
	kGaiaRemoteList_Obj_21,
	kGaiaRemoteList_Obj_22,
	kGaiaRemoteList_Obj_23,
	kGaiaRemoteList_Obj_24,
	kGaiaRemoteList_Obj_25,
	kGaiaRemoteList_Obj_26,
	kGaiaRemoteList_Obj_27,
	kGaiaRemoteList_Obj_28,
	kGaiaRemoteList_Obj_29,
	kGaiaRemoteList_Obj_30,
	kGaiaRemoteList_Obj_31,
//	kGaiaRemoteList_Obj_32,
//	kGaiaRemoteList_Obj_33,

	kGaiaRemoteList_Obj_Last	=	kGaiaRemoteList_Obj_31,
	kGaiaRemoteList_Average,

	kGaiaRemoteList_ScrollBar,

	kGaiaRemoteList_MsgBox,
	kGaiaRemoteList_Obj_Total,
	kGaiaRemoteList_ErrorCnt,

	kGaiaRemoteList_ClearButton,

	kGaiaRemoteList_AlpacaLogo,

	kGaiaRemoteList_last
};

//**************************************************************************************
class WindowTabGaiaRemote: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabGaiaRemote(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabGaiaRemote(void);

		virtual	void	SetupWindowControls(const char *argWindowName);
		virtual	void	ProcessButtonClick(const int buttonIdx);
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


//?				int				cSortColumn;
				int				cFirstLineIdx;

};


#endif // _WINDOWTAB_GAIAREMOTE_H_


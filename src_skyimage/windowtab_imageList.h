//*****************************************************************************
//#include	"windowtab_imageList.h"

#ifndef	_WINDOWTAB_IMAGELIST_H_
#define	_WINDOWTAB_IMAGELIST_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kImageList_Title	=	0,

	kImageList_ClmTitle1,
	kImageList_ClmTitle2,
	kImageList_ClmTitle3,
	kImageList_ClmTitle4,
	kImageList_ClmTitle5,
	kImageList_ClmTitle6,

	kImageList_AlpacaDev_01,
	kImageList_AlpacaDev_02,
	kImageList_AlpacaDev_03,
	kImageList_AlpacaDev_04,
	kImageList_AlpacaDev_05,
	kImageList_AlpacaDev_06,
	kImageList_AlpacaDev_07,
	kImageList_AlpacaDev_08,
	kImageList_AlpacaDev_09,
	kImageList_AlpacaDev_10,
	kImageList_AlpacaDev_11,
	kImageList_AlpacaDev_12,
	kImageList_AlpacaDev_13,
	kImageList_AlpacaDev_14,
	kImageList_AlpacaDev_15,
	kImageList_AlpacaDev_16,
	kImageList_AlpacaDev_17,
	kImageList_AlpacaDev_18,
	kImageList_AlpacaDev_19,
	kImageList_AlpacaDev_20,
	kImageList_AlpacaDev_21,
	kImageList_AlpacaDev_22,
	kImageList_AlpacaDev_23,
	kImageList_AlpacaDev_24,
	kImageList_AlpacaDev_25,
	kImageList_AlpacaDev_26,
	kImageList_AlpacaDev_27,
	kImageList_AlpacaDev_28,
	kImageList_AlpacaDev_29,
	kImageList_AlpacaDev_30,
	kImageList_AlpacaDev_31,
	kImageList_AlpacaDev_32,
	kImageList_AlpacaDev_33,
	kImageList_AlpacaDev_34,
	kImageList_AlpacaDev_35,
	kImageList_AlpacaDev_36,
	kImageList_AlpacaDev_37,
	kImageList_AlpacaDev_38,
	kImageList_AlpacaDev_39,
	kImageList_AlpacaDev_40,
//	kImageList_AlpacaDev_41,
//	kImageList_AlpacaDev_42,

	kImageList_AlpacaDev_Last	=	kImageList_AlpacaDev_40,


	kImageList_AlpacaDev_Total,
	kImageList_Btn_CloseAll,
	kImageList_Btn_Help,
	kImageList_ScrollBar,

	kImageList_AlpacaLogo,

	kImageList_last
};


//**************************************************************************************
class WindowTabImageList: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabImageList(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabImageList(void);
		virtual	void	RunWindowBackgroundTasks(void);

		virtual	void	SetupWindowControls(void);
		virtual void	HandleKeyDown(const int keyPressed);
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
		virtual	void	ProcessLineSelect(int widgetIdx);
				void	SetNewSelectedDevice(int imageIndex);
		virtual	void	UpdateSliderValue(	const int widgetIdx, double newSliderValue);
		virtual	void	UpdateOnScreenWidgetList(void);

				void	UpdateSortOrder(void);

				int		cImageListCount;


};




#endif // _WINDOWTAB_IMAGELIST_H_

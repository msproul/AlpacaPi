//*****************************************************************************
//#include	"windowtab_alpacalist.h"

#ifndef	_WINDOWTAB_ALPACALIST_H_
#define	_WINDOWTAB_ALPACALIST_H_

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif // _DISCOVERY_LIB_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H

//*****************************************************************************
enum
{
	kAlpacaList_Title	=	0,

	kAlpacaList_ClmTitle1,
	kAlpacaList_ClmTitle2,
	kAlpacaList_ClmTitle3,
	kAlpacaList_ClmTitle4,
	kAlpacaList_ClmTitle5,

	kAlpacaList_AlpacaDev_01,
	kAlpacaList_AlpacaDev_02,
	kAlpacaList_AlpacaDev_03,
	kAlpacaList_AlpacaDev_04,
	kAlpacaList_AlpacaDev_05,
	kAlpacaList_AlpacaDev_06,
	kAlpacaList_AlpacaDev_07,
	kAlpacaList_AlpacaDev_08,
	kAlpacaList_AlpacaDev_09,
	kAlpacaList_AlpacaDev_10,
	kAlpacaList_AlpacaDev_11,
	kAlpacaList_AlpacaDev_12,
	kAlpacaList_AlpacaDev_13,
	kAlpacaList_AlpacaDev_14,
	kAlpacaList_AlpacaDev_15,
	kAlpacaList_AlpacaDev_16,
	kAlpacaList_AlpacaDev_17,
	kAlpacaList_AlpacaDev_18,
	kAlpacaList_AlpacaDev_19,
	kAlpacaList_AlpacaDev_20,
	kAlpacaList_AlpacaDev_21,
	kAlpacaList_AlpacaDev_22,
	kAlpacaList_AlpacaDev_23,
	kAlpacaList_AlpacaDev_24,
	kAlpacaList_AlpacaDev_25,
	kAlpacaList_AlpacaDev_26,
	kAlpacaList_AlpacaDev_27,
	kAlpacaList_AlpacaDev_28,
	kAlpacaList_AlpacaDev_29,
	kAlpacaList_AlpacaDev_30,
	kAlpacaList_AlpacaDev_31,
	kAlpacaList_AlpacaDev_32,
	kAlpacaList_AlpacaDev_33,
	kAlpacaList_AlpacaDev_34,
	kAlpacaList_AlpacaDev_35,
	kAlpacaList_AlpacaDev_36,
	kAlpacaList_AlpacaDev_37,
	kAlpacaList_AlpacaDev_38,
	kAlpacaList_AlpacaDev_39,
	kAlpacaList_AlpacaDev_40,
	kAlpacaList_AlpacaDev_41,
	kAlpacaList_AlpacaDev_42,

	kAlpacaList_AlpacaDev_Last	=	kAlpacaList_AlpacaDev_42,


	kAlpacaList_AlpacaDev_Total,
	kAlpacaList_ChkBx_IncManagment,
	kAlpacaList_Btn_Refresh,
	kAlpacaList_Btn_CloseAll,

	kAlpacaList_AlpacaLogo,

	kAlpacaList_last
};

#define	kMaxDeviceCnt	50

//**************************************************************************************
class WindowTabAlpacaList: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabAlpacaList(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabAlpacaList(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);

				void	ClearRemoteDeviceList(void);
				void	UpdateRemoteDeviceList(void);
				void	UpdateOnScreenWidgetList(void);
				void	UpdateSortOrder(void);

				TYPE_REMOTE_DEV	cRemoteDeviceList[kMaxDeviceCnt];
				int				cAlpacaDevCnt;
				int				cPrevAlpacaDevCnt;
				int				cSortColumn;

				bool			cIncludeManagment;

};




#endif // _WINDOWTAB_ALPACALIST_H_

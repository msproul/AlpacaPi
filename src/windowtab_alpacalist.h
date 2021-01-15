//*****************************************************************************
//#include	"windowtab_alpacalist.h"

#ifndef	_WINDOWTAB_ALPACALIST_H_
#define	_WINDOWTAB_ALPACALIST_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H

//*****************************************************************************
enum
{
	kAlpacaList_Title	=	0,

//	kAlpacaList_ControllerVersion,

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

	kAlpacaList_AlpacaDev_Last	=	kAlpacaList_AlpacaDev_37,

//	kAlpacaList_AlpacaDev_OutLine,

	kAlpacaList_AlpacaDev_Total,

	kAlpacaList_AlpacaLogo,

//	kAlpacaList_AlpacaDrvrVersion,

	kAlpacaList_IPaddr,
	kAlpacaList_Readall,


	kAlpacaList_last
};


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
				void	UpdateList(void);
				int		cAlpacaDevCnt;
				int		cPrevAlpacaDevCnt;

};




#endif // _WINDOWTAB_ALPACALIST_H_

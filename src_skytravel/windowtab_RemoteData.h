//*****************************************************************************
//#include	"windowtab_RemoteData.h"

#ifndef	_WINDOWTAB_REMOTE_DATA_H_
#define	_WINDOWTAB_REMOTE_DATA_H_




#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kRemoteData_Title	=	0,

	kRemoteData_stsci_fits_checkbox,
	kRemoteData_stsci_fits_Count,
	kRemoteData_stsci_fits_LastCmd,

	kRemoteData_stsci_gif_checkbox,
	kRemoteData_stsci_gif_Count,
	kRemoteData_stsci_gif_LastCmd,

	kRemoteData_SDSS_checkbox,
	kRemoteData_SDSS_Count,
	kRemoteData_SDSS_LastCmd,

	kRemoteData_ImageOptOutline,

	kRemoteData_StatusMsg,

#ifdef 	_ENABLE_REMOTE_GAIA_

	kRemoteData_EnableRemoteGAIA,
	kRemoteData_SQLserverTxt,
	kRemoteData_SQLserverValue,

	kRemoteData_SQLportTxt,
	kRemoteData_SQLportValue,

	kRemoteData_SQLusernameTxt,
	kRemoteData_SQLusernameValue,

	kRemoteData_EnableSQLlogging,
	kRemoteData_OpenSQLWindowBtn,

	kRemoteData_GAIAoutline,

#endif // _ENABLE_REMOTE_GAIA_


	kRemoteData_AlpacaLogo,


	kRemoteData_last
};


//**************************************************************************************
class WindowTabRemoteData: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabRemoteData(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabRemoteData(void);

		virtual	void	RunBackgroundTasks(void);
		virtual	void	SetupWindowControls(void);
		virtual	void	ActivateWindow(void);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);

				void	UpdateSettings(void);
		uint32_t			cLastRemoteImageUpdate_ms;

};



#endif // _WINDOWTAB_REMOTE_DATA_H_






//*****************************************************************************
//#include	"controller_startup.h"

#ifdef __cplusplus
	extern "C" {
#endif

void	CreateStartupScreen(void);
void	CloseStartupScreen(void);
int		SetStartupText(const char *startupMsg);
void	SetStartupTextStatus(const int widgetIdx, const char *statusText);
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif

#ifndef _WINDOWTAB_STARTUP_H_
	#include	"windowtab_startup.h"
#endif

//#ifndef	_WINDOWTAB_ABOUT_H_
//	#include	"windowtab_about.h"
//#endif // _WINDOWTAB_ABOUT_H_


//**************************************************************************************
class ControllerStartup: public Controller
{
	public:
		//
		// Construction
		//
				ControllerStartup(void);
		virtual	~ControllerStartup(void);

		virtual	void	SetupWindowControls(void);
				int		SetStartupText(const char *startupMsg);
				void	SetStartupTextStatus(const int widgetIdx, const char *statusText);

				int		cCurrentMsgIdx;

			//*	tab information
				WindowTabStartup	*cStartupTabObjPtr;
//				WindowTabAbout		*cAboutBoxTabObjPtr;

};


#endif // __cplusplus

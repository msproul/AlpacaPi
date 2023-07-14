//*****************************************************************************
//#include	"controller_startup.h"


#ifndef _HELPER_FUNCTIONS_H_
	#include	"helper_functions.h"
#endif


#ifdef __cplusplus
	extern "C" {
#endif
//*	startup screen routines
void		CreateStartupScreen(void);
void		CloseStartupScreen(void);
int			SetStartupText(const char *startupMsg);
void		SetStartupTextStatus(const int widgetIdx, const char *statusText);
void		SetStartupUpdate(void);


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

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif


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
		virtual	void	RunBackgroundTasks(const char *callingFunction, bool enableDebug);
				int		SetStartupText(const char *startupMsg);
				void	SetStartupTextStatus(const int widgetIdx, const char *statusText);

				int		cCurrentMsgIdx;
				int		cBackGroundTaskCntr;
			//*	tab information
				WindowTabStartup	*cStartupTabObjPtr;
				WindowTabAbout		*cAboutBoxTabObjPtr;

};


#endif // __cplusplus

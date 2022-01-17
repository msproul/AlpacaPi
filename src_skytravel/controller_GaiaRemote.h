//*****************************************************************************
//#include	"controller_GaiaRemote.h"


#include	"controller.h"



#include	"windowtab_GaiaRemote.h"

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_



void	CreateGaiaRemoteListWindow(void);

//**************************************************************************************
class ControllerGaiaRemote: public Controller
{
	public:
		//
		// Construction
		//
				ControllerGaiaRemote(void);


		virtual	~ControllerGaiaRemote(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(bool enableDebug=false);
		virtual	bool	AlpacaGetStartupData(void);
		virtual	void	UpdateCommonProperties(void);

				//*	tab information
				WindowTabGaiaRemote		*cGaiaListTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

};


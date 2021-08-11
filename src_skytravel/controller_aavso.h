//*****************************************************************************
//#include	"controller_aavso.h"


#include	"controller.h"

#include	"windowtab_aavsolist.h"
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_

void	CreateAAVSOlistWindow(void);

//**************************************************************************************
class ControllerAAVSOlist: public Controller
{
	public:
		//
		// Construction
		//
				ControllerAAVSOlist(	const char			*argWindowName);


		virtual	~ControllerAAVSOlist(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(void);
		virtual	bool	AlpacaGetStartupData(void);
		virtual	void	UpdateCommonProperties(void);

				//*	tab information
				WindowTabAAVSOlist		*cAAVSOlistTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;
};


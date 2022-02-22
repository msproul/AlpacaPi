//*****************************************************************************
//#include	"controller_constList.h"


#include	"controller.h"

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_

#ifndef _CONSTELLATION_DATA_H_
	#include	"ConstellationData.h"
#endif // _CONSTELLATION_DATA_H_

#include	"windowtab_constList.h"
#include	"windowtab_starlist.h"
#include	"windowtab_about.h"



void	CreateConstellationListWindow(	TYPE_ConstVector	*constellationsPtr,
										const int			constellationCnt);

//**************************************************************************************
class ControllerConstellationList: public Controller
{
	public:
		//
		// Construction
		//
				ControllerConstellationList(	TYPE_ConstVector	*argConstellations,
												int					argConstellationCnt);


		virtual	~ControllerConstellationList(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
//		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
//		virtual	bool	AlpacaGetStartupData(void);
		virtual	void	UpdateCommonProperties(void);

				//*	tab information
				WindowTabConstellationList	*cConstListTabObjPtr;
				WindowTabAbout				*cAboutBoxTabObjPtr;

				TYPE_ConstVector		*cConstellationsPtr;
				int						cConstellationCnt;
};


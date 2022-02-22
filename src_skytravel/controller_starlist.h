//*****************************************************************************
//#include	"controller_starlist.h"


#include	"controller.h"

#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_



#include	"windowtab_starlist.h"
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_



void	CreateStarlistWindow(	const char		*windowName,
								TYPE_CelestData	*starListPtr,
								const int		starListCount,
								const char		*clmOneTitle = NULL);

//**************************************************************************************
class ControllerStarlist: public Controller
{
	public:
		//
		// Construction
		//
				ControllerStarlist(	const char		*argWindowName,
									TYPE_CelestData	*argStarList,
									int				argStarListCountt,
									const char		*clmOneTitle = NULL);


		virtual	~ControllerStarlist(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
		virtual	bool	AlpacaGetStartupData(void);
		virtual	void	UpdateCommonProperties(void);

				//*	tab information
				WindowTabStarList		*cStarListTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				TYPE_CelestData	*starListPtr;
				int				starListCount;
};


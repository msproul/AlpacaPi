//*****************************************************************************
//#include	"windowtab_moon.h"

#ifndef	_WINDOWTAB_MOON_H_
#define	_WINDOWTAB_MOON_H_




#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kMoon_Title	=	0,

	kMoon_AgeLbl,
	kMoon_Age,

	kMoon_PhaseLbl,
	kMoon_Phase,

	kMoon_IlluminationLbl,
	kMoon_Illumination,

	kMoon_AlpacaLogo,
//	kMoon_AlpacaDrvrVersion,
//	kMoon_IPaddr,
//	kMoon_Readall,


	kMoon_last
};


//**************************************************************************************
class WindowTabMoon: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabMoon(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabMoon(void);
		virtual	void	RunBackgroundTasks(void);

		virtual	void	SetupWindowControls(void);

		bool				cFirstRead;
		uint32_t			cLastUpdateTime_ms;

};



#endif // _WINDOWTAB_MOON_H_






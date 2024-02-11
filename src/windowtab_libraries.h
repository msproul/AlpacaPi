//*****************************************************************************
//#include	"windowtab_libraries.h"


#ifndef	_WINDOWTAB_LIBRARIES_H_
#define	_WINDOWTAB_LIBRARIES_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kLibraries_Title	=	0,

	kLibraries_DescriptionBox,
	kLibraries_LibrariesBox,

	kLibraries_AlpacaLogo,
	kLibraries_IPaddr,

	kLibraries_last
};


//**************************************************************************************
class WindowTabLibraries: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabLibraries(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabLibraries(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
//		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
//											const int	event,
//											const int	xxx,
//											const int	yyy,
//											const int	flags);



};



#endif // _WINDOWTAB_LIBRARIES_H_





//*****************************************************************************
//#include	"windowtab_filelist.h"



#ifndef	_WINDOWTAB_FILELIST_H_
#define	_WINDOWTAB_FILELIST_H_

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif // _CONTROLLER_H_

//*****************************************************************************
enum
{
	kFileList_Title	=	0,
	kFileList_Refresh,
	kFileList_CountText,
	kFileList_ScrollBar,
	kFileList_FistEntry,



	kFileList_LastEntry	=	30,

	kFileList_Outline,

	kFileList_AlpacaLogo,
	kFileList_AlpacaDrvrVersion,

	kFileList_IPaddr,
	kFileList_Readall,


	kFileList_last
};

#define	kMaxLogEnries	300

//**************************************************************************************
class WindowTabFileList: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabFileList(	const int	xSize,
									const int	ySize,
									CvScalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabFileList(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(const int buttonIdx);

				void	AlpacaGetFileList(void);


};


#endif // _WINDOWTAB_FILELIST_H_


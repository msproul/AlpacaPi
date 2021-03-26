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

	kMoon_Btn_Prev,
	kMoon_Btn_Today,
	kMoon_Btn_Next,
	kMoon_Btn_Reload,

	kMoon_FitsHeader,

	kMoon_Image,
	kMoon_MoonImgOutline,
	kMoon_FileName,


	kMoon_last
};

//**************************************************************************************
typedef struct
{
	char	fitsFileName[64];
	double	moonAge;

} TYPE_MoonFileEntry;

#define	kMaxMoonFiles	32

#define	kMaxFitsHeaderLen	10000

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
		virtual	void	ProcessButtonClick(const int buttonIdx);

				void	ReadMoonDirectory(void);
				void	NextImage(void);
				void	PrevImage(void);
				void	ReadMoonImage(const char *moonFileName);
				void	ReadFitsHeader(const char *fitsFilePath);

		bool			cFirstRead;
		uint32_t		cLastUpdateTime_ms;

		IplImage		*cMoonImage;
		int				cMaxMoonImgSize;

		TYPE_MoonFileEntry	cMoonFileList[kMaxMoonFiles];
		int					cMoonFileCnt;
		int					cCurrentMoonIdx;

		char				cFitsHeaderBuffer[kMaxFitsHeaderLen];
};



#endif // _WINDOWTAB_MOON_H_






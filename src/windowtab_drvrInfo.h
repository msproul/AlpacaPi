//*****************************************************************************
//#include	"windowtab_drvrInfo.h"

#ifndef	_WINDOWTAB_DRIVER_INFO_H_
#define	_WINDOWTAB_DRIVER_INFO_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kDriverInfo_Title	=	0,

	kDriverInfo_SubTitle,

	kDriverInfo_Name,
	kDriverInfo_Name_Lbl,
	kDriverInfo_Description,
	kDriverInfo_Description_Lbl,
	kDriverInfo_DriverInfo,
	kDriverInfo_DriverInfo_Lbl,
	kDriverInfo_DriverVersion,
	kDriverInfo_DriverVersion_Lbl,

	kDriverInfo_InterfaceVersion,
	kDriverInfo_InterfaceVersion_Lbl,

	kDriverInfo_AlpacaLogo,
	kDriverInfo_AlpacaDrvrVersion,
	kDriverInfo_IPaddr,
	kDriverInfo_Readall,






	kDriverInfo_last
};

#define	kFirstBoxID	kDriverInfo_Name
#define	kLastBoxID	kDriverInfo_InterfaceVersion

//**************************************************************************************
class WindowTabDriverInfo: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabDriverInfo(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabDriverInfo(void);

		virtual	void	SetupWindowControls(void);

};



#endif // _WINDOWTAB_DRIVER_INFO_H_





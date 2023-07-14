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

	//*	label, name, & outline for each item
	kDriverInfo_Name_Lbl,
	kDriverInfo_Name,
	kDriverInfo_Name_Outline,

	kDriverInfo_Description_Lbl,
	kDriverInfo_Description,
	kDriverInfo_Description_Outline,

	kDriverInfo_DriverInfo_Lbl,
	kDriverInfo_DriverInfo,
	kDriverInfo_DriverInfo_Outline,

	kDriverInfo_DriverVersion_Lbl,
	kDriverInfo_DriverVersion,
	kDriverInfo_DriverVersion_Outline,

	kDriverInfo_InterfaceVersion_Lbl,
	kDriverInfo_InterfaceVersion,
	kDriverInfo_InterfaceVersion_Outline,

	kDriverInfo_IPaddr,
	kDriverInfo_Readall,
	kDriverInfo_DeviceState,

	kDriverInfo_LaunchWeb,




	kDriverInfo_last
};

#define	kFirstBoxID	kDriverInfo_Name_Lbl
#define	kLastBoxID	kDriverInfo_InterfaceVersion_Lbl

//**************************************************************************************
class WindowTabDriverInfo: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabDriverInfo(const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabDriverInfo(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);

};



#endif // _WINDOWTAB_DRIVER_INFO_H_





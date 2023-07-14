//*****************************************************************************
//#include	"windowtab_auxmotor.h"

#ifndef	_WINDOWTAB_AUXMOTOR_H_
#define	_WINDOWTAB_AUXMOTOR_H_

#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kAuxMotorBox_Title	=	0,
	kAuxMotorBox_Position,
	kAuxMotorBox_Label,
	kAuxMotorBox_Button1,
	kAuxMotorBox_DesiredPos,
	kAuxMotorBox_Button2,

	kAuxMotorBox_m1,
	kAuxMotorBox_p1,

	kAuxMotorBox_m10,
	kAuxMotorBox_p10,

	kAuxMotorBox_m100,
	kAuxMotorBox_p100,

	kAuxMotorBox_m1000,
	kAuxMotorBox_p1000,

	kAuxMotorBox_m5000,
	kAuxMotorBox_p5000,

	kAuxMotorBox_Switch1,
	kAuxMotorBox_Switch2,
	kAuxMotorBox_SwitchLbl,

	kAuxMotorBox_LastCmdString,
	kAuxMotorBox_AlpacaLogo,
	kAuxMotorBox_AlpacaErrorMsg,
	kAuxMotorBox_IPaddr,
	kAuxMotorBox_Readall,
	kAuxMotorBox_DeviceState,

	kAuxMotorBox_last
};


//**************************************************************************************
class WindowTabAuxMotor: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabAuxMotor(const int xSize, const int ySize, cv::Scalar backGrndColor);
		virtual	~WindowTabAuxMotor(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
				void	DisableAuxMotorButons(void);


};



#endif // _WINDOWTAB_AUXMOTOR_H_

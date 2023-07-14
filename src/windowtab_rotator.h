//*****************************************************************************
//*	Oct  8,	2022	<MLS> Created windowtab_rotator.cpp
//*****************************************************************************
//#include	"windowtab_rotator.h"



#ifndef	_WINDOWTAB_ROTATOR_H_
#define	_WINDOWTAB_ROTATOR_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif


#define	kBoxesPerSwitch	5
//*****************************************************************************
enum
{
	kRotatorCtrl_Title	=	0,

	kRotatorCtrl_Connected,

//	bool	CanReverse;			//*	Indicates whether the Rotator supports the Reverse method.
//	bool	IsMoving;			//*	Indicates whether the rotator is currently moving
//	double	MechanicalPosition;	//*	This returns the raw mechanical position of the rotator in degrees.
//	double	Position;			//*	Current instantaneous Rotator position, allowing for any sync offset, in degrees.
//	bool	Reverse;			//*	Sets or Returns the rotator’s Reverse state.
//	double	StepSize;			//*	The minimum StepSize, in degrees.
//	double	TargetPosition;		//*	The destination position angle for Move() and MoveAbsolute().

	kRotatorCtrl_RotorCompass,

	kRotatorCtrl_Zero,
	kRotatorCtrl_Rot_m1,
	kRotatorCtrl_Rot_p1,

	kRotatorCtrl_Rot_m5,
	kRotatorCtrl_Rot_p5,

	kRotatorCtrl_Rot_m10,
	kRotatorCtrl_Rot_p10,

	kRotatorCtrl_Rot_m45,
	kRotatorCtrl_Rot_p45,

	kRotatorCtrl_Rot_m90,
	kRotatorCtrl_Rot_p90,

	kRotatorCtrl_Halt,
	kRotatorCtrl_BtnOutline,

	kRotatorCtrl_IsMoving_Lbl,
	kRotatorCtrl_IsMoving_Value,

	kRotatorCtrl_MechPos_Lbl,
	kRotatorCtrl_MechPos_Value,

	kRotatorCtrl_Position_Lbl,
	kRotatorCtrl_Position_Value,

	kRotatorCtrl_TargetPosition_Lbl,
	kRotatorCtrl_TargetPosition_Value,

	kRotatorCtrl_StepSize_Lbl,
	kRotatorCtrl_StepSize_Value,

	kRotatorCtrl_PropOutline,



	kRotatorCtrl_LastCmdString,
	kRotatorCtrl_AlpacaLogo,
	kRotatorCtrl_AlpacaErrorMsg,
	kRotatorCtrl_IPaddr,
	kRotatorCtrl_Readall,
	kRotatorCtrl_DeviceState,


	kRotatorCtrl_last
};

#define	kMaxSwitchControlBoxes	12

//**************************************************************************************
class WindowTabRotator: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabRotator(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabRotator(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
	#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
	#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
	#endif // _USE_OPENCV_CPP_
				void	DrawRotatorCompass(TYPE_WIDGET *theWidget);

				void	UpdateProperties_Rotator(TYPE_RotatorProperties *rotatorProp);

				bool	cFirstRead;
				double	cRotatorPosition_deg;
				double	cTargetPosition_deg;


};


#endif // _WINDOWTAB_ROTATOR_H_

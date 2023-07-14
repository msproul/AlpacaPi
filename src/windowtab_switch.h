//*****************************************************************************
//#include	"windowtab_switch.h"



#ifndef	_WINDOWTAB_SWITCH_H_
#define	_WINDOWTAB_SWITCH_H_

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif


#define	kBoxesPerSwitch	5
//*****************************************************************************
enum
{
	kSwitchBox_Title	=	0,

	kSwitchBox_Connected,

	kSwitchBox_Switch01,
	kSwitchBox_Name01,
	kSwitchBox_Description01,
	kSwitchBox_Value01,
	kSwitchBox_State01,

	kSwitchBox_Switch02,
	kSwitchBox_Name02,
	kSwitchBox_Description02,
	kSwitchBox_Value02,
	kSwitchBox_State02,

	kSwitchBox_Switch03,
	kSwitchBox_Name03,
	kSwitchBox_Description03,
	kSwitchBox_Value03,
	kSwitchBox_State03,

	kSwitchBox_Switch04,
	kSwitchBox_Name04,
	kSwitchBox_Description04,
	kSwitchBox_Value04,
	kSwitchBox_State04,

	kSwitchBox_Switch05,
	kSwitchBox_Name05,
	kSwitchBox_Description05,
	kSwitchBox_Value05,
	kSwitchBox_State05,

	kSwitchBox_Switch06,
	kSwitchBox_Name06,
	kSwitchBox_Description06,
	kSwitchBox_Value06,
	kSwitchBox_State06,

	kSwitchBox_Switch07,
	kSwitchBox_Name07,
	kSwitchBox_Description07,
	kSwitchBox_Value07,
	kSwitchBox_State07,

	kSwitchBox_Switch08,
	kSwitchBox_Name08,
	kSwitchBox_Description08,
	kSwitchBox_Value08,
	kSwitchBox_State08,

	kSwitchBox_Switch09,
	kSwitchBox_Name09,
	kSwitchBox_Description09,
	kSwitchBox_Value09,
	kSwitchBox_State09,

	kSwitchBox_Switch10,
	kSwitchBox_Name10,
	kSwitchBox_Description10,
	kSwitchBox_Value10,
	kSwitchBox_State10,

	kSwitchBox_Switch11,
	kSwitchBox_Name11,
	kSwitchBox_Description11,
	kSwitchBox_Value11,
	kSwitchBox_State11,

	kSwitchBox_Switch12,
	kSwitchBox_Name12,
	kSwitchBox_Description12,
	kSwitchBox_Value12,
	kSwitchBox_State12,

	//*	future expansion, not currently displayed
	kSwitchBox_Switch13,
	kSwitchBox_Name13,
	kSwitchBox_Description13,
	kSwitchBox_Value13,
	kSwitchBox_State13,

	kSwitchBox_Switch14,
	kSwitchBox_Name14,
	kSwitchBox_Description14,
	kSwitchBox_Value14,
	kSwitchBox_State14,

	kSwitchBox_Switch15,
	kSwitchBox_Name15,
	kSwitchBox_Description15,
	kSwitchBox_Value15,
	kSwitchBox_State15,

	kSwitchBox_Switch16,
	kSwitchBox_Name16,
	kSwitchBox_Description16,
	kSwitchBox_Value16,
	kSwitchBox_State16,


	kSwitchBox_AllOff,


	kSwitchBox_LastCmdString,
	kSwitchBox_AlpacaLogo,

	kSwitchBox_AlpacaErrorMsg,
	kSwitchBox_IPaddr,
	kSwitchBox_Readall,
	kSwitchBox_DeviceState,

	kSwitchBox_last
};

#define	kMaxSwitchControlBoxes	12

//**************************************************************************************
class WindowTabSwitch: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSwitch(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabSwitch(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);

				void	ToggleSwitchState(const int switchNum);
				void	TurnAllSwitchesOff(void);

				void	SetActiveSwitchCount(const int validSwitches);

				bool	cFirstRead;

				int		cValidSwitchCount;

};


#endif // _WINDOWTAB_SWITCH_H_

//*****************************************************************************
//#include	"controller_switch.h"


#include	"controller.h"

#include	"windowtab_switch.h"
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_


//**************************************************************************************
typedef struct
{
	char	switchName[32];
	char	switchDescription[128];
	bool	switchState;
	double	minswitchvalue;
	double	maxswitchvalue;
	double	switchvalue;

} TYPE_SWITCH_INFO;
#define	kMaxSwitches	32

//**************************************************************************************
class ControllerSwitch: public Controller
{
	public:
		//
		// Construction
		//
				ControllerSwitch(	const char			*argWindowName,
									struct sockaddr_in	*deviceAddress = NULL,
									const int			port = 0,
									const int			deviceNum = 0);


		virtual	~ControllerSwitch(void);

	//-			void	ControllerSwitchInit(void);

		virtual	void	SetupWindowControls(void);
	//	virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(void);
	//	virtual	void	DrawGraphWidget(const int widgitIdx);
				bool	AlpacaGetStartupData(void);
				bool	AlpacaGetStartupData_OneAAT(void);
				bool	AlpacaGetStatus(void);
		virtual	void	AlpacaProcessReadAll(const char *keywordString, const char *valueString);
				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time
				void	ToggleSwitchState(const int switchNum);

			//*	tab information
				WindowTabSwitch		*cSwitchTabObjPtr;
				WindowTabAbout		*cAboutBoxTabObjPtr;
				int					cMaxSwitch;

				TYPE_SWITCH_INFO	cSwitchInfo[kMaxSwitches];
};


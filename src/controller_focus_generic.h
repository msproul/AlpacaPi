//**************************************************************************************
//#include	"controller_focus_generic.h"

#ifndef _CONTROLLER_FOCUS_H_
	#include	"controller_focus.h"
#endif

#ifndef	_WINDOWTAB_MOONLITE_SINGLE_H_
	#include	"windowtab_ml_single.h"
#endif // _WINDOWTAB_MOONLITE_SINGLE_H_

#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

//**************************************************************************************
class ControllerFocusGeneric: public ControllerFocus
{
	public:
		//
		// Construction
		//
				ControllerFocusGeneric(	const char			*argWindowName,
										struct sockaddr_in	*deviceAddress,
										const int			port,
										const int			deviceNum,
										const int			focuserType = kFocuserType_Other);


		virtual	~ControllerFocusGeneric(void);


		virtual	void	CreateWindowTabs(void);


		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateFocuserPosition(	const int newFocuserPosition);
		virtual	void	UpdateTemperature(		const double newTemperature);
//		virtual	void	UpdateVoltage(			const double newVoltage);

		virtual	void	UpdateWindowTabs_Everything(void);
		virtual	void	UpdateWindowTabs_ReadAll(		bool hasReadAll);
//		virtual	void	UpdateWindowTabs_SwitchBits(	unsigned char switchBits);
//		virtual	void	UpdateWindowTabs_AuxSwitchBits(	unsigned char auxSwitchBits);
		virtual	void	UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion);
//		virtual	void	UpdateWindowTabs_DesiredRotatorPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_ConnectState(bool connectedState);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);



				//*	tab information
				WindowTabMLsingle		*cMLsingleTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

};


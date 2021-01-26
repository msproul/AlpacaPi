//**************************************************************************************
//#include	"controller_focus_generic.h"


#ifndef	_WINDOWTAB_MOONLITE_SINGLE_H_
	#include	"windowtab_ml_single.h"
#endif // _WINDOWTAB_MOONLITE_SINGLE_H_


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
										const int			deviceNum);


		virtual	~ControllerFocusGeneric(void);


		virtual	void	CreateWindowTabs(void);


		virtual	void	UpdateFocuserPosition(	const int newFocuserPosition);
		virtual	void	UpdateTemperature(		const double newTemperature);
//		virtual	void	UpdateVoltage(			const double newVoltage);

		virtual	void	UpdateWindowTabs_Everything(void);
		virtual	void	UpdateWindowTabs_ReadAll(		bool hasReadAll);
//		virtual	void	UpdateWindowTabs_SwitchBits(	unsigned char switchBits);
//		virtual	void	UpdateWindowTabs_AuxSwitchBits(	unsigned char auxSwitchBits);
		virtual	void	UpdateWindowTabs_Version(		const char *versionString);
		virtual	void	UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion);
//		virtual	void	UpdateWindowTabs_DesiredRotatorPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_ConnectState(bool connectedState);



				//*	tab information
				WindowTabMLsingle		*cMLsingleTabObjPtr;
//				WindowTabConfig			*cConfigTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

};


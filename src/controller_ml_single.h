//**************************************************************************************
//#include	"controller_ml_single.h"


#ifndef	_WINDOWTAB_MOONLITE_SINGLE_H_
	#include	"windowtab_ml_single.h"
#endif // _WINDOWTAB_MOONLITE_SINGLE_H_

#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

//**************************************************************************************
class ControllerMLsingle: public ControllerFocus
{
	public:
		//
		// Construction
		//
				ControllerMLsingle(	const char			*argWindowName,
									struct sockaddr_in	*deviceAddress,
									const int			port,
									const int			deviceNum);

				ControllerMLsingle(	const char			*argWindowName,
									const char			*usbPortPath);


		virtual	~ControllerMLsingle(void);


		virtual	void	CreateWindowTabs(void);

		virtual	void	UpdateCommonProperties(void);

		virtual	void	UpdateFocuserPosition(	const int newFocuserPosition);
//		virtual	void	UpdateRotatorPosition(	const int newRotatorPosition);
//		virtual	void	UpdateAuxMotorPosition(	const int newAuxMotorPosition);
//		virtual	void	UpdateStepsPerRev(		const int newStepsPerRev);
		virtual	void	UpdateTemperature(		const double newTemperature);
//		virtual	void	UpdateVoltage(			const double newVoltage);

		virtual	void	UpdateWindowTabs_Everything(void);
		virtual	void	UpdateWindowTabs_ReadAll(		bool hasReadAll);
//		virtual	void	UpdateWindowTabs_SwitchBits(	unsigned char switchBits);
//		virtual	void	UpdateWindowTabs_AuxSwitchBits(	unsigned char auxSwitchBits);
		virtual	void	UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion);
//		virtual	void	UpdateWindowTabs_DesiredRotatorPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_ConnectState(bool connectedState);



				//*	tab information
				WindowTabMLsingle		*cMLsingleTabObjPtr;
				WindowTabConfig			*cConfigTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

};


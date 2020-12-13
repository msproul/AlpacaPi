//**************************************************************************************
//#include	"controller_ml_single.h"


#ifndef	_WINDOWTAB_MOONLITE_SINGLE_H_
	#include	"windowtab_ml_single.h"
#endif // _WINDOWTAB_MOONLITE_SINGLE_H_


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


		virtual	void	UpdateFocuserPostion(	const int newFocuserPostion);
//		virtual	void	UpdateRotatorPostion(	const int newRotatorPostion);
//		virtual	void	UpdateAuxMotorPostion(	const int newAuxMotorPostion);
//		virtual	void	UpdateStepsPerRev(		const int newStepsPerRev);
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
				WindowTabConfig			*cConfigTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

};


//*****************************************************************************
//#include	"controller_ml_nc.h"


#include	"controller.h"
#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H


#ifndef _WINDOWTAB_NITECRAWLER_H_
	#include	"windowtab_nitecrawler.h"
#endif // _WINDOWTAB_NITECRAWLER_H_

#ifndef	_WINDOWTAB_AUXMOTOR_H_
	#include	"windowtab_auxmotor.h"
#endif // _WINDOWTAB_AUXMOTOR_H_

#ifndef	_WINDOWTAB_GRAPHS_H_
	#include	"windowtab_graphs.h"
#endif // _WINDOWTAB_GRAPHS_H_

#ifndef _WINDOWTAB_CONFIG_H_
	#include	"windowtab_config.h"
#endif // _WINDOWTAB_CONFIG_H_

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_


#define	kFocuserBoxWidth	302
#define	kFocuserBoxHeight	715


//**************************************************************************************
class ControllerNiteCrawler: public ControllerFocus
{
	public:
		//
		// Construction
		//
				ControllerNiteCrawler(	const char			*argWindowName,
										struct sockaddr_in	*deviceAddress,
										const int			port,
										const int			deviceNum);

				ControllerNiteCrawler(	const char			*argWindowName,
										const char			*usbPortPath);

		virtual	~ControllerNiteCrawler(void);


		virtual	void	CreateWindowTabs(void);

		//*	sub class specific routines

		virtual	void	UpdateFocuserPosition(const int newFocuserPosition);
		virtual	void	UpdateRotatorPosition(const int newRotatorPosition);
		virtual	void	UpdateAuxMotorPosition(const int newAuxMotorPosition);
		virtual	void	UpdateStepsPerRev(const int newStepsPerRev);
		virtual	void	UpdateTemperature(const double newTemperature);
		virtual	void	UpdateVoltage(const double newVoltage);

		virtual	void	UpdateWindowTabs_Everything(void);
		virtual	void	UpdateWindowTabs_ReadAll(bool hasReadAll);
		virtual	void	UpdateWindowTabs_SwitchBits(unsigned char switchBits);
		virtual	void	UpdateWindowTabs_AuxSwitchBits(unsigned char auxSwitchBits);
		virtual	void	UpdateWindowTabs_Version(const char *versionString);
		virtual	void	UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_DesiredRotatorPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_DesiredAuxPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_ConnectState(bool connectedState);
		virtual	void	UpdateWindowTabs_SwitchState(int switchId, bool onOffState);

				bool	AlpacaGetStatus(void);


				WindowTabNitecrawler	*cNiteCrawlerTabObjPtr;
				WindowTabAuxMotor		*cAuxTabObjPtr;
				WindowTabConfig			*cConfigTabObjPtr;
				WindowTabGraph			*cGraphTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;


};

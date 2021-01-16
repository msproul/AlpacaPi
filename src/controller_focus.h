//*****************************************************************************
//#include	"controller_focus.h"


#include	"controller.h"
#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H

#ifndef	_WINDOWTAB_FOCUSER_H_
//	#include	"windowtab_focuser.h"
#endif // _WINDOWTAB_FOCUSER_H_

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

#ifndef	_MOONLITE_COM_H_
	#include	"moonlite_com.h"
#endif


#define	kFocuserBoxWidth	302
#define	kFocuserBoxHeight	715


//**************************************************************************************
enum
{
	kFocuserType_NiteCrawler	=	0,
	kFocuserType_MoonliteSingle,
	kFocuserType_MoonliteDouble,

	kFocuserType_Other,

};

//**************************************************************************************
//*	this is for NiteCrawler
enum
{
	kSwitch_Out	=	0,
	kSwitch_In,
	kSwitch_Rot
};


//**************************************************************************************
class ControllerFocus: public Controller
{
	public:
		//
		// Construction
		//
				ControllerFocus(	const char			*argWindowName,
									struct sockaddr_in	*deviceAddress,
									const int			port,
									const int			deviceNum,
									const int			focuserType);

				ControllerFocus(	const char			*argWindowName,
									const char			*usbPortPath,
									int					focuserType = kFocuserType_NiteCrawler);

		virtual	~ControllerFocus(void);

				void	ControllerFocusInit(const int comMode, const int focuserType);

		virtual	void	CreateWindowTabs(void);

//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(void);

		//*	sub class specific routines


		virtual	void	UpdateFocuserPostion(const int newFocuserPostion);
		virtual	void	UpdateRotatorPostion(const int newRotatorPostion);
		virtual	void	UpdateAuxMotorPostion(const int newAuxMotorPostion);
		virtual	void	UpdateStepsPerRev(const int newStepsPerRev);
		virtual	void	UpdateTemperature(const double newTemperature);
		virtual	void	UpdateVoltage(const double newVoltage);
		virtual	void	UpdateFromFirstRead(void);

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

		virtual	bool	AlpacaGetStartupData(void);
		virtual	bool	AlpacaGetStatus(void);
		virtual	void	AlpacaProcessReadAll(	const char	*deviceType,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString);
				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time

				void	SendMoveFocuserCommand(int newDesiredPosition);
				void	SendMoveRotatorCommand(int newDesiredPosition);
				void	SendMoveAuxMotorCommand(int newDesiredPosition);
				void	SendStopMotorsCommand(void);
				void	ZeroMotorValues(void);

				void	ReadNiteCrawlerColors(void);

				void	OpenUSBport(void);
				void	CloseUSBport(void);
				bool	USBGetStatus(void);

				bool				cReadStartup;
				bool				cOnLine;
				int					cFocuserType;		//*	i.e. nitecrawler, etc
				int					cCommMode;			//*	Alpaca or USB
				char				cUSBpath[48];
				bool				cValidUSB;
				bool				cUSBportOpen;
				TYPE_MOONLITECOM	cMoonliteCom;

				char				cModelName[48];
				char				cSerialNumber[48];
				char				cUnitVersion[48];

				int					cFocuserPosition;
				int					cRotatorPosition;
				int					cFocuserDesiredPos;
				int					cRotatorDesiredPos;
				int					cAuxMotorPosition;
				int					cAuxMotorDesiredPos;
				int					cStepsPerRev;			//*	steps per revolution

				uint32_t			cLastTimeSecs_Temperature;
				double				cTemperature_DegC;
				double				cVoltage;
				bool				cIsMoving;


};

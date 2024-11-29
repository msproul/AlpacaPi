//**************************************************************************
//*	Name:			focuserdriver_nc.h
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 28,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*	Jun 10,	2023	<MLS> Changed class name from FocuserNiteCrawler to FocuserMoonLite
//**************************************************************************
//#include	"focuserdriver_nc.h"

#ifndef _FOCUSER_MOONLITE_H_
#define	_FOCUSER_MOONLITE_H_
//#include	"focuserdriver_nc.h"

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif
#ifndef	_FOCUSER_DRIVER_H_
	#include	"focuserdriver.h"
#endif

#define	_USE_MOONLITE_COM_

#include	"moonlite_com.h"

int	CreateFocuserObjects_MoonLite(void);


//**************************************************************************************
class FocuserMoonLite: public FocuserDriver
{
	public:

		//
		// Construction
		//
						FocuserMoonLite(const char *devicePath);
		virtual			~FocuserMoonLite(void);
		virtual	int32_t	RunStateMachine(void);

		virtual	TYPE_ASCOM_STATUS	SetFocuserPosition(const int32_t newPosition, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	HaltFocuser(char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	SetStepperPosition(const int axisNumber, const int32_t newPosition);
		virtual	TYPE_ASCOM_STATUS	HaltStepper(const int axisNumber);

	protected:
		bool			OpenFocuserConnection(const char *usbPortPath);		//*	returns true if open succeeded.
		void			SendCommand(const char *theCommand);
		int				ReadUntilChar(const int fd, char *readBuff, const int maxChars, const char terminator);
		bool			GetPosition(const int axisNumber, int32_t *valueToUpdate);

		//*	these get called from RunStateMachine()
		void			ProcessQueuedCommands(void);
		void			ProcessPeriodicRequests(void);

		int				cFileDesc;	//*	port file descriptor

		char			cLastCmdSent[16];

		TYPE_MOONLITECOM	cMoonliteCom;

		//*	command queue data
		bool			cSendHaltCmd;
		int				cHaltCmdAxis;

		bool			cSendMoveCmd;
		int				cMoveCmdAxis;
		int				cMoveCmdPosition;

};



#endif	//	_FOCUSER_MOONLITE_H_

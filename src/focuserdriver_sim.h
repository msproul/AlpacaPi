//**************************************************************************
//*	Name:			focuserdriver_sim.h
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  3,	2023	<MLS> Created focuserdriver_sim.h
//**************************************************************************
//#include	"focuserdriver_sim.h"


#ifndef _FOCUSER_SIMULATOR_H_
#define	_FOCUSER_SIMULATOR_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif
#ifndef	_FOCUSER_DRIVER_H_
	#include	"focuserdriver.h"
#endif

int	CreateFocuserObjects_SIM(void);


//**************************************************************************************
class FocuserDriverSIM: public FocuserDriver
{
	public:

		//
		// Construction
		//
						FocuserDriverSIM(const int argDevNum);
		virtual			~FocuserDriverSIM(void);
		virtual	int32_t	RunStateMachine(void);
//
//		virtual	TYPE_ASCOM_STATUS	SetFocuserPosition(const int32_t newPosition, char *alpacaErrMsg);
//		virtual	TYPE_ASCOM_STATUS	HaltFocuser(char *alpacaErrMsg);
//
//		virtual	TYPE_ASCOM_STATUS	SetStepperPosition(const int axisNumber, const int32_t newPosition);
//		virtual	TYPE_ASCOM_STATUS	HaltStepper(const int axisNumber);
//
//	protected:
//		bool			OpenFocuserConnection(const char *usbPortPath);		//*	returns true if open succeeded.
//		void			SendCommand(const char *theCommand);
//		int				ReadUntilChar(const int fd, char *readBuff, const int maxChars, const char terminator);
//		bool			GetPosition(const int axisNumber, int32_t *valueToUpdate);
//
//		//*	these get called from RunStateMachine()
//		void			ProcessQueuedCommands(void);
//		void			ProcessPeriodicRequests(void);
//
//		int				cFileDesc;	//*	port file descriptor
//		uint32_t		cLastTimeSecs_Temperature;
//		uint32_t		cLastTimeMilSecs_Position;
//
//		char			cLastCmdSent[16];


};



#endif	//	_FOCUSER_SIMULATOR_H_

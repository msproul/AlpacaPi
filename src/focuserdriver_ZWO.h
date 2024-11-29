//**************************************************************************
//*	Name:			focuserdriver_ZWO.h
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jul 25,	2024	<MLS> Created focuserdriver_ZWO.h
//**************************************************************************
//#include	"focuserdriver_ZWO.h"


#ifndef _FOCUSER_ZWO_H_
#define	_FOCUSER_ZWO_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif
#ifndef	_FOCUSER_DRIVER_H_
	#include	"focuserdriver.h"
#endif

#ifndef EAF_FOCUSER_H
	#include	"EAF_focuser.h"		//*	ZWO header file for EAF
#endif


int	CreateFocuserObjects_ZWO(void);


//**************************************************************************************
class FocuserDriverZWO: public FocuserDriver
{
	public:

		//
		// Construction
		//
						FocuserDriverZWO(const int eaf_ID_num);
		virtual			~FocuserDriverZWO(void);
		virtual	int32_t	RunStateMachine(void);
//
		virtual	TYPE_ASCOM_STATUS	SetFocuserPosition(const int32_t newPosition, char *alpacaErrMsg);
//		virtual	TYPE_ASCOM_STATUS	HaltFocuser(char *alpacaErrMsg);
//
//		virtual	TYPE_ASCOM_STATUS	SetStepperPosition(const int axisNumber, const int32_t newPosition);
//		virtual	TYPE_ASCOM_STATUS	HaltStepper(const int axisNumber);
//
//	protected:
		bool			OpenFocuserConnection(void);		//*	returns true if open succeeded.
//		void			SendCommand(const char *theCommand);
//		int				ReadUntilChar(const int fd, char *readBuff, const int maxChars, const char terminator);
//		bool			GetPosition(const int axisNumber, int32_t *valueToUpdate);
		void			ProcessEAFerror(const int eaf_ErrorCode, const char *functionName, const char *errorMssg);
//
//
//		char			cLastCmdSent[16];
		int				cEAF_ID_num;
		EAF_INFO		cEAFInfo;
		bool			cEAFconnectionIsOpen;


};



#endif	//	_FOCUSER_ZWO_H_

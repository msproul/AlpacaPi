//**************************************************************************
//*	Name:			rotatordriver.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 28,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*****************************************************************************
//#include	"rotatordriver.h"

#ifndef	_ROTATOR_DRIVER_H_
#define	_ROTATOR_DRIVER_H_


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

#include	"rotator_AlpacaCmds.h"


//**************************************************************************************
class RotatorDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									RotatorDriver(const int argDevNum);
		virtual						~RotatorDriver(void);
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

		TYPE_ASCOM_STATUS	Get_Canreverse(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Ismoving(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_MechanicalPosition(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Position(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Reverse(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Reverse(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Stepsize(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Targetposition(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Put_Halt(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_Move(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_Moveabsolute(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_MoveMechanical(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_Sync(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		//*	added by MLS
				TYPE_ASCOM_STATUS	Put_Step(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_Stepabsolute(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		virtual	bool					DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen);
		virtual	int32_t					ReadCurrentPoisiton_steps(void);
		virtual	double					ReadCurrentPoisiton_degs(void);
		virtual	TYPE_ASCOM_STATUS		SetCurrentPoisiton_steps(const int32_t newPosition);
		virtual	TYPE_ASCOM_STATUS		SetCurrentPoisiton_degs(const double newPosition);
		virtual	TYPE_ASCOM_STATUS		HaltMovement(void);
		virtual	bool					IsRotatorMoving(void);

				void					UpdateRotorPosition(bool updateTargetPosition=false);

				//*	these are access functions for FITS output
				void					GetRotatorManufacturer(char *manufactString);
				void					GetRotatorModel(char *modelName);
				void					GetRotatorSerialNumber(char *serialNumber);

				void					DumpRotatorProperties(const char *callingFunctionName);

	protected:
		TYPE_RotatorProperties	cRotatorProp;

		char		cRotatorManufacturer[64];
		char		cRotatorModel[64];
		char		cRotatorSerialNum[64];

		bool		cRotatorReverseState;		//*	reverse enable (default = false)

		int32_t		cRotatorStepsPerRev;		//*	number of steps per rev
		int32_t		cRotatorPosition_steps;		//*	current rotator position
};

void	CreateRotatorObjects(void);



#endif // _ROTATOR_DRIVER_H_

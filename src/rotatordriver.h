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

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif



//*****************************************************************************
//*	Rotator Specific Methods
//*****************************************************************************
enum
{
	kCmd_Rotator_canreverse=	0,		//*	Indicates whether the Rotator supports the Reverse method.
	kCmd_Rotator_ismoving,				//*	Indicates whether the rotator is currently moving.
	kCmd_Rotator_position,				//*	Returns the rotator's current position.
	kCmd_Rotator_reverse,				//*	Returns the rotator's Reverse state.
										//*	Sets the rotator's Reverse state.
	kCmd_Rotator_stepsize,				//*	Returns the minimum StepSize
	kCmd_Rotator_targetposition,		//*	Returns the destination position angle.
	kCmd_Rotator_halt,					//*	Immediately stops rotator motion.
	kCmd_Rotator_move,					//*	Moves the rotator to a new relative position.
	kCmd_Rotator_moveabsolute,			//*	Moves the rotator to a new absolute position.

	//*	added by MLS
	kCmd_Rotator_Extras,
	kCmd_Rotator_step,					//*	Moves the rotator to a new relative position.
	kCmd_Rotator_stepabsolute,			//*	Moves the rotator to a new absolute position.

	kCmd_Rotator_readall,				//*
};

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
//		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

		TYPE_ASCOM_STATUS	Get_Canreverse(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Ismoving(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Position(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Reverse(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Reverse(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Stepsize(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Targetposition(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Put_Halt(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_Move(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_Moveabsolute(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		//*	added by MLS
		TYPE_ASCOM_STATUS	Put_Step(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_Stepabsolute(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		//*	these are access functions for FITS output
		void	GetRotatorManufacturer(char *manufactString);
		void	GetRotatorModel(char *modelName);
		void	GetRotatorSerialNumber(char *serialNumber);

		virtual	int32_t					ReadCurrentPoisiton_steps(void);
		virtual	double					ReadCurrentPoisiton_degs(void);
		virtual	TYPE_ASCOM_STATUS		SetCurrentPoisiton_steps(const int32_t newPosition);
		virtual	TYPE_ASCOM_STATUS		SetCurrentPoisiton_degs(const double newPosition);
		virtual	TYPE_ASCOM_STATUS		HaltMovement(void);
		virtual	bool					IsRotatorMoving(void);

	protected:
		char		cRotatorManufacturer[64];
		char		cRotatorModel[64];
		char		cRotatorSerialNum[64];
		bool		cRotatorCanReverse;
		bool		cRotatorReverseState;		//*	reverse enable (default = false)
		bool		cRotatorIsMoving;
		double		cRotatorStepSize;

		int32_t		cRotatorStepsPerRev;		//*	number of steps per rev
		int32_t		cRotatorPos_step;			//*	current rotator position
		double		cRotatorPos_degs;
		double		cRotatorTrgtPos_degs;
};

void	CreateRotatorObjects(void);



#endif // _ROTATOR_DRIVER_H_

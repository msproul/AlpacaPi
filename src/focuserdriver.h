//**************************************************************************
//*	Name:			focuserdriver.h
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
//#include	"focuserdriver.h"


#ifndef	_FOCUSER_DRIVER_H_
#define	_FOCUSER_DRIVER_H_


#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

//#include	"focuser_AlpacaCmds.h"

int	CreateFocuserObjects(void);


//**************************************************************************************
class FocuserDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
						FocuserDriver(void);
		virtual			~FocuserDriver(void);

		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);
		virtual bool				GetCommandArgumentString(const int cmdNumber, char *agumentString, char *commentString);

			TYPE_ASCOM_STATUS	Get_Absolute(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
			TYPE_ASCOM_STATUS	Get_Ismoving(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
			TYPE_ASCOM_STATUS	Get_Maxincrement(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
			TYPE_ASCOM_STATUS	Get_Maxstep(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
			TYPE_ASCOM_STATUS	Get_Position(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
			TYPE_ASCOM_STATUS	Get_Stepsize(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
			TYPE_ASCOM_STATUS	Get_Tempcomp(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

			TYPE_ASCOM_STATUS	Put_Tempcomp(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
			TYPE_ASCOM_STATUS	Get_Tempcompavailable(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
			TYPE_ASCOM_STATUS	Get_Temperature(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
			TYPE_ASCOM_STATUS	Put_Halt(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
			TYPE_ASCOM_STATUS	Put_Move(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

			//*	not part of the Alpaca standard
			TYPE_ASCOM_STATUS	Put_MoveRelative(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
	virtual	TYPE_ASCOM_STATUS	Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

	virtual	bool		DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen);

			bool	RotationSupported(void);


			//*	these are access functions for FITS output
			int32_t	GetFocuserPosition(void);
			void	GetFocuserManufacturer(char *manufactString);
			void	GetFocuserModel(char *modelName);
			void	GetFocuserVersion(char *versionString);
			void	GetFocuserSerialNumber(char *serialNumString);
			double	GetFocuserTemperature(void);
			double	GetFocuserVoltage(void);


		//*	focuser specific commands
		virtual	TYPE_ASCOM_STATUS	SetFocuserPosition(const int32_t newPosition, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	HaltFocuser(char *alpacaErrMsg);


		//*	these functions are for rotator access
		int32_t			GetRotatorPosition(void);
		int32_t			GetRotatorStepsPerRev(void);
		bool			GetRotatorIsMoving(void);

		void			DumpFocuserProperties(const char *callingFunctionName);

	protected:
		TYPE_FocuserProperties	cFocuserProp;
		TYPE_RotatorProperties	cRotatorProp;

		int32_t			cFocuserPGet_Readallostion;
		int32_t			cPrevFocuserPosition;
		int32_t			cNewFocuserPosition;
												//*	i.e. the maximum number of steps allowed in one move operation

		//*	this is for support of Moonlite NiteCrawler
		bool			cIsNiteCrawler;
		bool			cFocuserSupportsRotation;
		int32_t			cRotatorStepsPerRev;

		int32_t			cRotatorPosition;
		int32_t			cPrevRotatorPosition;
		int32_t			cNewRotatorPosition;

		bool			cFocuserSupportsAux;
		int32_t			cAuxPosition;
		int32_t			cPrevAuxPosition;
		int32_t			cNewAuxPosition;
		bool			cAuxIsMoving;


		bool			cFocuserHasTemperature;
//		double			cFocuserTemp;			//*	degrees C
		bool			cHasTempComp;

		bool			cFocuserHasVoltage;
		double			cFocuserVoltage;
		bool			cSwitchIN;
		bool			cSwitchOUT;
		bool			cSwitchROT;
		bool			cSwitchAUX1;
		bool			cSwitchAUX2;

		int				cInvalidStringErrCnt;

};


#endif	//	_FOCUSER_DRIVER_H_

//**************************************************************************
//*	Name:			filterwheeldriver.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 10,	2019	<MLS> Created filterwheeldriver.h
//*****************************************************************************

//#include	"filterwheeldriver.h"

#ifndef _FILTERWHEELDRIVER_H_
#define	_FILTERWHEELDRIVER_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif
#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif


void	CreateFilterWheelObjects(void);



//**************************************************************************************
typedef enum
{
	kFilterWheelState_OK	=	0,
	kFilterWheelState_Moving,

	kFilterWheelState_last
} TYPE_FW_State;

//**************************************************************************************
class FilterwheelDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									FilterwheelDriver(const int argDevNum);
		virtual						~FilterwheelDriver(void);
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);


				TYPE_ASCOM_STATUS		Get_Position(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Put_Position(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS		Get_Focusoffsets(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS		Get_Names(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		virtual	TYPE_ASCOM_STATUS		Get_Readall(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		virtual	bool					DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen);

		virtual void				ReadFilterNamesTextFile(void);
		virtual	int					Read_CurrentFWstate(void);
		virtual	TYPE_ASCOM_STATUS	Read_CurrentFilterPositon(int *rtnCurrentPosition = NULL);
		virtual	TYPE_ASCOM_STATUS	Read_CurrentFilterName(char *rtnCurrentName);
		virtual	TYPE_ASCOM_STATUS	Set_CurrentFilterPositon(const int newPosition);
		virtual	bool				IsFilterwheelConnected(void);

				bool				cFilterWheelConnected;
				int					cFilterWheelDevNum;
				int					cNumberOfPositions;
				char				cFilterWheelCurrName[48];

				TYPE_FW_State				cFilterWheelState;
				TYPE_FilterWheelProperties	cFilterWheelProp;


				//*	this are primarily used for debugging
				bool				cFilterWheelIsOpen;
				int					cSuccesfullOpens;
				int					cSuccesfullCloses;
				int					cOpenFailures;
				int					cCloseFailures;

};




#endif // _FILTERWHEELDRIVER_H_

//**************************************************************************
//*	Name:			switchdriver.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec 26,	2019	<MLS> Created switchdriver.h
//*	Nov 28,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*	Jan  1,	2022	<MLS> Added kSwitchType_Status
//*****************************************************************************

//#include	"switchdriver.h"

#ifndef _SWITCHDRIVER_H_
#define	_SWITCHDRIVER_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

#include	"switch_AlpacaCmds.h"



//*****************************************************************************
enum
{
	kSwitchType_Relay	=	0,
	kSwitchType_Analog,
	kSwitchType_Status

};


void	CreateSwitchObjects(void);

//*****************************************************************************
typedef struct	//	TYPE_SwitchDescription
{
	int		switchType;
	char	switchName[kMaxSwitchNameLen];
	char	switchDescription[kMaxSwitchDescLen];
	int		hwPinNumber;							//*	hardware pin number
	int		valueForTrue;
} TYPE_SwitchDescription;


//**************************************************************************************
class SwitchDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									SwitchDriver(void);
		virtual						~SwitchDriver(void);
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
//		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

	protected:
				TYPE_ASCOM_STATUS	GetSwitchID(				TYPE_GetPutRequestData *reqData, int *idNum, char *alpacaErrMsg);

				TYPE_ASCOM_STATUS	Get_Maxswitch(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Canwrite(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_GetSwitch(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_GetSwitchDescription(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_GetSwitchName(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_GetSwitchValue(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_MinSwitchValue(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_MaxSwitchValue(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_SetSwitch(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_SetSwitchName(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_SetSwitchValue(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_SwitchStep(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		virtual	TYPE_ASCOM_STATUS	Get_Readall(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		virtual	bool				DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen);

				void	ReadSwitchDataFile(void);
				void	WriteSwitchDataFile(void);


		virtual	bool	GetSwitchState(const int switchNumber);
		virtual	void	SetSwitchState(const int switchNumber, bool on_off);

		virtual	void	SetSwitchValue(const int switchNumber, double switchValue);
		virtual	double	GetSwitchValue(const int switchNumber);



				void	ConfigureSwitch(	const int	switchNumber,
											const int	switchType,
											const int	hardWarePinNumber,
											const int	trueValue=1);

				int						cNumSwitches;
				TYPE_SwitchDescription	cSwitchTable[kMaxSwitchCnt];

				//*	min max values as doubles
//-				int		cSwitchType[kMaxSwitchCnt];
				double	cMinSwitchValue[kMaxSwitchCnt];
				double	cMaxSwitchValue[kMaxSwitchCnt];
				double	cCurSwitchValue[kMaxSwitchCnt];

				TYPE_SwitchProperties	cSwitchProp;
};



#endif // _SWITCHDRIVER_H_

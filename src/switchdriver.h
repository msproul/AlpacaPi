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



//*****************************************************************************
//switch
//*****************************************************************************
enum
{
	kCmd_Switch_maxswitch	=	0,		//*	The number of switch devices managed by this driver
	kCmd_Switch_canwrite,				//*	Indicates whether the specified switch device can be written to
	kCmd_Switch_getswitch,				//*	Return the state of switch device id as a boolean
	kCmd_Switch_getswitchdescription,	//*	Gets the description of the specified switch device
	kCmd_Switch_getswitchname,			//*	Gets the name of the specified switch device
	kCmd_Switch_getswitchvalue,			//*	Gets the value of the specified switch device as a double
	kCmd_Switch_minswitchvalue,			//*	Gets the minimum value of the specified switch device as a double
	kCmd_Switch_maxswitchvalue,			//*	Gets the maximum value of the specified switch device as a double
	kCmd_Switch_setswitch,				//*	Sets a switch controller device to the specified state, true or false
	kCmd_Switch_setswitchname,			//*	Sets a switch device name to the specified value
	kCmd_Switch_setswitchvalue,			//*	Sets a switch device value to the specified value
	kCmd_Switch_switchstep,				//*	Returns the step size that this device supports (the difference between successive values of the device).

	//*	added by MLS
	kCmd_Switch_Extras,
	kCmd_Switch_setswitchdescription,
	kCmd_Switch_readall,

	kCmd_Switch_last
};

//*****************************************************************************
enum
{
	kSwitchType_Relay	=	0,
	kSwitchType_Analog,
	kSwitchType_Status

};


void	CreateSwitchObjects(void);

#define	kMaxSwitchCnt			16
#define	kMaxSwitchNameLen		32
#define	kMaxSwitchDescLen		64
//*****************************************************************************
typedef struct
{
	int		switchType;
	char	switchName[kMaxSwitchNameLen];
	char	switchDesciption[kMaxSwitchDescLen];
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
				int		GetSwitchID(				TYPE_GetPutRequestData *reqData);

				TYPE_ASCOM_STATUS	Get_Maxswitch(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Canwrite(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Getswitch(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Getswitchdescription(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Getswitchname(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Getswitchvalue(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Minswitchvalue(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Maxswitchvalue(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_Setswitch(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_Setswitchname(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_Setswitchvalue(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Switchstep(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

				TYPE_ASCOM_STATUS	Get_Readall(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

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

				int		cNumSwitches;
				TYPE_SwitchDescription	cSwitchTable[kMaxSwitchCnt];

				//*	min max values as doubles
//-				int		cSwitchType[kMaxSwitchCnt];
				double	cMinSwitchValue[kMaxSwitchCnt];
				double	cMaxSwitchValue[kMaxSwitchCnt];
				double	cCurSwitchValue[kMaxSwitchCnt];
};



#endif // _SWITCHDRIVER_H_

//**************************************************************************
//*	Name:			domedriver.h
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  4,	2019	<MLS> Started on C++ version of dome driver
//*	Nov 28,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*****************************************************************************
//#include	"domedriver.h"

#ifndef _DOME_DRIVER_H_
#define	_DOME_DRIVER_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif // _ALPACA_DEFS_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif


#ifndef _SLIT_TRACKER_H_
	#include	"slittracker.h"
#endif // _SLIT_TRACKER_H_


//*****************************************************************************
//*	dome states
typedef enum DOME_STATE_TYPE
{
	kDomeState_Idle	=	0,
	kDomeState_SpeedingUp,
	kDomeState_Moving,
	kDomeState_SlowingDown,
	kDomeState_Stopped,
	kDomeState_Reversing_Slowing,
	kDomeState_Reversing_Waiting,

	kDomeState_last

} DOME_STATE_TYPE;


//*****************************************************************************
//*	Configuration
typedef enum DOME_CONFIG
{
	kIsDome	=	0,
	kIsRollOffRoof

} DOME_CONFIG;


#define		kSensorValueCnt	12


//**************************************************************************************
class DomeDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
						DomeDriver(const int argDevNum);
		virtual			~DomeDriver(void);
		virtual	void	Init_Hardware(void);

		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);


		virtual	int32_t				RunStateMachine(void);
		virtual	int32_t				RunStateMachine_Dome(void);
		virtual	int32_t				RunStateMachine_ROR(void);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

	protected:

				TYPE_ASCOM_STATUS	Get_Altitude(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Athome(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Atpark(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Azimuth(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Canfindhome(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Canpark(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Cansetaltitude(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Cansetazimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Cansetpark(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Cansetshutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Canslave(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Cansyncazimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Shutterstatus(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Slaved(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Put_Slaved(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Slewing(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

				TYPE_ASCOM_STATUS	Put_AbortSlew(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_CloseShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_FindHome(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_OpenShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_Park(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_SetPark(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_SlewToAltitude(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_SlewToAzimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_SyncToAzimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

				//*	extras added by MLS
				TYPE_ASCOM_STATUS	Put_BumpMove(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, int direction);
				TYPE_ASCOM_STATUS	Put_NormalMove(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, int direction);
				TYPE_ASCOM_STATUS	Put_SlowMove(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, int direction);

				TYPE_ASCOM_STATUS	Get_Currentstate(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Readall(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		virtual	void	StartDomeMoving(const int direction);
		virtual	void	StopDomeMoving(bool rightNow);
		virtual	void	CheckMoving(void);
		virtual	void	UpdateDomePosition(void);
		virtual	bool	BumpDomeSpeed(const int howMuch);
		virtual	void	CheckSensors(void);
		virtual	void	CheckDomeButtons(void);
		virtual	void 	ProcessButtonPressed(const int pressedButton);

		virtual	TYPE_ASCOM_STATUS 	OpenShutter(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS 	CloseShutter(char *alpacaErrMsg);


				DOME_STATE_TYPE	cDomeState;
				DOME_STATE_TYPE	cPreviousDomeState;

				//	The dome azimuth, increasing clockwise,
				//	i.e.,	North	=	0,
				//			East	=	90,
				//			South	=	180 South, 270
				//			West	=	270.
				//*	North is true north and not magnetic north.

				double			cParkAzimuth;		//*	the azimuth value for the park position
				double			cHomeAzimuth;		//*	the azimuth value for the Home position

				int				cCurrentPWM;
				int				cCurrentDirection;
				int				cBumpSpeedAmount;
				int32_t			cTimeOfLastSpeedChange;
				int32_t			cTimeOfMovingStart;

				int32_t			cTimeOfLastAzimuthUpdate;	//*	used to integrate position over time

				bool			cGoingBump;
				bool			cGoingHome;
				bool			cGoingPark;
				bool			cManualMove;

				DOME_CONFIG		cDomeConfig;

				//-----------------------------------------------------
				//*	ASCOM properties
				TYPE_DomeProperties	cDomeProp;


		//		int32_t			cShutterstatus;
				double			cAzimuth_Destination;		//*	were we want to go to, must be >= to 0 to be valid


	protected:
		//=====================================================
		//*	this is how we find the slit tracker
		virtual	void					ProcessDiscovery(		struct sockaddr_in	*deviceAddress,
																const int			ipPortNumber,
																const char			*deviceType,
																const int			deviceNumber);
#ifdef _ENABLE_SLIT_TRACKER_REMOTE_
				//*	Slit tracker device info
				void				GetSlitTrackerData(void);
				bool				cSlitTrackerInfoValid;
				struct sockaddr_in	cSlitTrackerDeviceAddress;
				int					cSlitTrackerPort;
				int					cSlitTrackerAlpacaDevNum;
				int32_t				cTimeOfLastSlitTrackerUpdate;
				TYPE_SLITCLOCK	cSlitDistance[kSensorValueCnt];
#endif // _ENABLE_SLIT_TRACKER_REMOTE_


#ifdef _ENABLE_REMOTE_SHUTTER_
		//=====================================================
		//*	this is for talking to the shutter as a separate device
				void				GetRemoteShutterStatus(void);
				TYPE_ASCOM_STATUS	OpenRemoteShutter(char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	CloseRemoteShutter(char *alpacaErrMsg);

				bool				cShutterInfoValid;
				struct sockaddr_in	cShutterDeviceAddress;
				int					cShutterPort;
				int					cShutterAlpacaDevNum;
				int32_t				cTimeOfLastShutterUpdate;
#endif // _ENABLE_REMOTE_SHUTTER_

};


//*****************************************************************************
typedef struct
{
	int 		pinNumber;
	uint32_t	debounceBits;
	int			previousState;
	int			curentState;
} BUTTON_ENTRY;

#define	kButtonReleased	0
#define	kButtonPressed	1




#define	kRotateDome_CW		0
#define	kRotateDome_CCW		1



#define	kStopRightNow		true
#define	kStopNormal			false



void	CreateDomeObjects(void);


#endif	//	_DOME_DRIVER_H_

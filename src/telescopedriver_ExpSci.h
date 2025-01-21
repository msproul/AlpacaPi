//**************************************************************************
//*	Name:			telescopedriver_ExpSci.h
//*
//*	Author:			Mark Sproul (C) 2021
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 21,	2024	<MLS> Created telescopedriver_ExpSci.h
//*****************************************************************************
//#include	"telescopedriver_ExpSci.h"



#ifdef _USE_TELESCOPE_COM_OBJECT_
	#ifndef _TELESCOPE_COMM_H_
		#include	"telescope_comm.h"
	#endif
#endif

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif


#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif

#ifndef _TELESCOPE_DRIVER_COMM_H_
	#include	"telescopedriver_comm.h"
#endif

int	CreateTelescopeObjects_ExploreScientific(void);
#define		kPMC8_RA_steps_EXOS		4147200
#define		kPMC8_DEC_steps_EXOS	4147200

#define		kPMC8_RA_steps_G11		4608000
#define		kPMC8_DEC_steps_G11		4608000


//**************************************************************************************
typedef enum		//	TYPE_PMC8mount
{
	kPMC8mount_Invalid			=	-1,
	kPMC8mount_iEXOS100			=	0x0,	//	0x0300	iEXOS100 ES1A01CB11A01	//	original 100 board with the original ESP module
	kPMC8mount_iEXOS100_ESP		=	0x1,	//	0x0301 iEXOS100 ES1A02AC11A01	//	ESP module
	kPMC8mount_iEXOS200			=	0x2,	//	0x0302 iEXOS200 ES1A02AC11A01
	kPMC8mount_iEXOS300			=	0x3,	//	0x0303 iEXOS300 ES2A07AC11A01
	kPMC8mount_G11_RN131		=	0x4,	//	0x0304 G11 ES2A06BA11A01, RN131
	kPMC8mount_G11_ESP32		=	0x5,	//	0x0305 G11 ES2A06BC11A01, ESP32
	kPMC8mount_G11_RN131_2		=	0x6,	//	0x0306 G11 ES2A07AA11A01, RN131
	kPMC8mount_G11_ESP32_2		=	0x7,	//	0x0307 G11 ES2A07AC11A01, ESP32
	kPMC8mount_EXOS2_RN131		=	0x8,	//	0x0308 EXOS2 ES2A06BA11A01, RN131
	kPMC8mount_EXOS2_ESP32		=	0x9,	//	0x0309 EXOS2 ES2A06BC11A01, ESP32
	kPMC8mount_EXOS2_RN131_2	=	0xA,	//	0x030A EXOS2 ES2A07AA11A01, RN131
	kPMC8mount_EXOS2_ESP32_2	=	0xB,	//	0x030B EXOS2 ES2A07AC11A01, ESP32
	kPMC8mount_Scotty_RN131		=	0xC,	//	0x030C Scotty ES2A07AC11A01, RN131
	kPMC8mount_Titan_RN131		=	0xD,	//	0x0207 Titan ES2A07AC11A01, RN131

	kPMC8mount_Last
} TYPE_PMC8mount;


////**************************************************************************************
typedef enum		//	TYPE_Wifi
{
	kPMC8wifi_RN131	=	0,
	kPMC8wifi_8266	=	1,
	kPMC8wifi_ESP32	=	2
} TYPE_Wifi;

//**************************************************************************************
typedef enum		//	TYPE_MoveAxisMode
{
	kMoveAxisMode_idle	=	0,
	kMoveAxisMode_Accel,
	kMoveAxisMode_Steady,
	kMoveAxisMode_Decel,
	kMoveAxisMode_Stop
} TYPE_MoveAxisMode;


//**************************************************************************************
typedef struct		//*	TYPE_PMC8info
{

	//----------------------------------------------------
	//*	Data from device
	TYPE_PMC8mount	MountType;
	char			ModelName[32];
	TYPE_Wifi		WifiType;
	char			WifiTypeStr[16];
	int				WifiChannel;
	int				BaudRate;
	bool			TrackAlways;
	bool			TrackOnBoot;
	bool			NorthernHemisphere;
	bool			AutoGuiderEnabled;
	int				TCP_UDP;
	bool			MotorEnable;
	int				MotorCurrentSlew;
	int				MotorCUrrentTrack;
	int				ST4_Type;
	int				SiderealRateFrac_RA;
	int				SiderealRateFrac_DEC;
	//----------------------------------------------------
	//*	static value settings for the PMC8
	//*	they are specific to the model in use and do not change unless the mount is changed
	double  ArcSecPerMicroStep;
	int		StepsPer360_RA;								//*	4147200 for iEXOS-100-2 PMC-Eight
	int		StepsPer360_DEC;
	//*	tracking rates are Sidereal, Lunar, Solar, King
	double	TrackingRate_ASCOM[kDriveRate_Count];		//*	values for tracking rates, refer to alpaca_defs.h
	double	TrackingRate_Sidereal[kDriveRate_Count];	//*	values for tracking rates, refer to alpaca_defs.h
	double	TrackingRate_MicroStep[kDriveRate_Count];	//*	values for tracking rates, refer to alpaca_defs.h
	int		TrackingRate_value[kDriveRate_Count];



	//----------------------------------------------------
	//*	dynamic value settings for the PMC8
	int		WirelessChannel;
	int		Direction_RA;
	int		Direction_DEC;


	//----------------------------------------------------
	//*	Movement limits
	bool	EnableMovementLimits;
	int		MovementLimit_RA_East;
	int		MovementLimit_RA_West;

} TYPE_PMC8info;



//**************************************************************************************
class TelescopeDriverExpSci: public TelescopeDriverComm
{
	public:

		//
		// Construction
		//
								TelescopeDriverExpSci(const char *devicePath);
		virtual					~TelescopeDriverExpSci(void);
		virtual	void			OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
//-		virtual	int32_t			RunStateMachine(void);

		virtual	bool			SendCmdsFromQueue(void);
		virtual	bool			SendCmdsPeriodic(void);


//		//--------------------------------------------------------------------------------------------------
//		//*	these routines should be implemented by the sub-classes
//		//*	all have to return an Alpaca Error code
		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(	char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_FindHome(		char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_MoveAxis(		const int axisNum,
															const double moveRate_degPerSec,
															char *alpacaErrMsg);
//
		virtual	TYPE_ASCOM_STATUS	Telescope_Park(			char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SetPark(		char *alpacaErrMsg);
//		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToAltAz(	const double	newAlt_Degrees,
//															const double	newAz_Degrees,
//															char *alpacaErrMsg);
//
		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(	const double	newRtAscen_Hours,
															const double	newDeclination_Degrees,
															char *alpacaErrMsg);
//
//
//
		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(	const double	newRtAscen_Hours,
															const double	newDeclination_Degrees,
															char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_TrackingOnOff(const bool newTrackingState,
															char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_TrackingRate(TYPE_DriveRates newTrackingRate,
															char *alpacaErrMsg);

//
//		virtual	TYPE_ASCOM_STATUS	Telescope_UnPark(		char *alpacaErrMsg);
//
//
//		virtual	int					Telescope_GetLimitSwitchStatus(const TYPE_LIMITSWITCH whichLimit);
		virtual	TYPE_PierSide		Telescope_GetPhysicalSideOfPier(void);
		virtual	TYPE_PierSide		Telescope_CalculateSideOfPier(void);


		//-------------------------------------------------------------------------
		//*	this is for the setup function
		virtual	bool					Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString);
		virtual void					Setup_SaveInit(void);
		virtual void					Setup_SaveFinish(void);
		virtual	bool					Setup_ProcessKeyword(const char *keyword, const char *valueString);

		//===========================================================================
				bool			SendPMC8command(const char *pmc8command);
				bool			ProcessPMC8response(char *pmc8Response);
				void			ProcessAcceleration_RA(int parsedAxisRate);
				void			ProcessAcceleration_DEC(int parsedAxisRate);
				void			ProcessESGI(const char *esgiString);

				//*	these functions are copied from the ASCOM driver written in Visual Basic
				//*	Driver.vb
				double			MotorCounts_to_RA(const int mcValue);
				double			MotorCounts_to_DEC(const int mcValue);
				int				DEC_to_MotorCounts(	double dec_Value_deg,	TYPE_PierSide pierSide);
				int				RA_to_MotorCounts(	double ra_value_hrs,	TYPE_PierSide pierSide);
				void			UpdateRaDec(const int	axis,
											const int	rawMotorPosition,
											const int	motorDirection,
											const int	rawMotorVelocity);

				void			SetPMC8parameters(TYPE_PMC8mount mountType);
				double			CalcTrackingRateFromStepValue(const int highResMicroSteps);
				double			CalcAxisRateFromStepValue(const int normalMicroSteps);

				bool			cTelescopeInfoValid;
				int				cPMC8_ErrCnt;
				TYPE_PMC8mount	cPMC8mountType;
				int				cLX200_OutOfBoundsCnt;
				char			cTelescopeRA_String[32];
				char			cTelescopeDecl_String[32];
				//*	PMC8 raw values
				TYPE_PMC8info	cPMC8;
				void			DumpPMC8data(void);
				int				cAxisRate_RA;
				int				cAxisRate_DEC;
				int				cTrackingRate;
				int				cPosition_RA;
				int				cPosition_DEC;
				int				cFormatErrCnt;
				int				cUSBxmitErrCnt;

				//*	these are the limits for move axis move rate
				TYPE_MoveAxisMode	cMoveAxisMode_RA;
				int					cMoveAxisCurrentStepRate_RA;
				int					cMoveAxisLimit_RA;
				uint32_t			cMoveAxisLastMilliSecs_RA;

				TYPE_MoveAxisMode	cMoveAxisMode_DEC;
				int					cMoveAxisCurrentStepRate_DEC;
				int					cMoveAxisLimit_DEC;
				uint32_t			cMoveAxisLastMilliSecs_DEC;



};

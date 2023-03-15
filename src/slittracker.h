//**************************************************************************
//*	Name:			slittracker.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	May  2,	2020	<MLS> Created slittracker.h
//*****************************************************************************
//#include	"slittracker.h"

#ifndef _SLIT_TRACKER_H_
#define	_SLIT_TRACKER_H_


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif


void	CreateSlitTrackerObjects(void);


#define	kLineBuffSize		64

//*****************************************************************************
typedef struct
{
	bool	validData;
	bool	updated;
	double	distanceInches;
	long	readCount;
} TYPE_SLITCLOCK;

#define	kSlitSensorCnt	12



//**************************************************************************************
class SlitTrackerDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									SlitTrackerDriver(const int argDevNum, const char *devicePath = NULL);
		virtual						~SlitTrackerDriver(void);
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);
		virtual bool				GetCommandArgumentString(const int cmdNumber, char *agumentString);


		TYPE_ASCOM_STATUS	Put_Setrate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_DomeAddress(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_TrackingEnabled(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_TrackingEnabled(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Readall(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		//-------------------------------------------------------------------------
		//*	this is for the setup function
		virtual	bool			Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString);
		virtual void			Setup_SaveInit(void);
		virtual void			Setup_SaveFinish(void);
		virtual	bool			Setup_ProcessKeyword(const char *keyword, const char *valueString);
				void			WriteConfigFile(void);
				bool			cSetupChangeOccured;

				void			OpenSlitTrackerPort(void);
				void			ProcessSlitTrackerLine(char *lineBuff);
				void			GetSlitTrackerData(void);
				void			SendSlitTrackerCmd(const char *cmdBuffer);

				void			ReadSlittrackerConfig(void);

				char			cUSBpath[32];
				char			cSlitTrackerLineBuf[kLineBuffSize];
				int				cSlitTrackerfileDesc;				//*	port file descriptor
				int				cSlitTrackerByteCnt;
				int				cSlitTrackerBufOverflowCnt;

				TYPE_SLITCLOCK	cSlitDistance[kSlitSensorCnt];

				double			cGravity_X;
				double			cGravity_Y;
				double			cGravity_Z;
				double			cGravity_T;

				//---------------------------------------------
				//*	connection to dome for slaving
				char				cDomeIPaddressString[48];
				int					cDomeAlpacaPort;
				int					cDomeAlpacaDevNum;
				TYPE_DomeProperties	cDomeProp;

				TYPE_SlittrackerProperties	cSlitProp;

};


//*****************************************************************************
//*	Slit Tracker commands
//*****************************************************************************
//*****************************************************************************
enum
{
	kCmd_SlitTracker_setrate	=	0,
	kCmd_SlitTracker_DomeAddress,
	kCmd_SlitTracker_TrackingEnabled,
	kCmd_SlitTracker_readall
};


#endif // _SLIT_TRACKER_H_


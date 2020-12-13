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


			TYPE_ASCOM_STATUS	Put_Setrate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
			TYPE_ASCOM_STATUS	Get_Readall(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

				void			OpenSlitTrackerPort(void);
				void			ProcessSlitTrackerLine(char *lineBuff);
				void			GetSlitTrackerData(void);
				void			SendSlitTrackerCmd(const char *cmdBuffer);


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
};


//*****************************************************************************
//*	Slit Tracker commands
//*****************************************************************************
//*****************************************************************************
enum
{
	kCmd_SlitTracker_setrate	=	0,
	kCmd_SlitTracker_readall
};


#endif // _SLIT_TRACKER_H_


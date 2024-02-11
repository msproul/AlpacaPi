//*****************************************************************************
//*		imu_lib.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Aug  4,	2023	<MLS> Created imu_lib.c
//*	Sep 11,	2023	<MLS> Added IMU_Print_Calibration()
//*	Sep 11,	2023	<MLS> Added IMU_IsAvailable(), IMU_GetAverageRoll()
//*	Jan 15,	2024	<MLS> Added IMU_GetRoll_Pitch_Yaw()
//*	Jan 15,	2024	<MLS> Moved IMU averaging from imu_lib_bno055.c to imu_lib.c
//*	Jan 15,	2024	<MLS> The LIS2DH12 has roll off by 90 degrees, fixed in IMU_GetAverageRoll()
//*****************************************************************************

#ifdef _ENABLE_IMU_


#include	<stdbool.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<pthread.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"imu_lib.h"
#include	"imu_lib_bno055.h"
#include	"i2c_bno055.h"
#include	"imu_lib_LIS2DH12.h"


#define	kSampleFreuency	5
#define	kAverageCount	10
//*****************************************************************************
typedef struct
{
	double			heading;
	double			roll;
	double			pitch;
	double			yaw;

} TYPE_IMU_DATA;


//static	bool			gIMU_IsAvailable			=	false;
//static	int				gIMUloopExceededErrCount	=	0;
//static	int				gIMUresetCount				=	0;
//static	bool			gIMU_ThreadIsRunning		=	false;
//static	pthread_mutex_t gMutex	=	PTHREAD_MUTEX_INITIALIZER;
//static long				gTotalDataErrors	=	0;
//static long				gTotalReadCount		=	0;
//
static	pthread_t			gIMUthreadID;

static	bool				gIMU_needsInit				=	true;
static TYPE_IMU_DATA		gIMUdata[kAverageCount];
static TYPE_IMU_DATA		gIMUaverage;
static int					gIMUdataIdx;

#define		kMaxIMUreadCnt	20


static int	gIMU_TypePresent	=	kIMU_type_None;

//*****************************************************************************
int	IMU_Init(void)
{
int		returnCode;

	CONSOLE_DEBUG(__FUNCTION__);

	returnCode	=   IMU_BNO055_Init();
	CONSOLE_DEBUG_W_NUM("IMU_BNO055_Init returned\t=", returnCode);
	if (returnCode == 0)
	{
		gIMU_TypePresent	=   kIMU_type_BNO055;
	}

	//*	if that didnt work, check the next one
	if (gIMU_TypePresent == kIMU_type_None)
	{
		returnCode	=	IMU_LIS2DH12_Init();
		if (returnCode == 0)
		{
			gIMU_TypePresent	=   kIMU_type_LIS2DH12;
		}
	}
	CONSOLE_DEBUG_W_NUM("gIMU_TypePresent\t=", gIMU_TypePresent);
	gIMU_needsInit	=	false;
//	CONSOLE_ABORT(__FUNCTION__);
	return(returnCode);
}


//*****************************************************************************
int		IMU_Print_Calibration(void)
{
int		returnCode	=	0;
	CONSOLE_DEBUG(__FUNCTION__);

	switch(gIMU_TypePresent)
	{
		case kIMU_type_BNO055:
			returnCode	=	IMU_BNO055_Print_Calibration();
			break;

		case kIMU_type_LIS2DH12:
			returnCode	=	-11;
			break;

		default:
			returnCode	=	-11;
			break;
	}

	return(returnCode);
}

//*****************************************************************************
void	IMU_SetDebug(const bool debugOnOff)
{
	CONSOLE_DEBUG(__FUNCTION__);
	BNO055_SetDebug(debugOnOff);
}

//*****************************************************************************
bool	IMU_IsAvailable(void)
{
	return(gIMU_TypePresent != 0);
}

//*****************************************************************************
static int	IMU_GetRoll_Pitch_Yaw(double *rollValue, double *pitchValue, double *yawValue)
{
int	returnCode;

	returnCode	=	-1;
	switch(gIMU_TypePresent)
	{
		case kIMU_type_BNO055:
//			returnCode	=	IMU_BNO055_GetRoll_Pitch_Yaw();
			break;

		case kIMU_type_LIS2DH12:
			returnCode	=	IMU_LIS2DH12_GetRoll_Pitch_Yaw(rollValue, pitchValue, yawValue);
			break;

		default:
			returnCode	=	-1;
			break;
	}
	return(returnCode);
}


//*****************************************************************************
double	IMU_GetAverageRoll(void)
{
double	adjustedRollValue;

	adjustedRollValue	=	gIMUaverage.roll;
	if (gIMU_TypePresent == kIMU_type_LIS2DH12)
	{
		adjustedRollValue	=	gIMUaverage.roll - 90;
	}
//	CONSOLE_DEBUG_W_DBL(__FUNCTION__, gIMUaverage.roll);
	return(adjustedRollValue);
}

//*****************************************************************************
double	IMU_GetAveragePitch(void)
{
//	CONSOLE_DEBUG_W_DBL(__FUNCTION__, gIMUaverage.pitch);
	return(gIMUaverage.pitch);
}

//*****************************************************************************
double	IMU_GetAverageYaw(void)
{
//	CONSOLE_DEBUG_W_DBL(__FUNCTION__, gIMUaverage.yaw);
	return(gIMUaverage.yaw);
}

//*****************************************************************************
static void	IMU_ComputeAverage(void)
{
int			iii;
double		rollSum;
double		pitchSum;
double		yawSum;

//	CONSOLE_DEBUG(__FUNCTION__);

	rollSum		=	0.0;
	pitchSum	=	0.0;
	yawSum		=	0.0;

	for (iii=0; iii<kAverageCount; iii++)
	{
		rollSum		+=	gIMUdata[iii].roll;
		pitchSum	+=	gIMUdata[iii].pitch;
		yawSum		+=	gIMUdata[iii].yaw;
	}

	gIMUaverage.roll	=	rollSum		/ kAverageCount;
	gIMUaverage.pitch	=	pitchSum	/ kAverageCount;
	gIMUaverage.yaw		=	yawSum		/ kAverageCount;

}
//*****************************************************************************
static void	*IMU_BackgroundThread(void *arg)
{
bool		keepRunning;
double		myRoll;
double		myPitch;
double		myYaw;
int			imuRetCode;

	if (arg != NULL)
	{
		CONSOLE_DEBUG("arg is not null");
	}
	keepRunning				=	true;
	while (keepRunning)
	{
		imuRetCode	=	IMU_GetRoll_Pitch_Yaw(&myRoll, &myPitch, &myYaw);
		if (imuRetCode == 0)
		{
			gIMUdata[gIMUdataIdx].roll	=	myRoll;
			gIMUdata[gIMUdataIdx].pitch	=	myPitch;
			gIMUdata[gIMUdataIdx].yaw	=	myYaw;

			//*	check for buffer limits
			gIMUdataIdx++;
			if (gIMUdataIdx > kAverageCount)
			{
				gIMUdataIdx	=	0;
			}
			IMU_ComputeAverage();
		}
		sleep(1);
	}
	return(NULL);
}

//*****************************************************************************
bool	IMU_StartBackgroundThread(void *arg)
{
int			threadErr	=	-1;
int			returnCode;
bool		okToStartThread;

	CONSOLE_DEBUG("***************************************************************");
	CONSOLE_DEBUG(__FUNCTION__);

	memset((void *)gIMUdata, 0, (kAverageCount * sizeof(TYPE_IMU_DATA)));
	gIMUdataIdx		=	0;
	okToStartThread	=	true;
	//*	check to see if the IMU needs to be initialized
	if (gIMU_needsInit)
	{
		returnCode	=	IMU_Init();
		if (returnCode != 0)
		{
			okToStartThread	=	false;
		}
	}

	if (okToStartThread)
	{
		//*	only start the thread if init was successful
		gIMUdataIdx	=	0;
		threadErr	=	pthread_create(&gIMUthreadID, NULL, &IMU_BackgroundThread, arg);
	}
	return(threadErr);
}

#endif // _ENABLE_IMU_

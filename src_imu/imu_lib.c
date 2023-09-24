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
//*****************************************************************************

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"imu_lib.h"
#include	"imu_lib_bno055.h"
#include	"imu_lib_LIS2DH12.h"


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
	BNO055__SetDebug(debugOnOff);
}

//*****************************************************************************
bool	IMU_IsAvailable(void)
{
	return(gIMU_TypePresent != 0);
}


//*****************************************************************************
double	IMU_GetAverageRoll(void)
{
double	averageRoll;

//	IMU_BNO055_ComputeAverage();
//	return(gIMUaverage.roll);
	switch(gIMU_TypePresent)
	{
		case kIMU_type_BNO055:
			averageRoll	=	IMU_BNO055_GetAverageRoll();
			break;

		case kIMU_type_LIS2DH12:
			averageRoll	=	0.0;
			break;

		default:
			averageRoll	=	0.0;
			break;
	}
	return(averageRoll);
}



//*****************************************************************************
bool	IMU_StartBackgroundThread(void)
{
int			threadErr	=	-1;
int			returnCode;
bool		okToStartThread;

//	CONSOLE_DEBUG("***************************************************************");
//	CONSOLE_DEBUG(__FUNCTION__);
////	CONSOLE_ABORT(__FUNCTION__);
//
//	memset((void *)gIMUdata, 0, (kAverageCount * sizeof(TYPE_IMU_DATA)));
//	gIMUdataIdx		=	0;
//	okToStartThread	=	true;
//	//*	check to see if the IMU needs to be initialized
//	if (gIMU_needsInit)
//	{
//		returnCode	=	IMU_BNO055_Init();
//		if (returnCode != 0)
//		{
//			okToStartThread	=	false;
//		}
//	}
//
//	if (okToStartThread)
//	{
//
//		//*	only start the thread if init was successful
//		threadErr		=	pthread_create(&gIMUthreadID, NULL, &IMU_BNO055_BackgroundThread, NULL);
//	}
	return(threadErr);
}




//*****************************************************************************
//#include "imu_lib.h"

#ifndef	_IMU_LIB_H_
#define	_IMU_LIB_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

#ifndef _IMU_LIB_BNO055_H_
//	#include "imu_lib_bno055.h"
#endif

#ifdef __cplusplus
	extern "C" {
#endif


int		IMU_Init(void);
bool	IMU_IsAvailable(void);
void	IMU_SetDebug(const bool debugOnOff);
int		IMU_Print_Calibration(void);

bool	IMU_StartBackgroundThread(void);
double	IMU_GetAverageRoll(void);
double	IMU_GetAveragePitch(void);


//*****************************************************************************
enum
{
	kIMU_type_None		=	0,
	kIMU_type_BNO055,
	kIMU_type_LIS2DH12,

	kIMU_Last
};


#ifdef __cplusplus
}
#endif

#endif // _IMU_LIB_H_

//*****************************************************************************
//#include	"imu_lib_LIS2DH12.h"


#ifndef _IMU_LIB_LIS2DH12_H_
#define	_IMU_LIB_LIS2DH12_H_


int		IMU_LIS2DH12_Init(void);
int		IMU_LIS2DH12_GetRoll_Pitch_Yaw(double *rollValue, double *pitchValue, double *yawValue);


#endif // _IMU_LIB_LIS2DH12_H_

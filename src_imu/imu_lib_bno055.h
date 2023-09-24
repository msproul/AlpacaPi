//*****************************************************************************
//#include "imu_lib_bno055.h"

#ifndef _IMU_LIB_BNO055_H_
#define	_IMU_LIB_BNO055_H_


#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

int		IMU_BNO055_Init(void);
bool	IMU_BNO055_IsAvailable(void);
void	IMU_BNO055_SetDebug(const bool debugOnOff);
int		IMU_BNO055_Read_Euler(double *heading, double *roll, double *pitch);
int		IMU_BNO055_Read_Gravity(double *roll_deg, double *pitch_deg);
int		IMU_BNO055_Read_Quaternion(double *www, double *xxx, double *yyy, double *zzz);
int		IMU_BNO055_Get_Calibration(const int whichUnit);
int		IMU_BNO055_Print_Calibration(void);


//*****************************************************************************
enum
{
	kIMU_System	=	0,
	kIMU_Gyro,
	kIMU_Accelerometer,
	kIMU_Magnetometer,

};

int		IMU_BNO055_Get_Calibration(const int whichUnit);
int		IMU_BNO055_Print_Calibration(void);
int		IMU_BNO055_Load_Calibration(void);
int		IMU_BNO055_Save_Calibration(void);


bool	IMU_BNO055_StartBackgroundThread(void);
double	IMU_BNO055_GetAverageRoll(void);
double	IMU_BNO055_GetAveragePitch(void);


#ifdef __cplusplus
}
#endif

#endif // _IMU_LIB_BNO055_H_

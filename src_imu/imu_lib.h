//*****************************************************************************
//#include "imu_lib.h"

#ifdef __cplusplus
	extern "C" {
#endif

int	IMU_Init(void);
int	IMU_Read_EUL(double *heading, double *roll, double *pitch);


#ifdef __cplusplus
}
#endif
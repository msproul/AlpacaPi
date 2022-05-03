//*****************************************************************************
//*		imu_lib.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	May  2,	2021	<MLS> Created imu_lib.c
//*****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include "getbno055.h"

#include "imu_lib.h"

		int		gBNO_verbose	=	0;
static	char	gSenaddr[256]	=	"0x28";
static	char	gI2C_bus[256]	=	I2CBUS;
static	bool	gIMU_needsInit	=	true;


//*****************************************************************************
int	IMU_Init(void)
{
int				returnCode;

	CONSOLE_DEBUG(__FUNCTION__);

	get_i2cbus(gI2C_bus, gSenaddr);

	returnCode	=	set_mode(imu);
	if (returnCode == 0)
	{
		gIMU_needsInit	=	false;
	}

	return(returnCode);
}

//*****************************************************************************
int	IMU_Read_EUL(double *heading, double *roll, double *pitch)
{
int				returnCode;
struct bnoeul	bnod;

	CONSOLE_DEBUG(__FUNCTION__);
	if (gIMU_needsInit)
	{
		IMU_Init();
	}

	returnCode	=	get_eul(&bnod);
	if (returnCode == 0)
	{
		*heading	=	bnod.eul_head;
		*roll		=	bnod.eul_roll;
		*pitch		=	bnod.eul_pitc;
	}
	return(returnCode);
}

#ifdef _INCLUDE_IMU_MAIN_

//*****************************************************************************
int main(int argc, char *argv[])
{
int				imu_mode;
int				returnCode;
struct bnoeul	bnod;
double			heading;
double			roll;
double			pitch;

	printf("IMU test\r\n");

	returnCode	=	IMU_Init();

	imu_mode	=	get_mode();

	printf("IMU mode = %d \n", imu_mode);

	if (imu_mode < 8)
	{
		printf("Error getting Euler data, sensor mode %d is not a fusion mode.\n", imu_mode);
		exit(-1);
	}

	while (returnCode == 0)
	{
		returnCode	=	IMU_Read_EUL(&heading, &roll, &pitch);

		//*EUL 66.06 -3.00 -15.56 (EUL H R P in Degrees)
		printf("EUL %3.4f %3.4f %3.4f\n", heading, roll, pitch);

		usleep(5000);
	}
}
#endif // _INCLUDE_IMU_MAIN_

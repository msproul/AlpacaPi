//*****************************************************************************
//*		imu_lib_bno055.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	May  2,	2022	<MLS> Created imu_lib_bno055.c
//*	May  3,	2022	<MLS> Added IMU_SetDebug()
//*	Jun  1,	2022	<MLS> Added IMU_StartBackgroundThread()
//*	Jun  1,	2022	<MLS> Added IMU averaging and error checking
//*	Jun  3,	2022	<MLS> Added more error checking for values differing from avg
//*	Jun  5,	2022	<MLS> Added IMU_BNO055_Read_Gravity()
//*	Jun 12,	2022	<MLS> Installed IMU on RA axis of main scope
//*	Jun 17,	2022	<MLS> Replaced angled IMU bracket with straight, MUCH better results
//*	Jun 17,	2022	<MLS> Started separating IMU lib from BNO055 so we can use other IMUs
//*	Jun 17,	2022	<MLS> Added requirements for IMU reading
//*	Jul  7,	2022	<MLS> Added IMU_BNO055_Read_Quaternion()
//*	Aug 23,	2022	<MLS> Added IMU_BNO055_Print_Calibration()
//*	Sep  9,	2022	<MLS> Added IMU_BNO055_Load_Calibration()
//*	Sep  9,	2022	<MLS> Added IMU_Save_Calibration()
//*	Sep  9,	2022	<MLS> Auto loading of IMU calibration working
//*	Sep 20,	2022	<MLS> Added IMU_BNO055_Get_Calibration()
//*	Feb 26,	2023	<MLS> Fixed error in reporting phys side of pier by enabling _USE_GRAVITY_ONLY_
//*	Jun 13,	2023	<MLS> Added IMU_BNO055_IsAvailable()
//*	Aug  4,	2023	<MLS> Changed file name from imu_lib.c to imu_lib_bno055.c
//*****************************************************************************
//*    REQUIREMENTS:
//*        This library is intended to be used on a telescope to measure hour angle of the Right Ascension axis.
//*        The ROLL function is the rotation of that axis
//*        For Right Ascension axis on a German Equatorial Mount
//*        The sensor must be placed at the 12:00 position of the axis, that its
//*        When the counter weights are at the lowest point, the ROLL axis MUST read ZERO
//*        Standing to the south of the telescope, looking along the RA axis at the North Celestial Poll
//*        When the telescope rotates to the right (clockwise), the ROLL angle must be NEGATIVE
//*        When the telescope rotates to the left (counter-clockwise), the ROLL angle must be POSITIVE
//*
//*
//*
//*    CONFIGURATION:
//*        there are many options in this routine that have to be configured
//*            Which IMU, currently supported BMO055
//*            Which orientation, Pitch or Roll (how you have it mounted)
//*            Which way is positive (which way does it point)
//*
//*****************************************************************************

#ifdef _ENABLE_IMU_

#error "IMU should be disabled"

//*	for now, these are compile time flags, maybe changed to config flags later
#define	_USE_BNO055_
#define	_FLIP_ROLL_SIGN_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<stdbool.h>
#include	<pthread.h>
#include	<string.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"imu_lib_bno055.h"


#include	"i2c_bno055.h"

//#define	_DUMP_IMU_ARRAY_
#define	DEGREES(radians)	((radians) * (180.0 / M_PI))
#define	PYTHAGOREAN(x,y,z)	sqrt((x*x)+(y*y)+(z*z))

#define	_USE_GRAVITY_ONLY_
//
//#define	kSampleFreuency	5
#define	kAverageCount	10

static	bool			gIMU_BNO055_needsInit		=	true;
static	int				gIMUloopExceededErrCount	=	0;
static	int				gIMUresetCount				=	0;
#define		kMaxIMUreadCnt	20


//*****************************************************************************
//*	returns 0 if all OK
//*****************************************************************************
int	IMU_BNO055_Init(void)
{
int		returnCode;
int		calibrationRC;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Using BNO055");

	returnCode	=	BNO055_Init();
	if (returnCode == 0)
	{
		gIMU_BNO055_needsInit		=	false;

		calibrationRC	=	IMU_BNO055_Load_Calibration();
		if (calibrationRC == 0)
		{
			CONSOLE_DEBUG("IMU calibration Loaded OK");
		}
		else
		{
			CONSOLE_DEBUG("Loading IMU calibration failed!!!");
//			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed to initialize BNO055!!!");
	}

	return(returnCode);
}

//*****************************************************************************
int		IMU_BNO055_Get_Calibration(const int whichUnit)
{
int				calibrationStatus;
int				myCalibStatus;
struct bnocal	calibrationStruct;

	// read calibration status
	myCalibStatus	=	get_calstatus(&calibrationStruct);
	if (myCalibStatus == 0)
	{
		switch(whichUnit)
		{
			case kIMU_Gyro:
				calibrationStatus	=	calibrationStruct.gcal_st;
				break;

			case kIMU_Accelerometer:
				calibrationStatus	=	calibrationStruct.acal_st;
				break;

			case kIMU_Magnetometer:
				calibrationStatus	=	calibrationStruct.mcal_st;
				break;

			case kIMU_System:
			default:
				calibrationStatus	=	calibrationStruct.scal_st;
				break;

		}
	}
	else
	{
		calibrationStatus	=	0;	//*	uncalibrated
	}
	return(calibrationStatus);
}


//*****************************************************************************
int		IMU_BNO055_Print_Calibration(void)
{
int				myCalibStatus;
struct bnocal	calibrationStruct;
struct bnoinf	bnoInformation;
int				iii;
int				returnCode;

	CONSOLE_DEBUG("****************************************************************");

	BNO055_SetDebug(true);
//	CONSOLE_DEBUG("Calling bno_reset()");
//	bno_reset();
//	sleep(2);
	CONSOLE_DEBUG("****************************************************************");

	// read calibration status
	myCalibStatus	=	get_calstatus(&calibrationStruct);

	get_caloffset(&calibrationStruct);
	get_inf(&bnoInformation);
	print_clksrc();
//+	print_acc_conf(struct bnoaconf *bnoc_ptr);		// print accelerometer config
//	print_mag_conf();			// print magnetometer config
//	print_gyr_conf();			// print gyroscope config

	CONSOLE_DEBUG("****************************************************************");
	BNO055_SetDebug(false);
	BNO055_Print_info();

	for (iii=0; iii<10; iii++)
	{
	double	roll_deg;
	double	pitch_deg;

		returnCode	=	IMU_BNO055_Read_Gravity(&roll_deg, &pitch_deg);
		if (returnCode == 0)
		{
			printf("Roll=%f\tPitch=%f\r\n", roll_deg, pitch_deg);
		}
		else
		{
			CONSOLE_DEBUG("IMU_BNO055_Read_Gravity() returned an error");
		}
		sleep(1);
	}
	BNO055_Print_info();

//	CONSOLE_ABORT(__FUNCTION__);
	return(myCalibStatus);
}

#define	kCalibrationFileName	"BNO055_calibration.cal"

//*****************************************************************************
//*	returns 0 on success
//*****************************************************************************
int		IMU_BNO055_Load_Calibration(void)
{
int		returnCode;

	CONSOLE_DEBUG(__FUNCTION__);
	returnCode	=	load_cal(kCalibrationFileName);
	return(returnCode);
}

//*****************************************************************************
//*	returns 0 on success
//*****************************************************************************
int		IMU_BNO055_Save_Calibration(void)
{
int		returnCode;

	returnCode	=	save_cal(kCalibrationFileName);
	return(returnCode);

}

//*****************************************************************************
void	IMU_BNO055_SetDebug(const bool debugOnOff)
{
	CONSOLE_DEBUG(__FUNCTION__);
	BNO055_SetDebug(debugOnOff);
}

//*****************************************************************************
int	IMU_BNO055_Read_Euler(double *heading, double *roll, double *pitch)
{
int				returnCode;
struct bnoeul	bnod;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (gIMU_BNO055_needsInit)
	{
		returnCode	=	IMU_Init();
		if (returnCode == 0)
		{
			gIMU_BNO055_needsInit	=	false;
		}
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


//*****************************************************************************
int	IMU_BNO055_Read_Quaternion(double *www, double *xxx, double *yyy, double *zzz)
{
int				returnCode;
struct bnoqua	bnoQuat;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (gIMU_BNO055_needsInit)
	{
		returnCode	=	IMU_BNO055_Init();
		if (returnCode == 0)
		{
			gIMU_BNO055_needsInit	=	false;
		}
	}

	returnCode	=	get_qua(&bnoQuat);
	if (returnCode == 0)
	{
		*www	=	bnoQuat.quater_w;
		*xxx	=	bnoQuat.quater_x;
		*yyy	=	bnoQuat.quater_y;
		*zzz	=	bnoQuat.quater_z;
	}
	return(returnCode);
}

//*****************************************************************************
//*	Return:  0 = OK,
//*			anything else = error
//*****************************************************************************
int	IMU_BNO055_Read_Gravity(double *roll_deg, double *pitch_deg)
{
int				returnCode;
struct bnogra	bno_gravity;
double			myRoll;
double			myPitch;
double			vectorTotal;
int				loopCnt;
bool			keepReading;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (gIMU_BNO055_needsInit)
	{
		returnCode	=	IMU_BNO055_Init();
		if (returnCode == 0)
		{
			gIMU_BNO055_needsInit	=	false;
		}
	}

	keepReading	=	true;
	loopCnt		=	0;
	while (keepReading && (loopCnt < kMaxIMUreadCnt))
	{
		returnCode	=	get_gra(&bno_gravity);
		if (returnCode == 0)
		{
			vectorTotal	=	PYTHAGOREAN(	bno_gravity.gravityx,
											bno_gravity.gravityy,
											bno_gravity.gravityz);
			if ((vectorTotal > 9.7) && (vectorTotal < 9.9))
			{
				//*	we have a valid reading
				keepReading	=	false;
				myRoll		=	atan2(bno_gravity.gravityy, bno_gravity.gravityz);
				myPitch		=	atan2(bno_gravity.gravityx, bno_gravity.gravityz);
			#ifdef _FLIP_ROLL_SIGN_
				myRoll		=	-myRoll;
			#endif // _FLIP_ROLL_SIGN_

				*roll_deg	=	DEGREES(myRoll);
				*pitch_deg	=	DEGREES(myPitch);

//				printf("GRA-RAW %3.4f %3.4f %3.4f Roll= %3.4f Pitch= %3.4f\n",
//														bno_gravity.gravityx,
//														bno_gravity.gravityy,
//														bno_gravity.gravityz,
//														DEGREES(myRoll),
//														DEGREES(myPitch));
				returnCode	=	0;
			}
			else
			{
//				CONSOLE_DEBUG_W_DBL("gravityx   \t=",	bno_gravity.gravityx)
//				CONSOLE_DEBUG_W_DBL("gravityy   \t=",	bno_gravity.gravityy)
//				CONSOLE_DEBUG_W_DBL("gravityz   \t=",	bno_gravity.gravityz)
//				CONSOLE_DEBUG_W_DBL("vectorTotal\t=",	vectorTotal)
				returnCode	=	-1;
			}
		}
		loopCnt++;
	}
	if (loopCnt > 1)
	{
	//	CONSOLE_DEBUG_W_NUM("loopCnt\t=",	loopCnt);
		if (loopCnt >= kMaxIMUreadCnt)
		{
			gIMUloopExceededErrCount++;
		}
	}
//	CONSOLE_DEBUG_W_DBL("roll_deg \t=", *roll_deg);
//	CONSOLE_DEBUG_W_DBL("pitch_deg\t=", *pitch_deg);

	return(returnCode);
}

#ifdef _DUMP_IMU_ARRAY_
int	gDumpCounter	=	0;
//*****************************************************************************
static void	DumpIMUarray(void)
{
int		iii;

#define ASCII_ESC 0x1b
	//*	clear the screen using VT100 commands
	printf("%c[2J", ASCII_ESC);
	printf("#%d\n\n", gDumpCounter++);

	for (iii=0; iii<kAverageCount; iii++)
	{
		printf("H=%3.4f R=%3.4f P=%3.4f\n", gIMUdata[iii].heading,
											gIMUdata[iii].roll,
											gIMUdata[iii].pitch);
	}
}
#endif // _DUMP_IMU_ARRAY_


#ifdef _ENABLE_BNO055_THREAD_
//
//#ifdef	_USE_GRAVITY_ONLY_
//
////*****************************************************************************
//static void	*IMU_BNO055_BackgroundThread(void *arg)
//{
//int				returnCode;
//double			roll_deg;
//double			pitch_deg;
//bool			keepRunning;
//int				delay_MicroSecs;
//bool			validReading;
//
//	CONSOLE_DEBUG(__FUNCTION__);
//	if (arg != NULL)
//	{
//		CONSOLE_DEBUG("Dont know what to do with arg");
//	}
//	gIMU_ThreadIsRunning	=	true;
//	delay_MicroSecs			=	1000000 / kSampleFreuency;
//	keepRunning				=	true;
//	while (keepRunning)
//	{
//		returnCode	=	IMU_BNO055_Read_Gravity(&roll_deg, &pitch_deg);
//		if (returnCode == 0)
//		{
////			printf("GRA R=%3.4f P=%3.4f %Y=3.4f\n", roll_deg, pitch_deg, yaw_deg);
////			printf("GRA R=%3.4f P=%3.4f\n", roll_deg, pitch_deg);
//
//			gTotalReadCount++;
//
//			validReading	=	true;
//			//*	occasionally there is a reading that is totally off, lets try to catch it
//			if (fabs(roll_deg) > 360.0)
//			{
//				validReading	=	false;
//				CONSOLE_DEBUG_W_DBL("roll_deg     \t=", roll_deg);
//			}
//			if (fabs(pitch_deg) > 360.0)
//			{
//				validReading	=	false;
//				CONSOLE_DEBUG_W_DBL("pitch_deg        \t=", pitch_deg);
//			}
//
//			if (validReading)
//			{
//				//*	lock the data to avoid race conditions
//				pthread_mutex_lock(&gMutex);
//
//				//*	save the data in the array for averaging
//				gIMUdata[gIMUdataIdx].roll		=	roll_deg;
//				gIMUdata[gIMUdataIdx].pitch		=	pitch_deg;
////				gIMUdata[gIMUdataIdx].yaw		=	yaw_deg;
//
//				//*	unlock
//				pthread_mutex_unlock(&gMutex);
//
//				//*	update array index
//				gIMUdataIdx++;
//				if (gIMUdataIdx >= kAverageCount)
//				{
//				#ifdef _DUMP_IMU_ARRAY_
//					DumpIMUarray();
//				#endif // _DUMP_IMU_ARRAY_
//					//*	reset array index
//					gIMUdataIdx	=	0;
//				}
//			}
//			else
//			{
//			double	errorPercent;
//
//				gTotalDataErrors++;
//				errorPercent	=	(gTotalDataErrors * 100.0) / gTotalReadCount;
//				CONSOLE_DEBUG_W_LONG("gTotalDataErrors\t=", gTotalDataErrors);
//				CONSOLE_DEBUG_W_LONG("gTotalReadCount \t=", gTotalReadCount);
//				CONSOLE_DEBUG_W_DBL("errorPercent     \t=", errorPercent);
//			}
//		}
//		else
//		{
//			CONSOLE_DEBUG("Error reading IMU");
//			if (gIMUloopExceededErrCount > 5)
//			{
//				gIMUresetCount++;
//				gIMU_BNO055_needsInit	=	true;
//			}
//		}
//		usleep(delay_MicroSecs);	//*	sleep in micro-seconds
//	}
//	gIMU_ThreadIsRunning	=	false;
//	return(NULL);
//}
//
//#else
//
//#define	kReadAvgCnt	3
//
////*****************************************************************************
//static void	*IMU_BNO055_BackgroundThread(void *arg)
//{
//int				returnCode;
//double			heading[kReadAvgCnt];
//double			roll[kReadAvgCnt];
//double			pitch[kReadAvgCnt];
//bool			keepRunning;
//int				delay_MicroSecs;
//double			aggregateValue;
//bool			validReading;
//int				iii;
//int				tryCounter;
//double			headingSum;
//double			rollSum;
//double			pitchSum;
//double			headingAvg;
//double			rollAvg;
//double			pitchAvg;
//double			headingDif;
//double			rollDif;
//double			pitchDif;
//double			totalDif;
//
//	CONSOLE_DEBUG(__FUNCTION__);
//
//	gIMU_ThreadIsRunning	=	true;
//	delay_MicroSecs			=	1000000 / kSampleFreuency;
//	keepRunning				=	true;
//	while (keepRunning)
//	{
//		//*	we are going to take 3 readings, and compute the average,
//		validReading	=	false;
//		tryCounter		=	0;
//		while ((validReading == false) && (tryCounter < 10))
//		{
//			headingSum	=	0.0;
//			rollSum		=	0.0;
//			pitchSum	=	0.0;
//			for (iii=0; iii<kReadAvgCnt; iii++)
//			{
//				returnCode	=	IMU_BNO055_Read_Euler(&heading[iii], &roll[iii], &pitch[iii]);
//				if (returnCode == 0)
//				{
//					headingSum	+=	heading[iii];
//					rollSum		+=	roll[iii];
//					pitchSum	+=	pitch[iii];
//				}
//				usleep(3000);
//			}
//			headingAvg	=	headingSum / kReadAvgCnt;
//			rollAvg		=	rollSum / kReadAvgCnt;
//			pitchAvg	=	pitchSum / kReadAvgCnt;
//
//			headingDif	=	0.0;
//			rollDif		=	0.0;
//			pitchDif	=	0.0;
//			for (iii=0; iii<kReadAvgCnt; iii++)
//			{
//				headingDif	+=	fabs(headingAvg - heading[iii]);
//				rollDif		+=	fabs(rollAvg - roll[iii]);
//				pitchDif	+=	fabs(pitchAvg - pitch[iii]);
//			}
//			//*	now compute the total diff
//			totalDif	=	headingDif + rollDif + pitchDif;
//
//			if (totalDif < 0.1)
//			{
//			//	printf("EUL %3.4f %3.4f %3.4f\n", headingAvg, rollAvg, pitchAvg);
//				validReading	=	true;
//				returnCode		=	0;
//			}
//			else
//			{
////				CONSOLE_DEBUG_W_NUM("tryCounter\t=", tryCounter);
////				CONSOLE_DEBUG_W_DBL("totalDif\t=", totalDif);
//			}
//			tryCounter++;
//		}
////		returnCode	=	IMU_BNO055_Read_Euler(&heading, &roll, &pitch);
//		if (returnCode == 0)
//		{
//			gTotalReadCount++;
//
//			validReading	=	true;
//			//*	occasionally there is a reading that is totally off, lets try to catch it
//			if (fabs(headingAvg) > 360.0)
//			{
//				validReading	=	false;
//				CONSOLE_DEBUG_W_DBL("heading     \t=", headingAvg);
//			}
//			if (fabs(rollAvg) > 360.0)
//			{
//				validReading	=	false;
//				CONSOLE_DEBUG_W_DBL("roll        \t=", rollAvg);
//			}
//			if (fabs(pitchAvg) > 360.0)
//			{
//				validReading	=	false;
//				CONSOLE_DEBUG_W_DBL("pitch       \t=", pitchAvg);
//			}
//
//			if (validReading)
//			{
//				//*	lock the data to avoid race conditions
//				pthread_mutex_lock(&gMutex);
//
//				//*	save the data in the array for averaging
//				gIMUdata[gIMUdataIdx].heading	=	headingAvg;
//				gIMUdata[gIMUdataIdx].roll		=	rollAvg;
//				gIMUdata[gIMUdataIdx].pitch		=	pitchAvg;
//
//				//*	unlock
//				pthread_mutex_unlock(&gMutex);
//
//				//*	update array index
//				gIMUdataIdx++;
//				if (gIMUdataIdx >= kAverageCount)
//				{
//				#ifdef _DUMP_IMU_ARRAY_
//					DumpIMUarray();
//				#endif // _DUMP_IMU_ARRAY_
//					//*	reset array index
//					gIMUdataIdx	=	0;
//				}
//			}
//			else
//			{
//			double	errorPercent;
//
//				gTotalDataErrors++;
//				errorPercent	=	(gTotalDataErrors * 100.0) / gTotalReadCount;
//				CONSOLE_DEBUG_W_LONG("gTotalDataErrors\t=", gTotalDataErrors);
//				CONSOLE_DEBUG_W_LONG("gTotalReadCount \t=", gTotalReadCount);
//				CONSOLE_DEBUG_W_DBL("errorPercent     \t=", errorPercent);
//			}
//		}
//		else
//		{
//			CONSOLE_DEBUG("Error reading IMU");
//		}
//		usleep(delay_MicroSecs);	//*	sleep in micro-seconds
//	}
//	gIMU_ThreadIsRunning	=	false;
//	return(NULL);
//}
//#endif // _USE_GRAVITY_ONLY_


#endif // _ENABLE_BNO055_THREAD_

#ifdef _INCLUDE_IMU_MAIN_

//#define	_USE_BACKGROUND_THREAD_

//*****************************************************************************
int main(int argc, char *argv[])
{
int				imu_mode;
int				returnCode;
struct bnoeul	bnod;
double			heading;
double			roll;
double			pitch;
struct bnogra	bno_gravity;
double			myRoll_rad;
double			myPitch_rad;

	printf("IMU test\r\n");

#ifdef _USE_BACKGROUND_THREAD_
	IMU_BNO055_StartBackgroundThread();

	while (true)
	{
		roll	=	IMU_BNO055_GetAverageRoll();
		pitch	=	IMU_BNO055_GetAveragePitch();
		printf("EUL R=%3.4f P=%3.4f\n", roll, pitch);

		usleep(5000);
	}

#else
	returnCode	=	IMU_BNO055_Init();

	imu_mode	=	get_mode();

	printf("IMU mode = %d \n", imu_mode);

	if (imu_mode < 8)
	{
		printf("Error getting Euler data, sensor mode %d is not a fusion mode.\n", imu_mode);
		exit(-1);
	}

	while (returnCode == 0)
	{
		returnCode	=	get_gra(&bno_gravity);
		myRoll_rad	=	atan2(bno_gravity.gravityy, bno_gravity.gravityz);
		myPitch_rad	=	atan2(bno_gravity.gravityx, bno_gravity.gravityz);


		printf("gravity x=%3.4f y=%3.4f z=%3.4f V=%3.4f   R=%f  P=%f\n",
								bno_gravity.gravityx,
								bno_gravity.gravityy,
								bno_gravity.gravityz,
								PYTHAGOREAN(bno_gravity.gravityx,
											bno_gravity.gravityy,
											bno_gravity.gravityz),
								DEGREES(myRoll_rad),
								DEGREES(myPitch_rad));


		sleep(1);
	}

//	while (returnCode == 0)
//	{
//		returnCode	=	IMU_BNO055_Read_Euler(&heading, &roll, &pitch);
//
//		//*EUL 66.06 -3.00 -15.56 (EUL H R P in Degrees)
//		printf("EUL %3.4f %3.4f %3.4f\n", heading, roll, pitch);
//
//		usleep(5000);
//	}
#endif
}
#endif // _INCLUDE_IMU_MAIN_


#endif // _ENABLE_IMU_

//**************************************************************************
//*	Name:			obsconditionsdriver_rpi.cpp
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*		https://www.raspberrypi.org/documentation/hardware/sense-hat/
//*		sudo apt install sense-hat
//*		/usr/src/sense-hat/examples.
//*		https://github.com/davebm1/c-sense-hat
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec 31,	2019	<MLS> Started on obsconditionsdriver_rpi.cpp
//*	Dec 31,	2019	<MLS> Added ReadPressure(), ReadTemperature(), ReadHumidity()
//*	Dec 31,	2019	<MLS> Working with RTIMULib on R-Pi
//*	Dec 31,	2019	<MLS> The Temperature value on the R-Pi is not valid for use
//*****************************************************************************

#ifdef _ENABLE_OBSERVINGCONDITIONS_RPI_


#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<time.h>




#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#ifdef _ENABLE_PI_HAT_SESNSOR_BOARD_
	#include	<unistd.h>
	#include	<linux/i2c-dev.h>
//	#include	<i2c/smbus.h>
	#include	<fcntl.h>
	#include	<sys/ioctl.h>
#endif

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"obsconditionsdriver.h"
#include	"obsconditionsdriver_rpi.h"
#include	"obsconditions_globals.h"

//*****************************************************************************
void	CreateObsConditionObjects_RPi(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	new ObsConditionsDriverRpi(0);
}

//**************************************************************************************
ObsConditionsDriverRpi::ObsConditionsDriverRpi(const int argDevNum)
	:ObsConditionsDriver(argDevNum)
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name, "RPiSenseHat");
	strcpy(gEnvData.siteDataSource,	"Data source:Raspberry Pi Sense hat");
	strcpy(gEnvData.domeDataSource,	"Data source:Raspberry Pi Sense hat");

#ifdef _ENABLE_RTIMULib_
	strcpy(cCommonProp.Description, "R-Pi dome conditions");
//	strcat(cCommonProp.Name, "-RTIMULib");

	cObsConditionProp.DewPoint.IsSupported			=	true;
	cObsConditionProp.Humidity.IsSupported			=	true;
	cObsConditionProp.Pressure.IsSupported			=	true;
	cObsConditionProp.Temperature.IsSupported		=	true;

	rt_settings	=	new RTIMUSettings("RTIMULib");
	if (rt_settings != NULL)
	{
		rt_imu		=	RTIMU::createIMU(rt_settings);
		rt_pressure	=	RTPressure::createPressure(rt_settings);
		rt_humidity	=	RTHumidity::createHumidity(rt_settings);

	   //  This is an opportunity to manually override any settings before the call IMUInit

		//  set up IMU

		rt_imu->IMUInit();

		//  this is a convenient place to change fusion parameters
		rt_imu->setSlerpPower(0.02);
		rt_imu->setGyroEnable(true);
		rt_imu->setAccelEnable(true);
		rt_imu->setCompassEnable(true);

		//  set up pressure sensor
		if (rt_pressure != NULL)
		{
			rt_pressure->pressureInit();
		}

		//  set up humidity sensor
		if (rt_humidity != NULL)
		{
			rt_humidity->humidityInit();
		}
		UpdateSensorsReadings();
	}
	else
	{
		CONSOLE_DEBUG("Failed to create RTIMUSettings");
	}

#endif	//	_ENABLE_RTIMULib_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ObsConditionsDriverRpi::~ObsConditionsDriverRpi(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
double	ObsConditionsDriverRpi::ReadPressure_kPa(void)
{
double	pressureReading	=	0.0;
#ifdef _ENABLE_RTIMULib_
RTIMU_DATA	imuData;

	imuData	=	rt_imu->getIMUData();
	if (rt_pressure != NULL)
	{
		rt_pressure->pressureRead(imuData);	//*	returns hPascals
		pressureReading	=	imuData.pressure / 10.0;
		cSuccesfullReadCnt++;
	}
#endif	//	_ENABLE_RTIMULib_

	return(pressureReading);
}

//*****************************************************************************
double	ObsConditionsDriverRpi::ReadTemperature(void)
{
double	temperatureReading	=	0.0;
#ifdef _ENABLE_RTIMULib_
RTIMU_DATA	imuData;

	imuData	=	rt_imu->getIMUData();
	if (rt_pressure != NULL)
	{
		rt_pressure->pressureRead(imuData);
		temperatureReading		=	imuData.temperature;

		cSuccesfullReadCnt++;
	}
#endif	//	_ENABLE_RTIMULib_
	return(temperatureReading);
}

//*****************************************************************************
double	ObsConditionsDriverRpi::ReadHumidity(void)
{
double	humidityReading	=	0.0;
#ifdef _ENABLE_RTIMULib_
RTIMU_DATA	imuData;

	imuData	=	rt_imu->getIMUData();

	//  add the humidity data to the structure
	if (rt_humidity != NULL)
	{
		rt_humidity->humidityRead(imuData);
		humidityReading	=	imuData.humidity;

		cSuccesfullReadCnt++;
	}
#endif	//	_ENABLE_RTIMULib_
	return(humidityReading);
}


//*****************************************************************************
//*	this MUST be implemented by the sub class
//*	change the ones that the sub class supports
//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriverRpi::GetSensorInfo(	TYPE_ObsConSensorType sensorType,
														char	*description,
														double	*timeSinceLastUpdate)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	strcpy(description, "AlpacaPi: Not implemented");

	switch(sensorType)
	{
		case kSensor_CloudCover:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_DewPoint:
			alpacaErrCode	=	kASCOM_Err_Success;
			strcpy(description, "AlpacaPi: Calculated from temp/humidity");
			break;

		case kSensor_Humidity:
			alpacaErrCode	=	kASCOM_Err_Success;
			strcpy(description, "AlpacaPi: Raspberry Pi sense-hat");
			*timeSinceLastUpdate	=	1.234;
			break;

		case kSensor_Pressure:
			alpacaErrCode	=	kASCOM_Err_Success;
			strcpy(description, "AlpacaPi: Raspberry Pi sense-hat");
			*timeSinceLastUpdate	=	1.234;
			break;

		case kSensor_RainRate:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_SkyBrightness:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_SkyQuality:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_StarFWHM:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_SkyTemperature:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_Temperature:
			alpacaErrCode	=	kASCOM_Err_Success;
			strcpy(description, "AlpacaPi: Raspberry Pi sense-hat");
			*timeSinceLastUpdate	=	1.234;
			break;

		case kSensor_WindDirection:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_WindGust:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_WindSpeed:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		default:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;
	}
	return(alpacaErrCode);

}


#pragma mark -


//*****************************************************************************
#ifdef _ENABLE_PI_HAT_SESNSOR_BOARD_
//*****************************************************************************
#define		DEV_ID_Pesure	0x5c
#define		DEV_PATH		"/dev/i2c-1"
#define		WHO_AM_I		0x0F
#define		CTRL_REG1		0x20
#define		CTRL_REG2		0x21
#define		PRESS_OUT_XL	0x28
#define		PRESS_OUT_L		0x29
#define		PRESS_OUT_H		0x2A
#define		Pres_TEMP_OUT_L		0x2B
#define		Pres_TEMP_OUT_H		0x2C

//#define	kPascals_to_lbf_in2_Constant		(1290320000.0 / 8896443230521.0)		//*	http://www.calualteme.com/Pressure


//*****************************************************************************
static void Delay_ms(int t)
{
	usleep(t * 1000);
}


//*****************************************************************************
static void	PrintValues(const char *sourceString, double temp_degC, double pressure, double humidity)
{

	printf("Temp (%s)\t= %.2f deg-C\t%.2f deg-F\t",	sourceString, temp_degC, DEGREES_F(temp_degC));
	printf("Pressure = %.1f hPa\t",					pressure);
	printf("Humidity = %.1f%% rH\t",				humidity);
	printf("\r\n");

}


//*****************************************************************************
void	ObsConditionsDriverRpi::ReadSenseHat_Pressure(void)
{
int		fd				=	0;
uint8_t	temp_out_l		=	0;
uint8_t	temp_out_h		=	0;
int16_t	temp_out		=	0;
double	temp_degC		=	0.0;
uint8_t	press_out_xl	=	0;
uint8_t	press_out_l		=	0;
uint8_t	press_out_h		=	0;
int32_t	press_out		=	0;
double	pressure		=	0.0;
uint8_t	status			=	0;
int		loopCntr;


	///* open i2c comms
	fd	=	open(DEV_PATH, O_RDWR);
	if (fd >= 0)
	{
		//*	configure i2c slave
		if (ioctl(fd, I2C_SLAVE, DEV_ID_Pesure) >= 0)
		{
			//*	check we are who we should be
			if (i2c_smbus_read_byte_data(fd, WHO_AM_I) == 0xBD)
			{
				//*	Power down the device (clean start)
				i2c_smbus_write_byte_data(fd, CTRL_REG1, 0x00);

				//*	Turn on the pressure sensor analog front end in single shot mode
				i2c_smbus_write_byte_data(fd, CTRL_REG1, 0x84);

				//*	Run one-shot measurement (temperature and pressure), the set bit will be reset by the
				//*	sensor itself after execution (self-clearing bit)
				i2c_smbus_write_byte_data(fd, CTRL_REG2, 0x01);

				//*	Wait until the measurement is complete
				status		=	1;
				loopCntr	=	0;
				while ((status != 0) && (loopCntr < 25))
				{
					Delay_ms(25);		//*	25 milliseconds
					status	=	i2c_smbus_read_byte_data(fd, CTRL_REG2);
					loopCntr++;
				}

				if (status == 0)
				{
					//*	Read the temperature measurement (2 bytes to read)
					temp_out_l		=	i2c_smbus_read_byte_data(fd, Pres_TEMP_OUT_L);
					temp_out_h		=	i2c_smbus_read_byte_data(fd, Pres_TEMP_OUT_H);

					//*	Read the pressure measurement (3 bytes to read)
					press_out_xl	=	i2c_smbus_read_byte_data(fd, PRESS_OUT_XL);
					press_out_l		=	i2c_smbus_read_byte_data(fd, PRESS_OUT_L);
					press_out_h		=	i2c_smbus_read_byte_data(fd, PRESS_OUT_H);

					//*	make 16 and 24 bit values (using bit shift)
					temp_out		=	temp_out_h << 8 | temp_out_l;
					press_out		=	press_out_h << 16 | press_out_l << 8 | press_out_xl;

					//*	calculate output values
					temp_degC		=	42.5 + (temp_out / 480.0);
					pressure		=	press_out / 4096.0;


					cCurrentPressure_kPa	=	pressure;
					cObsConditionProp.Temperature.Value		=	temp_degC;

					//*	output
					PrintValues("from press",	cObsConditionProp.Temperature.Value,
												cCurrentPressure_kPa,
												cObsConditionProp.Humidity.Value);

					//*	Power down the device
					i2c_smbus_write_byte_data(fd, CTRL_REG1, 0x00);

					cSuccesfullReadCnt++;

				}
			}
			else
			{
			//	printf("%s\n", "who_am_i error");
				CONSOLE_DEBUG("who_am_i error");
			}
		}
		else
		{
			perror("Unable to configure i2c slave device");
		}
		close(fd);
	}
	else
	{
		perror("Unable to open i2c device");
	}
}

//*****************************************************************************
#define		DEV_PATH		"/dev/i2c-1"
#define		DEV_ID_Humidity	0x5F
//#define		WHO_AM_I		0x0F

//#define		CTRL_REG1		0x20
//#define		CTRL_REG2		0x21

#define		T0_OUT_L		0x3C
#define		T0_OUT_H		0x3D
#define		T1_OUT_L		0x3E
#define		T1_OUT_H		0x3F
#define		T0_degC_x8		0x32
#define		T1_degC_x8		0x33
#define		T1_T0_MSB		0x35

#define		TEMP_OUT_L		0x2A
#define		TEMP_OUT_H		0x2B

#define		H0_T0_OUT_L		0x36
#define		H0_T0_OUT_H		0x37
#define		H1_T0_OUT_L		0x3A
#define		H1_T0_OUT_H		0x3B
#define		H0_rH_x2		0x30
#define		H1_rH_x2		0x31

#define		H_T_OUT_L		0x28
#define		H_T_OUT_H		0x29

//*****************************************************************************
void	ObsConditionsDriverRpi::ReadSenseHat_Humidity(void)
{
int		fd		=	0;
uint8_t status	=	0;
int		loopCntr;
double	T0_DegC;
double	T1_DegC;
double	H0_rH;
double	H1_rH;
double	t_gradient_m;
double	t_intercept_c;
double	h_gradient_m;
double	h_intercept_c;
uint8_t	t_out_l;
uint8_t	t_out_h;

	//*	open i2c comms
	fd	=	open(DEV_PATH, O_RDWR);
	if (fd >= 0)
	{
		//*	configure i2c slave
		if (ioctl(fd, I2C_SLAVE, DEV_ID_Humidity) >= 0)
		{
			//*	check we are who we should be
			if (i2c_smbus_read_byte_data(fd, WHO_AM_I) == 0xBC)
			{
				//*	Power down the device (clean start)
				i2c_smbus_write_byte_data(fd, CTRL_REG1, 0x00);

				//*	Turn on the humidity sensor analog front end in single shot mode
				i2c_smbus_write_byte_data(fd, CTRL_REG1, 0x84);

				//*	Run one-shot measurement (temperature and humidity). The set bit will be reset by the
				//*	sensor itself after execution (self-clearing bit)
				i2c_smbus_write_byte_data(fd, CTRL_REG2, 0x01);

				//*	Wait until the measurement is completed
				status		=	1;
				loopCntr	=	0;
				while ((status != 0) && (loopCntr < 25))
				{
					Delay_ms(25);		//*	25 milliseconds
					status	=	i2c_smbus_read_byte_data(fd, CTRL_REG2);
					loopCntr++;
				}

				if (status == 0)
				{
					//*	Read calibration temperature LSB (ADC) data
					//*	(temperature calibration x-data for two points)
					uint8_t t0_out_l	=	i2c_smbus_read_byte_data(fd, T0_OUT_L);
					uint8_t t0_out_h	=	i2c_smbus_read_byte_data(fd, T0_OUT_H);
					uint8_t t1_out_l	=	i2c_smbus_read_byte_data(fd, T1_OUT_L);
					uint8_t t1_out_h	=	i2c_smbus_read_byte_data(fd, T1_OUT_H);

					//*	Read calibration temperature (°C) data
					//*	(temperature calibration y-data for two points)
					uint8_t t0_degC_x8	=	i2c_smbus_read_byte_data(fd, T0_degC_x8);
					uint8_t t1_degC_x8	=	i2c_smbus_read_byte_data(fd, T1_degC_x8);
					uint8_t t1_t0_msb	=	i2c_smbus_read_byte_data(fd, T1_T0_MSB);

					//*	Read calibration relative humidity LSB (ADC) data
					//*	(humidity calibration x-data for two points)
					uint8_t h0_out_l	=	i2c_smbus_read_byte_data(fd, H0_T0_OUT_L);
					uint8_t h0_out_h	=	i2c_smbus_read_byte_data(fd, H0_T0_OUT_H);
					uint8_t h1_out_l	=	i2c_smbus_read_byte_data(fd, H1_T0_OUT_L);
					uint8_t h1_out_h	=	i2c_smbus_read_byte_data(fd, H1_T0_OUT_H);

					//*	Read relative humidity (% rH) data
					//*	(humidity calibration y-data for two points)
					uint8_t h0_rh_x2	=	i2c_smbus_read_byte_data(fd, H0_rH_x2);
					uint8_t h1_rh_x2	=	i2c_smbus_read_byte_data(fd, H1_rH_x2);

					//*	make 16 bit values (bit shift)
					//*	(temperature calibration x-values)
					int16_t T0_OUT		=	t0_out_h << 8 | t0_out_l;
					int16_t T1_OUT		=	t1_out_h << 8 | t1_out_l;

					//*	make 16 bit values (bit shift)
					//*	(humidity calibration x-values)
					int16_t H0_T0_OUT	=	h0_out_h << 8 | h0_out_l;
					int16_t H1_T0_OUT	=	h1_out_h << 8 | h1_out_l;

					//*	make 16 and 10 bit values (bit mask and bit shift)
					uint16_t T0_DegC_x8	=	(t1_t0_msb & 3) << 8 | t0_degC_x8;
					uint16_t T1_DegC_x8	=	((t1_t0_msb & 12) >> 2) << 8 | t1_degC_x8;

					//*	Calculate calibration values
					//*	(temperature calibration y-values)
					T0_DegC		=	T0_DegC_x8 / 8.0;
					T1_DegC		=	T1_DegC_x8 / 8.0;

					//*	Humidity calibration values
					//*	(humidity calibration y-values)
					H0_rH		=	h0_rh_x2 / 2.0;
					H1_rH		=	h1_rh_x2 / 2.0;

					//*	Solve the linear equasions 'y = mx + c' to give the
					//*	calibration straight line graphs for temperature and humidity
					t_gradient_m		=	(T1_DegC - T0_DegC) / (T1_OUT - T0_OUT);
					t_intercept_c	=	T1_DegC - (t_gradient_m * T1_OUT);

					h_gradient_m		=	(H1_rH - H0_rH) / (H1_T0_OUT - H0_T0_OUT);
					h_intercept_c	=	H1_rH - (h_gradient_m * H1_T0_OUT);

					//*	Read the ambient temperature measurement (2 bytes to read)
					t_out_l	=	i2c_smbus_read_byte_data(fd, TEMP_OUT_L);
					t_out_h	=	i2c_smbus_read_byte_data(fd, TEMP_OUT_H);

					//*	make 16 bit value
					int16_t T_OUT	=	t_out_h << 8 | t_out_l;

					//*	Read the ambient humidity measurement (2 bytes to read)
					uint8_t h_t_out_l	=	i2c_smbus_read_byte_data(fd, H_T_OUT_L);
					uint8_t h_t_out_h	=	i2c_smbus_read_byte_data(fd, H_T_OUT_H);

					//*	make 16 bit value
					int16_t H_T_OUT	=	h_t_out_h << 8 | h_t_out_l;

					//*	Calculate ambient temperature
					double temp_degC	=	(t_gradient_m * T_OUT) + t_intercept_c;

					//*	Calculate ambient humidity
					double H_rH	=	(h_gradient_m * H_T_OUT) + h_intercept_c;

					cObsConditionProp.Temperature.Value	=	temp_degC;
					cObsConditionProp.Humidity.Value	=	H_rH;
					//*	Output
					PrintValues("from humid",	cObsConditionProp.Temperature.Value,
												cCurrentPressure_kPa,
												cObsConditionProp.Humidity.Value);

					//*	Power down the device
					i2c_smbus_write_byte_data(fd, CTRL_REG1, 0x00);

					cSuccesfullReadCnt++;

				}
			}
			else
			{
				printf("%s\n", "who_am_i error");
			}
		}
		else
		{
			perror("Unable to configure i2c slave device");
		}
		close(fd);
	}
	else
	{
		perror("Unable to open i2c device");
	}
}
#endif	//	_ENABLE_PI_HAT_SESNSOR_BOARD_



#endif	//	_ENABLE_OBSERVINGCONDITIONS_RPI_


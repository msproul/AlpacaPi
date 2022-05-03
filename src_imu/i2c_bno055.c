/* ------------------------------------------------------------ *
 * file:        i2c_bno055.c                                    *
 * purpose:     Extract sensor data from Bosch BNO055 modules.  *
 *              Functions for I2C bus communication, get and    *
 *              set sensor register data. Ths file belongs to   *
 *              the pi-bno055 package. Functions are called     *
 *              from getbno055.c, globals are in getbno055.h.   *
 *                                                              *
 * Requires:	I2C development packages i2c-tools libi2c-dev   *
 *                                                              *
 * author:      07/14/2018 Frank4DD                             *
 * ------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include "getbno055.h"

#include "i2c_bno055.h"

/* ------------------------------------------------------------ *
 * global variables                                             *
 * ------------------------------------------------------------ */
int		i2cfd;			// I2C file descriptor

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_i2cbus() - Enables the I2C bus communication. Raspberry  *
 * Pi 2 uses i2c-1, RPI 1 used i2c-0, NanoPi also uses i2c-0.   *
 * ------------------------------------------------------------ */
//**************************************************************************************
void get_i2cbus(char *i2cbus, char *i2caddr)
{

	if ((i2cfd = open(i2cbus, O_RDWR)) < 0)
	{
		printf("Error failed to open I2C bus [%s].\n", i2cbus);
		exit(-1);
	}
	if (gBNO_verbose == 1) printf("Debug: I2C bus device: [%s]\n", i2cbus);
	/* --------------------------------------------------------- *
	* Set I2C device (BNO055 I2C address is  0x28 or 0x29)      *
	* --------------------------------------------------------- */
	int addr	=	(int)strtol(i2caddr, NULL, 16);
	if	(gBNO_verbose == 1) printf("Debug: Sensor address: [0x%02X]\n", addr);

	if	(ioctl(i2cfd, I2C_SLAVE, addr) != 0)
	{
		printf("Error can't find sensor at address [0x%02X].\n", addr);
		exit(-1);
	}
	/* --------------------------------------------------------- *
	* I2C communication test is the only way to confirm success *
	* --------------------------------------------------------- */
	char reg	=	BNO055_CHIP_ID_ADDR;
	if	(write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure register [0x%02X], sensor addr [0x%02X]?\n", reg, addr);
		exit(-1);
	}
}

//**************************************************************************************
/* --------------------------------------------------------------- *
 * bno_dump() dumps the register map data.                         *
 * --------------------------------------------------------------- */
//**************************************************************************************
int bno_dump(void)
{
	int count	=	0;

	printf("------------------------------------------------------\n");
	printf("BNO055 page-0:\n");
	printf("------------------------------------------------------\n");
	printf(" reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	printf("------------------------------------------------------\n");
	while(count < 8)
	{
		char reg	=	count;
		if (write(i2cfd, &reg, 1) != 1)
		{
			printf("Error: I2C write failure for register 0x%02X\n", reg);
			exit(-1);
		}

		char data[16]	=	{0};
		if (read(i2cfd, &data, 16) != 16)
		{
			printf("Error: I2C read failure for register 0x%02X\n", reg);
			exit(-1);
		}
		printf("[0x%02X] %02X %02X %02X %02X %02X %02X %02X %02X",
				 (reg*16), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
		printf(" %02X %02X %02X %02X %02X %02X %02X %02X\n",
				 data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
		count++;
	}

	set_page1();
	usleep(50 * 1000);
	count	=	0;
	printf("------------------------------------------------------\n");
	printf("BNO055 page-1:\n");
	printf("------------------------------------------------------\n");
	printf(" reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	printf("------------------------------------------------------\n");
	while(count < 8)
	{
		char reg	=	count;
		if (write(i2cfd, &reg, 1) != 1)
		{
			printf("Error: I2C write failure for register 0x%02X\n", reg);
			exit(-1);
		}

		char data[16]	=	{0};
		if (read(i2cfd, &data, 16) != 16)
		{
			printf("Error: I2C read failure for register 0x%02X\n", reg);
			exit(-1);

		}
		printf("[0x%02X] %02X %02X %02X %02X %02X %02X %02X %02X",
				 (reg*16), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
		printf(" %02X %02X %02X %02X %02X %02X %02X %02X\n",
				 data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
		count++;
	}

	set_page0();
	usleep(50 * 1000);
	exit(0);
}

//**************************************************************************************
/* --------------------------------------------------------------- *
 * bno_reset() resets the sensor. It will come up in CONFIG mode.  *
 * --------------------------------------------------------------- */
//**************************************************************************************
int bno_reset(void)
{
	char data[2];
	data[0]	=	BNO055_SYS_TRIGGER_ADDR;
	data[1]	=	0x20;
	if (write(i2cfd, data, 2) != 2)
	{
		printf("Error: I2C write failure for register 0x%02X\n", data[0]);
		exit(-1);
	}
	if (gBNO_verbose == 1) printf("Debug: BNO055 Sensor Reset complete\n");

	/* ------------------------------------------------------------ *
	* After a reset, the sensor needs at leat 650ms to boot up.    *
	* ------------------------------------------------------------ */
	usleep(650 * 1000);
	exit(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_calstatus() gets the calibration state from the sensor. *
 * Calibration status has 4 values, encoded as 2bit in reg 0x35 *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_calstatus(struct bnocal *bno_ptr)
{
char reg	=	BNO055_CALIB_STAT_ADDR;

	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	char data	=	0;
	if (read(i2cfd, &data, 1) != 1)
	{
		printf("Error: I2C read failure for register 0x%02X\n", reg);
		return(-1);
	}

	bno_ptr->scal_st	=	(data & 0b11000000) >> 6;	// system calibration status
	if (gBNO_verbose == 1) printf("Debug: sensor system calibration: [%d]\n", bno_ptr->scal_st);
	bno_ptr->gcal_st	=	(data & 0b00110000) >> 4;	// gyro calibration
	if (gBNO_verbose == 1) printf("Debug:     gyroscope calibration: [%d]\n", bno_ptr->gcal_st);
	bno_ptr->acal_st	=	(data & 0b00001100) >> 2;	// accel calibration status
	if (gBNO_verbose == 1) printf("Debug: accelerometer calibration: [%d]\n", bno_ptr->acal_st);
	bno_ptr->mcal_st	=	(data & 0b00000011);			// magneto calibration status
	if (gBNO_verbose == 1) printf("Debug:  magnetometer calibration: [%d]\n", bno_ptr->mcal_st);
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * Calibration offset is stored in 3x6 (18) registers 0x55~0x66 *
 * plus 4 registers 0x67~0x6A accelerometer/magnetometer radius *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_caloffset(struct bnocal *bno_ptr)
{
	/* --------------------------------------------------------- *
	* Registers may not update in fusion mode, switch to CONFIG *
	* --------------------------------------------------------- */
	opmode_t oldmode	=	get_mode();
	set_mode(config);

	char reg	=	ACC_OFFSET_X_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: I2C read %d bytes starting at register 0x%02X\n", CALIB_BYTECOUNT, reg);

	char data[CALIB_BYTECOUNT]	=	{0};
	if (read(i2cfd, data, CALIB_BYTECOUNT) != CALIB_BYTECOUNT)
	{
		printf("Error: I2C calibration data read from 0x%02X\n", reg);
		return(-1);
	}
	if (gBNO_verbose == 1)
	{
		int i	=	0;
		printf("Debug: Calibrationset:");
		while(i<CALIB_BYTECOUNT)
		{
			printf(" %02X", data[i]);
			i++;
		}
		printf("\n");
	}

	/* ------------------------------------------------------------ *
	* assigning accelerometer X-Y-Z offset, range per G-range      *
	* 16G = +/-16000, 8G = +/-8000, 4G = +/-4000, 2G = +/-2000     *
	* ------------------------------------------------------------ */
	if (gBNO_verbose == 1) printf("Debug: accelerometer offset: [%d] [%d] [%d] (X-Y-Z)\n",
					((int16_t)data[1] << 8) | data[0],
					((int16_t)data[3] << 8) | data[2],
					((int16_t)data[5] << 8) | data[4]);

	bno_ptr->aoff_x	=	((int16_t)data[1] << 8) | data[0];
	bno_ptr->aoff_y	=	((int16_t)data[3] << 8) | data[2];
	bno_ptr->aoff_z	=	((int16_t)data[5] << 8) | data[4];

	/* ------------------------------------------------------------ *
	 * assigning magnetometer X-Y-Z offset, offset range is +/-6400 *
	 * ------------------------------------------------------------ */
	if (gBNO_verbose == 1) printf("Debug:  magnetometer offset: [%d] [%d] [%d] (X-Y-Z)\n",
									((int16_t)data[7] << 8) | data[6],
									((int16_t)data[9] << 8) | data[8],
									((int16_t)data[11] << 8) | data[10]);
	bno_ptr->moff_x	=	((int16_t)data[7] << 8) | data[6];
	bno_ptr->moff_y	=	((int16_t)data[9] << 8) | data[8];
	bno_ptr->moff_z	=	((int16_t)data[11] << 8) | data[10];

	/* ------------------------------------------------------------ *
	 * assigning gyroscope X-Y-Z offset, range depends on dps value *
	 * 2000 = +/-32000, 1000 = +/-16000, 500 = +/-8000, etc         *
	 * ------------------------------------------------------------ */
	if (gBNO_verbose == 1) printf("Debug:     gyroscope offset: [%d] [%d] [%d] (X-Y-Z)\n",
									((int16_t)data[13] << 8) | data[12],
									((int16_t)data[15] << 8) | data[14],
									((int16_t)data[17] << 8) | data[16]);
	bno_ptr->goff_x	=	((int16_t)data[13] << 8) | data[12];
	bno_ptr->goff_y	=	((int16_t)data[15] << 8) | data[14];
	bno_ptr->goff_z	=	((int16_t)data[17] << 8) | data[16];

	/* ------------------------------------------------------------ *
	 * assigning accelerometer radius, range is +/-1000             *
	 * ------------------------------------------------------------ */
	if (gBNO_verbose == 1) printf("Debug: accelerometer radius: [%d] (+/-1000)\n",
									((int16_t)data[19] << 8) | data[18]);
	bno_ptr->acc_rad	=	((int16_t)data[19] << 8) | data[18];

	/* ------------------------------------------------------------ *
	 * assigning magnetometer radius, range is +/-960               *
    * ------------------------------------------------------------ */
	if (gBNO_verbose == 1) printf("Debug:  magnetometer radius: [%d] (+/- 960)\n",
									((int16_t)data[21] << 8) | data[20]);
	bno_ptr->mag_rad	=	((int16_t)data[21] << 8) | data[20];
	set_mode(oldmode);
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * save_cal() - writes calibration data to file for reuse       *
 * ------------------------------------------------------------ */
//**************************************************************************************
int save_cal(char *file)
{
	/* --------------------------------------------------------- *
	 * Read 34 bytes calibration data from registers 0x43~66,    *
	 * plus 4 reg 0x67~6A with accelerometer/magnetometer radius *
	 * switch to CONFIG, data is only visible in non-fusion mode *
	 * --------------------------------------------------------- */
	opmode_t oldmode	=	get_mode();
	set_mode(config);
	int i	=	0;
	//char reg	=	ACC_OFFSET_X_LSB_ADDR;
	char reg	=	BNO055_SIC_MATRIX_0_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: I2C read %d bytes starting at register 0x%02X\n",
		   CALIB_BYTECOUNT, reg);

	char data[CALIB_BYTECOUNT]	=	{0};
	if (read(i2cfd, data, CALIB_BYTECOUNT) != CALIB_BYTECOUNT)
	{
		printf("Error: I2C calibration data read from 0x%02X\n", reg);
		return(-1);
	}
	if (gBNO_verbose == 1)
	{
		printf("Debug: Calibrationset:");
		while(i<CALIB_BYTECOUNT)
		{
			printf(" %02X", data[i]);
			i++;
		}
		printf("\n");
	}

	/* -------------------------------------------------------- *
	*  Open the calibration data file for writing.             *
	* -------------------------------------------------------- */
	FILE *calib;
	if (! (calib=fopen(file, "w")))
	{
		printf("Error: Can't open %s for writing.\n", file);
		exit(-1);
	}
	if (gBNO_verbose == 1) printf("Debug:  Write to file: [%s]\n", file);

	/* -------------------------------------------------------- *
	* write the bytes in data[] out                            *
	* -------------------------------------------------------- */
	int outbytes	=	fwrite(data, 1, CALIB_BYTECOUNT, calib);
	fclose(calib);
	if (gBNO_verbose == 1) printf("Debug:  Bytes to file: [%d]\n", outbytes);
	if (outbytes != CALIB_BYTECOUNT)
	{
		printf("Error: %d/%d bytes written to file.\n", outbytes, CALIB_BYTECOUNT);
		return(-1);
	}
	set_mode(oldmode);
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * load_cal() load previously saved calibration data from file  *
 * ------------------------------------------------------------ */
//**************************************************************************************
int load_cal(char *file)
{
	/* -------------------------------------------------------- *
	*  Open the calibration data file for reading.             *
	* -------------------------------------------------------- */
	FILE *calib;
	if (! (calib=fopen(file, "r")))
	{
		printf("Error: Can't open %s for reading.\n", file);
		exit(-1);
	}
	if (gBNO_verbose == 1) printf("Debug: Load from file: [%s]\n", file);

	/* -------------------------------------------------------- *
	* Read 34 bytes from file into data[], starting at data[1] *
	* -------------------------------------------------------- */
	char data[CALIB_BYTECOUNT + 1]	=	{0};
	//data[0]						=	ACC_OFFSET_X_LSB_ADDR;
	data[0]							=	BNO055_SIC_MATRIX_0_LSB_ADDR;
	int inbytes	=	fread(&data[1], 1, CALIB_BYTECOUNT, calib);
	fclose(calib);

	if (inbytes != CALIB_BYTECOUNT)
	{
		printf("Error: %d/%d bytes read to file.\n", inbytes, CALIB_BYTECOUNT);
		return(-1);
	}
	if (gBNO_verbose == 1)
	{
		printf("Debug: Calibrationset:");
		int i	=	1;
		while(i<CALIB_BYTECOUNT+1)
		{
			printf(" %02X", data[i]);
			i++;
		}
		printf("\n");
	}

	/* -------------------------------------------------------- *
	* Write 34 bytes from file into sensor registers from 0x43 *
	* We need to switch in and out of CONFIG mode if needed... *
	* -------------------------------------------------------- */
	opmode_t oldmode	=	get_mode();
	set_mode(config);
	usleep(50 * 1000);

	if (write(i2cfd, data, (CALIB_BYTECOUNT+1)) != (CALIB_BYTECOUNT+1))
	{
	printf("Error: I2C write failure for register 0x%02X\n", data[0]);
	return(-1);
	}

	/* -------------------------------------------------------- *
	* To verify, we read 34 bytes from 0x43 & compare to input *
	* -------------------------------------------------------- */
	//char reg	=	ACC_OFFSET_X_LSB_ADDR;
	char reg	=	BNO055_SIC_MATRIX_0_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
	printf("Error: I2C write failure for register 0x%02X\n", reg);
	return(-1);
	}

	char newdata[CALIB_BYTECOUNT]	=	{0};
	if (read(i2cfd, newdata, CALIB_BYTECOUNT) != CALIB_BYTECOUNT)
	{
	printf("Error: I2C calibration data read from 0x%02X\n", reg);
	return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: Registerupdate:");
	int i	=	0;
	while(i<CALIB_BYTECOUNT)
	{
	if (data[i+1] != newdata[i])
	{
	printf("\nError: Calibration load failure %02X register 0x%02X\n", newdata[i], reg+i);
	//exit(-1);
	}
	if (gBNO_verbose == 1) printf(" %02X", newdata[i]);
	i++;
	}
	if (gBNO_verbose == 1) printf("\n");
	set_mode(oldmode);

	/* -------------------------------------------------------- *
	* 650 ms delay are only needed if -l and -t are both used  *
	* to let the fusion code process the new calibration data  *
	* -------------------------------------------------------- */
	usleep(650 * 1000);
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * print_unit() - Extract the SI unit config from register 0x3B *
 * ------------------------------------------------------------ */
//**************************************************************************************
void print_unit(int unit_sel)
{
	// bit-0
	printf("Acceleration Unit  = ");
	if ((unit_sel >> 0) & 0x01) printf("mg\n");
	else printf("m/s2\n");

	// bit-1
	printf("    Gyroscope Unit = ");
	if ((unit_sel >> 1) & 0x01) printf("rps\n");
	else printf("dps\n");

	// bit-2
	printf("        Euler Unit = ");
	if ((unit_sel >> 2) & 0x01) printf("Radians\n");
	else printf("Degrees\n");

	// bit-3: unused
	// bit-4
	printf("  Temperature Unit = ");
	if ((unit_sel >> 4) & 0x01) printf("Fahrenheit\n");
	else printf("Celsius\n");

	// bit-5: unused
	// bit-6: unused
	// bit-7
	printf("  Orientation Mode = ");
	if ((unit_sel >> 3) & 0x01) printf("Android\n");
	else printf("Windows\n");
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_inf() queries the BNO055 and write the info data into    *
 * the global struct bnoinf defined in getbno055.h              *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_inf(struct bnoinf *bno_ptr)
{
char reg	=	0x00;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	char data[7]	=	{0};
	if (read(i2cfd, data, 7) != 7)
	{
		printf("Error: I2C read failure for register data 0x00-0x06\n");
		return(-1);
	}
	/* --------------------------------------------------------- *
	* 1-byte chip ID in register 0x00, default: 0xA0            *
	* --------------------------------------------------------- */
	if (gBNO_verbose == 1) printf("Debug: Sensor CHIP ID: [0x%02X]\n", data[0]);
	bno_ptr->chip_id	=	data[0];

	/* --------------------------------------------------------- *
	* 1-byte Accelerometer ID in register 0x01, default: 0xFB   *
	* --------------------------------------------------------- */
	if (gBNO_verbose == 1) printf("Debug: Sensor  ACC ID: [0x%02X]\n", data[1]);
	bno_ptr->acc_id	=	data[1];

	/* --------------------------------------------------------- *
	* 1-byte Magnetometer ID in register 0x02, default 0x32     *
	* --------------------------------------------------------- */
	if (gBNO_verbose == 1) printf("Debug: Sensor  MAG ID: [0x%02X]\n", data[2]);
	bno_ptr->mag_id	=	data[2];

	/* --------------------------------------------------------- *
	* 1-byte Gyroscope ID in register 0x03, default: 0x0F       *
	* --------------------------------------------------------- */
	if (gBNO_verbose == 1) printf("Debug: Sensor  GYR ID: [0x%02X]\n", data[3]);
	bno_ptr->gyr_id	=	data[3];

	/* --------------------------------------------------------- *
	* 1-byte SW Revsion ID LSB in register 0x04, default: 0x08  *
	* --------------------------------------------------------- */
	if (gBNO_verbose == 1) printf("Debug: SW  Rev-ID LSB: [0x%02X]\n", data[4]);
	bno_ptr->sw_lsb	=	data[4];

	/* --------------------------------------------------------- *
	* 1-byte SW Revision ID MSB in register 0x05, default: 0x03 *
	* --------------------------------------------------------- */
	if (gBNO_verbose == 1) printf("Debug: SW  Rev-ID MSB: [0x%02X]\n", data[5]);
	bno_ptr->sw_msb	=	data[5];

	/* --------------------------------------------------------- *
	* 1-byte BootLoader Revision ID register 0x06, no default   *
	* --------------------------------------------------------- */
	if (gBNO_verbose == 1) printf("Debug: Bootloader Ver: [0x%02X]\n", data[6]);
	bno_ptr->bl_rev	=	data[6];

	/* --------------------------------------------------------- *
	* Read the operations mode with get_mode(), default: 0x0    *
	* --------------------------------------------------------- */
	bno_ptr->opr_mode	=	get_mode();

	/* --------------------------------------------------------- *
	* Read the power mode with get_power(), default: 0x0        *
	* --------------------------------------------------------- */
	bno_ptr->pwr_mode	=	get_power();

	/* --------------------------------------------------------- *
	* Read the axis remap config get_remap('c'), default: 0x24  *
	* --------------------------------------------------------- */
	bno_ptr->axr_conf	=	get_remap('c');

	/* --------------------------------------------------------- *
	* Read the axis remap sign get_remap('s'), default: 0x00    *
	* --------------------------------------------------------- */
	bno_ptr->axr_sign	=	get_remap('s');

	/* --------------------------------------------------------- *
	* Read 1-byte system status from register 0x39, no default  *
	* --------------------------------------------------------- */
	reg	=	BNO055_SYS_STAT_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	data[0]	=	0;
	if (read(i2cfd, data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}
	if (gBNO_verbose == 1) printf("Debug:  System Status: [0x%02X]\n", data[0]);
	bno_ptr->sys_stat	=	data[0];

	/* --------------------------------------------------------- *
	* Read 1-byte Self Test Result register 0x36, 0x0F=pass     *
	* --------------------------------------------------------- */
	reg	=	BNO055_SELFTSTRES_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	data[0]	=	0;
	if (read(i2cfd, data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}
	if (gBNO_verbose == 1) printf("Debug: Self-Test Mode: [0x%02X] 4bit [0x%02X]\n", data[0], data[0] & 0x0F);
	bno_ptr->selftest	=	data[0] & 0x0F; // only get the lowest 4 bits

	/* --------------------------------------------------------- *
	* Read 1-byte System Error from register 0x3A, 0=OK         *
	* --------------------------------------------------------- */
	reg	=	BNO055_SYS_ERR_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	data[0]	=	0;
	if (read(i2cfd, data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}
	if (gBNO_verbose == 1) printf("Debug: Internal Error: [0x%02X]\n", data[0]);
	bno_ptr->sys_err	=	data[0];

	/* --------------------------------------------------------- *
	* Read 1-byte Unit definition from register 0x3B, 0=OK      *
	* --------------------------------------------------------- */
	reg	=	BNO055_UNIT_SEL_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	data[0]	=	0;
	if (read(i2cfd, data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}
	if (gBNO_verbose == 1) printf("Debug: UnitDefinition: [0x%02X]\n", data[0]);
	bno_ptr->unitsel	=	data[0];

	/* --------------------------------------------------------- *
	* Extract the temperature unit from the unit selection data *
	* --------------------------------------------------------- */
	char t_unit;
	if ((data[0] >> 4) & 0x01) t_unit = 'F';
	else  t_unit	=	'C';

	/* --------------------------------------------------------- *
	* Read sensor temperature from register 0x34, no default    *
	* --------------------------------------------------------- */
	reg	=	BNO055_TEMP_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	data[0]	=	0;
	if (read(i2cfd, data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}
	if (gBNO_verbose == 1) printf("Debug:    Temperature: [0x%02X] [%d°%c]\n", data[0], data[0], t_unit);
	bno_ptr->temp_val	=	data[0];

	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 *  get_acc() - read accelerometer data into the global struct  *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_acc(struct bnoacc *bnod_ptr)
{
	char reg	=	BNO055_ACC_DATA_X_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	char data[6]	=	{0};
	if (read(i2cfd, data, 6) != 6)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	int16_t buf	=	((int16_t)data[1] << 8) | data[0];
	if (gBNO_verbose == 1) printf("Debug: Accelerometer Data X: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
	bnod_ptr->adata_x	=	(double) buf;

	buf	=	((int16_t)data[3] << 8) | data[2];
	if (gBNO_verbose == 1) printf("Debug: Accelerometer Data Y: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
	bnod_ptr->adata_y	=	(double) buf;

	buf	=	((int16_t)data[5] << 8) | data[4];
	if (gBNO_verbose == 1) printf("Debug: Accelerometer Data Z: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
	bnod_ptr->adata_z	=	(double) buf;
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 *  get_mag() - read magnetometer data into the global struct   *
 *  Convert magnetometer data in microTesla. 1 microTesla = 16  *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_mag(struct bnomag *bnod_ptr)
{
	char reg	=	BNO055_MAG_DATA_X_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	char data[6]	=	{0};
	if (read(i2cfd, data, 6) != 6)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	int16_t buf	=	((int16_t)data[1] << 8) | data[0];
	if (gBNO_verbose == 1) printf("Debug: Magnetometer Data X: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
	bnod_ptr->mdata_x	=	(double) buf / 1.6;

	buf	=	((int16_t)data[3] << 8) | data[2];
	if (gBNO_verbose == 1) printf("Debug: Magnetometer Data Y: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
	bnod_ptr->mdata_y	=	(double) buf / 1.6;

	buf	=	((int16_t)data[5] << 8) | data[4];
	if (gBNO_verbose == 1) printf("Debug: Magnetometer Data Z: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
	bnod_ptr->mdata_z	=	(double) buf / 1.6;
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 *  get_gyr() - read gyroscope data into the global struct      *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_gyr(struct bnogyr *bnod_ptr)
{
	char reg	=	BNO055_GYRO_DATA_X_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	char data[6]	=	{0};
	if (read(i2cfd, data, 6) != 6)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	int16_t buf	=	((int16_t)data[1] << 8) | data[0];
	if (gBNO_verbose == 1) printf("Debug: Gyroscope Data X: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
	bnod_ptr->gdata_x	=	(double) buf / 16.0;

	buf	=	((int16_t)data[3] << 8) | data[2];
	if (gBNO_verbose == 1) printf("Debug: Gyrosscope Data Y: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
	bnod_ptr->gdata_y	=	(double) buf / 16.0;

	buf	=	((int16_t)data[5] << 8) | data[4];
	if (gBNO_verbose == 1) printf("Debug: Gyroscope Data Z: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
	bnod_ptr->gdata_z	=	(double) buf / 16.0;
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 *  get_eul() - read Euler orientation into the global struct   *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_eul(struct bnoeul *bnod_ptr)
{
	char reg	=	BNO055_EULER_H_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: I2C read 6 bytes starting at register 0x%02X\n", reg);

	unsigned char data[6]	=	{0, 0, 0, 0, 0, 0};
	if (read(i2cfd, data, 6) != 6)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	int16_t buf	=	((int16_t)data[1] << 8) | data[0];
	if (gBNO_verbose == 1) printf("Debug: Euler Orientation H: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
	bnod_ptr->eul_head	=	(double) buf / 16.0;

	buf	=	((int16_t)data[3] << 8) | data[2];
	if (gBNO_verbose == 1) printf("Debug: Euler Orientation R: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
	bnod_ptr->eul_roll	=	(double) buf / 16.0;

	buf	=	((int16_t)data[5] << 8) | data[4];
	if (gBNO_verbose == 1) printf("Debug: Euler Orientation P: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
	bnod_ptr->eul_pitc	=	(double) buf / 16.0;
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 *  get_qua() - read Quaternation data into the global struct   *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_qua(struct bnoqua *bnod_ptr)
{
	char reg	=	BNO055_QUATERNION_DATA_W_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: I2C read 8 bytes starting at register 0x%02X\n", reg);

	unsigned char data[8]	=	{0};
	if (read(i2cfd, data, 8) != 8)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	int16_t buf	=	((int16_t)data[1] << 8) | data[0];
	if (gBNO_verbose == 1) printf("Debug: Quaternation W: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
	bnod_ptr->quater_w	=	(double) buf / 16384.0;

	buf	=	((int16_t)data[3] << 8) | data[2];
	if (gBNO_verbose == 1) printf("Debug: Quaternation X: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
	bnod_ptr->quater_x	=	(double) buf / 16384.0;

	buf	=	((int16_t)data[5] << 8) | data[4];
	if (gBNO_verbose == 1) printf("Debug: Quaternation Y: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
	bnod_ptr->quater_y	=	(double) buf / 16384.0;

	buf	=	((int16_t)data[7] << 8) | data[6];
	if (gBNO_verbose == 1) printf("Debug: Quaternation Z: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[6], data[7],buf);
	bnod_ptr->quater_z	=	(double) buf / 16384.0;
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 *  get_gra() - read gravity vector into the global struct      *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_gra(struct bnogra *bnod_ptr)
{
char reg	=	BNO055_UNIT_SEL_ADDR;

	/* --------------------------------------------------------- *
	* Get the unit conversion: 1 m/s2 = 100 LSB, 1 mg = 1 LSB   *
	* --------------------------------------------------------- */
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}
	char unit_sel;
	if (read(i2cfd, &unit_sel, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	double ufact;
	if ((unit_sel >> 0) & 0x01)
	{
		ufact	=	1.0;
	}
	else
	{
		ufact = 100.0;
	}
	/* --------------------------------------------------------- *
	* Get the gravity vector data                               *
	* --------------------------------------------------------- */
	reg	=	BNO055_GRAVITY_DATA_X_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: I2C read 6 bytes starting at register 0x%02X\n", reg);

	unsigned char data[6]	=	{0, 0, 0, 0, 0, 0};
	if (read(i2cfd, data, 6) != 6)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	int16_t buf	=	((int16_t)data[1] << 8) | data[0];
	if (gBNO_verbose == 1) printf("Debug: Gravity Vector H: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
	bnod_ptr->gravityx	=	(double) buf / ufact;

	buf	=	((int16_t)data[3] << 8) | data[2];
	if (gBNO_verbose == 1) printf("Debug: Gravity Vector M: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
	bnod_ptr->gravityy	=	(double) buf / ufact;

	buf	=	((int16_t)data[5] << 8) | data[4];
	if (gBNO_verbose == 1) printf("Debug: Gravity Vector P: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
	bnod_ptr->gravityz	=	(double) buf / ufact;
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 *  get_lin() - read linear acceleration into the global struct *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_lin(struct bnolin *bnod_ptr)
{
	/* --------------------------------------------------------- *
	* Get the unit conversion: 1 m/s2 = 100 LSB, 1 mg = 1 LSB   *
	* --------------------------------------------------------- */
	char reg	=	BNO055_UNIT_SEL_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}
	char unit_sel;
	if (read(i2cfd, &unit_sel, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	double ufact;
	if ((unit_sel >> 0) & 0x01) ufact	=	1.0;
	else ufact	=	100.0;

	/* --------------------------------------------------------- *
	* Get the linear acceleration data                          *
	* --------------------------------------------------------- */
	reg	=	BNO055_LIN_ACC_DATA_X_LSB_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: I2C read 6 bytes starting at register 0x%02X\n", reg);

	unsigned char data[6]	=	{0, 0, 0, 0, 0, 0};
	if (read(i2cfd, data, 6) != 6)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	int16_t buf	=	((int16_t)data[1] << 8) | data[0];
	if (gBNO_verbose == 1) printf("Debug: Linear Acceleration H: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
	bnod_ptr->linacc_x	=	(double) buf / ufact;

	buf	=	((int16_t)data[3] << 8) | data[2];
	if (gBNO_verbose == 1) printf("Debug: Linear Acceleration M: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
	bnod_ptr->linacc_y	=	(double) buf / ufact;

	buf	=	((int16_t)data[5] << 8) | data[4];
	if (gBNO_verbose == 1) printf("Debug: Linear Acceleration P: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
	bnod_ptr->linacc_z	=	(double) buf / ufact;
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * set_mode() - set the sensor operational mode register 0x3D   *
 * The modes cannot be switched over directly, first it needs   *
 * to be set to "config" mode before switching to the new mode. *
 * ------------------------------------------------------------ */
//**************************************************************************************
int set_mode(opmode_t newmode)
{
	char data[2]		=	{0};
	data[0]				=	BNO055_OPR_MODE_ADDR;
	opmode_t oldmode	=	get_mode();

	if (oldmode == newmode)
	{
		return(0); // if new mode is the same
	}
	else if (oldmode > 0 && newmode > 0)
	{  // switch to "config" first
		data[1]	=	0x0;
		if (gBNO_verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);

		if (write(i2cfd, data, 2) != 2)
		{
			printf("Error: I2C write failure for register 0x%02X\n", data[0]);
			return(-1);
		}
		/* --------------------------------------------------------- *
		* switch time: any->config needs 7ms + small buffer = 10ms  *
		* --------------------------------------------------------- */
		usleep(10 * 1000);
	}

	data[1]	=	newmode;
	if (gBNO_verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);

	if (write(i2cfd, data, 2) != 2)
	{
		printf("Error: I2C write failure for register 0x%02X\n", data[0]);
		return(-1);
	}
	/* --------------------------------------------------------- *
	* switch time: config->any needs 19ms + small buffer = 25ms *
	* --------------------------------------------------------- */
	usleep(25 * 1000);

	if (get_mode() == newmode) return(0);
	else return(-1);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_mode() - returns sensor operational mode register 0x3D   *
 * Reads 1 byte from Operations Mode register 0x3d, and uses    *
 * only the lowest 4 bit. Bits 4-7 are unused, stripped off     *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_mode(void)
{
	int reg	=	BNO055_OPR_MODE_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	unsigned int data	=	0;
	if (read(i2cfd, &data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: Operation Mode: [0x%02X]\n", data & 0x0F);

	return(data & 0x0F);  // only return the lowest 4 bits
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * print_mode() - prints sensor operational mode string from    *
 * sensor operational mode numeric value.                       *
 * ------------------------------------------------------------ */
//**************************************************************************************
int print_mode(int mode)
{
	if (mode < 0 || mode > 12) return(-1);

	switch (mode)
	{
		case 0x00:
			printf("CONFIG\n");
			break;
		case 0x01:
			printf("ACCONLY\n");
			break;
		case 0x02:
			printf("MAGONLY\n");
			break;
		case 0x03:
			printf("GYRONLY\n");
			break;
		case 0x04:
			printf("ACCMAG\n");
			break;
		case 0x05:
			printf("ACCGYRO\n");
			break;
		case 0x06:
			printf("MAGGYRO\n");
			break;
		case 0x07:
			printf("AMG\n");
			break;
		case 0x08:
			printf("IMU\n");
			break;
		case 0x09:
			printf("COMPASS\n");
			break;
		case 0x0A:
			printf("M4G\n");
			break;
		case 0x0B:
			printf("NDOF_FMC_OFF\n");
			break;
		case 0x0C:
			printf("NDOF_FMC\n");
			break;
	}
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * set_power() - set the sensor power mode in register 0x3E.    *
 * The power modes cannot be switched over directly, first the  *
 * ops mode needs to be "config"  to write the new power mode.  *
 * ------------------------------------------------------------ */
//**************************************************************************************
int set_power(power_t pwrmode)
{
char data[2]	=	{0};

	/* ------------------------------------------------------------ *
	* Check what operational mode we are in                        *
	* ------------------------------------------------------------ */
	opmode_t oldmode	=	get_mode();

	/* ------------------------------------------------------------ *
	* If ops mode wasn't config, switch to "CONFIG" mode first     *
	* ------------------------------------------------------------ */
	if (oldmode > 0)
	{
		data[0]	=	BNO055_OPR_MODE_ADDR;
		data[1]	=	0x0;

		if (gBNO_verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);

		if (write(i2cfd, data, 2) != 2)
		{
			printf("Error: I2C write failure for register 0x%02X\n", data[0]);
			return(-1);
		}
		usleep(30 * 1000);
	}  // now we are in config mode

	/* ------------------------------------------------------------ *
	* Set the new power mode                                       *
	* ------------------------------------------------------------ */
	data[0]	=	BNO055_PWR_MODE_ADDR;
	data[1]	=	pwrmode;
	if (gBNO_verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
	if (write(i2cfd, data, 2) != 2)
	{
		printf("Error: I2C write failure for register 0x%02X\n", data[0]);
		return(-1);
	}
	usleep(30 * 1000);

	/* ------------------------------------------------------------ *
	* If ops mode wasn't config, switch back to original ops mode  *
	* ------------------------------------------------------------ */
	if (oldmode > 0)
	{
		data[0]	=	BNO055_OPR_MODE_ADDR;
		data[1]	=	oldmode;
		if (gBNO_verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
		if (write(i2cfd, data, 2) != 2)
		{
			printf("Error: I2C write failure for register 0x%02X\n", data[0]);
			return(-1);
		}
		usleep(30 * 1000);
	}  // now the previous mode is back

	if (get_power() == pwrmode) return(0);
	else return(-1);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_power() returns the sensor power mode from register 0x3e *
 * Only the lowest 2 bit are used, ignore the unused bits 2-7.  *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_power(void)
{
int reg	=	BNO055_PWR_MODE_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	unsigned int data	=	0;
	if (read(i2cfd, &data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug:     Power Mode: [0x%02X] 2bit [0x%02X]\n", data, data & 0x03);

	return(data & 0x03);  // only return the lowest 2 bits
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * print_power() - prints the sensor power mode string from the *
 * sensors power mode numeric value.                            *
 * ------------------------------------------------------------ */
//**************************************************************************************
int print_power(int mode)
{
	if (mode < 0 || mode > 2) return(-1);

	switch (mode)
	{
		case 0x00:
			printf("NORMAL\n");
			break;
		case 0x01:
			printf("LOW\n");
			break;
		case 0x02:
			printf("SUSPEND\n");
			break;
	}
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_sstat() returns the sensor sys status from register 0x39 *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_sstat(void)
{
	int reg	=	BNO055_SYS_STAT_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	unsigned int data	=	0;
	if (read(i2cfd, &data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug:  System Status: [0x%02X]\n", data);

	return(data);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * print_sstat() - prints the sensor system status string from  *
 * the numeric value located in the sys_stat register 0x39      *
 * ------------------------------------------------------------ */
//**************************************************************************************
int print_sstat(int stat_code)
{
	if (stat_code < 0 || stat_code > 6) return(-1);

	switch (stat_code)
	{
		case 0x00:
			printf("Idle\n");
			break;
		case 0x01:
			printf("System Error\n");
			break;
		case 0x02:
			printf("Initializing Peripherals\n");
			break;
		case 0x03:
			printf("System Initalization\n");
			break;
		case 0x04:
			printf("Executing Self-Test\n");
			break;
		case 0x05:
			printf("Sensor running with fusion algorithm\n");
			break;
		case 0x06:
			printf("Sensor running without fusion algorithm\n");
			break;
	}
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_remap() returns axis remap data from registers 0x41 0x42 *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_remap(char mode)
{
int reg;

	if (mode == 'c') 		reg	=	BNO055_AXIS_MAP_CONFIG_ADDR;
	else if (mode == 's')	reg	=	BNO055_AXIS_MAP_SIGN_ADDR;
	else
	{
		printf("Error: Unknown remap function mode %c.\n", mode);
		exit(-1);
	}

	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		return(-1);
	}

	unsigned int data	=	0;
	if (read(i2cfd, &data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: Axis Remap '%c': [0x%02X]\n", mode, data);

	return(data);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * print_remap_conf() - prints the sensor axis configuration.   *
 * the numeric values are located in register 0x41. Valid modes *
 * are:
 * 0x24 (default), 0x21,
 * ------------------------------------------------------------ */
//**************************************************************************************
int print_remap_conf(int mode)
{

	if (mode != 0x24 && mode != 0x18 && mode != 0x09 && mode != 0x36) return(-1);

	switch (mode)
	{
		case 0x24:     // 0 1 | 1 0 | 0 0
			printf("X==X Y==Y Z==Z (ENU)\n");
			break;
		case 0x18:     // 0 1 | 0 0 | 1 0
			printf("X<>Y Y<>X Z==Z (NEU)\n");
			break;
		case 0x09:     // 0 0 | 1 0 | 0 1
			printf("X<>Z Y==Y Z<>X (UNE)\n");
			break;
		case 0x36:     // 1 0 | 0 1 | 0 0
			printf("X==X Y<>Z Z<>Y (EUN)\n");
			break;
	}
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * print_remap_sign() - prints the sensor axis remapping +/-.   *
 * the numeric values are located in register 0x42.             *
 * ------------------------------------------------------------ */
//**************************************************************************************
int print_remap_sign(int mode)
{

	if (mode < 0 || mode > 7) return(-1);

	switch (mode)
	{
		case 0x00:
			printf("X+ Y+ Z+\n");
			break;
		case 0x01:
			printf("X+ Y+ Z-\n");
			break;
		case 0x02:
			printf("X+ Y- Z+\n");
			break;
		case 0x03:
			printf("X+ Y- Z-\n");
			break;
		case 0x04:
			printf("X- Y+ Z+\n");
			break;
		case 0x05:
			printf("X- Y+ Z-\n");
			break;
		case 0x06:
			printf("X- Y- Z+\n");
			break;
		case 0x07:
			printf("X- Y- Z-\n");
			break;
	}
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * set_page0() - Set page ID = 0 to set default register access *
 * ------------------------------------------------------------ */
//**************************************************************************************
int set_page0(void)
{
	char data[2]	=	{0};
	data[0]			=	BNO055_PAGE_ID_ADDR;
	data[1]			=	0x0;

	if (gBNO_verbose == 1) printf("Debug: write page-ID: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
	if (write(i2cfd, data, 2) != 2)
	{
		printf("Error: I2C write failure for register 0x%02X\n", data[0]);
		return(-1);
	}
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * set_page1() - Set page ID = 1 to switch the register access  *
 * ------------------------------------------------------------ */
//**************************************************************************************
int set_page1(void)
{
	char data[2]	=	{0};
	data[0]			=	BNO055_PAGE_ID_ADDR;
	data[1]			=	0x1;

	if (gBNO_verbose == 1) printf("Debug: write page-ID: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
	if (write(i2cfd, data, 2) != 2)
	{
		printf("Error: I2C write failure for register 0x%02X\n", data[0]);
		return(-1);
	}
	return(0);
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_clksrc() - return setting for internal/external clock    *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_clksrc(void)
{
	char reg	=	BNO055_SYS_TRIGGER_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		set_page0();
		return(-1);
	}

	char data;
	if (read(i2cfd, &data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		set_page0();
		return(-1);
	}

	if (gBNO_verbose == 1) printf("Debug: CLK_SEL bit-7 in register %d: [%d]\n", reg, (data & 0b10000000) >> 7);
	return (data & 0b10000000) >> 7; // system calibration status
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * print_clksrc() - print setting for internal/external clock   *
 * ------------------------------------------------------------ */
//**************************************************************************************
void print_clksrc(void)
{
int src	=	get_clksrc();

	if (src == 0) printf("Internal Clock (default)\n");
	if (src == 1) printf("External Clock\n");
	if (src == -1) printf("Clock Reading error\n");
}

//**************************************************************************************
/* ------------------------------------------------------------ *
 * get_acc_conf() read accelerometer config into global struct  *
 * Requires switching register page 0->1 and back after reading *
 * ------------------------------------------------------------ */
//**************************************************************************************
int get_acc_conf(struct bnoaconf *bnoc_ptr)
{
char reg	=	BNO055_ACC_CONFIG_ADDR;

	set_page1();
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		set_page0();
		return(-1);
	}

	char data;
	if (read(i2cfd, &data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		set_page0();
		return(-1);
	}

	bnoc_ptr->range		=	(data & 0b00000011) >> 2; // accel range
	if (gBNO_verbose == 1) printf("Debug:       accelerometer range: [%d]\n", bnoc_ptr->pwrmode);
	bnoc_ptr->bandwth	=	(data & 0b00011100) >> 4; // accel bandwidth
	if (gBNO_verbose == 1) printf("Debug:   accelerometer bandwidth: [%d]\n", bnoc_ptr->bandwth);
	bnoc_ptr->pwrmode	=	(data & 0b11100000) >> 6; // accel power mode
	if (gBNO_verbose == 1) printf("Debug:  accelerometer power mode: [%d]\n", bnoc_ptr->pwrmode);

	reg	=	BNO055_ACC_SLEEP_CONFIG_ADDR;
	if (write(i2cfd, &reg, 1) != 1)
	{
		printf("Error: I2C write failure for register 0x%02X\n", reg);
		set_page0();
		return(-1);
	}

	data	=	0;
	if (read(i2cfd, &data, 1) != 1)
	{
		printf("Error: I2C read failure for register data 0x%02X\n", reg);
		set_page0();
		return(-1);
	}

	bnoc_ptr->slpmode	=	(data & 0b00000011) >> 2; // accel sleep mode
	if (gBNO_verbose == 1) printf("Debug:  accelerometer sleep mode: [%d]\n", bnoc_ptr->slpmode);
	bnoc_ptr->slpdur	=	(data & 0b00011100) >> 4; // accel sleep duration
	if (gBNO_verbose == 1) printf("Debug:   accelerometer sleep dur: [%d]\n", bnoc_ptr->slpdur);

	set_page0();
	return(0);
}

//**************************************************************************************
/* ----------------------------------------------------------- *
 *  print_acc_conf() - print accelerometer configuration       *
 * ----------------------------------------------------------- */
//**************************************************************************************
void print_acc_conf(struct bnoaconf *bnoc_ptr)
{
	printf("Accelerometer  Power = ");
	switch (bnoc_ptr->pwrmode)
	{
		case 0:
			printf("NORMAL\n");
			break;
		case 1:
			printf("SUSPEND\n");
			break;
		case 2:
			printf("LOW POWER1\n");
			break;
		case 3:
			printf("STANDBY\n");
			break;
		case 4:
			printf("LOW POWER2\n");
			break;
		case 5:
			printf("DEEP SUSPEND\n");
			break;
	}

	printf("Accelerometer Bwidth = ");
	switch (bnoc_ptr->bandwth)
	{
		case 0:
			printf("7.81Hz\n");
			break;
		case 1:
			printf("15.63Hz\n");
			break;
		case 2:
			printf("31.25Hz\n");
			break;
		case 3:
			printf("62.5Hz\n");
			break;
		case 4:
			printf("125Hz\n");
			break;
		case 5:
			printf("250Hz\n");
			break;
		case 6:
			printf("500Hz\n");
			break;
		case 7:
			printf("1KHz\n");
			break;
	}
	printf("Accelerometer GRange = ");
	switch (bnoc_ptr->range)
	{
		case 0:
			printf("2G\n");
			break;
		case 1:
			printf("4G\n");
			break;
		case 2:
			printf("8G\n");
			break;
		case 3:
			printf("16G\n");
			break;
	}

	printf("Accelerometer  Sleep = ");
	switch (bnoc_ptr->slpmode)
	{
		case 0:
			printf("event-driven, ");
			break;
		case 1:
			printf("equidistant sampling, ");
			break;
	}

	if (bnoc_ptr->slpdur < 6) printf("0.5ms\n");
	else switch (bnoc_ptr->slpdur)
	{
		case 6:
			printf("1ms\n");
			break;
		case 7:
			printf("2ms\n");
			break;
		case 8:
			printf("4ms\n");
			break;
		case 9:
			printf("6ms\n");
			break;
		case 10:
			printf("10ms\n");
			break;
		case 11:
			printf("25ms\n");
			break;
		case 12:
			printf("50ms\n");
			break;
		case 13:
			printf("100ms\n");
			break;
		case 14:
			printf("500ms\n");
			break;
		case 15:
			printf("1s\n");
			break;
	}
}

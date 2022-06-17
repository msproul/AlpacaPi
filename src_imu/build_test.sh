clear

gcc -D_INCLUDE_IMU_MAIN_	\
	imu_lib.c				\
	i2c_bno055.c			\
	-o imutest

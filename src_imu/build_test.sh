clear
echo "Building test program for IMU BNO055"
gcc -D_INCLUDE_IMU_MAIN_	\
	imu_lib.c				\
	i2c_bno055.c			\
	-I../src_mlsLib			\
	-lpthread				\
	-lm						\
	-o imutest

######################################################################################
#	Make file for alpaca driver
#	written by hand by Mark Sproul
#	(C) 2019 by Mark Sproul
#
#		sudo apt-get install build-essential
#		sudo apt-get install libusb-1.0-0-dev
#		sudo apt-get install libudev-dev
#		sudo apt-get install libopencv-dev
#		sudo apt-get install libi2c-dev
#		sudo apt-get install libjpeg-dev
#		sudo apt-get install libcfitsio-dev
#
#		sudo apt-get install wiringpi
#
#		sudo apt-get install libnova-dev		<<<< required for TSC
#
#		sudo apt-get install git-gui
#
#	https://www.gnu.org/software/make/manual/make.html
#
#	https://github.com/TheNextLVL/wiringPi
######################################################################################
#	Edit History
######################################################################################
#++	Apr  9,	2019	<MLS> Started on alpaca driver
#++	Apr 29,	2019	<MLS> Added openCV support
#++	May  7,	2019	<MLS> Added smate build option
#++	May 24,	2019	<MLS> Added wx build option
#++	Jun 25,	2019	<MLS> Added jetson build option
#++	Aug 20,	2019	<MLS> Added ATIK support
#++	Jan  9,	2020	<MLS> Added ToupTek support
#++	Jan 24,	2020	<MLS> Moved _ENABLE_FITS_ to Makefile
#++	Feb 11,	2020	<MLS> Added shutter
#++	Apr  3,	2020	<MLS> Added _ENABLE_FLIR_
#++	Apr 16,	2020	<MLS> Added _ENABLE_PWM_SWITCH_
#++	Apr 22,	2020	<MLS> Added flir to build flir camera on ubuntu
#++	Jun  8,	2020	<MLS> Added video controller
#++	Jun 23,	2020	<MLS> Added preview controller
#++	Jul 16,	2020	<MLS> Added pi64 for 64 bit Raspberry Pi OS
#++	Dec 12,	2020	<MLS> Moved _ENABLE_REMOTE_SHUTTER_ into Makefile
#++	Jan 13,	2021	<MLS> Added build commands for touptech cameras
#++	Mar 18,	2021	<MLS> Updating Makefile to use AtikCamerasSDK_2020_10_19
#++	Mar 18,	2021	<MLS> Updating QHY camera support
#++	Apr 20,	2021	<MLS> Added _ENABLE_TELESCOPE_RIGEL_
#++	Apr 26,	2021	<MLS> Added _ENABLE_FILTERWHEEL_ZWO_
#++	Apr 26,	2021	<MLS> Added _ENABLE_FILTERWHEEL_ATIK_
#++	Jan  6,	2022	<MLS> Added _ENABLE_REMOTE_SQL_  & _ENABLE_REMOTE_GAIA_
#++	Jan 13,	2022	<MLS> Added _ENABLE_ASTEROIDS_
#++	Jan 18,	2022	<MLS> Added fitsview to makefile
#++	Mar 24,	2022	<MLS> Added -fPIE to compile options
#++	Mar 25,	2022	<MLS> Added _ENABLE_TELESCOPE_SERVO_
#++	Mar 26,	2022	<MLS> Added make_checkplatform.sh
#++	Mar 26,	2022	<MLS> Added make_checkopencv.sh
#++	Mar 26,	2022	<MLS> Added make_checksql.sh
#++	May  2,	2022	<MLS> Added IMU source directory (src_imu)
#++	May  2,	2022	<MLS> Added _ENABLE_IMU_
#++	May  2,	2022	<MLS> Added make moonlite for stand alone moonlite focuser driver
#++	May  4,	2022	<MLS> Added camera simulator (make camerasim)
#++	May 19,	2022	<MLS> Updated Makefile to reflect RNS filename changes
#++	Jun 30,	2022	<MLS> Added dumpfits to makefile
#++	Oct 17,	2022	<MLS> Added _ENABLE_FOCUSER_MOONLITE_
#++	Oct 17,	2022	<MLS> Added _ENABLE_FILTERWHEEL_USIS_
#++	Mar  5,	2023	<MLS> Re-organizing object lists
#++	Dec  2,	2023	<MLS> Added piswitch3
######################################################################################
#	Cr_Core is for the Sony camera
######################################################################################

#PLATFORM			=	x86
#PLATFORM			=	x64
#PLATFORM			=	armv7

###########################################
#	lets try to determine platform
MACHINE_TYPE		=	$(shell uname -m)
PLATFORM			=	$(shell ./make_checkplatform.sh)
OPENCV_VERSION		=	$(shell ./make_checkopencv.sh)
SQL_VERSION			=	$(shell ./make_checksql.sh)

###########################################
# default settings for Desktop Linux build
USR_HOME			=	$(HOME)/
GCC_DIR				=	/usr/bin/
INCLUDE_BASE		=	/usr/include/
#LIB_BASE			=	/usr/lib/

#	/usr/local/lib/pkgconfig/opencv.pc
OPENCV_COMPILE		=	$(shell pkg-config --cflags $(OPENCV_VERSION))
OPENCV_LINK			=	$(shell pkg-config --libs $(OPENCV_VERSION))
OPENCV_LIB			=	/usr/local/lib


PHASEONE_INC		=	/usr/local/include/phaseone/include/
PHASEONE_LIB		=	/usr/local/lib/

SRC_DIR				=	./src/
SRC_IMGPROC			=	./src_imageproc/
SRC_IMU				=	./src_imu/
SRC_DISCOVERY		=	./src_discovery/
SRC_MOONRISE		=	./src_MoonRise/
SRC_SERVO			=	./src_servo/
SRC_PDS				=	./src_pds/
SRC_SKYIMAGE		=	./src_skyimage/

MLS_LIB_DIR			=	./src_mlsLib/
OBJECT_DIR			=	./Objectfiles/


GD_DIR				=	../gd/
ASI_LIB_DIR			=	./ASI_lib
ASI_INCLUDE_DIR		=	./ASI_lib/include
EFW_LIB_DIR			=	./EFW_linux_mac_SDK

############################################
#	as of Mar 18, 2021, supporting the AtikCamerasSDK_2020_10_19 version of ATIK
ATIK_DIR			=	./AtikCamerasSDK
ATIK_LIB_MASTER_DIR	=	$(ATIK_DIR)/lib
ATIK_INCLUDE_DIR	=	$(ATIK_DIR)/include
ATIK_INCLUDE_DIR2	=	$(ATIK_DIR)/inc
#ATIK_LIB_DIR		=	$(ATIK_LIB_MASTER_DIR)/linux/x64/NoFlyCapture
ATIK_LIB_DIR		=	$(ATIK_LIB_MASTER_DIR)/linux/64/NoFlyCapture
#ATIK_LIB_DIR_V129	=	$(ATIK_LIB_MASTER_DIR)/ARM/pi/pi3/x86/NoFlyCapture
ATIK_LIB_DIR_ARM32	=	$(ATIK_LIB_MASTER_DIR)/ARM/32/NoFlyCapture
ATIK_LIB_DIR_ARM64	=	$(ATIK_LIB_MASTER_DIR)/ARM/64/NoFlyCapture

ATIK_PLATFORM		=	unknown

ifeq ($(PLATFORM),  x64)
	ATIK_PLATFORM	=	linux/64
endif
ifeq ($(PLATFORM),  armv7)
	ATIK_PLATFORM	=	ARM/32
endif
ifeq ($(PLATFORM),  armv8)
	ATIK_PLATFORM	=	ARM/64
endif
ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/$(ATIK_PLATFORM)/NoFlyCapture

############################################
TOUP_DIR			=	./toupcamsdk
TOUP_INCLUDE_DIR	=	$(TOUP_DIR)/inc
TOUP_LIB_DIR		=	$(TOUP_DIR)/linux/x64

############################################
FLIR_INCLUDE_DIR	=	/usr/include/spinnaker


############################################
SONY_INCLUDE_DIR	=	./SONY_SDK/CRSDK
SONY_LIB_DIR		=	./SONY_SDK/lib

############################################
#	QHY support
QHY_INCLUDE_DIR		=	./QHY/include

############################################
#	QSI support
QSI_INCLUDE_DIR		=	./qsiapi-7.6.0

DEFINEFLAGS		+=	-D_INCLUDE_HTTP_HEADER_
DEFINEFLAGS		+=	-D_INCLUDE_ALPACA_EXTENSIONS_
DEFINEFLAGS		+=	-D_ALPACA_PI_
DEFINEFLAGS		+=	-D_USE_CAMERA_READ_THREAD_
CFLAGS			=	-Wall -Wno-multichar -Wno-unknown-pragmas -Wstrict-prototypes
CFLAGS			+=	-Wextra
#CFLAGS			+=	-Werror
CFLAGS			+=	-Wmissing-prototypes
#CFLAGS			+=	-trigraphs
CFLAGS			+=	-g
#CFLAGS			+=	-Wno-unused-but-set-variable
#CFLAGS			+=	-Wstrict-prototypes
#CFLAGS			+=	-mx32
CFLAGS			+=	-fPIE
CFLAGS			+=	-Wno-implicit-fallthrough

CPLUSFLAGS		=	-Wall -Wno-multichar -Wno-unknown-pragmas
CPLUSFLAGS		+=	-Wextra
CPLUSFLAGS		+=	-Wuninitialized
CPLUSFLAGS		+=	-Wmaybe-uninitialized
CPLUSFLAGS		+=	-Wno-unused-parameter
#CPLUSFLAGS		+=	-Wno-class-memaccess
#CPLUSFLAGS		+=	-O2
#CPLUSFLAGS		+=	-trigraphs
CPLUSFLAGS		+=	-g
#CPLUSFLAGS		+=	-Wno-unused-but-set-variable
CPLUSFLAGS		+=	-fPIE
CPLUSFLAGS		+=	-Wno-format-overflow
CPLUSFLAGS		+=	-Wno-implicit-fallthrough


COMPILE			=	gcc -c $(CFLAGS) $(DEFINEFLAGS) $(OPENCV_COMPILE)
COMPILEPLUS		=	g++ -c $(CPLUSFLAGS) $(DEFINEFLAGS) $(OPENCV_COMPILE)
LINK			=	g++


INCLUDES		=	-I/usr/include					\
					-I/usr/local/include			\
					-I$(SRC_DIR)					\
					-I$(SRC_SKYIMAGE)				\
					-I$(ASI_INCLUDE_DIR)			\
					-I$(ATIK_INCLUDE_DIR)			\
					-I$(ATIK_INCLUDE_DIR2)			\
					-I$(EFW_LIB_DIR)				\
					-I$(FLIR_INCLUDE_DIR)			\
					-I$(MLS_LIB_DIR)				\
					-I$(QHY_INCLUDE_DIR)			\
					-I$(SRC_IMGPROC)				\
					-I$(SRC_PDS)					\
					-I$(TOUP_INCLUDE_DIR)			\
					-I$(SONY_INCLUDE_DIR)			\


#					-I/usr/include/opencv2			\
#					-I/usr/local/include/opencv2	\
#					-I/usr/include/opencv4			\
#					-I/usr/local/include/opencv4	\


######################################################################################
ASI_CAMERA_OBJECTS=												\
				$(ASI_LIB_DIR)/lib/$(PLATFORM)/libASICamera2.a	\


######################################################################################
ZWO_EFW_OBJECTS=												\
				$(EFW_LIB_DIR)/lib/$(PLATFORM)/libEFWFilter.a	\



######################################################################################
SOCKET_OBJECTS=												\
				$(OBJECT_DIR)socket_listen.o				\
				$(OBJECT_DIR)json_parse.o					\
				$(OBJECT_DIR)sendrequest_lib.o				\


######################################################################################
DISCOVERY_LIB_OBJECTS=										\
				$(OBJECT_DIR)discovery_lib.o				\



######################################################################################
# CPP objects
CPP_OBJECTS=												\
				$(OBJECT_DIR)cpu_stats.o					\
				$(OBJECT_DIR)discoverythread.o				\
				$(OBJECT_DIR)eventlogging.o					\
				$(OBJECT_DIR)HostNames.o					\
				$(OBJECT_DIR)JsonResponse.o					\
				$(OBJECT_DIR)linuxerrors.o					\
				$(OBJECT_DIR)lx200_com.o					\
				$(OBJECT_DIR)managementdriver.o				\
				$(OBJECT_DIR)observatory_settings.o			\
				$(OBJECT_DIR)readconfigfile.o				\
				$(OBJECT_DIR)sidereal.o						\
				$(OBJECT_DIR)serialport.o					\
				$(OBJECT_DIR)telescopedriver.o				\
				$(OBJECT_DIR)telescopedriver_comm.o			\
				$(OBJECT_DIR)telescopedriver_lx200.o		\
				$(OBJECT_DIR)telescopedriver_Rigel.o		\
				$(OBJECT_DIR)telescopedriver_servo.o		\
				$(OBJECT_DIR)telescopedriver_sim.o			\
				$(OBJECT_DIR)telescopedriver_skywatch.o		\


######################################################################################
LIVE_WINDOW_OBJECTS=										\
				$(OBJECT_DIR)controller.o					\
				$(OBJECT_DIR)controller_image.o				\
				$(OBJECT_DIR)opencv_utils.o					\
				$(OBJECT_DIR)windowtab.o					\
				$(OBJECT_DIR)windowtab_about.o				\
				$(OBJECT_DIR)windowtab_fitsheader.o			\
				$(OBJECT_DIR)windowtab_image.o				\
				$(OBJECT_DIR)windowtab_imageinfo.o			\
				$(OBJECT_DIR)fits_opencv.o					\


#				$(OBJECT_DIR)controllerAlpaca.o				\

######################################################################################
#	Driver Objects
DRIVER_OBJECTS=												\
				$(OBJECT_DIR)alpacadriver.o					\
				$(OBJECT_DIR)alpacadriverConnect.o			\
				$(OBJECT_DIR)alpacadriverSetup.o			\
				$(OBJECT_DIR)alpacadriverThread.o			\
				$(OBJECT_DIR)alpacadriver_templog.o			\
				$(OBJECT_DIR)alpacadriver_helper.o			\
				$(OBJECT_DIR)alpaca_discovery.o				\
				$(OBJECT_DIR)alpacadriverLogging.o			\
				$(OBJECT_DIR)commoncolor.o					\
				$(OBJECT_DIR)julianTime.o					\
				$(OBJECT_DIR)moonphase.o					\
				$(OBJECT_DIR)MoonRise.o						\
				$(OBJECT_DIR)cpu_stats.o					\
				$(OBJECT_DIR)discoverythread.o				\
				$(OBJECT_DIR)eventlogging.o					\
				$(OBJECT_DIR)HostNames.o					\
				$(OBJECT_DIR)JsonResponse.o					\
				$(OBJECT_DIR)linuxerrors.o					\
				$(OBJECT_DIR)managementdriver.o				\
				$(OBJECT_DIR)observatory_settings.o			\
				$(OBJECT_DIR)readconfigfile.o				\
				$(OBJECT_DIR)sidereal.o						\

######################################################################################
# Camera objects
CAMERA_DRIVER_OBJECTS=										\
				$(OBJECT_DIR)cameradriver.o					\
				$(OBJECT_DIR)cameradriverAnalysis.o			\
				$(OBJECT_DIR)cameradriver_ASI.o				\
				$(OBJECT_DIR)cameradriver_ATIK.o			\
				$(OBJECT_DIR)cameradriver_fits.o			\
				$(OBJECT_DIR)cameradriver_gps.o				\
				$(OBJECT_DIR)cameradriver_FLIR.o			\
				$(OBJECT_DIR)cameradriver_jpeg.o			\
				$(OBJECT_DIR)cameradriver_livewindow.o		\
				$(OBJECT_DIR)cameradriver_opencv.o			\
				$(OBJECT_DIR)cameradriver_overlay.o			\
				$(OBJECT_DIR)cameradriver_png.o				\
				$(OBJECT_DIR)cameradriver_QHY.o				\
				$(OBJECT_DIR)cameradriver_QSI.o				\
				$(OBJECT_DIR)cameradriver_readthread.o		\
				$(OBJECT_DIR)cameradriver_SONY.o			\
				$(OBJECT_DIR)cameradriver_save.o			\
				$(OBJECT_DIR)cameradriver_sim.o				\
				$(OBJECT_DIR)cameradriver_TOUP.o			\
				$(OBJECT_DIR)multicam.o						\
				$(OBJECT_DIR)ParseNMEA.o					\
				$(OBJECT_DIR)NMEA_helper.o					\


######################################################################################
CALIBRATION_DRIVER_OBJECTS=									\
				$(OBJECT_DIR)calibrationdriver.o			\
				$(OBJECT_DIR)calibrationdriver_rpi.o		\
				$(OBJECT_DIR)calibration_Alnitak.o		\
				$(OBJECT_DIR)calibration_sim.o				\

######################################################################################
DOME_DRIVER_OBJECTS=										\
				$(OBJECT_DIR)domedriver.o					\
				$(OBJECT_DIR)domedriver_sim.o				\
				$(OBJECT_DIR)domeshutter.o					\
				$(OBJECT_DIR)domedriver_rpi.o				\
				$(OBJECT_DIR)domedriver_ror_rpi.o			\
				$(OBJECT_DIR)raspberrypi_relaylib.o			\

######################################################################################
SHUTTER_DRIVER_OBJECTS=										\
				$(OBJECT_DIR)shutterdriver.o				\
				$(OBJECT_DIR)shutterdriver_arduino.o		\

######################################################################################
# Filterwheel objects
FITLERWHEEL_DRIVER_OBJECTS=									\
				$(OBJECT_DIR)filterwheeldriver.o			\
				$(OBJECT_DIR)filterwheeldriver_ATIK.o		\
				$(OBJECT_DIR)filterwheeldriver_QHY.o		\
				$(OBJECT_DIR)filterwheeldriver_ZWO.o		\
				$(OBJECT_DIR)filterwheeldriver_sim.o		\

######################################################################################
FOCUSER_DRIVER_OBJECTS=										\
				$(OBJECT_DIR)focuserdriver.o				\
				$(OBJECT_DIR)focuserdriver_nc.o				\
				$(OBJECT_DIR)focuserdriver_sim.o			\
				$(OBJECT_DIR)moonlite_com.o					\
				$(OBJECT_DIR)rotatordriver.o				\
				$(OBJECT_DIR)rotatordriver_nc.o				\
				$(OBJECT_DIR)rotatordriver_sim.o			\

######################################################################################
SLITTRACKER_DRIVER_OBJECTS=									\
				$(OBJECT_DIR)slittracker.o					\

######################################################################################
OBSCOND_DRIVER_OBJECTS=										\
				$(OBJECT_DIR)obsconditionsdriver.o			\
				$(OBJECT_DIR)obsconditionsdriver_rpi.o		\
				$(OBJECT_DIR)obsconditionsdriver_sim.o		\


######################################################################################
SWITCH_DRIVER_OBJECTS=										\
				$(OBJECT_DIR)switchdriver.o					\
				$(OBJECT_DIR)switchdriver_rpi.o				\
				$(OBJECT_DIR)switchdriver_sim.o				\

######################################################################################
TELESCOPE_DRIVER_OBJECTS=									\
				$(OBJECT_DIR)telescopedriver.o				\
				$(OBJECT_DIR)telescopedriver_comm.o			\
				$(OBJECT_DIR)telescopedriver_lx200.o		\
				$(OBJECT_DIR)telescopedriver_Rigel.o		\
				$(OBJECT_DIR)telescopedriver_servo.o		\
				$(OBJECT_DIR)telescopedriver_sim.o			\
				$(OBJECT_DIR)lx200_com.o					\

######################################################################################
SERIAL_OBJECTS=												\
				$(OBJECT_DIR)serialport.o					\
				$(OBJECT_DIR)usbmanager.o					\

######################################################################################
TEST_OBJECTS=												\
				$(OBJECT_DIR)cameradriver_PhaseOne.o		\

######################################################################################
#	Camera Objects
IMAGEPROC_OBJECTS=											\
				$(OBJECT_DIR)imageprocess_orb.o				\


CLIENT_OBJECTS=												\
				$(OBJECT_DIR)json_parse.o					\
				$(OBJECT_DIR)discoveryclient.o				\

HELPER_OBJECTS=												\
				$(OBJECT_DIR)helper_functions.o				\


######################################################################################
#	Roll Off Roof Objects
ROR_OBJECTS=												\
				$(OBJECT_DIR)alpacadriver.o					\
				$(OBJECT_DIR)alpacadriverConnect.o			\
				$(OBJECT_DIR)alpacadriverSetup.o			\
				$(OBJECT_DIR)alpacadriverThread.o			\
				$(OBJECT_DIR)alpacadriver_helper.o			\
				$(OBJECT_DIR)alpacadriverLogging.o			\
				$(OBJECT_DIR)alpaca_discovery.o				\
				$(OBJECT_DIR)cpu_stats.o					\
				$(OBJECT_DIR)discoverythread.o				\
				$(OBJECT_DIR)domedriver.o					\
				$(OBJECT_DIR)domedriver_ror_rpi.o			\
				$(OBJECT_DIR)eventlogging.o					\
				$(OBJECT_DIR)HostNames.o					\
				$(OBJECT_DIR)JsonResponse.o					\
				$(OBJECT_DIR)linuxerrors.o					\
				$(OBJECT_DIR)managementdriver.o				\
				$(OBJECT_DIR)observatory_settings.o			\
				$(OBJECT_DIR)raspberrypi_relaylib.o			\

######################################################################################
# CPP objects
IMU_OBJECTS=												\
				$(OBJECT_DIR)imu_lib.o						\
				$(OBJECT_DIR)imu_lib_bno055.o				\
				$(OBJECT_DIR)imu_lib_LIS2DH12.o				\
				$(OBJECT_DIR)i2c_bno055.o					\

######################################################################################
#pragma mark make help
help:
	#################################################################################
	# The AlpacaPi project consists of two main parts, drivers and clients,
	#	There are 2 major variants that to be dealt with opencv4 and opencv
	#	The newer opencv4 variant only supports the C++ interface
	#	AlpacaPi was originally written with the C interface
	#   Once everything is converted to opencv4, the opencv options will go away
	#
	#	Driver make options
	#        make dome          Raspberry pi version to control dome using DC motor controller
	#
	#     opencv4 only options
	#        make alpacapicv4   Driver for x86 linux
	#        make camerasim     Camera simulator
	#        make simulator     Several different simulators
	#        make picv4         Version for Raspberry Pi using OpenCV 4 or later

	#     opencv only options
	#        make alpacapi       Driver for x86 linux
	#        make jetson         Version to run on nvidia jetson board, this is an armv8
	#        make moonlite       Driver for moonlite focusers ONLY
	#        make nocamera       Build without the camera support
	#        make noopencv       Camera driver for ZWO WITHOUT opencv
	#        make pi             Version for Raspberry Pi
	#        make piqhy          Camera driver for QHY cameras only for Raspberry-Pi
	#        make qsi            Camera driver for QSI cameras
	#        make wx             Version that uses the R-Pi sensor board
	#
	#
	# Telescope drivers,
	# As of May 2022, the telescope driver is still in development,
	# There are several options that are in progress
	#        make tele      Makes a version which speaks LX200 over a TCP/IP connection
	#        make rigel     Makes a special version for a user that uses a rigel controller
	#        make eq6       A version to control eq6 style mounts
	#        make servo     A telescope controller based on servo motors using LM628/629
	#
	# Miscellaneous
	#        make clean      removes all binaries
	#        make help       this message
	#
	#    Client make options
	#       SkyTravel is an all in one client program, it has all of the controllers built in
	#       with full Alpaca Discovery support and generates a list of available devices
	#
	#       make sky         makes SkyTravel with openCV 3.3.1 or earlier
	#       make skysql      same as sky but with SQL database support
	#>      make skycv4      makes SkyTravel with newer Versions after 3.3.1
	#>      make skycv4sql   same as skycv4 with SQL database support
	#
	#   Some of the clients can also be built separately
	#       make camera
	#       make domectrl
	#       make focuser
	#       make switch
	#
	# MACHINE_TYPE  =$(MACHINE_TYPE)
	# PLATFORM      =$(PLATFORM)
	# OPENCV_VERSION=$(OPENCV_VERSION)
	# SQL_VERSION   =$(SQL_VERSION)
	# ATIK_PLATFORM =$(ATIK_PLATFORM)
	#################################################################################



#	Debug                     Makefile
#        smate      Build a version to run on a Stellarmate running smate OS



######################################################################################
#pragma mark make alpacapi  C++ linux-x86
alpacapi		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
alpacapi		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_LX200_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
alpacapi		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
alpacapi		:									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(ASI_CAMERA_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ZWO_EFW_OBJECTS)				\
					-ludev							\
					-lusb-1.0						\
					-lpthread						\
					-lcfitsio						\
					-o alpacapi


#					-L$(ATIK_LIB_DIR)/			\
#					-latikcameras				\
#					-lqhyccd					\

######################################################################################
#pragma mark make alpacapicv4  C++ linux-x86
alpacapicv4		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
alpacapicv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#alpacapicv4	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_LX200_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
alpacapicv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
alpacapicv4		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
alpacapicv4		:									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\


		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					$(ZWO_EFW_OBJECTS)				\
					-ludev							\
					-lusb-1.0						\
					-lpthread						\
					-lcfitsio						\
					-o alpacapi


#					-L$(ATIK_LIB_DIR)/			\
#					-latikcameras				\
#					-lqhyccd					\

######################################################################################
#pragma mark make mulc
mulc		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
#mulc		:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#mulc		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
#mulc	:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
mulc		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
mulc		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
mulc		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
mulc		:									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					$(ZWO_EFW_OBJECTS)				\
					-L$(ATIK_LIB_DIR)/				\
					-latikcameras					\
					-ludev							\
					-lusb-1.0						\
					-lpthread						\
					-lcfitsio						\
					-o alpacapi

#					-lqhyccd						\
#					-lSpinnaker_C					\


######################################################################################
#	make simulator  several simulators
simulator	:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_DOME_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
simulator	:	DEFINEFLAGS		+=	-D_ENABLE_SWITCH_SIMULATOR_
simulator	:	DEFINEFLAGS		+=	-D_USE_OPENCV_
simulator	:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
simulator	:										\
					$(DRIVER_OBJECTS)				\
					$(HELPER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(DOME_DRIVER_OBJECTS)			\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(OBSCOND_DRIVER_OBJECTS)		\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(SERIAL_OBJECTS)				\
					$(TELESCOPE_DRIVER_OBJECTS)		\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\


		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(HELPER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(DOME_DRIVER_OBJECTS)			\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(OBSCOND_DRIVER_OBJECTS)		\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(SERIAL_OBJECTS)				\
					$(TELESCOPE_DRIVER_OBJECTS)		\
					$(LIVE_WINDOW_OBJECTS)			\
					$(SOCKET_OBJECTS)				\
					$(OPENCV_LINK)					\
					-lcfitsio						\
					-lpthread						\
					-o alpacasim

#					-lwiringPi					\
#					-lusb-1.0					\
#					-ludev						\





######################################################################################
#pragma mark make moonlite
moonlite		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
moonlite		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
moonlite		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
moonlite		:
					$(DRIVER_OBJECTS)			\
					$(FOCUSER_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(FOCUSER_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-o alpaca-moonlite





######################################################################################
#pragma mark make allcam
#	this is primarily for development, all cameras are enabled
allcam		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#allcam		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
#allcam		:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_SONY_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
allcam		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
allcam		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
#allcam		:		INCLUDES		+=	-I$(SONY_INCLUDE_DIR)
allcam		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
allcam		:									\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(LIVE_WINDOW_OBJECTS)		\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(LIVE_WINDOW_OBJECTS)		\
					$(SOCKET_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					-L$(TOUP_LIB_DIR)/			\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-latikcameras				\
					-ltoupcam					\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-lqhyccd					\
					-o alpacapi

#					-lSpinnaker_C				\


######################################################################################
#pragma mark make tele  C++ linux-x86
tele		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_ROR_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
tele	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_LX200_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_LX200_COM_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_IMU_
tele	:		INCLUDES		+=	-I$(SRC_IMU)
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_SKYWATCH_
tele	:										\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)	\
					$(TELESCOPE_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SERIAL_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)	\
					$(TELESCOPE_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SERIAL_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					-lusb-1.0					\
					-lcfitsio					\
					-li2c						\
					-lpthread					\
					-o alpacapi-telescope


#					-ludev						\

######################################################################################
#pragma mark make imutest
imutest		:		DEFINEFLAGS		+=	-D_INCLUDE_IMU_MAIN_DH12_
imutest		:		INCLUDES		+=	-I$(SRC_IMU)
imutest		:		$(IMU_OBJECTS)				\

		$(LINK)  								\
					$(IMU_OBJECTS)				\
					-lpthread					\
					-li2c						\
					-o imutest

######################################################################################
#pragma mark make telecv4  C++ linux-x86
telecv4	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
telecv4	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
telecv4	:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
telecv4	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
telecv4	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
telecv4	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
telecv4	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_LX200_
telecv4	:		DEFINEFLAGS		+=	-D_ENABLE_LX200_COM_
telecv4	:		DEFINEFLAGS		+=	-D_ENABLE_IMU_
telecv4	:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
telecv4	:		INCLUDES		+=	-I$(SRC_IMU)
#telecv4	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_SKYWATCH_
telecv4	:										\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)	\
					$(TELESCOPE_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)	\
					$(TELESCOPE_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					-lusb-1.0					\
					-lcfitsio					\
					-lpthread					\
					-o alpacapi-telescope



######################################################################################
#pragma mark make eq6
eq6		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
eq6		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#eq6	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_LX200_
#eq6	:		DEFINEFLAGS		+=	-D_ENABLE_LX200_COM_
eq6		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_SKYWATCH_
eq6		:										\
					$(DRIVER_OBJECTS)			\
					$(TELESCOPE_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SERIAL_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(TELESCOPE_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SERIAL_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-o alpacapi-eq6

######################################################################################
#make newt16
#pragma mark Newt 16 C++ Raspberry pi
newt16		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_QHY_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
newt16		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
newt16		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
newt16		:										\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SLITTRACKER_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SLITTRACKER_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					$(ZWO_EFW_OBJECTS)				\
					-lcfitsio						\
					-lusb-1.0						\
					-ludev							\
					-ljpeg							\
					-lpthread						\
					-lqhyccd						\
					-o alpacapi

#					-lwiringPi					\


######################################################################################
#pragma mark make wo71
#	this is for my William Optics 71 configuration
wo71		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
wo71		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
wo71		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
wo71		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
wo71		:										\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\


		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					$(ASI_CAMERA_OBJECTS)			\
					$(ZWO_EFW_OBJECTS)				\
					-lcfitsio						\
					-lusb-1.0						\
					-ludev							\
					-lpthread						\
					-o alpacapi


#					-lqhyccd					\
#					-L$(ATIK_LIB_DIR)/			\
#					-latikcameras				\


######################################################################################
#pragma mark make wo102
#	this is for my William Optics 102 configuration
wo102		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_ALNITAK_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
wo102		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
wo102		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
#wo102		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
wo102		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
wo102		:										\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(TELESCOPE_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(TELESCOPE_DRIVER_OBJECTS)		\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					$(ZWO_EFW_OBJECTS)				\
					-ljpeg							\
					-lqhyccd						\
					-lcfitsio						\
					-lusb-1.0						\
					-ludev							\
					-lpthread						\
					-o alpacapi



######################################################################################
TELESCOPE_OBJECTS=											\
				$(OBJECT_DIR)alpacadriver.o					\
				$(OBJECT_DIR)alpacadriverConnect.o			\
				$(OBJECT_DIR)alpacadriverSetup.o			\
				$(OBJECT_DIR)alpacadriverThread.o			\
				$(OBJECT_DIR)alpacadriver_helper.o			\
				$(OBJECT_DIR)alpacadriver_templog.o			\
				$(OBJECT_DIR)alpaca_discovery.o				\
				$(OBJECT_DIR)alpacadriverLogging.o			\
				$(OBJECT_DIR)discoverythread.o				\
				$(OBJECT_DIR)eventlogging.o					\
				$(OBJECT_DIR)HostNames.o					\
				$(OBJECT_DIR)JsonResponse.o					\
				$(OBJECT_DIR)linuxerrors.o					\
				$(OBJECT_DIR)managementdriver.o				\
				$(OBJECT_DIR)observatory_settings.o			\
				$(OBJECT_DIR)readconfigfile.o				\
				$(OBJECT_DIR)serialport.o					\
				$(OBJECT_DIR)sidereal.o						\
				$(OBJECT_DIR)telescopedriver.o				\
				$(OBJECT_DIR)telescopedriver_comm.o			\
				$(OBJECT_DIR)telescopedriver_Rigel.o		\
				$(OBJECT_DIR)telescopedriver_servo.o		\
				$(OBJECT_DIR)cpu_stats.o					\


######################################################################################
#pragma mark make rigel
rigel		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
rigel		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
rigel		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_RIGEL_
rigel		:									\
					$(TELESCOPE_OBJECTS)		\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(TELESCOPE_OBJECTS)		\
					-lpthread					\
					-o alpacapi-rigel

######################################################################################
SERVO_OBJECTS=										\
				$(OBJECT_DIR)servo_mount_cfg.o		\
				$(OBJECT_DIR)servo_time.o			\
				$(OBJECT_DIR)servo_motion.o			\
				$(OBJECT_DIR)servo_motion_cfg.o		\
				$(OBJECT_DIR)servo_mount.o			\
				$(OBJECT_DIR)servo_observ_cfg.o		\
				$(OBJECT_DIR)servo_rc_utils.o		\
				$(OBJECT_DIR)servo_mc_core.o		\


######################################################################################
#pragma mark make servo
servo		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
servo		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
servo		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_SERVO_
servo		:		INCLUDES		+=	-I$(SRC_SERVO)
servo		:										\
					$(DRIVER_OBJECTS)				\
					$(TELESCOPE_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(SERVO_OBJECTS)				\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(TELESCOPE_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(SERVO_OBJECTS)				\
					-lpthread						\
					-o alpacapi-servo


######################################################################################
#pragma mark make servoimu
servoimu	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
servoimu	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
servoimu	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_SERVO_
servoimu	:		DEFINEFLAGS		+=	-D_ENABLE_IMU_
servoimu	:		INCLUDES		+=	-I$(SRC_IMU)
servoimu	:		INCLUDES		+=	-I$(SRC_SERVO)
servoimu	:										\
					$(DRIVER_OBJECTS)				\
					$(TELESCOPE_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERVO_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(IMU_OBJECTS)					\


		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(TELESCOPE_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERVO_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(IMU_OBJECTS)					\
					-lpthread						\
					-o alpacapi-servo

######################################################################################
#pragma mark make toup
toup	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
toup	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
toup	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
toup	:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
toup	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
toup	:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
toup	:										\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\


		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(TOUP_LIB_DIR)/			\
					-ltoupcam					\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-o alpacapi

######################################################################################
#pragma mark make touppi
touppi	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
touppi	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
touppi	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#touppi		:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
touppi	:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
touppi	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
touppi	:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
touppi	:		TOUP_LIB_DIR	=	$(TOUP_DIR)/linux/arm64
touppi	:										\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(TOUP_LIB_DIR)/			\
					-ltoupcam					\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-o alpacapi


#					$(CPP_OBJECTS)				\

######################################################################################
#pragma mark C++ linux-x86
Release		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
Release		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
Release		:									\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					-L$(TOUP_LIB_DIR)/			\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-latikcameras				\
					-ltoupcam					\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-lqhyccd					\
					-o bin/Release/alpaca



######################################################################################
#pragma mark Flir version for x86
#make flir
flir		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_IMU_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
flir		:		INCLUDES		+=	-I$(SRC_IMU)
flir		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
flir		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
flir		:									\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SERIAL_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(LIVE_WINDOW_OBJECTS)		\
					$(IMU_OBJECTS)				\


		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SERIAL_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(LIVE_WINDOW_OBJECTS)		\
					$(IMU_OBJECTS)				\
					$(OPENCV_LINK)				\
					-lSpinnaker_C				\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-o alpacapi

######################################################################################
#pragma mark phaseone version for x86
#make phaseone
phaseone	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
phaseone	:		DEFINEFLAGS		+=	-D_ENABLE_PHASEONE_
phaseone	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
phaseone	:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
phaseone	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
phaseone	:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
phaseone	:		DEFINEFLAGS		+=	-D_ENABLE_PHASEONE_
phaseone	:		INCLUDES		+=	-I$(PHASEONE_INC)
phaseone	:									\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(TEST_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\

		$(LINK)  								\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(TEST_OBJECTS)				\
					$(SOCKET_OBJECTS)			\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(PHASEONE_LIB)			\
					-lCameraSdkCpp				\
					-lpthread					\
					-lcfitsio					\
					-o alpacapi

#					-lusb-1.0					\
#					-ludev						\
#					$(CPP_OBJECTS)				\



######################################################################################
#pragma mark imu
#make imu
imu		:			DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
imu		:			DEFINEFLAGS		+=	-D_ENABLE_ASI_
imu		:			DEFINEFLAGS		+=	-D_ENABLE_FITS_
imu		:			DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
imu		:			DEFINEFLAGS		+=	-D_ENABLE_IMU_
imu		:			INCLUDES		+=	-I$(SRC_IMU)
imu		:			DEFINEFLAGS		+=	-D_USE_OPENCV_
imu		:			DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
imu		:										\
					$(CPP_OBJECTS)				\
					$(HELPER_OBJECTS)			\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\


		$(LINK)  								\
					$(CPP_OBJECTS)				\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(IMU_OBJECTS)				\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-o imu


#					-ludev						\

######################################################################################
#pragma mark nousb -C++ linux-x86
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
nousb		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_SHUTTER_
nousb		:									\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					-lpthread					\
					-lcfitsio					\
					-o alpacapi

######################################################################################
#pragma mark dome
#make dome
#dome		:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
dome		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
dome		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_SHUTTER_
dome		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_RPI_
dome		:	DEFINEFLAGS		+=	-D_ENABLE_REMOTE_SHUTTER_
dome		:	DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
dome		:									\
					$(DRIVER_OBJECTS)			\
					$(DOME_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\

				$(LINK)  						\
					$(DRIVER_OBJECTS)			\
					$(DOME_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					-lusb-1.0					\
					-lpthread					\
					-lwiringPi					\
					-o domedriver

#							-ludev						\
#							$(ASI_CAMERA_OBJECTS)		\


######################################################################################
#pragma mark ROR
#make rortest
#rortest	:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
rortest		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_ROR_
rortest		:								\
					$(ROR_OBJECTS)			\
					$(SOCKET_OBJECTS)		\

				$(LINK)  					\
					$(ROR_OBJECTS)			\
					$(SOCKET_OBJECTS)		\
					-lpthread				\
					-o ror

######################################################################################
#pragma mark rorpi
#	make rorpi
rorpi		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
rorpi		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_ROR_
rorpi		:	DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
rorpi		:	DEFINEFLAGS		+=	-D_ENABLE_4RELAY_BOARD_
rorpi		:	DEFINEFLAGS		+=	-D_CHRIS_A_ROLL_OFF_ROOF_
rorpi		:									\
					$(DRIVER_OBJECTS)			\
					$(DOME_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\

				$(LINK)  						\
					$(DRIVER_OBJECTS)			\
					$(DOME_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					-lpthread					\
					-lwiringPi					\
					-o ror


######################################################################################
#pragma mark topens
#	make topens
topens		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
topens		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_ROR_
topens		:	DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
topens		:	DEFINEFLAGS		+=	-D_ENABLE_4RELAY_BOARD_
topens		:	DEFINEFLAGS		+=	-D_TOPENS_ROLL_OFF_ROOF_
topens		:									\
					$(DRIVER_OBJECTS)			\
					$(DOME_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\

				$(LINK)  						\
					$(DRIVER_OBJECTS)			\
					$(DOME_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					-lpthread					\
					-lwiringPi					\
					-o ror

######################################################################################
#pragma mark topens_calib
#	make topens_calib
topens_calib		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
topens_calib		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_ROR_
topens_calib		:	DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
topens_calib		:	DEFINEFLAGS		+=	-D_ENABLE_4RELAY_BOARD_
topens_calib		:	DEFINEFLAGS		+=	-D_TOPENS_ROLL_OFF_ROOF_
topens_calib		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
topens_calib		:	DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
topens_calib		:	DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_RPI_
topens_calib		:								\
					$(DRIVER_OBJECTS)				\
					$(DOME_DRIVER_OBJECTS)			\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)				\
					$(SOCKET_OBJECTS)				\

				$(LINK)								\
					$(DRIVER_OBJECTS)				\
					$(DOME_DRIVER_OBJECTS)			\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					-lpthread						\
					-lwiringPi						\
					-o ror

######################################################################################
steve : DEFINEFLAGS += -D_ENABLE_DOME_
steve : DEFINEFLAGS += -D_ENABLE_DOME_ROR_
steve : DEFINEFLAGS += -D_ENABLE_WIRING_PI_
steve : DEFINEFLAGS += -D_ENABLE_4RELAY_BOARD_
steve : DEFINEFLAGS += -D_TOPENS_ROLL_OFF_ROOF_
steve : DEFINEFLAGS += -D_INCLUDE_MILLIS_
steve : DEFINEFLAGS += -D_ENABLE_CALIBRATION_
steve : DEFINEFLAGS += -D_ENABLE_CALIBRATION_RPI_
steve : \
	$(DRIVER_OBJECTS) \
	$(DOME_DRIVER_OBJECTS) \
	$(CALIBRATION_DRIVER_OBJECTS) \
	$(HELPER_OBJECTS) \
	$(SOCKET_OBJECTS) \

	$(LINK)   \
		$(DRIVER_OBJECTS) \
		$(CALIBRATION_DRIVER_OBJECTS) \
		$(DOME_DRIVER_OBJECTS) \
		$(HELPER_OBJECTS) \
		$(SOCKET_OBJECTS) \
		-lpthread \
		-lusb-1.0 \
		-lwiringPi \
		-o ror \


######################################################################################
#pragma mark rorpi2
#rorpi2		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
rorpi2		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_ROR_
rorpi2		:	DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
rorpi2		:	DEFINEFLAGS		+=	-D_ENABLE_4RELAY_BOARD_
rorpi2		:	DEFINEFLAGS		+=	-D_TEST_DISCOVERQUERY_
rorpi2		:									\
						$(ROR_OBJECTS)			\
						$(SOCKET_OBJECTS)		\

				$(LINK)  						\
						$(SOCKET_OBJECTS)		\
						$(ROR_OBJECTS)			\
						-lpthread				\
						-lwiringPi				\
						-o ror2



######################################################################################
#pragma mark Raspberry pi - calibration
#	this is a stand alone R-Pi based calibration device that uses the PWM power board
#	or any other PWM based control system
#make calib
######################################################################################
calib		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
calib		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
calib		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_RPI_
calib		:										\
					$(DRIVER_OBJECTS)				\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)				\
					$(SOCKET_OBJECTS)				\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					-lpthread						\
					-lusb-1.0						\
					-lwiringPi						\
					-o alpacapi-calib



######################################################################################
#pragma mark Raspberry pi - calibration
#make calibcv4
calibcv4		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
calibcv4		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
calibcv4		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_RPI_
calibcv4		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
calibcv4		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
calibcv4		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
calibcv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
calibcv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
#calibcv4		:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
calibcv4		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
calibcv4		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
calibcv4		:										\
					$(DRIVER_OBJECTS)				\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(SOCKET_OBJECTS)				\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(SOCKET_OBJECTS)				\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					-lcfitsio						\
					-lpthread						\
					-lusb-1.0						\
					-lwiringPi						\
					-o alpacapi-calib



######################################################################################
#pragma mark Alnitak - calibration
#make alnitak
alnitak		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
alnitak		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
alnitak		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_ALNITAK_
alnitak		:										\
					$(DRIVER_OBJECTS)				\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)				\
					$(TEST_OBJECTS)					\
					$(SERIAL_OBJECTS)				\
					$(HELPER_OBJECTS)				\


		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CALIBRATION_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)				\
					$(TEST_OBJECTS)					\
					$(SERIAL_OBJECTS)				\
					$(HELPER_OBJECTS)				\
					-lpthread						\
					-o alpacapi-alnitak

#					-lusb-1.0					\
#					$(CPP_OBJECTS)				\
#					$(CPP_OBJECTS)				\

######################################################################################
#	Camera Objects
SONY_OBJECTS=												\
				$(OBJECT_DIR)cameradriver_SONY.o			\

######################################################################################
#make sony
sony		:		DEFINEFLAGS		+=	-D_ENABLE_SONY_
sony		:		DEFINEFLAGS		+=	-D_INCLUDE_SONY_MAIN_
sony		:		CPLUSFLAGS		+=	-fsigned-char
sony		:		CPLUSFLAGS		+=	-std=gnu++17
sony		:		CPLUSFLAGS		+=	-O3 -DNDEBUG -std=c++17
sony		:		INCLUDES		+=	-I$(SONY_INCLUDE_DIR)
sony		:		$(SONY_OBJECTS)

		$(LINK)  								\
					$(SONY_OBJECTS)				\
					-L$(SONY_LIB_DIR)			\
					-lCr_Core					\
					-o sony

######################################################################################
#pragma mark make pi
pi		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
pi		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
pi		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
pi		:											\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-lcfitsio					\
					-lusb-1.0					\
					-ludev						\
					-lwiringPi					\
					-lpthread					\
					-o alpacapi

######################################################################################
#pragma mark make picv4
#picv4		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#picv4		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
#picv4		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
picv4		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
picv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
picv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
picv4		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
picv4		:										\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					$(ZWO_EFW_OBJECTS)				\
					-lcfitsio						\
					-lusb-1.0						\
					-ludev							\
					-lpthread						\
					-o alpacapi

#					-lwiringPi					\
#					-latikcameras				\
#					-L$(ATIK_LIB_DIR_ARM32)/	\


######################################################################################
#pragma mark make pi64
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
pi64		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
pi64		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
pi64		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
pi64		:		CPLUSFLAGS		+=	-fsigned-char
pi64		:		CPLUSFLAGS		+=	-std=gnu++17
#pi64		:		INCLUDES		+=	-I$(SONY_INCLUDE_DIR)
pi64		:									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\


		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					-L$(ATIK_LIB_DIR)				\
					-latikcameras					\
					-lcfitsio						\
					-lusb-1.0						\
					-lpthread						\
					-o alpacapi

#					-ludev						\
#					-L$(SONY_LIB_DIR)/			\
#					$(ZWO_EFW_OBJECTS)			\
#					-ltoupcam					\
#					-lwiringPi					\


######################################################################################
#pragma mark C++ Raspberry pi zwo only
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
pizwo		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
pizwo		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
#pizwo		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
pizwo		:		$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					-lusb-1.0					\
					-ludev						\
					-lpthread					\
					-o alpacapi

######################################################################################
#pragma mark Raspberry pi - WaveShare RPI RELAY BOARD (3 relays
#make piswitch3 using 3 relay board
piswitch3	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
piswitch3	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
piswitch3	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
piswitch3	:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
piswitch3	:		DEFINEFLAGS		+=	-D_ENABLE_WAVESHARE_3RELAY_
piswitch3	:									\
					$(DRIVER_OBJECTS)			\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\
					-lwiringPi					\
					-lpthread					\
					-o piswitch3


######################################################################################
#pragma mark Raspberry pi - switch
#make piswitch4 using 4 relay board
piswitch4	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_4RELAY_BOARD_
piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
piswitch4	:									\
					$(DRIVER_OBJECTS)			\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\
					-lwiringPi					\
					-lpthread					\
					-o piswitch4


######################################################################################
#pragma mark Raspberry pi - switch
#make piswitch4ks using Keyestudio 4 relay board
piswitch4ks	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
piswitch4ks	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
piswitch4ks	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
piswitch4ks	:		DEFINEFLAGS		+=	-D_ENABLE_4RELAY_KEYSTUDIO_
piswitch4ks	:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
piswitch4ks	:									\
					$(DRIVER_OBJECTS)			\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\
					-lwiringPi					\
					-lpthread					\
					-lcrypt						\
					-lrt						\
					-o piswitch4

######################################################################################
#pragma mark Raspberry pi - switch
#make piswitch8 using 8 relay board DIN board
piswitch8	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
piswitch8	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
piswitch8	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
piswitch8	:		DEFINEFLAGS		+=	-D_ENABLE_8RELAY_DIN_
piswitch8	:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
piswitch8	:									\
					$(DRIVER_OBJECTS)			\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(SOCKET_OBJECTS)			\
					$(HELPER_OBJECTS)			\
					-lwiringPi					\
					-lpthread					\
					-o piswitch8


######################################################################################
#pragma mark make piqhy
#	make qhy
piqhy		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS
piqhy		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
piqhy		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
piqhy		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
piqhy		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
piqhy		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_QHY_
piqhy		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
piqhy		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
piqhy		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
piqhy		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
piqhy		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
piqhy		:									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(SERIAL_OBJECTS)				\


		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(SERIAL_OBJECTS)				\
					-lcfitsio						\
					-lusb-1.0						\
					-ludev							\
					-lpthread						\
					-lqhyccd						\
					-o alpacapi


######################################################################################
#pragma mark make qsi
qsi		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
qsi		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
qsi		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
qsi		:		DEFINEFLAGS		+=	-D_ENABLE_QSI_
qsi		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
qsi		:		INCLUDES		+=	-I$(QSI_INCLUDE_DIR)
qsi		:										\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\

		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					-lcfitsio					\
					-lqsiapi					\
					-lftd2xx					\
					-lusb-1.0					\
					-ludev						\
					-lpthread					\
					-o alpacapi

######################################################################################
#pragma mark make qsicv4
qsicv4		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
qsicv4		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
qsicv4		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
qsicv4		:		DEFINEFLAGS		+=	-D_ENABLE_QSI_
qsicv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
qsicv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
qsicv4		:		INCLUDES		+=	-I$(QSI_INCLUDE_DIR)
qsicv4		:									\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\

		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					-lcfitsio					\
					-lqsiapi					\
					-lftd2xx					\
					-lusb-1.0					\
					-ludev						\
					-lpthread					\
					-o alpacapi

######################################################################################
#pragma mark make qsiimu
qsiimu		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
qsiimu		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
qsiimu		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
qsiimu		:		DEFINEFLAGS		+=	-D_ENABLE_QSI_
qsiimu		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
qsiimu		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
qsiimu		:		INCLUDES		+=	-I$(QSI_INCLUDE_DIR)
qsiimu		:		INCLUDES		+=	-I$(SRC_IMU)
qsiimu		:		DEFINEFLAGS		+=	-D_ENABLE_IMU_
qsiimu		:									\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(IMU_OBJECTS)				\

		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(IMU_OBJECTS)				\
					-lcfitsio					\
					-lqsiapi					\
					-lftd2xx					\
					-lusb-1.0					\
					-ludev						\
					-lpthread					\
					-o alpacapi


######################################################################################
#pragma mark Management only
#make manag
manag		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
manag		:									\
					$(DRIVER_OBJECTS)			\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(HELPER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					-lpthread					\
					-o alpacapi


######################################################################################
#pragma mark ZWO
zwo		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#zwo		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#zwo		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
zwo		:			$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(DRIVER_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-lcfitsio					\
					-lusb-1.0					\
					-ludev						\
					-ljpeg						\
					-lpthread					\
					-o alpacapi

#					-lwiringPi					\



######################################################################################
#pragma mark Switch - C++ Raspberry pi
#	make finder
finder		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_8RELAY_DIN_
finder		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
finder		:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
finder		:										\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\

			$(LINK)  								\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					$(ZWO_EFW_OBJECTS)				\
					-lcfitsio						\
					-lusb-1.0						\
					-ludev							\
					-lwiringPi						\
					-lpthread						\
					-o alpacapi



######################################################################################
#pragma mark Switch - C++ Raspberry pi
#	make findercv4
findercv4		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_ASI_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_8RELAY_DIN_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
findercv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
findercv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
findercv4		:	DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
findercv4		:								\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\

		$(LINK)  									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					$(ZWO_EFW_OBJECTS)				\
					-lcfitsio						\
					-lusb-1.0						\
					-ludev							\
					-lwiringPi						\
					-lpthread						\
					-o alpacapi

######################################################################################
#pragma mark Switch - C++ Raspberry pi
#	make attic
attic		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#attic		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#attic		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#attic		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#attic		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#attic		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
#attic		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#attic		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_NITECRAWLER_
attic		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
attic		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
attic		:		DEFINEFLAGS		+=	-D_ENABLE_8RELAY_DIN_
attic		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
attic		:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
attic		:										\
					$(DRIVER_OBJECTS)				\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\

			$(LINK)  								\
					$(DRIVER_OBJECTS)				\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(OPENCV_LINK)					\
					-lwiringPi						\
					-lpthread						\
					-o alpacapi


######################################################################################
#pragma mark make shutter
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_SHUTTER_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_STATUS_SWITCH_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_8RELAY_DIN_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
shutter		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
shutter		:									\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(SHUTTER_DRIVER_OBJECTS)	\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SERIAL_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(LIVE_WINDOW_OBJECTS)		\

		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(CAMERA_DRIVER_OBJECTS)	\
					$(SHUTTER_DRIVER_OBJECTS)	\
					$(SWITCH_DRIVER_OBJECTS)	\
					$(HELPER_OBJECTS)			\
					$(SERIAL_OBJECTS)			\
					$(SOCKET_OBJECTS)			\
					$(LIVE_WINDOW_OBJECTS)		\
					$(ASI_CAMERA_OBJECTS)		\
					$(OPENCV_LINK)				\
					-lcfitsio					\
					-lusb-1.0					\
					-ludev						\
					-lwiringPi					\
					-lpthread					\
					-o alpacapi


######################################################################################
#pragma mark make shuttercv4
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_SHUTTER_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_RPI_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_STATUS_SWITCH_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_8RELAY_DIN_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
shuttercv4		:		DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
shuttercv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
shuttercv4		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
shuttercv4		:									\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(SHUTTER_DRIVER_OBJECTS)		\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\

			$(LINK)  								\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(SHUTTER_DRIVER_OBJECTS)		\
					$(SWITCH_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(OPENCV_LINK)					\
					$(ASI_CAMERA_OBJECTS)			\
					-lcfitsio						\
					-lusb-1.0						\
					-ludev							\
					-lwiringPi						\
					-lpthread						\
					-o alpacapi


######################################################################################
#pragma mark linux-x86 - No opencv
#make noopencv
noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
noopencv		:									\
						$(DRIVER_OBJECTS)			\
						$(CPP_OBJECTS)				\
						$(SOCKET_OBJECTS)			\
						$(ASI_CAMERA_OBJECTS)		\
						$(ZWO_EFW_OBJECTS)			\

		$(LINK)  									\
						$(DRIVER_OBJECTS)			\
						$(CPP_OBJECTS)				\
						$(SOCKET_OBJECTS)			\
						$(ASI_CAMERA_OBJECTS)		\
						$(ZWO_EFW_OBJECTS)			\
						-lcfitsio					\
						-ludev						\
						-lpthread					\
						-lusb-1.0					\
						-ljpeg						\
						-o alpacapi


######################################################################################
#pragma mark Debug linux-x86
#Debug		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#Debug		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
#Debug		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#Debug		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#Debug		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#Debug		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
Debug			:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#Debug			:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
Debug			:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
Debug			:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
Debug			:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
Debug			:		DEFINEFLAGS		+=	-D_USE_OPENCV_
Debug			:								\
					$(CPP_OBJECTS)				\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					$(ASI_CAMERA_OBJECTS)		\
					-lusb-1.0					\
					-ludev						\
					-lpthread					\
					-latikcameras				\
					-lcfitsio					\
					-o bin/Debug/alpaca



######################################################################################
#	Camera Objects
JETSON_OBJECTS=												\
				$(OBJECT_DIR)startextrathread.o				\


######################################################################################
#pragma mark make jetson
jetson		:	DEFINEFLAGS		+=	-D_JETSON_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
jetson		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#jetson		:	DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
#jetson		:	DEFINEFLAGS		+=	-D_ENABLE_ASI_
#jetson		:	DEFINEFLAGS		+=	-D_ENABLE_TOUP_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_FLIR_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
jetson		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_STAR_SEARCH_
jetson		:	DEFINEFLAGS		+=	-D_PLATFORM_STRING_=\"Nvidia-jetson\"
#jetson		:	DEFINEFLAGS		+=	-D_ENABLE_IMU_
#jetson		:	INCLUDES		+=	-I$(SRC_IMU)
jetson		:											\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(IMAGEPROC_OBJECTS)			\
					$(JETSON_OBJECTS)				\
					$(IMU_OBJECTS)					\


				$(LINK)  							\
					$(DRIVER_OBJECTS)				\
					$(CAMERA_DRIVER_OBJECTS)		\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)			\
					$(IMAGEPROC_OBJECTS)			\
					$(JETSON_OBJECTS)				\
					$(IMU_OBJECTS)					\
					$(OPENCV_LINK)					\
					-lcfitsio						\
					-lpthread						\
					-lSpinnaker_C					\
					-li2c							\
					-o alpacapi



######################################################################################
#pragma mark wx
#make wx
#wx			:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#wx			:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
#wx			:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#wx			:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_MOONLITE_
wx			:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
wx			:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_RPI_
wx			:	DEFINEFLAGS		+=	-D_ENABLE_PI_HAT_SESNSOR_BOARD_
wx			:										\
					$(DRIVER_OBJECTS)				\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(OBSCOND_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\

				$(LINK)  							\
					$(DRIVER_OBJECTS)				\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(OBSCOND_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					-lRTIMULib						\
					-lusb-1.0						\
					-lpthread						\
					-lwiringPi						\
					-o alpacapi


######################################################################################
#pragma mark smate
smate		:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
smate		:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
smate		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
smate		:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
smate		:	DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
smate		:				$(DRIVER_OBJECTS)			\
							$(SOCKET_OBJECTS)			\

				$(LINK)  								\
							$(DRIVER_OBJECTS)			\
							$(SOCKET_OBJECTS)			\
							$(ASI_CAMERA_OBJECTS)		\
							$(ZWO_EFW_OBJECTS)			\
							$(OPENCV_LINK)				\
							-lusb-1.0					\
							-lpthread					\
							-ludev						\
							-o alpacapi



######################################################################################
# ATIK objects
ATIK_OBJECTS=												\
				$(OBJECT_DIR)alpacadriver.o					\
				$(OBJECT_DIR)alpacadriverConnect.o			\
				$(OBJECT_DIR)alpacadriverSetup.o			\
				$(OBJECT_DIR)alpacadriverThread.o			\
				$(OBJECT_DIR)alpacadriver_helper.o			\
				$(OBJECT_DIR)alpacadriverLogging.o			\
				$(OBJECT_DIR)alpaca_discovery.o				\
				$(OBJECT_DIR)cpu_stats.o					\
				$(OBJECT_DIR)discoverythread.o				\
				$(OBJECT_DIR)json_parse.o					\
				$(OBJECT_DIR)filterwheeldriver_ATIK.o		\
				$(OBJECT_DIR)cameradriver.o					\
				$(OBJECT_DIR)cameradriver_readthread.o		\
				$(OBJECT_DIR)cameradriverAnalysis.o			\
				$(OBJECT_DIR)cameradriver_fits.o			\
				$(OBJECT_DIR)cameradriver_save.o			\
				$(OBJECT_DIR)cameradriver_opencv.o			\
				$(OBJECT_DIR)cameradriver_jpeg.o			\
				$(OBJECT_DIR)cameradriver_livewindow.o		\
				$(OBJECT_DIR)cameradriver_overlay.o			\
				$(OBJECT_DIR)cameradriver_png.o				\
				$(OBJECT_DIR)cameradriver_ATIK.o			\
				$(OBJECT_DIR)filterwheeldriver.o			\
				$(OBJECT_DIR)moonphase.o					\
				$(OBJECT_DIR)MoonRise.o						\
				$(OBJECT_DIR)julianTime.o					\


######################################################################################
#pragma mark ATIK Linux
#	make atik
atik	:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/linux/x64/NoFlyCapture
atik	:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
atik	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
atik	:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ATIK_
atik	:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
atik	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
atik	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
atik	:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
atik	:										\
					$(DRIVER_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)		\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(DRIVER_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(LIVE_WINDOW_OBJECTS)		\
					$(SOCKET_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					-latikcameras				\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-o atik


######################################################################################
#pragama mark make client client
client	:	DEFINEFLAGS		+=	-D_CLIENT_MAIN_
client	:			$(CLIENT_OBJECTS)

				$(LINK)  										\
							$(CLIENT_OBJECTS)					\
							-o client

#client	:	DEFINEFLAGS		+=	-D_DEBUG_PARSER_

######################################################################################
MANDELBROT_OBJECTS=												\
				$(OBJECT_DIR)mandelbrot.o						\

######################################################################################
CONTROLLER_MAIN_OBJECTS=										\
				$(OBJECT_DIR)controller_main.o					\
				$(OBJECT_DIR)readconfigfile.o				\


######################################################################################
CONTROLLER_OBJECTS=												\
				$(OBJECT_DIR)alpacadriver_helper.o				\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)controller_focus.o					\
				$(OBJECT_DIR)controller_focus_generic.o			\
				$(OBJECT_DIR)controller_focus_ml_nc.o			\
				$(OBJECT_DIR)controller_focus_ml_hr.o			\
				$(OBJECT_DIR)controller_switch.o				\
				$(OBJECT_DIR)controller_camera.o				\
				$(OBJECT_DIR)controllerImageArray.o				\
				$(OBJECT_DIR)controller_cam_normal.o			\
				$(OBJECT_DIR)controller_dome.o					\
				$(OBJECT_DIR)controller_dome_common.o			\
				$(OBJECT_DIR)controller_filterwheel.o			\
				$(OBJECT_DIR)controller_image.o					\
				$(OBJECT_DIR)controller_obsconditions.o			\
				$(OBJECT_DIR)controller_rotator.o				\
				$(OBJECT_DIR)controller_usb.o					\
				$(OBJECT_DIR)cpu_stats.o						\
				$(OBJECT_DIR)HostNames.o						\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)linuxerrors.o						\
				$(OBJECT_DIR)moonlite_com.o						\
				$(OBJECT_DIR)nitecrawler_image.o				\
				$(OBJECT_DIR)opencv_utils.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\
				$(OBJECT_DIR)serialport.o						\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_auxmotor.o				\
				$(OBJECT_DIR)windowtab_camera.o					\
				$(OBJECT_DIR)windowtab_camsettings.o			\
				$(OBJECT_DIR)windowtab_camcooler.o				\
				$(OBJECT_DIR)windowtab_camvideo.o				\
				$(OBJECT_DIR)windowtab_capabilities.o			\
				$(OBJECT_DIR)windowtab_config.o					\
				$(OBJECT_DIR)windowtab_DeviceState.o			\
				$(OBJECT_DIR)windowtab_dome.o					\
				$(OBJECT_DIR)windowtab_drvrInfo.o				\
				$(OBJECT_DIR)windowtab_filelist.o				\
				$(OBJECT_DIR)windowtab_fitsheader.o				\
				$(OBJECT_DIR)windowtab_graphs.o					\
				$(OBJECT_DIR)windowtab_image.o					\
				$(OBJECT_DIR)windowtab_imageinfo.o				\
				$(OBJECT_DIR)windowtab_ml_single.o				\
				$(OBJECT_DIR)windowtab_nitecrawler.o			\
				$(OBJECT_DIR)windowtab_switch.o					\
				$(OBJECT_DIR)windowtab_slit.o					\
				$(OBJECT_DIR)windowtab_slitdome.o				\
				$(OBJECT_DIR)windowtab_slitgraph.o				\
				$(OBJECT_DIR)windowtab_usb.o					\

VIDEO_OBJECTS=													\
				$(OBJECT_DIR)alpacadriver_helper.o				\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)controller_camera.o				\
				$(OBJECT_DIR)controller_video.o					\
				$(OBJECT_DIR)controller_preview.o				\
				$(OBJECT_DIR)controllerImageArray.o				\
				$(OBJECT_DIR)cpu_stats.o						\
				$(OBJECT_DIR)discovery_lib.o					\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)linuxerrors.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_video.o					\
				$(OBJECT_DIR)windowtab_preview.o				\


PREVIEW_OBJECTS=												\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)controller_camera.o				\
				$(OBJECT_DIR)controller_main.o					\
				$(OBJECT_DIR)controller_preview.o				\
				$(OBJECT_DIR)cpu_stats.o						\
				$(OBJECT_DIR)discovery_lib.o					\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_preview.o				\

#				$(OBJECT_DIR)controller_cam_normal.o			\

######################################################################################
FITS_OBJECTS=													\
				$(OBJECT_DIR)fits_opencv.o						\

######################################################################################
#pragma mark mandelbrot
mandelbrot	:	DEFINEFLAGS		+=	-D_INCLUDE_MAIN_
mandelbrot	:			$(MANDELBROT_OBJECTS)

				$(LINK)  										\
							$(MANDELBROT_OBJECTS)				\
							$(OPENCV_LINK)						\
							-o mandlebrot;



######################################################################################
#make camera
#pragma mark camera-controller
camera		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
camera		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
camera		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
camera		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
camera		:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
camera		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
camera		:										\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(FITS_OBJECTS)					\
					$(HELPER_OBJECTS)				\

				$(LINK)  							\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(FITS_OBJECTS)					\
					$(HELPER_OBJECTS)				\
					$(OPENCV_LINK)					\
					-lcfitsio						\
					-lpthread						\
					-o camera


######################################################################################
#make cameracv4
#pragma mark camera-controller
cameracv4		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
cameracv4		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
cameracv4		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
cameracv4		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
cameracv4		:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
cameracv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
cameracv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
cameracv4		:									\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(FITS_OBJECTS)					\

				$(LINK)  							\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(FITS_OBJECTS)					\
					$(OPENCV_LINK)					\
					-lcfitsio						\
					-lpthread						\
					-o camera

######################################################################################
#pragma mark dome-controller
#	make domectrl
domectrl		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
domectrl		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
domectrl		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_
domectrl		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
domectrl		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
domectrl		:									\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\

				$(LINK)  							\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\
							$(OPENCV_LINK)			\
							-lpthread				\
							-o domectrl

######################################################################################
#pragma mark dome-controller
#	make domectrlcv4
domectrlcv4		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
domectrlcv4		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
domectrlcv4		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_
domectrlcv4		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
domectrlcv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
domectrlcv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
domectrlcv4		:									\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\

				$(LINK)  							\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\
							$(OPENCV_LINK)			\
							-lpthread				\
							-o domectrl

######################################################################################
#pragma mark focuser-controller
#	make focuser
focuser		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
focuser		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
focuser		:	DEFINEFLAGS		+=	-D_ENABLE_USB_FOCUSERS_
focuser		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
focuser		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
focuser		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
focuser		:										\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\

				$(LINK)  							\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(OPENCV_LINK)					\
					-lpthread						\
					-o focuser

######################################################################################
#pragma mark focuser-controller
#	make focuser
focusercv4		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
focusercv4		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
focusercv4		:	DEFINEFLAGS		+=	-D_ENABLE_USB_FOCUSERS_
focusercv4		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
focusercv4		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
focusercv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
focusercv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
focusercv4		:	OPENCV_COMPILE	=	$(shell pkg-config --cflags opencv4)
focusercv4		:	OPENCV_LINK		=	$(shell pkg-config --libs opencv4)
focusercv4		:									\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\

				$(LINK)  							\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(OPENCV_LINK)					\
					-lpthread						\
					-o focuser


######################################################################################
#make switch
#pragma mark switch-controller
switch		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
switch		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
switch		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
switch		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
switch		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
switch		:										\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\

				$(LINK)  							\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\
							$(OPENCV_LINK)			\
							-lpthread				\
							-o switch

######################################################################################
#make switch
#pragma mark switch-controller
switchcv4		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
switchcv4		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
switchcv4		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
switchcv4		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
switchcv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
switchcv4		:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
switchcv4		:	OPENCV_COMPILE	=	$(shell pkg-config --cflags opencv4)
switchcv4		:	OPENCV_LINK		=	$(shell pkg-config --libs opencv4)
switchcv4		:									\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\

				$(LINK)  							\
					$(CONTROLLER_MAIN_OBJECTS)		\
					$(CONTROLLER_OBJECTS)			\
					$(DISCOVERY_LIB_OBJECTS)		\
					$(HELPER_OBJECTS)				\
							$(OPENCV_LINK)			\
							-lpthread				\
							-o switch


######################################################################################
# SkyTravel objects

SRC_SKYTRAVEL=./src_skytravel/
SKYTRAVEL_OBJECTS=											\
				$(OBJECT_DIR)aavso_data.o					\
				$(OBJECT_DIR)AsteroidData.o					\
				$(OBJECT_DIR)ConstellationData.o			\
				$(OBJECT_DIR)controller_alpacaUnit.o		\
				$(OBJECT_DIR)controller_constList.o			\
				$(OBJECT_DIR)controller_covercalib.o		\
				$(OBJECT_DIR)controller_multicam.o			\
				$(OBJECT_DIR)controller_remoteview.o		\
				$(OBJECT_DIR)controller_skytravel.o			\
				$(OBJECT_DIR)controller_slit.o				\
				$(OBJECT_DIR)controller_starlist.o			\
				$(OBJECT_DIR)controller_startup.o			\
				$(OBJECT_DIR)controller_telescope.o			\
				$(OBJECT_DIR)discoverythread.o				\
				$(OBJECT_DIR)eph.o							\
				$(OBJECT_DIR)fits_opencv.o					\
				$(OBJECT_DIR)HipparcosCatalog.o				\
				$(OBJECT_DIR)julianTime.o					\
				$(OBJECT_DIR)moonphase.o					\
				$(OBJECT_DIR)NGCcatalog.o					\
				$(OBJECT_DIR)observatory_settings.o			\
				$(OBJECT_DIR)OpenNGC.o						\
				$(OBJECT_DIR)polaralign.o					\
				$(OBJECT_DIR)readconfigfile.o				\
				$(OBJECT_DIR)RemoteImage.o					\
				$(OBJECT_DIR)sidereal.o						\
				$(OBJECT_DIR)StarCatalogHelper.o			\
				$(OBJECT_DIR)skytravel_main.o				\
				$(OBJECT_DIR)SAO_stardata.o					\
				$(OBJECT_DIR)StarData.o						\
				$(OBJECT_DIR)MessierData.o					\
				$(OBJECT_DIR)SkyTravelTimeRoutines.o		\
				$(OBJECT_DIR)windowtab_alpacalist.o			\
				$(OBJECT_DIR)windowtab_alpacaUnit.o			\
				$(OBJECT_DIR)windowtab_constList.o			\
				$(OBJECT_DIR)windowtab_cpustats.o			\
				$(OBJECT_DIR)windowtab_covercalib.o			\
				$(OBJECT_DIR)windowtab_deviceselect.o		\
				$(OBJECT_DIR)windowtab_filterwheel.o		\
				$(OBJECT_DIR)windowtab_fitsheader.o			\
				$(OBJECT_DIR)windowtab_fov.o				\
				$(OBJECT_DIR)windowtab_iplist.o				\
				$(OBJECT_DIR)windowtab_libraries.o			\
				$(OBJECT_DIR)windowtab_moon.o				\
				$(OBJECT_DIR)windowtab_mount.o				\
				$(OBJECT_DIR)windowtab_obscond.o			\
				$(OBJECT_DIR)windowtab_multicam.o			\
				$(OBJECT_DIR)windowtab_RemoteData.o			\
				$(OBJECT_DIR)windowtab_rotator.o			\
				$(OBJECT_DIR)windowtab_skytravel.o			\
				$(OBJECT_DIR)windowtab_startup.o			\
				$(OBJECT_DIR)windowtab_starlist.o			\
				$(OBJECT_DIR)windowtab_STsettings.o			\
				$(OBJECT_DIR)windowtab_sw_versions.o		\
				$(OBJECT_DIR)windowtab_telescope.o			\
				$(OBJECT_DIR)windowtab_teleSettings.o		\
				$(OBJECT_DIR)windowtab_time.o				\
				$(OBJECT_DIR)YaleStarCatalog.o				\

#				$(OBJECT_DIR)controller_image.o				\
#				$(OBJECT_DIR)controller_obsconditions.o		\
#				$(OBJECT_DIR)nitecrawler_image.o			\
#				$(OBJECT_DIR)windowtab_switch.o				\
#				$(OBJECT_DIR)moonlite_com.o					\


MISC_UNUSED=												\
				$(OBJECT_DIR)controller_camera.o			\
				$(OBJECT_DIR)controllerImageArray.o			\
				$(OBJECT_DIR)controller_cam_normal.o		\
				$(OBJECT_DIR)controller_dome.o				\
				$(OBJECT_DIR)controller_dome_common.o		\
				$(OBJECT_DIR)controller_filterwheel.o		\
				$(OBJECT_DIR)controller_focus.o				\
				$(OBJECT_DIR)controller_focus_generic.o		\
				$(OBJECT_DIR)controller_focus_ml_hr.o		\
				$(OBJECT_DIR)controller_focus_ml_nc.o		\
				$(OBJECT_DIR)controller_rotator.o			\
				$(OBJECT_DIR)controller_switch.o			\
				$(OBJECT_DIR)windowtab_auxmotor.o			\
				$(OBJECT_DIR)windowtab_camera.o				\
				$(OBJECT_DIR)windowtab_camsettings.o		\
				$(OBJECT_DIR)windowtab_capabilities.o		\
				$(OBJECT_DIR)windowtab_config.o				\
				$(OBJECT_DIR)windowtab_dome.o				\
				$(OBJECT_DIR)windowtab_drvrInfo.o			\
				$(OBJECT_DIR)windowtab_filelist.o			\
				$(OBJECT_DIR)windowtab_graphs.o				\
				$(OBJECT_DIR)windowtab_ml_single.o			\
				$(OBJECT_DIR)windowtab_nitecrawler.o		\
				$(OBJECT_DIR)windowtab_slit.o				\
				$(OBJECT_DIR)windowtab_slitdome.o			\
				$(OBJECT_DIR)windowtab_slitgraph.o			\


#	5/27/2022	Gaia data no longer used, using GaiaSQL instead
#				$(OBJECT_DIR)GaiaData.o						\

######################################################################################
SPECTROGRAPH_DRIVER_OBJECTS=								\
				$(OBJECT_DIR)spectrodriver.o				\
				$(OBJECT_DIR)spectrodrvr_usis.o				\
				$(OBJECT_DIR)filterwheeldriver_usis.o		\
				$(OBJECT_DIR)focuserdriver_USIS.o			\
				$(OBJECT_DIR)usis_communications.o			\

######################################################################################
SPECTROGRAPH_OBJECTS=										\
				$(OBJECT_DIR)controller_spectrograph.o		\
				$(OBJECT_DIR)windowtab_spectro.o			\


SRC_SPECTROGRAPH=./src_spectrograph/


######################################################################################
GAIA_SQL_OBJECTS=											\
				$(OBJECT_DIR)GaiaSQL.o						\
				$(OBJECT_DIR)controller_GaiaRemote.o		\
				$(OBJECT_DIR)windowtab_GaiaRemote.o			\


######################################################################################
#make skytravel
#pragma mark skytravel
sky		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
sky		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_SKYTRAVEL_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_OBS_CONDITIONS_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_ROTATOR_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_TELESCOPE_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
sky		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_CONTROLLER_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
sky		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#sky		:	DEFINEFLAGS		+=	-D_ENABLE_ASTEROIDS_
sky		:	INCLUDES		+=	-I$(SRC_SKYTRAVEL)
sky		:												\
						$(CONTROLLER_OBJECTS)			\
						$(SKYTRAVEL_OBJECTS)			\
						$(HELPER_OBJECTS)				\


				$(LINK)  								\
						$(CONTROLLER_OBJECTS)			\
						$(SKYTRAVEL_OBJECTS)			\
						$(HELPER_OBJECTS)				\
						$(OPENCV_LINK)					\
						-lpthread						\
						-lcfitsio						\
						-o skytravel

######################################################################################
#make skycv4
#pragma mark skytravel
skycv4			:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_SKYTRAVEL_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_OBS_CONDITIONS_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_ROTATOR_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_TELESCOPE_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
skycv4			:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_CONTROLLER_
skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
skycv4			:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#skycv4			:	DEFINEFLAGS		+=	-D_ENABLE_ASTEROIDS_
skycv4			:	DEFINEFLAGS		+=	-D_USE_OPENCV_
skycv4			:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
skycv4			:	INCLUDES		+=	-I$(SRC_SKYTRAVEL)
skycv4			:										\
						$(CONTROLLER_OBJECTS)			\
						$(SKYTRAVEL_OBJECTS)			\
						$(HELPER_OBJECTS)				\

				$(LINK)  								\
						$(CONTROLLER_OBJECTS)			\
						$(SKYTRAVEL_OBJECTS)			\
						$(HELPER_OBJECTS)				\
						$(OPENCV_LINK)					\
						-lpthread						\
						-lcfitsio						\
						-o skytravel

######################################################################################
#make skycv4sql
#pragma mark skytravel
skycv4sql			:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_SKYTRAVEL_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_OBS_CONDITIONS_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_ROTATOR_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_TELESCOPE_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
skycv4sql			:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_CONTROLLER_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
skycv4sql			:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_ASTEROIDS_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_REMOTE_SQL_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_REMOTE_GAIA_
skycv4sql			:	DEFINEFLAGS		+=	-D_USE_OPENCV_
skycv4sql			:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
skycv4sql			:	DEFINEFLAGS		+=	-D_ENABLE_CPU_STATS_
skycv4sql			:	DEFINEFLAGS		+=	-D_SQL_$(SQL_VERSION)
skycv4sql			:	INCLUDES		+=	-I$(SRC_SKYTRAVEL)
skycv4sql			:										\
						$(CONTROLLER_OBJECTS)				\
						$(GAIA_SQL_OBJECTS)					\
						$(SKYTRAVEL_OBJECTS)				\

				$(LINK)  									\
						$(CONTROLLER_OBJECTS)				\
						$(GAIA_SQL_OBJECTS)					\
						$(SKYTRAVEL_OBJECTS)				\
							-L/usr/local/lib				\
							$(OPENCV_LINK)					\
							-l$(SQL_VERSION)				\
							-lpthread						\
							-lcfitsio						\
							-o skytravel




######################################################################################
#make skysql
#pragma mark camera-controller
skysql		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
skysql		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_SKYTRAVEL_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_OBS_CONDITIONS_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_ROTATOR_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_TELESCOPE_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
skysql		:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_CONTROLLER_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
skysql		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_ASTEROIDS_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_REMOTE_SQL_
skysql		:	DEFINEFLAGS		+=	-D_ENABLE_REMOTE_GAIA_
skysql		:	INCLUDES		+=	-I$(SRC_SKYTRAVEL)
skysql		:												\
						$(CONTROLLER_OBJECTS)				\
						$(GAIA_SQL_OBJECTS)					\
						$(SKYTRAVEL_OBJECTS)				\


				$(LINK)  									\
						$(CONTROLLER_OBJECTS)				\
						$(GAIA_SQL_OBJECTS)					\
						$(SKYTRAVEL_OBJECTS)				\
						$(OPENCV_LINK)						\
						-lpthread							\
						-l$(SQL_VERSION)					\
						-lcfitsio							\
						-o skytravel




######################################################################################
#make sss
#pragma mark skytravel
sss			:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_SKYTRAVEL_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_OBS_CONDITIONS_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_ROTATOR_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SPECTROGRAPH_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_TELESCOPE_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
sss			:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_CONTROLLER_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
sss			:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_REMOTE_SQL_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_REMOTE_GAIA_
sss			:	DEFINEFLAGS		+=	-D_USE_OPENCV_
sss			:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_CPU_STATS_
sss			:	DEFINEFLAGS		+=	-D_ENABLE_IMU_
#sss			:	DEFINEFLAGS		+=	-D_ENABLE_ASTEROIDS_
sss			:	DEFINEFLAGS		+=	-D_SQL_$(SQL_VERSION)
sss			:	INCLUDES		+=	-I$(SRC_SKYTRAVEL)
sss			:	INCLUDES		+=	-I$(SRC_SPECTROGRAPH)
sss			:										\
					$(CONTROLLER_OBJECTS)			\
					$(SKYTRAVEL_OBJECTS)			\
					$(GAIA_SQL_OBJECTS)				\
					$(SPECTROGRAPH_OBJECTS)			\
					$(HELPER_OBJECTS)				\

			$(LINK)  								\
					$(CONTROLLER_OBJECTS)			\
					$(SKYTRAVEL_OBJECTS)			\
					$(GAIA_SQL_OBJECTS)				\
					$(SPECTROGRAPH_OBJECTS)			\
					$(HELPER_OBJECTS)				\
					-L/usr/local/lib				\
					$(OPENCV_LINK)					\
					-l$(SQL_VERSION)				\
					-lpthread						\
					-lcfitsio						\
					-o skytravel


######################################################################################
#	make spectro
spectro		:	DEFINEFLAGS		+=	-D_ENABLE_SPECTROGRAPH_
spectro		:	DEFINEFLAGS		+=	-D_ENABLE_SPECTROGRAPH_USIS_
spectro		:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
spectro		:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_USIS_
spectro		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
spectro		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_USIS_
spectro		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
spectro		:	INCLUDES		+=	-I$(SRC_SPECTROGRAPH)
spectro		:										\
					$(DRIVER_OBJECTS)				\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(SPECTROGRAPH_DRIVER_OBJECTS)	\

				$(LINK)  							\
					$(DRIVER_OBJECTS)				\
					$(FITLERWHEEL_DRIVER_OBJECTS)	\
					$(FOCUSER_DRIVER_OBJECTS)		\
					$(HELPER_OBJECTS)				\
					$(SERIAL_OBJECTS)				\
					$(SOCKET_OBJECTS)				\
					$(SPECTROGRAPH_DRIVER_OBJECTS)	\
						-lpthread					\
						-o spectro

#spectro		:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#spectro		:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_SIMULATOR_


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)spectrodriver.o :		$(SRC_SPECTROGRAPH)spectrodriver.cpp	\
									$(SRC_SPECTROGRAPH)spectrodriver.h		\
									Makefile
	$(COMPILEPLUS) $(INCLUDES)		$(SRC_SPECTROGRAPH)spectrodriver.cpp -o$(OBJECT_DIR)spectrodriver.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)spectrodrvr_usis.o :	$(SRC_SPECTROGRAPH)spectrodrvr_usis.cpp	\
									$(SRC_SPECTROGRAPH)spectrodrvr_usis.h	\
									$(SRC_SPECTROGRAPH)spectrodriver.h		\
									Makefile
	$(COMPILEPLUS) $(INCLUDES)		$(SRC_SPECTROGRAPH)spectrodrvr_usis.cpp -o$(OBJECT_DIR)spectrodrvr_usis.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_spectrograph.o :	$(SRC_SPECTROGRAPH)controller_spectrograph.cpp		\
											$(SRC_SPECTROGRAPH)controller_spectrograph.h		\
											Makefile
	$(COMPILEPLUS) $(INCLUDES)				$(SRC_SPECTROGRAPH)controller_spectrograph.cpp -o$(OBJECT_DIR)controller_spectrograph.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)filterwheeldriver_usis.o :		$(SRC_SPECTROGRAPH)filterwheeldriver_usis.cpp		\
											$(SRC_SPECTROGRAPH)filterwheeldriver_usis.h			\
											Makefile
	$(COMPILEPLUS) $(INCLUDES)				$(SRC_SPECTROGRAPH)filterwheeldriver_usis.cpp -o$(OBJECT_DIR)filterwheeldriver_usis.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)focuserdriver_USIS.o :			$(SRC_SPECTROGRAPH)focuserdriver_USIS.cpp		\
											$(SRC_SPECTROGRAPH)focuserdriver_USIS.h			\
											Makefile
	$(COMPILEPLUS) $(INCLUDES)				$(SRC_SPECTROGRAPH)focuserdriver_USIS.cpp -o$(OBJECT_DIR)focuserdriver_USIS.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_spectro.o :			$(SRC_SPECTROGRAPH)windowtab_spectro.cpp	\
											$(SRC_SPECTROGRAPH)windowtab_spectro.h		\
											Makefile
	$(COMPILEPLUS) $(INCLUDES)				$(SRC_SPECTROGRAPH)windowtab_spectro.cpp -o$(OBJECT_DIR)windowtab_spectro.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)usis_communications.o :		$(SRC_SPECTROGRAPH)usis_communications.cpp	\
											$(SRC_SPECTROGRAPH)usis_communications.h	\
											Makefile
	$(COMPILEPLUS) $(INCLUDES)				$(SRC_SPECTROGRAPH)usis_communications.cpp -o$(OBJECT_DIR)usis_communications.o

######################################################################################
NETTEST_OBJECTS=												\
				$(OBJECT_DIR)cpu_stats.o						\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)controller_main.o					\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controller_nettest.o				\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)discovery_lib.o					\
				$(OBJECT_DIR)helper_functions.o					\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)linuxerrors.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_nettest.o				\


######################################################################################
FITSVIEW_OBJECTS=												\
				$(OBJECT_DIR)fitsview.o							\
				$(OBJECT_DIR)fits_opencv.o						\


######################################################################################
PDS_OBJECTS=													\
				$(OBJECT_DIR)PDS_ReadNASAfiles.o				\
				$(OBJECT_DIR)PDS_decompress.o					\

DUMPFITS_OBJECTS=												\
				$(OBJECT_DIR)dumpfits.o							\

######################################################################################
#pragma mark make fitsview
fitsview	:		$(FITSVIEW_OBJECTS)

				$(LINK)  										\
							$(FITSVIEW_OBJECTS)					\
							$(OPENCV_LINK)						\
							-lcfitsio							\
							-lm									\
							-o fitsview


######################################################################################
#pragma mark make fitsviewcv4
fitsviewcv4	:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
fitsviewcv4	:		$(FITSVIEW_OBJECTS)

				$(LINK)  										\
							$(FITSVIEW_OBJECTS)					\
							$(OPENCV_LINK)						\
							-lcfitsio							\
							-lm									\
							-o fitsview

######################################################################################
#pragma mark make fitsviewcv4
fv4	:			DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
fv4	:			DEFINEFLAGS		+=	-D_ENABLE_NASA_PDS_
fv4	:			$(FITSVIEW_OBJECTS)								\
				$(PDS_OBJECTS)									\

				$(LINK)  										\
							$(FITSVIEW_OBJECTS)					\
							$(OPENCV_LINK)						\
							$(PDS_OBJECTS)						\
							-lcfitsio							\
							-lm									\
							-o fitsview

######################################################################################
#pragma mark make dumpfits
dumpfits	:		$(DUMPFITS_OBJECTS)


				$(LINK)  										\
							$(DUMPFITS_OBJECTS)					\
							-lcfitsio							\
							-o dumpfits


######################################################################################
#pragma mark make net
SRC_NETTEST=./src_nettest/

net		:		DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
net		:		DEFINEFLAGS		+=	-D_ENABLE_NET_TEST_
net		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
net		:		INCLUDES		+=	-I$(SRC_NETTEST)
net		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
net		:		DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_

net		:			$(NETTEST_OBJECTS)

				$(LINK)  										\
							$(NETTEST_OBJECTS)					\
							$(OPENCV_LINK)						\
							-lpthread							\
							-o nettest


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_nettest.o :		$(SRC_NETTEST)controller_nettest.cpp		\
										$(SRC_NETTEST)controller_nettest.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_NETTEST)controller_nettest.cpp -o$(OBJECT_DIR)controller_nettest.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_nettest.o :		$(SRC_NETTEST)windowtab_nettest.cpp		\
										$(SRC_NETTEST)windowtab_nettest.h		\
										$(SRC_DIR)windowtab.h					\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_NETTEST)windowtab_nettest.cpp -o$(OBJECT_DIR)windowtab_nettest.o


######################################################################################
#pragma mark video-controller
#	make video
video			:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
video			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_VIDEO_
video			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
video			:	DEFINEFLAGS		+=	-D_VIDEO_CONTROLLER_WORK_
video			:	DEFINEFLAGS		+=	-D_USE_OPENCV_
video			:	DEFINEFLAGS		+=	-D_USE_OPENCV_CPP_
video			:	DEFINEFLAGS		+=	-D_CONTROLLER_USES_ALPACA_
video			:			$(VIDEO_OBJECTS)				\
							$(HELPER_OBJECTS)				\

				$(LINK)  									\
							$(VIDEO_OBJECTS)				\
							$(HELPER_OBJECTS)				\
							$(OPENCV_LINK)					\
							-o video


######################################################################################
#pragma mark preview-controller
#make preview
preview			:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
preview			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_PREVIEW_
preview			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_

preview			:			$(PREVIEW_OBJECTS)

				$(LINK)  										\
							$(PREVIEW_OBJECTS)					\
							$(OPENCV_LINK)						\
							-o preview
######################################################################################
SKYIMAGE_OBJECTS=											\
				$(OBJECT_DIR)alpacadriver_helper.o			\
				$(OBJECT_DIR)controller.o					\
				$(OBJECT_DIR)controller_image.o				\
				$(OBJECT_DIR)controller_skyimage.o			\
				$(OBJECT_DIR)cpu_stats.o					\
				$(OBJECT_DIR)readconfigfile.o				\
				$(OBJECT_DIR)commoncolor.o					\
				$(OBJECT_DIR)fits_opencv.o					\
				$(OBJECT_DIR)opencv_utils.o					\
				$(OBJECT_DIR)windowtab.o					\
				$(OBJECT_DIR)windowtab_about.o				\
				$(OBJECT_DIR)windowtab_image.o				\
				$(OBJECT_DIR)windowtab_imageinfo.o			\
				$(OBJECT_DIR)windowtab_imageList.o			\
				$(OBJECT_DIR)windowtab_fitsheader.o			\

######################################################################################
# make si skyimage
si			:	DEFINEFLAGS		+=	-D_ENABLE_SKYIMAGE_
si			:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
si			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
si	:			DEFINEFLAGS		+=	-D_ENABLE_NASA_PDS_

si			:			$(SKYIMAGE_OBJECTS)					\
						$(PDS_OBJECTS)						\
						$(HELPER_OBJECTS)

				$(LINK)  									\
						$(SKYIMAGE_OBJECTS)					\
						$(PDS_OBJECTS)						\
						$(HELPER_OBJECTS)					\
						$(OPENCV_LINK)						\
						-L/usr/local/lib					\
						-lpthread							\
						-lcfitsio							\
						-o skyimage


######################################################################################
#pragma mark clean
#	make clean
clean:
	rm -vf $(OBJECT_DIR)*.o

######################################################################################
cleanskytravel:
	rm -vf $(OBJECT_DIR)*.o
	rm -vf obj/skytravel/src/*.o

cleansss:
	rm -vf $(OBJECT_DIR)*.o
	rm -vf obj/sss/src/*.o

######################################################################################
$(OBJECT_DIR)socket_listen.o : $(SRC_DIR)socket_listen.c $(SRC_DIR)socket_listen.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)socket_listen.c -o$(OBJECT_DIR)socket_listen.o

$(OBJECT_DIR)JsonResponse.o : $(SRC_DIR)JsonResponse.c $(SRC_DIR)JsonResponse.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)JsonResponse.c -o$(OBJECT_DIR)JsonResponse.o


$(OBJECT_DIR)eventlogging.o : $(SRC_DIR)eventlogging.c $(SRC_DIR)eventlogging.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)eventlogging.c -o$(OBJECT_DIR)eventlogging.o

######################################################################################
$(OBJECT_DIR)readconfigfile.o : $(SRC_DIR)readconfigfile.c $(SRC_DIR)readconfigfile.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)readconfigfile.c -o$(OBJECT_DIR)readconfigfile.o


######################################################################################
# CPP objects
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpacadriver.o :			$(SRC_DIR)alpacadriver.cpp				\
										$(SRC_DIR)alpacadriver.h				\
										$(SRC_DIR)alpaca_defs.h					\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpacadriver.cpp -o$(OBJECT_DIR)alpacadriver.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpacadriverConnect.o :	$(SRC_DIR)alpacadriverConnect.cpp		\
										$(SRC_DIR)alpaca_defs.h					\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpacadriverConnect.cpp -o$(OBJECT_DIR)alpacadriverConnect.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpacadriverThread.o :		$(SRC_DIR)alpacadriverThread.cpp		\
										$(SRC_DIR)alpacadriver.h				\
										$(SRC_DIR)alpaca_defs.h					\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpacadriverThread.cpp -o$(OBJECT_DIR)alpacadriverThread.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpacadriverSetup.o :		$(SRC_DIR)alpacadriverSetup.cpp			\
										$(SRC_DIR)alpacadriver.h				\
										$(SRC_DIR)alpaca_defs.h					\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpacadriverSetup.cpp -o$(OBJECT_DIR)alpacadriverSetup.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpacadriver_helper.o :	$(SRC_DIR)alpacadriver_helper.c			\
										$(SRC_DIR)alpacadriver_helper.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpacadriver_helper.c -o$(OBJECT_DIR)alpacadriver_helper.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpaca_discovery.o :		$(SRC_DIR)alpaca_discovery.cpp		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpaca_discovery.cpp -o$(OBJECT_DIR)alpaca_discovery.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpacadriver_templog.o :	$(SRC_DIR)alpacadriver_templog.cpp		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpacadriver_templog.cpp -o$(OBJECT_DIR)alpacadriver_templog.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpacadriverLogging.o :	$(SRC_DIR)alpacadriverLogging.cpp	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpacadriverLogging.cpp -o$(OBJECT_DIR)alpacadriverLogging.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver.o :			$(SRC_DIR)cameradriver.cpp			\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver.cpp -o$(OBJECT_DIR)cameradriver.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_readthread.o :$(SRC_DIR)cameradriver_readthread.cpp	\
										$(SRC_DIR)cameradriver.h				\
										$(SRC_DIR)alpacadriver.h				\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_readthread.cpp -o$(OBJECT_DIR)cameradriver_readthread.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriverAnalysis.o :	$(SRC_DIR)cameradriverAnalysis.cpp	\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriverAnalysis.cpp -o$(OBJECT_DIR)cameradriverAnalysis.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_fits.o :		$(SRC_DIR)cameradriver_fits.cpp		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_fits.cpp -I$(SRC_MOONRISE) -o$(OBJECT_DIR)cameradriver_fits.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_gps.o :		$(SRC_DIR)cameradriver_gps.cpp		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_gps.cpp -o$(OBJECT_DIR)cameradriver_gps.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_ASI.o :		$(SRC_DIR)cameradriver_ASI.cpp		\
										$(SRC_DIR)cameradriver_ASI.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_ASI.cpp -o$(OBJECT_DIR)cameradriver_ASI.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_ATIK.o :		$(SRC_DIR)cameradriver_ATIK.cpp		\
										$(SRC_DIR)cameradriver_ATIK.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_ATIK.cpp -o$(OBJECT_DIR)cameradriver_ATIK.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_overlay.o :	$(SRC_DIR)cameradriver_overlay.cpp	\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_overlay.cpp -o$(OBJECT_DIR)cameradriver_overlay.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)filterwheeldriver_ATIK.o :	$(SRC_DIR)filterwheeldriver_ATIK.cpp	\
										$(SRC_DIR)filterwheeldriver_ATIK.h		\
										$(SRC_DIR)filterwheeldriver.h			\
										$(SRC_DIR)alpacadriver.h				\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)filterwheeldriver_ATIK.cpp -o$(OBJECT_DIR)filterwheeldriver_ATIK.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)filterwheeldriver_QHY.o :	$(SRC_DIR)filterwheeldriver_QHY.cpp		\
										$(SRC_DIR)filterwheeldriver_QHY.h		\
										$(SRC_DIR)filterwheeldriver.h			\
										$(SRC_DIR)alpacadriver.h				\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)filterwheeldriver_QHY.cpp -o$(OBJECT_DIR)filterwheeldriver_QHY.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)filterwheeldriver_sim.o :	$(SRC_DIR)filterwheeldriver_sim.cpp		\
										$(SRC_DIR)filterwheeldriver_sim.h		\
										$(SRC_DIR)filterwheeldriver.h			\
										$(SRC_DIR)alpacadriver.h				\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)filterwheeldriver_sim.cpp -o$(OBJECT_DIR)filterwheeldriver_sim.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_QHY.o :		$(SRC_DIR)cameradriver_QHY.cpp		\
										$(SRC_DIR)cameradriver_QHY.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_QHY.cpp -o$(OBJECT_DIR)cameradriver_QHY.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)ParseNMEA.o :				$(MLS_LIB_DIR)ParseNMEA.c 	\
										$(MLS_LIB_DIR)ParseNMEA.h
	$(COMPILEPLUS) $(INCLUDES)			$(MLS_LIB_DIR)ParseNMEA.c -o$(OBJECT_DIR)ParseNMEA.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)NMEA_helper.o :			$(MLS_LIB_DIR)NMEA_helper.c 	\
										$(MLS_LIB_DIR)NMEA_helper.h
	$(COMPILEPLUS) $(INCLUDES)			$(MLS_LIB_DIR)NMEA_helper.c -o$(OBJECT_DIR)NMEA_helper.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_QSI.o :		$(SRC_DIR)cameradriver_QSI.cpp		\
										$(SRC_DIR)cameradriver_QSI.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_QSI.cpp -o$(OBJECT_DIR)cameradriver_QSI.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_FLIR.o :		$(SRC_DIR)cameradriver_FLIR.cpp		\
										$(SRC_DIR)cameradriver_FLIR.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_FLIR.cpp -o$(OBJECT_DIR)cameradriver_FLIR.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_livewindow.o :$(SRC_DIR)cameradriver_livewindow.cpp	\
									 	$(SRC_DIR)cameradriver.h				\
										$(SRC_DIR)alpacadriver.h				\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_livewindow.cpp -o$(OBJECT_DIR)cameradriver_livewindow.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_PhaseOne.o 	:$(SRC_DIR)cameradriver_PhaseOne.cpp	\
									 	$(SRC_DIR)cameradriver_PhaseOne.h		\
									 	$(SRC_DIR)cameradriver.h				\
										$(SRC_DIR)alpacadriver.h				\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_PhaseOne.cpp -o$(OBJECT_DIR)cameradriver_PhaseOne.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_save.o :		$(SRC_DIR)cameradriver_save.cpp		\
									 	$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_save.cpp -o$(OBJECT_DIR)cameradriver_save.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_sim.o :		$(SRC_DIR)cameradriver_sim.cpp		\
									 	$(SRC_DIR)cameradriver_sim.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_sim.cpp -o$(OBJECT_DIR)cameradriver_sim.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_opencv.o :	$(SRC_DIR)cameradriver_opencv.cpp	\
									 	$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_opencv.cpp -o$(OBJECT_DIR)cameradriver_opencv.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_TOUP.o :		$(SRC_DIR)cameradriver_TOUP.cpp		\
									 	$(SRC_DIR)cameradriver_TOUP.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_TOUP.cpp -o$(OBJECT_DIR)cameradriver_TOUP.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_jpeg.o :		$(SRC_DIR)cameradriver_jpeg.cpp 	\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_jpeg.cpp -o$(OBJECT_DIR)cameradriver_jpeg.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_png.o :		$(SRC_DIR)cameradriver_png.cpp 		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_png.cpp -o$(OBJECT_DIR)cameradriver_png.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_SONY.o :		$(SRC_DIR)cameradriver_SONY.cpp 	\
										$(SRC_DIR)cameradriver_SONY.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_SONY.cpp -o$(OBJECT_DIR)cameradriver_SONY.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)multicam.o :				$(SRC_DIR)multicam.cpp				\
										$(SRC_DIR)multicam.h				\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)multicam.cpp -o$(OBJECT_DIR)multicam.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)domedriver.o :				$(SRC_DIR)domedriver.cpp			\
										$(SRC_DIR)domedriver.h				\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)domedriver.cpp -o$(OBJECT_DIR)domedriver.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)domedriver_sim.o :			$(SRC_DIR)domedriver_sim.cpp		\
										$(SRC_DIR)domedriver_sim.h			\
										$(SRC_DIR)domedriver.h				\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)domedriver_sim.cpp -o$(OBJECT_DIR)domedriver_sim.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)domeshutter.o :			$(SRC_DIR)domeshutter.cpp			\
										$(SRC_DIR)domedriver.h				\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)domeshutter.cpp -o$(OBJECT_DIR)domeshutter.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)domedriver_rpi.o :			$(SRC_DIR)domedriver_rpi.cpp		\
										$(SRC_DIR)domedriver.h				\
										$(SRC_DIR)domedriver_rpi.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)domedriver_rpi.cpp -o$(OBJECT_DIR)domedriver_rpi.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)domedriver_ror_rpi.o :		$(SRC_DIR)domedriver_ror_rpi.cpp	\
										$(SRC_DIR)domedriver_ror_rpi.h		\
										$(SRC_DIR)domedriver.h				\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)domedriver_ror_rpi.cpp -o$(OBJECT_DIR)domedriver_ror_rpi.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)shutterdriver.o :			$(SRC_DIR)shutterdriver.cpp			\
										$(SRC_DIR)shutterdriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)shutterdriver.cpp -o$(OBJECT_DIR)shutterdriver.o
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)shutterdriver_arduino.o :	$(SRC_DIR)shutterdriver_arduino.cpp	\
										$(SRC_DIR)shutterdriver_arduino.h	\
										$(SRC_DIR)shutterdriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)shutterdriver_arduino.cpp -o$(OBJECT_DIR)shutterdriver_arduino.o
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)filterwheeldriver.o :		$(SRC_DIR)filterwheeldriver.cpp		\
										$(SRC_DIR)filterwheeldriver.h		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)filterwheeldriver.cpp -o$(OBJECT_DIR)filterwheeldriver.o
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)filterwheeldriver_ZWO.o :	$(SRC_DIR)filterwheeldriver_ZWO.cpp	\
										$(SRC_DIR)filterwheeldriver_ZWO.h 	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)filterwheeldriver_ZWO.cpp -o$(OBJECT_DIR)filterwheeldriver_ZWO.o
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)focuserdriver.o :			$(SRC_DIR)focuserdriver.cpp			\
										$(SRC_DIR)focuserdriver.h	 		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)focuserdriver.cpp -o$(OBJECT_DIR)focuserdriver.o
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)focuserdriver_nc.o :		$(SRC_DIR)focuserdriver_nc.cpp		\
										$(SRC_DIR)focuserdriver_nc.h 		\
										$(SRC_DIR)focuserdriver.h	 		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)focuserdriver_nc.cpp -o$(OBJECT_DIR)focuserdriver_nc.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)focuserdriver_sim.o :		$(SRC_DIR)focuserdriver_sim.cpp		\
										$(SRC_DIR)focuserdriver_sim.h 		\
										$(SRC_DIR)focuserdriver.h	 		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)focuserdriver_sim.cpp -o$(OBJECT_DIR)focuserdriver_sim.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)rotatordriver.o :			$(SRC_DIR)rotatordriver.cpp			\
										$(SRC_DIR)rotatordriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)rotatordriver.cpp -o$(OBJECT_DIR)rotatordriver.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)rotatordriver_nc.o :		$(SRC_DIR)rotatordriver_nc.cpp		\
										$(SRC_DIR)rotatordriver_nc.h	 	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)rotatordriver_nc.cpp -o$(OBJECT_DIR)rotatordriver_nc.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)rotatordriver_sim.o :		$(SRC_DIR)rotatordriver_sim.cpp		\
										$(SRC_DIR)rotatordriver_sim.h	 	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)rotatordriver_sim.cpp -o$(OBJECT_DIR)rotatordriver_sim.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)slittracker.o :		$(SRC_DIR)slittracker.cpp				\
										$(SRC_DIR)slittracker.h	 			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)slittracker.cpp -o$(OBJECT_DIR)slittracker.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)telescopedriver.o :		$(SRC_DIR)telescopedriver.cpp		\
										$(SRC_DIR)telescopedriver.h			\
										$(SRC_DIR)domedriver.h				\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)telescopedriver.cpp -o$(OBJECT_DIR)telescopedriver.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)telescopedriver_comm.o :	$(SRC_DIR)telescopedriver_comm.cpp	\
										$(SRC_DIR)telescopedriver_comm.h	\
										$(SRC_DIR)telescopedriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)telescopedriver_comm.cpp -o$(OBJECT_DIR)telescopedriver_comm.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)telescopedriver_lx200.o :	$(SRC_DIR)telescopedriver_lx200.cpp	\
										$(SRC_DIR)telescopedriver_lx200.h	\
										$(SRC_DIR)telescopedriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)telescopedriver_lx200.cpp -o$(OBJECT_DIR)telescopedriver_lx200.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)telescopedriver_Rigel.o :	$(SRC_DIR)telescopedriver_Rigel.cpp	\
										$(SRC_DIR)telescopedriver_Rigel.h	\
										$(SRC_DIR)telescopedriver_comm.h	\
										$(SRC_DIR)telescopedriver.h			\
										$(SRC_DIR)alpacadriver.h
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)telescopedriver_Rigel.cpp -o$(OBJECT_DIR)telescopedriver_Rigel.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)telescopedriver_servo.o :	$(SRC_DIR)telescopedriver_servo.cpp	\
										$(SRC_DIR)telescopedriver_servo.h	\
										$(SRC_DIR)telescopedriver_comm.h	\
										$(SRC_DIR)telescopedriver.h			\
										$(SRC_DIR)alpacadriver.h
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)telescopedriver_servo.cpp -o$(OBJECT_DIR)telescopedriver_servo.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)telescopedriver_sim.o :	$(SRC_DIR)telescopedriver_sim.cpp	\
										$(SRC_DIR)telescopedriver_sim.h	\
										$(SRC_DIR)telescopedriver.h			\
										$(SRC_DIR)alpacadriver.h
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)telescopedriver_sim.cpp -o$(OBJECT_DIR)telescopedriver_sim.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)telescopedriver_skywatch.o :	$(SRC_DIR)telescopedriver_skywatch.cpp	\
											$(SRC_DIR)telescopedriver_skywatch.h	\
											$(SRC_DIR)telescopedriver.h				\
											$(SRC_DIR)alpacadriver.h				\
											Makefile
	$(COMPILEPLUS) $(INCLUDES)				$(SRC_DIR)telescopedriver_skywatch.cpp -o$(OBJECT_DIR)telescopedriver_skywatch.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)managementdriver.o :		$(SRC_DIR)managementdriver.cpp		\
										$(SRC_DIR)managementdriver.h 		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)managementdriver.cpp -o$(OBJECT_DIR)managementdriver.o
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)switchdriver.o :			$(SRC_DIR)switchdriver.cpp			\
										$(SRC_DIR)switchdriver.h		 	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)switchdriver.cpp -o$(OBJECT_DIR)switchdriver.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)switchdriver_rpi.o :		$(SRC_DIR)switchdriver_rpi.cpp		\
										$(SRC_DIR)raspberrypi_relaylib.h	\
										$(SRC_DIR)switchdriver_rpi.h		\
										$(SRC_DIR)switchdriver.h		 	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)switchdriver_rpi.cpp -o$(OBJECT_DIR)switchdriver_rpi.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)switchdriver_sim.o :		$(SRC_DIR)switchdriver_sim.cpp		\
										$(SRC_DIR)switchdriver_sim.h		\
										$(SRC_DIR)switchdriver.h		 	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)switchdriver_sim.cpp -o$(OBJECT_DIR)switchdriver_sim.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)obsconditionsdriver.o :	$(SRC_DIR)obsconditionsdriver.cpp	\
										$(SRC_DIR)obsconditionsdriver.h	 	\
										$(SRC_DIR)alpacadriver.h			\
										$(SRC_DIR)alpaca_defs.h				\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)obsconditionsdriver.cpp -o$(OBJECT_DIR)obsconditionsdriver.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)obsconditionsdriver_rpi.o :	$(SRC_DIR)obsconditionsdriver_rpi.cpp 	\
											$(SRC_DIR)obsconditionsdriver.h			\
											$(SRC_DIR)alpacadriver.h				\
											$(SRC_DIR)alpaca_defs.h					\
											Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)obsconditionsdriver_rpi.cpp -o$(OBJECT_DIR)obsconditionsdriver_rpi.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)obsconditionsdriver_sim.o :	$(SRC_DIR)obsconditionsdriver_sim.cpp 	\
											$(SRC_DIR)obsconditionsdriver_sim.h		\
											$(SRC_DIR)obsconditionsdriver.h			\
											$(SRC_DIR)alpacadriver.h				\
											$(SRC_DIR)alpaca_defs.h					\
											Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)obsconditionsdriver_sim.cpp -o$(OBJECT_DIR)obsconditionsdriver_sim.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)calibrationdriver.o :			$(SRC_DIR)calibrationdriver.cpp 	\
											$(SRC_DIR)calibrationdriver.h		\
											$(SRC_DIR)alpacadriver.h			\
											Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)calibrationdriver.cpp -o$(OBJECT_DIR)calibrationdriver.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)calibrationdriver_rpi.o :		$(SRC_DIR)calibrationdriver_rpi.cpp \
											$(SRC_DIR)calibrationdriver_rpi.h	\
											$(SRC_DIR)calibrationdriver.h		\
											$(SRC_DIR)alpacadriver.h			\
											Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)calibrationdriver_rpi.cpp -o$(OBJECT_DIR)calibrationdriver_rpi.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)calibration_sim.o :			$(SRC_DIR)calibration_sim.cpp	\
											$(SRC_DIR)calibration_sim.h		\
											$(SRC_DIR)calibrationdriver.h	\
											$(SRC_DIR)alpacadriver.h		\
											Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)calibration_sim.cpp -o$(OBJECT_DIR)calibration_sim.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)calibration_Alnitak.o :		$(SRC_DIR)calibration_Alnitak.cpp 	\
											$(SRC_DIR)calibration_Alnitak.h		\
											$(SRC_DIR)calibrationdriver.h		\
											$(SRC_DIR)alpacadriver.h			\
											Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)calibration_Alnitak.cpp -o$(OBJECT_DIR)calibration_Alnitak.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)usbmanager.o :					$(SRC_DIR)usbmanager.cpp 			\
											$(SRC_DIR)usbmanager.h				\
											Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)usbmanager.cpp -o$(OBJECT_DIR)usbmanager.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)discoverythread.o :		$(SRC_DIR)discoverythread.c 		\
										$(SRC_DIR)discoverythread.h 		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)discoverythread.c -o$(OBJECT_DIR)discoverythread.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)HostNames.o :				$(SRC_DIR)HostNames.c 	\
										$(SRC_DIR)HostNames.h 	\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)HostNames.c -o$(OBJECT_DIR)HostNames.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)sendrequest_lib.o :		$(SRC_DIR)sendrequest_lib.c 	\
										$(SRC_DIR)sendrequest_lib.h 	\
										$(MLS_LIB_DIR)json_parse.h
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)sendrequest_lib.c -o$(OBJECT_DIR)sendrequest_lib.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)observatory_settings.o :	$(SRC_DIR)observatory_settings.c 	\
										$(SRC_DIR)observatory_settings.h
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)observatory_settings.c -o$(OBJECT_DIR)observatory_settings.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)serialport.o :				$(SRC_DIR)serialport.c 	\
										$(SRC_DIR)serialport.h
	$(COMPILE) $(INCLUDES)				$(SRC_DIR)serialport.c -o$(OBJECT_DIR)serialport.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)sidereal.o :				$(SRC_DIR)sidereal.c 			\
										$(SRC_DIR)sidereal.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)sidereal.c -o$(OBJECT_DIR)sidereal.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cpu_stats.o :				$(SRC_DIR)cpu_stats.c 			\
										$(SRC_DIR)cpu_stats.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)cpu_stats.c -o$(OBJECT_DIR)cpu_stats.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)helper_functions.o :		$(SRC_DIR)helper_functions.c 			\
										$(SRC_DIR)helper_functions.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)helper_functions.c -o$(OBJECT_DIR)helper_functions.o



######################################################################################
# ATIK objects
$(OBJECT_DIR)camera_atik.o : $(SRC_DIR)camera_atik.c $(SRC_DIR)camera_atik.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)camera_atik.c -o$(OBJECT_DIR)camera_atik.o



######################################################################################
#	CLIENT_OBJECTS
$(OBJECT_DIR)json_parse.o : $(MLS_LIB_DIR)json_parse.c $(MLS_LIB_DIR)json_parse.h
	$(COMPILE) $(INCLUDES) $(MLS_LIB_DIR)json_parse.c -o$(OBJECT_DIR)json_parse.o

$(OBJECT_DIR)discoveryclient.o : $(SRC_DISCOVERY)discoveryclient.c $(SRC_DISCOVERY)discoveryclient.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DISCOVERY)discoveryclient.c -o$(OBJECT_DIR)discoveryclient.o




######################################################################################
$(OBJECT_DIR)mandelbrot.o : $(SRC_DIR)mandelbrot.c
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)mandelbrot.c -o$(OBJECT_DIR)mandelbrot.o



######################################################################################
$(OBJECT_DIR)controller.o : $(SRC_DIR)controller.cpp $(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller.cpp -o$(OBJECT_DIR)controller.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controllerClient.o : 		$(SRC_DIR)controllerClient.cpp		\
										$(SRC_DIR)controllerClient.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controllerClient.cpp -o$(OBJECT_DIR)controllerClient.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controllerServer.o : 		$(SRC_DIR)controllerServer.cpp		\
										$(SRC_DIR)controllerServer.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controllerServer.cpp -o$(OBJECT_DIR)controllerServer.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controllerAlpaca.o : 		$(SRC_DIR)controllerAlpaca.cpp		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controllerAlpaca.cpp -o$(OBJECT_DIR)controllerAlpaca.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_filterwheel.o : $(SRC_DIR)controller_filterwheel.cpp	\
										$(SRC_DIR)controller_fw_common.cpp		\
										$(SRC_DIR)controller_filterwheel.h		\
										$(SRC_DIR)controller.h					\
										$(SRC_DIR)windowtab_about.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_filterwheel.cpp -o$(OBJECT_DIR)controller_filterwheel.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_focus.o : 		$(SRC_DIR)controller_focus.cpp		\
										$(SRC_DIR)controller_focus.h		\
										$(SRC_DIR)controller.h				\
										$(SRC_DIR)focuser_AlpacaCmds.cpp	\
										$(SRC_DIR)windowtab_auxmotor.h		\
										$(SRC_DIR)windowtab_config.h		\
										$(SRC_DIR)windowtab_ml_single.h		\
										$(SRC_DIR)windowtab_graphs.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_focus.cpp -o$(OBJECT_DIR)controller_focus.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_focus_generic.o : 	$(SRC_DIR)controller_focus_generic.cpp	\
											$(SRC_DIR)controller_focus_generic.h	\
											$(SRC_DIR)controller_focus.h			\
											$(SRC_DIR)controller.h					\
											$(SRC_DIR)windowtab_ml_single.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_focus_generic.cpp -o$(OBJECT_DIR)controller_focus_generic.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_focus_ml_nc.o : $(SRC_DIR)controller_focus_ml_nc.cpp	\
										$(SRC_DIR)controller_focus_ml_nc.h		\
										$(SRC_DIR)controller_focus.h			\
										$(SRC_DIR)windowtab_about.h				\
										$(SRC_DIR)windowtab_nitecrawler.h		\
										$(SRC_DIR)windowtab_graphs.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_focus_ml_nc.cpp -o$(OBJECT_DIR)controller_focus_ml_nc.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_focus_ml_hr.o : $(SRC_DIR)controller_focus_ml_hr.cpp	\
										$(SRC_DIR)controller_focus_ml_hr.h		\
										$(SRC_DIR)controller_focus.h			\
										$(SRC_DIR)windowtab_about.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_focus_ml_hr.cpp -o$(OBJECT_DIR)controller_focus_ml_hr.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_obsconditions.o : 	$(SRC_DIR)controller_obsconditions.cpp		\
											$(SRC_DIR)controller_obsconditions.h		\
											$(SRC_DIR)controller.h						\
											$(SRC_DIR)alpaca_defs.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_obsconditions.cpp -o$(OBJECT_DIR)controller_obsconditions.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_rotator.o : 		$(SRC_DIR)controller_rotator.cpp		\
											$(SRC_DIR)controller_rotator.h			\
											$(SRC_DIR)controller.h					\
											$(SRC_DIR)alpaca_defs.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_rotator.cpp -o$(OBJECT_DIR)controller_rotator.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_rotator.o : 			$(SRC_DIR)windowtab_rotator.cpp			\
											$(SRC_DIR)windowtab_rotator.h			\
											$(SRC_DIR)windowtab.h					\
											$(SRC_DIR)alpaca_defs.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_rotator.cpp -o$(OBJECT_DIR)windowtab_rotator.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_switch.o : 		$(SRC_DIR)controller_switch.cpp		\
										$(SRC_DIR)controller_switch.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_switch.cpp -o$(OBJECT_DIR)controller_switch.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_camera.o : 		$(SRC_DIR)controller_camera.cpp		\
										$(SRC_DIR)controller_camera.h		\
										$(SRC_DIR)controller_fw_common.cpp	\
										$(SRC_DIR)windowtab_camera.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_camera.cpp -o$(OBJECT_DIR)controller_camera.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_cam_normal.o : 	$(SRC_DIR)controller_cam_normal.cpp	\
										$(SRC_DIR)controller_cam_normal.h	\
										$(SRC_DIR)controller_fw_common.cpp	\
										$(SRC_DIR)windowtab_camera.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_cam_normal.cpp -o$(OBJECT_DIR)controller_cam_normal.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controllerImageArray.o : 	$(SRC_DIR)controllerImageArray.cpp	\
										$(SRC_DIR)controller_fw_common.cpp	\
										$(SRC_DIR)controller_camera.h		\
										$(SRC_DIR)windowtab_camera.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controllerImageArray.cpp -o$(OBJECT_DIR)controllerImageArray.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_covercalib.o : 	$(SRC_DIR)controller_covercalib.cpp	\
										$(SRC_DIR)controller_covercalib.h	\
										$(SRC_DIR)windowtab_covercalib.h	\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_covercalib.cpp -o$(OBJECT_DIR)controller_covercalib.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_dome.o : 		$(SRC_DIR)controller_dome.cpp		\
										$(SRC_DIR)controller_dome.h			\
										$(SRC_DIR)windowtab_dome.h			\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_dome.cpp -o$(OBJECT_DIR)controller_dome.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_multicam.o : 	$(SRC_DIR)controller_multicam.cpp	\
										$(SRC_DIR)controller_multicam.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_multicam.cpp -o$(OBJECT_DIR)controller_multicam.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_startup.o : 	$(SRC_DIR)controller_startup.cpp	\
										$(SRC_DIR)controller_startup.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_startup.cpp -o$(OBJECT_DIR)controller_startup.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_startup.o : 		$(SRC_DIR)windowtab_startup.cpp		\
										$(SRC_DIR)windowtab_startup.h		\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_startup.cpp -o$(OBJECT_DIR)windowtab_startup.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_fitsheader.o : 	$(SRC_DIR)windowtab_fitsheader.cpp		\
										$(SRC_DIR)windowtab_fitsheader.h		\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_fitsheader.cpp -o$(OBJECT_DIR)windowtab_fitsheader.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_slit.o : 		$(SRC_DIR)controller_slit.cpp		\
										$(SRC_DIR)controller_slit.h			\
										$(SRC_DIR)windowtab_slit.h			\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_slit.cpp -o$(OBJECT_DIR)controller_slit.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_dome_common.o : $(SRC_DIR)controller_dome_common.cpp	\
										$(SRC_DIR)controller_dome.h				\
										$(SRC_DIR)windowtab_dome.h				\
										$(SRC_DIR)windowtab_about.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_dome_common.cpp -o$(OBJECT_DIR)controller_dome_common.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_telescope.o :	$(SRC_DIR)controller_telescope.cpp		\
										$(SRC_DIR)controller_tscope_common.cpp	\
										$(SRC_DIR)controller_telescope.h		\
										$(SRC_DIR)windowtab_telescope.h			\
										$(SRC_DIR)windowtab_about.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_telescope.cpp -o$(OBJECT_DIR)controller_telescope.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_alpacalist.o :	$(SRC_DIR)windowtab_alpacalist.cpp		\
										$(SRC_DIR)windowtab_alpacalist.h		\
										$(SRC_DIR)windowtab.h					\
										$(SRC_DIR)discoverythread.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_alpacalist.cpp -o$(OBJECT_DIR)windowtab_alpacalist.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_iplist.o : 		$(SRC_DIR)windowtab_iplist.cpp		\
										$(SRC_DIR)windowtab_iplist.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)discoverythread.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_iplist.cpp -o$(OBJECT_DIR)windowtab_iplist.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_sw_versions.o : 	$(SRC_DIR)windowtab_sw_versions.cpp		\
										$(SRC_DIR)windowtab_sw_versions.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)discoverythread.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_sw_versions.cpp -o$(OBJECT_DIR)windowtab_sw_versions.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_deviceselect.o : $(SRC_DIR)windowtab_deviceselect.cpp	\
										$(SRC_DIR)windowtab_deviceselect.h		\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_deviceselect.cpp -o$(OBJECT_DIR)windowtab_deviceselect.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_image.o : 		$(SRC_DIR)controller_image.cpp		\
										$(SRC_DIR)controller_image.h		\
										$(SRC_DIR)windowtab_image.h			\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_image.cpp -o$(OBJECT_DIR)controller_image.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_image.o : 		$(SRC_DIR)windowtab_image.cpp		\
										$(SRC_DIR)windowtab_image.h			\
										$(SRC_DIR)controller_image.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_image.cpp -o$(OBJECT_DIR)windowtab_image.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_imageinfo.o : 	$(SRC_DIR)windowtab_imageinfo.cpp	\
										$(SRC_DIR)windowtab_imageinfo.h		\
										$(SRC_DIR)controller_image.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_imageinfo.cpp -o$(OBJECT_DIR)windowtab_imageinfo.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_usb.o : 		$(SRC_DIR)controller_usb.cpp		\
										$(SRC_DIR)controller_usb.h			\
										$(SRC_DIR)windowtab_usb.h			\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_usb.cpp -o$(OBJECT_DIR)controller_usb.o




#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_preview.o : 	$(SRC_DIR)controller_preview.cpp	\
										$(SRC_DIR)controller_preview.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_preview.cpp -o$(OBJECT_DIR)controller_preview.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab.o : 				$(SRC_DIR)windowtab.cpp				\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab.cpp -o$(OBJECT_DIR)windowtab.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_camsettings.o : 	$(SRC_DIR)windowtab_camsettings.cpp	\
										$(SRC_DIR)windowtab_camsettings.h	\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller_camera.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_camsettings.cpp -o$(OBJECT_DIR)windowtab_camsettings.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_camcooler.o : 	$(SRC_DIR)windowtab_camcooler.cpp	\
										$(SRC_DIR)windowtab_camcooler.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller_camera.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_camcooler.cpp -o$(OBJECT_DIR)windowtab_camcooler.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_camvideo.o : 	$(SRC_DIR)windowtab_camvideo.cpp	\
										$(SRC_DIR)windowtab_camvideo.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_camvideo.cpp -o$(OBJECT_DIR)windowtab_camvideo.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_filelist.o : 	$(SRC_DIR)windowtab_filelist.cpp	\
										$(SRC_DIR)controller_camera.h		\
										$(SRC_DIR)windowtab_filelist.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_filelist.cpp -o$(OBJECT_DIR)windowtab_filelist.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_filterwheel.o : 	$(SRC_DIR)windowtab_filterwheel.cpp	\
										$(SRC_DIR)windowtab_filterwheel.h	\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_filterwheel.cpp -o$(OBJECT_DIR)windowtab_filterwheel.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_ml_single.o : 	$(SRC_DIR)windowtab_ml_single.cpp	\
										$(SRC_DIR)windowtab_ml_single.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_ml_single.cpp -o$(OBJECT_DIR)windowtab_ml_single.o

$(OBJECT_DIR)windowtab_nitecrawler.o : 	$(SRC_DIR)windowtab_nitecrawler.cpp	\
										$(SRC_DIR)windowtab_nitecrawler.h	\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_nitecrawler.cpp -o$(OBJECT_DIR)windowtab_nitecrawler.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_about.o : 		$(SRC_DIR)windowtab_about.cpp		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_about.cpp -o$(OBJECT_DIR)windowtab_about.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_drvrInfo.o : 	$(SRC_DIR)windowtab_drvrInfo.cpp		\
										$(SRC_DIR)windowtab_drvrInfo.h			\
										$(SRC_DIR)windowtab.h					\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_drvrInfo.cpp -o$(OBJECT_DIR)windowtab_drvrInfo.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_capabilities.o : $(SRC_DIR)windowtab_capabilities.cpp	\
										$(SRC_DIR)windowtab_capabilities.h		\
										$(SRC_DIR)windowtab.h					\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_capabilities.cpp -o$(OBJECT_DIR)windowtab_capabilities.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_DeviceState.o :	$(SRC_DIR)windowtab_DeviceState.cpp		\
										$(SRC_DIR)windowtab_DeviceState.h		\
										$(SRC_DIR)windowtab.h					\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_DeviceState.cpp -o$(OBJECT_DIR)windowtab_DeviceState.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_moon.o : 		$(SRC_DIR)windowtab_moon.cpp		\
										$(SRC_DIR)windowtab_moon.h			\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_moon.cpp -o$(OBJECT_DIR)windowtab_moon.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_obscond.o : 		$(SRC_DIR)windowtab_obscond.cpp		\
										$(SRC_DIR)windowtab_obscond.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)alpaca_defs.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_obscond.cpp -o$(OBJECT_DIR)windowtab_obscond.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_multicam.o : 	$(SRC_DIR)windowtab_multicam.cpp	\
										$(SRC_DIR)windowtab_multicam.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)alpaca_defs.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_multicam.cpp -o$(OBJECT_DIR)windowtab_multicam.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)opencv_utils.o : 			$(SRC_DIR)opencv_utils.cpp		\
										$(SRC_DIR)opencv_utils.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)opencv_utils.cpp -o$(OBJECT_DIR)opencv_utils.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_preview.o : 		$(SRC_DIR)windowtab_preview.cpp		\
										$(SRC_DIR)windowtab_preview.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_preview.cpp -o$(OBJECT_DIR)windowtab_preview.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_auxmotor.o : 	$(SRC_DIR)windowtab_auxmotor.cpp	\
										$(SRC_DIR)windowtab_auxmotor.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_auxmotor.cpp -o$(OBJECT_DIR)windowtab_auxmotor.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_camera.o : 		$(SRC_DIR)windowtab_camera.cpp		\
										$(SRC_DIR)windowtab_camera.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_camera.cpp -o$(OBJECT_DIR)windowtab_camera.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_config.o : 		$(SRC_DIR)windowtab_config.cpp		\
										$(SRC_DIR)windowtab_config.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_config.cpp -o$(OBJECT_DIR)windowtab_config.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_covercalib.o : 	$(SRC_DIR)windowtab_covercalib.cpp	\
										$(SRC_DIR)windowtab_covercalib.h	\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_covercalib.cpp -o$(OBJECT_DIR)windowtab_covercalib.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_dome.o : 		$(SRC_DIR)windowtab_dome.cpp		\
										$(SRC_DIR)windowtab_dome.h			\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_dome.cpp -o$(OBJECT_DIR)windowtab_dome.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_mount.o : 		$(SRC_DIR)windowtab_mount.cpp		\
										$(SRC_DIR)windowtab_mount.h			\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_mount.cpp -o$(OBJECT_DIR)windowtab_mount.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_graphs.o : 		$(SRC_DIR)windowtab_graphs.cpp		\
										$(SRC_DIR)windowtab_graphs.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_graphs.cpp -o$(OBJECT_DIR)windowtab_graphs.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_slit.o : 		$(SRC_DIR)windowtab_slit.cpp		\
										$(SRC_DIR)windowtab_slit.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_slit.cpp -o$(OBJECT_DIR)windowtab_slit.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_slitdome.o : 	$(SRC_DIR)windowtab_slitdome.cpp		\
										$(SRC_DIR)windowtab_slitdome.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_slitdome.cpp -o$(OBJECT_DIR)windowtab_slitdome.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_slitgraph.o : 	$(SRC_DIR)windowtab_slitgraph.cpp	\
										$(SRC_DIR)windowtab_slitgraph.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_slitgraph.cpp -o$(OBJECT_DIR)windowtab_slitgraph.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_switch.o : 		$(SRC_DIR)windowtab_switch.cpp		\
										$(SRC_DIR)windowtab_switch.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_switch.cpp -o$(OBJECT_DIR)windowtab_switch.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_usb.o : 			$(SRC_DIR)windowtab_usb.cpp			\
										$(SRC_DIR)windowtab_usb.h			\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_usb.cpp -o$(OBJECT_DIR)windowtab_usb.o





#-------------------------------------------------------------------------------------
$(OBJECT_DIR)nitecrawler_image.o : 		$(SRC_DIR)nitecrawler_image.c		\
										$(SRC_DIR)nitecrawler_image.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)nitecrawler_image.c -o$(OBJECT_DIR)nitecrawler_image.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)moonlite_com.o : 			$(SRC_DIR)moonlite_com.c			\
										$(SRC_DIR)moonlite_com.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)moonlite_com.c -o$(OBJECT_DIR)moonlite_com.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_main.o : 		$(SRC_DIR)controller_main.cpp		\
										$(SRC_DIR)controller_focus.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_main.cpp -o$(OBJECT_DIR)controller_main.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)fits_opencv.o :			$(SRC_DIR)fits_opencv.c			\
										$(SRC_DIR)fits_opencv.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)fits_opencv.c -o$(OBJECT_DIR)fits_opencv.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)fitsview.o :				$(SRC_DIR)fitsview.c
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)fitsview.c -o$(OBJECT_DIR)fitsview.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)dumpfits.o :				$(SRC_DIR)dumpfits.c
	$(COMPILE) $(INCLUDES) $(SRC_DIR)dumpfits.c -o$(OBJECT_DIR)dumpfits.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)PDS_ReadNASAfiles.o :		$(SRC_PDS)PDS_ReadNASAfiles.c			\
										$(SRC_PDS)PDS_ReadNASAfiles.h
	$(COMPILE) $(INCLUDES) $(SRC_PDS)PDS_ReadNASAfiles.c -o$(OBJECT_DIR)PDS_ReadNASAfiles.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)PDS_decompress.o :			$(SRC_PDS)PDS_decompress.c			\
										$(SRC_PDS)PDS_decompress.h
	$(COMPILE) $(INCLUDES) $(SRC_PDS)PDS_decompress.c -o$(OBJECT_DIR)PDS_decompress.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)discovery_lib.o :			$(SRC_DIR)discovery_lib.c			\
										$(SRC_DIR)discovery_lib.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)discovery_lib.c -o$(OBJECT_DIR)discovery_lib.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)commoncolor.o :			$(SRC_DIR)commoncolor.c				\
										$(SRC_DIR)commoncolor.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)commoncolor.c -o$(OBJECT_DIR)commoncolor.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)imageprocess_orb.o :		$(SRC_IMGPROC)imageprocess_orb.cpp	\
										$(SRC_IMGPROC)imageprocess_orb.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_IMGPROC)imageprocess_orb.cpp -o$(OBJECT_DIR)imageprocess_orb.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)moonphase.o :				$(SRC_DIR)moonphase.c	\
										$(SRC_DIR)moonphase.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)moonphase.c -o$(OBJECT_DIR)moonphase.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)MoonRise.o :				$(SRC_MOONRISE)MoonRise.cpp	\
										$(SRC_MOONRISE)MoonRise.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_MOONRISE)MoonRise.cpp -I$(SRC_MOONRISE) -o$(OBJECT_DIR)MoonRise.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)julianTime.o :				$(SRC_DIR)julianTime.c	\
										$(SRC_DIR)julianTime.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)julianTime.c -o$(OBJECT_DIR)julianTime.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)raspberrypi_relaylib.o :	$(SRC_DIR)raspberrypi_relaylib.c	\
										$(SRC_DIR)raspberrypi_relaylib.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)raspberrypi_relaylib.c -o$(OBJECT_DIR)raspberrypi_relaylib.o




######################################################################################
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)skytravel_main.o :			$(SRC_SKYTRAVEL)skytravel_main.cpp	\
										$(SRC_SKYTRAVEL)windowtab_skytravel.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)skytravel_main.cpp -o$(OBJECT_DIR)skytravel_main.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_telescope.o :	$(SRC_DIR)windowtab_telescope.cpp	\
										$(SRC_DIR)windowtab_telescope.h		\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_telescope.cpp -o$(OBJECT_DIR)windowtab_telescope.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_teleSettings.o :	$(SRC_DIR)windowtab_teleSettings.cpp	\
										$(SRC_DIR)windowtab_teleSettings.h		\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_teleSettings.cpp -o$(OBJECT_DIR)windowtab_teleSettings.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_skytravel.o :	$(SRC_SKYTRAVEL)controller_skytravel.cpp	\
										$(SRC_DIR)controller_tscope_common.cpp		\
										$(SRC_DIR)controller_dome_common.cpp		\
										$(SRC_SKYTRAVEL)controller_skytravel.h		\
										$(SRC_SKYTRAVEL)SkyStruc.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)controller_skytravel.cpp -o$(OBJECT_DIR)controller_skytravel.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_remoteview.o :	$(SRC_DIR)controller_remoteview.cpp		\
										$(SRC_DIR)controller_remoteview.cpp		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_remoteview.cpp -o$(OBJECT_DIR)controller_remoteview.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_time.o : 			$(SRC_SKYTRAVEL)windowtab_time.cpp			\
											$(SRC_SKYTRAVEL)windowtab_time.h			\
											$(SRC_DIR)windowtab.h					\
											$(SRC_DIR)alpaca_defs.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_time.cpp -o$(OBJECT_DIR)windowtab_time.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_fov.o : 			$(SRC_SKYTRAVEL)windowtab_fov.cpp			\
										$(SRC_SKYTRAVEL)windowtab_fov.h				\
										$(SRC_SKYTRAVEL)cameraFOV.h					\
										$(SRC_DIR)windowtab.h						\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_fov.cpp -o$(OBJECT_DIR)windowtab_fov.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)StarData.o :				$(SRC_SKYTRAVEL)StarData.c	\
										$(SRC_SKYTRAVEL)StarData.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)StarData.c -o$(OBJECT_DIR)StarData.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)MessierData.o :			$(SRC_SKYTRAVEL)MessierData.c	\
										$(SRC_SKYTRAVEL)StarData.h		\
										$(SRC_SKYTRAVEL)SkyStruc.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)MessierData.c -o$(OBJECT_DIR)MessierData.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)SAO_stardata.o :			$(SRC_SKYTRAVEL)SAO_stardata.c	\
										$(SRC_SKYTRAVEL)SAO_stardata.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)SAO_stardata.c -o$(OBJECT_DIR)SAO_stardata.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)aavso_data.o :				$(SRC_SKYTRAVEL)aavso_data.c	\
										$(SRC_SKYTRAVEL)aavso_data.h	\
										$(SRC_SKYTRAVEL)SkyStruc.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)aavso_data.c -o$(OBJECT_DIR)aavso_data.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)polaralign.o :				$(SRC_SKYTRAVEL)polaralign.cpp	\
										$(SRC_SKYTRAVEL)polaralign.h	\
										$(SRC_SKYTRAVEL)SkyStruc.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)polaralign.cpp -o$(OBJECT_DIR)polaralign.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_skytravel.o :	$(SRC_SKYTRAVEL)windowtab_skytravel.cpp	\
										$(SRC_SKYTRAVEL)windowtab_skytravel.h	\
										$(SRC_DIR)windowtab.h					\
										$(SRC_SKYTRAVEL)AsteroidData.h			\
										$(SRC_SKYTRAVEL)aavso_data.h			\
										$(SRC_SKYTRAVEL)StarData.h				\
										$(SRC_SKYTRAVEL)SkyStruc.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_skytravel.cpp -o$(OBJECT_DIR)windowtab_skytravel.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_cpustats.o :		$(SRC_SKYTRAVEL)windowtab_cpustats.cpp	\
										$(SRC_SKYTRAVEL)windowtab_cpustats.h	\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_cpustats.cpp -o$(OBJECT_DIR)windowtab_cpustats.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_STsettings.o :	$(SRC_SKYTRAVEL)windowtab_STsettings.cpp	\
										$(SRC_SKYTRAVEL)windowtab_STsettings.h		\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_STsettings.cpp -o$(OBJECT_DIR)windowtab_STsettings.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_RemoteData.o :	$(SRC_SKYTRAVEL)windowtab_RemoteData.cpp	\
										$(SRC_SKYTRAVEL)windowtab_RemoteData.h		\
										$(SRC_DIR)windowtab.h						\
										$(SRC_SKYTRAVEL)RemoteImage.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_RemoteData.cpp -o$(OBJECT_DIR)windowtab_RemoteData.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)eph.o :					$(SRC_SKYTRAVEL)eph.c	\
										$(SRC_SKYTRAVEL)eph.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)eph.c -o$(OBJECT_DIR)eph.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)SkyTravelTimeRoutines.o :	$(SRC_SKYTRAVEL)SkyTravelTimeRoutines.c	\
										$(SRC_SKYTRAVEL)SkyTravelTimeRoutines.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)SkyTravelTimeRoutines.c -o$(OBJECT_DIR)SkyTravelTimeRoutines.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)NGCcatalog.o :				$(SRC_SKYTRAVEL)NGCcatalog.c	\
										$(SRC_SKYTRAVEL)NGCcatalog.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)NGCcatalog.c -o$(OBJECT_DIR)NGCcatalog.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)OpenNGC.o :				$(SRC_SKYTRAVEL)OpenNGC.c	\
										$(SRC_SKYTRAVEL)OpenNGC.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)OpenNGC.c -o$(OBJECT_DIR)OpenNGC.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)StarCatalogHelper.o :		$(SRC_SKYTRAVEL)StarCatalogHelper.c	\
										$(SRC_SKYTRAVEL)StarCatalogHelper.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)StarCatalogHelper.c -o$(OBJECT_DIR)StarCatalogHelper.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)YaleStarCatalog.o :		$(SRC_SKYTRAVEL)YaleStarCatalog.c	\
										$(SRC_SKYTRAVEL)YaleStarCatalog.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)YaleStarCatalog.c -o$(OBJECT_DIR)YaleStarCatalog.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)HipparcosCatalog.o :		$(SRC_SKYTRAVEL)HipparcosCatalog.c	\
										$(SRC_SKYTRAVEL)HipparcosCatalog.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)HipparcosCatalog.c -o$(OBJECT_DIR)HipparcosCatalog.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)ConstellationData.o :		$(SRC_SKYTRAVEL)ConstellationData.c	\
										$(SRC_SKYTRAVEL)ConstellationData.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)ConstellationData.c -o$(OBJECT_DIR)ConstellationData.o

#-------------------------------------------------------------------------------------
#$(OBJECT_DIR)GaiaData.o :				$(SRC_SKYTRAVEL)GaiaData.c	\
#										$(SRC_SKYTRAVEL)GaiaData.h
#	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)GaiaData.c -o$(OBJECT_DIR)GaiaData.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)GaiaSQL.o :				$(SRC_SKYTRAVEL)GaiaSQL.cpp	\
										$(SRC_SKYTRAVEL)GaiaSQL.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)GaiaSQL.cpp -o$(OBJECT_DIR)GaiaSQL.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_GaiaRemote.o :	$(SRC_SKYTRAVEL)controller_GaiaRemote.cpp	\
										$(SRC_SKYTRAVEL)controller_GaiaRemote.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)controller_GaiaRemote.cpp -o$(OBJECT_DIR)controller_GaiaRemote.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_GaiaRemote.o :	$(SRC_SKYTRAVEL)windowtab_GaiaRemote.cpp	\
										$(SRC_DIR)windowtab.h						\
										$(SRC_SKYTRAVEL)windowtab_GaiaRemote.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_GaiaRemote.cpp -o$(OBJECT_DIR)windowtab_GaiaRemote.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)AsteroidData.o :			$(SRC_SKYTRAVEL)AsteroidData.c	\
										$(SRC_SKYTRAVEL)AsteroidData.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)AsteroidData.c -o$(OBJECT_DIR)AsteroidData.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_starlist.o : 	$(SRC_SKYTRAVEL)controller_starlist.cpp	\
										$(SRC_SKYTRAVEL)controller_starlist.h	\
										$(SRC_DIR)windowtab_about.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)controller_starlist.cpp -o$(OBJECT_DIR)controller_starlist.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_starlist.o : 	$(SRC_SKYTRAVEL)windowtab_starlist.cpp	\
										$(SRC_SKYTRAVEL)windowtab_starlist.h	\
										$(SRC_DIR)windowtab.h					\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_starlist.cpp -o$(OBJECT_DIR)windowtab_starlist.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_constList.o :	$(SRC_SKYTRAVEL)windowtab_constList.cpp	\
										$(SRC_SKYTRAVEL)windowtab_constList.h	\
										$(SRC_DIR)windowtab.h					\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_constList.cpp -o$(OBJECT_DIR)windowtab_constList.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_constList.o : 	$(SRC_SKYTRAVEL)controller_constList.cpp			\
												$(SRC_SKYTRAVEL)controller_constList.h		\
												$(SRC_DIR)windowtab_about.h					\
												$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)controller_constList.cpp -o$(OBJECT_DIR)controller_constList.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)RemoteImage.o : 			$(SRC_SKYTRAVEL)RemoteImage.cpp	\
										$(SRC_SKYTRAVEL)RemoteImage.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)RemoteImage.cpp -o$(OBJECT_DIR)RemoteImage.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_alpacaUnit.o : 		$(SRC_DIR)controller_alpacaUnit.cpp		\
											$(SRC_DIR)controller_alpacaUnit.h		\
											$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_alpacaUnit.cpp -o$(OBJECT_DIR)controller_alpacaUnit.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_libraries.o : 		$(SRC_DIR)windowtab_libraries.cpp		\
											$(SRC_DIR)windowtab_libraries.h		\
											$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_libraries.cpp -o$(OBJECT_DIR)windowtab_libraries.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_alpacaUnit.o : 		$(SRC_DIR)windowtab_alpacaUnit.cpp		\
											$(SRC_DIR)windowtab_alpacaUnit.h		\
											$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_alpacaUnit.cpp -o$(OBJECT_DIR)windowtab_alpacaUnit.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)lx200_com.o :				$(SRC_DIR)lx200_com.c	\
										$(SRC_DIR)lx200_com.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)lx200_com.c -o$(OBJECT_DIR)lx200_com.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)linuxerrors.o :			$(SRC_DIR)linuxerrors.c	\
										$(SRC_DIR)linuxerrors.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)linuxerrors.c -o$(OBJECT_DIR)linuxerrors.o


SRC_SPECIAL			=	./src_special/

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_video.o : 		$(SRC_SPECIAL)windowtab_video.cpp	\
										$(SRC_SPECIAL)windowtab_video.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SPECIAL)windowtab_video.cpp -o$(OBJECT_DIR)windowtab_video.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_video.o : 		$(SRC_SPECIAL)controller_video.cpp	\
										$(SRC_SPECIAL)controller_video.h	\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SPECIAL)controller_video.cpp -o$(OBJECT_DIR)controller_video.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)startextrathread.o : 		$(SRC_SPECIAL)startextrathread.cpp	\
										$(SRC_DIR)alpacadriver_helper.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SPECIAL)startextrathread.cpp -o$(OBJECT_DIR)startextrathread.o

##################################################################################
#		Servo source code
##################################################################################
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)servo_mount_cfg.o : 		$(SRC_SERVO)servo_mount_cfg.c	\
										$(SRC_SERVO)servo_mount_cfg.h	\
										$(SRC_SERVO)servo_std_defs.h
	$(COMPILE) $(INCLUDES) $(SRC_SERVO)servo_mount_cfg.c -o$(OBJECT_DIR)servo_mount_cfg.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)servo_time.o : 			$(SRC_SERVO)servo_time.c	\
										$(SRC_SERVO)servo_time.h	\
										$(SRC_SERVO)servo_std_defs.h
	$(COMPILE) $(INCLUDES) $(SRC_SERVO)servo_time.c -o$(OBJECT_DIR)servo_time.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)servo_mount.o : 			$(SRC_SERVO)servo_mount.c	\
										$(SRC_SERVO)servo_mount.h	\
										$(SRC_SERVO)servo_std_defs.h
	$(COMPILE) $(INCLUDES) $(SRC_SERVO)servo_mount.c -o$(OBJECT_DIR)servo_mount.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)servo_motion.o : 			$(SRC_SERVO)servo_motion.c	\
										$(SRC_SERVO)servo_motion.h	\
										$(SRC_SERVO)servo_std_defs.h
	$(COMPILE) $(INCLUDES) $(SRC_SERVO)servo_motion.c -o$(OBJECT_DIR)servo_motion.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)servo_motion_cfg.o : 		$(SRC_SERVO)servo_motion_cfg.c	\
										$(SRC_SERVO)servo_motion_cfg.h	\
										$(SRC_SERVO)servo_std_defs.h
	$(COMPILE) $(INCLUDES) $(SRC_SERVO)servo_motion_cfg.c -o$(OBJECT_DIR)servo_motion_cfg.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)servo_observ_cfg.o : 		$(SRC_SERVO)servo_observ_cfg.c	\
										$(SRC_SERVO)servo_observ_cfg.h	\
										$(SRC_SERVO)servo_std_defs.h
	$(COMPILE) $(INCLUDES) $(SRC_SERVO)servo_observ_cfg.c -o$(OBJECT_DIR)servo_observ_cfg.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)servo_rc_utils.o : 		$(SRC_SERVO)servo_rc_utils.c	\
										$(SRC_SERVO)servo_rc_utils.h	\
										$(SRC_SERVO)servo_std_defs.h
	$(COMPILE) $(INCLUDES) $(SRC_SERVO)servo_rc_utils.c -o$(OBJECT_DIR)servo_rc_utils.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)servo_mc_core.o : 			$(SRC_SERVO)servo_mc_core.c	\
										$(SRC_SERVO)servo_mc_core.h	\
										$(SRC_SERVO)servo_std_defs.h
	$(COMPILE) $(INCLUDES) $(SRC_SERVO)servo_mc_core.c -o$(OBJECT_DIR)servo_mc_core.o



##################################################################################
#		SkyImage stuff
##################################################################################

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_skyimage.o : 	$(SRC_SKYIMAGE)controller_skyimage.cpp	\
										$(SRC_SKYIMAGE)controller_skyimage.h	\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYIMAGE)controller_skyimage.cpp -o$(OBJECT_DIR)controller_skyimage.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_imageList.o : 	$(SRC_SKYIMAGE)windowtab_imageList.cpp	\
										$(SRC_SKYIMAGE)windowtab_imageList.h	\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYIMAGE)windowtab_imageList.cpp -o$(OBJECT_DIR)windowtab_imageList.o



##################################################################################
#		IMU source code
##################################################################################
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)imu_lib.o : 				$(SRC_IMU)imu_lib.c			\
										$(SRC_IMU)imu_lib.h			\
										$(SRC_IMU)imu_lib_bno055.h	\
										$(SRC_IMU)getbno055.h
	$(COMPILE) $(INCLUDES) $(SRC_IMU)imu_lib.c -o$(OBJECT_DIR)imu_lib.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)imu_lib_LIS2DH12.o : 		$(SRC_IMU)imu_lib_LIS2DH12.c		\
										$(SRC_IMU)imu_lib_LIS2DH12.h
	$(COMPILE) $(INCLUDES) $(SRC_IMU)imu_lib_LIS2DH12.c -o$(OBJECT_DIR)imu_lib_LIS2DH12.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)imu_lib_bno055.o : 		$(SRC_IMU)imu_lib_bno055.c	\
										$(SRC_IMU)imu_lib_bno055.h	\
										$(SRC_IMU)getbno055.h
	$(COMPILE) $(INCLUDES) $(SRC_IMU)imu_lib_bno055.c -o$(OBJECT_DIR)imu_lib_bno055.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)i2c_bno055.o : 			$(SRC_IMU)i2c_bno055.c	\
										$(SRC_IMU)getbno055.h
	$(COMPILE) $(INCLUDES) $(SRC_IMU)i2c_bno055.c -o$(OBJECT_DIR)i2c_bno055.o

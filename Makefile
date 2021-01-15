######################################################################################
#	Make file for alpaca driver
#	written by hand by Mark Sproul
#	(C) 2019 by Mark Sproul
#
#
#		sudo apt-get install libusb-1.0-0-dev
#		sudo apt-get install libudev-dev
#		sudo apt-get install libopencv-dev
#		sudo apt-get install libi2c-dev
#		sudo apt-get install libjpeg-dev
#
#		sudo apt-get install wiringpi
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
######################################################################################

#PLATFORM			=	x86
PLATFORM			=	x64
#PLATFORM			=	armv7

# default settings for Desktop Linux build
USR_HOME			=	$(HOME)/
GCC_DIR				=	/usr/bin/
INCLUDE_BASE		=	/usr/include/
LIB_BASE			=	/usr/lib/

OPENCV_COMPILE		=	$(shell pkg-config --cflags opencv)
OPENCV_LINK			=	$(shell pkg-config --libs opencv)

#OPENCV_V331			=	-L/usr/local/lib/i386-linux-gnu
#OPENCV_V331			+=	-lopencv_viz

OPENCV_V331			+=	-lopencv_calib3d
OPENCV_V331			+=	-lopencv_core
OPENCV_V331			+=	-lopencv_dnn
OPENCV_V331			+=	-lopencv_features2d
OPENCV_V331			+=	-lopencv_flann
OPENCV_V331			+=	-lopencv_highgui
OPENCV_V331			+=	-lopencv_imgcodecs
OPENCV_V331			+=	-lopencv_imgproc
OPENCV_V331			+=	-lopencv_ml
OPENCV_V331			+=	-lopencv_objdetect
OPENCV_V331			+=	-lopencv_photo
OPENCV_V331			+=	-lopencv_shape
OPENCV_V331			+=	-lopencv_stitching
OPENCV_V331			+=	-lopencv_superres
OPENCV_V331			+=	-lopencv_video
OPENCV_V331			+=	-lopencv_videoio
OPENCV_V331			+=	-lopencv_videostab


SRC_DIR				=	./src/
SRC_IMGPROC			=	./src_imageproc/
SRC_DISCOVERY		=	./src_discovery/
SRC_MOONRISE		=	./src_MoonRise/
SRC_SPECIAL			=	./src_special/
#MLS_LIB_DIR		=	../MLS_Library/
MLS_LIB_DIR			=	./src_mlsLib/
OBJECT_DIR			=	./Objectfiles/

GD_DIR				=	../gd/
ASI_LIB_DIR			=	./ASI_lib
ASI_INCLUDE_DIR		=	./ASI_lib/include
#ASI_LIB_DIR			=	./ASI_lib_V1.14.0227
EFW_LIB_DIR			=	./EFW_linux_mac_SDK

############################################
ATIK_DIR			=	./AtikCamerasSDK
ATIK_LIB_MASTER_DIR	=	$(ATIK_DIR)/lib
ATIK_INCLUDE_DIR	=	$(ATIK_DIR)/inc
ATIK_LIB_DIR		=	$(ATIK_LIB_MASTER_DIR)/linux/x64/NoFlyCapture
ATIK_LIB_DIR_V129	=	$(ATIK_LIB_MASTER_DIR)/ARM/pi/pi3/x86/NoFlyCapture

############################################
TOUP_DIR			=	./toupcamsdk
TOUP_INCLUDE_DIR	=	$(TOUP_DIR)/inc
TOUP_LIB_DIR		=	$(TOUP_DIR)/linux/x64

############################################
FLIR_INCLUDE_DIR	=	/usr/include/spinnaker


############################################
SONY_INCLUDE_DIR	=	./SONY_SDK/CRSDK
SONY_LIB_DIR		=	./SONY_SDK/lib

#DEFINEFLAGS		=	-D_GENERATE_GRAPHICS_
#DEFINEFLAGS		+=	-D_USE_WEB_GRAPH_
DEFINEFLAGS		+=	-D_INCLUDE_HTTP_HEADER_
DEFINEFLAGS		+=	-D_INCLUDE_ALPACA_EXTENSIONS_
DEFINEFLAGS		+=	-D_ALPACA_PI_

CFLAGS			=	-Wall -Wno-multichar -Wno-unknown-pragmas -Wstrict-prototypes
#CFLAGS			+=	-Werror
CFLAGS			+=	-Wmissing-prototypes
#CFLAGS			+=	-trigraphs
CFLAGS			+=	-g
#CFLAGS			+=	-Wno-unused-but-set-variable
#CFLAGS			+=	-Wstrict-prototypes
#CFLAGS			+=	-mx32

CPLUSFLAGS		=	-Wall -Wno-multichar -Wno-unknown-pragmas
CPLUSFLAGS		+=	-O2
#CPLUSFLAGS		+=	-trigraphs
CPLUSFLAGS		+=	-g
#CPLUSFLAGS		+=	-Wno-unused-but-set-variable


COMPILE			=	gcc -c $(CFLAGS) $(DEFINEFLAGS) $(OPENCV_COMPILE)
COMPILEPLUS		=	g++ -c $(CPLUSFLAGS) $(DEFINEFLAGS)
LINK			=	g++


INCLUDES		=	-I$(SRC_DIR)			\
					-I$(MLS_LIB_DIR)		\
					-I$(ASI_INCLUDE_DIR)	\
					-I$(EFW_LIB_DIR)		\
					-I$(ATIK_INCLUDE_DIR)	\
					-I$(TOUP_INCLUDE_DIR)	\
					-I$(FLIR_INCLUDE_DIR)	\
					-I$(SRC_IMGPROC)		\


######################################################################################
ASI_CAMERA_OBJECTS=												\
				$(ASI_LIB_DIR)/lib/$(PLATFORM)/libASICamera2.a	\


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
				$(OBJECT_DIR)alpacadriver.o					\
				$(OBJECT_DIR)alpaca_discovery.o				\
				$(OBJECT_DIR)alpacadriverLogging.o			\
				$(OBJECT_DIR)discoverythread.o				\
				$(OBJECT_DIR)domedriver.o					\
				$(OBJECT_DIR)domeshutter.o					\
				$(OBJECT_DIR)domedriver_rpi.o				\
				$(OBJECT_DIR)eventlogging.o					\
				$(OBJECT_DIR)JsonResponse.o					\
				$(OBJECT_DIR)managementdriver.o				\
				$(OBJECT_DIR)observatory_settings.o			\
				$(OBJECT_DIR)sidereal.o						\
				$(OBJECT_DIR)shutterdriver.o				\
				$(OBJECT_DIR)shutterdriver_arduino.o		\
				$(OBJECT_DIR)serialport.o					\
				$(OBJECT_DIR)telescopedriver.o				\
				$(OBJECT_DIR)telescopedriver_lx200.o		\
				$(OBJECT_DIR)cpu_stats.o					\

######################################################################################
#	Camera Objects
ALPACA_OBJECTS=												\
				$(OBJECT_DIR)cameradriver.o					\
				$(OBJECT_DIR)cameradriverAnalysis.o			\
				$(OBJECT_DIR)cameradriver_fits.o			\
				$(OBJECT_DIR)cameradriver_save.o			\
				$(OBJECT_DIR)cameradriver_opencv.o			\
				$(OBJECT_DIR)cameradriver_jpeg.o			\
				$(OBJECT_DIR)cameradriver_png.o				\
				$(OBJECT_DIR)cameradriver_ASI.o				\
				$(OBJECT_DIR)cameradriver_ATIK.o			\
				$(OBJECT_DIR)cameradriver_TOUP.o			\
				$(OBJECT_DIR)cameradriver_SONY.o			\
				$(OBJECT_DIR)cameradriver_QHY.o				\
				$(OBJECT_DIR)cameradriver_FLIR.o			\
				$(OBJECT_DIR)filterwheeldriver.o			\
				$(OBJECT_DIR)filterwheeldriver_ZWO.o		\
				$(OBJECT_DIR)focuserdriver.o				\
				$(OBJECT_DIR)focuserdriver_nc.o				\
				$(OBJECT_DIR)multicam.o						\
				$(OBJECT_DIR)rotatordriver.o				\
				$(OBJECT_DIR)rotatordriver_nc.o				\
				$(OBJECT_DIR)slittracker.o					\
				$(OBJECT_DIR)switchdriver.o					\
				$(OBJECT_DIR)switchdriver_rpi.o				\
				$(OBJECT_DIR)obsconditionsdriver.o			\
				$(OBJECT_DIR)obsconditionsdriver_rpi.o		\
				$(OBJECT_DIR)moonlite_com.o					\
				$(OBJECT_DIR)commoncolor.o					\
				$(OBJECT_DIR)calibrationdriver.o			\
				$(OBJECT_DIR)calibrationdriver_rpi.o		\
				$(OBJECT_DIR)moonphase.o					\
				$(OBJECT_DIR)MoonRise.o						\
				$(OBJECT_DIR)julianTime.o					\

######################################################################################
#	Camera Objects
IMAGEPROC_OBJECTS=											\
				$(OBJECT_DIR)imageprocess_orb.o				\


CLIENT_OBJECTS=												\
				$(OBJECT_DIR)json_parse.o					\
				$(OBJECT_DIR)discoveryclient.o				\

######################################################################################
# ATIK objects
ATIK_OBJECTS=												\
				$(OBJECT_DIR)camera_atik.o					\


######################################################################################
#	Roll Off Roof Objects
ROR_OBJECTS=												\
				$(OBJECT_DIR)alpacadriver.o					\
				$(OBJECT_DIR)alpaca_discovery.o				\
				$(OBJECT_DIR)alpacadriverLogging.o			\
				$(OBJECT_DIR)cpu_stats.o					\
				$(OBJECT_DIR)discoverythread.o				\
				$(OBJECT_DIR)domedriver.o					\
				$(OBJECT_DIR)domedriver_ror_rpi.o			\
				$(OBJECT_DIR)eventlogging.o					\
				$(OBJECT_DIR)JsonResponse.o					\
				$(OBJECT_DIR)managementdriver.o				\
				$(OBJECT_DIR)observatory_settings.o			\
				$(OBJECT_DIR)raspberrypi_relaylib.o			\


######################################################################################
#pragma mark make cpp  C++ linux-x86
cpp		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#cpp	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#cpp	:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#cpp		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#cpp			:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#cpp		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
cpp		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#cpp	:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#cpp		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#cpp		:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
#cpp	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#cpp		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
cpp		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#cpp		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#cpp		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_LX200_
cpp		:			$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
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


######################################################################################
#pragma mark make tele  C++ linux-x86
tele		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_ROR_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#tele		:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#tele	:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
#tele	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
tele	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_LX200_
tele	:			$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-o alpacapi-telescope


######################################################################################
#pragma mark make toup
toup	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
toup	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
toup	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#toup		:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
toup	:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
toup	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#toup	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
toup	:			$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
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
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#touppi		:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
touppi	:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
touppi	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#touppi	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
touppi	:			$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(TOUP_LIB_DIR)/			\
					-ltoupcam					\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-o alpacapi



######################################################################################
#pragma mark C++ linux-x86
Release		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
Release		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
Release		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#Release		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
Release		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
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
#flir			:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
#flir		:		DEFINEFLAGS		+=	-D_ENABLE_QHY_
flir		:		DEFINEFLAGS		+=	-D_ENABLE_FLIR_
flir		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
flir		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					-L$(TOUP_LIB_DIR)/			\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-lSpinnaker_C				\
					-latikcameras				\
					-ltoupcam					\
					-ludev						\
					-lusb-1.0					\
					-lpthread					\
					-lcfitsio					\
					-lqhyccd					\
					-o alpacapi


######################################################################################
#pragma mark nousb -C++ linux-x86
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#nousb		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
nousb		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
nousb		:		DEFINEFLAGS		+=	-D_ENABLE_SHUTTER_
nousb		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					-lpthread					\
					-lcfitsio					\
					-o alpacapi

######################################################################################
#pragma mark dome
#dome		:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
dome		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
dome		:	DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_REMOTE_
dome		:	DEFINEFLAGS		+=	-D_ENABLE_REMOTE_SHUTTER_
dome		:	DEFINEFLAGS		+=	-D_INCLUDE_WIRINGPI_
#dome		:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#dome		:	DEFINEFLAGS		+=	-D_ENABLE_ASI_
dome		:	PLATFORM		=	armv7
dome		:				$(CPP_OBJECTS)				\
							$(SOCKET_OBJECTS)			\

				$(LINK)  								\
							$(CPP_OBJECTS)				\
							$(SOCKET_OBJECTS)			\
							-lusb-1.0					\
							-lpthread					\
							-lwiringPi					\
							-o domecontroller

#							-ludev						\
#							$(ASI_CAMERA_OBJECTS)		\


######################################################################################
#pragma mark ROR
#ror		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
ror		:	DEFINEFLAGS		+=	-D_ENABLE_ROR_
ror		:					$(ROR_OBJECTS)				\
							$(SOCKET_OBJECTS)			\

				$(LINK)  								\
							$(SOCKET_OBJECTS)			\
							$(ROR_OBJECTS)				\
							-lpthread					\
							-o ror

######################################################################################
#pragma mark rorpi
#rorpi		:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
rorpi		:	DEFINEFLAGS		+=	-D_ENABLE_ROR_
rorpi		:	DEFINEFLAGS		+=	-D_ENABLE_4REALY_BOARD
rorpi		:				$(ROR_OBJECTS)				\
							$(SOCKET_OBJECTS)			\

				$(LINK)  								\
							$(SOCKET_OBJECTS)			\
							$(ROR_OBJECTS)				\
							-lpthread					\
							-lwiringPi					\
							-o ror


######################################################################################
#pragma mark make pi
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
pi		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
pi		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
pi		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#pi		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
pi		:		PLATFORM		=	armv7
pi		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
pi		:			$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					-L$(ATIK_LIB_DIR_V129)/		\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-latikcameras				\
					-lcfitsio					\
					-ltoupcam					\
					-lusb-1.0					\
					-ludev						\
					-lwiringPi					\
					-lpthread					\
					-o alpacapi


######################################################################################
#pragma mark Raspberry pi - calibration
#make calib
calib		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
calib		:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
calib		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
#calib		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
#calib		:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
calib		:		PLATFORM		=	armv7
calib		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
calib		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					-lwiringPi					\
					-lpthread					\
					-o alpacapi-calib

#					$(OPENCV_LINK)				\
#					$(ASI_CAMERA_OBJECTS)		\
#					-lcfitsio					\
#					-lusb-1.0					\
#					-ludev						\


######################################################################################
#pragma mark Raspberry pi - switch
#make piswitch4
piswitch4	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_CALIBRATION_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_4REALY_BOARD
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
#piswitch4	:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
piswitch4	:		DEFINEFLAGS		+=	-D_ENABLE_WIRING_PI_
piswitch4	:		PLATFORM		=	armv7
piswitch4	:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
piswitch4	:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					-lwiringPi					\
					-lpthread					\
					-o alpacapi


######################################################################################
#	Camera Objects
SONY_OBJECTS=												\
				$(OBJECT_DIR)cameradriver_SONY.o			\

######################################################################################
sony		:		DEFINEFLAGS		+=	-D_ENABLE_SONY_
sony		:		DEFINEFLAGS		+=	-D_INCLUDE_SONY_MAIN_
sony		:		CPLUSFLAGS		+=	-fsigned-char
sony		:		CPLUSFLAGS		+=	-std=gnu++17
sony		:		CPLUSFLAGS		+=	-O3 -DNDEBUG -std=c++17
sony		:		PLATFORM		=	armv8
sony		:		INCLUDES		+=	-I$(SONY_INCLUDE_DIR)
sony		:		$(SONY_OBJECTS)

		$(LINK)  								\
					$(SONY_OBJECTS)				\
					-L$(SONY_LIB_DIR)			\
					-lCr_Core					\
					-o sony

######################################################################################
#pragma mark make pi64
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
pi64		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
pi64		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
pi64		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
#pi64		:		DEFINEFLAGS		+=	-D_ENABLE_SONY_
pi64		:		DEFINEFLAGS		+=	-D_INCLUDE_EXIT_COMMAND_
pi64		:		CPLUSFLAGS		+=	-fsigned-char
pi64		:		CPLUSFLAGS		+=	-std=gnu++17
pi64		:		PLATFORM		=	armv8
pi64		:		INCLUDES		+=	-I$(SONY_INCLUDE_DIR)
#pi64		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
pi64		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/odroid/x86/NoFlyCapture
pi64		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(SONY_LIB_DIR)/			\
					$(ASI_CAMERA_OBJECTS)		\
					-L$(ATIK_LIB_DIR)/			\
					-latikcameras				\
					-lCr_Core					\
					-lcfitsio					\
					-lusb-1.0					\
					-ludev						\
					-lpthread					\
					-o alpacapi

#					$(ZWO_EFW_OBJECTS)			\
#					-ltoupcam					\
#					-lwiringPi					\

######################################################################################
#pragma mark C++ Raspberry pi zwo only
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
pizwo		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#pizwo		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
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
pizwo		:		PLATFORM		=	armv7
#pizwo		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
pizwo		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					-lusb-1.0					\
					-ludev						\
					-lpthread					\
					-o alpacapi

######################################################################################
#pragma mark Management only
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
manag		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
manag		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
#manag		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
#manag		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
manag		:		PLATFORM		=	armv7
#manag		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
manag		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					-lusb-1.0					\
					-ludev						\
					-lpthread					\
					-o alpacapi

######################################################################################
#pragma mark Newt 16 C++ Raspberry pi
newt16		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
newt16		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
#newt16		:		DEFINEFLAGS		+=	-D_ENABLE_TOUP_
newt16		:		DEFINEFLAGS		+=	-D_ENABLE_SLIT_TRACKER_
newt16		:		PLATFORM		=	armv7
newt16		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture
newt16		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					-L$(ATIK_LIB_DIR_V129)/		\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-latikcameras				\
					-lcfitsio					\
					-ltoupcam					\
					-lusb-1.0					\
					-ludev						\
					-ljpeg						\
					-lpthread					\
					-o alpacapi

#					-lwiringPi					\

######################################################################################
#pragma mark ZWO
zwo		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#zwo		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
zwo		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
#zwo		:		PLATFORM		=	armv7
zwo		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
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
#piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
piswitch		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
piswitch		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
piswitch		:		DEFINEFLAGS		+=	-D_INCLUDE_WIRINGPI_
piswitch		:		DEFINEFLAGS		+=	-D_ENABLE_PWM_SWITCH_
piswitch		:		PLATFORM		=	armv7
piswitch		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture

piswitch		:	$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					-L$(ATIK_LIB_DIR_V129)/		\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-latikcameras				\
					-lcfitsio					\
					-lusb-1.0					\
					-ludev						\
					-lwiringPi					\
					-lpthread					\
					-o alpacapi


######################################################################################
#pragma mark Switch - C++ Raspberry pi64
piswitch64		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
piswitch64		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
piswitch64		:		DEFINEFLAGS		+=	-D_INCLUDE_WIRINGPI_
#piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_PWM_SWITCH_
piswitch64		:		DEFINEFLAGS		+=	-D_ENABLE_4REALY_BOARD
piswitch64		:		CPLUSFLAGS		+=	-std=gnu++17
piswitch64		:		PLATFORM		=	armv8

piswitch64		:	$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					$(ASI_CAMERA_OBJECTS)		\
					-lcfitsio					\
					-lusb-1.0					\
					-ludev						\
					-lwiringPi					\
					-lpthread					\
					-o alpacapi


######################################################################################
#pragma mark shutter
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
#shutter	:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_FITS_
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_MULTICAM_
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_DOME_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#shutter	:		DEFINEFLAGS		+=	-D_ENABLE_ATIK_
shutter		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_SHUTTER_
shutter		:		DEFINEFLAGS		+=	-D_ENABLE_PWM_SWITCH_
shutter		:		DEFINEFLAGS		+=	-D_INCLUDE_WIRINGPI_
shutter		:		PLATFORM		=	armv7
shutter		:		ATIK_LIB_DIR	=	$(ATIK_LIB_MASTER_DIR)/ARM/x86/NoFlyCapture

shutter		:		$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(SOCKET_OBJECTS)			\


		$(LINK)  								\
					$(SOCKET_OBJECTS)			\
					$(CPP_OBJECTS)				\
					$(ALPACA_OBJECTS)			\
					$(OPENCV_LINK)				\
					-L$(ATIK_LIB_DIR)/			\
					-L$(ATIK_LIB_DIR_V129)/		\
					$(ASI_CAMERA_OBJECTS)		\
					$(ZWO_EFW_OBJECTS)			\
					-lcfitsio					\
					-lusb-1.0					\
					-ludev						\
					-lwiringPi					\
					-lpthread					\
					-o alpacapi

#					-latikcameras				\


######################################################################################
#pragma mark linux-x86 - No opencv
#noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
#noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_ROTATOR_
#noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_SAFETYMONITOR_
#noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_SWITCH_
#noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_ASI_
#noopencv		:		DEFINEFLAGS		+=	-D_USE_OPENCV_
noopencv		:		DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
noopencv		:		$(ALPACA_OBJECTS)			\
						$(SOCKET_OBJECTS)			\
						$(CPP_OBJECTS)				\

		$(LINK)  									\
						$(ALPACA_OBJECTS)			\
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
#Debug		:		DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
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
Debug			:		$(CPP_OBJECTS)				\
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
#pragma mark jetson
jetson		:	DEFINEFLAGS		+=	-D_JETSON_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
jetson		:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
#jetson		:	DEFINEFLAGS		+=	-D_ENABLE_JPEGLIB_
#jetson		:	DEFINEFLAGS		+=	-D_ENABLE_ASI_
#jetson		:	DEFINEFLAGS		+=	-D_ENABLE_TOUP_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_FLIR_
#jetson		:	DEFINEFLAGS		+=	-D_ENABLE_FITS_
jetson		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
jetson		:	DEFINEFLAGS		+=	-D_USE_OPENCV_
jetson		:	DEFINEFLAGS		+=	-D_PLATFORM_STRING_=\"Nvidia-jetson\"

jetson		:	PLATFORM		=	armv8
jetson		:	TOUP_LIB_DIR	=	$(TOUP_DIR)/linux/arm64
jetson		:				$(ALPACA_OBJECTS)			\
							$(SOCKET_OBJECTS)			\
							$(CPP_OBJECTS)				\
							$(IMAGEPROC_OBJECTS)		\


				$(LINK)  								\
							$(ALPACA_OBJECTS)			\
							$(SOCKET_OBJECTS)			\
							$(CPP_OBJECTS)				\
							$(IMAGEPROC_OBJECTS)		\
							$(OPENCV_LINK)				\
							-lcfitsio					\
							-lpthread					\
							-lSpinnaker_C				\
							-o alpacapi

#							$(ASI_CAMERA_OBJECTS)		\
#							$(ZWO_EFW_OBJECTS)			\
#							-lusb-1.0					\
#							-ltoupcam					\
#							-ljpeg						\
#							-lSpinnaker					\

#							-ludev						\


######################################################################################
#pragma mark wx
#wx			:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
#wx			:	DEFINEFLAGS		+=	-D_ENABLE_DOME_
#wx			:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
wx			:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
wx			:	DEFINEFLAGS		+=	-D_ENABLE_PI_HAT_SESNSOR_BOARD_
wx			:	PLATFORM		=	armv7
wx			:				$(ALPACA_OBJECTS)			\
							$(SOCKET_OBJECTS)			\
							$(CPP_OBJECTS)				\

				$(LINK)  								\
							$(ALPACA_OBJECTS)			\
							$(CPP_OBJECTS)				\
							$(SOCKET_OBJECTS)			\
							$(ASI_CAMERA_OBJECTS)		\
							$(ZWO_EFW_OBJECTS)			\
							$(OPENCV_LINK)				\
							-lRTIMULib					\
							-lusb-1.0					\
							-lpthread					\
							-lwiringPi					\
							-o alpacapi


######################################################################################
#pragma mark smate
smate		:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
smate		:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
smate		:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
smate		:	DEFINEFLAGS		+=	-D_ENABLE_OBSERVINGCONDITIONS_
smate		:	DEFINEFLAGS		+=	-D_ENABLE_TELESCOPE_
smate		:	PLATFORM		=	armv7
smate		:				$(ALPACA_OBJECTS)			\
							$(SOCKET_OBJECTS)			\

				$(LINK)  								\
							$(ALPACA_OBJECTS)			\
							$(SOCKET_OBJECTS)			\
							$(ASI_CAMERA_OBJECTS)		\
							$(ZWO_EFW_OBJECTS)			\
							$(OPENCV_LINK)				\
							-lusb-1.0					\
							-lpthread					\
							-ludev						\
							-o alpacapi




######################################################################################
#pragma mark ATIK Linux
atik	:	ATIK_LIB_DIR		=	$(ATIK_LIB_MASTER_DIR)/linux/x64/NoFlyCapture
atik	:	DEFINEFLAGS			+=	-D_ENABLE_ATIK_
atik	:					$(ATIK_OBJECTS)

				$(LINK)  										\
							$(ATIK_OBJECTS)						\
							-L$(ATIK_LIB_DIR)/					\
							-latikcameras						\
							-o atik




######################################################################################
#pragama mark client
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
CONTROLLER_BASE_OBJECTS=										\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_image.o					\
				$(OBJECT_DIR)discoverythread.o					\

#				$(OBJECT_DIR)controller_image.o					\

######################################################################################
CONTROLLER_OBJECTS=												\
				$(OBJECT_DIR)controller_main.o					\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)controller_focus.o					\
				$(OBJECT_DIR)controller_focus_generic.o			\
				$(OBJECT_DIR)controller_switch.o				\
				$(OBJECT_DIR)controller_camera.o				\
				$(OBJECT_DIR)controller_cam_normal.o			\
				$(OBJECT_DIR)controller_dome.o					\
				$(OBJECT_DIR)controller_dome_common.o			\
				$(OBJECT_DIR)controller_image.o					\
				$(OBJECT_DIR)controller_ml_nc.o					\
				$(OBJECT_DIR)controller_ml_single.o				\
				$(OBJECT_DIR)controller_usb.o					\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_auxmotor.o				\
				$(OBJECT_DIR)windowtab_camera.o					\
				$(OBJECT_DIR)windowtab_camsettings.o			\
				$(OBJECT_DIR)windowtab_config.o					\
				$(OBJECT_DIR)windowtab_dome.o					\
				$(OBJECT_DIR)windowtab_image.o					\
				$(OBJECT_DIR)windowtab_ml_single.o				\
				$(OBJECT_DIR)windowtab_nitecrawler.o			\
				$(OBJECT_DIR)windowtab_filelist.o				\
				$(OBJECT_DIR)windowtab_graphs.o					\
				$(OBJECT_DIR)windowtab_switch.o					\
				$(OBJECT_DIR)windowtab_slit.o					\
				$(OBJECT_DIR)windowtab_slitgraph.o				\
				$(OBJECT_DIR)windowtab_usb.o					\
				$(OBJECT_DIR)moonlite_com.o						\
				$(OBJECT_DIR)nitecrawler_image.o				\
				$(OBJECT_DIR)discovery_lib.o					\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)serialport.o						\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\

VIDEO_OBJECTS=													\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)controller_camera.o				\
				$(OBJECT_DIR)controller_video.o					\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_video.o					\
				$(OBJECT_DIR)windowtab_preview.o				\
				$(OBJECT_DIR)discovery_lib.o					\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\
				$(OBJECT_DIR)controller_preview.o				\


PREVIEW_OBJECTS=												\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)controller_preview.o				\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_preview.o				\
				$(OBJECT_DIR)discovery_lib.o					\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\



######################################################################################
# SkyTravel objects

SRC_SKYTRAVEL=./src_skytravel/
SKYTRAVEL_OBJECTS=											\
				$(OBJECT_DIR)controller_skytravel.o			\
				$(OBJECT_DIR)controller_dome_common.o		\
				$(OBJECT_DIR)eph.o							\
				$(OBJECT_DIR)skytravel_main.o				\
				$(OBJECT_DIR)windowtab_skytravel.o			\
				$(OBJECT_DIR)StarData.o						\
				$(OBJECT_DIR)SkyTravelTimeRoutines.o		\
				$(OBJECT_DIR)NGCcatalog.o					\
				$(OBJECT_DIR)StarCatalogHelper.o			\
				$(OBJECT_DIR)YaleStarCatalog.o				\
				$(OBJECT_DIR)HipparcosCatalog.o				\
				$(OBJECT_DIR)ConstellationData.o			\
				$(OBJECT_DIR)lx200_com.o					\
				$(OBJECT_DIR)windowtab_dome.o				\
				$(OBJECT_DIR)windowtab_alpacalist.o			\



######################################################################################
#pragma mark mandelbrot
mandelbrot	:	DEFINEFLAGS		+=	-D_INCLUDE_MAIN_
mandelbrot	:			$(MANDELBROT_OBJECTS)

				$(LINK)  										\
							$(MANDELBROT_OBJECTS)				\
							$(OPENCV_LINK)						\
							-o mandlebrot;


######################################################################################
#pragma mark focuser-controller
focuser		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
focuser		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
#focuser		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
focuser		:	DEFINEFLAGS		+=	-D_ENABLE_USB_FOCUSERS_

focuser		:			$(CONTROLLER_OBJECTS)

				$(LINK)  										\
							$(CONTROLLER_OBJECTS)				\
							$(OPENCV_LINK)						\
							-o focuser

######################################################################################
#make switch
#pragma mark focuser-controller
switch		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
#switch		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_FOCUSERS_
switch		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_SWITCHES_
#switch		:	DEFINEFLAGS		+=	-D_ENABLE_USB_FOCUSERS_

switch		:			$(CONTROLLER_OBJECTS)

				$(LINK)  										\
							$(CONTROLLER_OBJECTS)				\
							$(OPENCV_LINK)						\
							-o switch


######################################################################################
#make camera
#pragma mark camera-controller
camera		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
camera		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
camera		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
camera		:			$(CONTROLLER_OBJECTS)					\

				$(LINK)  										\
							$(CONTROLLER_OBJECTS)				\
							$(OPENCV_LINK)						\
							-o camera

######################################################################################
#make skytravel
#pragma mark camera-controller
sky		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_SKYTRAVEL_
sky		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_
sky		:	INCLUDES		+=	-I$(SRC_SKYTRAVEL)

sky		:				$(SKYTRAVEL_OBJECTS)					\
						$(CONTROLLER_BASE_OBJECTS)				\


				$(LINK)  										\
							$(SKYTRAVEL_OBJECTS)				\
							$(CONTROLLER_BASE_OBJECTS)			\
							$(OPENCV_LINK)						\
							-lpthread							\
							-o skytravel

#							$(CONTROLLER_OBJECTS)				\

######################################################################################
#pragma mark camera-controller
domectrl		:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
domectrl		:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_DOME_

domectrl		:			$(CONTROLLER_OBJECTS)

				$(LINK)  										\
							$(CONTROLLER_OBJECTS)				\
							$(OPENCV_LINK)						\
							-o domectrl

######################################################################################
NETTEST_OBJECTS=												\
				$(OBJECT_DIR)controller_main.o					\
				$(OBJECT_DIR)controller.o						\
				$(OBJECT_DIR)controller_nettest.o				\
				$(OBJECT_DIR)controllerAlpaca.o					\
				$(OBJECT_DIR)windowtab.o						\
				$(OBJECT_DIR)windowtab_about.o					\
				$(OBJECT_DIR)windowtab_nettest.o				\
				$(OBJECT_DIR)discovery_lib.o					\
				$(OBJECT_DIR)json_parse.o						\
				$(OBJECT_DIR)commoncolor.o						\
				$(OBJECT_DIR)sendrequest_lib.o					\


######################################################################################
SRC_NETTEST=./src_nettest/

#pragma mark camera-controller
net		:		DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
net		:		DEFINEFLAGS		+=	-D_ENABLE_NET_TEST_
net		:		DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
net		:		INCLUDES		+=	-I$(SRC_NETTEST)

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
										$(SRC_NETTEST)windowtab_nettest.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_NETTEST)windowtab_nettest.cpp -o$(OBJECT_DIR)windowtab_nettest.o


######################################################################################
#pragma mark video-controller
video			:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
video			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_VIDEO_
video			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_CAMERA_
video			:	DEFINEFLAGS		+=	-D_VIDEO_CONTROLLER_WORK_


video			:			$(VIDEO_OBJECTS)

				$(LINK)  										\
							$(VIDEO_OBJECTS)					\
							$(OPENCV_LINK)						\
							-o video


######################################################################################
#pragma mark preview-controller
preview			:	DEFINEFLAGS		+=	-D_INCLUDE_CTRL_MAIN_
preview			:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_PREVIEW_

preview			:			$(PREVIEW_OBJECTS)

				$(LINK)  										\
							$(PREVIEW_OBJECTS)					\
							$(OPENCV_LINK)						\
							-o preview


######################################################################################
#pragma mark clean
clean:
	rm -vf $(OBJECT_DIR)*.o



######################################################################################
#pragma mark help
help:
	#    make options
	#        alpaca (default)
	#        dome       Raspberry pi version to control dome using DC motor controller
	#        jetson     Version to run on nvidia jetson board, this is an armv8
	#        nocamera   Build without the camera support
	#        smate      Build a version to run on a Stellarmate running smate OS
	#        pi         Version for Raspberry Pi
	#        wx         Version that uses
	#        noopencv   Dont include opencv
	#        clean		removes all binaries
	#        help		this message

#	Debug                     Makefile


######################################################################################
$(OBJECT_DIR)socket_listen.o : $(SRC_DIR)socket_listen.c $(SRC_DIR)socket_listen.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)socket_listen.c -o$(OBJECT_DIR)socket_listen.o

$(OBJECT_DIR)JsonResponse.o : $(SRC_DIR)JsonResponse.c $(SRC_DIR)JsonResponse.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)JsonResponse.c -o$(OBJECT_DIR)JsonResponse.o


$(OBJECT_DIR)eventlogging.o : $(SRC_DIR)eventlogging.c $(SRC_DIR)eventlogging.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)eventlogging.c -o$(OBJECT_DIR)eventlogging.o

######################################################################################
# CPP objects
$(OBJECT_DIR)alpacadriver.o :			$(SRC_DIR)alpacadriver.cpp			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpacadriver.cpp -o$(OBJECT_DIR)alpacadriver.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)alpaca_discovery.o :		$(SRC_DIR)alpaca_discovery.cpp		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)alpaca_discovery.cpp -o$(OBJECT_DIR)alpaca_discovery.o


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
$(OBJECT_DIR)cameradriver_QHY.o :		$(SRC_DIR)cameradriver_QHY.cpp		\
										$(SRC_DIR)cameradriver_QHY.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_QHY.cpp -o$(OBJECT_DIR)cameradriver_QHY.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_FLIR.o :		$(SRC_DIR)cameradriver_FLIR.cpp		\
										$(SRC_DIR)cameradriver_FLIR.h		\
										$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_FLIR.cpp -o$(OBJECT_DIR)cameradriver_FLIR.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)cameradriver_save.o :		$(SRC_DIR)cameradriver_save.cpp		\
									 	$(SRC_DIR)cameradriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)cameradriver_save.cpp -o$(OBJECT_DIR)cameradriver_save.o

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
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)focuserdriver_nc.cpp -o$(OBJECT_DIR)focuserdriver_nc.o
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
$(OBJECT_DIR)telescopedriver_lx200.o :	$(SRC_DIR)telescopedriver_lx200.cpp	\
										$(SRC_DIR)telescopedriver_lx200.h	\
										$(SRC_DIR)telescopedriver.h			\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)telescopedriver_lx200.cpp -o$(OBJECT_DIR)telescopedriver_lx200.o

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
										$(SRC_DIR)switchdriver_rpi.h		\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)switchdriver_rpi.cpp -o$(OBJECT_DIR)switchdriver_rpi.o
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)obsconditionsdriver.o :	$(SRC_DIR)obsconditionsdriver.cpp	\
										$(SRC_DIR)obsconditionsdriver.h	 	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)obsconditionsdriver.cpp -o$(OBJECT_DIR)obsconditionsdriver.o
#-------------------------------------------------------------------------------------
$(OBJECT_DIR)obsconditionsdriver_rpi.o :	$(SRC_DIR)obsconditionsdriver_rpi.cpp 	\
											$(SRC_DIR)obsconditionsdriver.h			\
											$(SRC_DIR)alpacadriver.h			\
											Makefile
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)obsconditionsdriver_rpi.cpp -o$(OBJECT_DIR)obsconditionsdriver_rpi.o

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
$(OBJECT_DIR)discoverythread.o :		$(SRC_DIR)discoverythread.c 	\
										$(SRC_DIR)discoverythread.h 	\
										$(SRC_DIR)alpacadriver.h			\
										Makefile
	$(COMPILEPLUS) $(INCLUDES)			$(SRC_DIR)discoverythread.c -o$(OBJECT_DIR)discoverythread.o

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


######################################################################################
# ATIK objects
$(OBJECT_DIR)camera_atik.o : $(SRC_DIR)camera_atik.c $(SRC_DIR)camera_atik.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)camera_atik.c -o$(OBJECT_DIR)camera_atik.o



######################################################################################
#	CLIENT_OBJECTS=
$(OBJECT_DIR)json_parse.o : $(MLS_LIB_DIR)json_parse.c $(MLS_LIB_DIR)json_parse.h
	$(COMPILE) $(INCLUDES) $(MLS_LIB_DIR)json_parse.c -o$(OBJECT_DIR)json_parse.o

$(OBJECT_DIR)discoveryclient.o : $(SRC_DISCOVERY)discoveryclient.c $(SRC_DISCOVERY)discoveryclient.h
	$(COMPILE) $(INCLUDES) $(SRC_DISCOVERY)discoveryclient.c -o$(OBJECT_DIR)discoveryclient.o




######################################################################################
$(OBJECT_DIR)mandelbrot.o : $(SRC_DIR)mandelbrot.c
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)mandelbrot.c -o$(OBJECT_DIR)mandelbrot.o



######################################################################################
$(OBJECT_DIR)controller.o : $(SRC_DIR)controller.cpp $(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller.cpp -o$(OBJECT_DIR)controller.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controllerAlpaca.o : 		$(SRC_DIR)controllerAlpaca.cpp		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controllerAlpaca.cpp -o$(OBJECT_DIR)controllerAlpaca.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_focus.o : 		$(SRC_DIR)controller_focus.cpp		\
										$(SRC_DIR)controller_focus.h		\
										$(SRC_DIR)controller.h				\
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
$(OBJECT_DIR)controller_ml_nc.o : 		$(SRC_DIR)controller_ml_nc.cpp		\
										$(SRC_DIR)controller_ml_nc.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)windowtab_nitecrawler.h	\
										$(SRC_DIR)windowtab_graphs.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_ml_nc.cpp -o$(OBJECT_DIR)controller_ml_nc.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_ml_single.o : 	$(SRC_DIR)controller_ml_single.cpp	\
										$(SRC_DIR)controller_ml_single.h	\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_ml_single.cpp -o$(OBJECT_DIR)controller_ml_single.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_switch.o : 		$(SRC_DIR)controller_switch.cpp		\
										$(SRC_DIR)controller_switch.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_switch.cpp -o$(OBJECT_DIR)controller_switch.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_camera.o : 		$(SRC_DIR)controller_camera.cpp		\
										$(SRC_DIR)controller_camera.h		\
										$(SRC_DIR)windowtab_camera.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_camera.cpp -o$(OBJECT_DIR)controller_camera.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_cam_normal.o : 	$(SRC_DIR)controller_cam_normal.cpp		\
										$(SRC_DIR)controller_cam_normal.h		\
										$(SRC_DIR)windowtab_camera.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_cam_normal.cpp -o$(OBJECT_DIR)controller_cam_normal.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_dome.o : 		$(SRC_DIR)controller_dome.cpp		\
										$(SRC_DIR)controller_dome.h			\
										$(SRC_DIR)windowtab_dome.h			\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_dome.cpp -o$(OBJECT_DIR)controller_dome.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_dome_common.o : $(SRC_DIR)controller_dome_common.cpp	\
										$(SRC_DIR)controller_dome.h				\
										$(SRC_DIR)windowtab_dome.h				\
										$(SRC_DIR)windowtab_about.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_dome_common.cpp -o$(OBJECT_DIR)controller_dome_common.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_alpacalist.o : $(SRC_DIR)windowtab_alpacalist.cpp		\
										$(SRC_DIR)windowtab_alpacalist.h		\
										$(SRC_DIR)windowtab.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_alpacalist.cpp -o$(OBJECT_DIR)windowtab_alpacalist.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_image.o : 		$(SRC_DIR)controller_image.cpp		\
										$(SRC_DIR)controller_image.h		\
										$(SRC_DIR)windowtab_image.h			\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_image.cpp -o$(OBJECT_DIR)controller_image.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_image.o : 		$(SRC_DIR)windowtab_image.cpp		\
										$(SRC_DIR)windowtab_image.h			\
										$(SRC_DIR)controller_image.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_image.cpp -o$(OBJECT_DIR)windowtab_image.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_usb.o : 		$(SRC_DIR)controller_usb.cpp		\
										$(SRC_DIR)controller_usb.h			\
										$(SRC_DIR)windowtab_usb.h			\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_usb.cpp -o$(OBJECT_DIR)controller_usb.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_video.o : 		$(SRC_SPECIAL)controller_video.cpp	\
										$(SRC_SPECIAL)controller_video.h	\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SPECIAL)controller_video.cpp -o$(OBJECT_DIR)controller_video.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)controller_preview.o : 	$(SRC_DIR)controller_preview.cpp	\
										$(SRC_DIR)controller_preview.h		\
										$(SRC_DIR)windowtab_about.h			\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)controller_preview.cpp -o$(OBJECT_DIR)controller_preview.o


#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab.o : 				$(SRC_DIR)windowtab.cpp				\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)windowtab_ml_single.h		\
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
$(OBJECT_DIR)windowtab_filelist.o : 	$(SRC_DIR)windowtab_filelist.cpp	\
										$(SRC_DIR)controller_camera.h		\
										$(SRC_DIR)windowtab_filelist.h		\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_filelist.cpp -o$(OBJECT_DIR)windowtab_filelist.o



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
$(OBJECT_DIR)windowtab_preview.o : 		$(SRC_DIR)windowtab_preview.cpp		\
										$(SRC_DIR)windowtab_preview.h			\
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
$(OBJECT_DIR)windowtab_dome.o : 		$(SRC_DIR)windowtab_dome.cpp		\
										$(SRC_DIR)windowtab_dome.h			\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_DIR)windowtab_dome.cpp -o$(OBJECT_DIR)windowtab_dome.o



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
$(OBJECT_DIR)windowtab_slitgraph.o : 	$(SRC_DIR)windowtab_slitgraph.cpp		\
										$(SRC_DIR)windowtab_slitgraph.h			\
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
$(OBJECT_DIR)windowtab_video.o : 		$(SRC_SPECIAL)windowtab_video.cpp	\
										$(SRC_SPECIAL)windowtab_video.h		\
										$(SRC_DIR)windowtab.h				\
										$(SRC_DIR)controller.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SPECIAL)windowtab_video.cpp -o$(OBJECT_DIR)windowtab_video.o



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
$(OBJECT_DIR)discovery_lib.o :			$(SRC_DIR)discovery_lib.c			\
										$(SRC_DIR)discovery_lib.h
	$(COMPILE) $(INCLUDES) $(SRC_DIR)discovery_lib.c -o$(OBJECT_DIR)discovery_lib.o

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
$(OBJECT_DIR)controller_skytravel.o :	$(SRC_SKYTRAVEL)controller_skytravel.cpp	\
										$(SRC_SKYTRAVEL)controller_skytravel.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)controller_skytravel.cpp -o$(OBJECT_DIR)controller_skytravel.o



#-------------------------------------------------------------------------------------
$(OBJECT_DIR)StarData.o :				$(SRC_SKYTRAVEL)StarData.c	\
										$(SRC_SKYTRAVEL)StarData.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)StarData.c -o$(OBJECT_DIR)StarData.o

#-------------------------------------------------------------------------------------
$(OBJECT_DIR)windowtab_skytravel.o :	$(SRC_SKYTRAVEL)windowtab_skytravel.cpp	\
										$(SRC_SKYTRAVEL)windowtab_skytravel.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)windowtab_skytravel.cpp -o$(OBJECT_DIR)windowtab_skytravel.o


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
$(OBJECT_DIR)lx200_com.o :				$(SRC_SKYTRAVEL)lx200_com.c	\
										$(SRC_SKYTRAVEL)lx200_com.h
	$(COMPILEPLUS) $(INCLUDES) $(SRC_SKYTRAVEL)lx200_com.c -o$(OBJECT_DIR)lx200_com.o



############################################################################
#
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
############################################################################
#++	May  9,	2022	<MLS> Updated to use Objectfiles directory for .o files
#++	May 15,	2022	<RNS> Added servo_test_tty and fixed typos
#++	May 19,	2022	<RNS> Change all refs of 'scope' to 'mount', including filenames
#++	May 19,	2022	<RNS> Added support for rc_utils unit test
#++	May 19,	2022	<RNS> Fixed all unit tests for *.c files with unit _TEST_
#++	May 27,	2022	<RNS> Added support for servo_time unit test
############################################################################

CC			=	gcc -I$(MLS_LIB_DIR)

CFLAGS 		=	-Wall
RM 			=	/bin/rm -v -f
OBJECT_DIR	=	./Objectfiles/
MLS_LIB_DIR	=	../src_mlsLib/

# typing 'make' will invoke the first target entry in the file
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: servo_test_tty

SERVO_OBJECTS	=								\
				$(OBJECT_DIR)servo_mount.o		\
				$(OBJECT_DIR)servo_mount_cfg.o	\
				$(OBJECT_DIR)servo_time.o		\
				$(OBJECT_DIR)servo_mc_core.o	\
				$(OBJECT_DIR)servo_rc_utils.o

TESTCFG_OBJECTS	=								\
				$(OBJECT_DIR)servo_mount_cfg.o 	\
				$(OBJECT_DIR)servo_time.o

RCUTILS_OBJECTS	=								\
				$(OBJECT_DIR)servo_rc_utils.o 	\
				$(OBJECT_DIR)servo_mc_core.o

servo_test_tty:	CFLAGS	+=
servo_test_tty: 	$(SERVO_OBJECTS)	$(OBJECT_DIR)servo_test_tty.o

	$(CC) $(CFLAGS) -o servo_test_tty				\
					$(SERVO_OBJECTS)				\
					$(OBJECT_DIR)servo_test_tty.o	\
					-lm

servo_test: 	CFLAGS	+=	-D_TEST_SERVO_MOUNT_
servo_test: 	$(SERVO_OBJECTS)
	$(CC) $(CFLAGS) -o servo_test 				\
					$(SERVO_OBJECTS)			\
					-lm


test_cfg: 	CFLAGS	+=	-D_TEST_SERVO_MOUNT_CFG_
test_cfg: 	$(TESTCFG_OBJECTS)
	$(CC) $(CFLAGS) -o test_cfg 				\
					$(TESTCFG_OBJECTS)			\
					-lm

rc_utils: 	CFLAGS	+=	-D_TEST_RC_UTILS_
rc_utils: 	$(RCUTILS_OBJECTS)
	$(CC) $(CFLAGS) -o rc_utils				\
					$(RCUTILS_OBJECTS)			\
					-lm

test_time:  CFLAGS  +=  -D_TEST_SERVO_TIME_
test_time:  servo_time.c
	$(CC) $(CFLAGS) -Wextra -o test_time			\
 					servo_time.c					\
 					-lm

mc_core: 	CFLAGS	+=	-D_TEST_SERVO_MC_CORE_
mc_core: 	servo_mc_core.c
	$(CC) $(CFLAGS) -o mc_core 						\
					servo_mc_core.c	\


############################################################################
$(OBJECT_DIR)servo_mc_core.o:  servo_mc_core.c servo_mc_core.h  servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_mc_core.c -o $(OBJECT_DIR)servo_mc_core.o

$(OBJECT_DIR)servo_rc_utils.o:  servo_rc_utils.c servo_rc_utils.h servo_rc_cmds.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_rc_utils.c -o $(OBJECT_DIR)servo_rc_utils.o

$(OBJECT_DIR)servo_time.o: 		servo_time.c servo_time.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_time.c -o $(OBJECT_DIR)servo_time.o

$(OBJECT_DIR)servo_mount_cfg.o: servo_mount_cfg.c servo_mount_cfg.h servo_time.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_mount_cfg.c -o $(OBJECT_DIR)servo_mount_cfg.o

$(OBJECT_DIR)servo_mount.o:		servo_mount.c servo_mount.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_mount.c -o $(OBJECT_DIR)servo_mount.o

$(OBJECT_DIR)servo_test_tty.o:		servo_test_tty.c servo_mount.c servo_mount.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_test_tty.c -o $(OBJECT_DIR)servo_test_tty.o
############################################################################
# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
clean:
	$(RM) $(OBJECT_DIR)*

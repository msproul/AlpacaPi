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
#++	May 27,	2022	<RNS> Added support for test_cop
#++	Jun 24,	2022	<RNS> Added -Wextra to CFLAGS
#++	Jun 27,	2022	<RNS> Added support for servo_motion build
#++	Jun 27,	2022	<RNS> Mod'd mc_core build to read motion config file
#++	Jun 28,	2022	<RNS> Add support for servo_local_cfg build
#++	Jul  2,	2022	<RNS> Support for some renamed observ files
#++	Jul  2,	2022	<RNS> Support for servo_motion test build
#++	Jul  2,	2022	<RNS> Enabled gdb support in CFLAGS (-g)
#++	Jul 20,	2022	<RNS> Support servo_move and servo_pos_step test progs
############################################################################

CC			=	gcc -I$(MLS_LIB_DIR)

CFLAGS 		=	-Wall -Wextra -g
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
				$(OBJECT_DIR)servo_motion.o 	\
				$(OBJECT_DIR)servo_motion_cfg.o	\
				$(OBJECT_DIR)servo_time.o		\
				$(OBJECT_DIR)servo_observ_cfg.o	\
				$(OBJECT_DIR)servo_mc_core.o	\
				$(OBJECT_DIR)servo_rc_utils.o

TESTMOUNTCFG_OBJECTS	=						\
				$(OBJECT_DIR)servo_mount_cfg.o	\
				$(OBJECT_DIR)servo_time.o

TESTMOTIONCFG_OBJECTS	=						\
				$(OBJECT_DIR)servo_motion_cfg.o 

TESTOBSERVCFG_OBJECTS	=						\
				$(OBJECT_DIR)servo_observ_cfg.o	\
				$(OBJECT_DIR)servo_time.o

RCUTILS_OBJECTS	=								\
				$(OBJECT_DIR)servo_rc_utils.o 	\
				$(OBJECT_DIR)servo_mc_core.o

MOTION_OBJECTS	=								\
				$(OBJECT_DIR)servo_motion.o 	\
				$(OBJECT_DIR)servo_motion_cfg.o	\
				$(OBJECT_DIR)servo_time.o		\
				$(OBJECT_DIR)servo_rc_utils.o 	\
				$(OBJECT_DIR)servo_mc_core.o

MCCORE_OBJECTS	=								\
				$(OBJECT_DIR)servo_mc_core.o 	\
				$(OBJECT_DIR)servo_motion_cfg.o

servo_test_tty:	CFLAGS	+=
servo_test_tty: 	$(SERVO_OBJECTS)	$(OBJECT_DIR)servo_test_tty.o
	$(CC) $(CFLAGS) -o servo_test_tty				\
					$(SERVO_OBJECTS)				\
					$(OBJECT_DIR)servo_test_tty.o	\
					-lm

servo_move:		CFLAGS	+=
servo_move: 	$(SERVO_OBJECTS)	$(OBJECT_DIR)servo_move.o
	$(CC) $(CFLAGS) -o servo_move				\
					$(SERVO_OBJECTS)				\
					$(OBJECT_DIR)servo_move.o	\
					-lm					
	
servo_pos_step:		CFLAGS	+=
servo_pos_step: 	$(SERVO_OBJECTS)	$(OBJECT_DIR)servo_pos_step.o
	$(CC) $(CFLAGS) -o servo_pos_step			\
					$(SERVO_OBJECTS)				\
					$(OBJECT_DIR)servo_pos_step.o	\
					-lm				

test_cop: 	CFLAGS	+=
test_cop: 	$(SERVO_OBJECTS) $(OBJECT_DIR)test_cop.o
	$(CC) $(CFLAGS) -o test_cop					\
					$(SERVO_OBJECTS)			\
					$(OBJECT_DIR)test_cop.o	\
					-lm

servo_test: 	CFLAGS	+=	-D_TEST_SERVO_MOUNT_
servo_test: 	$(SERVO_OBJECTS)
	$(CC) $(CFLAGS) -o servo_test 				\
					$(SERVO_OBJECTS)			\
					-lm

motion_test: 	CFLAGS	+=	-D_TEST_SERVO_MOTION_
motion_test: 	$(MOTION_OBJECTS)
	$(CC) $(CFLAGS) -o motion_test				\
					$(MOTION_OBJECTS)			\
					-lm

test_mnt_cfg: 	CFLAGS	+=	-D_TEST_SERVO_MOUNT_CFG_
test_mnt_cfg: 	$(TESTMOUNTCFG_OBJECTS)
	$(CC) $(CFLAGS) -o test_mount_cfg 				\
					$(TESTMOUNTCFG_OBJECTS)			\
					-lm

test_mot_cfg: 	CFLAGS	+=	-D_TEST_SERVO_MOTION_CFG_
test_mot_cfg: 	$(TESTMOTIONCFG_OBJECTS)
	$(CC) $(CFLAGS) -o test_motion_cfg 				\
					$(TESTMOTIONCFG_OBJECTS)			\
					-lm

test_obs_cfg: 	CFLAGS	+=	-D_TEST_SERVO_OBSERV_CFG_
test_obs_cfg: 	$(TESTOBSERVCFG_OBJECTS)
	$(CC) $(CFLAGS) -o test_observ_cfg 				\
					$(TESTOBSERVCFG_OBJECTS)			\
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
mc_core: 	$(MCCORE_OBJECTS)
	$(CC) $(CFLAGS) -o mc_core 						\
					$(MCCORE_OBJECTS)


############################################################################
$(OBJECT_DIR)servo_mc_core.o:  servo_mc_core.c servo_mc_core.h servo_motion_cfg.c servo_motion_cfg.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_mc_core.c -o $(OBJECT_DIR)servo_mc_core.o

$(OBJECT_DIR)servo_rc_utils.o:  servo_rc_utils.c servo_rc_utils.h servo_rc_cmds.h servo_mc_core.h servo_motion_cfg.c servo_motion_cfg.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_rc_utils.c -o $(OBJECT_DIR)servo_rc_utils.o

$(OBJECT_DIR)servo_time.o: 		servo_time.c servo_time.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_time.c -o $(OBJECT_DIR)servo_time.o

$(OBJECT_DIR)servo_mount_cfg.o: servo_mount_cfg.c servo_mount_cfg.h servo_time.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_mount_cfg.c -o $(OBJECT_DIR)servo_mount_cfg.o

$(OBJECT_DIR)servo_motion_cfg.o: servo_motion_cfg.c servo_motion_cfg.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_motion_cfg.c -o $(OBJECT_DIR)servo_motion_cfg.o	

$(OBJECT_DIR)servo_observ_cfg.o: servo_observ_cfg.c servo_observ_cfg.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_observ_cfg.c -o $(OBJECT_DIR)servo_observ_cfg.o	

$(OBJECT_DIR)servo_mount.o:		servo_mount.c servo_mount.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_mount.c -o $(OBJECT_DIR)servo_mount.o

$(OBJECT_DIR)servo_motion.o:	servo_motion.c servo_motion.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_motion.c -o $(OBJECT_DIR)servo_motion.o

$(OBJECT_DIR)test_cop.o:		test_cop.c servo_mount.c servo_mount.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c test_cop.c -o $(OBJECT_DIR)test_cop.o

$(OBJECT_DIR)servo_test_tty.o:		servo_test_tty.c servo_mount.c servo_mount.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_test_tty.c -o $(OBJECT_DIR)servo_test_tty.o

$(OBJECT_DIR)servo_move.o:		servo_move.c servo_mount.c servo_mount.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_move.c -o $(OBJECT_DIR)servo_move.o	

$(OBJECT_DIR)servo_pos_step.o:		servo_pos_step.c servo_mount.c servo_mount.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_pos_step.c -o $(OBJECT_DIR)servo_pos_step.o	

############################################################################
# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
clean:
	$(RM) $(OBJECT_DIR)*

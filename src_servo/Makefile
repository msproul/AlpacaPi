############################################################################
#
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
############################################################################
#++	May  9,	2022	<MLS> Updated to use Objectfiles directory for .o files
#++	May 12,	2022	<RNS> Corrected OBJECT_DIR pathname
############################################################################

CC			=	gcc -I$(MLS_LIB_DIR)

CFLAGS 		=	-Wall
RM 			=	/bin/rm -v
OBJECT_DIR	=	../Objectfiles/
MLS_LIB_DIR	=	../src_mlsLib/

# typing 'make' will invoke the first target entry in the file
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: servo_test

# To create the executable file count we need the object files
# countwords.o, counter.o, and scanner.o:
#
#rc_test:  servo_mc_core.o servo_rc_utils.o
#	$(CC) $(CFLAGS) -o rc_test servo_mc_core.o servo_rc_utils.o -lm

SERVO_OBJECTS	=								\
				$(OBJECT_DIR)servo_scope.o		\
				$(OBJECT_DIR)servo_scope_cfg.o	\
				$(OBJECT_DIR)servo_time.o		\
				$(OBJECT_DIR)servo_mc_core.o	\
				$(OBJECT_DIR)servo_rc_utils.o

TESTCFG_OBJECTS	=								\
				$(OBJECT_DIR)servo_scope_cfg.o	\


servo_test: 	CFLAGS	+=	-D_TEST_SERVO_SCOPE_
servo_test: 	$(SERVO_OBJECTS)
	$(CC) $(CFLAGS) -o servo_test 				\
					$(SERVO_OBJECTS)			\
					-lm


test_cfg: 	CFLAGS	+=	-D_TEST_SERVO_SCOPE_CFG_
test_cfg: 	$(TESTCFG_OBJECTS)
	$(CC) $(CFLAGS) -o test_cfg 				\
					$(TESTCFG_OBJECTS)			\
					-lm


core: 	CFLAGS	+=	-D_TEST_SERVO_ME_CORE_
core: 	$(SERVO_OBJECTS)
	$(CC) $(CFLAGS) -o core 				\
					$(SERVO_OBJECTS)			\
					-lm

############################################################################
$(OBJECT_DIR)servo_mc_core.o:  servo_mc_core.c servo_mc_core.h servo_mc_core.h  servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_mc_core.c -o $(OBJECT_DIR)servo_mc_core.o

$(OBJECT_DIR)servo_rc_utils.o:  servo_rc_utils.c servo_rc_utils.h servo_rc_cmds.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_rc_utils.c -o $(OBJECT_DIR)servo_rc_utils.o

$(OBJECT_DIR)servo_time.o: 		servo_time.c servo_time.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_time.c -o $(OBJECT_DIR)servo_time.o

$(OBJECT_DIR)servo_scope_cfg.o: servo_scope_cfg.c servo_scope_cfg.h servo_time.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_scope_cfg.c -o $(OBJECT_DIR)servo_scope_cfg.o

$(OBJECT_DIR)servo_scope.o:		servo_scope.c servo_scope.h servo_rc_utils.h servo_rc_cmds.h servo_time.h servo_mc_core.h servo_std_defs.h
	$(CC) $(CFLAGS) -c servo_scope.c -o $(OBJECT_DIR)servo_scope.o


############################################################################
# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
clean:
	$(RM) $(OBJECT_DIR)*

CC=gcc
CFLAGS= -O3 -Wall -g
LIBS= -lm
AR=ar

BNO_OBJECTS	=								\
				$(OBJECT_DIR)i2c_bno055.o	\
				$(OBJECT_DIR)getbno055.o	\


ALLBIN=getbno055
OBJECT_DIR	=	./Objectfiles/

all: ${ALLBIN}


getbno055:	$(BNO_OBJECTS)

	$(CC) $(BNO_OBJECTS) -o getbno055 ${LIBS}

clean:
	rm -f $(OBJECT_DIR)*

#################################################################
$(OBJECT_DIR)i2c_bno055.o:  i2c_bno055.c	\
							i2c_bno055.h
	$(CC) $(CFLAGS) -c i2c_bno055.c -o $(OBJECT_DIR)i2c_bno055.o

$(OBJECT_DIR)getbno055.o:  getbno055.c	\
							getbno055.h
	$(CC) $(CFLAGS) -c getbno055.c -o $(OBJECT_DIR)getbno055.o

platform = x86

CC = g++

ifeq ($(platform), mac32)
CFLAGS += -m32 -framework IOKit -framework CoreFoundation

endif

ifeq ($(platform), mac64)

CFLAGS += -framework IOKit -framework CoreFoundation
endif

ifeq ($(platform), mac)
CFLAGS += -arch i386 -arch x86_64 -framework IOKit -framework CoreFoundation

endif

ifeq ($(platform), x86)
CFLAGS += -ludev
endif


ifeq ($(platform), x64)
CFLAGS += -ludev
endif

ifeq ($(platform), armv5)
CFLAGS += -ludev
endif


ifeq ($(platform), armv6)
CFLAGS += -ludev

endif

ifeq ($(platform), armv8)
CFLAGS += -ludev

endif

all:test_console
test_console: main.cpp
	$(CC) main.cpp -o test_console $(CFLAGS) ../../lib/$(platform)/libEFWFilter.a -I../../include $(CFLAGS) -lpthread
	cp test_console bin/$(platform)/
clean:
	rm -f test_console



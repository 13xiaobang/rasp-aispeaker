#common makefile header

#DIR_INC = include
DIR_LIB = ./lib
#DIR_LIB = libs

TARGET	= libwebrtc.so
BIN_TARGET = $(DIR_LIB)/$(TARGET)

CROSS_COMPILE = 
CFLAGS = -g -Wall #-I$(DIR_INC) -DWEBRTC_POSIX

ifdef LINUX64
#LDFLAGS := -L$(DIR_LIB)/x64
else
#LDFLAGS := -L$(DIR_LIB)/x86 
endif
LDFLAGS = -lrt -ldl -lpthread -lm -fPIC -shared

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

$(BIN_TARGET) : $(OBJECTS)
	$(CROSS_COMPILE)gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o : %.c
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@
clean:
	@rm -f *.o $(BIN_TARGET)

.PHONY:clean

#common makefile foot

# Makefile template credits to Nebuleon and mth

CC          :=  /opt/gcw0-toolchain/usr/bin/mipsel-linux-gcc

SYSROOT     := $(shell $(CC) --print-sysroot)

OBJS        := lineloop.o
HEADERS     :=

INCLUDE     := -I. -I$(SYSROOT)/usr/include
DEFS        +=

CFLAGS       = -Wall -Wno-unused-variable -O2 $(DEFS) $(INCLUDE)
LDFLAGS     := -L$(SYSROOT)/usr/lib -lSDL -lGLESv1_CM -lEGL -lpthread

.PHONY: all clean

all: lineloop

lineloop: lineloop.o
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm lineloop *.o

# The two below declarations ensure that editing a .c file recompiles only that
## file, but editing a .h file recompiles everything.
## Courtesy of Maarten ter Huurne.
#
## Each object file depends on its corresponding source file.
#$(C_OBJS): %.o: %.c
#
## Object files all depend on all the headers.
#$(OBJS): $(HEADERS)


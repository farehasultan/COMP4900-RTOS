
#
#	Makefile for backend.c
#

DEBUG = -g
CC    = qcc
LD    = qcc


TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntox86
#TARGET = -Vgcc_ntoarmv7le
#TARGET = -Vgcc_ntoaarch64le


CFLAGS  += $(DEBUG) $(TARGET) -Wall -l socket -lm
LDFLAGS += $(DEBUG) $(TARGET)
BINS     = backend
all: $(BINS)

clean:
	rm -f *.o $(BINS);


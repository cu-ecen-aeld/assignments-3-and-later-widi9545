CC ?=mpicc
CFLAGS ?= -g -Wall 
EXTRA_CFLAGS ?= -DUSE_AESD_CHAR_DEVICE=1
LDFLAGS ?= -I/usr/lib/x86_64-linux-gnu/openmpi/include 
MPIFLAGS ?= -L/usr/lib/x86_64-linux-gnu/openmpi/lib -lmpi

default:
	$(CROSS_COMPILE)$(CC) $(EXTRA_CFLAGS) $(CFLAGS) $(LDFLAGS)  aesdsocket.c -o aesdsocket $(MPIFLAGS)
all: 
	$(CROSS_COMPILE)$(CC) $(EXTRA_CFLAGS) $(CFLAGS) $(LDFLAGS)  aesdsocket.c -o aesdsocket $(MPIFLAGS)
clean:
	rm -r aesdsocket
	

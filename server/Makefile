OMPI_CC=aarch64-linux-gnu-gcc

CC ?=mpicc
CFLAGS ?= -g -Wall 
EXTRA_CFLAGS ?= -DUSE_AESD_CHAR_DEVICE=1

default:
	$(CROSS_COMPILE)$(CC) $(EXTRA_CFLAGS) $(CFLAGS) $(LDFLAGS)  aesdsocket.c -o aesdsocket $(MPIFLAGS)
all: 
	$(CROSS_COMPILE)$(CC) $(EXTRA_CFLAGS) $(CFLAGS) $(LDFLAGS)  aesdsocket.c -o aesdsocket $(MPIFLAGS)
clean:
	rm -r aesdsocket
	

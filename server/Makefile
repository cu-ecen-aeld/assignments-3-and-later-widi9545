#OMPI_CC=aarch64-linux-gnu-gcc

CC ?=mpicc
CFLAGS ?= -g -Wall 
EXTRA_CFLAGS ?= -DUSE_AESD_CHAR_DEVICE=1
MPI_COMPILE_FLAGS ?= ~/assignment-8-widi9545/buildroot/output/host/aarch64-buildroot-linux-gnu/sysroot/usr/include
MPI_LINK_FLAGS ?= ~/assignment-8-widi9545/buildroot/output/host/aarch64-buildroot-linux-gnu/sysroot/usr/lib
default:
	$(CROSS_COMPILE)$(CC) $(MPI_COMPILE_FLAGS) $(EXTRA_CFLAGS) $(CFLAGS) $(LDFLAGS)  aesdsocket.c -o aesdsocket $(MPI_LINK_FLAGS)
all: 
	$(CROSS_COMPILE)$(CC) $(MPI_COMPILE_FLAGS) $(EXTRA_CFLAGS) $(CFLAGS) $(LDFLAGS)  aesdsocket.c -o aesdsocket $(MPI_LINK_FLAGS)
clean:
	rm -r aesdsocket
	

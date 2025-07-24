#OMPI_CC=aarch64-linux-gnu-gcc

CC ?=mpicc
CFLAGS ?= -g -Wall 
EXTRA_CFLAGS ?= -DUSE_AESD_CHAR_DEVICE=1
MPI_COMPILE_FLAGS ?= $(shell mpicc --showme:compile)
MPI_LINK_FLAGS ?= $(shell mpicc --showme:link)

default:
	$(CROSS_COMPILE)$(CC) $(MPI_COMPILE_FLAGS) $(EXTRA_CFLAGS) $(CFLAGS) $(LDFLAGS)  aesdsocket.c -o aesdsocket $(MPI_LINK_FLAGS)
all: 
	$(CROSS_COMPILE)$(CC) $(MPI_COMPILE_FLAGS) $(EXTRA_CFLAGS) $(CFLAGS) $(LDFLAGS)  aesdsocket.c -o aesdsocket $(MPI_LINK_FLAGS)
clean:
	rm -r aesdsocket
	

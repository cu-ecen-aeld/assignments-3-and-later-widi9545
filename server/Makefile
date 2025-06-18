CC ?=gcc
CFLAGS ?= -g -Wall 
EXTRA_CFLAGS ?= -DUSE_AESD_CHAR_DEVICE=1

default:
	$(CROSS_COMPILE)$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS)  aesdsocket.c -o aesdsocket
all:
	$(CROSS_COMPILE)$(CC) $(EXTRA_CFLAGS) $(LDFLAGS) $(CFLAGS)  aesdsocket.c -o aesdsocket
clean:
	rm -r aesdsocket
	

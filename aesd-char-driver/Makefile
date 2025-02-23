CC ?=gcc
CFLAGS ?= -g -Wall -Werror

default:
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) $(CFLAGS) aesdsocket.c -o aesdsocket
all:
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) $(CFLAGS) aesdsocket.c -o aesdsocket
clean:
	rm -r aesdsocket
	

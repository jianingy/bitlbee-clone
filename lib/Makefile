###########################
## Makefile for BitlBee  ##
##                       ##
## Copyright 2006 Lintux ##
###########################

### DEFINITIONS

-include ../Makefile.settings

# [SH] Program variables
objects = arc.o base64.o $(EVENT_HANDLER) http_client.o ini.o md5.o misc.o proxy.o sha1.o $(SSL_CLIENT) url.o xmltree.o

CFLAGS += -Wall
LFLAGS += -r

# [SH] Phony targets
all: lib.o
check: all
lcov: check
gcov:
	gcov *.c

.PHONY: all clean distclean

clean: $(subdirs)
	rm -f *.o $(OUTFILE) core

distclean: clean $(subdirs)

### MAIN PROGRAM

lib.o: $(objects) $(subdirs)
	@echo '*' Linking lib.o
	@$(LD) $(LFLAGS) $(objects) -o lib.o

$(objects): ../Makefile.settings Makefile

$(objects): %.o: %.c
	@echo '*' Compiling $<
	@$(CC) -c $(CFLAGS) $< -o $@

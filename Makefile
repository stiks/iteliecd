#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS=`csoap-config --cflags`

# define any directories containing header files
INCLUDES = -Isrc -Isrc/core

# define any libraries to link into executable:
# removed: -levent
LIBS = `csoap-config --libs` -lstatgrab -ldevstat -lconfig

# define the C source files
SRCS = src/iteliec-client.c \
       src/core/log.c \
       src/core/conf.c \
       src/core/soap.c \
       src/system/cpu.c \
       src/system/swap.c \
       src/system/host.c \
       src/system/disk.c \
       src/system/user.c \
       src/system/load.c \
       src/system/memory.c \
       src/system/process.c \
       src/system/network.c

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = iteliecd

# define delete fuction
RM = rm

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)
	@echo  Iteliec client has been compiled

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

install: all
	install -m 755 $(MAIN) $(DESTDIR)$(PREFIX)/bin
	test -e $(DESTDIR)$(PREFIX)/src/examples/iteliecd.conf || install -m 644 src/examples/iteliecd.conf $(DESTDIR)$(PREFIX)/etc
#	test -e /etc/iteliec.conf || ln -s $(DESTDIR)$(PREFIX)/etc/iteliec.conf /etc/iteliec.conf

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) -f $(OBJS) $(MAIN) $(MAIN).core

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
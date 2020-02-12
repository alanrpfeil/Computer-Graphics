# SIG locations:
SIGROOT = ../../sig
LIBDIR = $(SIGROOT)/lib/linux64
INCLUDEDIR = -I$(SIGROOT)/include -I/X11

# compilation options:
CC = g++
COPT = -O2 -std=c++11
WARN = #-Wall -Wno-switch -Wno-maybe-uninitialized -Wno-unused-variable 
CFLAGS = $(COPT) $(WARN) $(INCLUDEDIR)
SRCDIR = ../src/

# linker options:
LIBS = -lsigogl -lsigos -lsig -lglfw -lX11 -lGL
LFLAGS = -L$(LIBDIR) $(LIBS)
BIN = ./sigapp.x

# automatic selection of all sources to compile:
CPPFILES := $(shell echo $(SRCDIR)*.cpp)
OBJFILES = $(CPPFILES:.cpp=.o)
OBJECTS = $(notdir $(OBJFILES))
DEPENDS = $(OBJECTS:.o=.d)

# Be quiet when building:
.SILENT:

# rules:
$(BIN): $(OBJECTS)
	echo "creating:" $(BIN);
	$(CC) $(OBJECTS) $(LFLAGS) -o $(BIN)

%.o: $(SRCDIR)%.cpp
	echo "compiling:" $<;
	$(CC) -c $(CFLAGS) $< -o $@

%.d: $(SRCDIR)%.cpp
	echo "upddepend:" $<;
	$(CC) -MM $(CFLAGS) $< > $@

ifneq ($(MAKECMDGOALS),cleanall)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif
endif

cleanall:
	$(RM) core *.d *.o *~ *.x

clean:
	$(RM) core *.d *.o *~

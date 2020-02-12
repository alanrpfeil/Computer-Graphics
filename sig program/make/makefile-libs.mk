SRCDIR = $(ROOT)/src/$(TARGET)/
LIB = $(LIBDIR)/lib$(TARGET).a

CPPFILES := $(shell echo $(SRCDIR)*.cpp)
OBJFILES = $(CPPFILES:.cpp=.o)
OBJECTS = $(notdir $(OBJFILES))
DEPENDS = $(OBJECTS:.o=.d)

$(LIB): $(OBJECTS)
	ar -r $(LIB) $(OBJECTS)

%.o: $(SRCDIR)%.cpp
	echo "compiling:" $<;
	$(CC) -c $(CFLAGS) $< -o $@

%.d: $(SRCDIR)%.cpp
	echo "upddepend:" $<;
	$(CC) -MM $(CFLAGS) $< > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

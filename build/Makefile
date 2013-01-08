.SUFFIXES:

CC := gcc
CCFLAGS := -m32
CPPFLAGS := -Wall
LDLIBS := -lreadline

vpath %.c ../src
vpath %.h ../src

SOURCES := tomoko.c input.c machine.c native.c
OBJECTS := $(SOURCES:.c=.o)
DEPENDS := $(SOURCES:.c=.d)
PROGRAM := ../tomoko

#------------------------------------------------------------------------------

.phony: all
all: $(PROGRAM)

.phony: clean
clean:
	-rm -f $(OBJECTS) $(DEPENDS)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $@ $^ $(LDLIBS)

#------------------------------------------------------------------------------

%.d: %.c Makefile
	$(CC) -MM $(CPPFLAGS) $(CCFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' \
			< $@.$$$$ > $@; \
	        sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
       		    -e '/^$$/ d' -e 's/$$/ :/' \
			< $@.$$$$ >> $@; \
		rm -f $@.$$$$

%.o: %.c Makefile
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CCFLAGS)

#------------------------------------------------------------------------------
# No point in building *.d if we only want to make clean.

ifneq (clean,$(MAKECMDGOALS))
-include $(DEPENDS)
endif

#------------------------------------------------------------------------------

	
	
	

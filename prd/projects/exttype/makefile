PRG = libexttype.a
OBJDIR  =../../bin/obj
BINDIR  =../../bin/lib
SRCROOT = ../../code

#CC = gcc
CC = g++
CXX = $(CC)

CFLAGS = $(shell cat ./../cflags)
CFLAGS += -I$(SRCROOT)

CPPFILES = dynamic/block_allocator.cpp\
	data/dataset.cpp\
	dynamic/num_array.cpp\
	files/xfs.cpp\
	debug/logs.cpp\
	debug/performance.cpp\
	exttype/pvect.cpp\
	streams/xstringstream.cpp\
	streams/file_stream.cpp\
	streams/binary_stream.cpp\
	streams/text_stream.cpp\
	geom/math.cpp\
	geom/vectn.cpp


# Additional directories to be searched for header files
RANLIB = ranlib
AR = ar
ARFLAGS = cr
# Directory where the libAb.a will be saved to 


DEPEND = makedepend -fdeps -- $(CFLAGS) $(SRCROOT) -- 


OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPPFILES))


.PHONY: all
all: DIRS $(BINDIR)/$(PRG)

DIRS:

$(BINDIR)/$(PRG): $(OBJ)
	@if [ ! -d $(BINDIR) ]; then mkdir -p $(BINDIR); fi
	@echo linking $(BINDIR)/$(PRG)
	@$(AR) $(ARFLAGS) $(BINDIR)/$(PRG) $(OBJ)
	@echo Done
#	$(RANLIB) $(INSTLIBDIR)/$(LIB)

${OBJDIR}/%.o : $(SRCROOT)/%.cpp ${HEADERS}
	@mkdir -p `dirname $@`
	@echo $*.cpp
	@$(CXX) ${CFLAGS} -o $@ -c $(SRCROOT)/$*.cpp

# generate source dependencies
deps: $(CPPFILES)
	touch deps
	$(DEPEND) $(CPPFILES)                             
	echo -e "\nPlease ignore errors above.\n"

GARBAGE = $(OBJ) core *~ $(BINDIR)/$(PRG)

.PHONY:clean

clean:
	@$(RM) $(OBJ) $(GARBAGE)

.PHONY:tags

tags:
	ctags -R    	

#include deps


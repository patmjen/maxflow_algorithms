PRG = libmaxflow_GT.a
OBJDIR  =../../bin/obj
BINDIR  =../../bin/lib
SRCROOT = ../../code

CC = g++
CXX = $(CC)
CFLAGS = $(shell cat ./../cflags)
CFLAGS += -I$(SRCROOT)

CPPFILES = d_maxflow/maxflow_GT.cpp\
hi_pr/construct.cpp\
hi_pr/hi_pr.cpp\
hi_pr/parser.cpp\
hi_pr/timer.cpp

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


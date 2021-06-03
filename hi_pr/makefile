# makefile for programs and generator

# compiler flags:
#
# CHECK_SOLUTION     check if at the end the sink is unreachable 
#                    from the source
# CUT_ONLY           define to compute min-cut but not max-flow
# PRINT_STAT         define to print detailed operation statistics
# PRINT_FLOW         define to print out flow value
# PRINT_CUT          define to print out the sink size of the cut
# WAVE_INIT          wave-style initialization (initial global update,
#                    then a wave of flow pushing). Better on some problems
# OLD_INIT           define to use old-style initialization (large excess
#                    at the source, global update)
# INIT_UPDATE        perform initial global update
# EXCESS_TYPE_LONG   set excessType to long, otherwise it is long long int
#                    if "long long int" not supported, change to "double"
#                    (in types.h)

CCOMP = g++
CFLAGS = -O4 -DNDEBUG -DPRINT_STAT -DCHECK_SOLUTION -Wall
#CFLAGS = -g -DPRINT_FLOW -DEXCESS_TYPE_LONG -DPRINT_STAT -DCHECK_SOLUTION -Wall

all: hi_pr.exe hi_prw.exe hi_pro.exe
hi_pr.exe: hi_pr.cpp parser.o types.h timer.o
	$(CCOMP) $(CFLAGS) -o hi_pr hi_pr.cpp construct.cpp hi_pr_main.cpp parser.o timer.o
	%.o: %.cpp
	$(CCOMP) $(CFLAGS) -c $@
hi_prw.exe: hi_pr.cpp parser.cpp types.h timer.cpp
	$(CCOMP) $(CFLAGS) -DWAVE_INIT -o hi_prw hi_pr.cpp
hi_pro.exe: hi_pr.cpp parser.cpp types.h timer.cpp
	$(CCOMP) $(CFLAGS) -DOLD_INIT -o hi_pro hi_pr.cpp
clean: 
	rm -f hi_pr.exe hi_pro.exe hi_prw.exe *~

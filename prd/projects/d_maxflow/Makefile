BINDIR  =../../bin/d_maxflow
LIBDIR  =../../bin/lib

CFLAGS = $(shell cat ./../cflags)

LDFLAGS = $(shell cat ./../lflags)


targets = lib/libexttype.a lib/libd_maxflow.a lib/libmaxflow_BK.a lib/libmaxflow_GT lib/libmaxflow_DD.a solve_dimacs.exe solve_random.exe

all:
	@make -f ../exttype/makefile
	@make -f d_maxflow.makefile
	@make -f maxflow_BK.makefile
	@make -f maxflow_GT.makefile
	@make -f maxflow_DD.makefile
	@make -f solve_dimacs.makefile
	@make -f solve_random.makefile
	@g++ $(CFLAGS) ../../code/d_maxflow/cut_cost.cpp -o $(BINDIR)/cut_cost

clean:
	@make -f ../exttype/makefile clean
	@make -f d_maxflow.makefile clean
	@make -f maxflow_BK.makefile clean
	@make -f maxflow_GT.makefile clean
	@make -f maxflow_DD.makefile clean
	@make -f solve_dimacs.makefile clean
	@make -f solve_random.makefile clean
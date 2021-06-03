#ifndef maxflow_dd_h
#define maxflow_dd_h

#define USE_INTEGER

#include "split-cpp/parallelmaxflow.h"

#include "d_maxflow/maxflow_solver.h"
#include "d_maxflow/dimacs_parser.h"
#include "exttype/exttype.h"


class maxflow_dd : public maxflow_solver, public dimacs_parser_callback{
public:
	typedef ParallelGraph<int,int,int> tgraph;
	tgraph * g;
public:
	maxflow_dd();
	void construct(const char * filename);
	tflow maxflow()override;
	void save_cut(const std::string & filename)override;
	virtual long long cut_cost()override;
	~maxflow_dd();
public:
	virtual void allocate1(int n ,int m, int S, int T,int d,int * sz) override;
	virtual void allocate2(int loop) override;
	virtual void read_arc(int loop,int u,int v,int cap1, int cap2) override;
public:
	int nR;
	int get_region(int u);
private:
	int S,T;
	int nV;
	int nE;
	int margin;
	int slice;
	exttype::intf sz;
	exttype::intf nnE;
	exttype::intf split;
};


#endif
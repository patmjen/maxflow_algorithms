#ifndef maxflow_GT_h
#define maxflow_GT_h

#include "hi_pr/hi_pr.h"
#include "dimacs_parser.h"
#include "maxflow_solver.h"

class maxflow_GT : public maxflow_solver, public dimacs_parser_callback{
public:
	typedef int tcap;
	typedef long long tflow;
public:
	hi_pr g;
public:
	maxflow_GT();
	void construct(const dynamic::num_array<int,2>  & E, dynamic::num_array<int,2> & cap, dynamic::num_array<int,1> & excess);
	tflow maxflow();
	void construct(const char * filename);
	virtual void save_cut(const std::string & filename)override;
	virtual long long cut_cost()override;
	virtual void get_cut(int * S)override;//!< read out minimum cut, S must be allocated, S[v] \in {0,1} -- indicator if the source set of the cut
public:
	virtual void allocate1(int n ,int m, int S, int T,int d,int * sz) override;
	virtual void allocate2(int loop) override;
	virtual void read_arc(int loop,int u,int v,int cap1, int cap2) override;
};

#endif
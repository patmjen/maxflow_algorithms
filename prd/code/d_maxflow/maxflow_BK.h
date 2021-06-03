// A wrapper to run BK solver on problems in the DIMACS format
//
//

#ifndef maxflow_BK_h
#define maxflow_BK_h

#include "maxflow/graph.h"
#include "maxflow_solver.h"
#include "dimacs_parser.h"

//! A wrapper to run BK solver on problems in the DIMACS format
class maxflow_BK : public maxflow_solver, public dimacs_parser_callback{
public:
	typedef Graph<tcap,tcap,tflow> tgraph;
	tgraph * g;
public:
	maxflow_BK();
	//depricated:
	//void construct(const dynamic::num_array<int,2>  & E, dynamic::num_array<int,2> & cap, dynamic::num_array<int,1> & excess);
	void construct(const char * filename);//!< initialize the graph from the file in DIMACS format
	tflow maxflow()override;//!< compute maximum flow
	void save_cut(const std::string & filename)override;//!< extract and save minimum cut, after maxflow is computed
	virtual void get_cut(int * S)override;//!< read out minimum cut, S must be allocated, S[v] \in {0,1} -- indicator if the source set of the cut
	virtual long long cut_cost(int * S)override;//! compute the cost if the cut S
	virtual long long cut_cost()override;//! verification: calculate the cost of the minimum cut, must equal the flow
	bool is_weak_source(int v);
	~maxflow_BK();
private://temp
	int nV;
	int nE;
public://! implementing dimacs_parser_callback interface
	virtual void allocate1(int n ,int m, int S, int T,int d,int * sz) override;
	virtual void allocate2(int loop) override;
	virtual void read_arc(int loop,int u,int v,int cap1, int cap2) override;
};

#endif
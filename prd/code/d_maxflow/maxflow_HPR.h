#ifndef maxflow_HPR_h
#define maxflow_HPR_h

//#include "seed_hi_pr1.h"
#include "hpr1.h"
#include "dimacs_parser.h"
#include "maxflow_solver.h"

class maxflow_HPR : public maxflow_solver, public dimacs_parser_callback{
public:
	typedef int tcap;
	typedef long long tflow;
	//typedef seed_hi_pr_n hpr;
	typedef hpr_n hpr;
public:
	hpr g;
public:
	maxflow_HPR();
	~maxflow_HPR();
//	void construct(const dynamic::num_array<int,2>  & E, dynamic::num_array<int,2> & cap, dynamic::num_array<int,1> & excess);
	tflow maxflow();
	void construct(const char * filename);
	void save_cut(const std::string & filename)override{};
	virtual tflow cut_cost()override;
	virtual void get_cut(int * S)override;
public:
	virtual void allocate1(int n ,int m, int S, int T,int d,int * sz)override;
	virtual void allocate2(int loop)override;
	virtual void read_arc(int loop,int u,int v,int cap1, int cap2)override;
};

#endif
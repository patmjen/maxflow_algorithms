#ifndef sequential_discharge_h
#define sequential_discharge_h

#include "defs.h"
#include "exttype/fixed_vect.h"
#include "region_graph.h"
#include "maxflow_solver.h"

//using namespace dynamic;

template<class RD> class sequential_discharge : public maxflow_solver{
protected:
	dynamic::page_allocator page1;
public:
	typedef int tcap;
	typedef long long tflow;
public://input
	region_graph * G;
	bool unload;
public:
	dynamic::fixed_array1<RD*> R;//!< region networks, aligned with nodes of G
public:
	tflow flow;
	int d_inf;
	int d_min;
protected:
	exttype::intf d_hist;
	int nractive;
	int nrchanged;
	long long dead;
	long long dead0;
	bool some_active;
	bool preprocess;
	int n_relabelled;
	const char * tmp_pth;
public:
	sequential_discharge();
	virtual ~sequential_discharge();
	void construct(region_graph * G, bool unload = true);
	void set_tmp(const char * tmp_pth);
protected:
	virtual void init();
	void construct_region(int r);
	void load_region(int r);
	void unload_region(int r);
	void msg_in(int sweep, int r);
	void msg_out(int sweep, int r);
	virtual void global_gap();
	virtual void process_region(int sweep, int r, bool flow_optimal);
	virtual void hist_inc(int sweep,int r);
	virtual void hist_dec(int sweep, int r);
	virtual void global_heuristic(){};
	virtual void flow_is_maximum(){};
public:
	tflow maxflow()override;
	void save_cut(const std::string & filename)override;
	void get_cut(int * C)override;
	virtual long long cut_cost()override;
	void check_validity();
};

#endif
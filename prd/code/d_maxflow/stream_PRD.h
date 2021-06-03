#ifndef stream_PRD_h
#define stream_PRD_h

#include "PRD.h"
#include "sequential_discharge.h"

class stream_PRD : virtual public sequential_discharge<PRD>{
public:
	typedef sequential_discharge<PRD> parent;
public:
	stream_PRD();
	void init();
	void hist_dec(int sweep, int r);
	void hist_inc(int sweep,int r);
	void global_gap();
	void process_region(int sweep, int r, bool flow_optimal);
	bool non_empty_gap(int r);
	virtual void flow_is_maximum()override;
};

/*
#include "region_graph.h"
#include "maxflow_solver.h"

using namespace dynamic;

class stream_PRD : public maxflow_solver{
private:
	dynamic::page_allocator page;
public:
	typedef int tcap;
	typedef long long tflow;
public:
	region_graph & G;
	dynamic::fixed_array1<PRD> R;//!< region networks, aligned with nodes of G
public:
	tflow flow;
	int d_inf;
	intf d_hist;
	bool unload;
	int global_gap;
public:
	stream_PRD(region_graph & _G, bool unload=true);
	~stream_PRD();
	void construct();
public:
	tflow maxflow()override;
	void save_cut(const std::string & filename)override{};
	virtual long long cut_cost()override;
private:
	//void check_incoming_flow(int r);//check for messages with non-zero flow
};
*/

#endif
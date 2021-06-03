#ifndef stream_ARD1_h
#define stream_ARD1_h

#include "sequential_discharge.h"
#include "seed_BK1.h"

class stream_ARD1 : virtual public sequential_discharge<seed_BK>{
public:
	typedef sequential_discharge<seed_BK> parent;
public:
	stream_ARD1();
	void init();
	void hist_dec(int sweep, int r)override;
	void hist_inc(int sweep,int r)override;
	void global_gap()override;
	void process_region(int sweep, int r, bool flow_optimal)override;
	//void apply_gap(int r);
	//void initial_LB();
public://boundary relabel heuristic
	dynamic::fixed_array1<region_graph::d_group> groups_pool;// at most nB elements
	dynamic::fixed_array1<region_graph::d_group*> g_cache;  // at most d_inf elements
	region_graph::d_group* g_free;
	void global_heuristic()override;//!< boundary relabel heuristic
};

#endif
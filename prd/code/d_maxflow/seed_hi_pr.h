#ifndef seed_hi_pr_h
#define seed_hi_pr_h

#include "seed_buckets.h"
#include "dynamic/dynamic.h"
#include "dynamic/basic_heap.h"

#include "debug/performance.h"

typedef int tcap;
typedef long long tflow;

//__________________________________________________
class seed_hi_pr{
public:
	class arc;
	//__________________________________________________
	class node: public bucket_node<node>{
		//inherits
		// int d;
	public:
		tcap excess;
	public:
		arc * arc_first;
		arc * arc_end;
		arc * current;
	public:
		node():arc_first(0){};
	};
	//__________________________________________________
	class arc{
	public:
		node * head;
		arc * rev;
		tcap resCap;
		arc(){};
	};
	//___________________________________________________
public:
	int nV;
	int nB;
	int n;
	int m;
	int d_inf;
	dynamic::fixed_array1<node> nodes;
	dynamic::fixed_array1<arc> arcs;
	tflow flow;
	int S,T;//dummy
public:
	debug::PerformanceCounter c1;
	debug::PerformanceCounter c2;
	debug::PerformanceCounter c3;
public:
	seed_buckets<node> buckets;
	typedef seed_buckets<node>::bucket tbucket;
public:
	dynamic::fixed_array1<node*> BS; //boundary nodes, sorted by distance
public:
	seed_hi_pr();
	~seed_hi_pr();
	void construct(int nV, int nE, const int * E, const tcap *cap, const tcap * excess, int nB=0, int nE_out=0, int * E_out=0, int d_inf=0);
	void inplace_edge_reorder(int nE_out=0, int * E_out=0);
	void global_update();
	void stageOne(bool reuse=false);
	void raise_seed(node *i, int d);
	tflow cut_cost(int * C);
	tflow cut_cost();
public:
	//__forceinline
	void elliminate(node *u);
public:
	__forceinline void init_arc(node *u, node *v, arc* uv, arc * vu, tcap cap1, tcap cap2);
private:
	__forceinline void dismiss_bucket(tbucket * a);
	__forceinline tbucket * gap(tbucket * b);
	__forceinline void push1(node *u, arc *a, node *&v);
	__forceinline bool is_out_bnd(arc * a);
public:
	void global_gap(int gap);
	void clear_seeds();
	void update_seeds();
	bool is_weak_source(node * v){
		return v->d>=d_inf; //source or free
	};
private:
	int nE;
	int nE_out;
	int * E_out;
public:
	static long long size_required(int nV, int nB, int nE, int nE_out,int d_inf);
	void allocate1(int nV, int nB, int nE, int nE_out =0, int *E_out=0, int d_inf=0, int S=-1, int T=-1);
	void add_edge(int u, int v, int cap1, int cap2, int loop=1);
	void add_tweights(int u,int cap1, int cap2);
	void allocate2();
	void reset_counters();
};

//_____________________inlined code________________________________________________
__forceinline void seed_hi_pr::elliminate(seed_hi_pr::node *u){
	assert(u->state==bucket_node<node>::free);
	u->d = d_inf;
	u->set_state(bucket_node<node>::dead);
};


#endif

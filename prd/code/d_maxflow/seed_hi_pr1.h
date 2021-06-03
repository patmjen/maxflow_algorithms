#ifndef seed_hi_pr_h
#define seed_hi_pr_h

#include "seed_buckets.h"
#include "dynamic/fixed_array1.h"
#include "debug/performance.h"

typedef int tcap;
typedef long long tflow;
//typedef int tflow;
#include "base_graph.h"

//______________________________________________________
template<template<class Node, class Arc> class base_graph> class seed_hi_pr_defs{
public:
	class node;
	class arc;
	typedef base_graph<node,arc> tgraph;
	class node : public bucket_node<node>, public base_graph<node,arc>::graph_node{
	public:
		typename tgraph::arc_ptr current;
	public:
		node(){};
	};
	class arc : public tgraph::graph_arc{
	public:
	};
};
//
template<class base = seed_hi_pr_defs<base_graph> > class seed_hi_pr : public base::tgraph{
public:
	typedef typename base::tgraph tgraph;
	typedef typename base::node node;
	typedef typename base::arc arc;
	typedef typename base::tgraph::arc_ptr arc_ptr;
	//___________________________________________________
public:
	int d_inf;
	//	int S,T;//dummy
public:
	debug::PerformanceCounter c1;
	debug::PerformanceCounter c2;
	debug::PerformanceCounter c3;
public:
	seed_buckets<node> buckets;
	typedef typename seed_buckets<node>::bucket tbucket;
public:
	dynamic::fixed_array1<node*> BS; //boundary nodes, sorted by distance
public:
	seed_hi_pr();
	~seed_hi_pr();
	//	void construct(int nV, int nE, const int * E, const tcap *cap, const tcap * excess, int nB=0, int nE_out=0, int * E_out=0, int d_inf=0);
	//	void inplace_edge_reorder(int nE_out=0, int * E_out=0);
	void global_update();
	void stageOne(bool reuse=false);
	void raise_seed(node *i, int d);
	tflow cut_cost(int * C);
	tflow cut_cost();
public:
	void elliminate(node *u);
private:
	__forceinline void dismiss_bucket(tbucket * a);
	__forceinline tbucket * gap(tbucket * b);
	__forceinline void push1(node *u, arc_ptr & a, node *v);
	//__forceinline bool is_out_bnd(arc_ptr & a);
public:
	void global_gap(int gap);
	void clear_seeds();
	void update_seeds();
	bool is_weak_source(node * v){
		return v->d>=d_inf; //source or free
	};
public:
	static long long size_required(int nV, int nB, int nE, int nE_out,int d_inf);
public: //construction interface
	void allocate1(int nV, int nB, int nE, int nE_out =0, int *E_out=0, int d_inf=0, int S=-1, int T=-1);
	__forceinline void init_arc(node *u, node *v, arc* uv, arc * vu, tcap cap1, tcap cap2);
	// inherited from tgraph:
	//void add_edge(int u, int v, int cap1, int cap2, int loop=1);
	//void add_tweights(int u,int cap1, int cap2);
	//void allocate2();
	//void reset_counters();
	//______________________________
	struct node_less{
		bool operator()(
			const node * a,	const node * b)const{
				return a->d<b->d;
		};
	};
};

//_____________________inlined code________________________________________________
template<class defs> __forceinline void seed_hi_pr<defs>::elliminate(node *u){
	assert(u->state==bucket_node<node>::free);
	u->d = d_inf;
	u->set_state(bucket_node<node>::dead);
};

typedef seed_hi_pr<seed_hi_pr_defs<base_graph_n> > seed_hi_pr_n;

#endif

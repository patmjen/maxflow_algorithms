#ifndef hpr1_h
#define hpr1_h

#include "dynamic/fixed_array1.h"
#include "debug/performance.h"

typedef int tcap;
typedef long long tflow;
//typedef int tflow;
#include "base_graph.h"
#include "array_buckets.h"

//______________________________________________________
template<template<class Node, class Arc> class base_graph> class hpr_defs{
public:
	class node;
	class arc;
	typedef base_graph<node,arc> tgraph;
	class node : public array_buckets<node>::bucket_node, public base_graph<node,arc>::graph_node{
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
template<class base = hpr_defs<base_graph> > class hpr : public base::tgraph{
public:
	typedef typename base::tgraph tgraph;
	typedef typename base::node node;
	typedef typename base::arc arc;
	typedef typename base::tgraph::arc_ptr arc_ptr;
	typedef typename array_buckets<node>::bucket_node bucket_node;
	//___________________________________________________
public:
	int d_inf;
	//	int S,T;//dummy
public:
	debug::PerformanceCounter c1;
	debug::PerformanceCounter c2;
	debug::PerformanceCounter c3;
public:
	array_buckets<node> buckets;
	typedef typename array_buckets<node>::bucket tbucket;
public:
	hpr();
	~hpr();
	void global_update();
	void global_relabel();
	void stageOne(bool reuse=false);
	tflow cut_cost(int * C);
	tflow cut_cost();
public:
	void elliminate(node *u);
private:
	__forceinline void dismiss_bucket(tbucket * a);
	__forceinline tbucket * gap(tbucket * b);
	__forceinline void push1(node *u, arc_ptr & a, node *v);
public:
	void global_gap(int gap);
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
template<class defs> __forceinline void hpr<defs>::elliminate(node *u){
	assert(u->state==bucket_node::free);
	u->d = d_inf;
	u->set_state(bucket_node::dead);
};

typedef hpr<hpr_defs<base_graph_n> > hpr_n;

#endif

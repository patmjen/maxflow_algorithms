//
//AS: Push-Relabel Region Discharge (PRD) implementation, based on (reimplementation of) Goldberg-Tarjan HI-PR maxflow implementation
//
#ifndef _PRD_h_
#define _PRD_h_

#include "region_discharge.h"
#include "debug/logs.h"
#include "dynamic/dynamic.h"
#include "exttype/exttype.h"
#include <algorithm>
#include <limits>
#include "exttype/key_less.h"

#include "seed_hi_pr1.h"

using namespace exttype;

class PRD : public region_discharge{
public:
	typedef int tcap;
	typedef long long tflow;
	typedef seed_hi_pr<> hpr;
public:
	hpr * g;
	bool reuse;
	//int dead;
	//int global_gap;
	intf global_index;
	bool has_active;
public:
	PRD(){
		reuse = false;
		g = 0;
	};
public:
	static long long size_required(int nV, int nB, int nE, int nE_out,int d_inf);
	void construct();
	long long maxflow(bool reuse=false);
	void discharge();
	void relabel();
	mint2 rcap(int e);
	long long cut_cost();
public:
	size_t load(const std::string & id);
	//void save(const std::string & id);
	size_t unload(const std::string & id);
public:
	~PRD();
public:
	inline void msg_in(int e, int d, tcap f){
		d_in(e,d);
		flow_in(e,f);
	};
	inline void msg_out(int e, int * d, tcap * f){
		hpr::arc_ptr a = g->arc_for_index(e);
		hpr::node * u = g->arc_tail(a);
		hpr::node * v = g->arc_head(u,a);
		assert(u-g->nodes.begin()<nV);// this must be innter vertex
		tcap df = g->arc_rev(a)->resCap;
		*d = u->d;
		*f += df;
		v->excess -= df;
		assert(df>=0);
		g->arc_rev(a)->resCap = 0;
	};
public://old interface
	int d_out(int e){
		hpr::arc_ptr a = g->arc_for_index(e);
		//return g->arcs[e].rev->head->d;
		return g->arc_tail(a)->d;
	};
	void d_in(int e,int new_d);
	int & dist(int u){
		return g->nodes[u].d;
	};
	inline tcap flow_out(int e);
	void flow_in(int e, tcap f);
	int g_index(int v);
	bool is_weak_source(int v);
};

#endif
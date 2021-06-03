#include "maxflow_HPR.h"

maxflow_HPR::maxflow_HPR(){
	info.name = "HPR";
	g.n_shifts = 4;
};

maxflow_HPR::~maxflow_HPR(){
};

/*
void maxflow_HPR::construct(const dynamic::num_array<int,2>  & E, dynamic::num_array<int,2> & cap, dynamic::num_array<int,1> & excess){
	using namespace exttype;
	info.construct_t.start();
	g.construct(excess.size(),E.size()[1],E.begin(),cap.begin(),excess.begin());
	info.construct_t.stop();
};
*/

maxflow_HPR::tflow maxflow_HPR::maxflow(){
	info.solve_t.start();
	g.global_update();
	g.stageOne(true);
	info.flow = g.flow;
	g.global_update();//required to get the minimum cut
	info.solve_t.stop();
	info.nV = g.n;
	info.nE = g.m/2;
	info.mem_shared = dynamic::memserver::get_al_blocks().mem_used();
	return info.flow;
};

void maxflow_HPR::construct(const char * filename){
	info.construct_t.start();
	dimacs_parser(filename,*this,2);
	info.construct_t.stop();
};

void maxflow_HPR::allocate1(int n ,int m, int S, int T,int d,int * sz){
	g.allocate1(n,0,m,0,0,0,S,T);
	g.reset_counters();
	this->S = S;
	this->T = T;
};

void maxflow_HPR::allocate2(int loop){
	if(loop==0){
		g.allocate2();
		g.reset_counters();
	};
};

void maxflow_HPR::read_arc(int loop,int u,int v,int cap1, int cap2){
	g.add_edge(u,v,cap1,cap2,loop);
};

tflow maxflow_HPR::cut_cost(){
	return g.cut_cost();
};

void maxflow_HPR::get_cut(int * C){
	for(int v=0;v<g.nV;++v){
		C[v] = g.is_weak_source(&g.nodes[v]);
	};
	C[S] = 1;
	C[T] = 0;
};
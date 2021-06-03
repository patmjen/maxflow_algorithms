#include "parallel_ARD1.h"
#include "debug/performance.h"
#include <omp.h>

#include "parallel_discharge.cpp"
template class parallel_discharge<seed_BK>;

using namespace dynamic;
//#define assert(expression) {if(!(expression))throw debug_exception("assertion failed");}

void parallel_ARD1::construct(region_graph * G){
	parent::construct(G,false);
};

parallel_ARD1::parallel_ARD1(){
	info.name = "P-ARD1";
};

void parallel_ARD1::hist_dec(int sweep, int r){
	for(region_graph::arc * a = (*G)[r].out.begin();a<(*G)[r].out.end();++a){//over outcoming boundary edges of region r
		if(a->d<d_inf){
#pragma omp atomic
			--d_hist[a->d];
			assert(d_hist[a->d]>=0);
		};
	};
};

void parallel_ARD1::hist_inc(int sweep,int r){
	//update global hist for new labels of r
	for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary edges of region r
		region_graph::arc * a = &((*G)[r].out[j]);
		if(a->d<d_inf){
#pragma omp atomic
			++d_hist[a->d];
		};
	};
};


/*

void parallel_ARD1::fuse_flow(){
	info.msg_t.resume();
	for(int r=0;r<G->nR;++r){
		for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary edges of region r
			region_graph::arc * a = &((*G)[r].out[j]);
			int & d1 = a->d;
			int & f1 = a->f;
			int & d2 = a->rev->d;
			int & f2 = a->rev->f;
			if(d2>d1+1 && f1>0){
				//cancel f1
				//apply and return it
				a->r_cap-=f1;
				a->rev->r_cap+=f1;
				f2 = f2+f1;
				f1 = 0;
				(*G)[r].has_active = true;
			};
			if(a->rev->r_cap-f2+f1>0){
				assert(d2==d_inf || d2<=d1+1);
			};
		};
	};
	info.msg_t.pause();
};

void parallel_ARD1::hist_dec(int sweep, int r){
	for(region_graph::arc * a = (*G)[r].out.begin();a<(*G)[r].out.end();++a){//over outcoming boundary edges of region r
		if(a->d<d_inf){
#pragma omp atomic
			--d_hist[a->d];
			assert(d_hist[a->d]>=0);
		};
	};
};

void parallel_ARD1::hist_inc(int sweep,int r){
	//update global hist for new labels of r
	for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary edges of region r
		region_graph::arc * a = &((*G)[r].out[j]);
		if(a->d<d_inf){
#pragma omp atomic
			++d_hist[a->d];
		};
	};
};

void parallel_ARD1::lock_msg(int r){
#pragma omp critical //be sure to get all locks for itself
	{
		for(region_graph::region_arc * A = G->nodes[r].Out.begin(); A<G->nodes[r].Out.end();++A){//outcoming arcs
			int rr = A->rev->head-G->nodes.begin();
			int q = A->head-G->nodes.begin();
			omp_set_lock(&A->lock);
			omp_set_lock(&A->rev->lock);
		};
	};
};

void parallel_ARD1::unlock_msg(int r){
	//releasing locks can go in parallel, not critical
	for(region_graph::region_arc * A = G->nodes[r].Out.begin(); A<G->nodes[r].Out.end();++A){
		omp_unset_lock(&A->lock);
		omp_unset_lock(&A->rev->lock);
	};
};

parallel_ARD1::tflow parallel_ARD1::maxflow(){
	init();
	dead = 0;
	dead0 = 0;
	int sweep = 0;
	nractive = G->nR;
	int nrchanged = G->nR;
	double cpu=0;
	double cpu0=0;
	bool flow_optimal = false;
	//load data
	for(int r = 0;r<G->nR;++r){
		load_region(r);
		for(region_graph::region_arc * A = G->nodes[r].Out.begin(); A<G->nodes[r].Out.end();++A){
			omp_init_lock(&A->lock);
		};
	};
	//
	omp_lock_t msg_lock;
	omp_init_lock(&msg_lock);
	//
	info.solve_t.start();
	//
	while(nractive>0 || nrchanged>0){
		if(nractive==0 && !flow_optimal){
			flow_optimal = true;
			debug::stream<<"flow is optimal\n";
		};
		flow = 0;
		dead = dead0;
		info.augment_t.resume();
#pragma omp parallel for schedule(dynamic,1)
		for(int s = 0;s<G->nR;++s){
			int r = (s*15427)%G->nR;//shuffle processing order of the regions
			if((*G)[r].has_active || (flow_optimal && (*G)[r].labels_changed)){
				//
				//omp_set_lock(&msg_lock);
				//lock_msg(r);
//#pragma omp critical
				lock_msg(r);
				hist_dec(sweep,r);//read out msg, atomic hist
				msg_in(sweep,r);// read in msg
				unlock_msg(r);
				//
				//process_region(sweep,r,flow_optimal);
				int max_levels = d_inf;
				//if(sweep<2)
					max_levels = sweep;
				process_region(max_levels,r,flow_optimal);
				//
//#pragma omp critical
				lock_msg(r);
				msg_out(sweep,r);//write msg
				hist_inc(sweep,r);//read msg, atomic hist
				unlock_msg(r);
				//omp_unset_lock(&msg_lock);
			};
			//
		};//implied barrier
		//master
		for(int r = 0;r<G->nR;++r){
			dead+=(*G)[r].dead;
			flow+=(*G)[r].flow;
		};
		info.augment_t.pause();
		global_gap();
		fuse_flow();
		if(info.relabel_t.time()<0.1*info.solve_t.time()){
			global_heuristic();
		}else{
			n_relabelled=-1;
		};
		//
		info.msg_t.resume();
		nractive = 0;
		nrchanged = 0;
		for(int r=0;r<G->nR;++r){
			if((*G)[r].has_active){
				++nractive;
			};
			if((*G)[r].labels_changed){
				++nrchanged;
			};
		};
		if(sweep==0 && preprocess){
			dead = dead0;
		};
		info.msg_t.pause();
		info.solve_t.pause();
		char s [10];
		sprintf(s,"%4.1f",dead*100.0/G->nV);
		debug::stream<<"sweep: "<<sweep<<" flow: "<<flow<< " decided: "<<s<<"% active: "<<nractive<<" "<<"cahnged: "<<nrchanged;
		if(n_relabelled>=0){
			debug::stream<<" relabelled: "<<n_relabelled;
		};
		//for(int r=0;r<G->nR;++r){
		//	debug::stream<<((*G)[r].has_active?1:0);
		//};
		cpu = info.augment_t.time()+info.msg_t.time()+info.gap_t.time()+info.relabel_t.time();
		debug::stream<<" CPU: "<<cpu-cpu0<<"sec.";
		cpu0=cpu;
		debug::stream<<"\n";
		info.solve_t.resume();
		++sweep;
	};
	for(int r = 0;r<G->nR;++r){
		load_region(r);
		for(region_graph::region_arc * A = G->nodes[r].Out.begin(); A<G->nodes[r].Out.end();++A){
			omp_destroy_lock(&A->lock);
		};
	};
	info.solve_t.stop();
	omp_destroy_lock(&msg_lock);
	debug::stream<<"Total CPU: "<<cpu<<"\n";
	info.sweeps = sweep;
	info.flow = flow;
	info.nV = G->nV;
	info.nE = G->nE;
	info.nBE = G->nBE;
	info.mem_shared = memserver::get_al_blocks().mem_used();
	return flow;
};
*/
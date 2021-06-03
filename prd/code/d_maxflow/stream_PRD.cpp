#include "stream_PRD.h"
//#include "debug/performance.h"

using namespace dynamic;
#include "sequential_discharge.cpp"

template class sequential_discharge<PRD>;

stream_PRD::stream_PRD(){
	info.name = "S-PRD";
};

void stream_PRD::init(){
	d_inf = G->nV-1;//assuming nV includes sink and source
	d_min = 1;
	d_hist.resize(d_inf);
	d_hist << 0;
	d_hist[0] = 1; //sink has label 0
	d_hist[1] = G->nV-2;// all but source and sink have label 1
	parent::init();
};

void stream_PRD::hist_dec(int sewwp, int r){
	info.gap_t.resume();
	//subtract r from global label hist
	for(int v=0;v<R[r]->nV;++v){//go over inner vertices
		int d = R[r]->g->nodes[v].d;
		if(d<d_inf){
			--d_hist[d];
		};
	};
	info.gap_t.pause();
};

void stream_PRD::hist_inc(int sweep,int r){
	info.gap_t.resume();
	//update global hist for new labels of r
	for(int v=0;v<R[r]->nV;++v){//go over inner vertices
		int d = R[r]->g->nodes[v].d;
		if(d<d_inf){
			++d_hist[d];
		};
	};
	info.gap_t.pause();
};

bool stream_PRD::non_empty_gap(int r){
	return (*G)[r].global_gap < (*G)[r].max_d;
};

void stream_PRD::flow_is_maximum(){//triger the region updates for all regions
	for(int r=0;r<G->nR;++r){
		(*G)[r].labels_changed = true;
	};
	nrchanged = G->nR;
};

void stream_PRD::global_gap(){	
	//find a gap
	for(int gap=1;gap<d_hist.size();++gap){
		if(d_hist[gap]==0){//this is a global gap
			for(int q=0;q<G->nR;++q){
				(*G)[q].global_gap = std::min((*G)[q].global_gap,gap);
				if(non_empty_gap(q)){
					(*G)[q].labels_changed = true;
				};
			};
			break;
		};
	};
	info.gap_t.pause();
};

void stream_PRD::process_region(int sweep, int r, bool flow_optimal){
	info.gap_t.resume();
	//trigger region-relabel, not necessary but emperically usefull
	//if((*G)[r].global_gap <= R[r]->g->buckets.last->prev->d || flow_optimal){//there are labels above gap in r
	if(non_empty_gap(r) || flow_optimal){//there are labels above gap in r
		R[r]->reuse = false;
	};
	// apply to r its global_gap
	R[r]->g->global_gap((*G)[r].global_gap);
	(*G)[r].global_gap = d_inf;
	info.gap_t.pause();
	//
	info.relabel_t.resume();
	R[r]->relabel();//this relabels only if reuse = false, otherwise only reinsert seeds
	info.relabel_t.pause();
	//
	info.augment_t.resume();
	R[r]->discharge();
	info.augment_t.pause();
	//
	(*G)[r].flow = R[r]->F;
	(*G)[r].dead = R[r]->dead;
	(*G)[r].max_d = R[r]->g->buckets.last->prev->d;
};

/*
//#define assert(expression) {if(!(expression))throw debug_exception("assertion failed");}

stream_PRD::stream_PRD(region_graph & _G, bool _unload):G(_G),page(1*dynamic::block_allocator::GB),unload(_unload){
	info.name = "S-PRD";
	construct();
};

void stream_PRD::construct(){
	info.construct_t.resume();
	d_inf = G.nV+1;//1 for sink
	R.resize(G.nR);
	for(int r=0;r<G.nR;++r){
		R[r].d_inf = d_inf;
		R[r].nV = G[r].nV;
		R[r].nB = G[r].nB;
		R[r].nE = G[r].nE;
		R[r].nE_out = G[r].nE_out;
		R[r].global_gap = d_inf;
		dynamic::memserver::set_allocator(page);
		info.diskr_b += R[r].load(G.pth + G[r].id);
		for(int i=0;i<R[r].g->nodes.size();++i){
			R[r].g->nodes[i].d = 1;
		};
		for(int j=0;j<G[r].out.size();++j){//over outcoming r-edges of region r
			region_edge * ee = &(G[r].out[j]); // outcoming r->q edge
			for(int k=0;k<ee->emap.size();++k){ // over all boundary edges of this r-edge
				int be = ee->emap[k][0]-(R[r].nE-R[r].nE_out);
				ee->emap[k][0] = R[r].E_out[be];
				ee->msg[k].d = 1;
			};
			ee = ee->rev;
			for(int k=0;k<ee->emap.size();++k){ // over all boundary edges of this r-edge
				int be = ee->emap[k][1]-(R[r].nE-R[r].nE_out);
				ee->emap[k][1] = R[r].E_out[be];
				ee->msg[k].d = 1;
			};
		};
		dynamic::memserver::set_allocator(memserver::get_al_blocks());
		info.mem_region = std::max(page.mem_used(),info.mem_region);
		if(unload){
			info.diskw_b += page.unload((G[r].id+".page").c_str());
		};
	};
	
	////update indices in emap, due to reordering
	//for(int r=0;r<G.nR;++r){
	//	for(int j=0;j<G[r].out.size();++j){//over outcoming r-edges of region r
	//		region_edge * ee = &(G[r].out[j]); // outcoming r->q edge
	//		int q = ee->head;
	//		for(int k=0;k<ee->emap.size();++k){ // over all boundary edges of this r-edge
	//			int be = ee->emap[k][0]-(R[r].nE-R[r].nE_out);
	//			ee->emap[k][0] = R[r].E_out[be];
	//			be = ee->emap[k][1]-(R[q].nE-R[q].nE_out);
	//			ee->emap[k][1] = R[q].E_out[be];
	//			ee->msg[k].d = 1;
	//			ee->rev->msg[k].d = 1;
	//		};
	//	};
	//};
	
	d_hist.resize(d_inf);
	//init gap
	d_hist << 0;
	d_hist[1] = G.nV-2;
	d_hist[0] = 1;
	info.construct_t.stop();
	global_gap = d_inf;
};

stream_PRD::~stream_PRD(){
	//instead of loading the objects and deallocating them correctly
	//just clear all pointers, memory is freed by deallocating the page
	for(int r=0;r<G.nR;++r){
		//page.load((G[r].id+".page").c_str());//<loads all of R[r]
		//R[r].~PRD();
		if(!unload){
			dynamic::memserver::set_allocator(page);
			R[r].~PRD();
			dynamic::memserver::set_allocator(memserver::get_al_blocks());
		};
		memset(&R[r],0,sizeof(R[r]));
	};
};

stream_PRD::tflow stream_PRD::maxflow(){
	//bool some_active;
	int sweep = 0;
	info.solve_t.start();
	int nractive = 0;
	do{
		int dead = 0;
		flow = 0;
		//computation
		for(int r=0;r<G.nR;++r){// for all regions r
			if(!G[r].has_active){
				flow+=R[r].F;
				dead+=R[r].dead;
				continue; //skip this region -- no excess
			};
			++nractive;
			//region r has active nodes, bring it on
			//restore network from file
			info.diskr_t.resume();
			//R[r].load(G[r].id);
			if(unload){
				info.diskr_b += page.load((G[r].id+".page").c_str());//<loads all of R[r]
			};
			dynamic::memserver::set_allocator(page);
			info.diskr_t.pause();
			//
			//
			info.msg_t.resume();
			for(int j=0;j<G[r].out.size();++j){//over outcoming r-edges of region r
				//collect flow and boundary distance labels from neighboring regions
				region_edge * ee = G[r].out[j].rev; // outcoming q->r edge
				assert(ee->head==r);
				for(int k=0;k<ee->emap.size();++k){ // over all boundary edges of this r-edge
					int e_r = ee->emap[k][1]; // local edge index in r
					tcap & f = ee->msg[k].f;
					int d = ee->msg[k].d;
					R[r].msg_in(e_r,d,f);
					f = 0; //this flow is taken to region r and cleared from message
				};
			};
			info.msg_t.pause();
			//
			//
			//subtract r from global label hist
			//this assumes labeling of inner nodes has preserved since last touch of r
			//it is actually stored in the outcoming messages of r
			info.gap_t.resume();
			//subtract r from global label hist
			for(int v=0;v<R[r].nV;++v){//go over inner vertices
				int d = R[r].g->nodes[v].d;
				if(d<d_inf){
					--d_hist[d];
				};
			};
			R[r].g->global_gap(R[r].global_gap);
			info.gap_t.pause();
			//
			info.relabel_t.resume();
			R[r].relabel();//this relabels only if reuse = false, otherwise only reinsert seeds
			info.relabel_t.pause();
			//
			info.augment_t.resume();
			G[r].has_active = false;
			R[r].discharge();
			info.augment_t.pause();
			//
			flow+=R[r].F;
			//
			//
			//info.relabel_t.resume();
			//R[r].reuse = false;
			//R[r].relabel();//this relabels only if reuse = false, otherwise only reinsert seeds
			//info.relabel_t.pause();
			//
			info.gap_t.resume();
			//update global hist for new labels of r
			for(int v=0;v<R[r].nV;++v){//go over inner vertices
				int d = R[r].g->nodes[v].d;
				if(d<d_inf){
					++d_hist[d];
				};
			};
			//
			//find a gap
			global_gap = d_inf;
			//for(tbucket<seed_hi_pr::node> *b = R[r].g->buckets.last->prev; b!=R[r].g->buckets.first;b = b->prev){
			//for(seed_hi_pr::tbucket *b = R[r].g->buckets.first->next; b!=R[r].g->buckets.last;b=b->next){

			for(int gap=1;gap<d_hist.size();++gap){
				if(d_hist[gap]==0){//this is a global gap
					for(int q=0;q<G.nR;++q){
						R[q].global_gap = std::min(R[q].global_gap,gap);
					};
					R[r].global_gap = gap;
					if(gap<=R[r].g->buckets.last->prev->d){
						R[r].reuse = false;
					};
					break;
				};
			};
			
			info.gap_t.pause();
			//R[r].relabel();
			//
			//send messages out of r
			info.msg_t.resume();
			for(int j=0;j<G[r].out.size();++j){//over outcoming r-edges of region r
				region_edge * ee = &(G[r].out[j]); // outcoming r->q edge
				for(int k=0;k<ee->emap.size();++k){ // over all boundary edges of this r-edge
					int e_r = ee->emap[k][0]; // local edge index in r
					int & f = ee->msg[k].f;
					int & d = ee->msg[k].d;
					R[r].msg_out(e_r,d,f); // remember updated distance and flow
#ifndef NDEBUG
					if(!unload){
						int e_q = ee->emap[k][1];
						int d_q = R[ee->head].d_out(e_q);
						int & d_r = R[r].g->arcs[e_r].head->d;
						assert(d_q==d_r || (d_r==d_inf && d_q>R[ee->head].global_gap));
						if(f>0){
							assert(R[ee->head].d_out(e_q) < d_inf);
						};
					};
#endif
					if(d>=R[r].global_gap){
						assert(d>R[r].global_gap);
						d = d_inf;//to the region itself the gap will be applied later, but for the outside already now
					};
					if(f>0){
						G[ee->head].has_active = true;
					};
				};
			};
			info.msg_t.pause();
			//
			//
			//count dead nodes
			R[r].dead = 0;
			for(int i=0;i<R[r].nV;++i){//only inner nodes
				//if(R[r].g->nodes[i].tr_cap>0 && R[r].d[i]==d_inf){
				if(R[r].g->nodes[i].d==d_inf){
					++R[r].dead;
				};
			};
			dead+=R[r].dead;
			//
			//
			//
			//some_active = some_active || R[r].has_active;
			//save remained network
			info.diskw_t.resume();
			//R[r].unload(G[r].id);// save and free memory
			dynamic::memserver::set_allocator(memserver::get_al_blocks());
			if(unload){
				info.diskw_b += page.unload((G[r].id+".page").c_str());//saves all R[r]
			};
			info.diskw_t.pause();
		};
		nractive = 0;
		for(int r=0;r<G.nR;++r){
			if(G[r].has_active){
				++nractive;
			}
		};
		char s [10];
		sprintf(s,"%4.1f",dead*100.0/G.nV);
		debug::stream<<"sweep: "<<sweep<<" flow: "<<flow<<" decided: "<<s<<"% active_regions: "<<nractive<<"\n";
		++sweep;
	}while(nractive>0);
	info.solve_t.stop();
	info.sweeps = sweep;
	info.flow = flow;
	info.nV = G.nV;
	info.nE = G.nE;
	info.nBE = G.nBE;
	info.mem_shared = memserver::get_al_blocks().mem_used();
	return flow;
};

long long stream_PRD::cut_cost(){
	long long cost = 0;
	for(int r=0;r<G.nR;++r){
//		load_region(r);
		cost += R[r].cut_cost();
//		unload_region(r);
	};
	return cost;
};
*/
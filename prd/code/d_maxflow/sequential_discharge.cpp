#include "sequential_discharge.h"
#include "debug/performance.h"
#include "dynamic/block_allocator.h"

#include <omp.h>
//#include "seed_BK1.h"

using namespace dynamic;

template<class RD> sequential_discharge<RD>::sequential_discharge(){
	info.name = "sequential_discharge";
};

template<class RD> void sequential_discharge<RD>::construct(region_graph * G, bool unload){
	this->G = G;
	this->unload = unload;
	d_inf = 0;
	d_min = 0;
};

template<class RD> void sequential_discharge<RD>::set_tmp(const char * tmp_pth){
	this->tmp_pth = tmp_pth;
};

template<class RD> void sequential_discharge<RD>::init(){
	assert(d_inf>0);
	info.construct_t.resume();
	R.resize(G->nR);
	this->S = G->S;
	this->T = G->T;
	long long max_size = 0;
	//calculate maximum required mem per region == page_size
	//init messages to zero flow and label
	for(int r=0;r<G->nR;++r){
		max_size = std::max(max_size,RD::size_required((*G)[r].nV,(*G)[r].nB,(*G)[r].nE,(*G)[r].nE_out,d_inf));
		(*G)[r].page = &page1;
		for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary edges of region r
			region_graph::arc &a = (*G)[r].out[j];
			a.d = d_min;
			a.f = 0;
		};
		(*G)[r].global_gap = d_inf;
		(*G)[r].max_d = d_min;
	};
	long long required = max_size;
	if(!unload){
		required = required*G->nR;
	};
	required+=20*1024*1024;
	//debug::stream<<"mem required (regions): "<<double(required)/1024/1024<<"Mb\n";
	if(unload){
        page1.reserve(required);
	};
	info.construct_t.pause();
};

template<class RD> sequential_discharge<RD>::~sequential_discharge(){
	//instead of loading the objects and deallocating them correctly
	//just clear all pointers, memory is freed by deallocating the page
	for(int r=0;r<G->nR;++r){
		if((*G)[r].loaded){
			dynamic::memserver::set_allocator(*(*G)[r].page);
			delete R[r];
			dynamic::memserver::set_allocator(memserver::get_al_blocks());
		};
		R[r] = 0;
	};
};

template<class RD> void sequential_discharge<RD>::construct_region(int r){
	info.construct_t.resume();
	info.solve_t.pause();
	dynamic::memserver::set_allocator(*(*G)[r].page);
	R[r] = new RD();
	R[r]->d_inf = d_inf;
	R[r]->nV = (*G)[r].nV;
	R[r]->nB = (*G)[r].nB;
	R[r]->nE = (*G)[r].nE;
	R[r]->nE_out = (*G)[r].nE_out;
	R[r]->load(G->pth + (*G)[r].id);
	dynamic::memserver::set_allocator(memserver::get_al_blocks());
	info.mem_region = std::max((*G)[r].page->mem_used(),info.mem_region);
	//replace indices in the emaps to the indices assigned during allocation
	for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary arcs of region r
		region_graph::arc * a = &((*G)[r].out[j]); // outcoming r->q edge
		int be = a->local_index-(R[r]->nE-R[r]->nE_out);
		a->local_index = R[r]->E_out[be];
	};
	info.construct_t.pause();
	info.solve_t.resume();
};

template<class RD> void sequential_discharge<RD>::load_region(int r){
	if((*G)[r].loaded){
		return;
	};
	if(!(*G)[r].constructed){
		construct_region(r);
		(*G)[r].constructed = true;
		(*G)[r].loaded = true;
		(*G)[r].file_type = 1;
		return;
	};
	//
	info.diskr_t.resume();
	std::string pagename = std::string(tmp_pth)+(*G)[r].id+".page";
	if((*G)[r].file_type == 0){
		dynamic::memserver::set_allocator(*(*G)[r].page);
		info.diskr_b += R[r]->load((*G)[r].id);
		dynamic::memserver::set_allocator(memserver::get_al_blocks());
		(*G)[r].file_type = 1;
	}else{//file_type = 1
		info.diskr_b += (*G)[r].page->load(pagename.c_str());
	};
	(*G)[r].loaded = true;
	info.diskr_t.pause();
};

template<class RD> void sequential_discharge<RD>::unload_region(int r){
	if(!(*G)[r].loaded)return;
	if(unload){
		info.diskw_t.resume();
		if((*G)[r].file_type == 0){
			dynamic::memserver::set_allocator(*(*G)[r].page);
			info.diskw_b += R[r]->unload((*G)[r].id);
			dynamic::memserver::set_allocator(memserver::get_al_blocks());
		}else{
			std::string pagename = std::string(tmp_pth)+(*G)[r].id+".page";
			info.diskw_b += (*G)[r].page->unload(pagename.c_str());
		};
		(*G)[r].loaded = false;
		info.diskw_t.pause();
	};
};

template<class RD> void sequential_discharge<RD>::msg_in(int sweep,int r){
	//collect flow and boundary distance labels from neighboring regions
	for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary arc of region r
		region_graph::arc * a = &(*G)[r].out[j]; // outcoming r->q edge
		int e_r = a->local_index;
		tcap & f = a->rev->f;
		int d = a->rev->d; // d is local, the message is not modified
		if(d == d_inf-1){
			d = d_inf;
		};
		//apply best known gap on d
		//if(d<(*G)[a->head_region].global_gap){
		//};
		assert(a->rev->r_cap>=f);
		a->r_cap += f; //here message is modified
		a->rev->r_cap -= f;
		R[r]->msg_in(e_r,d,f);
		f = 0;//remove the flow from the message
	};
	// messages are read, clear flags of incoming updates
	(*G)[r].labels_changed = false;
	(*G)[r].has_active = false;
};

template<class RD> void sequential_discharge<RD>::msg_out(int sweep, int r){
	(*G)[r].max_d = d_min;
	for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary edges of region r
		region_graph::arc * a = &((*G)[r].out[j]); // outcoming r->q edge
		int e_r = a->local_index; // local edge index in r
		a->old_d = a->d;
		int d_old = a->d;
		R[r]->msg_out(e_r,&a->d, &a->f); // read out updated distance and flow
		if(a->d<d_inf){
			(*G)[r].max_d = std::max((*G)[r].max_d,a->d);
		};
		assert(a->d >= d_old);
		if(d_old<a->d){
			(*G)[a->head_region].labels_changed = true;
		};
		if(a->f>0){
			(*G)[a->head_region].has_active = true;
		};
	};
	////handle not fully discharged regions (lazy discharge)
//#pragma omp atomic
	(*G)[r].has_active |= R[r]->has_active;
};

template<class RD> void sequential_discharge<RD>::check_validity(){
#ifndef NDEBUG
	for(int r=0;r<G->nR;++r){
		for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary edges of region r
			region_graph::arc * a = &((*G)[r].out[j]);
			int & d1 = a->d;
			int & f1 = a->f;
			int & d2 = a->rev->d;
			int & f2 = a->rev->f;
			int c1 = a->r_cap-f1+f2;
			int c2 = a->rev->r_cap-f2+f1;
			assert(c1>=0);
			assert(c2>=0);
			if(c1>0 && d1<d_inf)assert(d1<=d2+1);
			if(c2>0 && d2<d_inf)assert(d2<=d1+1);
		};
	};
#endif
};
template<class RD> void sequential_discharge<RD>::hist_dec(int sweep, int r){
};

template<class RD> void sequential_discharge<RD>::hist_inc(int sweep, int r){
};

template<class RD> void sequential_discharge<RD>::global_gap(){
};

template<class RD> void sequential_discharge<RD>::process_region(int sweep,int r, bool flow_optimal){
};

template<class RD> typename sequential_discharge<RD>::tflow sequential_discharge<RD>::maxflow(){
	init();
	debug::stream<<"Regions: "<<G->nR<<"\n";
	//
	info.solve_t.start();
	//
	dead = 0;
	dead0 = 0;
	int sweep = 0;
	nractive = G->nR;
	nrchanged = G->nR;
	double cpu=0;
	double cpu0=0;
	bool flow_optimal = false;
while(nractive>0 || nrchanged >0){
		flow = 0;
		dead = dead0;
		for(int r = 0;r<G->nR;++r){
			if((*G)[r].has_active || (flow_optimal && (*G)[r].labels_changed)){
			//if((*G)[r].has_active || flow_optimal){
				load_region(r);
				//
				info.msg_t.resume();
				msg_in(sweep,r);
				info.msg_t.pause();
				//
				hist_dec(sweep,r);
				//
				info.augment_t.resume();
				process_region(sweep,r,flow_optimal);
				info.augment_t.pause();
				//
				info.msg_t.resume();
				msg_out(sweep,r);
				info.msg_t.pause();
				//
				unload_region(r);
				hist_inc(sweep,r);
				global_gap();
			};
			dead+=(*G)[r].dead;
			flow+=(*G)[r].flow;
		};
		if(info.relabel_t.time()<0.1*info.solve_t.time()){
			global_heuristic();
		}else{
			n_relabelled=-1;
		};
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
		info.msg_t.pause();
		info.solve_t.pause();
		char s [20];
		sprintf(s,"%4.1f",dead*100.0/G->nV);
		debug::stream<<"sweep: "<<sweep<<" flow: "<<flow<< " decided: "<<s<<"% active: "<<nractive<<" " " changed: "<<nrchanged;
		if(n_relabelled>=0){
			debug::stream<<" relabelled: "<<n_relabelled;
		};
		cpu = info.augment_t.time()+info.msg_t.time()+info.gap_t.time()+info.relabel_t.time();
		debug::stream<<" CPU: "<<cpu-cpu0<<"sec.";
		cpu0=cpu;
		debug::stream<<"\n";
		info.solve_t.resume();
		if(nractive==0 && !flow_optimal){
			info.solve_t.pause();
			debug::stream<<"flow is optimal\n";
			info.solve_t.resume();
			flow_optimal = true;
			flow_is_maximum();
		};
		++sweep;
	};
	info.solve_t.stop();
	//
	debug::stream<<"Total CPU: "<<cpu<<"\n";
	info.sweeps = sweep;
	info.flow = flow;
	info.nV = G->nV;
	info.nE = G->nE;
	info.nBE = G->nBE;
	info.mem_shared = memserver::get_al_blocks().mem_used();
	return flow;
};

template<class RD> void sequential_discharge<RD>::save_cut(const std::string & filename){
	FILE * f = fopen(filename.c_str(),"wt+");
	setvbuf(f,NULL,_IOFBF,1024*1024*2);
	fprintf(f,"p max %i %i\n",G->nV,G->nE);
	fprintf(f,"c minimum cut, generated by %s\n",info.name.c_str());
	fprintf(f,"f %lli\n",info.flow);
	fprintf(f,"n 1 1\n");//source has label 1
	fprintf(f,"n 2 0\n");//sink has label 0
	for(int r=0;r<G->nR;++r){
		load_region(r);
		for(int v=0;v<(*G)[r].nV;++v){
			fprintf(f,"n %i %i\n",R[r]->g_index(v)+1,R[r]->is_weak_source(v));
		};
		unload_region(r);
	};
};

template<class RD> void sequential_discharge<RD>::get_cut(int * C){
	C[S] = 1;
	C[T] = 0;
	for(int r=0;r<G->nR;++r){
		load_region(r);
		for(int v=0;v<(*G)[r].nV;++v){
			int i = R[r]->g_index(v);
			C[i] = R[r]->is_weak_source(v);
		};
		unload_region(r);
	};
};

template<class RD> long long sequential_discharge<RD>::cut_cost(){
	long long cost = 0;
	for(int r=0;r<G->nR;++r){
		load_region(r);
		cost += R[r]->cut_cost();
		unload_region(r);
	};
	return cost;
};

//template sequential_discharge<seed_BK>;

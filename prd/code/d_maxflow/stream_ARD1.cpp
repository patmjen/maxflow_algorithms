#include "stream_ARD1.h"

using namespace dynamic;
#include "sequential_discharge.cpp"

template class sequential_discharge<seed_BK>;

stream_ARD1::stream_ARD1(){
	info.name = "S-ARD1";
};

void stream_ARD1::init(){
	d_inf = std::max(G->nB,1);
	d_hist.resize(d_inf);
	d_hist << 0;
	d_hist[0] = 2*G->nBE;
	parent::init();
};

void stream_ARD1::hist_dec(int sweep, int r){
	//subtract r from global label hist
	//this assumes labeling of inner nodes has preserved since last touch of r
	//it is actually stored in the outcoming messages of r
	info.gap_t.resume();
	for(region_graph::arc * a = (*G)[r].out.begin();a<(*G)[r].out.end();++a){//over outcoming boundary edges of region r
		if(a->d<d_inf){
			--d_hist[a->d];
			assert(d_hist[a->d]>=0);
		};
	};
	info.gap_t.pause();
};

void stream_ARD1::hist_inc(int sweep,int r){
	info.gap_t.resume();
	//update global hist for new labels of r
	for(int j=0;j<(*G)[r].out.size();++j){//over outcoming boundary edges of region r
		region_graph::arc * a = &((*G)[r].out[j]);
		if(a->d<d_inf){
			++d_hist[a->d];
		};
	};
	info.gap_t.pause();
};

void stream_ARD1::global_gap(){
	info.gap_t.resume();
	// detect global gap and apply it to all boundary nodes
	for(int g=0;g<d_inf;++g)assert(d_hist[g]>=0);
	int gap = 0;
	for(;gap<d_inf && d_hist[gap]>0;++gap);
	//gap is a global gap, all above must be lifted to d_inf
	if(gap<d_inf){
		//apply to all immediately
		for(int r=0;r<G->nR;++r){//applied globally
			(*G)[r].global_gap = std::min((*G)[r].global_gap,gap);
			if(gap < (*G)[r].max_d){
				for(region_graph::arc * a = (*G)[r].out.begin(); a<(*G)[r].out.end(); ++a){//over outcoming boundary edges of region r
					int & d = a->d;
					if(d<d_inf && d>gap){
						--d_hist[d];
						assert(d_hist[d]>=0);
						d = d_inf;
						(*G)[a->head_region].labels_changed = true;
					};
				};
				(*G)[r].global_gap = d_inf;
			};
		};
	};
	info.gap_t.pause();
};

void stream_ARD1::process_region(int sweep, int r, bool flow_optimal){
	R[r]->discharge(flow_optimal?d_inf:sweep);
	(*G)[r].flow = R[r]->flow;
	(*G)[r].dead = R[r]->dead;
};

//! boundary relabel heuristic
void stream_ARD1::global_heuristic(){
	info.relabel_t.resume();
	g_cache.resize(d_inf);
	groups_pool.reserve(G->nB);
	groups_pool.clear();
	//if(groups_pool.size()==0)
	g_free = 0;
	dynamic::fixed_array1<region_graph::d_group*> open;
	open.reserve(G->nB);
	//
	for(int r = 0;r<G->nR;++r){
		region_graph::region & R = (*G)[r];
		//set region_arc->next according to distance group
		for(region_graph::arc * a = R.out.begin(); a<R.out.end();++a){
			if(a->d<d_inf){
				region_graph::d_group * g = g_cache[a->d];
				if(!g){//no group allocated yet
					if(g_free){
						g = g_free;
						g_free = g_free->g_next;
					}else{
						g = groups_pool.push_back();
					};
					g_cache[a->d] = g;
					g->arc_first = 0;
					if(a->d>0){
						g->relabel_d = d_inf;
						g->is_open = false;
					}else{
						g->relabel_d = 0;
						//add this group to open
						//g->next_open = open;
						//open = g;
						open.push_back(g);
						g->is_open = true;
					};
				};
				a->next = g->arc_first;
				g->arc_first = a;
				a->group = g;
			};
		};
		//link groups in the region and clear g_cache
		R.g_first = 0;
		for(int d = 0;d<d_inf;++d){
			if(region_graph::d_group * g = g_cache[d]){
				g->g_next = R.g_first;
				R.g_first = g;
				g_cache[d] = 0;
			};
		};
	};
	//constructed and initialized
	for(int o1=0,o2=open.size(); o1<o2; o1=o2,o2=open.size()){// all gropus from o1 to o2 form a level with the same relabel_d
		//process neighbours at distance 0 (those which are in the same region)
		for(int o=o1;o<o2;++o){
			region_graph::d_group * U = open[o];
			for(region_graph::d_group * V = U->g_next; V!=0; V=V->g_next){// all groups below U
				// it might be that V->U but not the other way around
				// relabel V
				if(V->relabel_d > U->relabel_d){// this link has length 0
					V->relabel_d = U->relabel_d;
					if(!V->is_open){
						assert(open.back()->relabel_d <= V->relabel_d);
						open.push_back(V);
						V->is_open = true;
					};
				};
			};
		};
		o2 = open.size();
		//process neighbours at distance 1
		for(int o=o1;o<o2;++o){
			region_graph::d_group * U = open[o];
			// find a better label for neighbors
			// in the other region
			for(region_graph::arc * a = U->arc_first; a!=0; a=a->next){//outcoming arcs
				//assert(a->d <= U->relabel_d);
				if( a->rev->r_cap-a->rev->f+a->f > 0 && a->rev->d<d_inf){
					region_graph::d_group * V = a->rev->group;
					if(V->relabel_d > U->relabel_d+1){
						V->relabel_d = U->relabel_d+1;
						if(!V->is_open){
							assert(open.back()->relabel_d <= V->relabel_d);
							open.push_back(V);
							V->is_open = true;
						};
					};
				};
			};
		};
	};
	n_relabelled=0;
	//all groups relabelled
	//set the improved labels to messages
	for(int r=0;r<G->nR;++r){
		for(region_graph::arc * a = (*G)[r].out.begin(); a<(*G)[r].out.end(); ++a){
			int & d = a->d;
			if(d<d_inf){
				int new_d = a->group->relabel_d;
				if(d < new_d){// take max of the two lower bounds: d = max(d,new_d)
					--d_hist[d];
					assert(d_hist[d]>=0);
					d = new_d;
					if(d<d_inf){
						++d_hist[d];
					};
					++n_relabelled;
					(*G)[a->head_region].labels_changed = true;
				};
			};
		};
	};
	info.relabel_t.pause();
};

/*
A reimplementation of Goldberg and Tarjans and Goldbegr and Cherkassky's HI-PR algorithm.

Alexander Shekhovtsov
*/

#include "hpr1.h"
#include <algorithm>

template<class defs> hpr<defs>::hpr(){
	c1.start();c1.pause();
	c2.start();c2.pause();
	c3.start();c3.pause();
};

template<class defs> void hpr<defs>::global_update(){
	c2.resume();
	buckets.clear();
	for(int u=0;u<this->nV;++u){//only regular nodes, excluding boundary
		node * i = &this->nodes[u];
		i->current = arc_first(i);
		if(i->excess<0){//linked to sink - inactive
			i->d = 1;
			buckets.iAdd(i);
		}else{//assume the node is unreachable
			i->d = d_inf;
			i->set_state(bucket_node::dead);
		};
	};
	//now go in order of increasing distance, relabeling free nodes
	for(tbucket * b = buckets.first()+1; !b->isempty(); ++b){//go through all buckets; more buckets are being added
		tbucket * c = b+1; // next bucket
		for(int uactive=0;uactive<2;++uactive){//go through active and inactive lists of bucket b
			node *u;
			if(uactive){
				u = b->first_active;
			}else{
				u = b->first_inactive;
			};
			for(; u!=0; u=u->bnext){//each node in the bucket
				for(arc_ptr a = arc_first(u); allowed(a); ++a){//all outcoming arcs
					if(has_head(u,a) && arc_rev(a)->resCap>0){//has residual incoming arc
						node * v = arc_head(u,a);
						if(v->d==d_inf && v-this->nodes.begin()<this->nV){//non-boundary node, unlabelled
							v->d = u->d+1;
							v->set_state(node::free);
							if(v->excess>0){//add to active
								buckets.aAdd(v);
							}else{
								assert(v->excess==0);//negative excess is connected to sink and has allready been initialized before
								buckets.iAdd(v);
							};
						};
					};
				};
			};
		};
	};
	c2.pause();
};


template<class defs> hpr<defs>::~hpr(){
};

template<class defs> void hpr<defs>::push1(node *u, arc_ptr & a, node *v){
	//v = arc_head(u,a);
	assert(v->d<d_inf);
	tcap f = a->resCap;
	if(f>=u->excess){//non-saturating push
		f = u->excess;
	};
	a->resCap-=f;
	u->excess-=f;
	arc_rev(a)->resCap+=f;
	tcap & e = v->excess;
	if(e<=0){
		if(e+f>0){//was inactive, became active
			this->flow -= e;
			if(v-this->nodes.begin()<this->nV){//if not a boundary node
				buckets.activate(v);//v below u, dont update top_active
			};
		}else{ // e+f<=0 still, remains inactive! (immediately put to the sink)
			this->flow += f;
		};
	}else{//was active, add more excess, keep bucket
	};
	e+=f;
};

//! gap heuristic here
template<class defs> typename hpr<defs>::tbucket * hpr<defs>::gap(tbucket * b){
	//found a gap
	//all buckets starting from b and until b->next_seed should be lifted to d_inf
	for(tbucket * c = b; !c->isempty();++c){
		dismiss_bucket(c);
	};
	//top_active bucket and everything above went to d_inf
	buckets.top_active = b-1;
	return 0;
};

template<class defs> void hpr<defs>::dismiss_bucket(tbucket * a){
	for(node * i=a->first_active;i!=0;i=i->bnext){
		i->set_state(bucket_node::free);
		elliminate(i);
	};
	a->first_active = 0;
	for(node * i=a->first_inactive;i!=0;i=i->bnext){
		i->set_state(bucket_node::free);
		elliminate(i);
	};
	a->first_inactive = 0;
};

template<class defs> void hpr<defs>::stageOne(bool reuse){
	if(!reuse){
		global_update();
	};
	//
	//assume all data structures are correctly initialized
	//
	c3.resume();
	while(1){
		//find active node
		node * u;
		while(1){
			if(buckets.top_active==buckets.first())return;//reached the bottom - no more active
			u = buckets.top_active->first_active;
			if(u)break;//found active node
			//search downwards to get next active node
			//more than one step might be needed only if some node went to d_inf
			--buckets.top_active;
		};
		assert(u->excess>0);
		assert(u->d<d_inf);
		while(1){//in fact, while u has excess
			node * v_min = 0; // this will search for minimal residual arc to d<d_inf
			int d_min = d_inf;
			//
			//go through all arcs out of u, if at distance u->d-1 push flow
			for(arc_ptr a = u->current; allowed(a); ++a){//all outcoming arcs
				if(a->resCap>0){
					node * v = arc_head(u,a);
					int d = v->d;
					if(d==u->d-1){
						push1(u,a,v);
						if(u->excess==0){
							//u stays in the same bucket but deactivated
							buckets.aDelete(u);
							buckets.iAdd(u);
							u->current = a;
							goto dblbreak; //break if push was non-saturating (u.excess==0)
						};
						//push has saturated arc a
						assert(a->resCap==0);
						//so we dont consider it for v_min
					}else if(d<d_inf){//this is a good residual arc
						if(d<d_min){
							v_min = v;
							d_min = d;
						};
					};
				};
			};
			//no more arcs at level u->d-1 but u has excess
			//u will be relabelled
			//u is active, there's a gap only if u is the last in its bucket
			tbucket * b = buckets.node_bucket(u);
			if(!u->bnext && !b->first_inactive){//found gap!
				assert(u==b->first_active);
				gap(b);
				//all above the gap, including u goes to d_inf
				break; // to next active node
			};
			//no gap, u will be relabeled by the neighbors
			buckets.aDelete(u);
			//search the rest of arcs down from u->current for residual with min label
			for(arc_ptr a = arc_first(u); a!=u->current; ++a){
				if(a->resCap>0){
					node *v = arc_head(u,a);
					int d = v->d;
					if(d<d_min){
						d_min = d;
						v_min = v;
					};
				};
			};
			if(v_min){//relabel after v_min
				u->d = d_min+1;
				//if(u->d == d_inf){//happens when no global gap heuristic
				//	elliminate(u);
				//	break;
				//};
				assert(u->d < d_inf);//because there is no gap
				buckets.aAdd(u);//jump to v+1's bucket
				//this raises top_active to v->d+1
				u->current = arc_first(u);
				continue; // u remains top active
			}else{// no alive neighbours
				elliminate(u); //we've done with this one
				break;
			};
		};
dblbreak:;
	};
	c3.pause();
};

template<class defs> void hpr<defs>::allocate1(int nV, int nB, int nE, int nE_out, int *E_out, int d_inf, int S, int T){
	if(nB==0 && d_inf==0)d_inf = nV+1;
	this->d_inf = d_inf;
	tgraph::allocate1(nV,nB,nE,nE_out,E_out,S,T);
	//
	buckets.init(this->n+2,d_inf);//+2 for 0 and d_inf buckets
};

template<class defs> tflow hpr<defs>::cut_cost(int * C){
	return 0;
};


template<class defs> tflow hpr<defs>::cut_cost(){
	tflow cost = this->flow;
	for(node * u = this->nodes.begin(); u<this->nodes.end();++u){
		if(is_weak_source(u)){
			for(arc_ptr a = arc_first(u);allowed(a);++a){
				if(has_head(u,a)){
					node * v = arc_head(u,a);
					if(!is_weak_source(v)){
						cost += a->resCap;
					};
				};
			};
		};
	};
	return cost;
};

template<class defs> long long hpr<defs>::size_required(int nV, int nB, int nE, int nE_out,int d_inf){
	long long r = 0;
	r+=sizeof(hpr);
	r+=tgraph::size_required(nV,nB,nE,nE_out);
	r+=array_buckets<node>::size_required(nV+nB+2,d_inf);// buckets
	r+=sizeof(node*)*(nB);// BS
	return r;
};

template class hpr<>;
template class hpr<hpr_defs<base_graph_n> >;

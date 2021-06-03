/*
A reimplementation of Goldberg and Tarjans and Goldbegr and Cherkassky's HI-PR algorithm.

Incorporates seeds - nodes with fixed label, which may not be relabeled 
(needed in Region discharge algorithms). The bucket data structure is 
implemented as double-linked list rather than array. This allows to perform
relabel and gap heuristics using O(n) memory, while labels can grow up to
maximum_seed + n.

This implementation is adjusted to vision graphs (all vertices except source
and sink have low degree, source and sink may be connected to all other). The
terminal links are represented implicitly by the excess/deficit.

Alexander Shekhovtsov
*/

#include "seed_hi_pr.h"
//#include "inline_heap.h"

seed_hi_pr::seed_hi_pr(){
	c1.start();c1.pause();
	c2.start();c2.pause();
	c3.start();c3.pause();
};

void seed_hi_pr::init_arc(seed_hi_pr::node *u, seed_hi_pr::node *v, seed_hi_pr::arc* uv, seed_hi_pr::arc * vu, tcap cap1, tcap cap2){
	uv->head = v;
	vu->head = u;
	uv->resCap = cap1;
	vu->resCap = cap2;
	uv->rev = vu;
	vu->rev = uv;
};

void seed_hi_pr::construct(int nV, int nE, const int * E, const tcap *cap, const tcap * excess, int nB, int nE_out, int * E_out, int d_inf){
	if(d_inf==0)d_inf = nV+nB+1;
	this->d_inf = d_inf;
	this->nV = nV;
	this->nB = nB;
	n = nV+nB;
	m = 2*nE;
	flow = 0;
	nodes.resize(n+1);
	arcs.resize(m);
	buckets.init(n+2,d_inf);//+2 for 0 and d_inf buckets
	//
	BS.reserve(nB);
	//init nodes
	flow =0;
	for(int v=0;v<n;++v){
		if(v<nV){//regular node
			nodes[v].excess = excess[v];
			if(excess[v]<0){
				flow+=excess[v];
			};
		}else{
			BS.push_back(&nodes[v]);
			nodes[v].excess = 0;
		};
		nodes[v].arc_first = 0;
	};
	//cont number of outcoming arcs
	for(int e=0;e<nE;++e){
		assert(E[2*e]>=0 && E[2*e]<n);
		assert(E[2*e+1]>=0 && E[2*e+1]<n);
		node * u = &nodes[E[2*e]];
		node * v = &nodes[E[2*e+1]];
		++(size_t&)u->arc_first;//for now will count how many outcoming arcs
		++(size_t&)v->arc_first;
	};
	//
	//compute desired starting positions for where to allocate arcs
	size_t accum_size = 0;
	for(node * v = nodes.begin();v!=nodes.end();++v){
		size_t s = (size_t&)v->arc_first;
		v->arc_first = arcs.begin()+accum_size;
		v->arc_end = v->arc_first;
		accum_size+=s;
	};

	//init arcs
	for(int e=0;e<nE;++e){
		node * u = &nodes[E[2*e]];
		node * v = &nodes[E[2*e+1]];
		arc *& uv = u->arc_end;
		arc *& vu = v->arc_end;
		tcap cap1 = cap[2*e];
		tcap cap2 = cap[2*e+1];
		if(e>=nE-nE_out){//this is outcoming boundary arc
			cap2=0;
			if(E_out){
				E_out[e-(nE-nE_out)] = uv-arcs.begin();//save the new position of this arc
			};
		};
		init_arc(u,v,uv,vu,cap1,cap2);
		++uv;
		++vu;
	};
	////init arcs
	//int m=0;
	////internode arcs
	//for(int e=0;e<nE;++e){
	//	node * u = &nodes[E[2*e]];
	//	node * v = &nodes[E[2*e+1]];
	//	arc * uv = &arcs[m];
	//	arc * vu = &arcs[m+1];
	//	tcap cap1 = cap[2*e];
	//	tcap cap2 = cap[2*e+1];
	//	if(e>=nE-nE_out)cap2=0;
	//	init_arc(u,v,uv,vu,cap1,cap2);
	//	m+=2;
	//};
	//// Algorithm for inplace arc reordering
	////
	//
	//for(int e=0;e<nE_out;++e){
	//	E_out[e] = -1;
	//};
};

__forceinline bool seed_hi_pr::is_out_bnd(arc * a){
	return a->head-nodes.begin()>=nV;
};

void seed_hi_pr::inplace_edge_reorder(int nE_out, int * E_out){
	int nE = arcs.size()/2;
	for(int v=0;v<n;++v){
		nodes[v].arc_first = 0;//for now will count how many outcoming arcs
	};
	//cont number of outcoming arcs = number of incoming arcs
	for(arc * a =arcs.begin();a!=arcs.end();){//all directed arcs
		//++(size_t&)a->rev->head->arc_first;//for now will count how many outcoming arcs
		++(size_t&)a->head->arc_first;
	};
	//
	//compute desired starting positions for where to place arcs
	size_t accum_size = 0;
	for(node * v = nodes.begin();v!=nodes.end();++v){
		size_t s = (size_t&)v->arc_first;
		v->arc_first = arcs.begin()+accum_size;
		v->arc_end = v->arc_first;
		accum_size+=s;
	};
	//nodes[v].arc_first is now the address where outcoming arcs of v will start after reordered
	//
	int ind=0;
	for(arc * a =arcs.begin();a!=arcs.end();){//each arc is checked at most twice
		node * i = a->rev->head;
		//arc a goes out of node i
		if(a>=i->arc_first && a<i->arc_end){//already in place (either was considered before or was never moved)
			//update its index
			if(is_out_bnd(a) && ind-(nE-nE_out)>=0){//head is in boundary
				E_out[ind-(nE-nE_out)] = a-arcs.begin();
			};
			++a;// advances only here
			ind = a-arcs.begin();
		}else{//arc a is not in place
			arc * b = i->arc_end; // this is where we want to put a
			node * j = b->rev->head; // arc b comes out of node j
			if(i!=j){//arc in pos b does not originate from i
				// will swap arcs a and b
				// ind is the original index of a
				int ind2 = b-arcs.begin(); // the new index of a
				if(is_out_bnd(a) && ind>=nE-nE_out){
					//save new index of the reordered arc
					E_out[ind-(nE-nE_out)] = ind2;
				};
				std::swap(a->head,b->head);//swap arc content
				std::swap(a->resCap,b->resCap);//swap arc content
				if(a->rev!=b){
					std::swap(a->rev,b->rev);
					//restore rev pointers to a and b
					a->rev->rev = a;
					b->rev->rev = b;
				};
				//now 'a' holds new homeless arc
				ind = ind2;//its original index
			}else{//adopt b in this position
				//b has never been moved - it preserved its index
				int ind2 = b-arcs.begin();
				if(is_out_bnd(b) && ind2>=nE-nE_out){
					E_out[ind2-(nE-nE_out)] = ind2;
				};
				//ind keeps original index of a
			};
			++i->arc_end;
		};
	};
	//reordered!
};

struct node_less{
	bool operator()(
		const seed_hi_pr::node * a,	const seed_hi_pr::node * b)const{
			return a->d<b->d;
	};
};

void seed_hi_pr::global_update(){
	c2.resume();
	//assume BS is list of seeds
	//sort seeds by increase of distance
	std::sort(BS.begin(),BS.end(),node_less());
	buckets.clear();
	//put seeds into buckets
	for(node ** u = BS.begin();u!=BS.end();++u){
		if((*u)->d<d_inf){
			buckets.iAdd(*u,buckets.last->prev);//boundary is inactive
		};
	};
	//set next_seed likns
	for(tbucket * b=buckets.first;b!=buckets.last;b=b->next){
		b->next_seed = b->next;
	};
	for(int u=0;u<nV;++u){//only regular nodes, excluding boundary
		node * i = &nodes[u];
		i->current = i->arc_first;
		if(i->d < d_inf){
			if(i->excess<0){//linked to sink - inactive
				i->d = 1;
				buckets.iAdd(i,buckets.first);
			}else{//free node
				i->d = d_inf+1;
			};
		};
	};
	//now go in order of increasing seeds, relabeling free nodes
	for(tbucket * b = buckets.first; b!=buckets.last; b = b->next){//go through all buckets; more buckets are being added
		if(b->d==d_inf-1){// need a global gap heuristic to remove this check
			break;//the rest is d_inf
		};
		tbucket * c = 0;
		if(b->next->d==b->d+1){
			c = b->next;
		};
		for(int uactive=0;uactive<2;++uactive){//go through active and inactive lists of bucket b
			node *u;
			if(uactive){
				u = b->first_active;
			}else{
				u = b->first_inactive;
			};
			for(; u!=0; u=u->bnext){//each node in the bucket
				for(arc * a = u->arc_first; a!=u->arc_end; ++a){//all outcoming arcs
					if(a->rev->resCap>0){//has residual incoming arc
						node * v = a->head;
						//assert(v->d != d_inf);
						if(v->d==d_inf+1 && v-nodes.begin()<nV){//non-boundary node, unlabelled
							v->d = b->d+1;
							if(!c){//no bucket yet for this distance
								c = buckets.add_bucket_after(b,b->d+1);
							};
							if(v->excess>0){
								//buckets.aAdd(v,b);
								v->my_bucket = c;
								buckets.aRestore(v);
							}else{
								assert(v->excess==0);//negative excess connected to sink allready initialized before
								//buckets.iAdd(v,b);
								v->my_bucket = c;
								buckets.iRestore(v);
							};
						};
					};
				};
			};
		};
	};
	for(int u=0;u<nV;++u){//only regular nodes, excluding boundary
		node * i = &nodes[u];
		if(i->d == d_inf+1){
			elliminate(i);
		};
	};
	c2.pause();
};

seed_hi_pr::~seed_hi_pr(){
};

void seed_hi_pr::push1(node *u, arc *a, node *&v){
	v = a->head;
	assert(v->d<d_inf);
	tcap f = a->resCap;
	if(f>=u->excess){//non-saturating push
		f = u->excess;
	};
	a->resCap-=f;
	u->excess-=f;
	a->rev->resCap+=f;
	tcap & e = v->excess;
	if(e<=0){
		if(e+f>0){//was inactive, became active
			flow -= e;
			if(v-nodes.begin()<nV){//if not a boundary node
				buckets.activate(v);//v below u, dont update top_active
			};
		}else{ // e+f<=0 still, remains inactive! (immediately put to the sink)
			flow += f;
		};
	}else{//was active, add more excess, keep bucket
	};
	e+=f;
};

//__forceinline 
/*
void seed_hi_pr::elliminate(seed_hi_pr::node *u){
	assert(u->state==bucket_node<node>::free);
	u->d = d_inf;
	u->set_state(bucket_node<node>::dead);
};
*/

//! gap heuristic here
seed_hi_pr::tbucket * seed_hi_pr::gap(tbucket * b){
	//found a gap
	//all buckets starting from b and until b->next_seed should be lifted to next_seed
	tbucket * seed = b->next_seed;
	if(seed!=buckets.last){//there is a seed, can lift only up to it (don't know if it is a global gap)
		for(;b!=seed;){
			buckets.merge_up(b,seed);
			b = buckets.delete_bucket(b);
		};
		//top_active bucket was merged to the seed
		buckets.top_active = seed;
	}else{//raise all to d_inf
		for(;b!=seed;){
			dismiss_bucket(b);
			b = buckets.delete_bucket(b);
		};
		//top_active bucket and everything above went to d_inf
		buckets.top_active = buckets.last->prev;
	};
	return seed;
};

//! global gap
void seed_hi_pr::global_gap(int gap){
	tbucket * b = buckets.last->prev;
	/*
	while(b->d >= gap){
		assert(b!=buckets.first);
		dismiss_bucket(b);
		b = buckets.delete_bucket(b);
	};
	*/
	if(buckets.top_active->d >= gap){//this bucket will be destroyed
		buckets.top_active = buckets.last;
	};//otherwise is intact
	while(b->d >= gap){
		//kill all active in a fast away
		for(node * i = b->first_active;i!=0;i=i->bnext){
			i->set_state(bucket_node<node>::free);
			elliminate(i);
		};
		b->first_active = 0;
		//delete only innter inactive, is a slow way
		for(node * i = b->first_inactive;i!=0;i=i->bnext){
			if(i-nodes.begin()<nV){//inner node
				buckets.iDelete(i);
				elliminate(i);
			};
		};
		if(b->isempty())b = buckets.delete_bucket(b);
		b = b->prev;
	};
	/*
	for(int i=0;i<nV;++i){//inner nodes only
		node * u = &nodes[i];
		int & d = u->d;
		if(d<d_inf && d>gap){
			if(u->excess>0){
				buckets.aDelete(u);
			}else{
				buckets.iDelete(u);
			};
			elliminate(u);
		};
	};
	*/
	/*
	if(buckets.top_active->d > b->d){//this bucket will be destroyed
		buckets.top_active = buckets.last;
	};//otherwise is intact
	//all buckets above b are lifted to d_inf and destryed
	for(;b!=buckets.last;){
		dismiss_bucket(b);
		//if(b->isempty()){//seeds should remain
		b = buckets.delete_bucket(b);
		//}else{
		//	b = b->next;
		//};
	};
	*/
};

//node * fidnd_active();

void seed_hi_pr::stageOne(bool reuse){
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
			u = buckets.top_active->first_active;
			if(u)break;//found active node
			//search downwards to get next active node
			//more than one step might be needed only if some node went to d_inf 
			if(!buckets.top_active->prev)return;//reached the bottom - no more active
			buckets.top_active = buckets.top_active->prev;
		};
		assert(u->excess>0);
		assert(u->d<d_inf);
		while(1){//in fact, while u has excess
			node * v_min = 0; // this will search for minimal residual arc to d<d_inf
			int d_min = d_inf;
			//
			//go through all arcs out of u, if at distance u->d-1 push flow
			for(arc * a = u->current;a!=u->arc_end;++a){
				if(a->resCap>0){
					node * v = a->head;
					int d = v->d;
					if(d==u->d-1){
						push1(u,a,v);
						if(u->excess==0){
							//u stays in the same bucket but deactivated
							buckets.aDelete(u);
							buckets.iRestore(u);
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
			//no more arcs at level d->u-1 but u has excess
			//u will be relabelled
			//u is active, there's a gap only if u isn't the last in its bucket
			tbucket * b = u->my_bucket;
			if(!u->bnext && !b->first_inactive){//found gap!
				assert(u==b->first_active);
				b = gap(b);
				if(b->d==d_inf){//all above the gap, including u goes to d_inf
					break; // to next active node
				};
				buckets.aDelete(u);
				if(d_min<=b->d){//wont find a better d_min, relabel now
					u->d = b->d+1;
					buckets.aAdd(u,b);
					u->current = u->arc_first;
					continue; // u remains top active
				}else{//go further and search the remaining arcs
					//update d_min first
					if(v_min)d_min = v_min->d;
				};
			}else{//no gap
				buckets.aDelete(u);
			};
			//search the rest of arcs down from u->current for residual with min label
			for(arc * a = u->arc_first;a!=u->current;++a){
				node *v = a->head;
				int d = v->d;
				if(a->resCap>0 && d<d_min){
					d_min = d;
					v_min = v;
				};
			};
			if(v_min){//relabel after v_min
				u->d = d_min+1;
				if(u->d == d_inf){//happens when no global gap heuristic
					elliminate(u);
					break;
				};
				assert(u->d < d_inf);//because there is no gap 
				buckets.aAdd(u,v_min->my_bucket);//jump to v+1's bucket
				//this raises top_active to v->d+1
				u->current = u->arc_first;
				continue; // u remains top active
			}else{
				elliminate(u); //we've done with this one
				break;
			};
		};
dblbreak:;
	};
	c3.pause();
};

void seed_hi_pr::dismiss_bucket(tbucket * a){
	for(node * i=a->first_active;i!=0;i=i->bnext){
		i->set_state(bucket_node<node>::free);
		elliminate(i);
	};
	a->first_active = 0;
	for(node * i=a->first_inactive;i!=0;i=i->bnext){
		//if(i-nodes.begin()<nV){
		i->set_state(bucket_node<node>::free);
		elliminate(i);
		//};
	};
	a->first_inactive = 0;
};

void seed_hi_pr::raise_seed(node *i, int d){
	//i is boundary vertex - it is in inactive list
	assert(d>i->d);
	tbucket * b = buckets.iDelete(i);
	if(b->isempty()){
		b = buckets.delete_bucket(b)->prev;
	};
	i->d = d;
	if(d<d_inf){
		buckets.iAdd(i,buckets.find(i->d,b));
	};
	//have to fix next_seed references
	//todo
	assert(0);
};

void seed_hi_pr::clear_seeds(){
	for(node ** u = BS.begin();u!=BS.end();++u){//boundary vertices
		//this was the order in which seeds were added (to the front)
		//guaranteed to delete always from the end (not required)
		if((*u)->d == d_inf )break;
		tbucket * b = buckets.iDelete(*u);
		if(b->isempty() && b!=buckets.last){// I really should not put anything into last
			buckets.delete_bucket(b);
		};
	};
};

void seed_hi_pr::update_seeds(){
	if(nB==0)return;//no seeds
	//tbucket * b = BS[0]->my_bucket;//could have start updates from this, if was not deleting buckets
	//sort seeds by increase of distance
	std::sort(BS.begin(),BS.end(),node_less());
	clear_seeds();
	//put seeds into buckets
	tbucket * b0 = buckets.first;
	tbucket * b = buckets.first;//lowest seeded bucket
	for(node ** u = BS.begin();u!=BS.end();++u){
		//can ignore all seeds with d=d_inf, but might be a problem with clearing later
		//if((*u)->d==d_inf)break;
		if((*u)->d==d_inf)break;
		b = buckets.iAdd(*u,buckets.find((*u)->d,b));
		//b is increased by at least number of steps made by "find"
		//total complexity O(n)
		//update the 'next_seed' pointer for buckets since last seed
		for(;b0!=b;b0=b0->next){
			b0->next_seed = b;
		};
	};
	for(;b0!=buckets.last;b0=b0->next){//some seeds could have been raisen to d_inf
		b0->next_seed = buckets.last;
	};
};

void seed_hi_pr::allocate1(int nV, int nB, int nE, int nE_out, int *E_out, int d_inf, int S, int T){
	this->E_out = E_out;
	this->nE_out = nE_out;
	this->S = S;
	this->T = T;
	if(nB==0 && d_inf==0)d_inf = nV+1;
	this->d_inf = d_inf;
	this->nV = nV;
	this->nB = nB;
	n = nV+nB;
	m = 0;//2*nE;
	flow = 0;
	nodes.resize(n+1);
	buckets.init(n+2,d_inf);//+2 for 0 and d_inf buckets
	//
	BS.reserve(nB);
	flow =0;
	//init nodes
	for(int v=0;v<n;++v){
		nodes[v].arc_first = 0;
		nodes[v].excess = 0;
		if(v>=nV){//add to boundary list
			BS.push_back(&nodes[v]);
		};
	};
};

void seed_hi_pr::reset_counters(){
	m = 0;
};

void seed_hi_pr::allocate2(){
	arcs.resize(m);
	size_t accum_size = 0;
	//compute desired starting positions for where to allocate arcs
	for(node * v = nodes.begin();v!=nodes.end();++v){
		size_t s = (size_t&)v->arc_first;
		v->arc_first = arcs.begin()+accum_size;
		v->arc_end = v->arc_first;
		accum_size+=s;
	};
	assert(accum_size<=m);
	nE = m/2;
};

void seed_hi_pr::add_edge(int u,int v,int cap1, int cap2,int loop){
	if(loop==0){
		if(u==S){
		}else if(v==T){
		}else{//regular edge
			m+=2;
			++(size_t&)nodes[u].arc_first;//for now will count how many outcoming arcs
			++(size_t&)nodes[v].arc_first;
		};
	}else{//loop==1
		if(u==S){
			add_tweights(v,cap1,0);
		}else if(v==T){
			add_tweights(u,0,cap1);
		}else{//regular edge
			//fill in arcs
			arc *& uv = nodes[u].arc_end;
			arc *& vu = nodes[v].arc_end;
			assert(uv>=arcs.begin() && uv<arcs.end());
			assert(vu>=arcs.begin() && vu<arcs.end());
			int e = m/2;
			if(e>=nE-nE_out){//this is outcoming boundary arc
				cap2=0;
				if(E_out){
					E_out[e-(nE-nE_out)] = uv-arcs.begin();//save the new position of this arc
				};
			};
			init_arc(&nodes[u],&nodes[v],uv,vu,cap1,cap2);
			++uv;
			++vu;
			m+=2;
		};		
	};
};

void seed_hi_pr::add_tweights(int u,int cap1,int cap2){
	int delta = nodes[u].excess;
	if (delta > 0) cap1 += delta;
	else           cap2 -= delta;
	flow += (cap1 < cap2) ? cap1 : cap2;
	nodes[u].excess = cap1-cap2;
};

tflow seed_hi_pr::cut_cost(int * C){
	return 0;
};

/*
__forceinline bool seed_hi_pr::is_weak_source(seed_hi_pr::node * v){
	return v->d==d_inf;
};
*/

tflow seed_hi_pr::cut_cost(){
	tflow cost = flow;
	for(arc * a=arcs.begin();a!=arcs.end();++a){
		node * u = a->rev->head;
		node * v = a->head;
		if(is_weak_source(u) && !is_weak_source(v)){
			cost += a->resCap;
		};
	};
	return cost;
};

long long seed_hi_pr::size_required(int nV, int nB, int nE, int nE_out,int d_inf){
	long long r = 0;
	r+=sizeof(seed_hi_pr);
	r+=sizeof(seed_hi_pr::node)*(nV+nB);
	r+=sizeof(seed_hi_pr::arc)*2*nE;
	r+=seed_buckets<node>::size_required(nV+nB+2,d_inf);// buckets
	r+=sizeof(node*)*(nB);// BS
	return r;
};
#include "seed_BK1.h"

/*
	special constants for node->parent
*/
#define TERMINAL ( (arc *) 1 )		/* to terminal */
#define ORPHAN   ( (arc *) 2 )		/* orphan */

#define INFINITE_D ((int)(((unsigned)-1)/2))		/* infinite distance to the terminal */

/***********************************************************************/
/*
inline void seed_BK::set_active(seed_BK::node *i)
{
	if (!i->next){// it's not in the list yet
		if (queue_last[1]){
			queue_last[1] -> next = i;
		}else{
			queue_first[1] = i;
		};
		queue_last[1] = i;
		i -> next = i;
	}
}
*/

/***********************************************************************/
/*
inline seed_BK::node* seed_BK::next_active()
{
	node *i;

	while ( 1 )
	{
		if (!(i=queue_first[0]))
		{
			queue_first[0] = i = queue_first[1];
			queue_last[0]  = queue_last[1];
			queue_first[1] = NULL;
			queue_last[1]  = NULL;
			if (!i) return NULL;
		}

		// remove it from the active list 
		if (i->next == i) queue_first[0] = queue_last[0] = NULL;
		else              queue_first[0] = i -> next;
		i -> next = NULL;

		// a node in the list is active iff it has a parent 
		if (i->parent) return i;
	}
}
*/
//_________________________node_queue________________________________
__forceinline void seed_BK::node_queue::push(seed_BK::node * q){
	if(!q->next){//q is not in the open set
		if(last_open){
			last_open->next = q;
		}else{
			assert(first_open==0);
			first_open = q;
		};
		last_open = q;
		q->next = q;
	};
};

__forceinline seed_BK::node * seed_BK::node_queue::top(){
	assert(!first_open || first_open->next);
	return first_open;
};

__forceinline seed_BK::node * seed_BK::node_queue::pop(){
	node * q = first_open;
	if(q){
		assert(q->next);
		if(q->next == q){//q was the last open node
			first_open = last_open = 0;
		}else{// there are some more open nodes
			first_open = q->next;
		};
		q->next = 0;
	};
	return q;
};
//___________________________orphan_queue___________________________

__forceinline void seed_BK::orphan_queue::push(seed_BK::node * q){
	assert(!q->is_orphan());
	if(!first_orphan){
		q->next_orphan = q;
		//queue_orphan = q;
	}else{
		q->next_orphan = first_orphan;
	};
	first_orphan = q;
	queue_orphan = q;
	//q->is_orphan = 1;
	//q->parent = 0;
	q -> parent = ORPHAN;
};

__forceinline void seed_BK::orphan_queue::insert(seed_BK::node *q){
	if(!first_orphan)return push(q);
	assert(!q->is_orphan());
	node * a = queue_orphan;
	node * b = queue_orphan->next_orphan;
	a->next_orphan = q;
	if(a==b){//queue_orphan is the last one
		q->next_orphan = q;
	}else{
		q->next_orphan = b;
	};
	queue_orphan = q;
	//q->parent = 0;
	q->parent = ORPHAN;
};

__forceinline seed_BK::node * seed_BK::orphan_queue::pop(){
	node * q = first_orphan;
	if(q){
		assert(q->is_orphan());
		if(q==q->next_orphan){//last one
			first_orphan = queue_orphan = 0;
		}else{
			first_orphan = q->next_orphan;
			if(q==queue_orphan){//queue_orphan was in the middle move to next
				queue_orphan = first_orphan;
			};
		};
	};
	return q;
};
/*
//__________________________________________________________________
	inline void seed_BK::set_orphan_front(node *i)
{
	nodeptr *np;
//	save(i);
	i -> parent = ORPHAN;
	np = nodeptr_block -> New();
	np -> ptr = i;
	np -> next = orphan_first;
	orphan_first = np;
}


	inline void seed_BK::set_orphan_rear(node *i)
{
	nodeptr *np;
//	save(i);
	i -> parent = ORPHAN;
	np = nodeptr_block -> New();
	np -> ptr = i;
	if (orphan_last) orphan_last -> next = np;
	else             orphan_first        = np;
	orphan_last = np;
	np -> next = NULL;
}
*/

bool seed_BK::is_free(node * q)const{
	return q->d == d_free;
};

bool seed_BK::is_bnd(node * q)const{
	return q-nodes.begin()>=nV;
};

bool seed_BK::is_source(node * q)const{
	return q->d == d_inf;
};

bool seed_BK::is_sink(node * q)const{
	return q->d < d_free;
};

bool seed_BK::is_sink(int v)const{
	return nodes[v].d < d_free;
};

bool seed_BK::is_weak_source(int v)const{
	return nodes[v].d >= d_free;
};

template<bool source> bool seed_BK::opposite_tree(node * q)const{
	if(source){
		return is_sink(q);
	}else{
		return is_source(q);
	};
};

bool seed_BK::same_tree(node * p, node * q)const{
	return q->d == p->d;
};

void seed_BK::set_free(node *q){
	q->d = d_free;
	q->parent = 0;
};
//__________________________________________________________
void seed_BK::check(){
/*
#ifndef NDEBUG
	for(node * p=nodes.begin();p<nodes.end();++p){
		assert(p->TS<=TIME);
		if(p->excess!=0)assert(p->parent == TERMINAL);
		if(p->parent==0)assert(is_free(p));
		if(p->parent && p->parent!=TERMINAL && p->parent!= ORPHAN){
			node * q = p->parent->head;
			//p is a child of q
			assert(p->TS <= q->TS);
			if(p->TS == q->TS){
				assert(p->DIST>q->DIST);
			};
			// must have the same tree label
			assert(p->d == q->d);
		};
		if(is_sink(p)){
			// check that only bucket labels are assigned to vertices
			int b = open_buckets.find_above(p->d);
			assert(open_buckets[b].d==p->d);
		};
	};
#endif
*/	
};

//__________________________________________________________

void seed_BK::augment(seed_BK::arc *middle_arc){
	node *i;
	arc *a;
	tcap bottleneck;

	/* 1. Finding bottleneck capacity */
	/* 1a - the source tree */
	bottleneck = middle_arc -> r_cap;
	for (i=middle_arc->sister->head; ; i=a->head){
		a = i -> parent;
		if (a == TERMINAL) break;
		if (bottleneck > a->sister->r_cap) bottleneck = a -> sister -> r_cap;
	}
	if (bottleneck > i->excess)bottleneck = i -> excess;
	/* 1b - the sink tree */
	for (i=middle_arc->head; ; i=a->head){
		a = i -> parent;
		if (a == TERMINAL) break;
		if (bottleneck > a->r_cap) bottleneck = a -> r_cap;
	}
	if (i->d==-1 && bottleneck > - i->excess) bottleneck = - i -> excess;

	assert(bottleneck>0);
	/* 2. Augmenting */
	/* 2a - the source tree */
	middle_arc -> sister -> r_cap += bottleneck;
	middle_arc -> r_cap -= bottleneck;
	for (i=middle_arc->sister->head; ; i=a->head){
		a = i -> parent;
		if (a == TERMINAL) break;
		a -> r_cap += bottleneck;
		a -> sister -> r_cap -= bottleneck;
		if (!a->sister->r_cap)
		{
			//set_orphan_front(i); // add i to the beginning of the adoption list
			orphans[1].push(i);
		}
	}
	i -> excess -= bottleneck;
	if (!i->excess){
		//set_orphan_front(i); // add i to the beginning of the adoption list
		orphans[1].push(i);
	}
	/* 2b - the sink tree */
	for (i=middle_arc->head; ; i=a->head){
		a = i -> parent;
		if (a == TERMINAL) break;
		a -> sister -> r_cap += bottleneck;
		a -> r_cap -= bottleneck;
		if (!a->r_cap){
			//set_orphan_front(i); // add i to the beginning of the adoption list
			orphans[0].push(i);
		}
	}
	if(i->d==-1){//augmentations to the boundary does not change the flow
		flow += bottleneck;
	};
	i -> excess += bottleneck;
	if (!i->excess){
		//set_orphan_front(i); // add i to the beginning of the adoption list
		orphans[0].push(i);
	}
}

/***********************************************************************/
void seed_BK::process_source_orphan(seed_BK::node *i){
	node *j;
	arc *a0, *a0_min = NULL, *a;
	int d, d_min = INFINITE_D;

	/* trying to find a new parent */
	for (a0=i->arc_first; a0!=i->arc_end; ++a0)
	if (a0->sister->r_cap){
		j = a0 -> head;
		if (is_source(j)){//is in the source tree
			/* checking the origin of j */
			d = 0;
			while ( 1 )
			{
				if (j->TS == TIME)
				{
					d += j -> DIST;
					break;
				}
				a = j -> parent;
				d ++;
				if (a==TERMINAL)
				{
//					save(j);
					j -> TS = TIME;
					j -> DIST = 1;
					break;
				}
				if (a==ORPHAN) { d = INFINITE_D; break; }
				j = a -> head;
			}
			if (d<INFINITE_D) /* j originates from the source - done */
			{
				if (d<d_min)
				{
					a0_min = a0;
					d_min = d;
				}
				/* set marks along the path */
				for (j=a0->head; j->TS!=TIME; j=j->parent->head)
				{
//					save(j);
					j -> TS = TIME;
					j -> DIST = d --;
				}
			}
		}
	}

//	save(i);
	i->clear_orphan();
	if (i->parent = a0_min)
	{
		i -> TS = TIME;
		i -> DIST = d_min + 1;
		i->d = a0_min->head->d;
	}
	else{
		/* no parent is found */
		i->d = d_free;
		/* process neighbors */
		for (a0=i->arc_first; a0!=i->arc_end; ++a0){
			j = a0 -> head;
			if (is_source(j)){//is in the source tree
				a = j->parent;
				if (a0->sister->r_cap){
					//set_active(j);
					open->push(j); // makes a source node active
				};
				if (is_child(j,i)){
					//set_orphan_rear(j); // add j to the end of the adoption list
					assert(!j->is_orphan());
					orphans[1].insert(j);
				}
			}
		}
	}
}

void seed_BK::process_sink_orphan(seed_BK::node *i){
	node *j;
	arc *a0, *a0_min = NULL, *a;
	int d, d_min = INFINITE_D;

	/* trying to find a new parent */
	for (a0=i->arc_first; a0!=i->arc_end; ++a0)
	if (a0->r_cap)
	{
		j = a0 -> head;
		//if (j->is_sink && (a=j->parent)){// is in the sink tree
		if (j->d==i->d){// is in the sink tree, same level
			/* checking the origin of j */
			d = 0;
			while ( 1 ){
				if (j->TS == TIME){
					d += j -> DIST;
					break;
				}
				a = j -> parent;
				d ++;
				if (a==TERMINAL){
//					save(j);
					j -> TS = TIME;
					j -> DIST = 1;
					break;
				}
				if (a==ORPHAN) { d = INFINITE_D; break; }
				j = a -> head;
			}
			if (d<INFINITE_D) /* j originates from the sink - done */
			{
				if (d<d_min){
					a0_min = a0;
					d_min = d;
				}
				/* set marks along the path */
				for (j=a0->head; j->TS!=TIME; j=j->parent->head){
//					save(j);
					j -> TS = TIME;
					j -> DIST = d --;
				}
			}
		}
	}

//	save(i);
	i->clear_orphan();
	if (i->parent = a0_min)
	{
		i -> TS = TIME;
		i -> DIST = d_min + 1;
		i->d = a0_min->head->d;
	}
	else{
		/* no parent is found */
		//make free
		i->d = d_free;
		/* process neighbors */
		for (a0=i->arc_first; a0!=i->arc_end; ++a0){
			j = a0 -> head;
			//if (j->is_sink && (a=j->parent)){
			if (is_sink(j)){
				a = j->parent;
				if (a0->r_cap){
					//set_active(j);
					//
					if(j->d == open->d){
						open->push(j);
					}else{
						bool r = open_buckets.push(j);
						assert(r==true);
					};
				};
				if (a!=TERMINAL && a!=ORPHAN && a->head==i){// j is a child of i
					//set_orphan_rear(j); // add j to the end of the adoption list
					assert(!j->is_orphan());
					orphans[0].insert(j);
				}
			}
		}
	}
}

/***********************************************************************/
	void seed_BK::process_level(){
	node *i, *j, *current_node = NULL;
	arc *a;
	//nodeptr *np, *np_next;
	while ( 1 )
	{
		 //test_consistency(current_node);
		check();

		if((i=current_node))
		{
			i -> next = NULL; /* remove active flag */
			if (!i->parent){
				assert(is_free(i));
				i = NULL;// the node became free
			};
		};
		if(!i){// i==0
			//if (!(i = next_active())) break;
			do{
				i=open->pop();
				if(!i)return;
			}while(is_free(i));
		}

		/* growth */
		if(is_source(i)){
			/* grow source tree */
			for (a=i->arc_first; a!=i->arc_end; ++a){
				if (a->r_cap){
					j = a -> head;
					if (j->d >= d_free){// j is a free or source
						if (j->d == d_free){// j is a free node
							//j becomes a new source node
							j -> d = d_inf;
							j -> parent = a -> sister;
							j -> TS = i -> TS;
							j -> DIST = i -> DIST + 1;
							j->d = i->d;
							open->push(j);
						}else{// j is source
							if(j->TS <= i->TS && j->DIST > i->DIST){
								// heuristic - trying to make the distance from j to the source shorter
								j -> parent = a -> sister;
								j -> TS = i -> TS;
								j -> DIST = i -> DIST + 1;
							};
						};
					}else{// j is sink or other level sink
						if(j->d == current_d){// j is sink node with current label
							goto augment;//found augmenting path
						}else{// j is other level sink node, must be processed later
							bool r= open_buckets.push(j);
							assert(r==true);
						};
					};
				};
			};
		}else{
			/* grow sink tree */
			for (a=i->arc_first; a!=i->arc_end; ++a)
			if (a->sister->r_cap){
				j = a -> head;
				if (j->d==d_free){// j is free
					// j becomes a new sink node
					j ->d = i->d;
					j -> parent = a -> sister;
					j -> TS = i -> TS;
					j -> DIST = i -> DIST + 1;
					j->d = i->d; // it recieves the current label
					open->push(j);
				}else if(is_source(j)){// j is source
					a = a -> sister; 
					goto augment;// found augmenting path
				}else if(j->d == i->d && j->TS <= i->TS && j->DIST > i->DIST){// j is sink of the same level
					/* heuristic - trying to make the distance from j to the sink shorter */
					j -> parent = a -> sister;
					j -> TS = i -> TS;
					j -> DIST = i -> DIST + 1;
				}
			}
		}
		current_node = NULL;
		check();
		continue;
augment:;
		TIME ++;
		// augmenting path with a linking source tree and sink tree
		assert(a!=0);
		i -> next = i; /* set active flag */
		current_node = i;

		/* augmentation */
		augment(a);
		/* augmentation end */

		/* adoption */
		while((i=orphans[0].pop())){
			process_sink_orphan(i);
		};
		while((i=orphans[1].pop())){
			process_source_orphan(i);
		};
		/* adoption end */
		check();
	};
	// test_consistency();
};

struct node_less{
	bool operator()(
		const seed_BK::node * a,	const seed_BK::node * b)const{
			return a->d<b->d;
	};
};

void seed_BK::maxflow_init(){
	node *i;
	orphans[0].clear();
	orphans[1].clear();
	TIME = 0;
	max_level = -1;
	open_buckets.current_bucket = 0;
	open_buckets.resize(2);
	open_buckets[0].d = -1;
	open_buckets[1].d = 0;
	open = open_buckets.begin();

	for (i=nodes.begin(); i<nodes.end(); ++i){
		i -> next = NULL;
		i->next_orphan = 0;
		i -> TS = TIME;
		if (i->excess > 0){/* i is connected to the source */
			i ->d = d_inf;
			i -> parent = TERMINAL;
			open->push(i);
			i -> DIST = 1;
		}else if (i->excess < 0){/* i is connected to the sink */
			i ->d = -1;
			i -> parent = TERMINAL;
			open->push(i);
			i -> DIST = 1;
		}else{// free 
			if(i-nodes.begin()<nV){ // free inner node
				i->d = d_free;
				i -> parent = NULL;
			}else{ // boundary node
				//i->d = 0;
				i -> parent = TERMINAL;
				i -> DIST = 1;
				open_buckets[1].push(i);
			};
		};
	};
};

//_______________________Maxflow,_Discharge__________________________________
long long seed_BK::maxflow(bool reuse){
	info.solve_t.start();
	if(!reuse){
		maxflow_init();
	};
	int & b = open_buckets.current_bucket;
	for(b=0;b<open_buckets.size();++b){
		open = &open_buckets[b];
		current_d = open->d;
		process_level();
		assert(open->is_empty());
	};
	b = 0;
	open = &open_buckets[0];
	current_d = open->d;

	info.flow = flow;
	info.nV = n;
	info.nE = m/2;
	info.mem_shared = dynamic::memserver::get_al_blocks().mem_used();
	info.solve_t.stop();
	return flow;
};
/*
void seed_BK::discharge0(){
	maxflow_init();
	//for(int i=0;i<nB;++i){
		//node * p = nodes.begin()+nV+i;
		//p->parent = 0;
		//for(arc * a = p->arc_first;a!=p->arc_end;++a){
		//	a->r_cap = 1;
		//};
	//};
	open_buckets.current_bucket = 0;
	open = &open_buckets[0];
	current_d = open->d;
	process_level();
	assert(open->is_empty());
	//for(int v=0; v<nV;++v){
	//	node * p = &nodes[v];
	//	if(!is_sink(p)){
	//		p->d = 0;
	//	};
	//};
	
	for(int i=0;i<nB;++i){
		node * p = nodes.begin()+nV+i;
		p->d = -1;
		p->next = 0;
		//d_old[i] = -1;
		//bool r = open_buckets.push(p);
		//assert(r);
		//p->parent = TERMINAL;
		//for(arc * a = p->arc_first;a!=p->arc_end;++a){
		//	node * q = a->head; // inner boundary node
		//	if(!is_sink(p->d)){
		//		p->d = 1;
		//	};
		//};
	};
};
*/
void seed_BK::discharge(int max_level){
	// adopt orphans
	check();
	//process
	if(!reuse){
		maxflow_init();
		reuse = true;
	};//else{
	update_seeds();
	//};
	has_active = false;
	node * i;
	while((i=orphans[0].pop())){
		process_sink_orphan(i);
	};
	while((i=orphans[1].pop())){
		process_source_orphan(i);
	};
	this->max_level = max_level;
	int & b = open_buckets.current_bucket;
	for(b=0; b<open_buckets.size() && open_buckets[b].d<max_level; ++b){
		open = &open_buckets[b];
		current_d = open->d;
		process_level();
		assert(open->is_empty());
	};
	b = 0;
	open = &open_buckets[0];
	current_d = open->d;
	check();
};

void seed_BK::relabel(){
};


void seed_BK::msg_in(int j, int d, tcap f){
		arc * a = &arcs[j]; // outcoming arc u->v
		node * u = a->sister->head;
		node * v = a->head;
		assert(d!=d_inf-1);
//		assert(v->parent == TERMINAL);
		assert(v-nodes.begin()>=nV);//boundary node
		assert(v->d<=d || d > max_level);//cannot go down
		if(d>v->d){//the label is raisen
			d_old[v-nodes.begin()-nV] = v->d;
			v->d = d;
		};
		assert(a->sister->r_cap==0);
		a->sister->r_cap = f; //remember incoming flow here
	};

	void seed_BK::msg_out(int j, int * d, tcap * f){
		arc * a = &arcs[j]; // outcoming arc u->v
		node * u = a->sister->head;
		node * v = a->head;
		assert(u-nodes.begin()<nV);// this must be innter vertex
		assert(v->parent == TERMINAL);
		int d_old = *d;
		*d = std::min(u->d+1,d_inf);// free and source vertices get d_inf
		if(open_buckets.back().d >= max_level){//some unprocessed levels remained
			*d = std::min(*d,max_level+1);//inf labels in the lazy variant become max_level+1
			has_active = true;
		};
		*d = std::max(*d,d_old);//never go below the previous label
		tcap df = a->sister->r_cap;
		// transmit the flow to the neighboring network and 
		*f += df;
		assert(df>=0);
		v->excess -= df;
		assert(v->excess>=0);
		assert(*d <= d_inf);
		a->sister->r_cap = 0;
		//return d_old<*d;// if the label waas raisen
	};

__forceinline bool seed_BK::is_child(node * q,node * p){// test if q is child of p
	return (q->parent!=0 && q->parent!=ORPHAN && q->parent!=TERMINAL && q->parent->head == p);
};

__forceinline void seed_BK::disconnect_sink(node * p){
	//make all p's children orphans
	for(arc * a=p->arc_first; a!=p->arc_end;++a){
		node * q = a->head;
		if(is_child(q,p)){//q is a child of p
			orphans[0].push(q);
		};
		if(is_sink(q) && !is_bnd(p) && a->r_cap>0){
			assert(q->d >= p->d);
			bool r = open_buckets.push(q);
			assert(r);
		};

		if(is_source(q) && a->sister->r_cap>0){
			open->push(q);
		};
	};
};

__forceinline void seed_BK::clear_sink_tree(node * p){
	p->clear_orphan();
	for(arc * a = p->arc_first; a!=p->arc_end; ++a){
		node * q = a->head;
		if(is_child(q,p)){
			orphans[0].push(q);
		};
		assert(a->sister->r_cap==0 || !is_source(q) || q->is_open() || (is_bnd(q) && q->d==d_inf));
		if(a->r_cap>0){
			node * q = a->head;
			assert(q->d >= p->d);
			//if(q->d > p->d  && is_sink(q)){
			if(is_sink(q)){
				bool r = open_buckets.push(q);
				//r may be fasle
			};
		};
	};
	set_free(p);
};

/*
void seed_BK::clear_boundary(){
	assert(orphans[0].is_empty());
	for(int i=0;i<nB;++i){
		node * p = &nodes[nV+i];
		for(arc * a = p->arc_first; a!=p->arc_end; ++a){
			node * q = a->head;
			if(is_child(q,p)){
				orphans[0].push(q);
			};
		};
	};
	while(node * p=orphans[0].pop()){
		clear_sink_tree(p);
	};
};
*/

void seed_BK::update_seeds(){
	//clear_boundary();
	//reorder the new boundary labels, and create the new label buckets
	std::sort(BS.begin(),BS.end(),node_less());
	open_buckets.resize(1);
	open = &open_buckets[0];
	for(node ** pv = BS.begin();pv!=BS.end();++pv){//go over boundary roots in the order of their labels
		int vi = ((*pv)-nodes.begin())-nV;
		assert(vi>=0);
		assert((*pv)->d != d_inf-1);
		if((*pv)->d >= d_inf-1)break; // do not take into account boundary vertices at d_inf-1 or above
		if((*pv)->d > open->d){//this seed is from the next level
			open_buckets.resize(++open_buckets.current_bucket+1);
			++open;
			open->d = (*pv)->d;
			open->clear();
		};
		//assert((*pv)->next == 0);
		assert((*pv)->next_orphan == 0);
		(*pv)->DIST = 1;
		(*pv)->TS = TIME;
		(*pv)->next = 0;
		//if((*pv)->parent == ORPHAN){// marked as modified
//		if((*pv)->d > d_old[vi]){// was raisen
//			open->push(*pv);
//		};
		assert((*pv)->parent == TERMINAL);
	};
	//
	open = &open_buckets[0];
	open_buckets.current_bucket = 0;
	// clear dead trees
	assert(orphans[0].is_empty());
	for(int i=0;i<nB;++i){
		node * p = &nodes[nV+i];
		assert(p->d!=d_inf-1);
		if(p->d > d_old[i]){// was raisen, clear its tree
			//check if there are still seeds for this label
			//if(!open_buckets.find_bucket(d_old[i])){
			int b = open_buckets.find_below(d_old[i]);
			if(open_buckets[b].d!=d_old[i]){//label is dead, clear completely
				/*
				for(arc * a = p->arc_first; a!=p->arc_end; ++a){
				node * q = a->head;
				if(is_child(q,p)){
				orphans[0].push(q);
				};
				};
				*/
				disconnect_sink(p);
			};
		};
	};
	while(node * p=orphans[0].pop()){
		clear_sink_tree(p);
	};
	
	//
	++TIME;
	// adopt other trees, which were raisen
	for(int i=0;i<nB;++i){
		node * p = &nodes[nV+i];
		assert(p->d!=d_inf-1);
		if(p->d > d_old[i]){// was raisen
			for(arc * a=p->arc_first; a!=p->arc_end;++a){
				node * q = a->head;
				if(is_child(q,p)){//q is a child of p
					orphans[0].push(q);
				};
			};
		};
		if(p->d < d_inf && (p->d > d_old[i] || p->d >= max_level)){// was raisen or has not been processed before
			bool r = open_buckets.push(p);
			assert(r);
		};
	};

	//adopt broken boundary trees
	while(node * i=orphans[0].pop()){
		process_sink_orphan(i);
	};
	
	check();

	//now buckets are correct, inject the flow
	//inject the flow
	for(int i=0;i<nB;++i){
		node * v = &nodes[nV+i];
		for(arc * b=v->arc_first; b!=v->arc_end;++b){
			node * u = b->head;
			arc * a = b->sister;
			tcap f = b->r_cap;
			b->r_cap = 0;
			assert(f>=0);
			if(f>0){
				if(a->r_cap==0){//this edge was zero and becomes non-zero, make v open
					// v is a boundary node, processed elsewhere
					//assert(v->is_open());
					//open_buckets.push(v);
				};
				a->r_cap+=f;
				if(is_sink(u)){// u is a sink node
					++TIME;
					if(u->excess<0){// u is a TERMINLA sink node
						flow+= std::min(-u->excess,f); // quick augment
					};
					u->excess+=f;
					if(u->excess<0){// remains a TERMINAL sink node
						continue;
					};
					if(u->excess==0){// becomes a sink orphan node
						orphans[0].push(u);
					}else{// u->excess>0 -- becomes a source node
						//assert(u->d==-1);
						disconnect_sink(u);
						u->parent = TERMINAL;
						u->DIST = 1;
						u->TS = TIME;
						u->d = d_inf;
						open->push(u);
						//open_buckets.push(u,v->d);
						//bool r  = open_buckets.push(u,v->d);// this sink becomes active later, starting from label v->d
						//assert(r);
					};
					while(node * i=orphans[0].pop()){
						process_sink_orphan(i);
					};
				}else if(is_source(u)){// u is a source node, add excess, make it TERMINAL
					u->excess += f;
					u->parent = TERMINAL;
					u->DIST = 1;
					u->TS = TIME;
					//open status is unchanged
				}else{// u is a free node, becomes a terminal source node
					assert(is_free(u));
					u->excess += f;
					u->parent = TERMINAL;
					u->DIST = 1;
					u->TS = TIME;
					u->d = d_inf;
					open->push(u);
				};
				assert(orphans[0].is_empty());
			};
			check();
		};
	};
};

/*
__forceinline void seed_BK::raise_seed(node * v , int d){
	assert(v->d<=d);//cannot go down
	if(d>v->d){//the label is raisen
		d_old[v-nodes.begin()-nV] = v->d;
		v->d = d;
	};
};
*/

/*
void seed_BK::msg_in(int j, int d, tcap f){
	arc * a = &arcs[j]; // outcoming arc u->v
	node * u = a->sister->head;
	node * v = a->head;
	assert(d!=d_inf-1);
	assert(v-nodes.begin()>=nV);//boundary node
	raise_seed(v,d);
	assert(a->sister->r_cap==0);
	a->sister->r_cap = f; //remember incoming flow here
};

void seed_BK::msg_out(int j, int * d, tcap * f){
	arc * a = &arcs[j]; // outcoming arc u->v
	node * u = a->sister->head;
	node * v = a->head;
	assert(u-nodes.begin()<nV);// this must be innter vertex
	*d = std::min(u->d+1,d_inf);// free and source vertices get d_inf
	tcap df = a->sister->r_cap;
	// transmit the flow to the neighboring network and 
	*f += df;
	assert(df>=0);
	a->head->excess -= df;
	assert(*d <= d_inf);
	a->sister->r_cap = 0;
};
*/

__forceinline int seed_BK::get_d(int v){
	if(n>nV){
		return nodes[v].d;
	}else{
		return std::min(nodes[v].d+1,d_inf);
	};
};

/***********************************************************************/
/*
void seed_BK::test_consistency(node* current_node){
	node *i;
	arc *a;
	int r;
	int num1 = 0, num2 = 0;

	// test whether all nodes i with i->next!=NULL are indeed in the queue
	for (i=nodes.begin(); i<nodes.end(); i++)
	{
		if (i->next || i==current_node) num1 ++;
	}
	for (r=0; r<3; r++)
	{
		i = (r == 2) ? current_node : queue_first[r];
		if (i)
		for ( ; ; i=i->next)
		{
			num2 ++;
			if (i->next == i)
			{
				if (r<2){ assert(i == queue_last[r]);
				}else     assert(i == current_node);
				break;
			}
		}
	}
	assert(num1 == num2);

	for (i=nodes.begin(); i<nodes.end(); i++)
	{
		// test whether all edges in seach trees are non-saturated
		if (i->parent == NULL) {}
		else if (i->parent == ORPHAN) {}
		else if (i->parent == TERMINAL)
		{
			if (is_source(i)){assert(i->excess > 0);
			}else             assert(i->excess < 0);
		}
		else
		{
			if (is_source(i)){ assert (i->parent->sister->r_cap > 0);
			}else             assert (i->parent->r_cap > 0);
		}
		// test whether passive nodes in search trees have neighbors in
		// a different tree through non-saturated edges
		if (i->parent && !i->next)
		{
			if (is_source(i))
			{
				assert(i->excess >= 0);
				for (a=i->arc_first; a!=i->arc_end; ++a)
				{
					if (a->r_cap > 0) assert(a->head->parent && is_source(a->head));
				}
			}
			else
			{
				assert(i->excess <= 0);
				for (a=i->arc_first; a!=i->arc_end; ++a)
				{
					if (a->sister->r_cap > 0) assert(a->head->parent && is_sink(a->head));
				}
			}
		}
		// test marking invariants
		if (i->parent && i->parent!=ORPHAN && i->parent!=TERMINAL)
		{
			assert(i->TS <= i->parent->head->TS);
			if (i->TS == i->parent->head->TS) assert(i->DIST > i->parent->head->DIST);
		}
	}
}
*/

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

seed_BK::seed_BK(){
	maxflow_iteration = 0;
	flow = 0;
	info.name = "seed_BK";
	//E_out = 0;
};

seed_BK::~seed_BK(){
	//if(E_out){
	//	delete E_out;
	//};
};

size_t seed_BK::load(const std::string & id){
	size_t read_bytes=0;
	FILE * f = fopen(id.c_str(),"rb");
	setvbuf(f,0,_IOFBF,1024*1024*2);
	//read sizes
	read_bytes += fread(&nV,1,sizeof(nV),f);
	read_bytes += fread(&nB,1,sizeof(nB),f);
	read_bytes += fread(&nE,1,sizeof(nE),f);
	read_bytes += fread(&nE_out,1,sizeof(nE_out),f);
	int nexcess;
	read_bytes += fread(&nexcess,1,sizeof(nexcess),f);
	//allocate graph and aux data
	E_out.resize(nE_out);
	//E_out = new int[nE_out];
	allocate1(nV,nB,nE,nE_out,E_out.begin(),d_inf);
	//read node global index
	global_index.resize(nV);
	for(int v=0;v<nV;++v){
		read_bytes += fread(&global_index[v],1,sizeof(int),f);
	};
	//read excess
	for(int i=0;i<nexcess;++i){//these shoud agregate correctly
		int v;
		int e;
		read_bytes += fread(&v,1,sizeof(v),f);
		read_bytes += fread(&e,1,sizeof(e),f);
		add_tweights(v,std::max(0,e),std::max(0,-e));
	};
	long pos = ftell(f);
	//read residual capacities
	for(int loop=0;loop<2;++loop){
		fseek(f,pos,SEEK_SET);
		reset_counters();
		for(int e=0;e<nE;++e){
			int u;
			int v;
			int cap1;
			int cap2;
			read_bytes += fread(&u,1,sizeof(u),f);
			read_bytes += fread(&v,1,sizeof(v),f);
			read_bytes += fread(&cap1,1,sizeof(cap1),f);
			read_bytes += fread(&cap2,1,sizeof(cap2),f);
			read_arc(loop,u, v, cap1, cap2);
		};
		allocate2(loop);
	};
	fclose(f);
	reuse = false;
	return read_bytes;
};

size_t seed_BK::save(const std::string & id){
	return 0;
};

size_t seed_BK::unload(const std::string & id){//save and free memory
	size_t bytes = save(id);
	nodes.destroy();
	arcs.destroy();
	BS.destroy();
	open_buckets.destroy();
	return bytes;
};


__forceinline void seed_BK::init_arc(seed_BK::node *u, seed_BK::node *v, seed_BK::arc* uv, seed_BK::arc * vu, tcap cap1, tcap cap2){
	uv->head = v;
	vu->head = u;
	uv->sister = vu;
	vu->sister = uv;
	uv->cap() = cap1;
	vu->cap() = cap2;
};

long long seed_BK::size_required(int nV, int nB, int nE, int nE_out,int d_inf){
	long long r = 0;
	r+=sizeof(seed_BK);
	r+=sizeof(node)*(nV+nB);
	r+=sizeof(arc)*2*nE;
	r+=sizeof(node_queue)*(nB+1);// open_buckets
	r+=sizeof(node*)*(nB);// BS
	r+=sizeof(arc*)*(nE_out);// E_out
	r+=sizeof(int)*(nV);// global_index
	//r+=1024;
	return r;
};

void seed_BK::allocate1(int nV, int nB, int nE, int nE_out, int *E_out, int d_inf, int S, int T){
	//this->E_out = E_out;
	this->nE_out = nE_out;
	this->S = S;
	this->T = T;
	if(nB==0 && d_inf==0)d_inf = 1;
	this->d_inf = d_inf;
	d_free = d_inf-1;
	this->nV = nV;
	this->nB = nB;
	n = nV+nB;
	m = 0;//2*nE;
	flow = 0;
	dead = 0;
	nodes.resize(n);
	//
	//trees.reserve(nB);
	BS.reserve(nB);
	d_old.resize(nB);
	open_buckets.reserve(nB+2);
	//open.reserve(n);
	//init nodes
	for(int v=0;v<n;++v){
		nodes[v].arc_first = 0;
		nodes[v].excess = 0;
		nodes[v].d = 0;
		if(v>=nV){
			BS.push_back(&nodes[v]);
		};
		//global_index[v] = v;
	};
	if(T>=0 && S>=0){
		nodes[T].excess = -1;
		nodes[S].excess = 1;
	};
};

int seed_BK::g_index(int v){
	if(global_index.size()>0){
		return global_index[v];
	}else{
		return v;
	};
};

void seed_BK::reset_counters(){
	m = 0;
};

void seed_BK::allocate2(int loop){
	if(loop==0){
		arcs.resize(m);
		size_t accum_size = 0;
		//compute desired starting positions for where to allocate arcs
		for(node * v = nodes.begin();v!=nodes.end();++v){
			size_t s = (size_t&)v->arc_first;
			v->arc_first = arcs.begin()+accum_size;
			v->arc_end = v->arc_first;
			//v->arc_current = v->arc_first;
			accum_size+=s;
		};
		assert(accum_size<=m);
		nE = m/2;
		reset_counters();
	};
};

void seed_BK::read_arc(int loop,int u,int v,int cap1, int cap2){
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
				if(E_out.size()>0){
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

__forceinline void seed_BK::add_tweights(int u,int cap1,int cap2){
	int delta = nodes[u].excess;
	if (delta > 0) cap1 += delta;
	else           cap2 -= delta;
	flow += (cap1 < cap2) ? cap1 : cap2;
	nodes[u].excess = cap1-cap2;
};

void seed_BK::allocate1(int n ,int m, int S, int T,int d,int * sz){
	allocate1(n,0,m,0,0,0,S,T);
	reset_counters();
};

void seed_BK::construct(const char * filename){
	info.construct_t.resume();
	dimacs_parser(filename,*this,2);
	info.construct_t.pause();
};

void seed_BK::save_cut(const std::string & filename){
	FILE * f = fopen(filename.c_str(),"wt+");
	setvbuf(f,NULL,_IOFBF,1024*1024*2);
	fprintf(f,"p max %i %i\n",nV,nE);
	fprintf(f,"c minimum cut, generated by %s\n",info.name.c_str());
	fprintf(f,"f %lli\n",info.flow);
	//fprintf(f,"n 1 1\n");//source has label 1
	//fprintf(f,"n 2 0\n");//sink has label 0
	for(int v=0;v<nV;++v){
		//fprintf(f,"n %i %i\n",g_index(v)+1,1-(nodes[v].d<d_inf));
		fprintf(f,"n %i %i\n",g_index(v)+1,is_weak_source(v));
	};
};

void seed_BK::get_cut(int * C){
	for(int v=0;v<nV;++v){
		C[v] = is_weak_source(v);
	};
};

long long seed_BK::cut_cost(int * C){
	tflow cost = 0;
	for(arc * a=arcs.begin();a!=arcs.end();++a){
		node * u = a->sister->head;
		node * v = a->head;
		if(C[u-nodes.begin()]==1 && C[v-nodes.begin()]==0){
			cost += a->r_cap;
		};
	};
	return cost;
};

long long seed_BK::cut_cost(){
	tflow cost = flow;
	for(arc * a=arcs.begin();a!=arcs.end();++a){
		node * u = a->sister->head;
		node * v = a->head;
		//if(!is_sink(u) && is_sink(v)){
		if(is_weak_source(u-nodes.begin()) && !is_weak_source(v-nodes.begin())){
			cost += a->r_cap;
		};
	};
	return cost;
};
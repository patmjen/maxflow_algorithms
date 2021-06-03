#ifndef seed_buckets_h
#define seed_buckets_h

#include "defs.h"
#include "dynamic/fixed_array1.h"

template<class node> class seed_buckets;

template<class node> class bucket_node{
public:
	typedef typename seed_buckets<node>::bucket tbucket;
public:
	int d;
public:
	node * bnext;
	node * bprev;
	tbucket * my_bucket;
public:
	enum tstate{free=0,active=1,inactive=2,dead=-1} state;	
#ifndef NDEBUG
	bucket_node():bnext(0),bprev(),my_bucket(0),state(free),d(1){};
	//
	void set_state(tstate st){
		state = st;
	};
#else
	bucket_node():bnext(0),bprev(),my_bucket(0),d(1){};
	void set_state(int st){};
#endif
};


//___________________________________________
template<class node> class seed_buckets{
public:
	class bucket;
	//______________________________________

	//_____________________________________
	class bucket{
	public:
		node * first_active;   //!< list of active nodes in the bucket
		node * first_inactive; //!< list of inactive nodes in the bucket
		bucket * next; //!< bucket with higher label
		bucket * prev; //!< bucket with lower label
		//
		bucket * next_seed;//!< pointer to next seed bucket, possibly d_inf bucket
	public:
		int d;
	public:
		bucket():first_active(0),first_inactive(0),next(0),prev(0),next_seed(0){
		};
	public:
		bool inline isempty()const{
			return first_active==0 && first_inactive==0;
		};
	};
//___________________________________________
public:
	typedef bucket tbucket;
private:
	dynamic::fixed_array1<bucket> pool;
public:
	tbucket * first;    //!< sink bucket, only sink is here
	tbucket * last;     //!< d_inf bucket - all nodes which cant reach sink are here, only inactive nodes
public:
	//int top_active_d;	//!< label of the highest active node
	tbucket * top_active;  //!< pointer to the highest active bucket
public:
	//bool inline has_active()const{return top_active_d>0;};
private:
	tbucket * empty;
public:
	seed_buckets():first(0),last(0),empty(0),top_active(0){
	};
	~seed_buckets(){
	};
private:
	inline void link(tbucket * a, tbucket * b){
		a->next = b;
		b->prev = a;
	};
public:
	static long long size_required(int n, int d_inf){
		long long sz = 0;
		sz += sizeof(seed_buckets<node>);
		sz += sizeof(bucket)*n;
		return sz;
	};
	//! must be called firtst
	void init(int n, int d_inf){
		assert(n>=2);
		pool.resize(n);
		first = &pool[0];
		first->d = 0;
		last = &pool[1];
		last->d = d_inf;
		top_active = first;
		link(&pool[0],&pool[1]);
		first->next_seed = last;
		if(n>2){
			empty = &pool[2];
			for(int i=2;i<n-1;++i){
				pool[i].next = &pool[i+1];
			};
		};
	};

public:
	inline tbucket * find(int d, tbucket * b){
		assert(b!=0);
		while(b->next && b->next->d<=d){
			b=b->next;
		};
		return b;
	};

private:
	//! returns bucket with largest b->d such that d->b <= d
	__forceinline tbucket * find_le(int d, tbucket * b){
		assert(b!=0);
		if(b->d<d && b->next!=0){//check the next bucket
			b=b->next;
		};
		return b;
	};
public:
	__forceinline tbucket * add_bucket_after(tbucket * a, int d){//adds new bucket after bucket a
		//take new bucket from the empty pool
		assert(empty!=0);
		tbucket * b = empty;
		b->d = d;
		empty = empty->next;
		assert(a->next!=0);
		link(b,a->next);
		link(a,b);
		b->next_seed = a->next_seed;
		return b;
	};

	__forceinline tbucket * find_or_add_bucket(node * i, tbucket * b){
		assert(b!=0);
		assert(b->d<=i->d);
		//
		if(b->d==i->d)return b;
		if(b->next->d==i->d)return b->next; //search only next bucket
		//bucket is not matching, create one just after b
		return add_bucket_after(b,i->d);
	}

public:

	inline tbucket * delete_bucket(tbucket * b){
		//should not be deleting non-empty buckets
		assert(b->first_active==0);
		assert(b->first_inactive==0);
		//never delete first or last buckets, even if empty
		assert(b!=first);
		assert(b!=last);
		//relink the list
		tbucket * c = b->next;
		link(b->prev,c);
		//add b to empty list
		b->next = empty;//empty is singly-linked
		empty = b;
		return c;
	};

	__forceinline void aAdd(node * i, tbucket * b){
		assert(i->state==node::free);
		assert(b!=last);//cant put active nodes in d_inf bucket
		assert(i->d >0);
		//make sure b is right bucket, get a new one when needed
		b = find_or_add_bucket(i,b);
		assert(b!=first);//dont put active nodes to sink bucket
		if(top_active->d<i->d){
			top_active = b;
		};
		//b = find_or_add_bucket(i,b);
		//add node to bucket's active list
		//active is single-linkes
		i->bnext = b->first_active;
		b->first_active = i;
		i->my_bucket = b;
		i->set_state(node::active);
	};

	__forceinline void aRestore(node * i){
		assert(i->state==node::free);
		tbucket * b = i->my_bucket;
		assert(i->d == b->d);
		assert(b!=last);//cant put active nodes in d_inf bucket
		assert(b!=first);//dont put active nodes to sink bucket
		if(top_active->d<i->d){
			top_active = b;
		};
		i->bnext = b->first_active;
		b->first_active = i;
		i->set_state(node::active);
	};

	public:
		// active nodes are deleted only when they are make non-saturating push or relabeled to d_inf
		// use relabel and deactivate functions instead
	inline tbucket * aDelete(node *i){
		assert(i->state==node::active);
		tbucket * b = i->my_bucket;
		assert(b);
		//active is singly-linked, delete only from top
		assert(i==b->first_active);
		b->first_active = i->bnext;
		//clear nodes's links - not necessary
		//a->bnext = 0;
		//if(!b->first_active){//no more active nodes in this bucket
		//	top_active = b->prev;//they must be in the bucket just below - where else?
		//};
		i->set_state(node::free);
		return b;
	};
	public:

	__forceinline tbucket * iAdd(node * i, tbucket * b){
		assert(i->state==node::free);
		assert(i->d < last->d);
		//make sure b is right bucket, get a new one when needed
		b = find_or_add_bucket(i,b);
		//add node to bucket's inactive list
		//inactive is double-linked
		i->bnext = b->first_inactive;
		if(b->first_inactive){
			b->first_inactive->bprev = i;
		};
		b->first_inactive = i;
		i->my_bucket = b;
		i->set_state(node::inactive);
		return b;
	};

	//assume i was in bucket b before, put it in without checks
	__forceinline tbucket * iRestore(node * i){
		assert(i->state==node::free);
		tbucket * b = i->my_bucket;
		assert(i->d == b->d);
		i->bnext = b->first_inactive;
		if(b->first_inactive){
			b->first_inactive->bprev = i;
		};
		b->first_inactive = i;
		i->set_state(node::inactive);
		return b;
	};

	inline tbucket * iDelete(node *i){
		assert(i->state==node::inactive);
		tbucket * b = i->my_bucket;
		//inactive is double-linked, delete from anywhere
		if(i==b->first_inactive){//delete from the top
			b->first_inactive = i->bnext;
			if(b->first_inactive){
				b->first_inactive->bprev = 0;
			};
		}else{
			if(i->bnext){//delete from the middle
				//has bprev and bnext - relink those
				i->bprev->bnext = i->bnext;
				i->bnext->bprev = i->bprev;
			}else{//detele from the end
				i->bprev->bnext = 0;
			};
		};
		i->set_state(node::free);
		return b;
	};

	/*
	void aRelabel_inf(node *i){//set i->d = d_inf, assuming i was active
		aDelete(i);
		iAdd(i,last);
	};

	void aRelabel_deactivate(node *i, node *j){//set i->d = j->d+1, assuming i was active and becomes inactive
		aDelete(i);
		i->d = j->d+1;
		iAdd(i,j->my_bucket);
	};
	*/

	//called when inactive node has recieved flow
	inline void activate(node *i){//move i from inactive to active, same bucket
		assert(i->state==node::inactive);
		tbucket * b = iDelete(i);
		//add to active list, without general checks
		assert(b!=last);//cant put active nodes in d_inf bucket
		assert(b!=first);//dont put active nodes to sink bucket
		assert(i->d == b->d);
		assert(top_active->d>=i->d);
		i->bnext = b->first_active;
		b->first_active = i;
		i->set_state(node::active);
	};

	//called when a non-saturating push occured (i deactivated, but not relabeled)
	//void deactivate(node *i){//move i from active to inactive, same bucket
	//	tbucket * b = aDelete(i);
	//	//add to inactive list, without general checks
	//	i->bnext = b->first_inactive;
	//	if(b->first_inactive){
	//		b->first_inactive->bprev = i;
	//	};
	//	b->first_inactive = i;
	//};

	inline void merge_up(tbucket * a, tbucket * b){
		assert(b->d>a->d);
		for(node * i=a->first_active;i!=0;i=i->bnext){
			assert(i->my_bucket == a);
			assert(i->state==node::active);
			i->my_bucket = b;
			i->d = b->d;
			if(i->bnext==0){//this is the last node in a's active
				i->bnext = b->first_active;
				b->first_active = a->first_active;
				a->first_active = 0;
				break;
			};
		};
		for(node * i=a->first_inactive;i!=0;i=i->bnext){
			assert(i->my_bucket == a);
			assert(i->state==node::inactive);
			assert(i==a->first_inactive || (i->bprev && i->bprev->bnext == i));
			i->my_bucket = b;
			i->d = b->d;
			if(i->bnext==0){//this is the last node in a's inactive
				i->bnext = b->first_inactive;
				if(b->first_inactive){//this should be generally true, for all seed buckets, except maybe d_inf
					b->first_inactive->bprev = i;
				};
				b->first_inactive = a->first_inactive;
				a->first_inactive = 0;
				break;
			};
		};
	};

	/*
	inline void dismiss(tbucket * a){
		for(node * i=a->first_active;i!=0;i=i->bnext){
			//i->my_bucket = 0;
			i->d = last->d;
			i->set_state(node::dead);
		};
		a->first_active = 0;
		for(node * i=a->first_inactive;i!=0;i=i->bnext){
			//i->my_bucket = 0;
			i->d = last->d;
			i->set_state(node::dead);
		};
		a->first_inactive = 0;
	};
	*/

	void clear(){
		for(tbucket *a=first->next;a!=last;a=first->next){
#ifndef NDEBUG
			for(node * i=a->first_active;i!=0;i=i->bnext){
				i->my_bucket = 0;
				i->set_state(node::free);
			};
			for(node * i=a->first_inactive;i!=0;i=i->bnext){
				i->my_bucket = 0;
				i->set_state(node::free);
			};
#endif
			a->first_active = 0;
			a->first_inactive = 0;
			delete_bucket(a);
		};
	};
};

#endif
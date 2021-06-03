#ifndef array_buckets_h
#define array_buckets_h

#include "defs.h"
#include "dynamic/fixed_array1.h"

template<class node> class array_buckets;

//___________________________________________
template<class node> class array_buckets{
public:
	//_______________
	class bucket_node{
	public:
		typedef typename array_buckets<node>::bucket tbucket;
	public:
		int d;
	public:
		node * bnext;
		node * bprev;
	public:
		enum tstate{free=0,active=1,inactive=2,dead=-1} state;	
#ifndef NDEBUG
		bucket_node():bnext(0),bprev(),state(free),d(1){};
		//
		void set_state(tstate st){
			state = st;
		};
#else
		bucket_node():bnext(0),bprev(),d(1){};
		void set_state(int st){};
#endif
	};
	//______________________________________
public:
	class bucket{
	public:
		node * first_active;   //!< list of active nodes in the bucket
		node * first_inactive; //!< list of inactive nodes in the bucket
	public:
		bucket():first_active(0),first_inactive(0){
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
	tbucket * first(){
		assert(pool.size()>0);
		return pool.begin();
	};    //!< sink bucket, only sink is here
	tbucket * last(){
		assert(pool.size()>0);
		return pool.end()-1;
	};     //!< d_inf bucket - all nodes which cant reach sink are here, only inactive nodes
	tbucket * top_active;  //!< pointer to the highest active bucket
public:
	array_buckets():top_active(0){
	};
	~array_buckets(){
	};
public:
	static long long size_required(int n, int d_inf){
		long long sz = 0;
		sz += sizeof(array_buckets<node>);
		sz += sizeof(bucket)*n;
		return sz;
	};
	//! must be called firtst
	void init(int n, int d_inf){
		assert(n>=2);
		pool.resize(d_inf+1);
		top_active = first();
	};
public:
	int d(tbucket * b){
		return b-pool.begin();
	};
	//! add node to active list of the bucket
	__forceinline void aAdd(node * i){
		assert(i->d >0);
		assert(i->state==node::free);
		tbucket * b = node_bucket(i);
		assert(b!=last());//cant put active nodes in d_inf bucket
		assert(b!=first());//dont put active nodes to sink bucket
		if(d(top_active)<i->d){
			top_active = b;
		};
		//add node to bucket's active list
		//active is single-linkes
		i->bnext = b->first_active;
		b->first_active = i;
		i->set_state(node::active);
	};
	public:
		// active nodes are deleted only when they make a non-saturating push or get relabeled to d_inf
		// use relabel and deactivate functions instead
	inline tbucket * aDelete(node *i){
		assert(i->state==node::active);
		tbucket * b = &pool[i->d];
		//active is singly-linked, delete only from top
		assert(i==b->first_active);
		b->first_active = i->bnext;
		i->set_state(node::free);
		return b;
	};
	public:

	__forceinline tbucket * iAdd(node * i){
		assert(i->state==node::free);
		assert(i->d < d(last()));
		//make sure b is right bucket, get a new one when needed
		tbucket * b = &pool[i->d];
		//add node to bucket's inactive list
		//inactive is double-linked
		i->bnext = b->first_inactive;
		if(b->first_inactive){
			b->first_inactive->bprev = i;
		};
		b->first_inactive = i;
		i->set_state(node::inactive);
		return b;
	};

	tbucket * node_bucket(node * u){
		return &pool[u->d];
	};

	inline tbucket * iDelete(node *i){
		assert(i->state==node::inactive);
		tbucket * b = &pool[i->d];
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

	//called when inactive node has recieved flow
	inline void activate(node *i){//move i from inactive to active, same bucket
		assert(i->state==node::inactive);
		tbucket * b = iDelete(i);
		//add to active list, without general checks
		assert(b!=last());//cant put active nodes in d_inf bucket
		assert(b!=first());//dont put active nodes to sink bucket
		assert(i->d == d(b));
		assert(d(top_active)>=i->d);
		i->bnext = b->first_active;
		b->first_active = i;
		i->set_state(node::active);
	};

	void clear(){
		for(tbucket *a=first()+1;a!=last();++a){
#ifndef NDEBUG
			for(node * i=a->first_active;i!=0;i=i->bnext){
				i->set_state(node::free);
			};
			for(node * i=a->first_inactive;i!=0;i=i->bnext){
				i->set_state(node::free);
			};
#endif
			a->first_active = 0;
			a->first_inactive = 0;
		};
	};
};

#endif
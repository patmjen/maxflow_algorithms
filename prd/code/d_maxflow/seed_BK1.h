#ifndef seed_BK1_h
#define seed_BK1_h

#include "dynamic/dynamic.h"
//#include "dynamic/basic_heap.h"
#include "debug/performance.h"

#include "dimacs_parser.h"
#include "maxflow_solver.h"
#include "exttype/arrays.h"


#include <string.h>
//#include "maxflow-v3.0/block.h"
#include "exttype/fixed_vect.h"

//__________________________________________________
class seed_BK : public maxflow_solver, public dimacs_parser_callback{
public:
	typedef int tcap;
	typedef long long tflow;
public:
	class node;
	class arc;
	//_________________________________________________
	class node{
	public:
		tcap	excess;		//!< if excess > 0 then excess is residual capacity of the arc SOURCE->node
		//!  otherwise         -excess is residual capacity of the arc node->SINK
		arc * arc_first;
		arc * arc_end;
		arc			*parent;	//!< node's parent
		node		*next;		//!< pointer to the next active node
		node * next_orphan;
//		int global_index;		//!< remember the vertex global index when solving by parts
		int			DIST;		//!< distance to the terminal
		int			TS;			//!< timestamp showing when DIST was computed
		int d;					//!< region distance, in case when there are no seeds:
								//!< d=-1 sink tree, d=0 free node, d=1 source tree
	public:
		node(){};
		bool is_open()const{return next!=0;};
		//void clear_open(){next = 0;};
		bool is_orphan()const{return next_orphan!=0;};
		void clear_orphan(){next_orphan = 0;};
	};
	//__________________________________________
	class arc{
	public:
		node		*head;		//!< node the arc points to
		//		arc			*next;		//!< next arc with the same originating node
		arc			*sister;	//!< reverse arc
		tcap		r_cap;		//!< residual capacity
	public:
		arc(){};
	public:
		tcap & cap(){return r_cap;};
		tcap & rev_cap(){return sister->r_cap;};
	};

	//__________________________________________
	class node_queue{
	public:
		node * first_open; // bucket has a list of open nodes (those which can grow) which is common for all trees with roots in this bucket
		node * last_open;  // this list is actually used as a FIFO queue
		int d;			   // common label of the bucket
	public:
		node_queue():first_open(0),last_open(0),d(0){
		};
	public:
		void push(node * q);
		node * top();
		node * pop();
		bool is_empty()const{return first_open==0;};
		//void reserve(int n){};
		void clear(){
			first_open = 0;
			last_open = 0;
		};
	};
	//___________________________________________
	class orphan_queue{
	public:
		node * first_orphan;
		node * queue_orphan;
	public:
		orphan_queue(){
			first_orphan = 0;
			queue_orphan = 0;
		};
		void push(node * q);// add a node on the top
		node * pop();
		void insert(node * q);// adds the node in the middle, where the current queue ends
		bool is_empty()const{return first_orphan==0;};
		void clear(){
			first_orphan = 0;
			queue_orphan = 0;
		};
	};
	//________________________node_buckets______________________________
	class node_buckets: public dynamic::fixed_array1<node_queue>{
	public:
		int current_bucket;
	public:
		/*
		int find_bucket(int d, int l = 0){
			int u = size()-1;
			assert(l<=u);
			while((*this)[u].d != d){
				int b = (u+l)/2;
				if((*this)[b].d >= d){
					u = b;
				}else{
					l = b+1;
				};
			};
			return u;
		};
		*/
		int find_above(int d, int l = 0){
			int u = size()-1;
			while(l<u){
				int b = (u+l)/2;
				if((*this)[b].d >= d){
					u = b;
				}else{
					l = b+1;
				};
			};
			assert(u==l);
			return u;
		};
		int find_below(int d){
			int u = size()-1;
			int l = 0;
			while(l<u){
				int b = (u+l+1)/2;
				if((*this)[b].d <= d){
					l = b;
				}else{
					u = b-1;
				};
			};
			assert(u==l);
			return u;
		};
		//! put the node v in the open list of the bucket with label v->d, if no such bucket return false
		bool push(node * v){
			//assert(v->d<d_free);
			if(v->is_open())return true;
			assert(v->d>= (*this)[current_bucket].d);
			//assert(v->d<=back().d);
			int b = find_above(v->d, current_bucket);
			if((*this)[b].d==v->d){
				(*this)[b].push(v);
				return true;
			};
			return false;
		};
		bool push(node * v, int d){
			if(v->is_open())return true;
			assert(v->d>= (*this)[current_bucket].d);
			int b = find_above(d, current_bucket);
			if((*this)[b].d==d){
				(*this)[b].push(v);
				return true;
			};
			return false;
		};
		bool push_at(node * v, int d){
			if(v->is_open())return true;
			int b = find_below(d);
			(*this)[b].push(v);
			return true;
		};
	};
	/*
	//___________________________________________
		struct nodeptr{
		node    	*ptr;
		nodeptr		*next;
	};
	static const int NODEPTR_BLOCK_SIZE = 128;
	node				*node_last, *node_max; //!< node_last = nodes+node_num, node_max = nodes+node_num_max;
	arc					*arc_last, *arc_max; //!< arc_last = arcs+2*edge_num, arc_max = arcs+2*edge_num_max;
	int					node_num;
	DBlock<nodeptr>		*nodeptr_block;
	tflow				flow;		//! total flow
	//! reusing trees & list of changed pixels
	int					maxflow_iteration; //! counter
	/////////////////////////////////////////////////////////////////////////
	node				*queue_first[2], *queue_last[2];	//! list of active nodes
	nodeptr				*orphan_first, *orphan_last;		//! list of pointers to orphans
	int					TIME;								//! monotonically increasing global counter
	/////////////////////////////////////////////////////////////////////////
	*/
	//! functions for processing active list
	//void set_active(node *i);
	//node *next_active();
	//! functions for processing orphans list
	//void set_orphan_front(node* i); //! add to the beginning of the list
	//void set_orphan_rear(node* i);  //! add to the end of the list

	void maxflow_init();             //! called if reuse_trees == false
	void augment(arc *middle_arc);
	void process_source_orphan(node *i);
	void process_sink_orphan(node *i);
	void process_level();
	tflow maxflow(bool reuse);
	tflow maxflow(){return maxflow(false);};
	tflow cut_cost(int * C);
	void save_cut(const std::string & filename)override;
	virtual void get_cut(int * S)override;//!< read out minimum cut, S must be allocated, S[v] \in {0,1} -- indicator if the source set of the cut
	virtual long long cut_cost()override;
//	void test_consistency(node* current_node=NULL); //! debug function
	void check();
public:// discharge interface
	void disconnect_sink(node * p);
	void discharge0();
	void discharge(int max_level);
	void relabel();
//	void raise_seed(node * p , int d);
	void update_seeds();
	bool is_child(node * q,node * p);// test if q is child of p
	void clear_sink_tree(node * p);
	//void clear_boundary();
	void msg_in(int a, int d, tcap f);
	void msg_out(int a, int * d, tcap * f);
/*
	void seed_BK::msg_in(int j, int d, tcap f){
		arc * a = &arcs[j]; // outcoming arc u->v
		node * u = a->sister->head;
		node * v = a->head;
		assert(d!=d_inf-1);
		//		assert(v->parent == TERMINAL);
		assert(v-nodes.begin()>=nV);//boundary node
		assert(v->d<=d);//cannot go down
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
		//int d_old = *d;
		*d = std::min(u->d+1,d_inf);// free and source vertices get d_inf
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
*/
	int get_d(int u);
	//___________________________________________________
public:
	int nV;
	int nB;
	int nE;
	int nE_out;
	exttype::intf E_out;
	exttype::intf global_index;
	//int * E_out;
	int n;
	int m;
	dynamic::fixed_array1<node> nodes;
	dynamic::fixed_array1<arc> arcs;
	int S,T;//dummy
	tflow	flow;//! total flow
public:// internal
	int	TIME;//! monotonically increasing global counter
	int d_inf;
	int d_free;
	dynamic::fixed_array1<node*> BS; //boundary nodes, sorted by distance
	exttype::intf d_old;
	orphan_queue orphans[2];
	node_queue * open;
	int current_d;
	node_buckets open_buckets;
	int maxflow_iteration;
	bool reuse;
	int dead;
	int max_level;
	bool has_active;
private:
	bool is_bnd(node * q)const;
	bool is_free(node * q)const;
	bool is_source(node * q)const;
	bool is_sink(node * q)const;
public:
	bool is_sink(int v)const;
	bool is_weak_source(int v)const;
private:
	template<bool source> bool opposite_tree(node * q)const;
	bool same_tree(node * p, node * q)const;
	void set_free(node *q);
public:
	seed_BK();
	~seed_BK();
public:
	__forceinline void init_arc(node *u, node *v, arc* uv, arc * vu, tcap cap1, tcap cap2);
	size_t load(const std::string & id);
	size_t save(const std::string & id);
	size_t unload(const std::string & id);//save and free memory
	void construct(const char * filename);// read from DIMACS file
public://construction interface
	static long long size_required(int nV, int nB, int nE, int nE_out,int d_inf);
	void reset_counters();
	void allocate1(int nV, int nB, int nE, int nE_out =0, int *E_out=0, int d_inf=0, int S=-1, int T=-1);
	virtual void allocate1(int n ,int m, int S, int T,int d,int * sz)override;
	virtual void allocate2(int loop)override;
	void add_tweights(int u,int cap1, int cap2);
	virtual void read_arc(int loop,int u,int v,int cap1, int cap2)override;
	int g_index(int v);
};

#endif
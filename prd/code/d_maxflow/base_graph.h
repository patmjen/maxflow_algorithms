#ifndef base_graph_h
#define base_graph_h

#include "dynamic/fixed_array1.h"
#include <map>

#define MAX_SHIFTS 128

template<class node, class arc> class base_graph_data{
public:
	int nV;      //!< number of vertices in the graph, source and sink not counted
	int nB;      //!< number of extra boundary vertices
	int n;       //!< total number of vertices = nV+nB+2
	int nE;      //!< number of input directed arcs
	int m;       //!< total number of non-zero directed arcs in the multigraph
	int nE_out;  //!< number of outcoming boundary edges (from V to E)
	int * E_out; //!< saves local indices for arcs nE-nE_out...nE-1 's arcs of the input
	int S;       //!< source node index
	int T;       //!< sink node index
	tflow flow;  //!< flow value
	dynamic::fixed_array1<node> nodes; //storage for nodes
	dynamic::fixed_array1<arc>  arcs;  //storage for arcs
public:
	void allocate1(int nV, int nB, int nE, int nE_out, int *E_out, int S, int T){
		this->E_out = E_out;
		this->nE_out = nE_out;
		this->S = S;
		this->T = T;
		this->nV = nV;
		this->nB = nB;
		n = nV+nB;
		m = 0;//2*nE;
		nodes.resize(n+1);
		flow = 0;
	};
	static long long size_required(int nV, int nB, int nE, int nE_out){
		long long r = 0;
		r+=sizeof(node)*(nV+nB);
		r+=sizeof(arc)*2*nE;
		return r;
	};
	void reset_counters(){
		m = 0;
	};
	void add_tweights(int u,int cap1,int cap2){
		int delta = nodes[u].excess;
		if (delta > 0) cap1 += delta;
		else           cap2 -= delta;
		flow += (cap1 < cap2) ? cap1 : cap2;
		nodes[u].excess = cap1-cap2;
	};
};

template<class node, class arc> class base_graph : public base_graph_data<node,arc>{
public:
	class graph_arc;
	class arc_ptr;
	class graph_node;
	//_______________________
	class graph_arc{
	private:
		node * head;
		arc * rev;
		friend class base_graph;
	public:
		tcap resCap;
	};
	//_______________________
	class arc_ptr{
	private:
		arc * ptr;
		arc * end;
		friend class base_graph;
	public:
		arc_ptr & operator ++(){
			++ptr;
			return *this;
		};
		//bool allowed(){
		//	return ptr<end;
		//};
		bool operator !=(arc_ptr & b){
			return ptr!=b.ptr;
		};
		arc * operator->(){return ptr;};
	public:
		explicit arc_ptr(arc * a, arc * End):ptr(a),end(End){
		};
		arc_ptr(){};//uninitialized
	};
	//_______________________
	class graph_node{
	private:
		arc * arc_first;
		arc * arc_end;
		friend class base_graph;
	public:
		tcap excess;
		graph_node():excess(0),arc_first(0){};
	};
	//_______________________
	arc_ptr arc_first(node * u){
		return arc_ptr(u->arc_first,u->arc_end);
	};
	arc_ptr arc_for_index(int i){
		return arc_ptr(&this->arcs[i],&this->arcs[i]+1);
	};
	arc * arc_rev(arc_ptr & a){
		return a->rev;
	};
	bool has_head(node * u, arc_ptr & a){
		return true;
	};
	node * arc_head(node * u, arc_ptr & a){
		return a->head;
	};
	node * arc_tail(arc_ptr & a){
		return a->rev->head;
	};
	bool allowed(arc_ptr & a){
		return a.ptr<a.end;
	};
public:
public:// construction interface
	void init_arc(node *u, node *v, arc* uv, arc * vu, tcap cap1, tcap cap2){
		uv->head = v;
		vu->head = u;
		uv->rev = vu;
		vu->rev = uv;
		uv->resCap = cap1;
		vu->resCap = cap2;
	};
	void add_edge(int u,int v,int cap1, int cap2,int loop){
		if(loop==0){
			if(u==this->S){
			}else if(v==this->T){
			}else{//regular edge
				this->m+=2;
				++(size_t&)this->nodes[u].arc_first;//for now will count how many outcoming arcs
				++(size_t&)this->nodes[v].arc_first;
			};
		}else{//loop==1
			if(u==this->S){
				add_tweights(v,cap1,0);
			}else if(v==this->T){
				add_tweights(u,0,cap1);
			}else{//regular edge
				//fill in arcs
				arc *& uv = this->nodes[u].arc_end;
				arc *& vu = this->nodes[v].arc_end;
				assert(uv>=arcs.begin() && uv<arcs.end());
				assert(vu>=arcs.begin() && vu<arcs.end());
				int e = this->m/2;
				if(e>=this->nE-this->nE_out){//this is outcoming boundary arc
					cap2=0;
					if(this->E_out){
						this->E_out[e-(this->nE-this->nE_out)] = uv-this->arcs.begin();//save the new position of this arc
					};
				};
				init_arc(&this->nodes[u],&this->nodes[v],uv,vu,cap1,cap2);
				++uv;
				++vu;
				this->m+=2;
			};
		};
	};
	void allocate2(){
		this->arcs.resize(this->m);
		size_t accum_size = 0;
		//compute desired starting positions for where to allocate arcs
		for(node * v = this->nodes.begin();v!=this->nodes.end();++v){
			size_t s = (size_t&)v->arc_first;
			v->arc_first = this->arcs.begin()+accum_size;
			v->arc_end = v->arc_first;
			accum_size+=s;
		};
		assert(accum_size<=m);
		this->nE = this->m/2;
	};
};
//
//
//
//
//_________________________________base_graph_n______________________________
template<class node, class arc> class base_graph_n : public base_graph_data<node,arc>{
    public:
    typedef base_graph_data<node,arc> parent;
public:
	class graph_arc;
	class arc_ptr;
	class graph_node;
	//_______________________
	class graph_arc{
	public:
		friend class base_graph_n;
		tcap resCap;
		graph_arc():resCap(0){};
	};
	//_______________________
	class arc_ptr{
	private:
		arc * parc;
		int shift;
		friend class base_graph_n;
	public:
		arc_ptr & operator ++(){
			++shift;
			++parc;
			return *this;
		};
		arc * operator->(){
			return parc;
		};
		bool operator !=(arc_ptr & b){
			return parc!=b.parc;
		};
	public:
		arc_ptr(){};//uninitialized
		explicit arc_ptr(arc * pArc, int Shift):parc(pArc),shift(Shift){
		};
	};
	//_______________________
	class graph_node{
	private:
		arc * arc_first;
	public:
		friend class base_graph_n;
		tcap excess;
		graph_node():excess(0){};
	};
public:
	class stride_struct{
	public:
		int s_node;
		int arc_rev;
		stride_struct():s_node(0),arc_rev(0){};
	};
	stride_struct stride[MAX_SHIFTS];
	int n_shifts;
	std::map<int,int> stride_shifts;
public:
	//_______________________
	arc_ptr arc_first(node * u){
		//int node_idx = u-nodes.begin();
		//return arc_ptr(arcs.begin()+node_idx*n_shifts,u,0);
		return arc_ptr(u->arc_first,0);
	};
	//
	arc * arc_rev(arc_ptr & a){
		return a.parc+stride[a.shift].arc_rev;
	};
	//
	bool has_head(node * u, arc_ptr & a){
		node * v = u + stride[a.shift].s_node;
		return v>=this->nodes.begin() && v<this->nodes.end();
	};
	//
	arc_ptr arc_for_index(int i){
		arc_ptr r;
		//r.tail = nodes[(i/n_shifts)*n_shifts];
		r.shift = i % n_shifts;
		r.parc = &this->arcs[i];
		return r;
	};
	//
	node * arc_head(node * u, arc_ptr & a){
		return u + stride[a.shift].s_node;
	};
	//
	//node * arc_tail(arc_ptr & a){
	//	return a.tail;
	//};
	bool allowed(arc_ptr & a){
		return a.shift<n_shifts;
	};
	//
public:// construction interface
	base_graph_n():n_shifts(0){
	};
	void set_structure(int n_shifts){
		if(n_shifts>MAX_SHIFTS){
			throw debug_exception("Too many shifts, increase MAX_SHIFTS");
		};
		this->n_shifts = n_shifts;
	};
	void allocate1(int nV, int nB, int nE, int nE_out, int *E_out, int S, int T){
		parent::allocate1(nV,nB,nE,nE_out,E_out,S,T);
		this->m = this->n*n_shifts;
		this->arcs.resize(this->m);
		this->nE = nE;
		this->m = 0;
	};
	//
	void add_edge(int u,int v,int cap1, int cap2,int loop){
		if(loop==0){
			if(u!=this->S && u!=this->T){
				this->m+=2;
			};
			return;
		};
		if(u==this->S){
			add_tweights(v,cap1,0);
		}else if(v==this->T){
			add_tweights(u,0,cap1);
		}else{//regular edge
			//calculate stride
			int strd = (v-u);
			//which stride of the structure
			std::map<int,int>::iterator it = stride_shifts.find(strd);
			int shift;
			if(it==stride_shifts.end()){//not found, make it a new stride
				shift = stride_shifts.size(); //assign new shift
				if(shift>=n_shifts-1){
					throw debug_exception("Using more shifts in the graph structure than reserved");
				};
				stride_shifts.insert(std::pair<int,int>(strd,shift));//map this shift
				stride_shifts.insert(std::pair<int,int>(-strd,shift+1));//map the reverse shift
				// precalculated offsets
				stride[shift].s_node = strd;    //arc stride
				stride[shift].arc_rev = strd*n_shifts + (shift ^ 1) - shift;
				stride[shift+1].s_node = -strd;    //arc stride
				stride[shift+1].arc_rev = -strd*n_shifts + ((shift+1) ^ 1) - (shift+1);
			}else{//stride is already known
				shift = it->second;
			};
			//assign capacities
			//arc_ptr a(arcs.begin()+u*n_shifts+shift,&nodes[u],shift);
			arc_ptr a(this->arcs.begin()+u*n_shifts+shift,shift);
			// check if need to save the arc index
			int e = this->m/2;
			if(e<this->nE && e>=this->nE-this->nE_out){//this is outcoming boundary arc
				cap2=0;
				if(this->E_out){
					this->E_out[e-(this->nE-this->nE_out)] = a.operator->()-this->arcs.begin();//save the new position of this arc
				};
			};
			//
			a->resCap+=cap1;
			arc_rev(a)->resCap+=cap2;
			//
			this->m+=2;
		};
	};
	void allocate2(){
		this->nE = this->m/2;
		for(int i=0;i<this->nV;++i){
			this->nodes[i].arc_first = this->arcs.begin()+i*n_shifts;
		};
	};
};

#endif

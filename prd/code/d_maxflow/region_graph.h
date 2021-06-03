#ifndef region_graph_h
#define region_graph_h

#include "dynamic/dynamic.h"
#include <omp.h>

//! Boundary graph for a region partition
/*! Its nodes represents regions in the partition
	Its edges are edges are all inter-region edges of the problem,
	they act as messages: how much flow to transfer and whether the label is raisen.
*/
class region_graph{
public:
	class region;
	class region_arc;
	class d_group;
	class arc{
	public:
		union{
			int head_region;
			int head_node;
		};
		arc * rev;
		//arc * next;
		int old_d;// tail old d, debug
		int d; //!< tail d
		int f; //!< new flow on this arc
		int local_index;//!< index of this boundary edge in the tail region
		int r_cap; //!< current residual capacity
		//boundary relabel heuristic
		arc * next; //!<next arc in the same group (same origin in splitter or same origin distance label in bnd_relabel)
		d_group * group;
	public:
		arc();
	};

	class region{
	public:
		int nV;        //!< num region regular nodes
		int nB;        //!< num region biundary nodes 
		int nE;        //!< num region total edges
		int nE_out;    //!< num region boundary edges
		std::string id;//!< name for load/unload file
		int nRE;	   //!< num neighboring regions
		dynamic::fixed_array1<arc> out; // list of all boundary edges, coming out of the region
		dynamic::fixed_array1<region_arc> Out; // list of all boundary edges, coming out of the region
	public:
		FILE * f;	   //!< file handler for the region part, created by splitter
		int file_type;// 0 capacities, 1 full page
		dynamic::page_allocator * page;
		bool loaded;
		bool constructed;
		bool preprocessed;
		int has_active;
		int labels_changed;//!< region has new label updates in the incoming messages
		long long flow; //!< flow to the sink in the region
		long long dead; //!< nomber of decided nodes
		int global_gap; //!< track the best known gap for this region (global gap heuristic)
		int d_LB;
		int max_d;//!< keeps track of largest label in the region
	public://temporary
		int last_u; //splitter
		int nexcess;
	public://boundary relabel
		d_group * g_first;
	public:
		region();
	protected:
		friend class region_graph;
		int last_r;
	};
	//! grou of boundary edges having the same tail distance label, used in boundary relabe lheuristic
	class d_group{
	public:
		int relabel_d; //!< new distance in boundary-relabel heuristic
		arc * arc_first; //!< first boundary edge in the group
		d_group * g_next; //!< next group in the region with lower label
		bool is_open;
	public:
		d_group();
	};
	//
	class region_arc{//regions heighbourhood relation
	public:
		region * head;
		region_arc * rev;
		omp_lock_t lock;
	};
public:
	int nV; //total nodes
	int nB; //total boundary vertices
	int nE; //total edges
	int nBE; //!< number of boundary edges
	int nR; //num regions
	int nRE;//num region edges
	dynamic::fixed_array1<region> nodes;
	std::string pth;
	int S,T;// source and sink vertices
	//dynamic::fixed_array1<region_arc> arcs;
public:
	void init_region_arcs();
public:
	region & operator[](int r){return nodes[r];};
public:
	void save(const char * filename);
	bool load(const char * filename);
};

/*

class region_edge{// R-edge between two regions
public:
	class tmsg{
	public:
		int f;
		int d;
	public:
		tmsg():f(0),d(0){};
	};
public:
	int head;//! region q 
	region_edge * rev; // reverse region edge
	int m; //! num edges
	dynamic::fixed_array1<exttype::mint2> emap; // store pairs (e_r,e_q), where e_r - edge index in r and e_q corresponding edge index in q
	dynamic::fixed_array1<tmsg> msg; //message from region r to region q: labeling and flow over boundary edges from r to q
	//region_edge(int h):head(h){};
	region_edge():rev(0),m(0),head(-1){};
	region_edge(int _head):rev(0),m(0),head(_head){};
};

class region_node{
public:
	int nV;        //!< num region regular nodes
	int nB;        //!< num region biundary nodes 
	int nE;        //!< num region total edges
	int nE_out;    //!< num region boundary edges
	std::string id;//!< name for load/unload file
public:
	dynamic::fixed_array1<region_edge> out; //!< list of outcoming r-edges
public:
	FILE * f;	   //!< file handler
	int file_type;// 0 capacities, 1 full page
	dynamic::page_allocator * page;// page1, page2
	bool loaded;
	bool constructed;
	bool preprocessed;
	bool has_active;
	bool labels_changed;
	long long flow;
	long long dead;
	int global_gap;
	int d_LB;
public://temporary
	int last_u;
	int nexcess;
public:
	region_node():nV(0),nB(0),nE(0),nE_out(0),f(0),last_u(-1),nexcess(0){
		file_type = 0;
		page = 0;
		loaded = 0;
		constructed = 0;
		preprocessed = 0;
		flow = 0;
		has_active = true;
		dead = 0;
	};
	~region_node(){
	};
public:
	region_edge * find_out_head(int q){
		int i=0;for(;i<out.size();++i)if(out[i].head==q)break;
		if(i!=out.size()){
			return &out[i];
		}else{
			return 0;
		};
	};
private://forbidden
	region_node(const region_node & r){
	};
};

class region_graph{
public:
	int nV; //total nodes
	int nB; //total boundary vertices
	int nE; //total edges
	int nBE; //!< number of boundary edges
	int nR; //num regions
	int nRE;//num region edges
	dynamic::fixed_array1<region_node> nodes;
	//edges are allocated directly in nodes[r].out
	//dynamic::fixed_array1<region_edge> edges;
	std::string pth;
public:
	region_node & operator[](int r){return nodes[r];};
public:
	void save(const char * filename);
	bool load(const char * filename);
};

*/

#endif

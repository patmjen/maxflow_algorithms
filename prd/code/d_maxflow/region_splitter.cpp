#include "region_splitter.h"

#include "debug/except.h"
#include <stdio.h>
#include "debug/performance.h"
#include "dimacs_parser.h"

#include <cstdlib>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
    int _mkdir(const char * pth){
        mkdir(pth,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    };
#endif
//#include <boost/filesystem.hpp>

int region_splitter::local_node(int u){
	return nodes[u].local_index;
};

int region_splitter::get_nR(){
	int nR=1;
	for(int l=0;l<d;++l)nR*=slice[l];
	return nR;
};

region_splitter::region_splitter(const std::string & filename, region_graph * _G, const exttype::intf & sslice):G(_G){
	slice = sslice;
	name = filename;
};

int region_splitter::region(int v){//!< which region is v
	v -= 2;//HACK, assuming {s,t}={0,1} and grid starts from 2
	int ii[4];
	for(int l = 0;l<d-1;++l){
		ii[l] = v % sz[l];
		v = v / sz[l];
	};
	ii[d-1] = v;
	//ii now are grid coordinates of the vertex v
	//slice in pieces in each dim
	int r=0;
	for(int l=d-1;l>=0;--l){
		ii[l] = ii[l]/((sz[l]+slice[l]-1)/slice[l]);
		r = r*slice[l]+ii[l];
	};
	return r;
};

void region_splitter::write_arc(int r,int lu, int lv, int cap1, int cap2){
	fwrite(&lu,sizeof(lu),1,(*G)[r].f);
	fwrite(&lv,sizeof(lv),1,(*G)[r].f);
	fwrite(&cap1,sizeof(cap1),1,(*G)[r].f);
	fwrite(&cap2,sizeof(cap2),1,(*G)[r].f);
};

void region_splitter::reset_counters(){
	for(int r=0;r<G->nR;++r){
		(*G)[r].nE = 0;//count the total number of edges in the region network
		(*G)[r].nE_out = 0;
	};
	G->nE = 0; //total number of edges in the full network
	G->nBE = 0; //total number of boundary edges
};


void region_splitter::allocate1(int n ,int m, int S, int T, int d, int * Sz){
	//this->S = S;
	//this->T = T;
	G->S = S;
	G->T = T;
	if(slice.size()==1){
		d = 1;
	};
	this->d = d;
	G->nV = n;
	sz.resize(d);
	for(int i=0;i<d;++i)sz[i] = Sz[i];
	if(d==1){
		sz[0]=G->nV;
		if(slice.size()>1){
			slice[0] = slice[0]*slice[1];// split unstructured problems (multiview) in fewer peaces
			slice.resize(1);
		};
	};
	if(d<slice.size()){
		slice.resize(d);
	};
	// debug::stream<<"split: ";
	// for(int i=0;i<d;++i)debug::stream<<slice[i]<<" ";
	// debug::stream<<"\n";
	G->nR = get_nR();
	G->nodes.resize(G->nR);
	nodes.resize(G->nV);
	char ss[100];
	sprintf(ss,"%i",G->nR);
	//G->pth = (name+"_reg"+ss+"/");
	G->pth = (name+"_reg/");
	//boost::filesystem::create_directory(G->pth.c_str());
	// To avoid error we just do this hack
	std::string mkdir_command = "mkdir -p ";
        mkdir_command += G->pth;
        std::system(mkdir_command.c_str());
	//_mkdir(G->pth.c_str());
	for(int r=0;r<G->nR;++r){
		(*G)[r].nV = 0;
		//(*G)[r].out.reserve(G->nR);//R-edges
		char s[200];
		sprintf(s,"r%3i.cap",r);
		(*G)[r].id = s;
		(*G)[r].f = fopen((G->pth+(*G)[r].id).c_str(),"wb+");
		if(!(*G)[r].f){
			throw debug_exception(std::string("Cant open file ")+G->pth+(*G)[r].id);
		};
		setvbuf((*G)[r].f,NULL,_IOFBF,1024*1024*2);
		fwrite(&(*G)[r].nV,sizeof((*G)[r].nV),1,(*G)[r].f);// these are stubms here, will overwrite later
		fwrite(&(*G)[r].nB,sizeof((*G)[r].nB),1,(*G)[r].f);
		fwrite(&(*G)[r].nE,sizeof((*G)[r].nE),1,(*G)[r].f);
		fwrite(&(*G)[r].nE_out,sizeof((*G)[r].nE_out),1,(*G)[r].f);
		fwrite(&(*G)[r].nexcess,sizeof((*G)[r].nexcess),1,(*G)[r].f);
	};
	//count vertices per region and assign local index and write global index into regions
	for(int v=2;v<G->nV;++v){
		int r = region(v);
		nodes[v].local_index = (*G)[r].nV;
		++(*G)[r].nV;
		fwrite(&v,sizeof(v),1,(*G)[r].f);
	};
	reset_counters();
};

//! count nBE, construct R-edges, count boundary edges per R-edge
void region_splitter::arc_loop1(int u,int v){
	++G->nE;
	int r = region(u);
	int q = region(v);
	if(r!=q){//boundary edge
		++G->nBE; //total boundary edges
		++(*G)[r].nE_out;//region boundary edges
		++(*G)[q].nE_out;//region boundary edges
	};
	++(*G)[r].nE; // total edges in region
};


// Allocate boundary edges, boundary edges per R-edge
void region_splitter::allocate2(int loop){
	if(loop==0){
		// debug::stream<<"nBE: "<<G->nBE<<"\n";
		//bnd.resize(2*G->nBE);
		for(int r=0;r<G->nR;++r){
			(*G)[r].out.reserve((*G)[r].nE_out);
			/*
			//allocate boundary edges per R-edge
			for(int j=0;j<(*G)[r].out.size();++j){
				(*G)[r].out[j].emap.reserve((*G)[r].out[j].m);
				(*G)[r].out[j].msg.resize((*G)[r].out[j].m);
			};
			*/
		};
		reset_counters();
	};
};


// write inner arcs, remember boundary arcs, recount nBE
void region_splitter::arc_loop2(int u,int v,int cap1, int cap2){
	++G->nE;
	int r = region(u);
	int q = region(v);
	if(r==q){//inner edge
		int lu=local_node(u);//local index of u in r
		int lv=local_node(v);
		//save this arc to region r
		write_arc(r,lu,lv,cap1,cap2);
		++(*G)[r].nE;
	}else{//boundary edge, remember it
		region_graph::arc * a = (*G)[r].out.push_back();
		region_graph::arc * b = (*G)[q].out.push_back();
		a->head_node = v;
		b->head_node = u;
		a->rev = b;
		b->rev = a;
		a->r_cap = cap1;
		b->r_cap = cap2;
		a->next = nodes[u].first_bnd;
		nodes[u].first_bnd = a;
		b->next = nodes[v].first_bnd;
		nodes[v].first_bnd = b;
		++G->nBE;
	};
};


void region_splitter::allocate3(){
	for(int r=0;r<G->nR;++r){
		(*G)[r].nE_out = 0;
	};
	G->nB = 0;
	//write out boundary arcs to region networks
	//loop over boundary edges in the order of the origin vertex
	//count # boundary nodes per region
	for(int u=0;u<G->nV;++u){
		int r = region(u);
		if(nodes[u].first_bnd){
			++G->nB;
		};
		for(region_graph::arc * a = nodes[u].first_bnd; a; a = a->next){//over outcoming bnd arcs, if any
			int v = a->head_node;
			int q = region(v);
			if((*G)[q].last_u!=u){//this is new bnd node for q
				(*G)[q].last_u = u;
				++(*G)[q].nB;
			};
			//write v->u arc to region q
			write_arc(q,local_node(v),(*G)[q].nV+(*G)[q].nB-1,a->rev->r_cap,a->r_cap);
			a->rev->local_index = (*G)[q].nE;
			a->head_region = q;
			++(*G)[q].nE;
			++(*G)[q].nE_out;
		};
	};
	//networks are ready
	// debug::stream<<"splitter, nV:"<<G->nV<<" nE:"<<G->nE<<" nB:"<<G->nB<<"  nBE:"<<G->nBE<<"\n";
	for(int r=0;r<G->nR;++r){
		rewind((*G)[r].f);
		fwrite(&(*G)[r].nV,sizeof((*G)[r].nV),1,(*G)[r].f);
		fwrite(&(*G)[r].nB,sizeof((*G)[r].nB),1,(*G)[r].f);// now correct nB
		fwrite(&(*G)[r].nE,sizeof((*G)[r].nE),1,(*G)[r].f);
		fwrite(&(*G)[r].nE_out,sizeof((*G)[r].nE_out),1,(*G)[r].f);
		fwrite(&(*G)[r].nexcess,sizeof((*G)[r].nexcess),1,(*G)[r].f);
		fclose((*G)[r].f);
	};
	G->init_region_arcs();
};

void region_splitter::read_arc(int loop,int u,int v,int cap1, int cap2){
	if(loop==0){
		if(u==G->S){// write this arc to file as excess of v
			int q = region(v);
			int lv = local_node(v);
			fwrite(&lv,sizeof(lv),1,(*G)[q].f);// local node index
			fwrite(&cap1,sizeof(cap1),1,(*G)[q].f); // excess
			++(*G)[q].nexcess;
		}else if(v == G->T){// write this arc to file as deficiete of u
			int r = region(u);
			int lu = local_node(u);
			fwrite(&lu,sizeof(lu),1,(*G)[r].f); // local node index
			cap1 = -cap1;
			fwrite(&cap1,sizeof(cap1),1,(*G)[r].f); // excess
			++(*G)[r].nexcess;
		}else{// this is an arc
			arc_loop1(u,v);
		};
	}else{
		if(u==G->S || v == G->T){
		}else{
			arc_loop2(u,v,cap1,cap2);
		};
	};
};

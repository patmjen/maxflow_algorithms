#include "PRD.h"

/*
void PRD::construct(){
	g = new seed_hi_pr();
	//have: E, cap, excess, nV, nB -> make the network
	g->construct(nV, nE, &E[0], &cap[0], &excess[0],nB,nE_out,&E_out[0],d_inf);
	// substitute new indices emaps later
	//
	// init all labels
	//for(int i=0;i<nV+nB;++i){
	//	g->nodes[i].d = d[i];
	//};
};
*/

/* first stage  -- maximum preflow*/

long long PRD::maxflow(bool reuse){
	g->stageOne(reuse);
	return g->flow;
};

void PRD::discharge(){
	has_active = false;
	g->stageOne(reuse);
	F = g->flow;
	reuse = true;
	//global_gap = d_inf;
};

void PRD::relabel(){
	if(reuse){
		g->update_seeds();
	}else{
		g->global_update();
		reuse = true;
		//global_gap = d_inf;
	};
	//g->global_update();
	//reuse = true;
	//optional
};

/*
inline int PRD::d_out(int e){
	return g->arcs[e].rev->head->d;
};

inline void PRD::d_in(int e,int new_d){
	seed_hi_pr::node * i = g->arcs[e].head;
	i->d = new_d;
	//if(reuse && i->d<new_d){
	//	g->raise_seed(i,new_d);
	//	reuse = false; //(use global_update to recompute seeds)
	//}else{
	//	i->d = new_d;
	//};
};
*/

inline PRD::tcap PRD::flow_out(int e){
	//return g->arcs[e].rev->resCap;
	hpr::arc_ptr a = g->arc_for_index(e);
	return g->arc_rev(a)->resCap;
};

void PRD::flow_in(int e, tcap f){
	assert(f>=0);
	hpr::arc_ptr a = g->arc_for_index(e);
	assert(g->arc_rev(a)->resCap == 0); //clear the reverse arc;
	{
		//seed_hi_pr::node * v = g->arcs[e].rev->head;
		hpr::node * v = g->arc_tail(a);
		//seed_hi_pr::node * u = g->arcs[e].head;
		hpr::node * u = g->arc_head(v,a);
		//assert(g->arcs[e].resCap+f == 0 || v->d==d_inf || v->d <= u->d+1);//validity check
		assert(a->resCap+f == 0 || v->d==d_inf || v->d <= u->d+1);//validity check
	};

	if(f>0){
		//has_active = true;
		//augment f on (v,u)
		a->resCap += f; //increases capacity of e=(u,v) by f
		//seed_hi_pr::node * v = g->arcs[e].rev->head;
		hpr::node * v = g->arc_tail(a);
		//assert(v->d<d_inf && v->state!=bucket_node<seed_hi_pr::node>::dead);
		//if(reuse && i->excess==0){//if v was inactive, move it to active bucket
		//	g->buckets.iDelete(i);
		//	g->buckets.aAdd(i,i->my_bucket);
		//};
		tcap & e = v->excess;
		if(e<=0){
			if(e+f>0){//was inactive, became active
				g->flow -= e;
				if(reuse){
					if(g->buckets.top_active->d<v->d){
						g->buckets.top_active = v->my_bucket;
					};
					if(v->d<d_inf){
						g->buckets.activate(v);
						//v->current = v->arc_first;
						v->current = g->arc_first(v);
					};
				};
			}else{ // e+f<=0 still, remains inactive! (immediately put to the sink)
				g->flow += f;
			};
		}else{//was active, add more excess, keep bucket
		};
		e+=f;
	};
};

void PRD::d_in(int e,int new_d){
	//seed_hi_pr::node * i = g->arcs[e].head;
	hpr::arc_ptr a = g->arc_for_index(e);
	hpr::node * u = g->arc_tail(a);
	hpr::node * i = g->arc_head(u,a);
	assert(new_d >= i->d);
if(new_d==d_inf && i->d<d_inf && reuse){//remove this seed
		g->buckets.iDelete(i);
		g->elliminate(i);
	}else{//will adjust buckets by update_seeds
		i->d = new_d;
	};
//	i->d = new_d;
//	if(new_d==d_inf && i->d<d_inf){//remove this seed
//		g->buckets.iDelete(i);
//		g->elliminate(i);
//	}else{//will adjust buckets by update_seeds
//		i->d = new_d;
//	};
};

/*
inline void PRD::msg_in(int e, int d, tcap f){
	d_in(e,d);
	flow_in(e,f);
};

inline void PRD::msg_out(int e, int & d, tcap & f){
	f = g->arcs[e].rev->resCap;
	d = g->arcs[e].rev->head->d;
};
*/

inline mint2 PRD::rcap(int e){
	mint2 r;
	hpr::arc_ptr a = g->arc_for_index(e);
	//r[0] = g->arcs[2*e].resCap;
	//r[1] = g->arcs[2*e].rev->resCap;
	r[0] = a->resCap;
	r[1] = g->arc_rev(a)->resCap;
	return r;
};

PRD::~PRD(){
	if(g)delete g;
};

size_t PRD::load(const std::string & id){
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
	assert(g==0);//not constructed yet
	g = new hpr;
	E_out.resize(nE_out);
	global_index.resize(nV);
	g->allocate1(nV,nB,nE,nE_out,E_out.begin(),d_inf);
	//read node global index
	for(int v=0;v<nV;++v){
		read_bytes += fread(&global_index[v],1,sizeof(int),f);
		g->nodes[v].d = 1;
	};
	//read excess
	for(int i=0;i<nexcess;++i){//these shoud agregate correctly
		int v;
		int e;
		read_bytes += fread(&v,1,sizeof(v),f);
		read_bytes += fread(&e,1,sizeof(e),f);
		g->add_tweights(v,std::max(0,e),std::max(0,-e));
	};
	long pos = ftell(f);
	//read residual capacities
	for(int loop=0;loop<2;++loop){
		fseek(f,pos,SEEK_SET);
		g->reset_counters();
		for(int e=0;e<nE;++e){
			int u;
			int v;
			int cap1;
			int cap2;
			read_bytes += fread(&u,1,sizeof(u),f);
			read_bytes += fread(&v,1,sizeof(v),f);
			read_bytes += fread(&cap1,1,sizeof(cap1),f);
			read_bytes += fread(&cap2,1,sizeof(cap2),f);
			g->add_edge(u, v, cap1, cap2,loop);
		};
		if(loop==0){
			g->allocate2();
		};
	};
	fclose(f);
	reuse = false;
	return read_bytes;
};
/*
void PRD::save(const std::string & id){
	FILE * f = fopen(id.c_str(),"wb");
	//dynamic::fixed_array1<char> buff(1024*1024);//1Mb buffer
	//setbuf(f,buff.begin());
	//read sizes, these however are kept in memory
	fwrite(&nV,sizeof(nV),1,f);
	fwrite(&nB,sizeof(nB),1,f);
	fwrite(&nE,sizeof(nE),1,f);
	fwrite(&nE_out,sizeof(nE_out),1,f);
	fwrite(&nV,sizeof(nV),1,f);//nexcess
	//write excess
	for(int v=0;v<nV;++v){
		int e = g->get_trcap(v);
		fwrite(&v,sizeof(v),1,f);
		fwrite(&e,sizeof(e),1,f);
	};
	//write residual capacities
	for(int e=0;e<nE;++e){
		int u = int(g->arcs[2*e+1].head - g->nodes);
		int v = int(g->arcs[2*e].head - g->nodes);
		int cap1 = g->arcs[2*e].r_cap;
		int cap2 = g->arcs[2*e+1].r_cap;
		fwrite(&u,sizeof(u),1,f);
		fwrite(&v,sizeof(v),1,f);
		fwrite(&cap1,sizeof(cap1),1,f);
		fwrite(&cap2,sizeof(cap2),1,f);
	};
	//todo: this should be ensured elsewhere
	//make sure (B,R) edges are 0
	for(int e=nE-nE_out;e<nE;++e){
		g->arcs[2*e+1].r_cap = 0;
	};
	fclose(f);
};
*/

size_t PRD::unload(const std::string & id){//save and free memory
	//save(id);
	//deallocate graph and aux data
	global_index.destroy();
	delete g;
	g = 0;
	return 0;
};


long long PRD::cut_cost(){
	return g->cut_cost();
};

long long PRD::size_required(int nV, int nB, int nE, int nE_out,int d_inf){
	long long r = 0;
	r+=sizeof(PRD);
	r+=hpr::size_required(nV,nB,nE,nE_out,d_inf);
	r+=sizeof(int)*nV;//global_index
	return r;
};

int PRD::g_index(int v){
	return global_index[v];
};

bool PRD::is_weak_source(int v){
	return g->is_weak_source(&g->nodes[v]);
};

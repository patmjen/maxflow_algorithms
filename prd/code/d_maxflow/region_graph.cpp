#include "region_graph.h"

region_graph::region::region(){
	nV = 0;
	nB = 0;
	nE = 0;
	nE_out = 0;
	f = 0;
	last_u = -1;
	nexcess = 0;
	//
	file_type = 0;
	page = 0;
	loaded = 0;
	constructed = 0;
	preprocessed = 0;
	flow = 0;
	has_active = true;
	dead = 0;
	max_d = 0;
	g_first = 0;
};

region_graph::arc::arc(){
	head_region = -1;
	rev = 0;
	local_index = -1;
	r_cap = 0;
	next = 0;
	d = 0;
	f = 0;
};

region_graph::d_group::d_group(){
	g_next = 0;
	arc_first = 0;
	relabel_d = 0;
};

void region_graph::init_region_arcs(){
	nRE=0;
	for(int r=0;r<nR;++r){
		nodes[r].nRE = 0;
		nodes[r].last_r = -1;
	};
	for(int loop=0;loop<2;++loop){
		for(int r=0;r<nR;++r){
			for(arc * a = nodes[r].out.begin(); a < nodes[r].out.end(); ++a){
				int q = a->head_region;
				//region_arc (r,q)
				//check if there was no region_arc r->q yet
				if(nodes[q].last_r!=r){
					if(loop==1  && r<q){
						region_arc * A = nodes[r].Out.push_back();
						region_arc * B = nodes[q].Out.push_back();
						A->head = &nodes[q];
						B->head = &nodes[r];
						A->rev = B;
						B->rev = A;
					};
					++nodes[q].nRE;
					nodes[q].last_r = r;
				};
			};
		};
		if(loop==0){
			for(int r=0;r<nR;++r){
				nRE+=nodes[r].nRE;
				nodes[r].Out.reserve(nodes[r].nRE);
				nodes[r].nRE = 0;
				nodes[r].last_r = -1;
			};
		};
	};
};

void region_graph::save(const char *filename){
	FILE * f = fopen(filename,"wb+");
	setvbuf(f,0,_IOFBF,1024*1024*2);
	fwrite(&nV,sizeof(nV),1,f);
	fwrite(&nB,sizeof(nB),1,f);
	fwrite(&nE,sizeof(nE),1,f);
	fwrite(&nBE,sizeof(nBE),1,f);
	fwrite(&nR,sizeof(nR),1,f);
	//fwrite(&nRE,sizeof(nRE),1,f);
	int sz = nodes.size();
	fwrite(&sz,sizeof(sz),1,f);
	for(int i=0;i<sz;++i){
		fwrite(&nodes[i].nV,sizeof(nodes[i].nV),1,f);
		fwrite(&nodes[i].nB,sizeof(nodes[i].nB),1,f);
		fwrite(&nodes[i].nE,sizeof(nodes[i].nE),1,f);
		fwrite(&nodes[i].nE,sizeof(nodes[i].nE_out),1,f);
		int sz1 = nodes[i].id.length();
		fwrite(&sz1,sizeof(sz1),1,f);
		fwrite(nodes[i].id.c_str(),1,sz1,f);
		sz1 = nodes[i].out.size();
		fwrite(&sz1,sizeof(sz1),1,f);
		for(int j=0;j<sz1;++j){
			arc & e = nodes[i].out[j];
			fwrite(&e.head_region,sizeof(e.head_region),1,f);
			int a = e.rev-nodes[e.head_region].out.begin();
			fwrite(&a,sizeof(a),1,f);
			fwrite(&e.local_index,sizeof(e.local_index),1,f);
			fwrite(&e.r_cap,sizeof(e.r_cap),1,f);
		};
	};
	fclose(f);
};

bool region_graph::load(const char *filename){
	FILE * f = fopen(filename,"rb");
	if(!f)return false;
	debug::stream<<"reading split "<<filename<<"\n";
	setvbuf(f,0,_IOFBF,1024*1024*2);
	fread(&nV,sizeof(nV),1,f);
	fread(&nB,sizeof(nB),1,f);
	fread(&nE,sizeof(nE),1,f);
	fread(&nBE,sizeof(nBE),1,f);
	fread(&nR,sizeof(nR),1,f);
	//debug::stream<<"Regions: "<<nR<<"\n";
	//fread(&nRE,sizeof(nRE),1,f);
	int sz;
	fread(&sz,sizeof(sz),1,f);
	nodes.resize(sz);
	for(int i=0;i<sz;++i){
		fread(&nodes[i].nV,sizeof(nodes[i].nV),1,f);
		fread(&nodes[i].nB,sizeof(nodes[i].nB),1,f);
		fread(&nodes[i].nE,sizeof(nodes[i].nE),1,f);
		fread(&nodes[i].nE_out,sizeof(nodes[i].nE_out),1,f);
		int sz1;
		fread(&sz1,sizeof(sz1),1,f);
		nodes[i].id.resize(sz1);
		fread(&nodes[i].id[0],1,sz1,f);
		sz1;
		fread(&sz1,sizeof(sz1),1,f);
		nodes[i].out.resize(sz1);
		for(int j=0;j<sz1;++j){
			arc & e = nodes[i].out[j];
			fread(&e.head_region,sizeof(e.head_region),1,f);
			int a;
			fread(&a,sizeof(a),1,f);
			(int&)e.rev = a;//a+nodes[e.head].out.begin();
			fread(&e.local_index,sizeof(e.local_index),1,f);
			fread(&e.r_cap,sizeof(e.r_cap),1,f);
		};
	};
	fclose(f);
	for(int i=0;i<nodes.size();++i){
		for(int j=0;j<nodes[i].out.size();++j){
			arc & e = nodes[i].out[j];
			e.rev = (int&)e.rev +nodes[e.head_region].out.begin();
		};
	};
	init_region_arcs();
	return true;
};
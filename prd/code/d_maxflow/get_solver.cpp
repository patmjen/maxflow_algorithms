#include "get_solver.h"

//debug
#include "debug/logs.h"
#include "debug/except.h"
#include "debug/performance.h"

//solvers:
#include "maxflow_BK.h"
#include "maxflow_GT.h"
#include "maxflow_HPR.h"

#include "stream_ARD1.h"
#include "stream_PRD.h"
#include "parallel_ARD1.h"
#include "parallel_PRD.h"
#include "seed_BK1.h"

#ifdef solver_DD
#include "mtmaxflow/maxflow_dd.h"
#endif

void get_solver(const char * _solver, region_graph * G, region_splitter * splitter, maxflow_solver *& solver, dimacs_parser_callback *& constructor, intf & slice, bool unload, const char * tmp){
	solver = 0;
	const char * param = "";

	if(strcmp(_solver,"BK")==0){//BK
		solver = new maxflow_BK();
		constructor = (maxflow_BK*)solver;
	};
	if(strcmp(_solver,"GT05")==0){//GT05
		solver = new maxflow_GT;
		((maxflow_GT*)solver)->g.globUpdtFreq = 0.5;
		constructor = (maxflow_GT*)solver;
	};
	if(strcmp(_solver,"GT0")==0 || strcmp(_solver,"GT")==0){//GT0
		solver = new maxflow_GT;
		((maxflow_GT*)solver)->g.globUpdtFreq = 0.0;
		constructor = (maxflow_GT*)solver;
	};
	if(strcmp(_solver,"HPR")==0){
		solver = new maxflow_HPR;
		constructor = (maxflow_HPR*)solver;
	};
	if(strcmp(_solver,"seed_BK")==0){//seed_BK
		solver = new seed_BK();
		constructor = (seed_BK*)solver;
	};
	if(strstr(_solver,"S-ARD")){
	    param = strpbrk(_solver+2,"x-");
		constructor = splitter;
		stream_ARD1 * s = new stream_ARD1;
		s->construct(G,unload);
		s->set_tmp(tmp);
		solver = s;
	};
	if(strstr(_solver,"P-ARD")){
	    param = strpbrk(_solver+2,"x-");
		constructor = splitter;
		parallel_ARD1 * s = new parallel_ARD1;
		s->construct(G);
		solver = s;
	};
	if(strstr(_solver,"S-PRD")){
	    param = strpbrk(_solver+2,"x-");
		constructor = splitter;
		stream_PRD * s = new stream_PRD;
		s->construct(G,unload);
		s->set_tmp(tmp);
		solver = s;
	};
	if(strstr(_solver,"P-PRD")){
	    param = strpbrk(_solver+2,"x-");
		constructor = splitter;
		parallel_PRD * s = new parallel_PRD;
		s->construct(G);
		solver = s;
	};

	//params
	//check if there is a split specified
	if(param && param[0]=='x'){
		int l;
		for (l=0; param && param[0]=='x'; ++l){
			slice[l] = atoi(param+1);
			param = strpbrk(param+1,"x-");
		};
		slice.resize(l);
	};
	while(param && param[0]=='-'){
	    switch(param[1]){
	        case 't': solver->params.n_threads = atoi(param+2);
	        break;
	    };
	    param = strpbrk(param+1,"-");
	};

#ifdef solver_DD
	if(strstr(_solver,"P-DDx")){
		const char * param = _solver+5;
		solver = new maxflow_dd;
		int nR = atoi(param);
		((maxflow_dd*)solver)->nR = nR;
		constructor = (maxflow_dd*)solver;
	};
#endif

	if(!solver){
		debug::stream<<"Unknown solver: "<<_solver<<"\n";
		exit(1);
	};
};

#include "parallel_PRD.h"

#include "parallel_discharge.cpp"
template class parallel_discharge<PRD>;

void parallel_PRD::construct(region_graph * G){
	parent::construct(G,false);
};

parallel_PRD::parallel_PRD(){
	info.name = "P-PRD";
};

void parallel_PRD::hist_dec(int sewwp, int r){
//#pragma omp critical
	{
		//subtract r from global label hist
		for(int v=0;v<R[r]->nV;++v){//go over inner vertices
			int d = R[r]->g->nodes[v].d;
			if(d<d_inf){
#pragma omp atomic
				--d_hist[d];
			};
		};
	};
};

void parallel_PRD::hist_inc(int sweep,int r){
//#pragma omp critical
	{
		//update global hist for new labels of r
		for(int v=0;v<R[r]->nV;++v){//go over inner vertices
			int d = R[r]->g->nodes[v].d;
			if(d<d_inf){
#pragma omp atomic
				++d_hist[d];
			};
		};
	};
};

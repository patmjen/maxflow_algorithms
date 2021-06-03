#include "maxflow_solver.h"

maxflow_solver::maxflow_solver(){
	info.nV = 0;
	info.nE = 0;
	info.nBE = 0;
	info.name = "solver";
	info.flow = -1;
	info.sweeps=0;
	info.construct_t.start();
	info.construct_t.stop();
	info.solve_t.start();
	info.solve_t.stop();
	info.augment_t.start();
	info.augment_t.stop();
	info.msg_t.start();
	info.msg_t.stop();
	info.relabel_t.start();
	info.relabel_t.stop();
	info.gap_t.start();
	info.gap_t.stop();
	info.diskr_t.start();
	info.diskr_t.stop();
	info.diskw_t.start();
	info.diskw_t.stop();
	info.mem_shared = 0;
	info.mem_region = 0;
	info.diskr_b = 0;
	info.diskw_b = 0;
	params.n_threads = 0;
};

void maxflow_solver::print_info(){
	//txt::FileStream f((std::string(file)+"."+info.name+".sol").c_str(),append);
	//txt::EchoStream g(&f,&debug::stream);
	txt::TextStream & g = debug::stream;
	g<<"solver: "<<info.name<<"\n";
	g<<"vert: "<<info.nV<<"\n";
	g<<"edges: "<<info.nE<<"\n";
	g<<"bnd_edges: "<<info.nBE<<"\n";
	g<<"mem_shared: "<<txt::String::Format("%4.1f",info.mem_shared/1024.0/1024)<<"Mb\n";
	g<<"mem_region: "<<txt::String::Format("%4.1f",info.mem_region/1024.0/1024)<<"Mb\n";
	g<<"flow: "<<info.flow<<"\n";
	g<<"sweeps: "<<info.sweeps<<"\n";
	g<<"construct: "<<info.construct_t.time()<<"\n";
	g<<"    solve: "<<info.solve_t.time()<<"\n";
	g<<"      msg: "<<info.msg_t.time()<<"\n";
	g<<"  augment: "<<info.augment_t.time()<<"\n";
	g<<"  relabel: "<<info.relabel_t.time()<<"\n";
	g<<"      gap: "<<info.gap_t.time()<<"\n";
	g<<"disk read: "<<info.diskr_t.time()<<"s / "<<txt::String::Format("%4.1f",info.diskr_b/1024.0/1024)<<"Mb\n";
	g<<"disk writ: "<<info.diskw_t.time()<<"s / "<<txt::String::Format("%4.1f",info.diskw_b/1024.0/1024)<<"Mb\n";
	g<<"_____________________________________________\n";
};

#include "d_maxflow/get_solver.h"
#include "files/xfs.h"

using namespace debug;
using namespace dynamic;


debug::PerformanceCounter c1;

void solve(const char * _solver, const char * file, intf & slice, bool unload, const char * tmp=""){
	try{
		const char * splitter_file = file;
		maxflow_solver * solver;
		dimacs_parser_callback * constructor;
		region_graph G;
		G.pth = std::string(splitter_file)+"_reg/";
		region_splitter splitter(splitter_file,&G,slice);
		get_solver(_solver,&G,&splitter,solver,constructor,slice,unload,tmp);
		splitter.slice = slice;

		if(constructor != &splitter){//try to load if was already parsed, otherwise parse anew
			PerformanceCounter c0;
			c0.start();
			dimacs_parser(file,*constructor,2);
			c0.stop();
			debug::stream<<" parser: "<<c0.time()<<"\n";
		};
		if(constructor == &splitter && !G.load((G.pth + "rgraph").c_str())){
			PerformanceCounter c0;
			c0.start();
			{
				region_graph G0;
				G0.pth = G.pth;
				splitter.G = &G0;
				dimacs_parser(file,splitter,2);
				c0.stop();
				debug::stream<<" splitter: "<<c0.time()<<"\n";
				G0.save((G.pth + "rgraph").c_str());
			};
			G.load((G.pth + "rgraph").c_str());
		};
		long long F  = solver->maxflow();
		solver->print_info();
		txt::StringStream log_name;
		log_name <<std::string(file)<<"."<<_solver;
		log_name<<".cut";
		solver->save_cut(log_name);
		//cleanup
		delete solver;

	}catch(const std::exception & e){
		debug::stream<<"Exception: "<<e.what()<<"\n";
		exit(1);
	}catch(...){
		debug::stream<<"ERROR\n";
		exit(1);
	};
}

void try_solve(const char * solver, const char * file, intf & slice, bool unload, const char * tmp){
	//__try{
	solve(solver,file,slice,unload,tmp);
	//}__except(EXCEPTION_EXECUTE_HANDLER){
	//	debug::stream<<"ERROR\n";
	//	exit(1);
	//};
};

int main(int argc, char *argv[]){
	intf slice = intf(4,4,4,4,1);
	const char * solver;
	const char * file;
	const char * options;
	std::string root;
	dynamic::fixed_array1<std::string> solvers;
	solvers.reserve(20);
	root = xfs::getPath(argv[0]);
	printf("my path: %s\n",root.c_str());
	if(argc<3){
		file =  "../../test/d_maxflow/BVZ-tsukuba0.max";
		//
		printf("Usage: solve_dimacs <solver>[split][options] <problem>\n");
		printf("Arguments:\n");
		printf("<solver>: BK, GT, GT05, seed_BK, HPR, S-ARD, P-ARD, S-PRD, P-PRD, P-DDx2 P-DDx4\n");
		printf("<problem>: text file in DIMACS format (must be s=1, t=2)\n");
		printf("[split]: x<n1>x<n2>x<n3> -- splits problem along dimensions into <n1>*<n2>*<n3> parts.\n");
		printf("[options]: -t<n> : set number of threads to <n> for parallel algorithms\n");
		printf("/nNo arguments provided, running test: %s\n",file);
		solvers.push_back("BK");
		solvers.push_back("GT");
		solvers.push_back("GT05");
		solvers.push_back("seed_BK");
		solvers.push_back("HPR");
		solvers.push_back("S-ARD1");
		solvers.push_back("P-ARD1");
		solvers.push_back("S-PRD");
		solvers.push_back("P-PRD");
		//solvers.push_back("P-DDx2");
		//solvers.push_back("P-DDx4");
	}else{
		solver = argv[1];
		file = argv[2];
		solvers.push_back(solver);
	};

	for(int i=0; i< solvers.size();++i){
		solver = solvers[i].c_str();
		txt::StringStream log_name;
		log_name <<std::string(file)<<"."<<solver;
		log_name<<".sol";
		txt::FileStream f(log_name.c_str());
		txt::EchoStream g(&f,&stdoutStream::the_stream);
		debug::stream.attach(&g);
		try_solve(solver,file,slice,true,root.c_str());
		debug::stream<<"All Ok\n";
	};
	return 0;
};

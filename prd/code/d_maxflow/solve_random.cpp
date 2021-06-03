#include "d_maxflow/get_solver.h"
#include "files/xfs.h"

#include "dynamic/array_allocator.h"

using namespace debug;
using namespace dynamic;

debug::PerformanceCounter c1;

long long solve(const char * _solver, int rand_inst, intf & slice, int * sz,int conn, int str, int u_str = 500, const char * tmp="", int * C_ref = 0, bool is_ref = 0){
	// C_ref -- reference cut for verification
	// is_ref -- true if it is the reference solution
	int n1 = sz[0];
	int n2 = sz[1];
	int nV = n1*n2;
	int shift1[] = {0,1,1,2,1,3,2,3,0,2,2,3,3,4};
	int shift2[] = {1,0,2,1,3,1,3,2,2,0,2,3,4,2};
	const char * splitter_file = "rnd_inst";
	debug::PerformanceCounter c0;
	maxflow_solver * solver;
	dimacs_parser_callback * constructor;
	region_graph G;
	G.pth = std::string(splitter_file)+"_reg";
	region_splitter splitter(splitter_file,&G,slice);
	get_solver(_solver,&G,&splitter,solver,constructor,slice,false);
	//construct
	c0.start();
	int S = 0;
	int T = 1;
	constructor->allocate1(nV+2,0,S,T,2,sz);
	//add some edges
	for(int loop=0;loop<2;++loop){
		srand(rand_inst);
		int e = 0;
		for(int x1=0;x1<n1;++x1){
			for(int x2=0;x2<n2;++x2){
				int u = x1+x2*n1+2;
				//add excess
				int ex = math::rand32()%(2*u_str)-u_str;
				constructor->read_arc(loop,S,u,std::max(0,ex),0);
				constructor->read_arc(loop,u,T,std::max(0,-ex),0);
				for(int s=0;s<conn;++s){
					//add edges
					int y1 = x1+shift1[s];
					int y2 = x2+shift2[s];
					if(y1<0 || y1>=n1)continue;
					if(y2<0 || y2>=n2)continue;
					int v = y1+y2*n1+2;
					if(v<u)continue;//already added edge (u,v), don't add the reverse
					//int cap1 = math::rand32()%50;
					//int cap2 = math::rand32()%50;
					//homogenous pairwise
					int cap = str/conn;
					//constructor->read_arc(loop,u,v,cap1,cap2);
					constructor->read_arc(loop,u,v,cap,cap);
				};
			};
		};
		constructor->allocate2(loop);
	};
	constructor->allocate3();
	c0.pause();
	long long F  = solver->maxflow();
	solver->print_info();
	{
		long long C = solver->cut_cost();
		if(F!=C){
			throw debug_exception("cut!=flow");
		};
	}
	intf C(nV+2);
	solver->get_cut(C.begin());
	if(C_ref){
		for(int i=0;i<nV+2;++i){
			if(is_ref){
				C_ref[i]=C[i];
			}else{
				if(C_ref[i]!=C[i]){
					throw debug_exception("wrong cut");
				};
			};
		};
	};
	//cleanup
	delete solver;
	return F;
};

void rand_test(dynamic::fixed_array1<std::string> & solvers, int niter, int start_it, int N, int conn, int sl, int str, int u_str = 500,const char * tmp=""){
	intf slice = intf(4,sl,sl,1,1);
	mint4 sz(N,N,1,1);
	int nV = sz.prod();
	intf C_ref(nV+2);
	long long F0;
	for(int it=start_it;it<niter;++it){
		for(int k=0;k<solvers.size();++k){
			const char * solver = solvers[k].c_str();
			char ss[1024];
			char pth[1024];
			sprintf(pth,"%s../../data/d_maxflow/rnd_s%i/",tmp,str);
			//_mkdir(pth);
			sprintf(ss,"%srnd_sz%4.4i_%3.3i_conn%2.2i_inst%4.4i_r%2.2i_str%3.3i",pth,sz[0],sz[1],conn,it,slice[0]*slice[1],str);
			std::string log_name = std::string(ss)+"."+solver+".sol";
			/*
			txt::FileStream f(log_name.c_str());
			txt::EchoStream g(&f,&stdoutStream::the_stream);
			debug::stream.attach(&g);
			*/
			long long F;
			try{
				bool is_ref = (k==0);
				F = solve(solver,it,slice,sz.begin(),conn,str,u_str,tmp,C_ref.begin(),is_ref);
			}catch(const std::exception & e){
				debug::stream<<"Exception: "<<e.what()<<"\n";
				exit(1);
			}catch(...){
				debug::stream<<"ERROR\n";
				exit(1);
			};
			if(k==0){
				F0=F;
			}else if(F0!=F){
				debug::errstream<<"Solver fault: "<<solver<<"\ninstance: "<<log_name<<"\n";
				exit(1);
			};
		};
	};
};

void rand_tests(){
	int N[] = {20,100,200,400,600,800,1000,1200,1600,1800,2000};
	int conn[] = {2, 4, 6, 8, 10};
	int str [] = {20, 30, 35, 40, 45, 50, 60, 80, 100, 120, 140, 160, 180, 200};
	dynamic::fixed_array1<std::string> solvers;
	solvers.reserve(6);
	solvers.push_back("BK");
	solvers.push_back("HPR");
	solvers.push_back("GT0");
	solvers.push_back("GT05");
	solvers.push_back("S-ARD");
	solvers.push_back("S-PRD");

	/*
	//strength
	for(int i=0;i<14;++i){
	rand_test(100,1000,4,2,str[i],solvers);
	};
	*/

	//connectivity
	{
		int conn[] = {2, 4, 6, 8, 10, 12, 16, 18};
		for(int k=0;k<5;++k){
			rand_test(solvers,100,0,1000,conn[k],2,600);
		};
	};

	//size
	for(int i=0;i<11;++i){
		rand_test(solvers,100,0,N[i],4,2,600);
	};

	//#regions
	for(int sl=2;sl<16;++sl){
		rand_test(solvers,100,0,1000,4,sl,600);
	};
};


int main(int argc, char *argv[]){
	dynamic::array_allocator<int> al;
	al.allocate(1);
	{
		std::string root = xfs::getPath(argv[0]);
		dynamic::fixed_array1<std::string> solvers;
		solvers.reserve(20);
		solvers.push_back("BK");
		solvers.push_back("GT");
		solvers.push_back("GT05");
		solvers.push_back("seed_BK");
		solvers.push_back("HPR");
		solvers.push_back("S-ARD");
		solvers.push_back("P-ARD");
		solvers.push_back("S-PRD");
		solvers.push_back("P-PRD");

		//solvers.push_back("P-DDx2");
		//solvers.push_back("P-DDx4");
		//
		//solve 1000 random problems
		//starting from problem 0 (the problem is uniquely determined by its number)
		//rand_test(solvers,/*num_problems*/1000,/*first_problem*/0,/*size*/100,/*conn*/8,/*slices*/4,/*strength*/600,/*unary strength*/500,root.c_str());
		rand_test(solvers,/*num_problems*/1000,/*first_problem*/0,/*size*/100,/*conn*/2,/*slices*/4,/*strength*/600,/*unary strength*/500,root.c_str());
		//
	};
	return 0;
};

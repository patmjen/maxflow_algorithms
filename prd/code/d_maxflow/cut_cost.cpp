#include <stdio.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <stdlib.h>

void read_cut(const char * name, int *& S, long long & F0, int & n){
	FILE * f = fopen(name,"rt");
	if(!f){
		printf("%s\n",(std::string() + "cant read file: " + name).c_str());
		exit(1);
	};
	char s[1024];
	int m;
	int line = 0;
	while(!feof(f)){
		++line;
		char c = fgetc(f);
		switch(c){
			case 'p'://problem specification
				fscanf(f," %*s %i %i ",&n,&m);// number of nodes and arcs
				S = new int[n];
				for(int i=0;i<n;++i){
					S[i] = -1;
				};
				break;
			case 'c':
				fgets(s,1024,f); //read comment line until the end
				break;
			case 'f':{ //claimed flow
				fgets(s,1024,f);
				std::stringstream ss(s);
				ss>>F0;
				//fscanf(f," %llu ",F0);
				break;
					 };
			case 'n':{ //node label
				int i,l;
				fgets(s,1024,f);
				std::stringstream ss(s);
				ss>>i>>l;
				if(i<1 || i>n){
					printf("Bad node index %i on line %i \n",i,line);
					exit(1);
				};
				if(S[i-1]!=-1){
					printf("Node index %i repeats on line %i \n",i,line);
					exit(1);
				};
				S[i-1] = l;
				break;
					 };
		};
	};
	for(int i=0;i<n;++i){
		if(S[i] == -1){
			printf("not a complete cut\n");
			exit(1);
		};
	};
	fclose(f);
};

long long cut_cost(const char * problem, int * S, int n){
	FILE * f = fopen(problem,"rt");
	if(!f){
		printf("%s\n",(std::string() + "cant read file: " + problem).c_str());
		exit(1);
	};
	char s[1024];
	int m;
	long long F = 0;
	int line = 0;
	//read header
	while(!feof(f)){
		++line;
		char c = fgetc(f);
		switch(c){
			case 'p'://problem specification
				fgets(s,1024,f);
				//fscanf(f," %*s %i %i ",&n,&m);// number of nodes and arcs
				break;
			case 'c'://comment line
				fgets(s,1024,f);
				break;
			case 'n':
				fgets(s,1024,f);
				break;
			case 'a':
				int u,v; int cap;
				fscanf(f," %i %i %i ",&u,&v,&cap);
				if(u<1 || u>n){
					printf("Bad node index %i on line %i \n",u,line);
					exit(1);
				};
				if(v<1 || v>n){
					printf("Bad node index %i on line %i \n",v,line);
					exit(1);
				};
				if(S[u-1]==1 && S[v-1]==0){
					F+=cap;
				};
				break;
		};
	};
	fclose(f);
	return F;
};

int main(int argc, char *argv[]){
	if(argc!=3){
		printf("Usage: cut_cost maxflow_dimacs_definition.max cut_file.cut");
		exit(1);
	};
	const char * problem = argv[1];
	const char * cut = argv[2];
	int * S=0; // source set indicator
	long long F0;
	int n;
	read_cut(cut,S,F0,n);
	long long F = cut_cost(problem,S,n);
	if(F==F0){
		printf("Cut cost confirmed, %lli\n",F);
	}else{
		printf("Error: claimed flow, %lli, differs from the cut cost, %lli\n",F0, F);
	};
	if(S)delete S;
};

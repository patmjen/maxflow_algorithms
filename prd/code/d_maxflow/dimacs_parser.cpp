#include "dimacs_parser.h"
#include "debug/except.h"
#include <stdio.h>
#include "dynamic/dynamic.h"
#include <string.h>

class buff_read{
public:
	static const int max_size = 1024*1024*32;//8Mb
	dynamic::fixed_array1<char> buff;
	//char * buff;
	int sz;
	int pos;
	FILE *f;
private:
	void readahead(){
		sz = fread(buff.begin(),1,buff.size(),f);
		pos = 0;
	};
	__forceinline bool ready(){
		if(pos<sz)return true;
		if(feof(f))return false;
		readahead();
		return true;
	};
public:
	bool eof()const{return pos==sz && feof(f);};
	//
	buff_read(FILE * _f):f(_f){
		pos = 0;
		sz = 0;
		//buff = new char[max_size];
		buff.resize(max_size);
	};
	~buff_read(){
		//delete[] buff;
	};
	//
	__forceinline char getc(){
		ready();
		return buff[pos++];
	};
	//
	__forceinline void skip_line(){
		while(ready()){
			while(pos<sz){
				if(buff[pos]=='\n')return;
				++pos;
			};
		};
	};
	//
	__forceinline int read_line(char * s,int max_count){
		int k = 0;
		while(ready() && k<max_count-1){
			while(pos<sz && k<max_count-1){
				s[k] = buff[pos];
				if(buff[pos]=='\n'){
					s[k+1] = 0;
					return k;
				};
				++pos;
				++k;
			};
		};
		s[k+1] = 0;
		return k;
	};
};

__forceinline char * eatspace(char * ps){
	do{
		char c = *ps;
		if(c==' ' || c== '\t' || c== '\n' || c=='\r'){
			++ps;
			continue;
		};
		return ps;
	}while(1);
};
__forceinline int strtol10(char *& ps){
	int r = 0;
	bool isnumber = false;
	ps = eatspace(ps);
	do{
		char c = *ps;
		if(c<'0' || c>'9'){
			if(!isnumber)throw debug_exception("number expected");
			return r;
		};
		isnumber = true;
		r = r*10+(c-'0');
		++ps;
	}while(1);
};

void dimacs_parser(const char * filename, dimacs_parser_callback & A, int loops){
	FILE * f = fopen(filename,"rb");
	if(!f){
		//char s[1024];
		//sprintf(s, "cant read file: %s",filename);
		throw debug_exception(std::string() + "cant read file: " + filename);
	};
	char s[1024];
	char s1[1024];
	//int d; //d is number of dimensions
	//int sz[4]; // grid sizes
	//int s,t;//source and sink nodes
	long int pos;// remember position where arcs start in the stream
	char c = fgetc(f);
	int n,m;
	int S,T;
	int sz[4];//grid size
	int d=1;//grid dimensions, 1 by default
	struct tlast_arc{
		int u;
		int v;
		int cap;
		bool parsed;
		tlast_arc():parsed(true){};
	}last_arc;
	//
	//read header
	for(;!feof(f) && c!='a'; c = fgetc(f)){
		switch(c){
			case 'p'://problem specification
				fscanf(f," %*s %i %i ",&n,&m);// number of nodes and arcs
				break;
			case 'c'://comment line, look for regulargreed, complexgrid
				{fgets(s,1024,f); //read comment line until the end
				sscanf(s," %s ", s1);
				if(strcmp(s1,"regulargrid")==0 || strcmp(s1,"complexgrid")==0){
					d = sscanf(s," %*s %i %i %i %i",sz,sz+1,sz+2,sz+3);
				};
				if(strcmp(s1,"Size:")==0){
					//d = sscanf(s," %*s %i%*c%i%*c%i%*c%i",sz,sz+1,sz+2,sz+3);
					//d = sscanf(s," %*s %i%*c%i%*c%i%*c%i",sz+3,sz+2,sz+1,sz);
					//slice[2] = 1;
					//std::swap(slice[0],slice[3]);
					//std::swap(slice[1],slice[2]);
					d = 1;
				};
				break;
				};
			case 'n'://source or sink nodes
				int v;
				fscanf(f," %i %c ",&v,s);
				--v; //zero-based index
				if(s[0]=='s'){
					S = v;
				}else{
					T = v;
				};
				break;
				//case 'a'://arc node
				//	pos = ftell(f)-1;
				//break;
		};
	};
	pos = ftell(f)-1;
	//R.nR = get_nR();
	//
	A.allocate1(n,m,S,T,d,sz);
	// now a double loop over edges - count and read
	for(int loop=0;loop<loops;++loop){
		fseek(f,pos,SEEK_SET);//rewind to where arcs begin
		buff_read ff(f);
		//
		while(!ff.eof()){
			//while(!feof(f)){
			c = ff.getc();
			//c = fgetc(f);
			switch(c){
			case 'c':
				ff.skip_line();
				//fgets(s,1024,f);
				break;
			case 'a':
				int u,v; int cap;
				//fscanf(f," %i %i %i ",&u,&v,&cap);
				ff.read_line(s,1024);
				//fgets(s,1024,f);
				char *ps = s;
				//break;
				//u = strtol(ps,&ps,10);
				//v = strtol(ps,&ps,10);
				//cap = strtol(ps,&ps,10);
				u = strtol10(ps);
				v = strtol10(ps);
				cap = strtol10(ps);
				if(cap==0)continue;//skip zero arcs
				//sscanf(s," %i %i %i ",&u,&v,&cap);
				--u;//to zero-based index
				--v;
				//break;
				//if u is source or v is sink, assign this capacity to the excess
				//A.read_arc(loop,u,v,cap,0);//next arc was a reverse one
				//continue;
				if(u==S || v == T){
					A.read_arc(loop,u,v,cap,0);
				}else{
					if(!last_arc.parsed){
						if(last_arc.u==v && last_arc.v==u){
							A.read_arc(loop,u,v,cap,last_arc.cap);//next arc was a reverse one
							last_arc.parsed = true;
						}else{
							A.read_arc(loop,last_arc.u,last_arc.v,last_arc.cap,0);//parse as unpaired
							last_arc.u = u;
							last_arc.v = v;
							last_arc.cap = cap;
							last_arc.parsed = false;
						};
					}else{
						last_arc.u = u;
						last_arc.v = v;
						last_arc.cap = cap;
						last_arc.parsed = false;
					};
				};
				break;//case
			};
		};
		if(!last_arc.parsed){
			A.read_arc(loop,last_arc.u,last_arc.v,last_arc.cap,0);//parse as unpaired
			last_arc.parsed = true;
		};
		A.allocate2(loop);
	};
	A.allocate3();
	fclose(f);
};
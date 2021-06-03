#ifndef region_splitter_h
#define region_splitter_h

#include "dynamic/dynamic.h"
#include "exttype/fixed_vect.h"
#include <map>

#include "region_graph.h"
#include "dimacs_parser.h"

using namespace exttype;

class region_splitter: public dimacs_parser_callback{
public:
	class node;
	/*
	class bnd_arc{
	public:
		node * head;
		bnd_arc * next;
		bnd_arc * rev;
		bnd_arc():next(0){};
		int cap;
		int local_index;
	};
	*/
	
	class node{
	public:
		int local_index;
		region_graph::arc * first_bnd;
		node():first_bnd(0){};
	};

public:
	int d; //is number of dimensions
	//int sz[4]; // grid sizes
	exttype::intf sz;
	//int S,T; //source and sink nodes are in G
	//int slice[4];
	exttype::intf slice; //how  many pieces to slice each dim?
	dynamic::fixed_array1<node> nodes;
	//intf local_index;
	//dynamic::fixed_array1<bnd_arc> bnd;
	region_graph * G;
	std::string name;
private:
	int allocated;
	int local_node(int u);
	void reset_counters();
	void arc_loop1(int u,int v);
	void arc_loop2(int u,int v,int cap1, int cap2);
	void write_arc(int r,int lu,int lv,int cap1, int cap2);
public:
	region_splitter(const std::string & filename, region_graph * _G, const exttype::intf & sslice);
	int region(int v);//!< which region is v
	int get_nR();
public: //dimacs_parser_callback
	virtual void allocate1(int n ,int m, int S, int T,int d,int * sz)override;
	virtual void allocate2(int loop)override;
	virtual void read_arc(int loop,int u,int v,int cap1, int cap2)override;
	virtual void allocate3()override;
};
//
#endif
#ifndef region_discharge_h
#define region_discharge_h

#include "dynamic/dynamic.h"
#include "dynamic/num_array.h"
#include "exttype/exttype.h"

using namespace exttype;

class region_discharge{
public:
	typedef int tcap;
public://input
	int nV;//!< number of inner vertices
	int nB;//!< number of boundary vertices
	int nE, nE_out;
	int d_inf;
public://output
	intf E_out;//!< index of boundary edges
	long long F;//!< flow value
	long long dead;
public:
	bool is_boundary(int v)const;
	int B_vertex(int i)const;
public:
	region_discharge();
	~region_discharge(){};
public:
};

#endif
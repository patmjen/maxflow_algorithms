#include "region_discharge.h"

bool region_discharge::is_boundary(int v)const{
	return v<nV;
};
int region_discharge::B_vertex(int i)const{
	return i+nV;
};
region_discharge::region_discharge(){
	F = 0;
	dead = 0;
};
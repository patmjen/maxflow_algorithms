#ifndef math_h
#define math_h
#include <math.h>
#include <stdlib.h>

extern const double eps0;
extern const double pi;
class math{
public:
	static double sqrt(double x){return ::sqrt(x);};
	template<typename T> static T sqr(const T & x){return x*x;};
	template<typename T> static int sgn(const T & x){return x>=0 ? 1 : -1; };
	template<typename T> static T abs(const T & x){ return x>=0?x:-x; };
	static int rand32(){return (::rand()<<16)+::rand();};
	static double rand(){return rand32()/(double(RAND_MAX)*(1<<16));};
};

int epseq(double x, double y);
#endif

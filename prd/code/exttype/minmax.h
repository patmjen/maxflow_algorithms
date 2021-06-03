#ifndef minmax_h
#define minmax_h

#define NOMINMAX

#include <math.h>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
namespace exttype{
	template <class T>T min(const T & a, const T & b){
		return a<=b?a:b;
	};
	template <class T>T max(const T & a, const T & b){
		return a>=b?a:b;
	};
	template <class T>T min(const T & a, const T & b, const T & c){
		return (min(min(a,b),c));
	};
	template <class T>T max(const T & a, const T & b, const T & c){
		return (max(max(a,b),c));
	};

};
#endif

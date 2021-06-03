#ifndef geom_r1geometry_h
#define geom_r1geometry_h

#include "geom/r2geometry/vect2.h"

namespace geom{
	namespace R1{
		class Interval:public vect2{
		private:
			typedef vect2 parent;
		public:
			Interval(){};
			Interval(double a, double b):parent(a,b){};
			Interval(const parent & x):parent(x){};
			Interval(const Interval & x):parent(x){};
			Interval intersect(const Interval & x)const;
			Interval coverage(const Interval & x)const;
			Interval coverage(const double & x)const;
			static Interval order(double a, double b);
			double l()const;
			int isEmpty()const;
			int holds(double x);
			double center()const;
		public:
			static const Interval empty;
		};
	};
};
#endif

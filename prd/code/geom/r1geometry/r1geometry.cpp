#include "r1geometry.h"

#include "exttype/minmax.h"
#include "geom/math.h"

namespace geom{
	namespace R1{
		const double eps0=1e-6;
		//________________Interval______________________________
		const Interval Interval::empty(0,0);

		int Interval::isEmpty()const{
			return l()<eps0;
		};

		Interval Interval::order(double a, double b){
			return Interval(std::min(a,b),std::max(a,b));
		};

		Interval Interval::intersect(const Interval & x)const{
			Interval r( std::max((*this)[0],x[0]),std::min((*this)[1],x[1]) );
			if(r[1]<r[0])r[1]=r[0];
			return r;
		};

		Interval Interval::coverage(const Interval & x)const{
			if(x.isEmpty())return *this;
			if(isEmpty())return x;
			Interval r( std::min((*this)[0],x[0]),std::max((*this)[1],x[1]) );
			return r;
		};
		
		Interval Interval::coverage(const double & x)const{
			Interval r( std::min((*this)[0],x),std::max((*this)[1],x) );
			return r;
		};

		double Interval::l()const{
			return math::abs((*this)[1]-(*this)[0]);
		};

		int Interval::holds(double x){
			return (x>=(*this)[0] && x<=(*this)[1]);
		};
		double Interval::center()const{
			return ((*this)[0]+(*this)[1])/2;
		};
	};
};

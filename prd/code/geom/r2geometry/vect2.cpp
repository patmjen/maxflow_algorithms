#include "vect2.h"
#include "../math.h"

namespace geom{


	vectn<2> vectn<2>::project(const vectn & p1, const vectn & p2)const{
		vectn<2> v = (p2-p1).normalized();
		return v*((*this-p1)*v)+=p1;
	};

	double vectn<2>::distto(const vectn<2> & p1, const vectn<2>& p2)const{
		return (snaptoline(p1,p2)-*this).norm();
	};

	double vectn<2>::operator ^ (const vectn<2> & x)const{
		return operator[](0)*x.operator[](1) - operator[](1)*x.operator[](0);
	};

	double vectn<2>::sin(const vectn<2> & x)const{
		return ((*this)^x)/(norm()*x.norm());
	};

	double vectn<2>::deflection(const vectn<2> & x)const{
		return (1-((*this)*x)/(norm()*x.norm()))*math::sgn((*this)^x);
	};

	double vectn<2>::absdeflection(const vectn<2> & x)const{
		int sgnsin = math::sgn((*this)^x);
		double r = (1+((*this)*x)/(norm()*x.norm()));
		if(sgnsin!=0)return r*sgnsin; else return -r;
	};

	double vectn<2>::deflectionangle(const vectn<2> & x)const{
		double s = ((*this)^x)/(norm()*x.norm());
		if(s>1)s=1;
		if(s<-1)s=-1;
		double a = asin(s);
		if(math::sgn((*this)*x)<0)a=pi-a;
		if(a>pi+eps0)a = a-2*pi;
		return a;
	};

	double vectn<2>::angle(const vectn<2> &x)const{
		double s = ((*this)^x)/(norm()*x.norm());
		if(s>1)s=1;
		if(s<-1)s=-1;
		double a = asin(s);
		if(math::sgn((*this)*x)<0)a=pi-a;
		else if(a<0)a = 2*pi+a;
		return a;
	};

	vectn<2> vectn<2>::snaptoline(const vectn<2> &p1, const vectn<2> &p2)const{
		vectn<2> v = (p2-p1);
		if(v.norm()<eps0)return p1;
		v.norm();
		double x = (*this-p1)*v;
		vectn<2> r = v*x;
		if(x<eps0) return p1;
		if(x>(p2-p1)*v-eps0) return p2;
		return r+p1;
	};

	vectn<2> vectn<2>::getOrtogonalVector(){
		vectn<2> r;
		r[0] = (*this)[1];
		r[1] = -(*this)[0];
		return r;
	};
};

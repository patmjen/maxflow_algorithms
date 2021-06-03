#ifndef vect2_h
#define vect2_h
#include "../vectn.h"

namespace geom{
	template<> class vectn_binder<2>{
	public:
		typedef ivector<vectn<2>, array_n_binder<vectn,double,2>::base> base;
	};
	//_____________________vect2______________________________________________________________
	template <> class vectn<2>:public vectn_binder<2>::base{
	private:
		typedef vectn_binder<2>::base parent;
		typedef vectn<2> tvectn;
	public:
		typedef intn<1> tindex;
		typedef itern<1> titerator;
		titerator get_iterator()const{return titerator(size());};
	public:
		vectn(){};
		vectn(const vectn & x):parent(x){};
		vectn(const double x,const double y){
			set_elements(x,y);
		};
     
		vectn project(const vectn & p1, const vectn & p2)const;
		double distto(const vectn<2> & p1, const vectn<2>& p2)const;
		double operator ^ (const vectn<2> & x)const;
		double sin(const vectn<2> & x)const;
		double deflection(const vectn<2> & x)const;//def: 0->0 pi->0 (0,pi)->(0,-2) (pi,2*pi)->(-2,0)
		double absdeflection(const vectn<2> & x)const; //def: 0->-2 pi->0 (0,pi)->(-2,0) (pi,2*pi)->(0,2)
		double deflectionangle(const vectn<2> & x)const; //def: (-pi,pi) ;  v.deflectionangle(v)=0;
		double angle(const vectn<2> &x)const; //def (0,2*pi) ;  v.angle(v)=0;
		vectn<2> snaptoline(const vectn<2> &p1, const vectn<2> &p2)const;// closest point on the line segment (p1,p2)
		vectn<2> getOrtogonalVector();
	};

	typedef vectn<2> vect2;
};
#endif

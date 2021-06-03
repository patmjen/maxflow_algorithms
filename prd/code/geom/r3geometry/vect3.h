#ifndef geom_vect3_h
#define geom_vect3_h

#include "geom/vectn.h"

namespace geom{
	template <> class vectn<3>:public vectn_binder<3>::base{
	private:
		typedef vectn_binder<3>::base parent;
		typedef vectn<3> tvectn;
	public:
		typedef intn<1> tindex;
		typedef itern<1> titerator;
		titerator get_iterator()const{return titerator(size());};
	public:
		vectn(){};
		vectn(const vectn & x):parent(x){};
		vectn(const double x,const double y,const double z){
			set_elements(x,y,z);
		};
	public:
		//default operator = 
	public:
		vectn rotate0(double sf,double cf);
		vectn rotate1(double sf,double cf);
		vectn rotate2(double sf,double cf);
		vectn operator ^(const vectn & x2)const;
	public:

	};
	typedef vectn<3> vect3;
};
#endif

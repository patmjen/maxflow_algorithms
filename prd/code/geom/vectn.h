#ifndef geom_vectn_h
#define geom_vectn_h

#include "exttype/intn.h"
#include "exttype/itern.h"
#include "exttype/arrayn.h"
#include "exttype/ivector.h"

namespace geom{
	using namespace exttype;
	//___________________________vectn______________________________________
	//! vectn<n> - double type vector extension
	/*!
		Primal type for working with vectors in R^n.
	*/
	template<int n> class vectn;
	template<int n> class vectn_binder{
	public:
		typedef ivector<vectn<n>,typename array_n_binder<vectn,double,n>::base> base;
	};
	
	template<int n> class vectn: public vectn_binder<n>::base{
	private:
		typedef typename vectn_binder<n>::base parent;
		typedef vectn<n> tvectn;
	public:
		typedef intn<1> tindex;
		typedef itern<1> titerator;
		titerator get_iterator()const{return titerator(this->size());};
	public:
		template<int n2> struct rebind{
			typedef vectn<n2> array;
		};
	public:
		vectn(){};
		vectn(const vectn& x):parent(x){};
		vectn(double v0,double v1,double v2=0,double v3=0,double v4=0){
			parent::set_elements(v0,v1,v2,v3,v4);
		};
		template <class other, class base> explicit vectn(const iarray_n<other,base> & x){
			(*this) << x;
		};
		vectn& operator =(const vectn & x){parent::operator = (x); return *this;};
		vectn& operator =(double z){(*this)<<z;return *this;};
	};

	template <> class vectn<1> : public vectn_binder<1>::base{
	private:
		typedef vectn_binder<1>::base parent;
		typedef vectn<1> tvectn;
	public:
		template<int n2> struct rebind{
			typedef vectn<n2> array;
		};
	public:
		vectn(){};
		vectn(const double x){(*this)[0]=x;};
		operator double&(){return (*this)[0];};
		operator const double&()const{return (*this)[0];};
	};

	//_________________________typedefs__________________________
	//typedef vectn<2> vect2;
	//typedef vectn<3> vect3;
	typedef vectn<4> vect4;
};

#include "geom/r2geometry/vect2.h"
#include "geom/r3geometry/vect3.h"

#endif

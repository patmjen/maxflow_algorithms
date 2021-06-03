#ifndef geom_pvect_h
#define geom_pvect_h

#include "exttype/ivector.h"
#include "exttype/itern.h"
#include "exttype/intn.h"

#include "dynamic/buffer.h"

namespace exttype{
	//_________________________pvect_________________________________________________
	template <typename field> class pvect;

	template<typename field> struct pvect_binder{
		typedef typename dynamic::ref_buffer<field> container;
		typedef ivector<pvect<field>, container> base;
	};
	
	template <typename field> class pvect : public pvect_binder<field>::base{
	public:
		typedef typename pvect_binder<field>::container container;
		typedef typename pvect_binder<field>::base parent;
	private:
		//! forbidden
		pvect cpy()const{return pvect();};
	public:
		pvect(field * x, int n){
			this->set_ref(x,n);
		};
		//template<template <typename field2> class tvect> pvect(tvect<field> & x):parent(&x[0],x.size()){};
		template<int n> explicit pvect(array_tn<field,n> & x):parent(&x[0],x.size()){};
		template<class target,class base> explicit pvect(ivector<target, base> & y){
			assert(sizeof(field)==sizeof(typename base::telement));
			set_ref(&y[0],y.size());
		};
	};
	typedef pvect<double> pdouble;
	typedef pvect<float> pfloat;
};
#endif

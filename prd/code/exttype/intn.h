#ifndef intn_h
#define intn_h

#include "ivector.h"
#include "arrayn.h"
#include <stddef.h>

namespace exttype{
	template<int n> class intn;
	template<int n> struct intn_binder{
		typedef ivector<intn<n>,typename array_n_binder<intn,int,n>::base> base;
		//typedef ivector<intn<n>, iarray_n<intn<n> ,array_tn_base<array_n_binder<intn,int,n>,int,n> > > base;
	};
	//______________________________________intn___________________________________________
	//! intn<n> - int type vector extention
	template<int n> class intn: public intn_binder<n>::base{
	private:
		typedef typename intn_binder<n>::base parent;
		typedef intn<n> tintn;
	public:
		template<int n2> struct rebind{
			typedef intn<n2> array;
		};

	public:
		int & operator[](const int i){return parent::elem(i);};
		const int & operator[](const int i)const{return parent::elem(i);};

		intn(){};
		intn(const intn& x):parent(x){};
		intn(int v0,int v1,int v2=0,int v3=0,int v4=0){
			parent::set_elements(v0,v1,v2,v3,v4);
		};
		template <class other, class base> explicit intn(const iarray_n<other,base> & x){
			(*this) << x;
		};
		intn& operator =(const intn & x){parent::operator = (x); return *this;};
		intn& operator =(int z){(*this)<<z;return *this;};
	};

	template <> class intn<1> : public intn_binder<1>::base{
	private:
		typedef intn_binder<1>::base parent;
		typedef intn<1> tintn;
	public:
		template<int n2> struct rebind{
			typedef intn<n2> array;
		};
	public:
		int & operator[](const int i){return elem(i);};
		const int & operator[](const int i)const{return elem(i);};
		intn(){};
		intn(const int x){elem(0)=x;};//implicit
		operator int&(){return elem(0);};
		operator const int&()const{return elem(0);};
	};

	template<int n> class charn;
	template<int n> struct charn_binder{
		typedef ivector<charn<n>, typename array_n_binder<charn,char,n>::base> base;
	};
	//______________________________________charn___________________________________________
	//! charn<n> vector extention
	template<int n> class charn: public charn_binder<n>::base{
	private:
		typedef typename charn_binder<n>::base parent;
		typedef charn<n> tcharn;
	public:
		charn(){};
		charn(const charn& x):parent(x){};
		charn(char v0,char v1,char v2=0,char v3=0,char v4=0){
			parent::set_elements(v0,v1,v2,v3,v4);
		};
		template<typename type2> explicit charn(const array_tn<type2,n> & x){
			(*this) << x;
		};
		charn& operator =(const charn & x){parent::operator = (x); return *this;};
		charn& operator =(int z){(*this)<<z;return *this;};
	};

	//______________--
	template<int n> class ucharn;
	template<int n> struct ucharn_binder{
		typedef ivector<ucharn<n>, typename array_n_binder<ucharn,unsigned char,n>::base> base;
	};
	//______________________________________ucharn___________________________________________
	//! charn<n> vector extention
	template<int n> class ucharn: public ucharn_binder<n>::base{
	private:
		typedef typename ucharn_binder<n>::base parent;
		typedef ucharn<n> tcharn;
		typedef unsigned char telement;
	public:
		ucharn(){};
		ucharn(const ucharn& x):parent(x){};
		ucharn(telement v0,telement v1,telement v2=0,telement v3=0,telement v4=0){
			parent::set_elements(v0,v1,v2,v3,v4);
		};
		template<typename type2> explicit ucharn(const array_tn<type2,n> & x){
			(*this) << x;
		};
		ucharn& operator =(const ucharn & x){parent::operator = (x); return *this;};
		ucharn& operator =(int z){(*this)<<z;return *this;};
	};

//______________________________________size_tn___________________________________________
	template<int n> class size_tn;
	template<int n> struct size_tn_binder{
		typedef ivector<size_tn<n>, typename array_n_binder<size_tn,std::size_t ,n>::base> base;
	};
	//! size_tn<n> vector extention
	template<int n> class size_tn: public size_tn_binder<n>::base{
	private:
		typedef typename size_tn_binder<n>::base parent;
		typedef size_tn<n> mytype;
		typedef unsigned char telement;
	public:
		size_tn(){};
		size_tn(const mytype& x):parent(x){};
		size_tn(telement v0,telement v1,telement v2=0,telement v3=0,telement v4=0){
			parent::set_elements(v0,v1,v2,v3,v4);
		};
		template<typename type2> explicit size_tn(const array_tn<type2,n> & x){
			(*this) << x;
		};
		mytype& operator =(const mytype & x){parent::operator = (x); return *this;};
		mytype& operator =(int z){(*this)<<z;return *this;};
	};

	//______________________________________floatn___________________________________________
	template<int n> class floatn;
	template<int n> struct floatn_binder{
		typedef ivector<floatn<n>, typename array_n_binder<floatn,float ,n>::base> base;
	};
	//! float vector extention
	template<int n> class floatn: public floatn_binder<n>::base{
	private:
		typedef typename floatn_binder<n>::base parent;
		typedef floatn<n> mytype;
	public:
		floatn(){};
		floatn(const floatn& x):parent(x){};
		floatn(floatn v0,floatn v1,floatn v2=0,floatn v3=0,floatn v4=0){
			parent::set_elements(v0,v1,v2,v3,v4);
		};
		template<typename type2> explicit floatn(const array_tn<type2,n> & x){
			(*this) << x;
		};
		mytype& operator =(const mytype & x){parent::operator = (x); return *this;};
		mytype& operator =(int z){(*this)<<z;return *this;};
	};

	/*
	template<class vector_2, class vector2_container> static tintn floor(const Ivector<vector_2, vector2_container>& x){
	tintn r;
	for(int i=0;i<n;i++)r[i] = (int)::floor(x[i]);
	return r;
	};
	template<class vector_2, class vector2_container> static tintn ceil(const Ivector<vector_2, vector2_container>& x){
	tintn r;
	for(int i=0;i<n;i++)r[i] = (int)::ceil(x[i]);
	return r;
	};
	*/
	//_________________________typedefs__________________________
	typedef intn<1> mint1;
	typedef intn<2> mint2;
	typedef intn<3> mint3;
	typedef intn<4> mint4;
	typedef charn<2> mchar2;
	typedef charn<3> mchar3;
	typedef charn<4> mchar4;
	typedef ucharn<2> muchar2;
	typedef ucharn<3> muchar3;
	typedef ucharn<4> muchar4;
	typedef size_tn<2> size_t2;
	typedef size_tn<3> size_t3;
	typedef size_tn<4> size_t4;
	typedef floatn<2> mfloat2;
	typedef floatn<3> mfloat3;
	typedef floatn<4> mfloat4;
};
#endif

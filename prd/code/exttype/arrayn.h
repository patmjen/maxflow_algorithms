/*! \file arrayn.h
\brief Generic fixed-length array.
*/

#ifndef exttype_arrayn_h
#define exttype_arrayn_h

#include "assert.h"
#include "debug/logs.h"

namespace exttype{

	//_____________________iarray______________________________________________________	
	//! Basic array Traits.
	/*! requires noninherited target::size(), target::operator [],
	*/
	template <class target, typename telement> class iarray{
	private:
		target * ths(){return static_cast<target*>(this);};
		const target * ths()const{return static_cast<const target*>(this);};
	protected:
		iarray(){};
	public:
		int size()const{ return ths()->size();};
		int count(int i=0)const{ return size();};
		telement& operator [](const int i){return (*ths())[i];};
		const telement& operator [](const int i)const{return (*ths())[i];};
		telement * begin(){return &(*ths())[0];};
		const telement * begin()const{return &(*ths())[0];};
		telement * end(){return begin()+size();};
		const telement * end()const{return begin()+size();};
		telement & last(){return (*ths())[count()-1];};
		const telement & last()const{return (*ths())[count()-1];};
		telement & first(){return (*ths())[0];};
		const telement & first()const{return (*ths())[0];};
	protected:
		telement& item(const int i){return (*ths())[i];};
		const telement& item(const int i)const{return (*ths())[i];};
	public:
		bool inline operator == (const iarray<target,telement> & x)const{
			for(int i=0;i<size();i++)if(!((*ths())[i]==x[i]))return false;
			return true;
		};
		bool inline operator != (const iarray<target,telement> & x)const{
			for(int i=0;i<size();i++)if((*ths())[i]!=x[i])return true;
			return false;
		};
	};

	//____________________________iarray_n_____________________________________________
	//! Fixed-length Array Traits.
	template <class target, class base> class iarray_n: public base{
		//requires base : public iarray<base>, base::telement, base::n, base::rebind<type2,n2>
	private:
		target * ths(){return static_cast<target*>(this);};
		const target * ths()const{return static_cast<const target*>(this);};
	public:
		typedef typename base::telement telement;
		enum my_size{n = base::n};
		template<int n2> struct rebind{
			typedef typename base::template rebind<n2>::array array;
		};
	private:
		//template<class target2,class base2> struct concat_result{
		//	typedef typename base::template rebind<n+base2::n>::array array;
		//};
		typedef typename rebind<n-1>::array tsub_array;
		typedef typename rebind<n+1>::array tsup_array;
	public:
		iarray_n(){};
		void set_elements(const telement & v0, const telement & v1,const telement & v2=telement(), const telement & v3=telement(), const telement & v4=telement()){
			if(n>0)(*this)[0] = v0;
			if(n>1)(*this)[1] = v1;
			if(n>2)(*this)[2] = v2;
			if(n>3)(*this)[3] = v3;
			if(n>4)(*this)[4] = v4;
		};
	public:
		target reverse()const{
			target r;
			for(int i=0;i<n;++i){
				r[i] = (*this)[n-i-1];
			};
			return r;
		};
	public:
		//! subarray [0:n-2]
		tsub_array inline sub()const{
			tsub_array r;
			for(int i=0;i<n-1;++i){
				r[i] = (*this)[i];
			};
			return r;
		};

		//! subarray [0:n2-1]
		template<int n2> typename rebind<n2>::array sub()const{
			typename rebind<n2>::array r;
			for(int i=0;i<n2;++i){
				r[i] = (*this)[i];
			};
			return r;
		};

		//! subarray [n_first:n_last]
		template<int n_first, int n_last> inline typename rebind<n_last-n_first+1>::array sub()const{
			typename rebind<n_last-n_first+1>::array r;
			for(int i=n_first;i<=n_last;++i){
				r[i-n_first] = (*this)[i];
			};
			return r;
		};

		//! subarray [1:n-1]
		tsub_array inline rsub()const{
			tsub_array r;
			for(int i=0;i<n-1;++i){
				r[i] = (*this)[i+1];
			};
			return r;
		};

		//! subarray [n-n2:n-1]
		template<int n2> inline typename rebind<n2>::array rsub()const{
			typename rebind<n2>::array r;
			for(int i=0;i<n2;++i){
				r[i] = (*this)[i+n-n2];
			};
			return r;
		};

		//! subarray [0:l] U [l+1,n-1]
		tsub_array inline erase(int l)const{
			tsub_array r;
			for(int k=0, j=0;k<n;++k){
				if(k==l)continue;
				r[j++]=(*this)[k];
			};
			return r;
		};

	public:
		//! assignment
		template <class target2, class base2> inline target & operator << (const iarray_n<target2,base2> & y){
			for(int i=0;i<n && i<base2::n;++i){
				(*this)[i] = telement(y[i]);
			};
			return *ths();
		};
		//! assignment
		inline target & operator << (const telement & c){
			for(int i=0;i<n;++i){
				(*this)[i] = c;
			};
			return *ths();
		};
	public:
		//! concatenation
		tsup_array operator |(const telement & c)const{
			tsup_array r;
			r << *ths();
			r.last() = c;
			return r;
		};

		//! concatenation
		//template<class target2,class base2> typename concat_result<target2,base2>::array operator |(const iarray_n<target2,base2> & y)const{
		template<class target2,class base2> typename base::template rebind<n+base2::n>::array operator |(const iarray_n<target2,base2> & y)const{			
			//typename concat_result<target2,base2>::array r;
			typename base::template rebind<n+base2::n>::array r;
			for(int i=0;i<n;++i)r[i]=(*this)[i];
			for(int i=0;i<base2::n;++i)r[i+n]=y[i];
			return r;
		};

	};

	template <class target, class base> txt::TextStream & operator << (txt::TextStream & stream, const iarray_n<target,base> & x){
		stream<<"(";
		for(int i=0;i<x.size();++i){
			stream<<x[i];
			if(i<x.size()-1)stream<<", ";
		};
		stream<<")";
		return stream;
	};


	//__________________array_tn_base________________________________________
	template <class binder, typename telement1,int N> class array_tn_base : public iarray<array_tn_base<binder,telement1,N>,telement1>{
		//requires: telemnt1()
	public:
		enum my_size{n = N};
		typedef telement1 telement;
		template<int n2> struct rebind{
			typedef typename binder::template rebind<n2>::array array;
		};
	private:
		telement v[n];
	public:
		static int size(){return n;};
		static int count(int i=0){return n;};
		telement & operator[](const int i){assert(i>=0 && i<size());return v[i];};
		const telement & operator[](const int i)const{assert(i>=0 && i<size());return v[i];};
		telement & elem(const int i){
			assert(i>=0 && i<size());
			return v[i];
		};
		const telement & elem(const int i)const{assert(i>=0 && i<size());return v[i];};
	};
	//__________________array_tn_base<1>______________________________________
	template <class binder, typename telement1> class array_tn_base<binder,telement1,1> : public iarray<array_tn_base<binder,telement1,1>,telement1>{
	public:
		enum my_size{n = 1};
		typedef telement1 telement;
		template<int n2> struct rebind{
			typedef typename binder::template rebind<n2>::array array;
		};
	private:
		telement v[n];
	public:
		static int size(){return n;};
		telement & operator[](const int i){assert(i>=0 && i<size());return v[i];};
		const telement & operator[](const int i)const{assert(i>=0 && i<size());return v[i];};
		telement & elem(const int i){assert(i>=0 && i<size());return v[i];};
		const telement & elem(const int i)const{assert(i>=0 && i<size());return v[i];};
	public:
		operator telement&(){return (*this)[0];};
		operator const telement&()const{return (*this)[0];};
	};


	/*
	template<template<typename,int> class target, typename telement, int n> struct array_tn_binder{
	typedef target<telement,n> array;
	template<int n2> struct rebind{
	typedef typename target<telement,n2> array;
	};
	typedef iarray_n<array,array_tn_base<array_tn_binder, telement,n> > base;
	};
	*/

	template <typename telement1,int n> class array_tn;
	template<typename telement, int n> struct array_tn_binder{
		typedef array_tn<telement,n> array;
		template<int n2> struct rebind{
			typedef array_tn<telement,n2> array;
		};
		typedef iarray_n<array_tn<telement,n> ,array_tn_base<array_tn_binder, telement,n> > base;
	};

	template <template<int> class target,typename telement,int n> struct array_n_binder{
		typedef target<n> array;
		template<int n2> struct rebind{
			typedef target<n2> array;
		};
		typedef iarray_n<array,array_tn_base<array_n_binder<target,telement,n>, telement,n> > base;
	};

	//__________________array_tn_____________________________________________
	//! generic statically allocated array
	template <typename telement1,int n> class array_tn: public array_tn_binder<telement1,n>::base{
		//requires: telemnt1()
	private:
		typedef typename array_tn_binder<telement1,n>::base parent;
	public:
		typedef telement1 telement;
		typedef array_tn<telement,n> tarray;
		template<int n2> class rebind{
		public:
			typedef array_tn<telement1,n2> array;
		};
	public:
		array_tn(){};
		array_tn(const tarray & x){
			(*this)= x;
		};
		template<typename type2> explicit array_tn(const array_tn<type2,n> & x){
			(*this) << x;
		};
		array_tn(const telement & v0, const telement & v1,const telement & v2=telement(), const telement & v3=telement(), const telement & v4=telement()){
			set_elements(v0,v1,v2,v3,v4);
		};
		tarray & operator=(const tarray & x){
			(*this) << x;
			return *this;
		};
	public:
	};
};
#endif

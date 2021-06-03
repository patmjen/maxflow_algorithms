#ifndef fixed_array2_h
#define fixed_array2_h

#include "assert.h"
#include <memory>
#include "fixed_array1.h"
#include "exttype/intn.h"
#include "dynamic/buffer.h"

namespace dynamic{
	using namespace exttype;
	using exttype::mint2;
	//______________________fixed_array_2______________________________
	template<class type, class Allocator = dynamic::array_allocator<type> > class fixed_array2: public fixed_array1<type, Allocator>{
	private:
		mint2 CC;
	private:
		typedef fixed_array1<type, Allocator> parent;
	private:
		int linindex(const mint2 & i)const{return i[0]+i[1]*CC[0];};
		int linsize(const mint2 & size)const{return size[0]*size[1];};
		int linsize()const{return CC[0]*CC[1];};
	public:
		fixed_array2(const Allocator & al = Allocator()):CC(0,0),parent(al){};
		fixed_array2(const mint2 size, const Allocator & al = Allocator() ):CC(size),parent(linsize(size),al){};
		fixed_array2(const mint2 size, const type & val, const Allocator & al = Allocator() ):CC(size),parent(linsize(size),val,al){};
		fixed_array2(const fixed_array2 & x):CC(x.CC),parent(x){};
		fixed_array2 & operator = (const fixed_array2 & x){
			CC = x.CC;
			parent::operator =(x);
			return *this;
		};
	public:
		const mint2 & size()const{return CC;};
		type & operator [](const mint2 & i){
			return parent::operator[](linindex(i));
		};
		const type & operator [](const mint2 & i)const{
			return parent::operator[](linindex(i));
		};
		type & operator [](const int i){
			return parent::operator[](i);
		};
		const type & operator [](const int i)const{
			return parent::operator[](i);
		};
	public:
		void resize(const mint2 & size){
			parent::resize(linsize(size));
			CC = size;
		};
	private:// not available
		void capacity()const{};
		void free_capacity()const{};

	private:// not available
		void push_back(const type & val){};
		void pop_back(){};
		void erase(type * it){};
		void erase(type * it1,type * it2){};
	};

//______________________pfixed_array_2______________________________
	template<class type> class pfixed_array2: public ref_buffer<type>{
	private:
		mint2 CC;
	private:
		typedef ref_buffer<type> parent;
	private:
		int linindex(const mint2 & i)const{return i[0]+i[1]*CC[0];};
		int linsize(const mint2 & size)const{return size[0]*size[1];};
		int linsize()const{return CC[0]*CC[1];};
	public:
		pfixed_array2():CC(0,0){};
		pfixed_array2(type * pval,const mint2 size):CC(size),parent(pval,linsize(size)){};
		pfixed_array2(const pfixed_array2 & x):CC(x.CC),parent(x){};
		pfixed_array2 & operator = (const pfixed_array2 & x){
			CC = x.CC;
			parent::operator =(x);
			return *this;
		};
	public:
		void set_ref(type * pval,const mint2 size){
			CC = size;
			parent::set_ref(pval,linsize(size));
		};
	public:
		const mint2 & size()const{return CC;};
		type & operator [](const mint2 & i){
			return parent::operator[](linindex(i));
		};
		const type & operator [](const mint2 & i)const{
			return parent::operator[](linindex(i));
		};
	};


	template<class type> class varray2 : public fixed_array1<fixed_array1<type> >{
	public:
		typedef fixed_array1<fixed_array1<type> > parent;
	public:
	};

	template class fixed_array2<double,std::allocator<double> >;
	template class fixed_array2<double,dynamic::array_allocator<double> >;

	template class pfixed_array2<double>;
};

#endif

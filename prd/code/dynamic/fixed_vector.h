#ifndef fixed_vectr_h
#define fixed_vectr_h

#include "assert.h"
#include <memory>

namespace dynamic{
	//! fixed_vector implements (subset of) std::vector behaviour but never allows reallocation
	/*!
	Usful for allocating fixed size vectors, when size is small and known at run-time or bounded.
	Preserves its possition in the memory so pointers valid durin lifetime of fixed_vector.
	Also it can be used together with any std::allocator, it is especially usefull when particular allocator does
	not support reallocation. In particular efficient with array_allocator<type> --
	allows fast and efficient construction of vectors on the "stack".
	allocator::allocate() is guaranteed to be called at most once.
	*/
	template<typename type, class Allocator = std::allocator<type> > class fixed_vector{
	private:
		Allocator _al;
	private:
		type * _beg;
		type * _end;
		type * _cap;
	public:
		typedef type value_type;
		typedef int size_type;
		typedef type telement;
	public:
		type * begin()const{return _beg;};
		type * end()const{return _end;};
		int capacity()const{return int(_cap-_beg);};
		int free_capacity()const{return int(_cap-_end);};
		int size()const{return int(_end-_beg);};
		int count(int i=0)const{return size();};
		bool allocated()const{return _beg!=0;};
		bool empty()const{return size()==0;};
		type & back(){
			assert(size()>0);
			return *(end()-1);
		};
		const type & back()const{
			assert(size()>0);
			return *(end()-1);
		};
		type & front(){
			assert(size()>0);
			return *begin();
		};
		const type & front()const{
			assert(size()>0);
			return *begin();
		};
	public:

		type & operator[](const int i){
			assert(i>=0 && i<size());
			return *(begin()+i);
		};

		const type & operator[](const int i)const{
			assert(i>=0 && i<size());
			return *(begin()+i);
		};

	private:
		void allocate(const int size){
			_beg = _al.allocate(size);
			_end = _beg;
			_cap = _beg+size;
		};
		void create(type * beg, type *end, const type & val = type()){
			while(beg<end){
				_al.construct(beg,val);
				++beg;
			};
		};

		void inline create_copy(type * beg, type *end, type *it){
			while(beg<end){
				_al.construct(beg,*it);
				++beg;
				++it;
			};
		};

		void create(type * beg, const type & val){
			_al.construct(beg,val);
		};

		void destroy(type * beg, type *end){
			if(end==0)return;
			--end;
			while(end>=beg){
				_al.destroy(end);
				--end;
			};
		};

		void destroy(type * it){
			_al.destroy(it);
		};

	public:
		fixed_vector(const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){};

		explicit fixed_vector(int size, const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){
			allocate(size);
			create(_beg,_cap);
			_end=_cap;
		};

		explicit fixed_vector(int size, const type & val, const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){
			allocate(size);
			create(_beg,_cap,val);
			_end=_cap;
		};

		fixed_vector(const fixed_vector & x):_al(x._al),_beg(0),_end(0),_cap(0){
			*this = x;
		};

		void operator = (const fixed_vector & x){
			if(&x==this)return;
			if(!x.allocated()){
				clear();
				return;
			};
			if(!allocated()){
				allocate(x.capacity());
			}else{
				if(capacity()<x.size())throw debug_exception("reallocation of fixed_vector is not allowed");
			};
			if(begin()+x.size()<end()){
				destroy(begin()+x.size(),end());
				_end = begin()+x.size();
			};
			std::copy(x.begin(),x.begin()+size(),begin());   //copy size() elements by type::operator = 
			create_copy(end(),begin()+x.size(),x.begin()+size()); // copy x.size()-size() elements by type::type(const type&)
			_end = begin()+x.size();
		};

		void clear(){
			if(allocated()){
				destroy(begin(),end());
				_end = begin();
			};
		};

		~fixed_vector(){
			if(allocated()){
				clear();
				_al.deallocate(_beg,capacity());
				_beg = 0;
				_end = 0;
				_cap = 0;
			};
		};
	public:
		void reserve(const int Size){
			if(allocated()){
				if(capacity() < Size)throw debug_exception("reallocation of fixed_vector is not allowed");
			}else{
				allocate(Size);
			};
		};
		void resize(int Size, const type & val = type()){
			if(allocated() && Size>capacity())throw debug_exception("capacity exceeded");
			if(!allocated()){
				allocate(Size);
			};
			if(begin()+Size<end()){
				destroy(begin()+Size,end());
			}else{
				create(_beg+size(),_beg+Size,val);
			};
			_end = begin()+Size;
		};
	public:

		void push_back(const type & val){
			if(free_capacity()==0)throw debug_exception("fixed_vector capacity exceeded");
			create(end(),val);
			++_end;
		};

		void pop_back(){
			if(empty())throw debug_exception("pop_back from empty fixed_vector");
			destroy(end());
			--_end;
		};

		void erase(type * it){
			if(!(it>begin() && it<end()))throw debug_exception("erase out of range of fixed_vector");
			destroy(it);
			_end = std::copy(it+1,end(),it);
		};

		void erase(type * it1,type * it2){
			if(it1==it2)return;
			if(!(it1>=begin() && it1<=it2 && it2<=end()))throw debug_exception("erase out of range of fixed_vector");
			destroy(it1,it2);
			int n = int(it2-it1);
			_end = std::copy(it2,end(),it1);
		};
	};

	template class fixed_vector<int>;
};

#endif

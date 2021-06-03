#ifndef fixed_array1_h
#define fixed_array1_h

#include "assert.h"
#include <memory>
//#include "debug/logs.h"

#ifdef NO_EXTTYPE
#define DEFAULT_ALLOCATOR std::allocator<type>
#else
#include "array_allocator.h"
#define DEFAULT_ALLOCATOR array_allocator<type>
#endif

namespace dynamic{
	//! fixed_array1 implements (subset of) std::vector behaviour but never allows reallocation
	/*!
	Usful for allocating fixed size vectors, when size is small and known at run-time or bounded.
	Preserves its possition in the memory so pointers valid durin lifetime of fixed_array1.
	Also it can be used together with any std::allocator, it is especially usefull when particular allocator does
	not support reallocation. In particular efficient with array_allocator<type> --
	allows fast and efficient construction of vectors on the "stack".
	allocator::allocate() is guaranteed to be called at most once.
	*/
	template<typename type, class Allocator = DEFAULT_ALLOCATOR > class fixed_array1{
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

		void create(type * beg, type *end){
			while(beg<end){
				//std allocator does not have a member to initialize with default constructor
				//_al.construct(beg);
				new ((void *)beg) type();
				++beg;
			};
		};

		void create(type * beg, type *end, const type & val){
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
		fixed_array1(const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){};

		explicit fixed_array1(int size, const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){
			if(size>0){
				allocate(size);
			};
			create(_beg,_cap);
			_end=_cap;
		};

		explicit fixed_array1(int size, const type & val, const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){
			if(size>0){
				allocate(size);
			};
			create(_beg,_cap,val);
			_end=_cap;
		};

		fixed_array1(const fixed_array1 & x):_al(x._al),_beg(0),_end(0),_cap(0){
			*this = x;
		};

		void operator = (const fixed_array1 & x){
			const int xs = x.size();
			if(&x==this)return;
			if(!x.allocated()){
				clear();
				return;
			};
			if(!allocated()){
				if(x.capacity()>0){
					allocate(x.capacity());
				};
			}else{
				if(capacity()<xs)throw debug_exception("reallocation of fixed_array1 is not allowed");
			};
			if(begin()+xs<end()){
				destroy(begin()+xs,end());
				_end = begin()+xs;
			};
			const int s = size();
			std::copy(x.begin(),x.begin()+s,begin());   //copy size() elements by type::operator = 
			if(s<xs){
				create_copy(end(),begin()+xs,x.begin()+s); // copy xs-size() elements by type::type(const type&)
			};
			_end = begin()+xs;
		};

		//! makes empty, keeps memory reserved
		void clear(){
			if(allocated()){
				destroy(begin(),end());
				_end = begin();
			};
		};

		//! makes empty and deallocates memory
		// should not be called manually
		void destroy(){
			if(allocated()){
				clear();
				_al.deallocate(_beg,capacity());
				_beg = 0;
				_end = 0;
				_cap = 0;
			};
		};

		~fixed_array1(){
			destroy();
		};
	public:
		void reserve(const int Size){
			if(allocated()){
				if(capacity() < Size)throw debug_exception("reallocation of fixed_array1 is not allowed");
			}else{
				if(Size>0){
					allocate(Size);
				};
			};
		};
		void resize(int Size, const type & val){
			if(allocated() && Size>capacity())throw debug_exception("capacity exceeded");
			if(!allocated()){
				if(Size>0){
					allocate(Size);
				};
			};
			if(begin()+Size<end()){
				destroy(begin()+Size,end());
			}else{
				create(_beg+size(),_beg+Size,val);
			};
			_end = begin()+Size;
		};
		void resize(int Size){
			if(allocated() && Size>capacity())throw debug_exception("capacity exceeded");
			if(!allocated()){
				if(Size>0){
					allocate(Size);
				};
			};
			if(begin()+Size<end()){
				destroy(begin()+Size,end());
			}else{
				create(_beg+size(),_beg+Size);
			};
			_end = begin()+Size;
		};
	public:

		type * push_back(const type & val){
			if(free_capacity()==0)throw debug_exception("fixed_array1 capacity exceeded");
			create(end(),val);
			type * r = _end;
			++_end;
			return r;
		};

		type * push_back(){
			if(free_capacity()==0)throw debug_exception("fixed_array1 capacity exceeded");
			create(end(),end()+1);
			type * r = _end;
			++_end;
			return r;
		};

		void pop_back(){
			if(empty())throw debug_exception("pop_back from empty fixed_array1");
			destroy(end());
			--_end;
		};

		void erase(type * it){
			if(!(it>begin() && it<end()))throw debug_exception("erase out of range of fixed_array1");
			destroy(it);
			_end = std::copy(it+1,end(),it);
		};

		void erase(type * it1,type * it2){
			if(it1==it2)return;
			if(!(it1>=begin() && it1<=it2 && it2<=end()))throw debug_exception("erase out of range of fixed_array1");
			destroy(it1,it2);
			_end = std::copy(it2,end(),it1);
		};
	};

	template class fixed_array1<int>;

	/*
	template <class type, class al> txt::TextStream & operator << (txt::TextStream & stream, const fixed_array1<type,al> & x){
		stream<<"(";
		for(int i=0;i<x.size();++i){
			stream<<x[i];
			if(i<x.size()-1)stream<<", ";
		};
		stream<<")";
		return stream;
	};
	*/

/*
	template <class type> class pfixed_array<type>:protected ref_buffer<type>{
	public:
		pfixed_array(type * val,int n):ref_buffer(v,n){
		};
	};
*/
};

#endif

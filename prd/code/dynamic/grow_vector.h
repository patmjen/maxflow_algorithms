#ifndef grow_vectr_h
#define grow_vectr_h

#include "assert.h"
#include "debug/except.h"
#include <memory>

namespace dynamic{
	//! grow_vector implements (subset of) std::vector behaviour but has different allocation policy
	/*!
	*/
	template<typename type, class Allocator = std::allocator<type> > class grow_vector{
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
		void create(type * beg, type *end, const type & val){
			while(beg<end){
				_al.construct(beg,val);
				++beg;
			};
		};

		void create_copy(type * beg, type *end, type *it){
			while(beg<end){
				_al.construct(beg,*it);
				++beg;
				++it;
			};
		};

		void create(type * beg){
			//_al.construct(beg);
			new ((void *)beg) type;
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
		grow_vector(const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){};

		/*
		explicit grow_vector(int size, const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){
			allocate(size);
			create(_beg,_cap);
			_end=_cap;
		};
		*/

		explicit grow_vector(int size, const type & val, const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){
			allocate(size);
			create(_beg,_cap,val);
			_end=_cap;
		};

		grow_vector(const grow_vector & x):_al(x._al),_beg(0),_end(0),_cap(0){
			*this = x;
		};

		void operator = (const grow_vector & x){
			if(&x==this)return;
			if(!x.allocated() || x.size()==0){
				clear();
				return;
			};
			//if(capacity()<x.size()){
				type * w = _al.allocate(x.capacity());
				create_copy(w,w+x.size(),x.begin());
				clear();
				_al.deallocate(_beg,capacity());
				_beg = w;
				_end = _beg+x.size();
				_cap = _beg+x.capacity();
				/*
			}else{
				std::copy(x.begin(),x.begin()+x.size(),begin());
				if(begin()+x.size()<end()){
					destroy(begin()+x.size(),end());
					_end = begin()+x.size();
				};
				std::copy(x.begin(),x.begin()+size(),begin());   //copy size() elements by type::operator = 
				create_copy(end(),begin()+x.size(),x.begin()+size()); // copy x.size()-size() elements by type::type(const type&)
				_end = begin()+x.size();
			};
			*/
		};

		void clear(){
			if(allocated()){
				destroy(begin(),end());
				_end = begin();
			};
		};

		~grow_vector(){
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
				//if(capacity() < Size)throw debug_exception("reallocation of grow_vector is not allowed");
				//if(Size < size())throw debug_exception("reserve is called with size smaller then occupied range");
				if(Size > capacity()){
					type * w = _al.allocate(Size);
					create_copy(w,w+size(),begin());
					int m_size = size();
					clear();
					_al.deallocate(_beg,capacity());
					_beg = w;
					_end = _beg+m_size;
					_cap = _beg+Size;
				};
			}else{
				allocate(Size);
			};
		};
		void resize(int Size, const type & val){
//			if(allocated() && Size>capacity())throw debug_exception("capacity exceeded");
//			if(!allocated()){
//				allocate(Size);
//			};
			reserve(Size);
			if(begin()+Size<end()){
				destroy(begin()+Size,end());
			}else{
				create(_beg+size(),_beg+Size,val);
			};
			_end = begin()+Size;
		};

		void grow(){
			if(capacity()<16)reserve(16);
			else reserve(capacity()*2);
		};

	public:

		void push_back(const type & val){
			if(free_capacity()==0)grow();
			create(end(),val);
			++_end;
		};

		type & push_back(){
			if(free_capacity()==0)grow();
			create(end());
			++_end;
			return *(_end-1);
		};

		void pop_back(){
			if(empty())throw debug_exception("pop_back from empty grow_vector");
			destroy(end());
			--_end;
		};

		void erase(type * it){
			if(!(it>begin() && it<end()))throw debug_exception("erase out of range of grow_vector");
			destroy(it);
			_end = std::copy(it+1,end(),it);
		};

		void erase(type * it1,type * it2){
			if(it1==it2)return;
			if(!(it1>=begin() && it1<=it2 && it2<=end()))throw debug_exception("erase out of range of grow_vector");
			destroy(it1,it2);
			int n = int(it2-it1);
			_end = std::copy(it2,end(),it1);
		};
	};

	template class grow_vector<int>;
};

#endif
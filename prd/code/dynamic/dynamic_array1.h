#ifndef dynamic_array1_h
#define dynamic_array1_h

#include "assert.h"
#include <memory>
#include "dynamic/mallocator.h"

namespace dynamic{
	template<typename type, class Allocator = mallocator<type> > class dynamic_array1{
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
		bool full()const{return _end==_cap;}
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
		dynamic_array1(const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){};

		explicit dynamic_array1(int size, const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){
			if(size>0){
				allocate(size);
			};
			create(_beg,_cap);
			_end=_cap;
		};

		explicit dynamic_array1(int size, const type & val, const Allocator & al = Allocator() ):_al(al),_beg(0),_end(0),_cap(0){
			if(size>0){
				allocate(size);
			};
			create(_beg,_cap,val);
			_end=_cap;
		};

		dynamic_array1(const dynamic_array1 & x):_al(x._al),_beg(0),_end(0),_cap(0){
			*this = x;
		};

		//! swap without using deep copy
		void swap(dynamic_array1 & x){
			std::swap(this->_beg,x._beg);
			std::swap(this->_end,x._end);
			std::swap(this->_cap,x._cap);
			std::swap(this->_al,x._al);
		};

		void operator = (const dynamic_array1 & x){
			const int xs = x.size();
			if(&x==this)return;
			if(!x.allocated()){
				clear();
				return;
			};

			reserve(x.capacity());

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

		~dynamic_array1(){
			destroy();
		};
	public:
		//! grow x2
		void grow(){
			if(capacity()<16){
				reserve(32);
			}else{
				reserve(capacity()<<1);
			};
		};
		//! makes sure that array can hold Size elements, if the reallocation occurs the current content is copied into new position
		void reserve(const int Size){
			if(allocated()){
				//if(capacity() < Size)throw debug_exception("reallocation of dynamic_array1 is not allowed");
				if(capacity()>Size)return; // Size fits into current capacity
				//capacity is smaller than the size needed, allicate a new buffer, copy and swap
				dynamic_array1 y;
				y.reserve(Size);
				y.create_copy(y.begin(),y.begin()+this->size(),this->begin());
				y._end = y.begin()+this->size();
				this->swap(y);
			}else{
				if(Size>0){
					allocate(Size);
				};
			};
		};

		void resize(int Size, const type & val){
			reserve(Size);
			if(begin()+Size<end()){
				destroy(begin()+Size,end());
			}else{
				create(_beg+size(),_beg+Size,val);
			};
			_end = begin()+Size;
		};

		void resize(int Size){
			reserve(Size);
			if(begin()+Size<end()){
				destroy(begin()+Size,end());
			}else{
				create(_beg+size(),_beg+Size);
			};
			_end = begin()+Size;
		};
	public:
		//! add ann object to the back, initialized by copy constructor from val
		type * push_back(const type & val){
			if(full())grow();
			create(end(),val);
			type * r = _end;
			++_end;
			return r;
		};
		//! add an object to the back initialized with default constructor
		type * push_back(){
			if(full())grow();
			create(end(),end()+1);//construct 1 element
			type * r = _end;
			++_end;
			return r;
		};

		void pop_back(){
			if(empty())throw debug_exception("pop_back from empty dynamic_array1");
			destroy(end());
			--_end;
		};

		void erase(type * it){
			if(!(it>begin() && it<end()))throw debug_exception("erase out of range of dynamic_array1");
			destroy(it);
			_end = std::copy(it+1,end(),it);
		};

		void erase(type * it1,type * it2){
			if(it1==it2)return;
			if(!(it1>=begin() && it1<=it2 && it2<=end()))throw debug_exception("erase out of range of dynamic_array1");
			destroy(it1,it2);
			_end = std::copy(it2,end(),it1);
		};
	};
};

#endif

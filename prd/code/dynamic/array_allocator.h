#ifndef array_allocator_h
#define array_allocator_h

#include "block_allocator.h"
#include <limits>

namespace dynamic{
	//________________________array_allocator________________________
	//! Implementation of std::allocator for allocation on the block_allocator::global
	template<class T> class array_allocator{
	public:
		typedef int size_type;
		typedef int difference_type;
		typedef T *pointer;
		typedef const T *const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		pointer address(reference x) const{return &x;};
		const_pointer address(const_reference x) const{return &x;};
	public:
		//default constructor
		//default copy constructor
		//default operator =
	public:
		pointer allocate(size_type n, const void *hint=0){
			long long size_bytes = sizeof(T)*(long long)(n);
			if(size_bytes>std::numeric_limits<std::size_t>::max()){
				dynamic::memserver::error_allocate(size_bytes);
			};
			void * P = memserver::get_global()->allocate(size_t(size_bytes));
			return (T*)P;
		};

		void deallocate(pointer x, size_type n){
			memserver::get_global()->deallocate((char*)x);
		};

		void construct(pointer p){
			new ((void *)p) T();
		};

		void construct(pointer p, const T & val){
			new ((void *)p) T(val);
		};

		void destroy(pointer p){
			p->~T();
		};

		size_type max_size() const{
			return 0;
		};
	public:
		template<class U> struct rebind{
			typedef array_allocator<U> other;
		};
	};
};
#endif

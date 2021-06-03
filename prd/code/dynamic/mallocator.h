#ifndef mallocator_h
#define mallocator_h

	/*! mallocator - implements std::allocator using malloc
	used for low-level allocation
	*/
	//________________________array_allocator________________________
	//! Implementation of std::allocator for allocation on the block_allocator::global
	template<class T> class mallocator{
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
			void * P = malloc(n*sizeof(T));
			return (T*)P;
		};

		void deallocate(pointer x, size_type n){
			assert(x!=0);
			free(x);
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
			typedef mallocator<U> other;
		};
	};

#endif

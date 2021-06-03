#ifndef memmanager_static_allocator_h
#define memmanager_static_allocator_h

#include "buffer.h"
#include <list>

namespace dynamic{

	template<class T> class stack_allocator{
	private:
		class block{
		public:
			mutable void * p;
			mutable int size;
			mutable int blockp;
			block(int size){
				this->size = size;
				p = new char[size];
				blockp = 0;
			};
			block(const block & x):p(0){
				(*this)=x;
			};
			void operator = (const block & x){
				if(p && blockp>0)throw std::exception("block is in use");
				if(p)delete p;
				p = x.p;
				size = x.size;
				blockp = x.blockp;
				x.p = 0;
				x.size=0;
				x.blockp=0;
			};
			~block(){
				if(p)delete p;
				size = 0;
			};
		};
	private:
		bool root;
		int block_size;
		mutable std::list<block> * blocks;
		mutable int * ref_count;
	private:
		void add_block(int block_size_sp){
			blocks->push_front(block(block_size_sp));
		};
		void drop_block(){
			blocks->erase(blocks->begin());
		};
		void ready(int size){
			if(blocks->empty())add_block(std::max(block_size,size));
			else{
				block & b = blocks->front();
				if(b.blockp+size>=block_size)add_block(std::max(block_size,size));
			};
		};

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
		const static int KB = 1024;
		const static int MB = 1024*KB;
	private:
		void drop_ref(){
			if(--*ref_count==0){
				delete blocks;
				delete ref_count;
			};
			blocks=0;
			ref_count=0;
		};
	public:
		stack_allocator(int default_block_size=1*MB):block_size(default_block_size),root(true){
			blocks = new std::list<block>();
			ref_count = new int;
			*ref_count = 1;
		};
		stack_allocator(const stack_allocator & x):blocks(0),ref_count(0),root(false){
			(*this)=x;
		};
		stack_allocator<T>& operator=(const stack_allocator<T> & x){
			if(ref_count)drop_ref();
			blocks = x.blocks;
			block_size = x.block_size;
			ref_count = x.ref_count;
			++(*ref_count);
			return (*this);
		};

		void clear(){
			if(root){
				delete blocks;
				delete ref_count;
				blocks = new std::list<block>();
				ref_count = new int;
				*ref_count = 1;
			};
		};

		~stack_allocator(){
			assert(*ref_count>0);
			--(*ref_count);
			if((*ref_count)<=0 && ! root)throw std::bad_alloc("allocator scope violated");
/*
			if(root){
				delete blocks;
				delete ref_count;
			};
 */
			if((*ref_count)==0){
				assert(root);
				delete blocks;
				delete ref_count;
			};

		};
	public:
		pointer allocate(size_type n, const void *hint=0){
			int size =	sizeof(T)*n;
			ready(size);
			block & b = blocks->front();
			char * P = (char*)(b.p)+b.blockp;
			b.blockp += size;
			return (T*)P;
		};
		void deallocate(pointer x, size_type n){
			if(blocks->empty())throw std::exception("no memory allocated");
			{block & b = blocks->front();
			if(b.blockp==0)drop_block();
			};
			if(blocks->empty())throw std::exception("no memory allocated");
			block & b = blocks->front();
			char * P = (char*)(b.p)+b.blockp-sizeof(T)*n;
			if(P==(void*)x){
				b.blockp=(int)((char*)x-(char*)b.p);
			};//else throw std::exception("This allocator supports only reverse order deallocation");
		};

		void construct(pointer p, const T & val){
			new ((void *)p) T(val);
		};
		void destroy(pointer p){
			p->T::~T();
		};
		size_type max_size() const{
			return 1*MB;
		};

		template<class U>
		struct rebind {
			typedef stack_allocator<U> other;
		};
	};

};

#endif
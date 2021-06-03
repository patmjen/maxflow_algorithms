#ifndef block_allocator_h
#define block_allocator_h

//#include <list>
#include "defs.h"
#include "assert.h"
#include <list>
#include "debug/except.h"
#include "dynamic/dynamic_array1.h"
//#include "debug/logs.h"

namespace dynamic{
	typedef long long big_size;
	const int sign_block_used = 123456789;
	const int sign_block_unused = 987654321;
	const int sign_malloc = 123123123;
	/*!
	stack_allocator
	Objects are allocated in the stack-like fasion. Deallocation is delayed to match the reverse-stack order.
	If the deallocation occures in the reverse to allocation order it is processed immediately, otherwise
	it is delayed.
	When the scope is left it is guaranteed that all delayed deallocations within the scope are processed.
	Overheads: 2*sizeof(int) for each of the allocated blocks.
	_______
	Information stored per allocated block
	[size|signature_used|-block data-]
	(storing the size is only necessary to provide operator delete(void *) )
	[size|signature_unused|----------]
	buffer is filled from right to left:
	[............|block3|block2|block1]
	size is in ints (4bytes)
	*/
	class page_allocator;
	class stack_allocator{
	private:
		friend class page_allocator;
		//! memory from which to allocate
		mutable int * _beg;
		mutable int * _end;
		mutable int * _capbeg;
	public:
		static const int overhead = 2;
	public:
		int * cap_beg()const;
		int * beg()const;
		int * end()const;
		//! size in ints
		int size()const;
		//! capacity in ints
		int capacity()const;
		int cap_free()const;
		bool empty()const;
		bool allocated()const;
		bool can_allocate(size_t size_bytes)const;
	public:
		stack_allocator();
		//stack_allocator()
		void attach(int * _Beg, size_t size);
		void detach();
		stack_allocator(int * _Beg, size_t size);
	public:
		//! stealing copy constructor
		stack_allocator(const stack_allocator & x);
		//! stealing operator =
		void operator = (const stack_allocator & x);
	public:
		~stack_allocator();
	private:
		bool is_top_block(int * P)const;
		static bool is_block_used(int * P);
		static bool is_block_unused(int * P);
		static void mark_block_used(int * P);
		static void mark_block_unused(int * P);
	public:
		static int& block_size(int * P);
		static int& block_sign(int * P);
	public:
		char * allocate(int size_bytes);
		bool deallocate(void * vP);
		void clean_garbage();
		void load(char * filename);
		void unload(char * filename);
	};

	//____________________block_allocator______________________________
	/*!
	block_allocator keeps a list of buffers, and uses stack_allocator to allocate from the buffers.
	*/
	class block_allocator{
	public:
	protected:
		size_t buffer_size;
		mutable dynamic_array1<stack_allocator, mallocator<stack_allocator> > buffers;
		stack_allocator spare;//!< when deallocating, save one spare buffer
		size_t peak_reserved;
		size_t current_reserved;
		size_t current_used;
		int alloc_count;
	private:
		void took_mem(size_t size_bytes);
		void released_mem(size_t size_bytes);
	protected:
		void add_buffer(size_t buffer_size_sp);
		void drop_buffer();
	public:
		//!clean unused blocks in the buffers and drop empty buffers
		void clean_garbage();
	public:
		const static int KB = 1024;
		const static int MB = 1024*KB;
		const static int GB = 1024*MB;
	public:
		block_allocator(size_t default_buffer_size=16*MB);
		void reserve(size_t reserve_buffer_size);
	private://forbidden
		block_allocator(const block_allocator & x);
		void operator=(const block_allocator & x);
	public:
		~block_allocator();
	public:
		size_t mem_used()const;
		size_t mem_reserved()const;
		size_t mem_peak_reserved()const;
		static size_t round_up(size_t size_bytes);
	public:
		void * allocate(size_t size_bytes);
		size_t object_size(void * vP);
		void deallocate(void * vP);
		void* realloc(void * vP, size_t size_bytes);
		void error_allocate(big_size size_bytes);
	};

	//___________________page_allocaotor________________________
	class page_allocator: public block_allocator{
	public:
		page_allocator():block_allocator(){
		};
		page_allocator(int page_size):block_allocator(page_size){
			reserve(page_size);
		};
		size_t load(const char * filename);
		size_t unload(const char * filename);
	};

	//______________________memserver___________________________
	class memserver{
	public:
		//static block_allocator * global;
		//static block_allocator al_blocks;
		//static page_allocator al_page;
		static block_allocator & get_al_blocks();
		static block_allocator *& get_global();
		static void set_allocator(block_allocator & al);
		static void error_allocate(big_size size_bytes);
	};
};

void* operator new(size_t _Count);
void operator delete(void* _Ptr);
void operator delete[](void* _Ptr);
void * mmalloc(size_t size_bytes);
void * mcalloc(size_t size_bytes);
void * mrealloc(void * ptr, size_t new_size);
void mfree(void * ptr);
//
//
#endif

#ifndef memmanager_buffer_h
#define memmanager_buffer_h

#include "debug/except.h"
#include "assert.h"

#include <vector>

namespace dynamic{
	//______________________heap_buffer_____________________________
	//! heap-allocated memblock for dynamic data
	/*!
	All the reserved elements are constructed.
	*/
	template<typename type, class Allocator = std::allocator<type> > class heap_buffer{
	private:
		Allocator _al;
	protected:
		type * v;
		int sz;
		int cap;
	public:
		bool owned;
	public:
		typedef type telement;
	public:
		const type * begin()const{return v;};
		const type * end()const{return v+sz;};
		type * begin(){return v;};
		type * end(){return v+sz;};
		int capacity()const{return cap;};
		int size()const{return sz;};
		int count(int i=0)const{return size();};
		bool empty()const{return size()==0;};
	private:
		type * allocate(int Size){
			return _al.allocate(Size);
		};

		void create(type * beg, type *end, const type & val = type()){
			while(beg<end){
				_al.construct(beg,val);
				++beg;
			};
		};

		void create_copy(type * beg, type *end, const type *it){
			while(beg<end){
				_al.construct(beg,*it);
				++beg;
				++it;
			};
		};

		void destroy(type * beg, type *end){
			if(end==0)return;
			--end;
			while(end>=beg){
				_al.destroy(end);
				--end;
			};
		};

	public:
		void destroy(){
			if(owned){
				if(v){
					destroy(begin(),end());
					_al.deallocate(begin(),capacity());
					v = 0;
					cap = 0;
					sz = 0;
				};
			}else{
				v = 0;
				cap = 0;
				sz = 0;
				owned = true;
			};
		};
	public:
		//! swap without using deep copy
		void swap(heap_buffer<type, Allocator> & x){
			std::swap(v,x.v);
			std::swap(cap,x.cap);
			std::swap(sz,x.sz);
			std::swap(owned,x.owned);
		};
	public:

		heap_buffer(const Allocator & al = Allocator() ):v(0),cap(0),sz(0),owned(true),_al(al){
		};

		heap_buffer(int Size, const type & val = type(), const Allocator & al = Allocator() ):_al(al){
			v = allocate(Size);
			create(v,v+Size,val);
			cap = Size;
			sz = cap;
			owned = true;
		};

		heap_buffer(const heap_buffer & x):v(0),cap(0),sz(0),owned(true){
			if(x.owned){
				*this = x;
			}else{
				set_ref(x);
			};
		};

		//! reference constructor
		heap_buffer(type *V,int Size):owned(false){
			set_ref(V,Size);
		};

		~heap_buffer(){
			destroy();
		};
	public:
		//! operator= makes deep copy
		void operator = (const heap_buffer & x){
			if(capacity()<x.size() || !owned){
				type * w = allocate(x.size());
				create_copy(w,w+x.size(),x.begin());
				destroy();
				v = w;
				cap = x.sz;
				sz = x.sz;
				owned = true;
			}else{
				std::copy(x.begin(),x.begin()+x.size(),begin());
				sz = x.sz;
			};
		};
		//! operator= with type conversion
		template <typename type2, class Allocator2> void operator = (const heap_buffer<type2, Allocator2> & x){
			if(!owned){
				destroy();
			};
			resize(x.size());
			std::copy(x.begin(),x.begin()+x.size(),begin());
		};
	public:
		void set_ref(const heap_buffer & x){
			set_ref(x.v,x.size());
		};
		void set_ref(type * V, int Size){
			destroy();
			v = V;
			cap = Size;
			sz = Size;
			owned = false;
		};
		void set_ref(const type * V, int Size){
			destroy();
			//todo: do something about const references
			v = const_cast<type *>(V);
			cap = Size;
			sz = Size;
			owned = false;
		};
	public:
		void reserve(int Size){
			if(capacity()<Size){
				assert(sz==0);//dont try reserve non-empty buffer
				v = allocate(Size);
				cap = Size;
				sz = 0;
			}else{//has enough capacity
			};
		};

		void resize(int Size, const type & val = type()){
			if(!owned)throw debug_exception("cant resize referenced buffer");
			if(Size<=capacity()){
				if(Size>size()){
					create(begin()+size(),begin()+Size,val);
				}else{
					destroy(begin()+Size,begin()+size());
				};
				sz = Size;
			}else{
				//reallocate
				type * w = allocate(Size);
				cap = Size;
				create_copy(w,w+std::min(Size,size()),begin());
				create(w+size(),w+Size,val);
				destroy();
				v = w;
				sz = Size;
			};
		};

		type & operator [](const int i){
			assert(i>=0 && i<size());
			return v[i];
		};

		const type & operator [](const int i)const{
			assert(i>=0 && i<size());
			return v[i];
		};
		type & back(){return (*this)[size()-1];};
		const type & back()const{return (*this)[size()-1];};
		type & front(){return (*this)[0];};
		const type & front()const{return (*this)[0];};
	};

	//______________________ref_buffer_______________________________
	//! Buffer which references the memory allocated externally
	template<typename type> class ref_buffer : public heap_buffer<type>{
	protected:
	public:
		ref_buffer(){};
		ref_buffer(type *V,int Size){
			this->set_ref(V,Size);
		};
	public:
		//dafault copy constructor
		//default operator = 
		//default destructor
	private: //hide parent methods
		void resize(int Size, const type & val = type());
	};


	//______________________memblock_____________________________
	//! heap self-allocated memblock for dynamic data
	/*!
	Grows its size by powers of 2, which allows minimize expances on reallocation
	for incrementally growing arrays.
	All the reserved elements are constructed.
	*/
	template<class type> class memblock{
	protected:
		type * v;
		int reserved;
	public:
		int capacity()const{return reserved;};
		memblock():v(0),reserved(0){
		};
		memblock(int Size):reserved(Size){
			v = new type[reserved];
		};
		~memblock(){
			if(v)delete[] v;
		};
		memblock(const memblock & x):v(0),reserved(0){
			*this = x;
		};
		void operator = (const memblock & x){
			if(reserved<x.reserved){
				if(v)delete[] v;
				reserved = x.reserved;
				v = new type[reserved];
			};
			memcpy(v,x.v,x.reserved*sizeof(type));
		};
		void reserve(int Size){
			type * V = new type[Size];
			if(v){
				memcpy(V,v,std::min(reserved,Size)*sizeof(type));
				delete[] v;
			};
			v = V;
			reserved = Size;
		};
		void Grow(){
			if(reserved<16){
				reserve(16);
			}else{
				reserve(reserved<<1);
			};
		};
		type & operator [](const int i){
			assert(i>=0 && i<capacity());
			return v[i];
		};
		const type & operator [](const int i)const{
			assert(i>=0 && i<capacity());
			return v[i];
		};
	};

	//_____________________buffer_______________________________
	//! Basic buffer class: allocated on heap, provides add() with automatic growth
	/*!
	Time: O(log(size())) for add() on average.
	Overhead: O(size()).
	All the reserved elements are constructed.
	*/
	template<class type> class buffer : public memblock<type>{
	private:
		typedef memblock<type> parent;
	protected:
		int _end;
	public:
		buffer():_end(0){};
		//default copy constructor;
		//default operator = 
		buffer(int Size, int C = 0):parent(Size),_end(C){};
		~buffer(){};
	public:
		int count()const{return _end;};
		int size()const{return _end;};
		bool empty()const{return size()==0;};
		const type * begin()const{return &(this->v[0]);};
		const type * end()const{return begin()+_end;};
		type * begin(){return &(this->v[0]);};
		type * end(){return begin()+_end;};
	public:
		void setsize(int nsize){
			if(nsize>this->reserved()){
				this->reserve(nsize);
			};
			_end = nsize;
		};
		bool is_full(){
			return _end==this->capacity();
		};
		void add(const type & x){
			if(is_full())this->Grow();
			++_end;
			back()=x;
		};
		void push_back(const type & x){
			add(x);
		};
		void pop_back(){
			if(!empty()){
				--_end;
			}else{
				throw debug_exception("pop from empty buffer occured");
			};
		};

		void clear(){
			_end = 0;
		};
		//redefine to check new range defined by end
		type & operator [](const int i){
			assert(i>=0 && i<size());
			return parent::operator[](i);
		};
		const type & operator [](const int i)const{
			assert(i>=0 && i<size());
			return parent::operator[](i);
		};
		type & back(){return (*this)[size()-1];};
		const type & back()const{return (*this)[size()-1];};
		type & front(){return (*this)[0];};
		const type & front()const{return (*this)[0];};
	};
	//___________________ciclebuffer____________________________
	//!Cicle buffer: equivalent to buffer<type>, differs in internal representation
	/*!
	Elements are allocated in linear order with exception of at most one gap
	All the reserved elements are constructed.
	*/
	template<class type> class ciclebuffer : private buffer<type>{
	private:
		typedef buffer<type> parent;
	protected:
		int beg;
		int & _end(){return parent::_end;};
		int _end()const{return parent::_end;};
		type & unshifted(int i){return this->v[i];};
		const type & unshifted(int i)const{return this->v[i];};
	public:
		int capacity()const{return parent::capacity();};
		ciclebuffer():parent(0),beg(0){};
		ciclebuffer(int Size):parent(Size),beg(0){};
		~ciclebuffer(){};
		int count()const{return this->_end()>=beg?this->_end()-beg:capacity()-beg+this->_end();};
		int size()const{return count();};
		bool is_full(){return beg-this->_end()==1 || this->_end()-beg==this->reserved-1;};
		bool is_empty()const{return beg-this->_end()==0;};
		bool empty()const{return beg-this->_end()==0;};
	public:
		void reserve(int Size){
			type * V = new type[Size];
			if(this->v){
				if(this->_end()>=beg){
					memcpy(V,this->v+beg,(this->_end()-beg)*sizeof(type));
					this->_end() -=beg;
				}else{
					memcpy(V,this->v+beg,(this->reserved-this->beg)*sizeof(type));
					memcpy(V+(this->reserved-beg),this->v,this->_end()*sizeof(type));
					this->_end() = this->reserved-beg+this->_end();
				};
				delete[] this->v;
			};
			beg = 0;
			this->v = V;
			this->reserved = Size;
		}; 
	public:
		void Grow(){
			reserve(this->reserved<<1);
		};
		type & operator [](const int i){
			assert(i>=0 && i<count());
			if(beg+i<capacity())return this->v[beg+i];
			else return this->v[beg+i-capacity()];
		};
		const type & operator [](const int i)const{
			assert(i>=0 && i<count());
			if(beg+i<capacity())return this->v[beg+i];
			else return this->v[beg+i-capacity()];
		};		
		void clear(){
			parent::clear();
			beg = this->_end();
		};
	};
	//_______________________queue______________________________
	//! Basic queue
	/*!
	queue makes efficient use of ciclebuffer, so that enque() and dequeue() are 
	performed in O(1) if the number of elements in the queue is bounded.
	All the reserved elements are constructed.
	*/
	template<class type> class queue : public ciclebuffer<type>{
	private:
		typedef ciclebuffer<type> parent;
	public:
		typedef ciclebuffer<type> tbuffer;
	public:
		queue(int reservedSize):parent(reservedSize){
		};
		queue(){};
		~queue(){};

		void enqueue(const type & x){
			if(this->is_full())this->Grow();
			this->unshifted(this->_end()) = x;
			if(++this->_end()==this->capacity())this->_end() = 0;
		};

		type & back(){
			if(this->is_empty())throw debug_exception("memmanager::queue is empty");
			if(this->_end()==0)return this->unshifted(this->capacity()-1);
			return this->unshifted(this->_end()-1);
		};

		type & dequeue(){
			if(this->is_empty())throw debug_exception("memmanager::queue is empty");
			type & r = this->unshifted(this->beg);
			if(++this->beg==this->capacity())this->beg = 0;
			return r;
		};
	};
	//_____________________staticbuffer_______________________________
	//! memory buffer with compile-time specified size
	/*!
	staticmemblock allocates on the stack sufficient space to store up to reserved entries of type.
	All the reserved elements are constructed.
	*/
	template<class type,int reserved> class staticmemblock{
	private:
		type v[reserved];
	public:
		staticmemblock(){};
		int capacity()const{return reserved;};
		type & operator [](const int i){return v[i];};
		const type & operator [](const int i)const{return v[i];};
	};


	//! staticbuffer is a staticmemblock endowed with element management
	template<class type,int reserved> class staticbuffer: public staticmemblock<type,reserved>{
		int i_end;
	private:
		void drop(int k){
			for(int i=k;i<i_end-1;i++){
				(*this)[i] = (*this)[i+1];
			};
			--i_end;
		};
	public:
		staticbuffer():i_end(0){
		};
		int count()const{return i_end;};
		int size()const{return i_end;};
		int is_full(){
			return count()==this->capacity();
		};
		type * begin(){return &(*this)[0];};
		type * end(){return &(*this)[i_end];};
		bool is_empty(){
			return count()==0;
		};
		void add(const type & x){
			if(is_full()){
				throw debug_exception("buffer cant hold more elements");
			};
			(*this)[i_end++] = x;
		};
		void push_back(const type & x){
			add(x);
		};
		type & back(){
			assert(i_end>0);
			return (*this)[i_end-1];
		};
		void pop_back(){
			assert(i_end>0);
			--i_end;
		};
		void erase(type * from, type * to){
			int k = int(from - begin());
			int l = int(to - from);
			assert(l>=0);
			assert(k>=0 && l+k<=count());
			for(int i=k;i<count()-l;i++)(*this)[i]=(*this)[i+l];
			resize(count()-l);
		};
		void erase(type * el){
		};
		void reserve(int Size){
			if(Size<this->capacity())throw debug_exception("buffer capacity is fixed");
		};
		int contains(const type & x){
			for(int i=0;i<count();i++){
				if((*this)[i] == x)return 1;
			};
			return 0;
		};
		void drop(const type & x){
			for(int i=0;i<count();i++){
				if((*this)[i] == x){
					drop(i);
					break;
				};
			};
		};
		void clear(){
			i_end = 0;
		};
		void resize(int n){
			if(n>this->capacity())throw debug_exception("buffer capacity is less then requested");
			i_end=n;
		};
	};

};
#endif


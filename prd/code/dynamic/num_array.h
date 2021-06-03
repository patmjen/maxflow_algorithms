#ifndef num_array_h
#define num_array_h

#include "exttype/intn.h"
#include "exttype/itern.h"
#include "dynamic/buffer.h"
#include "exttype/pvect.h"

namespace dynamic{
	using namespace exttype;

	//_______________________________num_array______________________________________
	template <class type, int dims> class num_array : public ivector<num_array<type,dims>, dynamic::heap_buffer<type> >{
	public:
		typedef dynamic::heap_buffer<type> buffer;
		typedef ivector<num_array<type,dims>, dynamic::heap_buffer<type> > parent;
		//typedef num_array_tn<mwSize,dims> tindex;
		typedef intn<dims> tindex;
		typedef exttype::itern<dims> titer;
	private:
		tindex CC;
	public:
		//! first index fastest linear index
		int linindex(const tindex & ii)const{
			//this code will optimize out and inline
			int i =	ii[dims-1];
			for(int l =dims-2;l>=0;l--){
				//assert(ii[l]<CC[l]);
				i = i*CC[l]+ii[l];
			};
			return i;
		};
		int byte_size()const{
		    return buffer::size()*sizeof(type);
		};
	protected:
		int linsize(const tindex & sz)const{
			return sz.prod();
		};
	public:
		void set_ref(type * data, tindex Size){
			buffer::set_ref(data,linsize(Size));
			CC = Size;
		};
		void set_ref(const type * data, tindex Size){
			buffer::set_ref(data,linsize(Size));
			CC = Size;
		};
	public:
		int length()const{return  buffer::size();};
		int count()const{return length();};
		bool is_allocated()const{return !buffer::empty();};
		bool is_empty()const{return !is_allocated();};
	public:
		const tindex & size()const{return CC;};
		//! multiindex access operator
		type & operator[](const tindex & ii){
			return buffer::operator[](linindex(ii));
		};
		//! multiindex const access operator
		const type & operator[](const tindex & ii)const{
			return buffer::operator[](linindex(ii));
		};
		//! linear index access operator
		type & operator[](const int i){
			assert(i<length());
			return buffer::operator[](i);
		};
		type & operator()(const int i){
			assert(i<length());
			return buffer::operator[](i);
		};
		//! linear index const access operator
		const type & operator[](const int i)const{
			return buffer::operator[](i);
		};
		type & operator()(const int i)const{
			assert(i<length());
			return buffer::operator[](i);
		};
		type & operator()(int i, int j, int k=0, int l=0, int m=0){
			return (*this)[tindex(i,j,k,l,m)];
		};
		const type & operator()(int i, int j, int k=0, int l=0, int m=0)const{
			return (*this)[tindex(i,j,k,l,m)];
		};
		int stride(int i, int j, int k=0, int l=0, int m=0)const{
			return linindex(tindex(i,j,k,l,m));
		};
		exttype::pvect<type> get_vect(const tindex & ii, int Size){
			return pvect<type>(&(*this[ii]),Size);
		};
		template <int ndims> num_array<type,ndims> subdim(const intn<dims-ndims> & tail_ii)const{
			tindex ii;
			ii<<0;
			for(int k=ndims;k<dims;++k){
				ii[k] = tail_ii[k-ndims];
			};
			num_array<type,ndims> r;
			intn<ndims> sub;
			sub << size();
			r.set_ref(const_cast<type*>(&(*this)[ii]),sub);
			return r;
		};

		num_array<type,dims> subrange(const tindex _cc, const tindex _CC)const{
			tindex sz = _CC-_cc;
			num_array<type,dims> r(sz);
			itern<dims> jj(sz);
			for(range_itern<dims> ii(_cc,_CC);ii.allowed();++ii,++jj){
				r[jj] = (*this)[ii];
			};
			return r;
		};

		template <int ndims> void subassign(const intn<dims-ndims> & tail_ii,const num_array<type,ndims> & A){
			tindex ii;
			ii<<0;
			for(int k=ndims;k<dims;++k){
				ii[k] = tail_ii[k-ndims];
			};
			intn<ndims> sub;
			sub << size();
			for(itern<ndims> head_ii(sub);head_ii.allowed();++head_ii){
				ii<<head_ii;
				(*this)[ii] = A[head_ii];
			};
		};

		void set_zero(){
			memset(begin(),0,(end()-begin())*sizeof(type));
		};

		num_array<type,dims> permute_dims(tindex dd)const{
			tindex psize;
			for(int k=0;k<dims;++k){
				psize[k] = size()[dd[k]];
			};
			num_array<type,dims> r(psize);
			for(titer ii(size());ii.allowed();++ii){
				tindex jj;
				for(int k=0;k<dims;++k){
					jj[k] = ii[dd[k]];
				};
				r[jj] = (*this)[ii];
			};
			return r;
		};

	public:
		//! swap does not use deep copy
		void swap(num_array<type, dims> & x){
			buffer::swap(x);
			//std::swap(p,x.p);
			std::swap(CC,x.CC);
		};
	public:
		//! reshae: creates a reshaped reference
		template <int ndims> num_array<type,ndims> reshape(const intn<ndims> & newSize)const{
			if(size().prod()!=newSize.prod())debug_exception("reshape sizes must match");
			num_array<type,ndims> r;
			r.set_ref(&(*this)[0],newSize);
			return r;
		};
		//! reserve memory
		void reserve(const tindex & newSize){
			if(!parent::owned){
				throw debug_exception("resizing referenced object");
			};
			buffer::reserve(linsize(newSize));
		};

		//! resize: similar to constructor(Size), but reuses the object and memory if possible
		void resize(const tindex & newSize){
			if(CC.prod()==newSize.prod()){
				return;
			};
			if(!parent::owned){
				throw debug_exception("resizing referenced object");
			};
			buffer::resize(linsize(newSize));
			CC = newSize;
			/*
			if(Size==size())return;
			//try allocate and copy to new object
			num_array<type, dims> x(Size);
			for (titer ii(tindex::min(Size,size()));ii.allowed();++ii){
				x[ii] = (*this)[ii];
			};
			// sucessfull, can swap
			swap(x);
			// old content now in x, will be automatically destroyed
			*/
		};
		//! release allocated memory
		void destroy(){
			buffer::destroy();
			CC << 0;
		};
		//constructors:
	public:
		//! construct empty
		num_array(){
			CC << 0;
		};

		//! construct num_array of the given size
		explicit num_array(const tindex & sz):CC(sz){
			buffer::resize(linsize(sz));
		};

		//! full copy for owned and reference copy for referenced
		num_array(const num_array<type,dims> & x){
			if(x.owned){
				*this = x; //copy
			}else{
				buffer::set_ref(x);//make ref
				CC = x.size();
			};
		};

		void set_ref(const num_array<type,dims> & x){
			buffer::set_ref(x);//make ref
			CC = x.size();
		};

		//! make a reference
		num_array ref()const{
			num_array<type,dims> r;
			r.set_ref(&(*this)[0],size());
			return r;
		};

		//! full copy with type conversion
		template<class type2> explicit num_array(const num_array<type2,dims> & x){
			(*this) = x;
		};

		//! full copy operator =
		void operator = (const num_array<type,dims> & x){
			buffer::operator = (x);
			CC = x.CC;
		};
		//! full copy operator= with type conversion
		template<class type2> void operator = (const num_array<type2,dims> & x){
			buffer::operator = (x);
			CC = x.size();
		};

		//default destructor
	public:
		// inherited from buffer:
		// type * begin()
		// type * end()
		// const type * begin()
		// const type * end()
		// type first()
		type& last(){
			return (*this)[size()-1];
		};
	public:
		// inherited math operators from ivector
	public: //more math
		/*
		struct identity{
			type x;
			identity(type y){x=y;};
			operator type&(){return x;};
		};
		*/
		static type identity(type x){return x;};
		void cumsum(type(*foo)(type x)=&identity){
			if(dims==1){
				type * p = &operator()(0);
				(*p) = foo(*p);
				while(++p<end()){
					(*p) = foo(*p);
					*p += *(p-1);
				};
			}else if(dims==2){
				type * p = &operator()(0,0);
				int stride = this->stride(0,1);
				type * next = p+stride;
				//process first row
				(*p) = foo(*p);
				while(++p<next){
					(*p) = foo(*p);
					*p += *(p-1);
				};
				//process subsequent rows
				while(p<end()){
					type * next = p+stride;
					type s_row = 0;
					while(p<next){
						type x = foo(*p);
						*p = x + s_row + *(p-stride);
						s_row += x;
						++p;
					};
				};
			}else if(dims==3){//compute cumsum along dimensions 2 and 3
				type * p = &operator()(0,0,0);
				int stride1 = &operator()(0,1,0)-&operator()(0,0,0);
				int stride2 = &operator()(0,0,1)-&operator()(0,0,0);
				//process first plane
				while(p<begin()+stride1){
					(*p) = foo(*p);
					++p;
				};
				while(p<begin()+stride2){
					(*p) = foo(*p);
					*p += *(p-stride1);
					++p;
				};
				//process subsequent planes
				while(p<end()){
					type * next = p+stride1;
					while(p<next){
						(*p) = foo(*p);
						++p;
					};
					next = p-stride1+stride2;
					while(p<next){
						type x = foo(*p);
						*p = x + *(p-stride1);
						*(p-stride1) += *(p-stride1-stride2);
						++p;
					};
					//last line
					p-=stride1;
					while(p<next){
						*p += *(p-stride2);
						++p;
					};
				};
			}else{
				throw debug_exception("not implemented");
			};
		};
		num_array<type,dims> boxmax(mint2 box){

		};
	};
};

#endif

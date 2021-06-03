#ifndef data_dataset_h
#define data_dataset_h

#include "grid_topology.h"
#include "debug/except.h"
#include "dynamic/buffer.h"
#include "dynamic/fixed_array2.h"
#include "exttype/pvect.h"

namespace dynamic{
	using namespace exttype;

	//________________LinIndex_______________________
	template<bool lastindexfastest=true> class TLinIndex{
	public:
		//! compute linear index
		template<int rank> static int linindex(const intn<rank> & ii, const intn<rank> & CC){
			int i =	ii[0];
			for(int l =1;l<rank;l++){
				assert(ii[l]<CC[l]);
				i = i*CC[l]+ii[l];
			};
			return i;
		};

		//! compute multiindex
		template<int rank> static intn<rank> mindex(int i, const intn<rank> & CC){
			intn<rank> ii;
			for(int l = rank-1;l>0;--l){
				ii[l] = i % CC[l];
				i = i / CC[l];
			};
			ii[0] = i;
			assert(ii[0]<CC[0]);
			return ii;
		};
	};

	template<> class TLinIndex<false>{
	public:
		//! compute linear index
		template<int rank> static int linindex(const intn<rank> & ii, const intn<rank> & CC){
			int i =	ii[rank-1];
			for(int l =rank-2;l>=0;l--){
				assert(ii[l]<CC[l]);
				i = i*CC[l]+ii[l];
			};
			return i;
		};

		//! compute multiindex
		template<int rank> static intn<rank> mindex(int i, const intn<rank> & CC){
			intn<rank> ii;
			for(int l = 0;l<rank-1;++l){
				ii[l] = i % CC[l];
				i = i / CC[l];
			};
			ii[rank-1] = i;
			assert(ii[rank-1]<CC[rank-1]);
			return ii;
		};
	};


	template<int rank, bool lastindexfastest=true> class LinIndex: public geom::DataSetTopology<rank>, TLinIndex<lastindexfastest>{
	public:
		typedef geom::DataSetTopology<rank> parent;
	public:
		using parent::getiterator;

	public:
		LinIndex(){};
		explicit LinIndex(const intn<rank> & cc):parent(cc){
		};
		int linindex(const intn<rank> & ii)const{
			return TLinIndex<lastindexfastest>::linindex(ii,this->CC);
		};
	};

	/*
	template<> class LinIndex<0>{
	public:
	intn<rank> CC;
	public:
	template<int rank> static int linindex(const intn<rank> & ii, const intn<rank> & CC){
	int i =	ii[rank-1];
	for(int l =rank-2;l>=0;l--){
	assert(ii[l]<CC[l]);
	i = i*CC[l]+ii[l];
	};
	return i;
	};
	template<int rank> static intn<rank> mindex(int i, const intn<rank> & CC){
	throw debug_exception("not implemented");
	};
	public:
	LinIndex(){
	CC<<0;
	};
	explicit LinIndex(const int & cc):CC(cc){
	};
	int linindex(const tindex & ii)const{
	return LinIndex<lastindexfastest>::linindex(ii,CC);
	};
	};
	*/

	//______________________________DataSet__________________________________
	template<class type, int rank, bool lastindexfastest = true> 
	class DataSet : public geom::DataSetTopology<rank>{
	protected:
		typedef heap_buffer<type> tBuffer;
		tBuffer buffer;
	public:
		typedef geom::DataSetTopology<rank> topology;
		typedef typename topology::tindex tindex;
		typedef typename geom::DataSetTopology<rank>::titerator titerator;
	public:
		using topology::getiterator;
	protected:
		int linindex(const tindex & ii)const{
			return TLinIndex<lastindexfastest>::linindex(ii,this->size());
		};
	public:
		//const int count(int i=0)const{return topology::count(i);};
		//const tindex size()const{return this->CC;};
	public:
		tBuffer & getBuffer(){return buffer;};
		//int getBufferSize(){return parent::getBufferSize();};
	public:
		DataSet(){
		};

		DataSet(const DataSet & x):topology(x),buffer(x.buffer){
		};

		void operator = (const DataSet & x){
			if(!x.buffer.owned){
				buffer.set_ref(x.buffer);
			}else{
				buffer = x.buffer;
			};
			this->CC = x.size();
		};

		explicit DataSet(const tindex & CC, const type & v = type()):topology(CC){
			setSize(CC,v);
		};

		DataSet(const tindex & CC, type * v):topology(CC), buffer(v,topology::linsize(CC)){
		};

		void setSize(const tindex & CC, const type & v = type()){
			topology::setSize(CC);
			buffer.resize(topology::linsize(CC),v);
		};

		void resize(const tindex & CC, const type & v = type()){
			setSize(CC,v);
		};

		type & operator[](const tindex & ii){
			int i = linindex(ii);
			//assert(i < (int)buffer.size());
			return buffer[i];
		};

		const type & operator[](const tindex & ii)const{
			int i = linindex(ii);
			//assert(i < (int)buffer.size());
			return buffer[i];
		};

		type * get_ptr(const tindex & ii){
			int i = linindex(ii);
			return &buffer[i];
		};

		const type * get_ptr(const tindex & ii)const{
			int i = linindex(ii);
			return &buffer[i];
		};

		pvect<type> get_range(const tindex & ii, int n){
			int i = linindex(ii);
			if(i+n>buffer.size())throw debug_exception("get_range: the data attempted to access is out of range");
			return pvect<type>(&buffer[i],n);
		};

		pvect<type> get_pvect(const tindex & ii){
			if(lastindexfastest){ 
				if(ii[rank-1]!=0)throw debug_exception("invalid Dataset::get_pvect access");
				return pvect<type>(&(*this)[ii],this->size()[rank-1]);
			}else{
				if(ii[0]!=0)throw debug_exception("invalid Dataset::get_pvect access");
				return pvect<type>(&(*this)[ii],this->size()[0]);
			};
		};

		const pvect<type> get_range(const tindex & ii, int n)const{
			int i = linindex(ii);
			if(i+n>=buffer.size())throw debug_exception("out of range");
			return pvect<type>(&buffer[i],n);
		};


		template<bool lastindexfastest2> 
		void operator << (const DataSet<type,rank,lastindexfastest2> & x){
			resize(x.size());
			for(titerator ii=getiterator();ii.allowed();++ii){
				(*this)[ii] = x[ii];
			};
		};

		template<typename type2, bool lastindexfastest2> 
		void operator << (const DataSet<type2,rank,lastindexfastest2> & x){
			resize(x.size());
			for(titerator ii=getiterator();ii.allowed();++ii){
				(*this)[ii] = (type)x[ii];
			};
		};

		void operator << (const type & x){
			for(titerator ii=getiterator();ii.allowed();++ii){
				(*this)[ii] = x;
			};
		};

	};

	template<class type, class al1, bool lastindexfastest> fixed_array2<type,al1> & operator <<(fixed_array2<type,al1> & dest, const DataSet<type,2,lastindexfastest> & src){
		dest.resize(src.size());
		for(iter2 ii(src.size());ii.allowed();++ii){
			dest[ii] = src[ii];
		};
		return dest;
	}; 
	template<class type, class al1, bool lastindexfastest> DataSet<type,2,lastindexfastest> & operator <<(DataSet<type,2,lastindexfastest> & dest, const fixed_array2<type,al1> & src){
		dest.resize(src.size());
		for(iter2 ii(src.size());ii.allowed();++ii){
			dest[ii] = src[ii];
		};
		return dest;
	};

	template<class type, class al1, bool lastindexfastest> fixed_array1<type,al1> & operator <<(fixed_array1<type,al1> & dest, const DataSet<type,1,lastindexfastest> & src){
		dest.resize(src.size());
		for(iter1 ii(src.size());ii.allowed();++ii){
			dest[ii] = src[ii];
		};
		return dest;
	}; 
	template<class type, class al1, bool lastindexfastest> DataSet<type,1,lastindexfastest> & operator <<(DataSet<type,1,lastindexfastest> & dest, const fixed_array1<type,al1> & src){
		dest.resize(src.size());
		for(iter1 ii(src.size());ii.allowed();++ii){
			dest[ii] = src[ii];
		};
		return dest;
	};
};

#endif

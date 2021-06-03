#ifndef itern_h
#define itern_h

#include "intn.h"

namespace exttype{
	//! n-dimentional iterator.
	template <int n> class itern: public intn<n>{
	private:
		typedef intn<n> parent;
	public:
		typedef intn<n> tindex;
		typedef itern<n> titerator;
	private:
		tindex cc;
	public:
		int count(int i=0)const{return cc[i];};
		const tindex & range()const{return cc;};
	public:
		itern(){
			cc << 0;
			resetindex();
		};
		void set_range(const tindex& CC){
			cc << CC;
		};
		itern(const titerator & x):cc(x.cc){
			resetindex();
		};
		itern(const tindex& CC):cc(CC){
			resetindex();
		};
	private:
		template<int i, int start> bool p_iterate_skip_i(){
			if(start<0)return false;
			if(start==i)return p_iterate_skip_i<i,(start-1>=0?start-1:-1)>();
			if(++(*this)[start]<count(start))return true; 
			(*this)[start] = 0;
			return p_iterate_skip_i<i,(start-1>=0?start-1:-1)>();
		};
		template<int start> bool p_iterate_skip_i(int i){
			if(start<0)return false;
			if(start==i)return p_iterate_skip_i<(start-1>=0?start-1:-1)>(i);
			if(++(*this)[start]<count(start))return true; 
			(*this)[start] = 0;
			return p_iterate_skip_i<(start-1>=0?start-1:-1)>(i);
		};

	public:
		inline bool allowed()const{
			for(int j=0;j<n;j++)if((*this)[j]>=count(j)) return false; 
			return true;
		};

		inline bool caniterate()const{
			for(int j=0;j<n;j++)if((*this)[j]<count(j)-1) return true;
			return false;
		};

		itern& operator ++(){
			iterate();
			return *this;
		};

		inline bool iterate(){
			for(int j=n-1;j>=0;j--)if(++(*this)[j]<count(j)) return true; else if(j>0)(*this)[j]=0;
			return false;
		};

		template<int i> bool iterate_i(){
			if(++(*this)[i]<count(i))return true;
			(*this)[i]=0;
			return false;
		};

		template<int i> bool iterate_skip_i(){
			return p_iterate_skip_i<i,n-1>();
		};

		bool iterate_skip_i(int i){
			return p_iterate_skip_i<n-1>(i);
		};

		bool iterate_i(int i){
			if(++(*this)[i]<count(i))return true;
			(*this)[i]=0;
			return false;
		};

		void resetindex(){
			(*this) << 0;
		};
	};

	//______________________________________________________________________
	//! n-dimentional iterator with nonzero lower bounds.
	template <int n> class range_itern: public itern<n>{
	private:
		typedef itern<n> parent;
		typedef intn<n> tindex;
	private:
		tindex cc0;
	public:
		using itern<n>::count;
	public:
		range_itern(const tindex & c,const tindex & C):parent(C),cc0(c){
			(*this) << cc0;
		};

		/*
		range_itern(const intn<n+n> & cC):parent(cC.rsub<n>()),cc0(cC.sub<n>()){
			(*this) << cc0;
		};
		*/

		inline bool allowed()const{
			for(int j=0;j<n;j++)if((*this)[j]>=count(j))return false;
			return true;
		};

		range_itern& operator ++(){
			iterate();
			return *this;
		};

		inline int iterate(){
			for(int j=n-1;j>=0;j--)if(++(*this)[j]<count(j)) return true; else if(j>0)(*this)[j]=cc0[j];
			return false;
		};

	private:
		template<int start> bool p_iterate_skip_i(int i){
			if(start<0)return false;
			if(start==i)return p_iterate_skip_i<(start-1>=0?start-1:-1)>(i);
			if(++(*this)[start]<count(start))return true; 
			(*this)[start] = this->cc0[start];
			return p_iterate_skip_i<(start-1>=0?start-1:-1)>(i);
		};

	public:
		template<int i> bool iterate_i(){
			if(++(*this)[i]<count(i))return true;
			(*this)[i]=this->cco[i];
			return false;
		};

		bool iterate_skip_i(int i){
			return p_iterate_skip_i<n-1>(i);
		};

		bool iterate_i(int i){
			if(++(*this)[i]<count(i))return true;
			(*this)[i]=cc0[i];
			return false;
		};

		void resetindex(){
			(*this) << cc0;
		};
	};

	//________________shortcuts______________________
	typedef itern<1> iter1;
	typedef itern<2> iter2;
	typedef itern<3> iter3;
	typedef itern<4> iter4;
	typedef range_itern<1> range_iter1;
	typedef range_itern<2> range_iter2;
	typedef range_itern<3> range_iter3;
	typedef range_itern<4> range_iter4;
};
#endif

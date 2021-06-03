#ifndef maxplus_indexset_h
#define maxplus_indexset_h

#include <vector>
#include "debug/except.h"
#include <algorithm>
#include "debug/except.h"

namespace maxplus{

	template<class type> class subset;
	template<class type> class element;
	template<class type_x, class type_y> class mapping;

	//______________________indexptr____________________
	//! A type* implementation.
	template<class type> class indexptr{
	private:
		typedef indexptr<type> tptr;
	public:
		int i;
		subset<type> * base;
	private:
		friend class subset<type>;
	public:
		indexptr(subset<type> * S, int index):base(S),i(index){};
	public:
		static const indexptr<type> null;
	public:
		indexptr():base(0),i(-1){};
	public:
		bool operator == (const indexptr& p)const{
			return (base == p.base && i==p.i);
		};

		bool operator != (const indexptr& p)const{
			return !operator==(p);
		};

		bool isnull()const{
			return (base==0);
		};

		bool operator < (const indexptr& p)const{
			if(base!=p.base)throw debug_exception("different base");
			return i<p.i;
		};

		tptr & operator ++(){
			++i;
			return *this;
		};

		bool operator >= (const indexptr& p)const{
			if(base!=p.base)throw debug_exception("different base");
			return i>=p.i;
		};

		tptr & operator --(){
			--i;
			return *this;
		};

		type * operator ->(){
			return &(*base)[i];
		};

		const type * operator ->()const{
			return &(*base)[i];
		};

		type & getObj(){
			if(base) return (*base)[i];
			else throw debug_exception("null indexptr dereference");
		};

		const type & getObj()const{
			if(base)return (*base)[i];
			else throw debug_exception("null indexptr dereference");
		};
	};

	template<class type> const indexptr<type> indexptr<type>::null;

	//! Set element
	//______________________element_____________________
	template<class type> class element{
	private:
		friend class subset<type>;
		std::vector<indexptr<type> > tokens;
	private:
		void register_index(const indexptr<type> & p){
			tokens.push_back(p);
		};
		const int where_token(const subset<type> & S)const{
			for(int i=0;i<(int)tokens.size();++i){
				if(tokens[i].base == &S)return i;
			};
			throw debug_exception("Set not found");
		};
		void drop_index(const subset<type> & S){
			tokens.erase(tokens.begin()+where_token(S));
		};
	public:
		const indexptr<type> & find_index(const subset<type> & S)const{
			for(int i=0;i<(int)tokens.size();++i){
				if(tokens[i].base == &S)return tokens[i];
			};
			return indexptr<type>::null;
		};
		const int index_in(const subset<type> & S)const{//-1 if not found
			return find_index(S).i;
		};

	public:
		element(){
			static_cast<type&>(*this);//must be ancestor of class type
		};
	public:
		const indexptr<type> & ptr()const{
			return tokens[0];
		};

		indexptr<type> & ptr(){
			return tokens[0];
		};
	};

	//______________________subset________________________
	//! Subset of elements of type
	template<class type> class subset{
	public:
		typedef type telement;
	protected:
		int n;
		indexptr<type> _begin;
		indexptr<type> _end;
	protected:
		mutable std::vector<type* > v;
	protected:
		void setN(int new_n){
			n = new_n;
			_end = indexptr<type>(this,n);
		};
	public:
		subset(){
			_begin = indexptr<type>(this,0);
			setN(0);
		};

		subset(const subset<type> & X){
			_begin = indexptr<type>(this,0);
			setN(0);
			operator = (X);
		};

		void operator = (const subset<type> & X){
			clear();
			(*this)|=(X);
		};

		void clear(){
			for(int i=0;i<N();++i){
				(*this)[i].drop_index(*this);
			};
			v.resize(0);
			setN(0);
		};

		~subset(){
			clear();
		};

		//! union of type A = A union B. Runs in O((|A|+|B|)*K)
		subset<type> & operator |=(const subset<type> & X){
			v.resize(N()+X.N());
			for(int i=0;i<X.N();++i){
				v[N()+i] = &X[i];
				X[i].register_index(indexptr<type>(this,N()+i));
			};
			setN(N()+X.N());
			return *this;
		};

		//! intersection of type A = A intersection B. Runs in O((|A|+|B|)*K)
		subset<type> & operator &=(const subset<type> & X){
			std::vector<type*> live;
			std::vector<type*> dead;
			//mark.resize(N());
			for(int i=0;i<X.N();++i){
				if(contains(X[i]))live.push_back(&X[i]);
				else dead.push_back(&X[i]);
			};

			for(int i=0;i<(int)dead.size();++i){
				dead[i].drop_index(*this);
			};
			v = live;
			setN(v.size());
			return *this;
		};

		//! union: A = B \cup C, O((|B|+|C|)*K)
		subset<type> operator |(const subset<type> & X)const{
			return subset<type>(*this)|=(X);
		};

		//! intersection: A = B \cap C, O((|B|+|C|)*K)
		subset<type> operator &(const subset<type> & X)const{
			return subset<type>(*this)&=(X);
		};

	public:
		void add(type & x){
			static_cast<element<type>&>(x); // type must inherit from element<type>
			x.register_index(indexptr<type>(this,(int)v.size()));
			v.push_back(&x);
			setN(n+1);
		};
		void reserve(int m){
			v.reserve(m);
		};
		const int capacity()const{
			return (int)v.capacity();
		};
	protected:
		void drop(int i){
			(*this)[i].drop_index(*this);
			v.erase(v.begin()+i);
			setN(n-1);
		};
	public:
		bool contains(const element<type> & x)const{
			return !(x.find_index(*this).isnull());
		};
	public:
		int N()const{return n;};
		int size()const{return n;};
	public:
		const indexptr<type> & begin()const{
			return _begin;
		};
		const indexptr<type> & end()const{
			return _end;
		};
	public:
		type & operator[](int i)const{
			return *v[i];
		};

		type & operator[](indexptr<type> & p)const{
			if(p.base==this)return p.getObj();
			if(p->find_index(*this).isnull())throw debug_exception("Not an element of this subset");
			return p.getObj();
		};

	public:
		virtual void reorder(const mapping<type,int> & order){
			std::vector<type*> nv;
			nv.resize(v.size());
			for(indexptr<type> i=begin();i<end();++i){
				nv[i.i] = v[order[i]];
			};
			swap(v,nv);
		};
	};

	//______________________set________________________
	//! implementation of subset<type>, which allocates and deallocates elements
	template<class type> class set : public subset<type>{
	private:
		typedef subset<type> parent;
	public:
		using parent::N;
		using parent::setN;
	private:
		mutable std::vector<type > v;
	public:
		void reserve(int m){
			v.reserve(m);
			parent::reserve(m);
		};
		const int capacity()const{
			return (int)v.capacity();
		};

		void resize(int m){
			for(int i=N()-1;i>=m;--i){
				parent::drop(i);
			};
			v.resize(m);
			if(parent::capacity()<m)parent::reserve(m);
			for(int i=N();i<m;++i){
				parent::add(v[i]);
			};
		};
		set(){};
		~set(){
			parent::v.clear();
			setN(0);
		};
	private:
		//!forbidden
		void add(type & x);
		set(const set & X){};
		void operator = (const set & X){};
	public:
		type & operator[](int i)const{
			return v[i];
		};
		type & operator[](indexptr<type> & p)const{
			return parent::operator[](p);
		};
	public:
		virtual void reorder(const mapping<type,int> & order){
			throw debug_exception("not implemented. requires element reallocation");
		};
	};

	//__________________mapping_______________________
	//! mapping from subset<type_x> to type_y
	template<class type_x, class type_y> class mapping{
	private:
		//_geom::memmanager::buffer<type_y> v;
		std::vector<type_y> v;//, memmanager::stack_allocator<type_y> > v;
		const subset<type_x> * X;
	public:
		mapping(){};
		mapping(const subset<type_x> & baseset, const type_y & Val = type_y()){
			init(baseset,Val);
		};
		//default copy constructor;
		//default operator =
		/*
		void operator = (const mapping<type_x,type_y> & x){
			v = x.v;
			X = x.X;
			val = x.val;
		};
		*/
		void operator = (const type_y & z){
			for(int i=0;i<(int)v.size();++i)v[i]=z;
		};
	public:
		const subset<type_x> & base()const{return *X;};
		void init(const subset<type_x> & baseset, const type_y & Val = type_y()){
			X = &baseset;
			v.resize(X->N(),Val);
		};
		type_y & operator [] (const indexptr<type_x> & p){
			if(p.base == X)return v[p.i];
			else return (*this)[p.getObj()];
		};

		const type_y & operator [] (const indexptr<type_x> & p)const{
			if(p.base == X)return v[p.i];
			else return (*this)[p.getObj()];
		};

		type_y & operator [] (const type_x & x){
			const indexptr<type_x> & pp = x.find_index(*X);
			if(pp.isnull())throw debug_exception("Not an element of this mapping");
			return v[pp.i];
		};

		const type_y & operator [] (const type_x & x)const{
			const indexptr<type_x> & pp = x.find_index(*X);
			if(pp.isnull())throw debug_exception("Not an element of this mapping");
			return v[pp.i];
		};
	};
};

#endif

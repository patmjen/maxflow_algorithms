#ifndef data_arrays_h
#define data_arrays_h

#include "arrayn.h"
#include "itern.h"

namespace exttype{
	//_______________________________fixedlist____________________________
	template <class type,int n> class fixedlist : public array_tn<type,n>{
	private:
		typedef array_tn<type,n> parent;
	public:
		typedef fixedlist<type,n> tarray;
	public:
		fixedlist(){};
		fixedlist(const type & v0){
			for(int i=0;i<this->size();++i)(*this)[i] = v0;
		};
		fixedlist(const tarray & x):parent(x){};
		template<typename type2> explicit fixedlist(const fixedlist<type2,n> & x){
			(*this) << x;
		};
		fixedlist(const type & v0, const type & v1,const type & v2=type(), const type & v3=type(), const type & v4=type()){
			this->set_elements(v0,v1,v2,v3,v4);
		};
		tarray & operator=(const tarray & x){
			(*this) << x;
			return *this;
		};
	public:
		type & item(int i){return (*this)[i];};
		const type & item(int i)const{return (*this)[i];};
	};

	//_____________________________array_mn_________________________________
	//! Const size 2d array
	template <class type,int c0,int c1, template<class ttype, int tc1> class ttvect=fixedlist> class array_mn:public fixedlist<ttvect<type,c1>,c0>{
	private:
		typedef fixedlist<ttvect<type,c1>,c0> parent;
	public:
		typedef ttvect<type,c1> tvect;
		typedef itern<2> titerator;
		typedef intn<2> tindex;
		typedef array_mn<type,c0,c1,ttvect> tarray;
	public:
		static int count(int i=0){return i==0?c0:c1;};
		mint2 size()const{return mint2(c0,c1);};
	public:
		titerator getiterator()const{
			return tindex(mint2(count(0),count(1)));
		};
		tvect & operator[](int i){return parent::operator[](i);};
		const tvect & operator[](int i)const{return parent::operator[](i);};
		type & operator[](const tindex & ii){return parent::operator[](ii[0])[ii[1]];};
		const type & operator[](const tindex & ii)const{return parent::operator[](ii[0])[ii[1]];};
	public:
		array_mn(){};
		array_mn(const type & val):parent(tvect(val)){};
		//default copy constructor
		//default operator=
	};

	//___________________________________fixedlist2________________________________________
	template <class sublist,class type,int c0> class fixedlist2:public fixedlist<sublist,c0>{
	private:
		typedef fixedlist<sublist,c0> parent;
	public:
		typedef type telement;
		typedef sublist tvect;
		typedef itern<2> titerator;
		typedef intn<2> tindex;
		typedef fixedlist2<sublist,type,c0> tarray;
		titerator get_iterator()const{return titerator(size());};
	public:
		int count(int i=0)const{
			if(i==0){
				return c0;
			}else{
				return 1;
				//return sublist::count();
			};
		};
		mint2 size()const{return mint2(count(0),count(1));};
	public:
		titerator getiterator()const{
			return tindex(mint2(count(0),count(1)));
		};

		tvect & operator[](int i){return parent::operator[](i);};
		const tvect & operator[](int i)const{return parent::operator[](i);};
		type & operator[](const tindex & ii){return parent::operator[](ii[0])[ii[1]];};
		const type & operator[](const tindex & ii)const{return parent::operator[](ii[0])[ii[1]];};
	public:
		fixedlist2(){};
		fixedlist2(const fixedlist2 &x):parent(x){}; //copy
		void operator=(const fixedlist2 & x){parent::operator=(x);};
	};


	//_______________________pair__________________________
	template<class type> class pair:public fixedlist<type,2>{
	public:
		typedef fixedlist<type,2> parent;
	public:
		pair(){};	
		pair(const type & p1, const type & p2):parent(p1,p2){};
	public:
		//default copy constructor
		//default operator =
	};


};

#endif


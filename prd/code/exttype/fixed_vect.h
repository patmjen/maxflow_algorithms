#ifndef fixed_vect_h
#define fixed_vect_h

#include "dynamic/fixed_array1.h"
#include "ivector.h"

namespace exttype{
	template <typename type> class fixed_vect : public ivector<fixed_vect<type>, dynamic::fixed_array1<type, dynamic::array_allocator<type> > >{
	private:
		typedef ivector<fixed_vect, dynamic::fixed_array1<type, dynamic::array_allocator<type> > > parent;
	public:
		int count()const{return this->size();};
		fixed_vect(){};
		explicit fixed_vect(int K){// constructor with initialization
			this->resize(K);
			for(int i=0;i<this->size();++i)(*this)[i] = 0;
		};
		explicit fixed_vect(int n, const type v0,...){
			this->resize(n);
			(*this)[0] = v0;
			va_list a;
			va_start(a,v0);
			for(int i=1;i<n;++i){
				(*this)[i] = va_arg(a,type);
			};
			va_end(a);
		};
		//default operator =
		//default copy constructor
	private:
		fixed_vect cpy()const{return fixed_vect(*this);};
	public:
	};

	typedef fixed_vect<char> charf;
	typedef fixed_vect<int> intf;
	typedef fixed_vect<double> doublef;
	typedef fixed_vect<float> floatf;

};

#endif

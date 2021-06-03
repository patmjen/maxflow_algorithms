#ifndef geom_hvect_h
#define geom_hvect_h

#include "geom/vectn.h"

namespace geom{
	namespace homography{
				//_________________hvectn________________________
		template<int pn> class hvectn : public vectn<pn+1>{
		private:
			typedef vectn<pn+1> parent;
		public:
			using parent::count;
			typedef typename parent::titerator titerator;
		public:
			hvectn(){(*this)[this->count()-1] = 1.0;};
			hvectn(const hvectn & x):parent(x){};
			hvectn(const vectn<pn+1> & x):parent(x){};
			hvectn(const vectn<pn> & x){
				(*this) << x;
				(*this)[this->count()-1] = 1.0;
			};
			hvectn(const double x0, const double x1=0, const double x2=0):parent(x0,x1,x2){
				(*this)[count()-1] = 1.0;
			};
			operator const vectn<pn>()const{
				vectn<pn> r;
				r << (*this);
				r/=(*this)[count()-1];
				return r;
			};
			hvectn<pn> & normalize(){
				(*this)/=(*this)[count()-1];
				return *this;
			};
		};

		template<> class hvectn<1> : public vectn<2>{
		private:
			typedef vectn<2> parent;
		public:
			hvectn(){};
			hvectn(const hvectn & x):parent(x){};
			hvectn(const double & x):parent(x,1){};
			operator double()const{
				return (*this)[0]/(*this)[1];
			};
		};

		typedef hvectn<1> hvect1;
		typedef hvectn<2> hvect2;
		typedef hvectn<3> hvect3;
	};
};

#endif


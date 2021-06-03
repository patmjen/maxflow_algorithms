#ifndef geom_convexpoly_h
#define geom_convexpoly_h

#include "vect2.h"
#include "r2geometry.h"
#include "geom/Zn.h"
#include "exttype/arrays.h"
#include <vector>

namespace geom{
	using namespace exttype;
	namespace R2{
		template<int n, template<class type,int n> class fixed_container = fixedlist> class convexpoly : public fixed_container<vect2,n>{
		private:
			typedef fixed_container<vect2,n> parent;
			using parent::item;
		public:
			convexpoly(){};
			convexpoly(const vect2 & v1,const vect2 & v2,const vect2 & v3,const vect2 & v4=vect2()):parent(v1,v2,v3,v4){};
			int HitTest(const vect2 & p)const{//CCW+ hit test
				for(Zniterator<n> i;i.allowed();++i){
					if( ((p-item(i))^(item(i+1)-item(i))) < 0)return 0;
				};
				return 1;
			};
			int orientation()const{//CCW+ orientation
				return math::sgn((item(2)-item(1))^(item(1)-item(0)));
			};
			Rect getBoundRect()const{
				Rect r(item(0),item(1));
				for(Zniterator<n> i;i.allowed();++i){
					r = r.coverage(Rect(item(i),item(i+1)));
				};
				return r;
			};
			convexpoly<3,fixed_container> getTriangle(Zn<n> i)const{
				convexpoly<3,fixed_container> r;
				for(int j=0;j<3;++j,++i){
					r[j] = (*this)[i];
				};
				return r;
			};
		};

		typedef convexpoly<3> poly3;
		typedef convexpoly<4> poly4;
		
		template<int n> class polyvector : public std::vector<convexpoly<n> >{
		public:
			typedef std::vector<convexpoly<n> > parent;
			using parent::size;
		public:
			int hitTest(const vect2 & p)const{
				for(int i=0;i<this->size();i++)if((*this)[i].HitTest(p))return 1;
				return 0;
			};
			Rect getBoundRect()const{
				if(size()==0)return Rect();
				Rect r = (*this)[0].getBoundRect();
				for(int i=1;i<(int)this->size();i++){
					r = r.coverage((*this)[i].getBoundRect());
				};
				return r;
			};
		};

		typedef polyvector<3> poly3vector;
		typedef polyvector<4> poly4vector;
	};
};

#endif

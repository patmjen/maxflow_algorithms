#ifndef geom_r3geometry_flatpoly_h
#define geom_r3geometry_flatpoly_h

#include "vect3.h"
#include "geom/Zn.h"
#include "exttype/arrays.h"

namespace geom{
	namespace R3{
		template<int n,template<class type,int c> class fixed_container> class flatpoly : public fixed_container<vect3,n>{
		private:
			typedef fixed_container<vect3,n> parent;
		public:
			using parent::item;
		public:
			flatpoly(){};
			flatpoly(const vect3 & v1,const vect3 & v2,const vect3 & v3,const vect3 & v4=vect3()):parent(v1,v2,v3,v4){};
			vect3 orientation()const{//surface normal
				return (item(2)-item(1))^(item(1)-item(0));
			};
			flatpoly<3,fixed_container> getTriangle(Zn<n> i)const{
				flatpoly<3,fixed_container> r;
				for(int j=0;j<3;++j,++i){
					r[j] = (*this)[i];
				};
				return r;
			};
		};

		typedef flatpoly<3,fixedlist> flatpoly3;
		typedef flatpoly<4,fixedlist> flatpoly4;

		typedef std::vector<flatpoly3> flatpoly3vector;
		typedef std::vector<flatpoly4> flatpoly4vector;
	};
};
#endif

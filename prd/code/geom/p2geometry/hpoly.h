#ifndef geom_p2geometry_hpoly_h
#define geom_p2geometry_hpoly_h

#include "geom/transform/homography/homography.h"
#include "exttype/arrays.h"

namespace geom{
	namespace P2{
		template<int n> class hpolyn : public fixedlist<homography::hvect3,n>{
		};
		typedef hpolyn<3> hpoly3;
		typedef hpolyn<4> hpoly4;
	};
};

#endif

#include "vect3.h"

namespace geom{
	//_____________________________vectn<3>___________________________________________
	vectn<3> vectn<3>::rotate0(double sf,double cf){
		vectn<3> r;
		r[0]=(*this)[0];
		r[1]=(*this)[1]*cf-(*this)[2]*sf;
		r[2]=(*this)[2]*cf+(*this)[1]*sf;
		return r;
	};

	vectn<3> vectn<3>::rotate1(double sf,double cf){
		vectn<3> r;
		r[1]=(*this)[1];
		r[2]=(*this)[2]*cf-(*this)[0]*sf;
		r[0]=(*this)[0]*cf+(*this)[2]*sf;
		return r;
	};

	vectn<3> vectn<3>::rotate2(double sf,double cf){
		vectn<3> r;
		r[2]=(*this)[2];
		r[0]=(*this)[0]*cf-(*this)[1]*sf;
		r[1]=(*this)[1]*cf+(*this)[0]*sf;
		return r;
	};

	vectn<3> vectn<3>::operator ^(const vectn<3> & x2)const{
		vect3 r;
		r[0]=item(1)*x2.item(2)-item(2)*x2.item(1);
		r[1]=item(2)*x2.item(0)-item(0)*x2.item(2);
		r[2]=item(0)*x2.item(1)-item(1)*x2.item(0);
		return r;
	};
};

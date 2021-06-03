#include "r3geometry.h"
#include "../r2geometry/r2geometry.h"

namespace geom{
	namespace R3{
		//__________________Line__________________________
		Line::Line(const vect3 & p, const vect3 & v){
			this->p = p;
			this->v = v;
			this->v.norm();
		};
		double Line::distto(const vect3& x)const{
			return ((x-p)-v*((x-p)*v)).l();
		};
		//__________________Plane_________________________
		Plane::Plane(const vect3 & p, const vect3 & n){
			this->p = p;
			this->n = n;
		};
		Plane::Plane(const vect3 & p1, const vect3 & p2, const vect3 & p3){
			this->p = p1;
			this->n = ((p3-p2)^(p2-p1)).norm();
		};
		vect3 Plane::intersection(const Line & line)const{
			double d = line.v*n;
			vect3 x;
			x = p;
			if(d==0)return x;//throwException("no intersection");
			double alpha = ((p-line.p)*n)/d;
			return line.p+line.v*alpha;
		};
		double D2(vect3 & x, Line & line){
			return (x-(line.p+line.v*((x-line.p)*line.v))).l2();
		};
		//_________________Box______________________________
		Box::Box(const vect3 & P0,const vect3 & P1):P(P0,P1){
			center = (P[0]+P[1])/2;
			d = (P[1]-P[0]).l();
		};

		R1::Interval Box::intersec(const Line & line)const{
			R1::Interval I(-1e10,1e10);
			if(line.distto(center)>d/2)return R1::Interval::empty;
			for(int k=0;k<3;k++){
				if(math::abs(line.v[k])<eps0){
					if(line.p[k]<P[0][k] || line.p[k]>P[1][k])return R1::Interval::empty;
				}else{
					I = I.intersect(R1::Interval::order((P[0][k]-line.p[k])/line.v[k],(P[1][k]-line.p[k])/line.v[k]));
					if(I.isEmpty())return R1::Interval::empty;
				};
			};
			return I;
		};

		vect3 Box::getPoint(const int3 & ii)const{
			vect3 r;
			for(int k=0;k<3;k++)r[k] = P[ii[k]][k];
			return r;
		};
		/*
		ownobjectlist<Triangle> Box::getSurface()const{
			ownobjectlist<Triangle> r;
			for(faceiterator ii;ii.allowed();++ii){
				r.add(new Triangle(getPoint(ii.getPoint(int2(0,0))),getPoint(ii.getPoint(int2(1,0))),getPoint(ii.getPoint(int2(1,1)))));
				r.add(new Triangle(getPoint(ii.getPoint(int2(0,0))),getPoint(ii.getPoint(int2(1,1))),getPoint(ii.getPoint(int2(0,1)))));
			};
			return r;
		};
		*/
		flatpoly4vector Box::getSurface()const{
			flatpoly4vector r;
			for(faceiterator ii;ii.allowed();++ii){
				flatpoly4 poly;
				for(R2::Box::CCWiterator jj;jj.allowed();++jj){
					poly[jj.i] = (getPoint(ii.getPoint(jj)));
				};
				r.push_back(poly);
			};
			return r;
		};
	};
};

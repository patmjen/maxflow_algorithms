#ifndef geom_r3geometry_h
#define geom_r3geometry_h

#include "vect3.h"
#include "geom/r1geometry/r1geometry.h"
#include "geom/geom.h"
#include "flatpoly.h"
#include "exttype/itern.h"

namespace geom{
	namespace R3{
		//__________________Line__________________________
		class Line{
			public:
			vect3 p;
			vect3 v;
			friend class Plane;
			public:
			Line(const vect3 & p, const vect3 & v);
			double distto(const vect3& p)const;
		};
		//__________________Plane_________________________
		class Plane{
			private:
			vect3 p;
			vect3 n;
			friend class Line;
			public:
			Plane(const vect3 & p, const vect3 & n);
			Plane(const vect3 & p1, const vect3 & p2, const vect3 & p3);
			vect3 intersection(const Line & line)const;
		};
		//___________________CubeTopology_________________
		class CubeTopology{
		public:
			typedef CubeTopology topology;
			typedef mint3 tindx;
		public:
			CubeTopology(){};
		public:
			//___________vertexiterator____________
			class vertexiterator : public itern<3>{
			private:
				typedef itern<3> parent;
			public:
				vertexiterator(){setcount(int3(2,2,2));};
			};
			//____________edgeiterator_____________
			class edgeiterator : public itern<3>{
			private:
				typedef itern<3> parent;
				typedef parent::tindex tindex;
			public:
				edgeiterator(){setcount(int3(3,2,2));};
				tindex getPoint(int i)const{
					const tindex & ii = *this;
					tindex r(0,0,0);
					r[ii[0]] = i;
					r[(ii[0]+1)%3] = ii[1];
					r[(ii[0]+2)%3] = ii[2];
					return r;
				};
			};
			//____________edgeiterator_____________
			class faceiterator : public itern<2>{
			private:
				typedef itern<2> parent;
				typedef intn<2> tindex;
			public:
				faceiterator(){setcount(int2(3,2));};
				tindex getPoint(const int2 & i)const{
					const parent::tindex & ii = *this;
					tindex r(0,0,0);
					r[ii[0]] = ii[1];
					r[(ii[0]+1)%3] = i[ii[1]];
					r[(ii[0]+2)%3] = i[!ii[1]];
					return r;
				};
			};
		};
		//___________________Triangle_____________________
		class Triangle : public fixedlist2<vect3,double,3>{
		private:
			typedef fixedlist2<vect3,double,3> parent;
		public:
			Triangle(){};
			Triangle(const parent & x):parent(x){};
			Triangle(const vect3 &v1, const vect3 & v2,const vect3 &v3){
				item(0) = v1;
				item(1) = v2;
				item(2) = v3;
			};
		};
		//___________________Triangles_____________________________
		class Triangles : public _list::ownobjectlist<Triangle>{
		public:
			Triangles(){};
		};

		//___________________Box__________________________
		class Box : public CubeTopology{
		private:
			pair<vect3> P;
			vect3 center;
			double d;
			friend class Line;
		public:
			Box(const vect3 & p0,const vect3 & p1);
			R1::Interval intersec(const Line & line)const;
			vect3 getPoint(const int3 & ii)const;
			//ownobjectlist<Triangle> getSurface()const;
			flatpoly4vector getSurface()const;
		};
		double D2(vect3 & x,Line & line);
	};
};
#endif

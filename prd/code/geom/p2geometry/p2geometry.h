#ifndef geom_p2geometry_h
#define geom_p2geometry_h

#include "geom/geom.h"
#include "geom/r2geometry/convexpoly.h"
#include "geom/r3geometry/r3geometry.h"
#include "geom/nmmatrix.h"

namespace _geom{
	namespace P2{
		//_____________transform________________
		class transform : public nmmatrix<4,3>{
		private:
			typedef nmmatrix<4,3> parent;
		public:
			void init(const R2::poly4 & t, const R3::flatpoly4 & T);//projective transform t->T
			vect3 operator * (const vect2 & x)const;
			vect3 operator * (const vect3 & x)const;
		};
	};
	/*
	//___________________________homography________________________
	namespace transform{
		//__________________R2Homography________________________
		class R2Homography : public matrix3{// R2->R2 homography
		private:
			typedef matrix3 parent;
		public:
			R2Homography(){};
			R2Homography(const parent & x):parent(x){};
		public:
			static R2Homography map_SQ(const R2::poly4 & t);// t -> CCW Square (0,0)(0,1),(1,1)(1,0)
			static R2Homography SQ_map(const R2::poly4 & T);// CCW Square (0,0)(0,1),(1,1)(1,0) -> T
			static R2Homography map(const R2::poly4 & t, const R2::poly4 & T);//projective transform t->T
			static R2Homography map_SQ(const vect2 & p);//sends point p to (1,1)
			static R2Homography SQ_map(const vect2 & p);//sends point (1,1) to p
			vect2 operator << (const vect3 & x)const;
			vect2 operator << (const vect2 & x)const;
		};

		//___________________R2Affine_______________________
		class R2Affine : public R2Homography{
			typedef R2Homography parent;
		public:
			R2Affine(){};
			R2Affine(const matrix3 & x):parent(x){};
		public:
			static R2Affine map_I(const vect2 & p0, const vect2 & p1, const vect2 & p2); // p0 p1 p2 -> (0,0)(0,1)(1,0)
			static R2Affine I_map(const vect2 & p0, const vect2 & p1, const vect2 & p2); // (0,0)(0,1)(1,0) -> p0 p1 p2
			static R2Affine map(const R2::poly4 & t, const R2::poly4 & T);// affine transform t->T
		};
		//___________________R3R2Projector___________________
		class R3R2Projector : public nmmatrix<3,4>{
		public:
			R3R2Projector(){};
		public:
			static R3R2Projector map(const vect3 & p0, const vect3 & p1, const vect3 & p2); //Orthogonal projector onto affine frame p0+L(p1-p0,p2-p0)
		};
		//___________________R2R3Inductor____________________
		class R2R3Inductor : public nmmatrix<4,3>{
		public:
			R2R3Inductor(){};
		public:
			static R2R3Inductor map(const vect3 & p0, const vect3 & p1, const vect3 & p2); //Orthogonal induction of affine frame p0+L(p1-p0,p2-p0) into R3
		};

		class R2R3Homography : public nmmatrix<4,3>{
		private:
			typedef nmmatrix<4,3> parent;
		public:
			R2R3Homography(){};
			R2R3Homography(const parent & x):parent(x){};
		public:
			static R2R3Homography map(const R2::poly4 & t, const R3::flatpoly4 & T);//projective transform t->T
		};
	};
	*/
};
#endif
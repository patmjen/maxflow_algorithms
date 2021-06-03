#include "stdafx.h"
#include "p2geometry.h"
#include "geom/matrix.h"
#include "debug/logs.h"

namespace _geom{
	namespace P2{
		//_________________transform_____________________
		void transform::init(const R2::poly4 & t, const R3::flatpoly4 & T){
			vect2 v1 = t[1]-t[0];
			vect2 v2 = t[3]-t[0];
			vect2 v3 = t[2];
			_geom::matrix V1(3,3);
			V1[0] << vect3(v1[0],v2[0],t[0][0]);
			V1[1] << vect3(v1[1],v2[1],t[0][1]);
			V1[2] << vect3(0,0,1);
			matrix3 V = matrix3()<<V1.invert();
			vect3 p = V*vect3(v3[0],v3[1],1.0);
			double x = p[0]/p[2];
			double y = p[1]/p[2];
			matrix3 H1(vect3(x*y+y*y-y,0,0),vect3(0,x*y+x*x-x,0),vect3(y*y-y,x*x-x,x*y));
			matrix3 H2 = matrix3::E();//temporary put T forms square
			nmmatrix<4,3> U;
			vect3 u1 = T[1]-T[0];
			vect3 u2 = T[3]-T[0];
			U[0] = vect3(u1[0],u2[0],T[0][0]);
			U[1] = vect3(u1[1],u2[1],T[0][1]);
			U[2] = vect3(u1[2],u2[2],T[0][2]);
			U[3] = vect3(0,0,1);
			parent::operator = (U*H2.invert()*H1*V);
		};

		vect3 transform::operator * (const vect3 & x)const{
			vect4 x1 = parent::operator*(x);
			return vect3(x1[0],x1[1],x1[2])/=x1[3];
		};

		vect3 transform::operator * (const vect2 & x)const{
			return (*this)*(vect3(x[0],x[1],1.0));
		};
	};
	/*
	//____________________homography____________________________
	namespace transform{
		//_______________R2Homography___________________________
		R2Homography R2Homography::map_SQ(const vect2 & p){//sends point p to (1,1)
			double x = p[0];
			double y = p[1];
			return matrix3(vect3(x*y+y*y-y,0,0),vect3(0,x*y+x*x-x,0),vect3(y*y-y,x*x-x,x*y));
		};
		R2Homography R2Homography::SQ_map(const vect2 & p){//sends point (1,1) to p};
			return map_SQ(p).invert();
		};

		R2Homography R2Homography::map_SQ(const R2::poly4 & t){
			R2Affine A = R2Affine::map_I(t[0],t[1],t[3]);
			vect3 p = A*vect3(t[2][0],t[2][1],1.0);
			return map_SQ(vect2(p[0],p[1]))*A;
		};

		R2Homography R2Homography::SQ_map(const R2::poly4 & T){// CCW Square (0,0)(0,1),(1,1)(1,0) -> T
			return SQ_map(T).invert();
		};

		R2Homography R2Homography::map(const R2::poly4 & t, const R2::poly4 & T){//projective transform t->T
			return SQ_map(T)*map_SQ(t);
		};

		vect2 R2Homography::operator << (const vect3 & x)const{
			vect3 x1 = parent::operator*(x);
			return vect2(x1[0],x1[1])/=x1[2];
		};
		vect2 R2Homography::operator << (const vect2 & x)const{
			return (*this)*(vect3(x[0],x[1],1.0));
		};
		//_______________R2Affine________________________________
		R2Affine R2Affine::I_map(const vect2 & p0, const vect2 & p1, const vect2 & p2){ // (0,0)(0,1)(1,0) -> p0 p1 p2
			R2Affine H;
			vect2 v1 = p1-p0;
			vect2 v2 = p2-p0;
			H[0] << vect3(v1[0],v2[0],p0[0]);
			H[1] << vect3(v1[1],v2[1],p0[1]);
			H[2] << vect3(0,0,1);
			return H;
		};
		R2Affine R2Affine::map_I(const vect2 & p0, const vect2 & p1, const vect2 & p2){
			return I_map(p0,p1,p2).invert();
	
		};
		R2Affine R2Affine::map(const R2::poly4 & t, const R2::poly4 & T){// affine transform t->T
			return I_map(T[0],T[1],T[2])*map_I(t[0],t[1],t[2]);
		};
		//____________________R3R2Projector___________________________
		R3R2Projector R3R2Projector::map(const vect3 & p0, const vect3 & p1, const vect3 & p2){ //Orthogonal projector onto affine frame p0+L(p1-p0,p2-p0)
			R3R2Projector P;
			matrix3 M;
			M[0] = p1-p0;
			M[1] = p2-p0;
			M[2] = M[0]^M[1];
			M = M.transp().invert();
			vect3 p = -M*p0;
			P[0] = vect4(M[0][0],M[0][1],M[0][2],p[0]);
			P[1] = vect4(M[1][0],M[1][1],M[1][2],p[1]);
			P[2] = vect4(0,0,0,1);
			return P;
		};
		//_____________________R2R3Inductor____________________________
		R2R3Inductor map(const vect3 & p0, const vect3 & p1, const vect3 & p2){ //Orthogonal induction of affine frame p0+L(p1-p0,p2-p0) into R3
			R2R3Inductor I;
			vect3 v1 = p1-p0;
			vect3 v2 = p2-p0;
			I[0] = vect3(v1[0],v2[0],p0[0]);
			I[1] = vect3(v1[1],v2[1],p0[1]);
			I[2] = vect3(v1[2],v2[2],p0[2]);
			I[3] = vect3(0,0,1);
			return I;
		};
		//____________________R2R3Homography___________________________
		R2R3Homography R2R3Homography::map(const R2::poly4 & t, const R3::flatpoly4 & T){//projective transform t->T
			vect3 v1 = T[1]-T[0];
			vect3 v2 = T[3]-T[0];
			R3R2Projector P1 = R3R2Projector::map(T[0],T[1],T[3]);
			vect3 p = P1*vect4(T[2][0],T[2][1],T[2][2],1);
			R2Homography H = R2Homography::SQ_map(vect2(p[0],p[1]))*R2Homography::map_SQ(t);
			R2R3Inductor I1 = R2R3Inductor::map(T[0],T[1],T[3]);
			return I1*H;
		};
	};
	*/
};
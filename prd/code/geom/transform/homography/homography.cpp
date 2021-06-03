#include "homography.h"

namespace geom{
	namespace homography{
		//_______________R2Homography___________________________
		H<2,2> R2Homography::map_SQ(const vect2 & p){//sends point p to (1,1)
			double x = p[0];
			double y = p[1];

			matrix3 m3;
			nmmatrix<3,3> m33=m3;
			H<2,2> h22 = m33;

			H<2,2> r(matrix3(vect3(x*y+y*y-y,0,0),vect3(0,x*y+x*x-x,0),vect3(y*y-y,x*x-x,x*y)));
			return r;
		};
		H<2,2> R2Homography::SQ_map(const vect2 & p){//sends point (1,1) to p};
			return map_SQ(p).invert();
		};

		H<2,2> R2Homography::map_SQ(const R2::poly4 & t){
			H<2,2> A = R2Affine::map_I(t[0],t[1],t[3]);
			vect3 p = A*hvect2(t[2]);
			return map_SQ(vect2()<<p)*A;
		};

		H<2,2> R2Homography::SQ_map(const R2::poly4 & T){// CCW Square (0,0)(0,1),(1,1)(1,0) -> T
			return map_SQ(T).invert();
		};

		H<2,2> R2Homography::map(const R2::poly4 & t, const R2::poly4 & T){//projective transform t->T
			return SQ_map(T)*map_SQ(t);
		};

		H<2,2> R2Homography::cnter_to_image(double w, double h){//map (0,0)(1,0)(1,0)-> (w/2,h/2)(w,h/2)(w/2,0)
			H<2,2> M;
			M[0] = vect3(w/2,0,w/2);
			M[1] = vect3(0,-h/2,h/2);
			M[2] = vect3(0,0,1);
			return M;
		};

		H<2,2> R2Homography::mapZ(double z0,double z1,double z2){
			double zv1 = z1-z0;
			double zv2 = z2-z0;
			return H<2,2>(vect3(z1,0,0),vect3(0,z2,0),vect3(zv1,zv2,z0));
		};

		//_______________R2Affine________________________________
		H<2,2> R2Affine::I_map(const vect2 & p0, const vect2 & p1, const vect2 & p2){ // (0,0)(0,1)(1,0) -> p0 p1 p2
			R2Affine H;
			vect2 v1 = p1-p0;
			vect2 v2 = p2-p0;
			H[0] << vect3(v1[0],v2[0],p0[0]);
			H[1] << vect3(v1[1],v2[1],p0[1]);
			H[2] << vect3(0,0,1);
			return H;
		};
		H<2,2> R2Affine::map_I(const vect2 & p0, const vect2 & p1, const vect2 & p2){
			return I_map(p0,p1,p2).invert();
		};

		H<2,2> R2Affine::map(const R2::poly4 & t, const R2::poly4 & T){// affine transform t->T
			return I_map(T[0],T[1],T[2])*map_I(t[0],t[1],t[2]);
		};

		H<2,2> R2Affine::map(const R2::poly3 & t, const R2::poly3 & T){// affine transform t->T
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
		//_____________________R3R2Projector___________________________
		const R3R1Projector R3R1Projector::ZW(vect4(0,0,1,0),vect4(0,0,0,1));//Orthogonal projector onto ZW
		//_____________________R2R3Inductor____________________________
		R2R3Inductor R2R3Inductor::map(const vect3 & p0, const vect3 & p1, const vect3 & p2){ //Orthogonal induction of affine frame p0+L(p1-p0,p2-p0) into R3
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
		H<3,2> R2R3Homography::map(const R2::poly4 & t, const R3::flatpoly4 & T){//projective transform t->T
			vect3 v1 = T[1]-T[0];
			vect3 v2 = T[3]-T[0];
			R3R2Projector P1 = R3R2Projector::map(T[0],T[1],T[3]);
			vect3 p = P1*hvect3(T[2]);
			R2Homography H1 = R2Homography::SQ_map(vect2(p[0],p[1]));
			R2Homography H2 = R2Homography::map_SQ(t);
			R2Homography H = H1*H2;
			R2R3Inductor I1 = R2R3Inductor::map(T[0],T[1],T[3]);
			return I1*H;
		};
		H<1,2> R2R1Homography::map_1_z(const vect3 & p0,const vect3 & p1, const vect3 & p2){//maps (x,y) -> 1/z
			vect3 v1 = p1-p0;
			vect3 v2 = p2-p0;
			vect3 n = v1^v2;
			n/=(n*p0);
			return H<1,2>(vect3(0,0,n[2]),vect3(-n[0],-n[1],1));
		};

		H<1,2> R2R1Homography::map_value(const double z0,const double z1,const double z2){//Affine map I -> z, linear z-interpolation
			return H<1,2>(vect3(z1-z0,z2-z0,z0),vect3(0,0,1));
		};
		/*
		H<1,2> R2R1Homography::map_inv_value(const double z0,const double z1,const double z2){//Affine map I -> 1/z, linear z-interpolation
			return H<1,2>(vect3(math::sqr(z1)-math::sqr(z0),math::sqr(z2)-math::sqr(z0),math::sqr(z0)),vect3(z1-z0,z2-z0,z0));
		};
		*/
	};
};
#ifndef geom_transform_homography_h
#define geom_transform_homography_h

#include "hvect.h"
#include "geom/r2geometry/vect2.h"
//#include "geom/r3geometry/vect3.h"

#include "geom/nmmatrix.h"
#include "geom/r2geometry/convexpoly.h"
#include "geom/r3geometry/flatpoly.h"
//#include "geom/r3geometry/r3geometry.h"

namespace geom{
	namespace homography{

		//_________________homographies__________________
		template<int pn, int pm> class H : public nmmatrix<pn+1,pm+1>{
		protected:
			typedef vectn<pm+1> trvect;
		public:
			typedef nmmatrix<pn+1,pm+1> parent;
			typedef hvectn<pm> tvect;
		public:
			H(){};
			H(const parent & x):parent(x){};
			H(const matrixn<pn+1> &x):parent(x){};
			H(const trvect & x1, const trvect & x2 = trvect(), const trvect & x3 = trvect(), const trvect & x4 = trvect()):parent(x1,x2,x3,x4){};
		public:
			H operator *(const matrixn<pm+1>& x)const{
				H r;
				convolution<1,0>(r,(*this),x); 
				return r;
			};
			H operator *(const double & k)const{
				return parent::operator*(k);
			};
			template <int k> H<pn,k-1> operator *(const nmmatrix<pm+1,k> & x)const{
				H<pn,k-1> r; 
				convolution<1,0>(r,(*this),x); 
				return r;
			};

			template <int k> H<pn,k> operator *(const H<pm,k> & x)const{
				H<pn,k> r; 
				convolution<1,0>(r,(*this),x); 
				return r;
			};

			hvectn<pn> operator *(const hvectn<pm>& x)const{
				hvectn<pn> r;
				convolution<1,0>(r,(*this),x); 
				return r;
			};
			H<pn,pm> & normalize(){
				double A = 1;
				for(int i=0;i<pn+1;i++){
					for(int j=0;j<pm+1;j++){
						A+=abs((*this)[i][j]);
					};
				};
				A=A/((pn+1)*(pm+1))/256;
				(*this)/=A;
				return *this;
			};
			static H<pm,pm> displacement(const vectn<pm>& x){
				H<pm,pm> r;
				r<<matrixn<pm+1>::E();
				for(int i=0;i<pm;++i){
					r[i][pm] = x[i];
				};
				return r;
			};
		};
		//__________________R2Homography________________________
		class R2Homography : public H<2,2>{// R2->R2 homography
		private:
			typedef H<2,2> parent;
		public:
			R2Homography(){};
			R2Homography(const parent & x):parent(x){};
		public:
			static H<2,2> map_SQ(const R2::poly4 & t);// t -> CCW Square (0,0)(0,1),(1,1)(1,0) //??? (0,0)(1,0),(1,1)(0,1)
			static H<2,2> SQ_map(const R2::poly4 & T);// CCW Square (0,0)(0,1),(1,1)(1,0) -> T
			static H<2,2> map(const R2::poly4 & t, const R2::poly4 & T);//projective transform t->T
			static H<2,2> map_SQ(const vect2 & p);//sends point p to (1,1)
			static H<2,2> SQ_map(const vect2 & p);//sends point (1,1) to p
			static H<2,2> cnter_to_image(double width, double height);//map (0,0)(1,0)(1,0)-> (w/2,h/2)(w,h/2)(w/2,0)
			static H<2,2> mapZ(double z0,double z1,double z2); // I->I Homography, projecting zi to 1/zi
		};

		//___________________R2Affine_______________________
		class R2Affine : public R2Homography{
			typedef R2Homography parent;
		public:
			R2Affine(){};
		public:
			static H<2,2> map_I(const vect2 & p0, const vect2 & p1, const vect2 & p2); // p0 p1 p2 -> (0,0)(0,1)(1,0)
			static H<2,2> I_map(const vect2 & p0, const vect2 & p1, const vect2 & p2); // (0,0)(0,1)(1,0) -> p0 p1 p2
			static H<2,2> map(const R2::poly4 & t, const R2::poly4 & T);// affine transform t->T
			static H<2,2> map(const R2::poly3 & t, const R2::poly3 & T);// affine transform t->T
		};
		//___________________R3R2Projector___________________
		class R3R2Projector : public H<2,3>{
		public:
			R3R2Projector(){};
		public:
			static R3R2Projector map(const vect3 & p0, const vect3 & p1, const vect3 & p2); //Orthogonal projector onto affine frame p0+L(p1-p0,p2-p0)
		};
		//___________________R3R1Projector___________________
		class R3R1Projector : public H<1,3>{
		private:
			typedef H<1,3> parent;
		public:
			R3R1Projector(){};
			R3R1Projector(const vect4 & x1, const vect4 & x2):parent(x1,x2){};
		public:
			static const R3R1Projector ZW;//Orthogonal projector onto ZW
		};
		//___________________R2R3Inductor____________________
		class R2R3Inductor : public H<3,2>{
		public:
			R2R3Inductor(){};
		public:
			static R2R3Inductor map(const vect3 & p0, const vect3 & p1, const vect3 & p2); //Orthogonal induction of affine frame p0+L(p1-p0,p2-p0) into R3
		};

		//___________________R2R3Homography__________________
		class R2R3Homography : public H<3,2>{
		private:
			typedef H<3,2> parent;
		public:
			R2R3Homography(){};
			R2R3Homography(const parent & x):parent(x){};
		public:
			static H<3,2> map(const R2::poly4 & t, const R3::flatpoly4 & T);//projective transform t->T
		};
		class R2R1Homography : public H<1,2>{
		private:
			typedef H<1,2> parent;
		public:
			static H<1,2> map_1_z(const vect3 & p0,const vect3 & p1, const vect3 & p2);//maps (x,y) -> 1/z
			static H<1,2> map_value(const double z0,const double z1,const double z2);//Affine map I -> z
			//static H<1,2> map_inv_value(const double z0,const double z1,const double z2);//map I -> 1/z, linear z-interpolation
		};
	};
};

#endif

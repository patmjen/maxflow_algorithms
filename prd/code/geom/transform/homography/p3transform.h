#ifndef geom_transform_p3transform_h
#define geom_transform_p3transform_h

#include "geom/geom.h"
#include "homography.h"

namespace _geom{
	namespace P3{
		//____________________transform_______________________
		class transform : public homography::H<3,3>{
		private:
			typedef homography::H<3,3> parent;
		public:
			transform(){};
			transform(const transform & x):parent(x){};
			transform(const parent & x):parent(x){};
			transform(const matrixn<4> &x):parent(x){};
			transform(const nmmatrix<4,4> &x):parent(x){};
			transform(const trvect & x1, const trvect & x2 , const trvect & x3 , const trvect & x4 ):parent(x1,x2,x3,x4){};
		public:
			static transform displacement(const vect3 & p);
		public:
			void operator = (const parent & x){
				parent::operator=(x);
			};
		public:
			static const transform YZ;
			static const transform YZNX;
		};

		typedef transform ViewTransform;
		typedef transform ProjectionTransform;
		typedef transform WorldTransform;
		//____________________ViewProjection_____________

		class ViewProjection{
		public:
			static ProjectionTransform getProjection(double f = 1.0);
			static ViewTransform getViewTransform(const camera & c);
			static transform getScreenTransform(double screenWidth,double screenHeight);
			static transform getClearProjection(double screenWidth, double screenHeight,double f = 1.0,double k=0.5);
		private:
			ViewProjection();
		};
	};
};

#endif

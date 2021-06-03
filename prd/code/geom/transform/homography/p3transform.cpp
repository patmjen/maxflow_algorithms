#include "p3transform.h"

#include "phisics/phisics.h"

namespace _geom{
	namespace P3{
		using namespace homography;
		//__________________transform_______________________
		const transform transform::YZ(vect4(1,0,0,0),vect4(0,0,1,0),vect4(0,1,0,0),vect4(0,0,0,1));
		const transform transform::YZNX(vect4(-1,0,0,0),vect4(0,0,1,0),vect4(0,1,0,0),vect4(0,0,0,1));
		transform transform::displacement(const vect3 & p){
			return transform(vect4(1,0,0,p[0]),vect4(0,1,0,p[1]),vect4(0,0,1,p[2]),vect4(0,0,0,1));
		};
		//__________________ViewProjection__________________
		ViewProjection::ViewProjection(){
		};

		ViewTransform ViewProjection::getViewTransform(const camera & c){
			ViewTransform r;
			const ortcoords & im = c;
			vect3 wp = -im.ortbasis::operator<<(im.p);
			r[0] = im[0]|wp[0];
			r[1] = im[1]|wp[1];
			r[2] = im[2]|wp[2];
			r[3] = vect4(0,0,0,1);
			return r;
		};

		ProjectionTransform ViewProjection::getProjection(double f){
			ProjectionTransform proj;
			double screenWidth = 40*units::cm;
			double screenHeight = 30*units::cm;
			double zn = 20*units::cm, zf = 500*units::m;
			double Q = zf/(zf-zn);
			double w = f*0.4/screenWidth;
			double h = f*0.4/screenHeight;
			proj[0] = vect4(w,	0.0,	0.0,	0.0);
			proj[1] = vect4(0.0,	h,	0.0,	0.0);
			proj[2] = vect4(0.0,	0.0,	Q,	1.0);
			proj[3] = vect4(0.0, 0.0,	-Q*zn,	0.0);
			return proj.transp();
		};

		transform ViewProjection::getClearProjection(double screenWidth, double screenHeight,double f,double k){
			ProjectionTransform proj;
			double zn = screenWidth*0.1, zf = zn*1e4;
			double Q = zf/(zf-zn);
			double w = f/screenWidth;
			double h = f/screenHeight;
			proj[0] = vect4(w,	0.0,	0.0,	0.0);
			proj[1] = vect4(0.0,	h,	0.0,	0.0);
			proj[2] = vect4(0.0,	0.0,	Q,	1.0);
			proj[3] = vect4(0.0, 0.0,	-Q*zn,	0.0);
			return proj.transp();
		};

		transform ViewProjection::getScreenTransform(double W,double H){
			return transform(vect4(W/2,0,0,W/2),vect4(0,-H/2,0,H/2),vect4(0,0,1,0),vect4(0,0,0,1));
		};
	};
};
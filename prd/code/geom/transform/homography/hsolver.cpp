#include "hsolver.h"
#include "debug/logs.h"

namespace geom{
	namespace homography{
		H2Solver::H2Solver(){
			init();
//			x.resize(9);
//			XTb.resize(9);
			Cp = matrix3::eye(3);
			Cq = matrix3::eye(3);
		};
		void H2Solver::init(){
//			XTX = matrix::Zeros(9,9);
			XTX << 0;
			XTb << 0;
		};

		void H2Solver::setScale_left(const matrix3 & Cp){
			this->Cp = Cp;
		};

		void H2Solver::setScale_right(const matrix3 & Cq){
			this->Cq = Cq;
		};

		void H2Solver::push_pair(const hvect2 & p, const hvect2 & q){
			matrix3 pp = ortbasis::Cross(Cp*p);
			vect3 qq = Cq*q;
			for(int k=0;k<3;++k){
				for(int i=0;i<3;++i){
					for(int j=0;j<3;++j){
						x[i*3+j] = pp[k][i]*qq[j];
					};
				};
//				b = x[8];
				for(int i=0;i<XTX.count(0);++i){
					for(int j=0;j<XTX.count(1);++j){
						XTX[i][j]+=x[i]*x[j];
					};
//					XTb[i]+=x[i]*b;
				};
			};
		};

		H<2,2> H2Solver::solve(){
			matrix XTXI = XTX.invert();
			vect b_hat = XTXI*XTb;
			H<2,2> h;
			matrix U,V;
			vect s;
			XTX.SVD(U,V,s);
			int i_min = s.findmin();
			vect u_min = U.transp()[i_min] - b_hat;
			for(int i=0;i<3;++i){
				for(int j=0;j<3;++j){
					//if(i*3+j<u_min.count())
					h[i][j] = u_min[i*3+j];
				};
			};
//			h[2][2] = 1.0;
			stream <<"b_hat=" <<b_hat<<"\n";
			stream <<"h=" <<h<<"\n";
			//stream <<"Cp=" <<Cp<<"\n";
			//stream <<"Cq=" <<Cq<<"\n";
			h << Cp.invert()*h*Cq;
			return h;
		};
	};
};
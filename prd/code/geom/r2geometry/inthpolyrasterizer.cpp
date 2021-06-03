#include "inthpolyrasterizer.h"

namespace geom{
	namespace R2{
		namespace y_iterator{
			//________line_y_iterator_______
			void int_line_y_iterator::init(const vect2 & p1, const vect2 & p2){
				y2 = (ffloat)p2[1];
				vect2 v = p2-p1;
				xindex=(v[0]>0);
//				if(math::abs(p1[0])>(1<<15))throw exception("x is too large");
				(*this)[xindex] = (*this)[!xindex]= (ffloat)p1[0];
				y = (ffloat)::ceil(p1[1]);
				if(math::abs(v[1])>eps0){
					dxdy = ffloat(v[0]/v[1]);
					if(y2>y){
						(*this)[xindex]+=dxdy*(y-ffloat(p1[1]));
						_caniterate = 1;
					}else{
						(*this)[xindex]+=dxdy*(ffloat(v[1]));
						_caniterate = 0;
					};
				}else{
					(*this)[xindex] = (ffloat)p2[0];
					_caniterate = 0;
				};
				_allowed = 1;
			};

			inline ffloat int_line_y_iterator::center()const{
				ffloat r = (*this)[0]+(*this)[1];
				return shr(r,1);
			};

			int int_line_y_iterator::allowed()const{
				return _allowed;
			};

			int int_line_y_iterator::caniterate()const{
				return _caniterate;
			};

			int_line_y_iterator& int_line_y_iterator::operator ++(){
				if(_caniterate){
					(*this)[!xindex] =(*this)[xindex];
					y+=ffloat(1);
					if(y2>y){
						(*this)[xindex]+=dxdy;
					}else{
						(*this)[xindex]+=dxdy*(y2-(y-ffloat(1)));
						_caniterate = 0;
					};
				}else _allowed = 0;
				return *this;
			};
		};
		//________xiterator_______________
		void xiterator::init(const int x1,const int x2){
			I[0] = x = x1;
			I[1] = x2;
		};
		void xiterator::operator ++(){
			++x;
		};
		int xiterator::allowed()const{
			return x<I[1];
		};

		//__________________int_hpolyrastirizer_____________________
		template<int n> int_hpoly_rasterizer<n>::int_hpoly_rasterizer(){
		};

		template<int n> void int_hpoly_rasterizer<n>::init(const int2 & g_CC, const typename int_hpoly_rasterizer<n>::tpoly & T){
			using namespace P2;
			using namespace homography;
			CC = g_CC;
			int k = 0;

			_t.clear();

			//! z-clipping
			for(int i=0;i<T.count();i++){
				int i1 = (i+T.count()-1)%T.count();
				if(T[i1][2]>0 && T[i][2]<0){
					const hvect3 & p = T[i1];
					hvect3 p1 = T[i];
					double alpha = -p[2]/(p1[2]-p[2]);
					hvect3 np = p+(p1-p)*alpha;
					np/=np[3];
					_t.add(vect2(np[0],np[1]));
					++k;
				};
				if(T[i1][2]<0 && T[i][2]>0){
					const hvect3 & p = T[i];
					hvect3 p1 = T[i1];
					double alpha = -p[2]/(p1[2]-p[2]);
					hvect3 np = p+(p1-p)*alpha;
					np/=np[3];
					_t.add(vect2(np[0],np[1]));
					++k;
				};

				if(T[i][2]>0){
					_t.add(vect2(T[i][0]/T[i][3],T[i][1]/T[i][3]));
					++k;
				};
			};

			if(_t.count()<3)throw runtime_error("cannot rasterize hpoly");
			int topi = 0;
			double miny = _t[0][1];
			for(int i=1;i<(int)_t.count();++i){
				if(miny>_t[i][1]){
					miny = _t[i][1];
					topi = i;
				};
			};

			t.clear();
			for(int k=0;k<(int)_t.count();k++)t.add(_t[(k+topi)%_t.count()]);

			li=1;
			ri=(int)t.count()-1;
			left.init(t[0],t[li]);
			while(!left.caniterate() && (li<ri)){
				int bli=li;
				left.init(t[bli],t[++li]);
			};
			right.init(t[0],t[ri]);
			while(!right.caniterate() && (li<ri)){
				int bri=ri;
				right.init(t[bri],t[--ri]);
			};
			y = (int)::floor(t[0][1]);
			if(y>=CC[1])_yallowed = 0;
			else{
				_yallowed = 1;
				if(y<0)operator ++();
				if(y>=CC[1])_yallowed = 0;
				if(_yallowed){
					initx();
				};
				if(!x.allowed())operator ++();
			};
		};

		template<int n> void int_hpoly_rasterizer<n>::initx(){
			x.init( max(0,(int)left.center()) , min(CC[0],(int)right.center()) );
		};

		template<int n> void int_hpoly_rasterizer<n>::operator ++(){
			do{
				do{
					++y;
					if(!(++left).allowed()){
						if(li<ri){
							int bli=li;
							left.init(t[bli],t[++li]);
							++left;
						}else _yallowed = 0;
					};
					if(!(++right).allowed()){
						if(li<ri){
							int bri=ri;
							right.init(t[bri],t[--ri]);
							++right;
						}else _yallowed = 0;
					};
				}while(y<0 && _yallowed);
				if(y>=CC[1])_yallowed = 0;
				if(_yallowed){
					initx();
				};
			}while(_yallowed && !x.allowed());
		};

		namespace{
			template<int n> void link_Templatesn(){
				int_hpoly_rasterizer<n> hh;
				hh.init(int2(),int_hpoly_rasterizer<n>::tpoly());
				hh.allowed();
				++hh;
			};
			void link_Templates(){
				link_Templatesn<3>();
				link_Templatesn<4>();
			};
		};

	};
};

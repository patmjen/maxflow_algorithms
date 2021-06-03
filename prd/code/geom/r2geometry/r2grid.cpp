#include "r2grid.h"

#include "debug/except.h"
#include "exttype/minmax.h"


namespace geom{
	namespace R2{
		//_________________Grid________________________
				//________line_y_iterator_______
		void Grid::line_y_iterator::init(const vect2 & p1, const vect2 & p2){
			y2 = p2[1];
			vect2 v = p2-p1;
			xindex=(v[0]>0);
			(*this)[xindex] = (*this)[!xindex]= p1[0];
			//y = ::floor(p1[1])+1;
			y = ::floor(p1[1]-0.1)+1;
			if(math::abs(v[1])>eps0){
				dxdy = v[0]/v[1];
				if(y2>y){
					(*this)[xindex]+=dxdy*(y-p1[1]);
					_caniterate = 1;
				}else{
					(*this)[xindex]+=dxdy*(v[1]);
					_caniterate = 0;
				};
			}else{
				(*this)[xindex]+=v[0];
				_caniterate = 0;
			};
			_allowed = 1;
		};

		int Grid::line_y_iterator::allowed()const{
			return _allowed;
		};
		int Grid::line_y_iterator::caniterate()const{
			return _caniterate;
		};

		Grid::line_y_iterator& Grid::line_y_iterator::operator ++(){
			if(_caniterate){
				(*this)[!xindex] =(*this)[xindex];
				if(y2>++y){
					(*this)[xindex]+=dxdy;
				}else{
					(*this)[xindex]+=dxdy*(y2-(y-1));
					_caniterate = 0;
				};
			}else _allowed = 0;
			return *this;
		};
		//________xiterator_______________
		void Grid::xiterator::init(const int x1,const int x2){
			I[0] = x = x1;
			I[1] = x2;
		};
		void Grid::xiterator::operator ++(){
			++x;
		};
		int Grid::xiterator::allowed(){
			return x<I[1];
		};

		//________polyrastirizer______
		template<int n> void Grid::polyrasterizer<n>::init(const Grid & g, const typename polyrasterizer<n>::tpoly & T){
			CC = g.CC;
			vectn<n> top;
			for(int i=0;i<top.count();i++)top[i] = T[i][1];
			int topi = top.min().second;
			for(int k=0;k<t.count();k++)t[k] = T[(k+topi)%t.count()];
			li=1;
			ri=t.count()-1;
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
			y = (int)::floor(t[0][1]);//?fllor/ceil
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

		template<int n> void Grid::polyrasterizer<n>::firtstvalid(){
		};

		template<int n> void Grid::polyrasterizer<n>::initx(){
			x.init( max(0,(int)::floor(left[0])) , min(CC[0],(int)::ceil(right[1])) );
		};

		template<int n> void Grid::polyrasterizer<n>::operator ++(){
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
		template<int n> void Grid::polyrasterizer<n>::test(){
			init(Grid(),tpoly());
			allowed();
			++(*this);
		};
		//__________________hpolyrastirizer_____________________
		template<int n> void Grid::hpolyrasterizer<n>::init(const Grid & g, const typename hpolyrasterizer<n>::tpoly & T){
			using namespace P2;
			using namespace homography;
			CC = g.CC;
			int k = 0;

			_t.clear();

			for(int i=0;i<T.count();i++){
				int i1 = (i+T.count()-1)%T.count();
				if(T[i1][2]>0 && T[i][2]<0){
					const hvect3 & p = T[i1];
					hvect3 p1 = T[i];
					double alpha = -p[2]/(p1[2]-p[2]);
					hvect3 np = p+(p1-p)*alpha;
					np/=np[3];
					_t.push_back(vect2(np[0],np[1]));
					++k;
				};
				if(T[i1][2]<0 && T[i][2]>0){
					const hvect3 & p = T[i];
					hvect3 p1 = T[i1];
					double alpha = -p[2]/(p1[2]-p[2]);
					hvect3 np = p+(p1-p)*alpha;
					np/=np[3];
					_t.push_back(vect2(np[0],np[1]));
					++k;
				};

				if(T[i][2]>0){
					_t.push_back(vect2(T[i][0]/T[i][3],T[i][1]/T[i][3]));
					++k;
				};
			};

			//vect top((int)t.size());
			//for(int i=0;i<(int)_t.size();i++)top.add(_t[i][1]);
			//int topi = top.findmin();
			if(_t.size()<3)throw debug_exception("cannot rasterize triangle");
			int topi = 0;
			double miny = _t[0][1];
			for(int i=1;i<(int)_t.size();++i){
				if(miny>_t[i][1]){
					miny = _t[i][1];
					topi = i;
				};
			};

			t.clear();
			for(int k=0;k<(int)_t.size();k++)t.push_back(_t[(k+topi)%_t.size()]);

			li=1;
			ri=(int)t.size()-1;
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

		template<int n> void Grid::hpolyrasterizer<n>::firtstvalid(){
		};

		template<int n> void Grid::hpolyrasterizer<n>::initx(){
			x.init( max(0,(int)::floor(left.center())) , min(CC[0],(int)::floor(right.center())) );
		};

		template<int n> void Grid::hpolyrasterizer<n>::operator ++(){
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
		template<int n> void Grid::hpolyrasterizer<n>::test(){
			init(Grid(),tpoly());
			allowed();
			++(*this);
		};

		//_______Grid________________
		template<int n> Grid::polyrasterizer<n> Grid::getrasterizer(const typename Grid::polyrasterizer<n>::tpoly & t)const{
			polyrasterizer<n> ii;
			ii.init(*this,t);
			return ii;
		};
		template<int n> Grid::hpolyrasterizer<n> Grid::getrasterizer(const typename Grid::hpolyrasterizer<n>::tpoly & t)const{
			hpolyrasterizer<n> ii;
			ii.init(*this,t);
			return ii;
		};
		Grid::hpolyrasterizer<3> Grid::getrasterizer(const Grid::hpolyrasterizer<3>::tpoly & t)const{
			hpolyrasterizer<3> ii;
			ii.init(*this,t);
			return ii;
		};
		Grid::hpolyrasterizer<4> Grid::getrasterizer(const Grid::hpolyrasterizer<4>::tpoly & t)const{
			hpolyrasterizer<4> ii;
			ii.init(*this,t);
			return ii;
		};
		void Grid::nocallinit(){
			getrasterizer<3>(poly3()).test();
			getrasterizer<4>(poly4()).test();
			getrasterizer<3>(P2::hpoly3()).test();
			//getrasterizer<4>(poly4()).test();
		};
	};
};

#ifndef r2grid_h
#define r2grid_h

#include "r2geometry.h"
#include "geom/grid/grid.h"

#include "convexpoly.h"
#include "geom/p2geometry/hpoly.h"

namespace geom{
	namespace R2{
		typedef mint2 GridIndex;
		typedef std::vector<GridIndex> GridIndexList;

		class Grid : public grid::GridTopology<2>{
		private:
			typedef grid::GridTopology<2> parent;
		public:
			using parent::tindex;
			using parent::titerator;
			using parent::trangeiterator;
		public:
			Grid(const mint2 & size = mint2(0,0),double CellSize = 1.0):parent(size,CellSize){};
		public:
			//_______________line_y_iterator______________
			class line_y_iterator : public R1::Interval{
			private:
				typedef R1::Interval parent;
				double y2;
				double y;
				double dxdy;
				int _allowed;
				int _caniterate;
				int xindex;
			public:
				void init(const vect2 & p1, const vect2 & p2);
				line_y_iterator & operator ++();
				int allowed()const;
				int caniterate()const;
			};
			//_________________xiterator________________
			class xiterator{
			public:
				mint2 I;
			public:
				int x;
				void init(const int x1,const int x2);
				void operator ++();
				int allowed();
			};
			/*
			//_______________triangleiterator_____________
			class trianglerasterizer{
			private:
			Triangle t;
			line_y_iterator left;
			line_y_iterator right;
			int _yallowed;
			//int x2;
			int li;//left vertex index
			int ri;//right vertex index
			tindex CC;
			public:
			int y;
			xiterator x;
			private:
			void initx();
			public:
			void init(const Grid & g, const poly3 & t);
			int allowed()const;
			void operator ++();
			};
			trianglerasterizer gettrianglerasterizer(const poly3 & t)const;
			*/
			//_______________polyrasterizer_____________
			template <int n> class polyrasterizer{
			private:
				typedef convexpoly<n, fixedlist> tpoly;
			private:
				tpoly t;
				int _yallowed;
				int li;//left vertex index
				int ri;//right vertex index
				tindex CC;
			public:
				int y;
				xiterator x;
				line_y_iterator left;
				line_y_iterator right;
			private:
				void initx();
				void firtstvalid();
			public:
				void init(const Grid & g, const tpoly & t);
				int allowed()const{
					return _yallowed;
				};
				void operator ++();
			private:
				friend class Grid;
				void test();
			};
			template<int n> polyrasterizer<n> getrasterizer(const typename polyrasterizer<n>::tpoly & t)const;
			typedef polyrasterizer<3> trianglerasterizer;
			typedef polyrasterizer<4> quadrasterizer;
			//__________________________hpolyrasterizer_____________________________
			template <int n> class hpolyrasterizer{
			private:
				typedef P2::hpolyn<n> tpoly;
			private:
				//				tpoly t;
				std::vector<vect2> t;
				int _yallowed;
				int li;//left vertex index
				int ri;//right vertex index
				tindex CC;
				std::vector<vect2> _t;
			public:
				int y;
				xiterator x;
				line_y_iterator left;
				line_y_iterator right;
			private:
				void initx();
				void firtstvalid();
			public:
				void init(const Grid & g, const tpoly & t);
				int allowed()const{
					return _yallowed;
				};
				void operator ++();
			private:
				friend class Grid;
				void test();
			};
			template<int n> hpolyrasterizer<n> getrasterizer(const typename hpolyrasterizer<n>::tpoly & t)const;
			hpolyrasterizer<3> getrasterizer(const hpolyrasterizer<3>::tpoly & t)const;
			hpolyrasterizer<4> getrasterizer(const hpolyrasterizer<4>::tpoly & t)const;
		private:
			void nocallinit();
		};

		//_________________________ObjectGrid______________________________________
		template <class type> class ObjectGrid : public grid::ObjectGrid<type,2,Grid>{
		private:
			typedef grid::ObjectGrid<type,2,Grid> parent;
		public:
			ObjectGrid(const mint2 & size = mint2(0,0),double CellSize = 1.0):parent(size,CellSize){
			};
		};

	};
};

#endif

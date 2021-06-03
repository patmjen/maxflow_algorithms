#ifndef geom_inthpolyrasterizer_h
#define geom_inthpolyrasterizer_h

//#include "geom/intarithmetik/intarithmetik.h"
#include "geom/p2geometry/hpoly.h"
//#include "geom/r1geometry/r1geometry.h"
#include <vector>

//#include "memmanager/buffer.h"

namespace geom{
	namespace R2{
		namespace y_iterator{
			typedef float ffloat;
			typedef fixedlist<float,2> ffvect2;
			//_______________int_line_y_iterator______________
			class int_line_y_iterator : public ffvect2{
			private:
				typedef ffvect2 parent;
				ffloat y2;
				ffloat y;
				ffloat dxdy;
				int _allowed;
				int _caniterate;
				int xindex;
			public:
				inline ffloat center()const;
			public:
				void init(const vect2 & p1, const vect2 & p2);
				inline int_line_y_iterator & operator ++();
				inline int allowed()const;
				inline int caniterate()const;
			};
		};
		typedef y_iterator::int_line_y_iterator int_line_y_iterator;
		//_________________xiterator________________
		class xiterator{
		public:
			int2 I;
		public:
			int x;
			void init(const int x1,const int x2);
			inline void operator ++();
			inline int allowed()const;
		};

		//__________________________int_hpoly_rasterizer_____________________________
		template <int n> class int_hpoly_rasterizer{
		public:
			typedef P2::hpolyn<n> tpoly;
		private:
			memmanager::staticbuffer<vect2,8> t;
			int _yallowed;
			int li;//left vertex index
			int ri;//right vertex index
			int2 CC;
			memmanager::staticbuffer<vect2,8> _t;
		public:
			int y;
			xiterator x;
			int_line_y_iterator left;
			int_line_y_iterator right;
		private:
			inline void initx();
			void firtstvalid();
		public:
			int_hpoly_rasterizer();
			void init(const int2 & g_CC, const tpoly & t);
			int allowed()const{
				return _yallowed;
			};
			void operator ++();
		private:
			friend class Grid;
		};
	};
};

#endif
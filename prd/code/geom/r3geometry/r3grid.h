#ifndef r3grid_h
#define r3grid_h

#include "r3geometry.h"
#include "geom/grid/grid.h"

namespace _geom{
	namespace R3{
		typedef int3 GridIndex;
		typedef list<GridIndex> GridIndexList;

		typedef grid::GridTopology<3> Grid;

		template <class type> class ObjectGrid : public grid::ObjectGrid<type,3,Grid>{
		private:
			typedef grid::ObjectGrid<type,3,Grid> parent;
		public:
			ObjectGrid(const int3 & size = int3(0,0,0),double CellSize = 1.0):parent(size,CellSize){
			};
		};
	};
};

#endif

#include "dataset.h"

namespace dynamic{
	void test(){
		mint3 CC;
		CC[0] = 0;
		CC << 0;
		geom::DataSetTopology<3> topology3;
		geom::DataSetTopology<3>::nb_iterator nbit1(mint3(0,0,0),mint3(10,10,10));
		geom::DataSetTopology<3>::dir_edge_iterator eit(mint3(10,10,10));
		//geom::DataSetTopology<3>::ndir_edge_iterator neit(mint3(10,10,10));
		//++neit;
		DataSet<int,2> data1 (mint2(5,5),0);
		data1[mint2(0,0)];
		DataSet<int,2,true> data2 (mint2(5,5),0);
		data2[mint2(0,0)];
	};
};
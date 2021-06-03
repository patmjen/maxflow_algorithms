#ifndef parallel_discharge_h
#define parallel_discharge_h

#include "sequential_discharge.h"

template<class RD> class parallel_discharge : virtual public sequential_discharge<RD>{
public:
	typedef sequential_discharge<RD> parent;
	typedef typename parent::tflow tflow;
	using parent::info;
	using parent::G;
	using parent::dead;
	using parent::dead0;
	using parent::nractive;
	using parent::flow;
	using parent::d_inf;
	using parent::n_relabelled;
	using parent::preprocess;
private:
	void fuse_flow();
	void lock_msg(int r);
	void unlock_msg(int r);
	exttype::intf active;
public:
	tflow maxflow()override;
};

#endif
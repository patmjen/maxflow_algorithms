#ifndef parallel_PRD_h
#define parallel_PRD_h

#include "parallel_discharge.h"
#include "stream_PRD.h"

class parallel_PRD : public stream_PRD, public parallel_discharge<PRD>{
public:
	typedef stream_PRD parent;
private:
	void hist_dec(int sweep,int r)override;
	void hist_inc(int sweep,int r)override;
public:
	parallel_PRD();
	void construct(region_graph * G);
};

#endif
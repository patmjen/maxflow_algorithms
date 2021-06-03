#ifndef parallel_ARD1_h
#define parallel_ARD1_h

#include "parallel_discharge.h"
#include "stream_ARD1.h"

class parallel_ARD1 : public stream_ARD1, public parallel_discharge<seed_BK>{
public:
	typedef stream_ARD1 parent;
private:
	void hist_dec(int sweep,int r)override;
	void hist_inc(int sweep,int r)override;
public:
	parallel_ARD1();
	void construct(region_graph * G);
};

/*
class parallel_ARD1 : public stream_ARD1{
public:
	typedef stream_ARD1 parent;
private:
	void fuse_flow();
	void hist_dec(int sweep,int r)override;
	void hist_inc(int sweep,int r)override;
	//void msg_out(int sweep, int r);
	//void process_region(int sweep, int r);
	//void lock_in_msg(int r);
	//void unlock_in_msg(int r);
	//void lock_out_msg(int r);
	//void unlock_out_msg(int r);
	void lock_msg(int r);
	void unlock_msg(int r);
public:
	parallel_ARD1();
	void construct(region_graph * G);
	tflow maxflow()override;
};
*/
#endif
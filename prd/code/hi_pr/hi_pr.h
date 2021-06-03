/* Maximum flow - highest lavel push-relabel algorithm */
/* COPYRIGHT C 1995, 2000 by IG Systems, Inc., igsys@eclipse.net */
//AS: enclosed the code into a class, so that several instances could be run

#if _MSC_VER > 1000
#else
#define __forceinline inline
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "values.h"
#include "types.h"          /* type definitions */
#include "timer.h"          /* timing routine */

#define GLOB_UPDT_FREQ 0.5 //default value
#define WAVE_INIT
#define INIT_UPDATE
#undef  WAVE_INIT
//#undef INIT_UPDATE

class hi_pr{
public:
	/* global variables */

	long   n;                    /* number of nodes */
	long   m;                    /* number of arcs */
	long   nm;                   /* n + ALPHA * m */
	long   nMin;                 /* smallest node id */
	node   *nodes;               /* array of nodes */
	arc    *arcs;                /* array of arcs */
	bucket *buckets;             /* array of buckets */
	cType  *cap;                 /* array of capacities */
	node   *source;              /* source node pointer */
	node   *sink;                /* sink node pointer */
	//node   **queue;              /* queue for BFS */
	//node   **qHead, **qTail, **qLast;     /* queue pointers */
	long   dMax;                 /* maximum label */
	long   aMax;                 /* maximum actie node label */
	long   aMin;                 /* minimum active node label */
	excessType flow;             /* flow value */
	long pushCnt;                /* number of pushes */
	long relabelCnt;             /* number of relabels */
	long updateCnt;              /* number of updates */
	long gapCnt;                 /* number of gaps */
	long gNodeCnt;               /* number of nodes after gap */  
	float t, t2;                 /* for saving times */
	node   *sentinelNode;        /* end of the node list marker */
	arc *stopA;                  /* used in forAllArcs */
	long workSinceUpdate;        /* the number of arc scans since last update */
	float globUpdtFreq;          /* global update frequency */
	//
	long i_dist;
	//
	node *i_next, *i_prev;
public:
	hi_pr();
	int allocDS();/*!< allocate datastructures, initialize related variables */
	void init();
	void checkMax();
	void globalUpdate();/*!< global update via backward breadth first search from the sink */
	void stageTwo();/*!< second stage -- preflow to flow */
	void stageOne();/*!< first stage  -- maximum preflow*/
	long long cut_cost();
	int main(int argc, char *argv[]);
	bool is_weak_source(node * v);
public:
	template<typename tcap> 
	void construct(int nV, int nE, const int * E, const tcap * cap, const tcap * excess);
	void construct(const char * filename);
	~hi_pr(){
		destroy();
	};
	void destroy();
private:
	/*!
	parse (...) :
       1. Reads maximal flow problem in extended DIMACS format.
       2. Prepares internal data representation.
	*/
	int parse(long  *n_ad, long    *m_ad, node    **nodes_ad, arc     **arcs_ad, cType    **cap_ad, node    **source_ad, node    **sink_ad, long    *node_min_ad);
	__forceinline int gap (bucket *emptyB); /*!< gap relabeling */
	__forceinline long relabel(node *i);/*!<--- relabelling node i */
	__forceinline void discharge(node *i);/*!< discharge: push flow out of i until i becomes inactive */
	void wave();//!< go from higher to lower buckets, push flow
public:
	excessType flow0;             /* added flow value */
	void add_arc(int tail, int head, cType cap1, cType cap2, long *& arc_first, long *& arc_tail, long & pos_current, arc *& arc_current, node *& nodes, long & node_min, long & node_max, long & no_alines);
};
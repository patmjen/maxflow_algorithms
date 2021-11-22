/* Maximum flow - highest lavel push-relabel algorithm */
/* COPYRIGHT C 1995, 2000 by IG Systems, Inc., igsys@eclipse.net */
//AS: enclosed the code into a class, so that several instances could be run

#ifndef HI_PR_H__
#define HI_PR_H__

#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE __attribute__((always_inline))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "values.h"
#include "types.h"          /* type definitions */
#include "timer.h"          /* timing routine */

#define GLOB_UPDT_FREQ 0.5 //default value
#define WAVE_INIT
#define INIT_UPDATE
#undef  WAVE_INIT
//#undef INIT_UPDATE

namespace hi_pr {

class HiPr {
public:
	/* global variables */

	unsigned long   n;                    /* number of nodes */
	unsigned long   m;                    /* number of arcs */
	unsigned long   nm;                   /* n + ALPHA * m */
	unsigned long   nMin;                 /* smallest node id */
	node   *nodes;               /* array of nodes */
	arc    *arcs;                /* array of arcs */
	bucket *buckets;             /* array of buckets */
	cType  *cap;                 /* array of capacities */
	node   *source;              /* source node pointer */
	node   *sink;                /* sink node pointer */
	//node   **queue;              /* queue for BFS */
	//node   **qHead, **qTail, **qLast;     /* queue pointers */
	unsigned long   dMax;                 /* maximum label */
	unsigned long   aMax;                 /* maximum actie node label */
	unsigned long   aMin;                 /* minimum active node label */
	excessType flow;             /* flow value */
	unsigned long pushCnt;                /* number of pushes */
	unsigned long relabelCnt;             /* number of relabels */
	unsigned long updateCnt;              /* number of updates */
	unsigned long gapCnt;                 /* number of gaps */
	unsigned long gNodeCnt;               /* number of nodes after gap */
	float t, t2;                 /* for saving times */
	node   *sentinelNode;        /* end of the node list marker */
	arc *stopA;                  /* used in forAllArcs */
	unsigned long workSinceUpdate;        /* the number of arc scans since last update */
	float globUpdtFreq;          /* global update frequency */
	//
	unsigned long i_dist;
	//
	node *i_next, *i_prev;
public:
	HiPr();
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
	void construct(unsigned int nV, unsigned int nE, const int * E, const tcap * cap, const tcap * excess);
	void construct(const char * filename);
	~HiPr()
	{
		destroy();
	};
	void destroy();
private:
	/*!
	parse (...) :
	   1. Reads maximal flow problem in extended DIMACS format.
	   2. Prepares internal data representation.
	*/
	int parse(unsigned long *n_ad, unsigned long *m_ad, node **nodes_ad, arc **arcs_ad, cType **cap_ad, node **source_ad, node **sink_ad, unsigned long *node_min_ad);
	FORCE_INLINE int gap(bucket *emptyB); /*!< gap relabeling */
	FORCE_INLINE unsigned long relabel(node *i);/*!<--- relabelling node i */
	FORCE_INLINE void discharge(node *i);/*!< discharge: push flow out of i until i becomes inactive */
	void wave();//!< go from higher to lower buckets, push flow
public:
	excessType flow0;             /* added flow value */
	void add_arc(unsigned int tail, unsigned int head, cType cap1, cType cap2, unsigned long *& arc_first, unsigned long *& arc_tail, unsigned long & pos_current, arc *& arc_current, node *& nodes, unsigned long & node_min, unsigned long & node_max, unsigned long & no_alines);
};

} // namespace hi_pr

#endif // HI_PR_H__

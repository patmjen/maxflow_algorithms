/*
This software has been modified by Hossam Isack <isack.hossam@gmail.com> and Karin Ng <karinng10@gmail.com>,
to handle large graphs and to allow graph rests (to avoid reallocating memeory when weights change).
This software is provied "AS IS" without any warranty, please see original disclaimer below.
*/

/*
#########################################################
#                                                       #
#  IBFSGraph -  Software for solving                    #
#               Maximum s-t Flow / Minimum s-t Cut      #
#               using the IBFS algorithm                #
#                                                       #
#  http://www.cs.tau.ac.il/~sagihed/ibfs/               #
#                                                       #
#  Haim Kaplan (haimk@cs.tau.ac.il)                     #
#  Sagi Hed (sagihed@post.tau.ac.il)                    #
#                                                       #
#########################################################

This software implements the IBFS (Incremental Breadth First Search) maximum flow algorithm from
	"Faster and More Dynamic Maximum Flow
	by Incremental Breadth-First Search"
	Andrew V. Goldberg, Sagi Hed, Haim Kaplan, Pushmeet Kohli,
	Robert E. Tarjan, and Renato F. Werneck
	In Proceedings of the 23rd European conference on Algorithms, ESA'15
	2015
and from
	"Maximum flows by incremental breadth-first search"
	Andrew V. Goldberg, Sagi Hed, Haim Kaplan, Robert E. Tarjan, and Renato F. Werneck.
	In Proceedings of the 19th European conference on Algorithms, ESA'11, pages 457-468.
	ISBN 978-3-642-23718-8
	2011

Copyright Haim Kaplan (haimk@cs.tau.ac.il) and Sagi Hed (sagihed@post.tau.ac.il)

###########
# LICENSE #
###########
This software can be used for research purposes only.
If you use this software for research purposes, you should cite the aforementioned papers
in any resulting publication and appropriately credit it.

If you require another license, please contact the above.

*/

// TODO: node->label ~ numnodes affects this

#ifndef IBFS_H__
#define IBFS_H__

#include <stdio.h>
#include <string.h>
#include <cstdint>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T int64_t;
#endif

#define IB_BOTTLENECK_ORIG 0
#define IBTEST 0
#define IB_MIN_MARGINALS_DEBUG 0
#define IB_MIN_MARGINALS_TEST 0
#define IBSTATS 0
#define IBDEBUG(X) fprintf(stdout, "\n"); fflush(stdout)
#define IB_ALTERNATE_SMART 1
#define IB_HYBRID_ADOPTION 1
#define IB_EXCESSES 1
#define IB_ALLOC_INIT_LEVELS 4096
#define IB_ADOPTION_PR 0
#define IB_DEBUG_INIT 0

namespace ibfs {

class IBFSStats
{
public:
	IBFSStats()
	{
		reset();
	}
	void reset()
	{
		int C = (IBSTATS ? 0 : -1);
		augs = C;
		growthS = C;
		growthT = C;
		orphans = C;
		growthArcs = C;
		pushes = C;
		orphanArcs1 = C;
		orphanArcs2 = C;
		orphanArcs3 = C;
		if (IBSTATS) augLenMin = (1 << 30);
		else augLenMin = C;
		augLenMax = C;
	}
	void inline incAugs() { if (IBSTATS) augs++; }
	double inline getAugs() { return augs; }
	void inline incGrowthS() { if (IBSTATS) growthS++; }
	double inline getGrowthS() { return growthS; }
	void inline incGrowthT() { if (IBSTATS) growthT++; }
	double inline getGrowthT() { return growthT; }
	void inline incOrphans() { if (IBSTATS) orphans++; }
	double inline getOrphans() { return orphans; }
	void inline incGrowthArcs() { if (IBSTATS) growthArcs++; }
	double inline getGrowthArcs() { return growthArcs; }
	void inline incPushes() { if (IBSTATS) pushes++; }
	double inline getPushes() { return pushes; }
	void inline incOrphanArcs1() { if (IBSTATS) orphanArcs1++; }
	double inline getOrphanArcs1() { return orphanArcs1; }
	void inline incOrphanArcs2() { if (IBSTATS) orphanArcs2++; }
	double inline getOrphanArcs2() { return orphanArcs2; }
	void inline incOrphanArcs3() { if (IBSTATS) orphanArcs3++; }
	double inline getOrphanArcs3() { return orphanArcs3; }
	void inline addAugLen(int64_t len)
	{
		if (IBSTATS) {
			if (len > augLenMax) augLenMax = len;
			if (len < augLenMin) augLenMin = len;
		}
	}
	int64_t inline getAugLenMin() { return augLenMin; }
	int64_t inline getAugLenMax() { return augLenMax; }

private:
	double augs;
	double paugs;
	double growthS;
	double growthT;
	double orphans;
	double growthArcs;
	double pushes;
	double orphanArcs1;
	double orphanArcs2;
	double orphanArcs3;
	int64_t augLenMin;
	int64_t augLenMax;
};




template <typename captype, typename tcaptype, typename flowtype> class IBFSGraph
{
public:
	enum IBFSInitMode { IB_INIT_FAST, IB_INIT_COMPACT };
	IBFSGraph(IBFSInitMode initMode);
	~IBFSGraph();
	void setVerbose(bool a_verbose)
	{
		verbose = a_verbose;
	}
	void initSize(int64_t numNodes, int64_t numEdges);
	void reset();
	void addEdge(int64_t nodeIndexFrom, int64_t nodeIndexTo, captype capacity, captype reverseCapacity);
	void addNode(int64_t nodeIndex, tcaptype capFromSource, tcaptype capToSink);
	void incEdge(int64_t nodeIndexFrom, int64_t nodeIndexTo, captype capacity, captype reverseCapacity);
	void incNode(int64_t nodeIndex, tcaptype deltaCapFromSource, tcaptype deltaCapToSink);
	bool incShouldResetTrees();
	struct Arc;
	void incArc(Arc *a, captype deltaCap);
	void initGraph();
	flowtype computeMaxFlow();
	flowtype computeMaxFlow(bool allowIncrements);
	void resetTrees();
	void computeMinMarginals();
	void pushRelabel();


	inline bool getFileHasMore()
	{
		return fileHasMore;
	}
	inline IBFSStats getStats()
	{
		return stats;
	}
	inline flowtype getFlow()
	{
		return flow;
	}
	inline int64_t getNumNodes()
	{
		return nodeEnd - nodes;
	}
	inline int64_t getNumArcs()
	{
		return arcEnd - arcs;
	}
	int isNodeOnSrcSide(int64_t nodeIndex, int freeNodeValue = 0);


	struct Node;

	struct Arc
	{
		Node*		head;
		Arc*		rev;
		int			isRevResidual : 1;
		captype		rCap;
		//int			rCap :31; // they're using a bitfield?? TODO!!!
	};

	struct Node
	{
		int64_t		lastAugTimestamp;
		uint8_t		isParentCurr : 1;
		uint8_t		isIncremental : 1;
		Arc			*firstArc;
		Arc			*lastArc;
		Arc			*parent;
		Node		*firstSon;
		Node		*nextPtr;
		int64_t		label;	// label > 0: distance from s, label < 0: -distance from t
		tcaptype	excess;	 // excess > 0: capacity from s, excess < 0: -capacity to t
	};

private:
	int64_t init_n_nodes;
	int64_t init_n_edges;
	Arc *arcIter;
	void augment(Arc *bridge);
	template<bool sTree> int64_t augmentPath(Node *x, captype push);
	template<bool sTree> int64_t augmentExcess(Node *x, captype push);
	template<bool sTree> void augmentExcesses();
	template<bool sTree> void augmentDischarge(Node *x);
	template<bool sTree> void augmentExcessesDischarge();
	template<bool sTree> void augmentIncrements();
	template <bool sTree> void adoption(int64_t fromLevel, bool toTop);
	template <bool sTree> void adoption3Pass(int64_t minBucket);
	template <bool dirS> void growth();

	flowtype computeMaxFlow(bool trackChanges, bool initialDirS);
	void resetTrees(int64_t newTopLevelS, int64_t newTopLevelT);

	// push relabel
	template<bool sTree> void pushRelabelDischarge(Node *x);
	template<bool sTree> void pushRelabelGlobalUpdate();
	template<bool sTree> void pushRelabelDir();
	void pushRelabelShelve(int64_t fromLevel);

	class ActiveList
	{
	public:
		inline ActiveList()
		{
			list = NULL;
			len = 0;
		}
		inline void init(Node **mem)
		{
			list = mem;
			len = 0;
		}
		inline void clear()
		{
			len = 0;
		}
		inline void add(Node* x)
		{
			list[len] = x;
			len++;
		}
		inline Node* pop()
		{
			len--;
			return list[len];
		}
		inline Node** getEnd()
		{
			return list + len;
		}
		inline static void swapLists(ActiveList *a, ActiveList *b)
		{
			ActiveList tmp = (*a);
			(*a) = (*b);
			(*b) = tmp;
		}
		Node **list;
		int64_t len;
	};


#define IB_PREVPTR_EXCESS(x) (ptrs[(((x)-nodes)<<1) + 1])
#define IB_NEXTPTR_EXCESS(x) (ptrs[((x)-nodes)<<1])
#define IB_PREVPTR_3PASS(x) ((x)->firstSon)



	class BucketsOneSided
	{
	public:
		inline BucketsOneSided()
		{
			buckets = NULL;
			maxBucket = 0;
			nodes = NULL;
			allocLevels = 0;
		}
		inline void init(Node *a_nodes, int64_t numNodes)
		{
			nodes = a_nodes;
			allocLevels = numNodes / 8;
			if (allocLevels < IB_ALLOC_INIT_LEVELS) {
				if (numNodes < IB_ALLOC_INIT_LEVELS) allocLevels = numNodes;
				else allocLevels = IB_ALLOC_INIT_LEVELS;
			}
			buckets = new Node * [allocLevels + 1];
			memset(buckets, 0, sizeof(Node*) * (allocLevels + 1));
			maxBucket = 0;
		}
		inline void init_NoAlloc(Node *a_nodes, int64_t numNodes)
		{
			nodes = a_nodes;
			allocLevels = numNodes / 8;
			if (allocLevels < IB_ALLOC_INIT_LEVELS) {
				if (numNodes < IB_ALLOC_INIT_LEVELS) allocLevels = numNodes;
				else allocLevels = IB_ALLOC_INIT_LEVELS;
			}
			//buckets = new Node*[allocLevels + 1];
			memset(buckets, 0, sizeof(Node*) * (allocLevels + 1));
			maxBucket = 0;
		}
		inline void allocate(int64_t numLevels)
		{
			if (numLevels > allocLevels) {
				allocLevels <<= 1;
				Node **alloc = new Node * [allocLevels + 1];
				memset(alloc, 0, sizeof(Node*) * (allocLevels + 1));
				delete[]buckets;
				buckets = alloc;
			}
		}
		inline void free()
		{
			delete[]buckets;
			buckets = NULL;
		}
		template <bool sTree> inline void add(Node* x)
		{
			int64_t bucket = (sTree ? (x->label) : (-x->label));
			x->nextPtr = buckets[bucket];
			buckets[bucket] = x;
			if (bucket > maxBucket) maxBucket = bucket;
		}
		inline Node* popFront(int64_t bucket)
		{
			Node *x;
			if ((x = buckets[bucket]) == NULL) return NULL;
			buckets[bucket] = x->nextPtr;
			return x;
		}

		Node **buckets;
		int64_t maxBucket;
		Node *nodes;
		int64_t allocLevels;
	};



	class Buckets3Pass
	{
	public:
		inline Buckets3Pass()
		{
			buckets = NULL;
			nodes = NULL;
			maxBucket = allocLevels = -1;
		}
		inline void init(Node *a_nodes, int64_t numNodes)
		{
			nodes = a_nodes;
			allocLevels = numNodes / 8;
			if (allocLevels < IB_ALLOC_INIT_LEVELS) {
				if (numNodes < IB_ALLOC_INIT_LEVELS) allocLevels = numNodes;
				else allocLevels = IB_ALLOC_INIT_LEVELS;
			}
			buckets = new Node * [allocLevels + 1];
			memset(buckets, 0, sizeof(Node*) * (allocLevels + 1));
			maxBucket = 0;
		}
		inline void init_NoAlloc(Node *a_nodes, int64_t numNodes)
		{
			nodes = a_nodes;
			allocLevels = numNodes / 8;
			if (allocLevels < IB_ALLOC_INIT_LEVELS) {
				if (numNodes < IB_ALLOC_INIT_LEVELS) allocLevels = numNodes;
				else allocLevels = IB_ALLOC_INIT_LEVELS;
			}
			//buckets = new Node*[allocLevels + 1];
			memset(buckets, 0, sizeof(Node*) * (allocLevels + 1));
			maxBucket = 0;
		}
		inline void allocate(int64_t numLevels)
		{
			if (numLevels > allocLevels) {
				allocLevels <<= 1;
				Node **alloc = new Node * [allocLevels + 1];
				memset(alloc, 0, sizeof(Node*) * (allocLevels + 1));
				delete[]buckets;
				buckets = alloc;
			}
		}
		inline void free()
		{
			delete[]buckets;
			buckets = NULL;
		}
		template <bool sTree> inline void add(Node* x)
		{
			int64_t bucket = (sTree ? (x->label) : (-x->label));
			if ((x->nextPtr = buckets[bucket]) != NULL) IB_PREVPTR_3PASS(x->nextPtr) = x;
			buckets[bucket] = x;
			if (bucket > maxBucket) maxBucket = bucket;
		}
		inline Node* popFront(int64_t bucket)
		{
			Node *x = buckets[bucket];
			if (x == NULL) return NULL;
			buckets[bucket] = x->nextPtr;
			IB_PREVPTR_3PASS(x) = NULL;
			return x;
		}
		template <bool sTree> inline void remove(Node *x)
		{
			int64_t bucket = (sTree ? (x->label) : (-x->label));
			if (buckets[bucket] == x) {
				buckets[bucket] = x->nextPtr;
			} else {
				IB_PREVPTR_3PASS(x)->nextPtr = x->nextPtr;
				if (x->nextPtr != NULL) IB_PREVPTR_3PASS(x->nextPtr) = IB_PREVPTR_3PASS(x);
			}
			IB_PREVPTR_3PASS(x) = NULL;
		}
		inline bool isEmpty(int64_t bucket)
		{
			return buckets[bucket] == NULL;
		}

		Node **buckets;
		int64_t maxBucket;
		Node *nodes;
		int64_t allocLevels;
	};

	class ExcessBuckets
	{
	public:
		inline ExcessBuckets()
		{
			buckets = ptrs = NULL;
			nodes = NULL;
			allocLevels = maxBucket = minBucket = -1;
		}
		inline void init(Node *a_nodes, Node **a_ptrs, int64_t numNodes)
		{
			nodes = a_nodes;
			allocLevels = numNodes / 8;
			if (allocLevels < IB_ALLOC_INIT_LEVELS) {
				if (numNodes < IB_ALLOC_INIT_LEVELS) allocLevels = numNodes;
				else allocLevels = IB_ALLOC_INIT_LEVELS;
			}
			buckets = new Node * [allocLevels + 1];
			memset(buckets, 0, sizeof(Node*) * (allocLevels + 1));
			ptrs = a_ptrs;
			reset();
		}
		inline void init_NoAlloc(Node *a_nodes, Node **a_ptrs, int64_t numNodes)
		{
			nodes = a_nodes;
			allocLevels = numNodes / 8;
			if (allocLevels < IB_ALLOC_INIT_LEVELS) {
				if (numNodes < IB_ALLOC_INIT_LEVELS) allocLevels = numNodes;
				else allocLevels = IB_ALLOC_INIT_LEVELS;
			}
			//buckets = new Node*[allocLevels + 1];
			memset(buckets, 0, sizeof(Node*) * (allocLevels + 1));
			ptrs = a_ptrs;
			reset();
		}
		inline void allocate(int64_t numLevels)
		{
			if (numLevels > allocLevels) {
				allocLevels <<= 1;
				Node **alloc = new Node * [allocLevels + 1];
				memset(alloc, 0, sizeof(Node*) * (allocLevels + 1));
				//memcpy(alloc, buckets, sizeof(Node*)*(allocLevels+1));
				delete[]buckets;
				buckets = alloc;
			}
		}
		inline void free()
		{
			delete[]buckets;
			buckets = NULL;
		}

		template <bool sTree> inline void add(Node* x)
		{
			int64_t bucket = (sTree ? (x->label) : (-x->label));
			IB_NEXTPTR_EXCESS(x) = buckets[bucket];
			if (buckets[bucket] != NULL) {
				IB_PREVPTR_EXCESS(buckets[bucket]) = x;
			}
			buckets[bucket] = x;
			if (bucket > maxBucket) maxBucket = bucket;
			if (bucket != 0 && bucket < minBucket) minBucket = bucket;
		}
		inline Node* popFront(int64_t bucket)
		{
			Node *x = buckets[bucket];
			if (x == NULL) return NULL;
			buckets[bucket] = IB_NEXTPTR_EXCESS(x);
			return x;
		}
		template <bool sTree> inline void remove(Node *x)
		{
			int64_t bucket = (sTree ? (x->label) : (-x->label));
			if (buckets[bucket] == x) {
				buckets[bucket] = IB_NEXTPTR_EXCESS(x);
			} else {
				//IB_NEXTPTR_EXCESS(IB_PREVPTR_EXCESS(x)) = IB_NEXTPTR_EXCESS(x);
				Node * blah = IB_PREVPTR_EXCESS(x);
				Node * blah2 = IB_NEXTPTR_EXCESS(x);
				ptrs[((ptrs[((x - nodes) << 1) + 1]) - nodes) << 1] = ptrs[((x)-nodes) << 1];
				if (IB_NEXTPTR_EXCESS(x) != NULL) IB_PREVPTR_EXCESS(IB_NEXTPTR_EXCESS(x)) = IB_PREVPTR_EXCESS(x);
			}
		}
		inline void incMaxBucket(int64_t bucket)
		{
			if (maxBucket < bucket) maxBucket = bucket;
		}
		inline bool empty()
		{
			return maxBucket < minBucket;
		}
		inline void reset()
		{
			maxBucket = 0;
			minBucket = -1 ^ (1 << 31);
		}

		Node **buckets;
		Node **ptrs;
		int64_t maxBucket;
		int64_t minBucket;
		Node *nodes;
		int64_t allocLevels;
	};

	// members
	IBFSStats stats;
	Node	*nodes, *nodeEnd;
	Arc		*arcs, *arcEnd;
	Node	**ptrs;
	int64_t numNodes;
	flowtype flow;
	int64_t 	augTimestamp;
	int64_t topLevelS, topLevelT;
	ActiveList active0, activeS1, activeT1;
	Node **incList;
	int incLen;
	int incIteration;
	Buckets3Pass orphan3PassBuckets;
	BucketsOneSided orphanBuckets;
	ExcessBuckets excessBuckets;
	Buckets3Pass &prNodeBuckets;
	FILE *file;
	FILE *fileCompiled;
	bool fileIsCompiled;
	bool fileHasMore;
	bool verbose;
	flowtype testFlow;
	double testExcess;

	//
	// Orphans
	//
	int64_t uniqOrphansS, uniqOrphansT;
	template <bool sTree> inline void orphanFree(Node *x)
	{
		if (IB_EXCESSES && x->excess) {
			x->label = (sTree ? -topLevelT : topLevelS);
			if (sTree) activeT1.add(x);
			else activeS1.add(x);
			x->isParentCurr = 0;
		} else {
			x->label = 0;
		}
	}

	//
	// Initialization
	//
	struct TmpEdge
	{
		int64_t		head;
		int64_t		tail;
		captype		cap;
		captype		revCap;
	};
	struct TmpArc
	{
		TmpArc		*rev;
		captype		cap;
	};
	char	*memArcs;
	TmpEdge	*tmpEdges, *tmpEdgeLast;
	TmpArc	*tmpArcs;
	bool isInitializedGraph()
	{
		return memArcs != NULL;
	}
	IBFSInitMode initMode;
	void initGraphFast();
	void initGraphCompact();
	void initNodes();

	//
	// Testing
	//
	void testTree();
	void testPrint();
	void testExit()
	{
		exit(1);
	}
	inline void testNode(Node *x)
	{
		if (IBTEST && x - nodes == 110559) {
			IBDEBUG("*");
		}
	}
};




template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::addNode(int64_t nodeIndex, tcaptype capSource, tcaptype capSink)
{
	captype f = nodes[nodeIndex].excess;
	if (f > 0) {
		capSource += f;
	} else {
		capSink -= f;
	}
	if (capSource < capSink) {
		flow += capSource;
	} else {
		flow += capSink;
	}
	nodes[nodeIndex].excess = capSource - capSink;
}


// @pre: activeS1.len == 0 && activeT1.len == 0
template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::resetTrees()
{
	resetTrees(1, 1);
}

// @pre: activeS1.len == 0 && activeT1.len == 0
template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::resetTrees(int64_t newTopLevelS, int64_t newTopLevelT)
{
	uniqOrphansS = uniqOrphansT = 0;
	topLevelS = newTopLevelS;
	topLevelT = newTopLevelT;
	for (Node *y = nodes; y != nodeEnd; y++) {
		if (y->label < topLevelS && y->label > -topLevelT) continue;
		y->firstSon = NULL;
		if (y->label == topLevelS) activeS1.add(y);
		else if (y->label == -topLevelT) activeT1.add(y);
		else {
			y->parent = NULL;
			if (y->excess == 0) {
				y->label = 0;
			} else if (y->excess > 0) {
				y->label = topLevelS;
				activeS1.add(y);
			} else {
				y->label = -topLevelT;
				activeT1.add(y);
			}
		}
	}
}

template <typename captype, typename tcaptype, typename flowtype>
inline bool IBFSGraph<captype, tcaptype, flowtype>::incShouldResetTrees()
{
	return (uniqOrphansS + uniqOrphansT) >= 2 * numNodes; // TODO: Make sure uniqOrphansS + uniqOrphansT can be compared to int64_t
}

template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::incNode(int64_t nodeIndex, tcaptype deltaCapSource, tcaptype deltaCapSink)
{
	Node *x = (nodes + nodeIndex);

	// Initialize Incremental Phase
	if (incList == NULL) {
		// init list
		incList = active0.list;
		incLen = 0;

		// reset checks
		if (incShouldResetTrees()) {
			resetTrees(1, 1);
		}
	}

	// turn both deltas to positive
	if (deltaCapSource < 0) {
		flow += deltaCapSource;
		deltaCapSink -= deltaCapSource;
		deltaCapSource = 0;
	}
	if (deltaCapSink < 0) {
		flow += deltaCapSink;
		deltaCapSource -= deltaCapSink;
		deltaCapSink = 0;
	}

	// add delta
	addNode(nodeIndex, deltaCapSource, deltaCapSink);

	// add to incremental list
	if (!x->isIncremental) {
		incList[incLen++] = x;
		x->isIncremental = 1;
	}
}

template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::incArc(Arc *a, captype deltaCap)
{
	if (deltaCap == 0) return;
	if (a->rCap + a->rev->rCap + deltaCap < 0) { fprintf(stdout, "ERROR\n"); exit(1); }
	Node *x, *y;
	captype push;

	if (deltaCap > -a->rCap) {
		// there is enough residual capacity
		a->rCap += deltaCap;
		if (a->rCap == deltaCap) {
			// we added capcity (deltaCap > 0) and arc was just made residual
			x = a->rev->head;
			y = a->head;
			if (x->label > 0 && y->label == (x->label + 1) && y->isParentCurr && a->rev < y->parent) {
				y->isParentCurr = 0;
			} else if (x->label < 0 && y->label == (x->label + 1) && x->isParentCurr && a < x->parent) {
				x->isParentCurr = 0;
			} else if ((x->label > 0 && y->label <= 0)
				|| (x->label >= 0 && y->label < 0)
				|| (x->label > 0 && y->label > (x->label + 1))
				|| (x->label < (y->label - 1) && y->label < 0)) {
				// arc invalidates invariants - must saturate it
				a->rev->rCap += deltaCap;
				a->rCap = 0;
				flow -= deltaCap;
				incNode(x - nodes, 0, deltaCap);
				incNode(y - nodes, deltaCap, 0);
			}
		}
	} else {
		// there is not enough residual capacity
		// saturate the reverse arc
		x = a->rev->head;
		y = a->head;
		push = -(deltaCap + a->rCap);
		a->rev->rCap -= push;
		a->rCap = 0;
		flow -= push;
		incNode(y - nodes, 0, push);
		incNode(x - nodes, push, 0);
	}

	a->rev->isRevResidual = (a->rCap ? 1 : 0);
	a->isRevResidual = (a->rev->rCap ? 1 : 0);
}

template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::addEdge(int64_t nodeIndexFrom, int64_t nodeIndexTo, captype capacity, captype reverseCapacity)
{
	tmpEdgeLast->tail = nodeIndexFrom;
	tmpEdgeLast->head = nodeIndexTo;
	tmpEdgeLast->cap = capacity;
	tmpEdgeLast->revCap = reverseCapacity;
	tmpEdgeLast++;

	// use label as a temporary storage
	// to count the out degree of nodes
	nodes[nodeIndexFrom].label++;
	nodes[nodeIndexTo].label++;
}

template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::incEdge(int64_t nodeIndexFrom, int64_t nodeIndexTo, captype capacity, captype reverseCapacity)
{
	Node *x = nodes + nodeIndexFrom;
	Node *y = nodes + nodeIndexTo;
	if (arcIter == NULL || arcIter->rev->head != x) {
		arcIter = x->firstArc;
	}
	Arc *end = (x + 1)->firstArc;
	Arc *initIter = arcIter;
	if (arcIter->head != y)
		for ((++arcIter) == end && (arcIter = x->firstArc);
			arcIter != initIter;
			(++arcIter) == end && (arcIter = x->firstArc)) {
		if (arcIter->head == y) break;
	}
	if (arcIter->head != y) {
		fprintf(stdout, "Cannot increment arc (%d, %d)!\n", (int)(x - nodes), (int)(y - nodes));
		exit(1);
	}
	incArc(arcIter, capacity);
	incArc(arcIter->rev, reverseCapacity);
}


template <typename captype, typename tcaptype, typename flowtype>
inline int IBFSGraph<captype, tcaptype, flowtype>::isNodeOnSrcSide(int64_t nodeIndex, int freeNodeValue)
{
	if (nodes[nodeIndex].label == 0) {
		return freeNodeValue;
	}
	return (nodes[nodeIndex].label > 0 ? 1 : 0);
}


#define REMOVE_SIBLING(x, tmp) \
	{ (tmp) = (x)->parent->head->firstSon; \
	if ((tmp) == (x)) { \
		(x)->parent->head->firstSon = (x)->nextPtr; \
		} else { \
		for (; (tmp)->nextPtr != (x); (tmp) = (tmp)->nextPtr); \
		(tmp)->nextPtr = (x)->nextPtr; \
		} }

#define ADD_SIBLING(x, parentNode) \
	{ (x)->nextPtr = (parentNode)->firstSon; \
	(parentNode)->firstSon = (x); \
	}



template <typename captype, typename tcaptype, typename flowtype>
inline IBFSGraph<captype, tcaptype, flowtype>::IBFSGraph(IBFSInitMode a_initMode)
	:prNodeBuckets(orphan3PassBuckets)
{
	initMode = a_initMode;
	arcIter = NULL;
	incList = NULL;
	incLen = incIteration = 0;
	numNodes = 0;
	uniqOrphansS = uniqOrphansT = 0;
	augTimestamp = 0;
	verbose = IBTEST;
	arcs = arcEnd = NULL;
	nodes = nodeEnd = NULL;
	topLevelS = topLevelT = 0;
	flow = 0;
	memArcs = NULL;
	tmpArcs = NULL;
	tmpEdges = tmpEdgeLast = NULL;
	ptrs = NULL;
	testFlow = 0;
	testExcess = 0;
	file = fileCompiled = NULL;
	fileIsCompiled = false;
	fileHasMore = false;
}


template <typename captype, typename tcaptype, typename flowtype>
inline IBFSGraph<captype, tcaptype, flowtype>::~IBFSGraph()
{
	delete[]nodes;
	delete[]memArcs;
	orphanBuckets.free();
	orphan3PassBuckets.free();
	excessBuckets.free();
	if (file != NULL) fclose(file);
	if (fileCompiled != NULL) fclose(file);
}

template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::initGraph()
{
	if (initMode == IB_INIT_FAST) {
		initGraphFast();
	} else if (initMode == IB_INIT_COMPACT) {
		initGraphCompact();
	}
	topLevelS = topLevelT = 1;
}


template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::initSize(int64_t numNodes, int64_t numEdges)
{
	init_n_nodes = numNodes;
	init_n_edges = numEdges;
	// compute allocation size
	uint64_t arcTmpMemsize = (uint64_t)sizeof(TmpEdge) * (uint64_t)numEdges;
	uint64_t arcRealMemsize = (uint64_t)sizeof(Arc) * (uint64_t)(numEdges * 2);
	uint64_t nodeMemsize = (uint64_t)sizeof(Node**) * (uint64_t)(numNodes * 3) +
		(IB_EXCESSES ? ((uint64_t)sizeof(Node**) * (uint64_t)(numNodes * 2)) : 0);
	uint64_t arcMemsize = 0;
	if (initMode == IB_INIT_FAST) {
		arcMemsize = arcRealMemsize + arcTmpMemsize;
	} else if (initMode == IB_INIT_COMPACT) {
		arcTmpMemsize += (uint64_t)sizeof(TmpArc) * (uint64_t)(numEdges * 2);
		arcMemsize = arcTmpMemsize;
	}
	if (arcMemsize < (arcRealMemsize + nodeMemsize)) {
		arcMemsize = (arcRealMemsize + nodeMemsize);
	}

	// alocate arcs
	if (verbose) {
		fprintf(stdout, "c allocating arcs... \t [%lu MB]\n", (uint64_t)arcMemsize / (1 << 20));
		fflush(stdout);
	}
	memArcs = new char[arcMemsize];
	memset(memArcs, 0, (uint64_t)sizeof(char) * arcMemsize);
	if (initMode == IB_INIT_FAST) {
		tmpEdges = (TmpEdge*)(memArcs + arcRealMemsize);
	} else if (initMode == IB_INIT_COMPACT) {
		tmpEdges = (TmpEdge*)(memArcs);
		tmpArcs = (TmpArc*)(memArcs + arcMemsize - (uint64_t)sizeof(TmpArc) * (uint64_t)(numEdges * 2));
	}
	tmpEdgeLast = tmpEdges; // will advance as edges are added
	arcs = (Arc*)memArcs;
	arcEnd = arcs + numEdges * 2;

	// allocate nodes
	//	if (verbose) {
	//		fprintf(stdout, "c allocating nodes... \t [%lu MB]\n", (unsigned long)sizeof(Node)*(unsigned long)(numNodes+1) / (1<<20));
	//		fflush(stdout);
	//	}
	this->numNodes = numNodes;
	nodes = new Node[numNodes + 1];
	memset(nodes, 0, sizeof(Node) * (numNodes + 1));
	nodeEnd = nodes + numNodes;
	active0.init((Node**)(arcEnd));
	activeS1.init((Node**)(arcEnd)+numNodes);
	activeT1.init((Node**)(arcEnd)+(2 * numNodes));
	if (IB_EXCESSES) {
		ptrs = (Node**)(arcEnd)+(3 * numNodes);
		excessBuckets.init(nodes, ptrs, numNodes);
	}
	orphan3PassBuckets.init(nodes, numNodes);
	orphanBuckets.init(nodes, numNodes);

	// init members
	flow = 0;

	if (verbose) {
		fprintf(stdout, "c sizeof(ptr) = %d bytes\n", (int)sizeof(Node*));
		fprintf(stdout, "c sizeof(node) = %d bytes\n", (int)sizeof(Node));
		fprintf(stdout, "c sizeof(arc) = %d bytes\n", (int)sizeof(Arc));
	}
}
template <typename captype, typename tcaptype, typename flowtype>
inline  void IBFSGraph<captype, tcaptype, flowtype>::reset()
{
	// compute allocation size
	uint64_t arcTmpMemsize = (uint64_t)sizeof(TmpEdge) * (uint64_t)init_n_edges;
	uint64_t arcRealMemsize = (uint64_t)sizeof(Arc) * (uint64_t)(init_n_edges * 2);
	uint64_t nodeMemsize = (uint64_t)sizeof(Node**) * (uint64_t)(init_n_nodes * 3) +
		(IB_EXCESSES ? ((uint64_t)sizeof(Node**) * (uint64_t)(init_n_nodes * 2)) : 0);
	uint64_t arcMemsize = 0;
	if (initMode == IB_INIT_FAST) {
		arcMemsize = arcRealMemsize + arcTmpMemsize;
	} else if (initMode == IB_INIT_COMPACT) {
		arcTmpMemsize += (uint64_t)sizeof(TmpArc) * (uint64_t)(init_n_edges * 2);
		arcMemsize = arcTmpMemsize;
	}
	if (arcMemsize < (arcRealMemsize + nodeMemsize)) {
		arcMemsize = (arcRealMemsize + nodeMemsize);
	}

	// alocate arcs
	if (verbose) {
		fprintf(stdout, "c allocating arcs... \t [%lu MB]\n", (uint64_t)arcMemsize / (1 << 20));
		fflush(stdout);
	}
	memset(memArcs, 0, (uint64_t)sizeof(char) * arcMemsize);
	if (initMode == IB_INIT_FAST) {
		tmpEdges = (TmpEdge*)(memArcs + arcRealMemsize);
	} else if (initMode == IB_INIT_COMPACT) {
		tmpEdges = (TmpEdge*)(memArcs);
		tmpArcs = (TmpArc*)(memArcs + arcMemsize - (uint64_t)sizeof(TmpArc) * (uint64_t)(init_n_edges * 2));
	}
	tmpEdgeLast = tmpEdges; // will advance as edges are added
	arcs = (Arc*)memArcs;
	arcEnd = arcs + init_n_edges * 2;

	// allocate nodes
	//	if (verbose) {
	//		fprintf(stdout, "c allocating nodes... \t [%lu MB]\n", (unsigned long)sizeof(Node)*(unsigned long)(init_n_nodes+1) / (1<<20));
	//		fflush(stdout);
	//	}
	this->init_n_nodes = init_n_nodes;
	//nodes = new Node[init_n_nodes + 1];
	memset(nodes, 0, sizeof(Node) * (init_n_nodes + 1));
	nodeEnd = nodes + init_n_nodes;
	active0.init((Node**)(arcEnd));
	activeS1.init((Node**)(arcEnd)+init_n_nodes);
	activeT1.init((Node**)(arcEnd)+(2 * init_n_nodes));
	if (IB_EXCESSES) {
		ptrs = (Node**)(arcEnd)+(3 * init_n_nodes);
		excessBuckets.init_NoAlloc(nodes, ptrs, init_n_nodes);
	}
	orphan3PassBuckets.init_NoAlloc(nodes, init_n_nodes);
	orphanBuckets.init_NoAlloc(nodes, init_n_nodes);

	// init members
	flow = 0;

	if (verbose) {
		fprintf(stdout, "c sizeof(ptr) = %d bytes\n", (int)sizeof(Node*));
		fprintf(stdout, "c sizeof(node) = %d bytes\n", (int)sizeof(Node));
		fprintf(stdout, "c sizeof(arc) = %d bytes\n", (int)sizeof(Arc));
	}
}
template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::initNodes()
{
	Node *x;
	for (x = nodes; x <= nodeEnd; x++) {
		x->firstArc = (arcs + x->label);
		if (x->excess == 0) {
			x->label = 0;
			continue;
		}
		if (x->excess > 0) {
			x->label = 1;
			activeS1.add(x);
		} else {
			x->label = -1;
			activeT1.add(x);
		}
	}
}

template <typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::initGraphFast()
{
	Node *x;
	TmpEdge *te;
	Arc *a;

	// calculate start arc offsets and labels for every node
	nodes->firstArc = arcs;
	for (x = nodes; x != nodeEnd; x++) {
		(x + 1)->firstArc = x->firstArc + x->label;
		x->label = x->firstArc - arcs;
	}
	nodeEnd->label = arcEnd - arcs;

	// copy arcs
	for (te = tmpEdges; te != tmpEdgeLast; te++) {
		a = (nodes + te->tail)->firstArc;
		a->rev = (nodes + te->head)->firstArc;
		a->head = nodes + te->head;
		a->rCap = te->cap;
		a->isRevResidual = (te->revCap != 0);

		a = (nodes + te->head)->firstArc;
		a->rev = (nodes + te->tail)->firstArc;
		a->head = nodes + te->tail;
		a->rCap = te->revCap;
		a->isRevResidual = (te->cap != 0);

		++((nodes + te->head)->firstArc);
		++((nodes + te->tail)->firstArc);
	}

	initNodes();
}


template <typename captype, typename tcaptype, typename flowtype>
inline  void IBFSGraph<captype, tcaptype, flowtype>::initGraphCompact()
{
	Node *x;
	Arc *a;
	TmpArc *ta, *taEnd;
	TmpEdge *te;

	// tmpEdges:			edges read
	// node.label:			out degree

	// calculate start arc offsets every node
	nodes->firstArc = (Arc*)(tmpArcs);
	for (x = nodes; x != nodeEnd; x++) {
		(x + 1)->firstArc = (Arc*)(((TmpArc*)(x->firstArc)) + x->label);
		x->label = ((TmpArc*)(x->firstArc)) - tmpArcs;
	}
	nodeEnd->label = arcEnd - arcs;

	// tmpEdges:				edges read
	// node.label: 				index into arcs array of first out arc
	// node.firstArc-tmpArcs: 	index into arcs array of next out arc to be allocated
	//							(initially the first out arc)

	// copy to temp arcs memory
	if (IB_DEBUG_INIT) {
		IBDEBUG("c initFast copy1");
	}
	for (te = tmpEdges; te != tmpEdgeLast; te++) {
		ta = (TmpArc*)((nodes + te->tail)->firstArc);
		ta->cap = te->cap;
		ta->rev = (TmpArc*)((nodes + te->head)->firstArc);

		ta = (TmpArc*)((nodes + te->head)->firstArc);
		ta->cap = te->revCap;
		ta->rev = (TmpArc*)((nodes + te->tail)->firstArc);

		(nodes + te->tail)->firstArc = (Arc*)(((TmpArc*)((nodes + te->tail)->firstArc)) + 1);
		(nodes + te->head)->firstArc = (Arc*)(((TmpArc*)((nodes + te->head)->firstArc)) + 1);
	}

	// tmpEdges:				edges read
	// tmpArcs:					arcs with reverse pointer but no node id
	// node.label: 				index into arcs array of first out arc
	// node.firstArc-tmpArcs: 	index into arcs array of last allocated out arc

	// copy to permanent arcs array, but saving tail instead of head
	if (IB_DEBUG_INIT) {
		IBDEBUG("c initFast copy2");
	}
	a = arcs;
	x = nodes;
	taEnd = (tmpArcs + (arcEnd - arcs));
	for (ta = tmpArcs; ta != taEnd; ta++) {
		while (x->label <= (ta - tmpArcs)) x++;
		a->head = (x - 1);
		a->rCap = ta->cap;
		a->rev = arcs + (ta->rev - tmpArcs);
		a++;
	}

	// tmpEdges:				overwritten
	// tmpArcs:					overwritten
	// arcs:					arcs array
	// node.label: 				index into arcs array of first out arc
	// node.firstArc-tmpArcs: 	index into arcs array of last allocated out arc
	// arc.head = tail of arc

	// swap the head and tail pointers and set isRevResidual
	if (IB_DEBUG_INIT) {
		IBDEBUG("c initFast copy3");
	}
	for (a = arcs; a != arcEnd; a++) {
		if (a->rev <= a) continue;
		x = a->head;
		a->head = a->rev->head;
		a->rev->head = x;
		a->isRevResidual = (a->rev->rCap != 0);
		a->rev->isRevResidual = (a->rCap != 0);
	}

	// set firstArc pointers in nodes array
	if (IB_DEBUG_INIT) {
		IBDEBUG("c initFast nodes");
	}
	initNodes();

	// check consistency
	if (IBTEST || IB_DEBUG_INIT) {
		IBDEBUG("c initFast test");
		for (x = nodes; x != nodeEnd; x++) {
			if ((x + 1)->firstArc < x->firstArc) {
				fprintf(stderr, "INIT CONSISTENCY: arc pointers descending");
				exit(1);
			}
			for (a = x->firstArc; a != (x + 1)->firstArc; a++) {
				if (a->rev->head != x) {
					fprintf(stderr, "INIT CONSISTENCY: arc head pointer inconsistent");
					exit(1);
				}
				if (a->rev->rev != a) {
					fprintf(stderr, "INIT CONSISTENCY: arc reverse pointer inconsistent");
					exit(1);
				}
			}
		}
	}
}



// @ret: minimum orphan level
template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline  int64_t IBFSGraph<captype, tcaptype, flowtype>::augmentPath(Node *x, captype push)
{
	Node *y;
	Arc *a;
	int64_t orphanMinLevel = (sTree ? topLevelS : topLevelT) + 1;

	augTimestamp++;
	for (;; x = a->head) {
		stats.incPushes();
		if (x->excess) break;
		a = x->parent;
		if (sTree) {
			a->rCap += push;
			a->rev->isRevResidual = 1;
			a->rev->rCap -= push;
		} else {
			a->rev->rCap += push;
			a->isRevResidual = 1;
			a->rCap -= push;
		}

		// saturated?
		if ((sTree ? (a->rev->rCap) : (a->rCap)) == 0) {
			if (sTree) a->isRevResidual = 0;
			else a->rev->isRevResidual = 0;
			REMOVE_SIBLING(x, y);
			orphanMinLevel = (sTree ? x->label : -x->label);
			orphanBuckets.template add<sTree>(x);
		}
	}
	x->excess += (sTree ? -push : push);
	if (x->excess == 0) {
		orphanMinLevel = (sTree ? x->label : -x->label);
		orphanBuckets.template add<sTree>(x);
	}
	flow += push;

	return orphanMinLevel;
}


// @ret: minimum level in which created an orphan
template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline  int64_t IBFSGraph<captype, tcaptype, flowtype>::augmentExcess(Node *x, captype push)
{
	Node *y;
	Arc *a;
	int64_t orphanMinLevel = (sTree ? topLevelS : topLevelT) + 1;
	augTimestamp++;

	// start of loop
	//----------------
	// x 		  the current node along the path
	// a		  arc incoming into x
	// push 	  the amount of flow coming into x
	// a->resCap  updated with incoming flow already
	// x->excess  not updated with incoming flow yet
	//
	// end of loop
	//-----------------
	// x 		  the current node along the path
	// a		  arc outgoing from x
	// push 	  the amount of flow coming out of x
	// a->resCap  updated with outgoing flow already
	// x->excess  updated with incoming flow already
	while (sTree ? (x->excess <= 0) : (x->excess >= 0)) {
		testNode(x);
		stats.incPushes();
		a = x->parent;

		// update excess and find next flow
		if ((sTree ? (a->rev->rCap) : (a->rCap)) < (sTree ? (push - x->excess) : (x->excess + push))) {
			// some excess remains, node is an orphan
			x->excess += (sTree ? (a->rev->rCap - push) : (push - a->rCap));
			push = (sTree ? a->rev->rCap : a->rCap);
		} else {
			// all excess is pushed out, node may or may not be an orphan
			push += (sTree ? -(x->excess) : x->excess);
			x->excess = 0;
		}

		// push flow
		// note: push != 0
		if (sTree) {
			a->rCap += push;
			a->rev->isRevResidual = 1;
			a->rev->rCap -= push;
		} else {
			a->rev->rCap += push;
			a->isRevResidual = 1;
			a->rCap -= push;
		}

		// saturated?
		if ((sTree ? (a->rev->rCap) : (a->rCap)) == 0) {
			if (sTree) a->isRevResidual = 0;
			else a->rev->isRevResidual = 0;
			REMOVE_SIBLING(x, y);
			orphanMinLevel = (sTree ? x->label : -x->label);
			orphanBuckets.template add<sTree>(x);
			if (x->excess) excessBuckets.incMaxBucket(sTree ? x->label : -x->label);
		}

		// advance
		// a precondition determines that the first node on the path is not in excess buckets
		// so only the next nodes may need to be removed from there
		x = a->head;
		if (sTree ? (x->excess < 0) : (x->excess > 0)) excessBuckets.template remove<sTree>(x);
	}

	// update the excess at the root
	if (push <= (sTree ? (x->excess) : -(x->excess))) flow += push;
	else flow += (sTree ? (x->excess) : -(x->excess));
	x->excess += (sTree ? (-push) : push);
	if (sTree ? (x->excess <= 0) : (x->excess >= 0)) {
		orphanMinLevel = (sTree ? x->label : -x->label);
		orphanBuckets.template add<sTree>(x);
		if (x->excess) excessBuckets.incMaxBucket(sTree ? x->label : -x->label);
	}

	return orphanMinLevel;
}

template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline  void IBFSGraph<captype, tcaptype, flowtype>::augmentExcesses()
{
	Node *x;
	int64_t minOrphanLevel;
	int64_t adoptedUpToLevel = excessBuckets.maxBucket;

	if (!excessBuckets.empty())
		for (; excessBuckets.maxBucket != (excessBuckets.minBucket - 1); excessBuckets.maxBucket--)
			while ((x = excessBuckets.popFront(excessBuckets.maxBucket)) != NULL) {
				minOrphanLevel = augmentExcess<sTree>(x, 0);
				// if we did not create new orphans
				if (adoptedUpToLevel < minOrphanLevel) minOrphanLevel = adoptedUpToLevel;
				adoption<sTree>(minOrphanLevel, false);
				adoptedUpToLevel = excessBuckets.maxBucket;
			}
	excessBuckets.reset();
	if (orphanBuckets.maxBucket != 0) adoption<sTree>(adoptedUpToLevel + 1, true);
	// free 3pass orphans
	while ((x = excessBuckets.popFront(0)) != NULL) orphanFree<sTree>(x);
}

template <typename captype, typename tcaptype, typename flowtype>
inline  void IBFSGraph<captype, tcaptype, flowtype>::augment(Arc *bridge)
{
	Node *x, *y;
	Arc *a;
	captype bottleneck, bottleneckT, bottleneckS;
	int64_t minOrphanLevel;
	bool forceBottleneck;
	stats.incAugs();

	// must compute forceBottleneck once, so that it is constant throughout this method
	forceBottleneck = (IB_EXCESSES ? false : true);
	if (IB_BOTTLENECK_ORIG && IB_EXCESSES) {
		// limit by end nodes excess
		bottleneck = bridge->rCap;
		if (bridge->head->excess != 0 && -(bridge->head->excess) < bottleneck) {
			bottleneck = -(bridge->head->excess);
		}
		if (bridge->rev->head->excess != 0 && bridge->rev->head->excess < bottleneck) {
			bottleneck = bridge->rev->head->excess;
		}
	} else {
		bottleneck = bottleneckS = bridge->rCap;
		if (bottleneck != 1) {
			for (x = bridge->rev->head;; x = a->head) {
				if (x->excess) break;
				a = x->parent;
				if (bottleneckS > a->rev->rCap) {
					bottleneckS = a->rev->rCap;
				}
			}
			if (bottleneckS > x->excess) {
				bottleneckS = x->excess;
			}
			if (IB_EXCESSES && x->label != 1) forceBottleneck = true;
			if (x == bridge->rev->head) bottleneck = bottleneckS;
		}

		if (bottleneck != 1) {
			bottleneckT = bridge->rCap;
			for (x = bridge->head;; x = a->head) {
				if (x->excess) break;
				a = x->parent;
				if (bottleneckT > a->rCap) {
					bottleneckT = a->rCap;
				}
			}
			if (bottleneckT > (-x->excess)) {
				bottleneckT = (-x->excess);
			}
			if (IB_EXCESSES && x->label != -1) forceBottleneck = true;
			if (x == bridge->head && bottleneck > bottleneckT) bottleneck = bottleneckT;

			if (forceBottleneck) {
				if (bottleneckS < bottleneckT) bottleneck = bottleneckS;
				else bottleneck = bottleneckT;
			}
		}
	}

	// stats
	if (IBSTATS) {
		int64_t augLen = (-(bridge->head->label) - 1 + bridge->rev->head->label - 1 + 1);
		stats.addAugLen(augLen);
	}

	// augment connecting arc
	bridge->rev->rCap += bottleneck;
	bridge->isRevResidual = 1;
	bridge->rCap -= bottleneck;
	if (bridge->rCap == 0) {
		bridge->rev->isRevResidual = 0;
	}
	stats.incPushes();
	flow -= bottleneck;

	// augment T
	x = bridge->head;
	if (!IB_EXCESSES || bottleneck == 1 || forceBottleneck) {
		minOrphanLevel = augmentPath<false>(x, bottleneck);
		adoption<false>(minOrphanLevel, true);
	} else if (IB_ADOPTION_PR && !x->excess) {
		x->excess += bottleneck;
		excessBuckets.template add<false>(x);
		REMOVE_SIBLING(x, y);
		augmentExcessesDischarge<false>();
	} else {
		minOrphanLevel = augmentExcess<false>(x, bottleneck);
		adoption<false>(minOrphanLevel, false);
		augmentExcesses<false>();
	}

	// augment S
	x = bridge->rev->head;
	if (!IB_EXCESSES || bottleneck == 1 || forceBottleneck) {
		minOrphanLevel = augmentPath<true>(x, bottleneck);
		adoption<true>(minOrphanLevel, true);
	} else if (IB_ADOPTION_PR && !x->excess) {
		x->excess -= bottleneck;
		excessBuckets.template add<true>(x);
		REMOVE_SIBLING(x, y);
		augmentExcessesDischarge<true>();
	} else {
		minOrphanLevel = augmentExcess<true>(x, bottleneck);
		adoption<true>(minOrphanLevel, false);
		augmentExcesses<true>();
	}
}



template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline  void IBFSGraph<captype, tcaptype, flowtype>::adoption(int64_t fromLevel, bool toTop)
{
	Node *x, *y, *z;
	Arc *a;
	Arc *aEnd;
	int64_t threePassLevel;
	int64_t minLabel, numOrphans, numOrphansUniq;
	int64_t level;

	threePassLevel = 0;
	numOrphans = 0;
	numOrphansUniq = 0;
	for (level = fromLevel;
		level <= orphanBuckets.maxBucket &&
		(!IB_EXCESSES || toTop || threePassLevel || level <= excessBuckets.maxBucket);
		level++)
		while ((x = orphanBuckets.popFront(level)) != NULL) {
			testNode(x);
			stats.incOrphans();
			numOrphans++;
			if (x->lastAugTimestamp != augTimestamp) {
				x->lastAugTimestamp = augTimestamp;
				if (sTree) uniqOrphansS++;
				else uniqOrphansT++;
				numOrphansUniq++;
			}
			if (IB_HYBRID_ADOPTION && threePassLevel == 0 && numOrphans >= 3 * numOrphansUniq) {
				// switch to 3pass
				threePassLevel = 1;
			}

			//
			// check for same level connection
			//
			if (x->isParentCurr) {
				a = x->parent;
			} else {
				a = x->firstArc;
				x->isParentCurr = 1;
			}
			x->parent = NULL;
			aEnd = (x + 1)->firstArc;
			if (x->label != (sTree ? 1 : -1)) {
				minLabel = x->label - (sTree ? 1 : -1);
				for (; a != aEnd; a++) {
					stats.incOrphanArcs1();
					y = a->head;
					if ((sTree ? a->isRevResidual : a->rCap) != 0 && y->label == minLabel) {
						x->parent = a;
						ADD_SIBLING(x, y);
						break;
					}
				}
			}
			if (x->parent != NULL) {
				if (IB_EXCESSES && x->excess) excessBuckets.template add<sTree>(x);
				continue;
			}

			//
			// on the top level there is no need to relabel
			//
			if (x->label == (sTree ? topLevelS : -topLevelT)) {
				orphanFree<sTree>(x);
				continue;
			}

			//
			// give up on same level - relabel it!
			// (1) create orphan sons
			//
			for (y = x->firstSon; y != NULL; y = z) {
				stats.incOrphanArcs3();
				z = y->nextPtr;
				if (IB_EXCESSES && y->excess)
					excessBuckets.template remove<sTree>(y);
				orphanBuckets.template add<sTree>(y);
			}
			x->firstSon = NULL;

			//
			// (2) 3pass relabeling: move to buckets structure
			//
			if (threePassLevel) {
				x->label += (sTree ? 1 : -1);
				orphan3PassBuckets.template add<sTree>(x);
				if (threePassLevel == 1) {
					threePassLevel = level + 1;
				}
				continue;
			}

			//
			// (2) relabel: find the lowest level parent
			//
			minLabel = (sTree ? topLevelS : -topLevelT);
			if (x->label != minLabel) for (a = x->firstArc; a != aEnd; a++) {
				stats.incOrphanArcs2();
				y = a->head;
				if ((sTree ? a->isRevResidual : a->rCap) &&
					// y->label != 0 ---> holds implicitly
					(sTree ? (y->label > 0) : (y->label < 0)) &&
					(sTree ? (y->label < minLabel) : (y->label > minLabel))) {
					minLabel = y->label;
					x->parent = a;
					if (minLabel == x->label) break;
				}
			}

			//
			// (3) relabel onto new parent
			//
			if (x->parent != NULL) {
				x->label = minLabel + (sTree ? 1 : -1);
				ADD_SIBLING(x, x->parent->head);
				// add to active list of the next growth phase
				if (sTree) {
					if (x->label == topLevelS) activeS1.add(x);
				} else {
					if (x->label == -topLevelT) activeT1.add(x);
				}
				if (IB_EXCESSES && x->excess) excessBuckets.template add<sTree>(x);
			} else {
				orphanFree<sTree>(x);
			}
		}
	if (level > orphanBuckets.maxBucket) orphanBuckets.maxBucket = 0;

	if (threePassLevel) {
		adoption3Pass<sTree>(threePassLevel);
	}
}

template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline void IBFSGraph<captype, tcaptype, flowtype>::adoption3Pass(int64_t minBucket)
{
	Arc *a, *aEnd;
	Node *x, *y;
	int64_t minLabel, destLabel;

	for (int64_t level = minBucket; level <= orphan3PassBuckets.maxBucket; level++)
		while ((x = orphan3PassBuckets.popFront(level)) != NULL) {
			testNode(x);
			aEnd = (x + 1)->firstArc;

			// pass 2: find lowest level parent
			if (x->parent == NULL) {
				minLabel = (sTree ? topLevelS : -topLevelT);
				destLabel = x->label - (sTree ? 1 : -1);
				for (a = x->firstArc; a != aEnd; a++) {
					stats.incOrphanArcs3();
					y = a->head;
					if ((sTree ? a->isRevResidual : a->rCap) &&
						((sTree ? (y->excess > 0) : (y->excess < 0)) || y->parent != NULL) &&
						(sTree ? (y->label > 0) : (y->label < 0)) &&
						(sTree ? (y->label < minLabel) : (y->label > minLabel))) {
						x->parent = a;
						if ((minLabel = y->label) == destLabel) break;
					}
				}
				if (x->parent == NULL) {
					x->label = 0;
					if (IB_EXCESSES && x->excess) excessBuckets.template add<sTree>(x);
					continue;
				}
				x->label = minLabel + (sTree ? 1 : -1);
				if (x->label != (sTree ? level : -level)) {
					orphan3PassBuckets.template add<sTree>(x);
					continue;
				}
			}

			// pass 3: lower potential sons and/or find first parent
			if (x->label != (sTree ? topLevelS : -topLevelT)) {
				minLabel = x->label + (sTree ? 1 : -1);
				for (a = x->firstArc; a != aEnd; a++) {
					stats.incOrphanArcs3();
					y = a->head;

					// lower potential sons
					if ((sTree ? a->rCap : a->isRevResidual) &&
						(y->label == 0 ||
							(sTree ? (minLabel < y->label) : (minLabel > y->label)))) {
						if (y->label != 0) orphan3PassBuckets.template remove<sTree>(y);
						else if (IB_EXCESSES && y->excess) excessBuckets.template remove<sTree>(y);
						y->label = minLabel;
						y->parent = a->rev;
						orphan3PassBuckets.template add<sTree>(y);
					}
				}
			}

			// relabel onto new parent
			ADD_SIBLING(x, x->parent->head);
			x->isParentCurr = 0;
			if (IB_EXCESSES && x->excess) excessBuckets.template add<sTree>(x);

			// add to active list of the next growth phase
			if (sTree) {
				if (x->label == topLevelS) activeS1.add(x);
			} else {
				if (x->label == -topLevelT) activeT1.add(x);
			}
		}

	orphan3PassBuckets.maxBucket = 0;
}

template <typename captype, typename tcaptype, typename flowtype>
template<bool dirS>
inline void IBFSGraph<captype, tcaptype, flowtype>::growth()
{
	Node *x, *y;
	Arc *a, *aEnd;

	for (Node **active = active0.list; active != (active0.list + active0.len); active++) {
		// get active node
		x = (*active);
		testNode(x);

		// node no longer at level
		if (x->label != (dirS ? (topLevelS - 1) : -(topLevelT - 1))) {
			continue;
		}

		// grow or augment
		if (dirS) stats.incGrowthS();
		else stats.incGrowthT();
		aEnd = (x + 1)->firstArc;
		for (a = x->firstArc; a != aEnd; a++) {
			stats.incGrowthArcs();
			if ((dirS ? a->rCap : a->isRevResidual) == 0) continue;
			y = a->head;
			if (y->label == 0) {
				// grow node x (attach y)
				testNode(y);
				y->isParentCurr = 0;
				y->label = x->label + (dirS ? 1 : -1);
				y->parent = a->rev;
				ADD_SIBLING(y, x);
				if (dirS) activeS1.add(y);
				else activeT1.add(y);
			} else if (dirS ? (y->label < 0) : (y->label > 0)) {
				// augment
				augment(dirS ? a : (a->rev));
				if (x->label != (dirS ? (topLevelS - 1) : -(topLevelT - 1))) {
					break;
				}
				if (dirS ? (a->rCap) : (a->isRevResidual)) a--;
			}
		}
	}
	active0.clear();
}

template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline void IBFSGraph<captype, tcaptype, flowtype>::augmentIncrements()
{
	Node *x, *y;
	Node **end = incList + incLen;
	int64_t minOrphanLevel = 1 << 30;

	for (Node **inc = incList; inc != end; inc++) {
		x = (*inc);
		testNode(x);
		if (!x->isIncremental || (sTree ? (x->label < 0) : (x->label > 0))) continue;
		x->isIncremental = 0;
		if (x->label == 0) {
			//**** new root from outside the tree
			if (!x->excess) continue;
			x->isParentCurr = 0;
			if (x->excess > 0) {
				x->label = topLevelS;
				activeS1.add(x);
			} else if (x->excess < 0) {
				x->label = -topLevelT;
				activeT1.add(x);
			}
		} else if ((sTree ? (x->excess <= 0) : (x->excess >= 0)) &&
			(!x->parent || !(sTree ? x->parent->isRevResidual : x->parent->rCap))) {
			//**** new orphan
			if (x->parent) REMOVE_SIBLING(x, y);
			if ((sTree ? x->label : -x->label) < minOrphanLevel) {
				minOrphanLevel = (sTree ? x->label : -x->label);
			}
			orphanBuckets.template add<sTree>(x);
			if (x->excess) excessBuckets.incMaxBucket(sTree ? x->label : -x->label);
		} else if (sTree ? (x->excess < 0) : (x->excess > 0)) {
			//**** new deficit/excess to empty
			excessBuckets.template add<sTree>(x);
		} else if (x->excess && x->parent) {
			//**** new root
			REMOVE_SIBLING(x, y);
			x->parent = NULL;
			x->isParentCurr = 0;
		}
	}
	if (orphanBuckets.maxBucket != 0) adoption<sTree>(minOrphanLevel, false);
	if (IB_ADOPTION_PR) augmentExcessesDischarge<sTree>();
	else augmentExcesses<sTree>();
}


template<typename captype, typename tcaptype, typename flowtype>
inline flowtype IBFSGraph<captype, tcaptype, flowtype>::computeMaxFlow()
{
	return computeMaxFlow(true, false);
}

template<typename captype, typename tcaptype, typename flowtype>
inline flowtype IBFSGraph<captype, tcaptype, flowtype>::computeMaxFlow(bool allowIncrements)
{
	return computeMaxFlow(true, allowIncrements);
}

template<typename captype, typename tcaptype, typename flowtype>
inline flowtype IBFSGraph<captype, tcaptype, flowtype>::computeMaxFlow(bool initialDirS, bool allowIncrements)
{
	// incremental?
	if (incIteration >= 1 && incList != NULL) {
		augmentIncrements<true>();
		augmentIncrements<false>();
		incList = NULL;
	}

	// test
	if (IBTEST) {
		testFlow = flow;
		for (Node *x = nodes; x < nodeEnd; x++) {
			if (x->excess > 0) testExcess += x->excess;
		}
	}

	//
	// IBFS
	//
	bool dirS = initialDirS;
	while (true) {
		// BFS level
		if (dirS) {
			ActiveList::swapLists(&active0, &activeS1);
			topLevelS++;
		} else {
			ActiveList::swapLists(&active0, &activeT1);
			topLevelT++;
		}
		orphanBuckets.allocate((topLevelS > topLevelT) ? topLevelS : topLevelT);
		orphan3PassBuckets.allocate((topLevelS > topLevelT) ? topLevelS : topLevelT);
		if (IB_EXCESSES) excessBuckets.allocate((topLevelS > topLevelT) ? topLevelS : topLevelT);
		if (dirS)
			growth<true>();
		else
			growth<false>(); //second iteration 
		if (IBTEST) {
			testTree();
			//fprintf(stdout, "dirS=%d aug=%zd   S %zu / T %zu   flow=%lf\n", dirS, augTimestamp, uniqOrphansS, uniqOrphansT, flow);
			//fflush(stdout);
		}

		// switch to next level
		if (!allowIncrements && (activeS1.len == 0 || activeT1.len == 0)) break;
		if (activeS1.len == 0 && activeT1.len == 0) break;
		if (activeT1.len == 0) dirS = true;
		else if (activeS1.len == 0) dirS = false;
		else if (!IB_ALTERNATE_SMART && dirS) dirS = false;
		else if (IB_ALTERNATE_SMART && uniqOrphansT == uniqOrphansS && dirS) dirS = false;
		else if (IB_ALTERNATE_SMART && uniqOrphansT < uniqOrphansS) dirS = false;
		else dirS = true;
	}

	incIteration++;
	return flow;
}


///////////////////////////////////////////////////
// experimental min marginals
///////////////////////////////////////////////////
template<typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::computeMinMarginals()
{
	int *srcSide;
	Arc *a;
	Node *x;
	//	Node **inc, **end;
	stats.reset();

	// compute infinite capacity
	srcSide = new int[nodeEnd - nodes];
	//	int *flowDiffs = new int[nodeEnd-nodes];
	//	maxDeg = maxExcess = 0;
	for (x = nodes; x != nodeEnd; x++) {
		//		if (maxDeg < ((x+1)->firstArc - x->firstArc)) {
		//			maxDeg = ((x+1)->firstArc - x->firstArc);
		//		}
		//		if (x->excess > maxExcess) maxExcess = x->excess;
		//		else if (x->excess < -maxExcess) maxExcess = -x->excess;
		srcSide[x - nodes] = isNodeOnSrcSide(x - nodes, 2);
	}
	//	if (((double)maxDeg*(double)maxCap + (double)maxExcess + 1) > (double)(1<<30)) {
	//		fprintf(stdout, "ERROR: Infinite Capacity is too large %.0f\n",
	//				(double)maxDeg*(double)maxCap + (double)maxExcess + 1);
	//		exit(1);
	//	}
	//	infCap = maxDeg*maxCap + maxExcess + 1;

	// compute confidence
	//	incChangedList.init(numNodes);
	//	Arc *arcsCopy = new Arc[arcEnd-arcs];
	//	Node *nodesCopy = new Node[numNodes];

	//	for (x=nodes; x != nodeEnd; x++) {
	//		if (x->label > 0) {
	//			x->firstSon = NULL;
	//			x->parent = NULL;
	//			x->isParentCurr = 0;
	//			if (x->excess == 0) {
	//				x->label = 0;
	//			} else {
	//				x->label = 1;
	//			}
	//		}
	//	}
	//	topLevelS=1;

	flowtype flowCopy = flow;
	//	int topLevelSCopy = topLevelS;
	//	int topLevelTCopy = topLevelT;
	//	memcpy(arcsCopy, arcs, sizeof(Arc)*(arcEnd-arcs));
	//	memcpy(nodesCopy, nodes, sizeof(Node)*(numNodes));

	int64_t nEmpty = 0;
	for (int64_t nodeIndex = 0; nodeIndex < (nodeEnd - nodes); nodeIndex++) {
		if (srcSide[nodeIndex] == 2) {
			//			flowDiffs[nodeIndex] = 0;
			nEmpty++;
			continue;
		}

		//		//***** WHILE
		//		bool newCutHasSons = true;
		//		while (newCutHasSons) {
		//		int depth=0;
		captype infCap = (srcSide[nodeIndex] ? (nodes[nodeIndex].excess) : (-nodes[nodeIndex].excess));
		for (a = nodes[nodeIndex].firstArc; a != nodes[nodeIndex + 1].firstArc; a++) {
			if (srcSide[nodeIndex]) infCap += a->rev->rCap;
			else infCap += a->rCap;
		}
		if (srcSide[nodeIndex]) incNode(nodeIndex, 0, infCap);
		else incNode(nodeIndex, infCap, 0);
		flowtype flowDiff = computeMaxFlow(false, !srcSide[nodeIndex]) - flowCopy;
		if (flowDiff == infCap || flowDiff == -infCap) nEmpty++;
		//		testTree();

		//		srcSide[nodeIndex]|=8;
		//		newCutHasSons = false;
		//		if (nodes[nodeIndex].firstSon == NULL) nEmpty++;
		//		for (x=nodes[nodeIndex].firstSon; false && x != NULL; x=x->nextPtr)
		//		{
		//			if ((srcSide[x-nodes]&8)==0 && (srcSide[x-nodes]%4)==(srcSide[nodeIndex]%4))
		//			{
		//				depth++;
		//				if (depth >= 2) fprintf(stdout, "DEPTH %d\n", depth);
		//				if ((srcSide[nodeIndex]%4)) incNode(nodeIndex, 0, -infCap);
		//				else incNode(nodeIndex, -infCap, 0);
		//				nodeIndex = x-nodes;
		//				newCutHasSons = true;
		//				break;
		//			}
		//		}
		//		} //***** END WHILE


		if (IB_MIN_MARGINALS_DEBUG && (incIteration % ((nodeEnd - nodes) / 10) == 0)) {
			int64_t minLabelS = topLevelS;
			int64_t minLabelT = topLevelT;
			for (x = nodes; x != nodeEnd; x++) {
				if (x->label > 0 && x->label < minLabelS) minLabelS = x->label;
				else if (x->label < 0 && -x->label < minLabelT) minLabelT = -x->label;
			}
			//fprintf(stdout, "%zd (%zd>%zd, %zd>%zd, %lf) ", (int)(100 * incIteration / (nodeEnd - nodes)),minLabelS, topLevelS, minLabelT, topLevelT, flow);
			//fflush(stdout);
		}

		//		end=incChangedList.list + incChangedList.len;
		//		int maxLabel=0;
		//		for (inc=incChangedList.list; inc != end; inc++) {
		//			x = *inc;
		//			int label = (x->label > 0 ? x->label : (-x->label));
		//			if (label > maxLabel) maxLabel=label;
		//			(*x) = nodesCopy[(*inc)-nodes];
		//			memcpy(x->firstArc, arcsCopy+(x->firstArc-arcs), ((x+1)->firstArc-x->firstArc)*sizeof(Arc));
		//		}
		//		if (maxLabel > 0) memset(orphanBuckets.buckets, 0, sizeof(Node*)*(maxLabel+1));
		//		topLevelS = topLevelSCopy;
		//		topLevelT = topLevelTCopy;
		//		activeS1.len = 0;
		//		activeT1.len = 0;
		//		flow = flowCopy;
		//		incChangedList.len = 0;

		//		IBSTOP;
		//		double t = IBSECS;
		//		if (t >= 0.01) {
		//			fprintf(stdout, "\n iteration %d  time %f\n", incIteration-1, t);
		//			fflush(stdout);
		//		}

		if (srcSide[nodeIndex]) incNode(nodeIndex, 0, -infCap);
		else incNode(nodeIndex, -infCap, 0);
	}
	fprintf(stdout, "\n");
	//	if (IB_MIN_MARGINALS_TEST) {
	//		for (int i=0; i < numNodes; i++) {
	//			fprintf(stdout, "%d\n", flowDiffs[i]);
	//		}
	//	}
	fprintf(stdout, "c trivial = %f\n", nEmpty / (long double)numNodes);
	delete[]srcSide;
}



///////////////////////////////////////////////////
// experimental push relabel orphan processing
///////////////////////////////////////////////////
template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline void IBFSGraph<captype, tcaptype, flowtype>::augmentExcessesDischarge()
{
	Node *x;
	if (!excessBuckets.empty())
		for (; excessBuckets.maxBucket != (excessBuckets.minBucket - 1); excessBuckets.maxBucket--)
			while ((x = excessBuckets.popFront(excessBuckets.maxBucket)) != NULL) {
				augmentDischarge<sTree>(x);
			}
	excessBuckets.reset();
	while ((x = excessBuckets.popFront(0)) != NULL) {
		x->isIncremental = 0;
		orphanBuckets.template add<sTree>(x);
		// TODO: add orphan min level optimization here
	}
	augTimestamp++;
	adoption<sTree>(1, true);
}

// @pre: !x->isIncremental && x not in excessBuckets[0] && x not in x->parent sons list
template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline void IBFSGraph<captype, tcaptype, flowtype>::augmentDischarge(Node *x)
{
	Node *y, *z;
	int64_t minLabel;
	flowtype push;
	Arc *aEnd = (x + 1)->firstArc;
	Arc *a;
	int64_t startLabel = x->label;
	testNode(x);

	// loop
	while (true) {
		// push
		if (x->isParentCurr) {
			a = x->parent;
		} else {
			a = x->firstArc;
			x->isParentCurr = 1;
		}
		if (x->label != (sTree ? 1 : -1) && a != NULL) {
			minLabel = x->label + (sTree ? -1 : 1);
			for (; a != aEnd; a++) {
				// check admissible
				y = a->head;
				if ((sTree ? a->isRevResidual : a->rCap) == 0 || y->label != minLabel) {
					continue;
				}

				// push admissible
				push = (sTree ? (a->rev->rCap) : (a->rCap));
				if (push > (sTree ? (-x->excess) : (x->excess))) {
					push = (sTree ? (-x->excess) : (x->excess));
				}
				x->excess += (sTree ? push : (-push));
				if (sTree) {
					a->rev->rCap -= push;
					a->rCap += push;
					a->rev->isRevResidual = 1;
					a->isRevResidual = (a->rev->rCap ? 1 : 0);
				} else {
					a->rCap -= push;
					a->rev->rCap += push;
					a->rev->isRevResidual = (a->rCap ? 1 : 0);
					a->isRevResidual = 1;
				}

				// add excess
				if (sTree && y->excess > 0) {
					if (y->excess >= push) flow += push;
					else flow += y->excess;
				} else if (!sTree && y->excess < 0) {
					if (-y->excess >= push) flow += push;
					else flow -= y->excess;
				}
				y->excess += (sTree ? (-push) : push);
				if (y->excess == 0 /* implicit && !y->isIncremental && y has no parent */) {
					y->label = 0;
					excessBuckets.template add<sTree>(y);
					y->label = minLabel;
					y->isIncremental = 1;
				} else if (sTree ? (y->excess < 0 && y->excess >= -push) : (y->excess > 0 && y->excess <= push)) {
					if (y->isIncremental) {
						y->label = 0;
						excessBuckets.template remove<sTree>(y);
						y->label = minLabel;
						y->isIncremental = 0;
					} else if (y->parent != NULL) {
						REMOVE_SIBLING(y, z);
					}
					excessBuckets.template add<sTree>(y);
				}
				if (x->excess == 0) {
					x->parent = a;
					if (!(sTree ? a->isRevResidual : a->rCap)) {
						x->label = 0;
						excessBuckets.template add<sTree>(x);
						x->label = minLabel + (sTree ? 1 : -1);
						x->isIncremental = 1;
					}
					break;
				}
			}
		}
		if (x->excess == 0) break;

		// make sons orphans
		minLabel = x->label + (sTree ? 1 : -1);
		for (y = x->firstSon; y != NULL; y = z) {
			stats.incOrphanArcs3();
			z = y->nextPtr;
			// implicit !y->isIncremental && !y->excess
			y->label = 0;
			excessBuckets.template add<sTree>(y);
			y->label = minLabel;
			y->isIncremental = 1;
		}
		x->firstSon = NULL;

		// relabel
		minLabel = (sTree ? topLevelS : -topLevelT);
		x->parent = NULL;
		for (a = x->firstArc; a != aEnd; a++) {
			y = a->head;
			if ((sTree ? a->isRevResidual : a->rCap) &&
				// y->label != 0 ---> holds implicitly
				(sTree ? (y->label > 0) : (y->label < 0)) &&
				(sTree ? (y->label < minLabel) : (y->label > minLabel))) {
				minLabel = y->label;
				x->parent = a;
				if (minLabel == x->label) break;
			}
		}
		if (x->parent != NULL) {
			x->label = minLabel + (sTree ? 1 : -1);
		} else {
			orphanFree<sTree>(x);
			break;
		}
	}

	// set new parent
	if (x->parent != NULL && !x->isIncremental) ADD_SIBLING(x, x->parent->head);
	if (sTree) {
		if (startLabel != x->label && x->label == topLevelS) activeS1.add(x);
	} else {
		if (startLabel != x->label && x->label == -topLevelT) activeT1.add(x);
	}
}




///////////////////////////////////////////////////
// testing/debugging
///////////////////////////////////////////////////
template<typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::testTree()
{
	Node *x, *y;
	Arc *a;
	double totalExcess = 0;

	for (x = nodes; x != nodeEnd; x++) {
		if (x->label > topLevelS || x->label < -topLevelT) {
			IBDEBUG("ILLEGAL LABEL!");
			testExit();
		}
		if (x->label == 0) {
			if (x->excess) {
				IBDEBUG("EXCESS OUTSIDE!");
				testExit();
			}
			continue;
		}
		bool sTree = (x->label > 0);
		if (sTree ? (x->excess < 0) : (x->excess > 0)) {
			IBDEBUG("EXCESS ON WRONG SIDE!");
			testExit();
		}
		if (sTree && x->excess > 0) totalExcess += x->excess;
		if (!x->excess && x->parent == NULL) {
			IBDEBUG("NO PARENT!");
			testExit();
		}
		if (!x->excess && x->parent->head->label != x->label + (sTree ? -1 : 1)) {
			IBDEBUG("ILLEGAL PARENT!");
			testExit();
		}
		if (x->label == (sTree ? topLevelS : -topLevelT)) {
			int64_t k = 0;
			for (; k < (sTree ? activeS1 : activeT1).len; k++) {
				if ((sTree ? activeS1 : activeT1).list[k] == x) break;
			}
			if (k == (sTree ? activeS1 : activeT1).len && incIteration == 1) {
				IBDEBUG("NOT ACTIVE!");
				testExit();
			}
			continue;
		}
		for (y = x->firstSon; y != NULL; y = y->nextPtr) {
			if (y->parent->head != x) {
				IBDEBUG("ILLEGAL SIBLING!");
				testExit();
			}
		}
		for (a = x->firstArc; a != (x + 1)->firstArc; a++) {
			if (x->isParentCurr &&
				(sTree ? a->isRevResidual : a->rCap) &&
				(sTree ? (a->head->label > 0) : (a->head->label < 0)) &&
				a->head->label == (sTree ? (x->label - 1) : (x->label + 1)) &&
				a < x->parent) {
				IBDEBUG("ILLEGAL CURRENT ARC!");
				testExit();
			}
			if (!(sTree ? a->rCap : a->isRevResidual)) continue;
			if (a->head->label > topLevelS || a->head->label < -topLevelT) {
				IBDEBUG("ILLEGAL LABEL!");
				testExit();
			}
			if (a->head->label == 0 || (a->head->parent == NULL && a->head->excess == 0)) {
				IBDEBUG("CROSS OUT NODE!");
				testExit();
			}
			if (sTree ? (a->head->label < 0) : (a->head->label > 0)) {
				IBDEBUG("CROSS NODE!");
				testExit();
			}
			if (sTree ? (a->head->label > (x->label + 1)) : (a->head->label < (x->label - 1))) {
				IBDEBUG("EXTENDED ARC!");
				testExit();
			}
		}
	}

	if ((int64_t)(testExcess - totalExcess) != (int64_t)(flow - testFlow)) {
		//		IBDEBUG("ILLEGAL FLOW!");
		//		testExit();
	}
}

template<typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::testPrint()
{
	uint64_t *nums = new uint64_t[numNodes];
	memset(nums, 0, sizeof(uint64_t) * numNodes);
	for (Node *x = nodes; x != nodeEnd; x++) {
		if (x->label >= 0) {
			nums[x->label]++;
		} else {
			nums[numNodes + x->label]++;
		}
	}
	fprintf(stdout, "S = ");
	for (int64_t i = 1; i <= topLevelS; i++) {
		fprintf(stdout, "%zd ", nums[i]);
	}
	fprintf(stdout, "\nT = ");
	for (int64_t i = 1; i <= topLevelT; i++) {
		fprintf(stdout, "%zd ", nums[numNodes - i]);
	}
	delete[]nums;
	fprintf(stdout, "\n");
	fflush(stdout);
}




///////////////////////////////////////////////////
// push relabel implementation
///////////////////////////////////////////////////
template<typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::pushRelabelShelve(int64_t fromLevel)
{
	Node *x = NULL;
	for (int64_t bucket = fromLevel; bucket <= prNodeBuckets.maxBucket; bucket++) {
		if (prNodeBuckets.isEmpty(bucket)) continue;
		//		if (x == NULL) prNodeShelves.add(prNodeBuckets.buckets[bucket]);
		//		else x->nextPtr = prNodeBuckets.buckets[bucket];
		//		for (x=prNodeBuckets.buckets[bucket]; x->nextPtr != NULL; x = x->nextPtr) {
		//			x->label = -(x->label-fromLevel);
		//		}
		//		x->label = -(x->label-fromLevel);
		for (x = prNodeBuckets.buckets[bucket]; x != NULL; x = x->nextPtr) x->label = 0;
	}
	int64_t numLevels = prNodeBuckets.maxBucket - fromLevel + 1;
	memset(prNodeBuckets.buckets + fromLevel, 0, sizeof(Node*) * numLevels);
	memset(excessBuckets.buckets + fromLevel, 0, sizeof(Node*) * numLevels);
	prNodeBuckets.maxBucket = fromLevel - 1;
	excessBuckets.maxBucket = fromLevel - 1;
}

//template <bool sTree> void IBFSGraph::prUnshelve(int fromLevel)
//{
//	Node *next;
//	for (Node *x = prNodeShelves.pop(); x != NULL; x=next) {
//		next = x->nextPtr;
//		x->label = fromLevel - x->label;
//		prNodeBuckets.template add<sTree>(x);
//		if (x->excess) excessBuckets.template add<sTree>(x);
//	}
//}

template<typename captype, typename tcaptype, typename flowtype>
inline void IBFSGraph<captype, tcaptype, flowtype>::pushRelabel()
{
	return pushRelabelDir<false>();
}

template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline void IBFSGraph<captype, tcaptype, flowtype>::pushRelabelDir()
{
	Node *x;
	int64_t level;

	// init
	topLevelS = topLevelT = numNodes;
	pushRelabelGlobalUpdate<sTree>();

	// main loop
	uint64_t nDischarges = 0;
	for (; excessBuckets.maxBucket >= excessBuckets.minBucket; excessBuckets.maxBucket--)
		while ((x = excessBuckets.popFront(excessBuckets.maxBucket)) != NULL) {
			// discharge
			level = excessBuckets.maxBucket; // excessBuckets.maxBucket may change in discharge()
			pushRelabelDischarge<sTree>(x);
			nDischarges++;
			if (prNodeBuckets.maxBucket < level) {
				excessBuckets.allocate(level + 2);
				prNodeBuckets.allocate(level + 2);
			}

			// global update / gap heuristic
			if (nDischarges % (30 * numNodes) == 0) pushRelabelGlobalUpdate<sTree>();
			else if (prNodeBuckets.isEmpty(level)) pushRelabelShelve(level + 1);
		}
}

template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline void IBFSGraph<captype, tcaptype, flowtype>::pushRelabelGlobalUpdate()
{
	Node *x, *y;
	Arc *a, *aEnd;

	memset(prNodeBuckets.buckets, 0, sizeof(Node*) * (prNodeBuckets.allocLevels + 1));
	memset(excessBuckets.buckets, 0, sizeof(Node*) * (excessBuckets.allocLevels + 1));
	prNodeBuckets.maxBucket = 1;
	excessBuckets.reset();
	for (x = nodes; x != nodeEnd; x++) {
		x->parent = NULL;
		x->isParentCurr = 0;
		if ((sTree ? (x->excess > 0) : (x->excess < 0))) {
			x->label = (sTree ? 1 : -1);
			prNodeBuckets.template add<sTree>(x);
		} else x->label = 0;
	}
	for (int64_t bucket = 1; bucket <= prNodeBuckets.maxBucket; bucket++)
		for (x = prNodeBuckets.buckets[bucket]; x != NULL; x = x->nextPtr) {
			aEnd = (x + 1)->firstArc;
			for (a = x->firstArc; a != aEnd; a++) {
				if (!(sTree ? a->rCap : a->isRevResidual)) continue;
				y = a->head;
				if (y->parent != NULL || (sTree ? (y->excess > 0) : (y->excess < 0))) continue;
				y->label = (sTree ? (bucket + 1) : (-bucket - 1));
				prNodeBuckets.template add<sTree>(y);
				y->parent = a->rev;
				if (y->excess) excessBuckets.template add<sTree>(y);
			}
		}
}

template <typename captype, typename tcaptype, typename flowtype>
template<bool sTree>
inline void IBFSGraph<captype, tcaptype, flowtype>::pushRelabelDischarge(Node *x)
{
	Node *y;
	int64_t minLabel;
	captype push;
	Arc *aEnd = (x + 1)->firstArc;
	Arc *a;

	testNode(x);
	prNodeBuckets.template remove<sTree>(x);
	while (true) {
		// push
		if (x->isParentCurr) {
			a = x->parent;
		} else {
			a = x->firstArc;
			x->isParentCurr = 1;
		}
		if (x->label != (sTree ? 1 : -1)) {
			minLabel = x->label - (sTree ? 1 : -1);
			for (; a != aEnd; a++) {
				// check admissible
				y = a->head;
				if ((sTree ? a->isRevResidual : a->rCap) == 0 || y->label != minLabel) {
					continue;
				}

				// push admissible
				push = (sTree ? (a->rev->rCap) : (a->rCap));
				if (push > (sTree ? (-x->excess) : (x->excess))) {
					push = (sTree ? (-x->excess) : (x->excess));
				}
				x->excess += (sTree ? push : (-push));
				if (sTree) {
					a->rev->rCap -= push;
					a->rCap += push;
					a->rev->isRevResidual = 1;
					a->isRevResidual = (a->rev->rCap ? 1 : 0);
				} else {
					a->rCap -= push;
					a->rev->rCap += push;
					a->rev->isRevResidual = (a->rCap ? 1 : 0);
					a->isRevResidual = 1;
				}

				// add excess
				if (sTree && y->excess > 0) {
					if (y->excess >= push) flow += push;
					else flow += y->excess;
				} else if (!sTree && y->excess < 0) {
					if (-y->excess >= push) flow += push;
					else flow -= y->excess;
				}
				y->excess += (sTree ? (-push) : push);
				if (sTree ? (y->excess < 0 && y->excess >= -push) : (y->excess > 0 && y->excess <= push)) {
					excessBuckets.template add<sTree>(y);
				}
				if (x->excess == 0) {
					x->parent = a;
					break;
				}
			}
		}
		if (x->excess == 0) break;

		// relabel
		minLabel = (sTree ? ((int64_t)(numNodes)-1) : (-(int64_t)(numNodes)+1));
		x->parent = NULL;
		for (a = x->firstArc; a != aEnd; a++) {
			y = a->head;
			if ((sTree ? a->isRevResidual : a->rCap) &&
				// y->label != 0 ---> holds implicitly
				(sTree ? (y->label > 0) : (y->label < 0)) &&
				(sTree ? (y->label < minLabel) : (y->label > minLabel))) {
				minLabel = y->label;
				x->parent = a;
				if (minLabel == x->label) break;
			}
		}
		if (x->parent != NULL) {
			x->label = minLabel + (sTree ? 1 : -1);
		} else {
			x->label = 0;
			break;
		}
	}
	if (x->label != 0) prNodeBuckets.template add<sTree>(x);
}
template class IBFSGraph<int64_t, int64_t, int64_t>;

} // namespace ibfs

#endif // IBFS_H__

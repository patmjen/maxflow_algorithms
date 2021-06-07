/*
This software has been modified by Hossam Isack <isack.hossam@gmail.com> and Karin Ng <karinng10@gmail.com>,
to handle large graphs and to allow graph rests (to avoid reallocating memeory when weights change).
It has further been updated by Patrick M. Jensen <patmjen@dtu.dk> and Niels Jeppesen <niejep@dtu.dk>
with additional performance optimizations and a switch to using indices instead of pointers.
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

#ifndef REIMPLS_IBFS_H__
#define REIMPLS_IBFS_H__

#include <cstdio>
#include <string>
#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <cassert>

#include "util.h"

namespace reimpls {

template <class Cap, class Term, class Flow, class NodeIdx = uint32_t, class ArcIdx = uint32_t>
class IBFSGraph {
    static constexpr size_t ALLOC_INIT_LEVELS = 4096;

    static const NodeIdx INVALID_NODE = ~NodeIdx(0); // -1 for signed type, max. value for unsigned type
    static const ArcIdx INVALID_ARC = ~ArcIdx(0); // -1 for signed type, max. value for unsigned type

    using Dist = std::make_signed_t<NodeIdx>;

    // Forward decls.
public:
    struct Node;
    struct Arc;
private:
    struct TmpEdge;
    class ActiveList;
    class BucketsOneSided;
    class Buckets3Pass;
    class ExcessBuckets;

public:
    IBFSGraph();
    IBFSGraph(int64_t numNodes, int64_t numEdges);
    ~IBFSGraph();
    void initSize(int64_t numNodes, int64_t numEdges);
    void reset();
    void addEdge(NodeIdx from, NodeIdx to, Cap capacity, Cap revCapacity);
    void addNode(NodeIdx node, Term capSource, Term capSink);
    void incEdge(NodeIdx from, NodeIdx fo, Cap capacity, Cap revCapacity);
    void incNode(NodeIdx node, Term deltaCapSource, Term deltaCapSink);
    bool incShouldResetTrees();
    void incArc(ArcIdx arc, Cap deltaCap);
    void initGraph();
    Flow computeMaxFlow();
    Flow computeMaxFlow(bool allowIncrements);
    void resetTrees();

    inline Flow getFlow() const noexcept { return flow; }
    inline size_t getNumNodes() const noexcept { return nodeEnd - nodes; }
    inline size_t getNumArcs() const noexcept { return arcEnd - arcs; }
    int isNodeOnSrcSide(NodeIdx node, int freeNodeValue = 0);

#pragma pack (1)
    struct REIMPLS_PACKED Arc {
        NodeIdx head;
        ArcIdx rev;
        Cap    rCap;
        bool isRevResidual;
    };

    struct REIMPLS_PACKED Node {
        int32_t lastAugTimestamp;
        bool isParentCurr : 1;
        bool isIncremental : 1;
        ArcIdx firstArc;
        ArcIdx parent;
        NodeIdx firstSon;
        NodeIdx nextNode;
        Dist label; // label > 0: distance from s, label < 0: -distance from t
        Term excess; // excess > 0: capacity from s, excess < 0: -capacity to t
    };
#pragma pack ()

private:
    int64_t init_n_nodes;
    int64_t init_n_edges;
    Arc *arcIter;

    Node *nodes, *nodeEnd;
    Arc *arcs, *arcEnd;
    NodeIdx *ptrs;
    int64_t numNodes;
    Flow flow;
    int64_t augTimestamp;
    int64_t topLevelS, topLevelT;
    ActiveList active0, activeS1, activeT1;
    NodeIdx *incList;
    int incLen;
    int incIteration;
    Buckets3Pass orphan3PassBuckets;
    BucketsOneSided orphanBuckets;
    ExcessBuckets excessBuckets;
    Flow testFlow;
    double testExcess;
    char *memArcs;
    TmpEdge *tmpEdges, *tmpEdgeLast;

    void augment(ArcIdx bridge);
    template <bool sTree> int64_t augmentPath(NodeIdx i, Cap push);
    template <bool sTree> int64_t augmentExcess(NodeIdx i, Cap push);
    template <bool sTree> void augmentExcesses();
    template <bool sTree> void augmentIncrements();
    template <bool sTree> void adoption(int64_t fromLevel, bool toTop);
    template <bool sTree> void adoption3Pass(int64_t minBucket);
    template <bool dirS> void growth();

    Flow computeMaxFlow(bool trackChanges, bool initialDirS);
    void resetTrees(int64_t newTopLevelS, int64_t newTopLevelT);

    void remove_sibling(NodeIdx i);
    void add_sibling(NodeIdx i, NodeIdx parent);

    inline NodeIdx parent_idx(NodeIdx i) const { return arcs[nodes[i].parent].head; }
    inline Node& parent_node(NodeIdx i) { return nodes[parent_idx(i)]; }

    inline Arc& sister(ArcIdx a) { return arcs[arcs[a].rev]; }
    inline const Arc& sister(ArcIdx a) const { return arcs[arcs[a].rev]; }

    void print_graph(std::FILE *file = stdout) const;

    struct TmpEdge {
        int64_t head;
        int64_t tail;
        Cap cap;
        Cap revCap;
    };

    // TODO: Isn't this just a std::vector where we've called reserve?
    class ActiveList {
    public:
        NodeIdx *list;
        int64_t len;

        inline ActiveList()
        {
            list = NULL;
            len = 0;
        }

        inline void init(NodeIdx *mem)
        {
            list = mem;
            len = 0;
        }

        inline void clear()
        {
            len = 0;
        }

        inline void add(NodeIdx i)
        {
            list[len] = i;
            len++;
        }

        inline Node* pop()
        {
            len--;
            return list[len];
        }

        inline NodeIdx* getEnd()
        {
            return list + len;
        }

        inline static void swapLists(ActiveList *a, ActiveList *b)
        {
            ActiveList tmp = (*a);
            (*a) = (*b);
            (*b) = tmp;
        }
    };

    // TODO: Merge these three classes into one (maybe with subclasses)
    class BucketsOneSided {
    public:
        Node *nodes;
        NodeIdx *buckets;
        int64_t maxBucket;
        size_t allocLevels;

        inline BucketsOneSided()
        {
            nodes = NULL;
            buckets = NULL;
            maxBucket = 0;
            allocLevels = 0;
        }

        inline void init(Node *a_nodes, int64_t numNodes)
        {
            nodes = a_nodes;
            allocLevels = numNodes / 8;
            if (allocLevels < ALLOC_INIT_LEVELS) {
                if (numNodes < ALLOC_INIT_LEVELS) {
                    allocLevels = numNodes;
                } else {
                    allocLevels = ALLOC_INIT_LEVELS;
                }
            }
            buckets = new NodeIdx[allocLevels + 1];
            std::fill(buckets, buckets + allocLevels + 1, INVALID_NODE);
            maxBucket = 0;
        }

        inline void init_NoAlloc(Node *a_nodes, int64_t numNodes)
        {
            nodes = a_nodes;
            allocLevels = numNodes / 8;
            if (allocLevels < ALLOC_INIT_LEVELS) {
                if (numNodes < ALLOC_INIT_LEVELS) {
                    allocLevels = numNodes;
                } else {
                    allocLevels = ALLOC_INIT_LEVELS;
                }
            }
            std::fill(buckets, buckets + allocLevels + 1, INVALID_NODE);
            maxBucket = 0;
        }

        inline void allocate(int64_t numLevels)
        {
            if (numLevels > allocLevels) {
                allocLevels <<= 1;
                NodeIdx *alloc = new NodeIdx[allocLevels + 1];
                std::fill(alloc, alloc + allocLevels + 1, INVALID_NODE);
                delete[] buckets;
                buckets = alloc;
            }
        }

        inline void free()
        {
            delete[] buckets;
            buckets = NULL;
        }

        template <bool sTree>
        inline void add(NodeIdx i)
        {
            Node& x = nodes[i];
            int64_t bucket = (sTree ? (x.label) : (-x.label));
            x.nextNode = buckets[bucket];
            buckets[bucket] = i;
            if (bucket > maxBucket) {
                maxBucket = bucket;
            }
        }

        inline NodeIdx popFront(int64_t bucket)
        {
            NodeIdx i = buckets[bucket];
            if (i != INVALID_NODE) {
                buckets[bucket] = nodes[i].nextNode;
            }
            return i;
        }
    };

    class Buckets3Pass {
    public:
        Node *nodes;
        NodeIdx *buckets;
        int64_t maxBucket;
        size_t allocLevels;

        inline Buckets3Pass()
        {
            nodes = NULL;
            buckets = NULL;
            maxBucket = -1;
            allocLevels = -1;
        }

        inline void init(Node *a_nodes, int64_t numNodes)
        {
            nodes = a_nodes;
            allocLevels = numNodes / 8;
            if (allocLevels < ALLOC_INIT_LEVELS) {
                if (numNodes < ALLOC_INIT_LEVELS) {
                    allocLevels = numNodes;
                } else {
                    allocLevels = ALLOC_INIT_LEVELS;
                }
            }
            buckets = new NodeIdx[allocLevels + 1];
            std::fill(buckets, buckets + allocLevels + 1, INVALID_NODE);
            maxBucket = 0;
        }

        inline void init_NoAlloc(Node *a_nodes, int64_t numNodes)
        {
            nodes = a_nodes;
            allocLevels = numNodes / 8;
            if (allocLevels < ALLOC_INIT_LEVELS) {
                if (numNodes < ALLOC_INIT_LEVELS) {
                    allocLevels = numNodes;
                } else {
                    allocLevels = ALLOC_INIT_LEVELS;
                }
            }
            std::fill(buckets, buckets + allocLevels + 1, INVALID_NODE);
            maxBucket = 0;
        }

        inline void allocate(int64_t numLevels)
        {
            if (numLevels > allocLevels) {
                allocLevels <<= 1;
                NodeIdx *alloc = new NodeIdx[allocLevels + 1];
                std::fill(alloc, alloc + allocLevels + 1, INVALID_NODE);
                delete[] buckets;
                buckets = alloc;
            }
        }

        inline void free()
        {
            delete[] buckets;
            buckets = NULL;
        }

        template <bool sTree>
        inline void add(NodeIdx i)
        {
            Node& x = nodes[i];
            int64_t bucket = (sTree ? (x.label) : (-x.label));
            x.nextNode = buckets[bucket];
            if (x.nextNode != INVALID_NODE) {
                prev_idx(x.nextNode) = i;
            }
            buckets[bucket] = i;
            if (bucket > maxBucket) {
                maxBucket = bucket;
            }
        }

        inline NodeIdx popFront(int64_t bucket)
        {
            NodeIdx i = buckets[bucket];
            if (i != INVALID_NODE) {
                buckets[bucket] = nodes[i].nextNode;
                prev_idx(i) = INVALID_NODE;
            }
            return i;
        }

        template <bool sTree>
        inline void remove(NodeIdx i)
        {
            Node& x = nodes[i];
            int64_t bucket = (sTree ? (x.label) : (-x.label));
            if (buckets[bucket] == i) {
                buckets[bucket] = x.nextNode;
            } else {
                nodes[prev_idx(i)].nextNode = x.nextNode;
                if (x.nextNode != INVALID_NODE) {
                    prev_idx(x.nextNode) = prev_idx(i);
                }
            }
            prev_idx(i) = INVALID_NODE;
        }

        inline bool isEmpty(int64_t bucket)
        {
            return buckets[bucket] == INVALID_NODE;
        }

        inline NodeIdx& prev_idx(NodeIdx i)
        {
            return nodes[i].firstSon;
        }
    };

    class ExcessBuckets {
    public:
        Node *nodes;
        NodeIdx *buckets;
        NodeIdx *ptrs;
        int64_t maxBucket;
        int64_t minBucket;
        size_t allocLevels;

        inline ExcessBuckets()
        {
            nodes = NULL;
            buckets = NULL;
            ptrs = NULL;
            allocLevels = -1;
            maxBucket = -1;
            minBucket = -1;
        }

        inline void init(Node *a_nodes, NodeIdx *a_ptrs, int64_t numNodes)
        {
            nodes = a_nodes;
            allocLevels = numNodes / 8;
            if (allocLevels < ALLOC_INIT_LEVELS) {
                if (numNodes < ALLOC_INIT_LEVELS) {
                    allocLevels = numNodes;
                } else {
                    allocLevels = ALLOC_INIT_LEVELS;
                }
            }
            buckets = new NodeIdx[allocLevels + 1];
            std::fill(buckets, buckets + allocLevels + 1, INVALID_NODE);
            ptrs = a_ptrs;
            reset();
        }

        inline void init_NoAlloc(Node *a_nodes, NodeIdx *a_ptrs, int64_t numNodes)
        {
            nodes = a_nodes;
            allocLevels = numNodes / 8;
            if (allocLevels < ALLOC_INIT_LEVELS) {
                if (numNodes < ALLOC_INIT_LEVELS) {
                    allocLevels = numNodes;
                } else {
                    allocLevels = ALLOC_INIT_LEVELS;
                }
            }
            std::fill(buckets, buckets + allocLevels + 1, INVALID_NODE);
            ptrs = a_ptrs;
            reset();
        }

        inline void allocate(int64_t numLevels)
        {
            if (numLevels > allocLevels) {
                allocLevels <<= 1;
                NodeIdx *alloc = new NodeIdx[allocLevels + 1];
                std::fill(alloc, alloc + allocLevels + 1, INVALID_NODE);
                delete[]buckets;
                buckets = alloc;
            }
        }

        inline void free()
        {
            delete[]buckets;
            buckets = NULL;
        }

        template <bool sTree> inline void add(NodeIdx i)
        {
            Node& x = nodes[i];
            int64_t bucket = (sTree ? (x.label) : (-x.label));
            next_idx(i) = buckets[bucket];
            if (buckets[bucket] != INVALID_NODE) {
                prev_idx(buckets[bucket]) = i;
            }
            buckets[bucket] = i;
            if (bucket > maxBucket) {
                maxBucket = bucket;
            }
            if (bucket != 0 && bucket < minBucket) {
                minBucket = bucket;
            }
        }

        inline NodeIdx popFront(int64_t bucket)
        {
            NodeIdx i = buckets[bucket];
            if (i != INVALID_NODE) {
                buckets[bucket] = next_idx(i);
            }
            return i;
        }

        template <bool sTree>
        inline void remove(NodeIdx i)
        {
            Node& x = nodes[i];
            int64_t bucket = (sTree ? (x.label) : (-x.label));
            if (buckets[bucket] == i) {
                buckets[bucket] = next_idx(i);
            } else {
                next_idx(prev_idx(i)) = next_idx(i);
                if (next_idx(i) != INVALID_NODE) {
                    prev_idx(next_idx(i)) = prev_idx(i);
                }
            }
        }

        inline void incMaxBucket(int64_t bucket)
        {
            if (maxBucket < bucket) {
                maxBucket = bucket;
            }
        }

        inline bool empty()
        {
            return maxBucket < minBucket;
        }

        inline void reset()
        {
            maxBucket = 0;
            minBucket = -1 ^ (1 << 31); // What!? ...Why!?
        }

        inline NodeIdx& next_idx(NodeIdx i)
        {
            return ptrs[i * 2];
        }

        inline NodeIdx& prev_idx(NodeIdx i)
        {
            return ptrs[i * 2 + 1];
        }
    };

    //
    // Orphans
    //
    int64_t uniqOrphansS, uniqOrphansT;
    template <bool sTree> inline void orphanFree(NodeIdx i)
    {
        Node& x = nodes[i];
        if (x.excess) {
            x.label = (sTree ? -topLevelT : topLevelS);
            if (sTree) {
                activeT1.add(i);
            } else {
                activeS1.add(i);
            }
            x.isParentCurr = false;
        } else {
            x.label = 0;
        }
    }

    //
    // Initialization
    //
    inline bool isInitializedGraph() const noexcept { return memArcs != NULL; }
    void initGraphFast();
    void initNodes();
};

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::IBFSGraph() :
    init_n_nodes(0),
    init_n_edges(0),
    arcIter(NULL),
    incList(NULL),
    incLen(0),
    incIteration(0),
    numNodes(0),
    uniqOrphansS(0),
    uniqOrphansT(0),
    augTimestamp(0),
    arcs(NULL),
    arcEnd(NULL),
    nodes(NULL),
    nodeEnd(NULL),
    topLevelS(0),
    topLevelT(0),
    flow(0),
    memArcs(NULL),
    tmpEdges(NULL),
    tmpEdgeLast(NULL),
    ptrs(NULL),
    testFlow(0),
    testExcess(0)
{}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::IBFSGraph(int64_t numNodes, int64_t numEdges) :
    IBFSGraph()
{
    initSize(numNodes, numEdges);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::~IBFSGraph()
{
    delete[]nodes;
    delete[]memArcs;
    orphanBuckets.free();
    orphan3PassBuckets.free();
    excessBuckets.free();
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::addNode(NodeIdx node, Term capSource, Term capSink)
{
    Cap f = nodes[node].excess;
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
    nodes[node].excess = capSource - capSink;
}

// @pre: activeS1.len == 0 && activeT1.len == 0
template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::resetTrees()
{
    resetTrees(1, 1);
}

// @pre: activeS1.len == 0 && activeT1.len == 0
template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::resetTrees(
    int64_t newTopLevelS, int64_t newTopLevelT)
{
    uniqOrphansS = uniqOrphansT = 0;
    topLevelS = newTopLevelS;
    topLevelT = newTopLevelT;
    for (Node *y = nodes; y != nodeEnd; y++) {
        if (y->label < topLevelS && y->label > -topLevelT) {
            continue;
        }
        y->firstSon = INVALID_NODE;
        if (y->label == topLevelS) {
            activeS1.add(y);
        } else if (y->label == -topLevelT) {
            activeT1.add(y);
        } else {
            y->parent = INVALID_ARC;
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

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::remove_sibling(NodeIdx i)
{
    // TODO: This can very likely be made more readable!
    Node& parent = parent_node(i);
    NodeIdx first_parent_son = parent.firstSon;
    if (first_parent_son == i) {
        parent.firstSon = nodes[i].nextNode;
    } else {
        while (nodes[first_parent_son].nextNode != i) {
            first_parent_son = nodes[first_parent_son].nextNode;
        }
        nodes[first_parent_son].nextNode = nodes[i].nextNode;
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::add_sibling(NodeIdx i, NodeIdx parent)
{
    nodes[i].nextNode = nodes[parent].firstSon;
    nodes[parent].firstSon = i;
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::print_graph(std::FILE *file) const
{
    const NodeIdx num_nodes = nodeEnd - nodes;
    const NodeIdx num_arcs = arcEnd - arcs;
    std::fprintf(file, "%d %d\nnodes:\n", num_nodes, num_arcs);
    for (NodeIdx i = 0; i < num_nodes; ++i) {
        const Node& n = nodes[i];
        std::fprintf(file, "%d %d %d %d %d %d %d %d %d %d\n",
            i, n.lastAugTimestamp, n.isParentCurr, n.isIncremental, n.firstArc, n.parent, n.firstSon,
            n.nextNode, n.label, n.excess);
    }
    std::fprintf(file, "arcs:\n");
    for (ArcIdx i = 0; i < num_arcs; ++i) {
        const Arc& a = arcs[i];
        std::fprintf(file, "%d %d %d %d\n", i, a.head, a.rev, a.rCap);
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline bool IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::incShouldResetTrees()
{
    // TODO: Make sure uniqOrphansS + uniqOrphansT can be compared to int64_t
    return (uniqOrphansS + uniqOrphansT) >= 2 * numNodes;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::incNode(
    NodeIdx node, Term deltaCapSource, Term deltaCapSink)
{
    Node& x = nodes[node];

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
    addNode(node, deltaCapSource, deltaCapSink);

    // add to incremental list
    if (!x.isIncremental) {
        incList[incLen++] = node;
        x.isIncremental = true;
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::incArc(ArcIdx ai, Cap deltaCap)
{
    if (deltaCap == 0) {
        return;
    }
    Arc& a = arcs[ai];
    Arc& sister = arcs[a.rev];
    assert(a.rCap + sister.rCap + deltaCap < 0);

    if (deltaCap > -a.rCap) {
        // there is enough residual capacity
        a.rCap += deltaCap;
        if (a.rCap == deltaCap) {
            // we added capcity (deltaCap > 0) and arc was just made residual
            Node& x = nodes[sister.head];
            Node& y = nodes[a.head];
            if (x.label > 0 && y.label == (x.label + 1) && y.isParentCurr && a.rev < y.parent) {
                y.isParentCurr = false;
            } else if (x.label < 0 && y.label == (x.label + 1) && x.isParentCurr && ai < x.parent) {
                x.isParentCurr = false;
            } else if ((x->label > 0 && y.label <= 0)
                || (x.label >= 0 && y.label < 0)
                || (x.label > 0 && y.label > (x.label + 1))
                || (x.label < (y.label - 1) && y.label < 0)) {
                // arc invalidates invariants - must saturate it
                sister.rCap += deltaCap;
                a.rCap = 0;
                flow -= deltaCap;
                incNode(sister.head, 0, deltaCap);
                incNode(a.head, deltaCap, 0);
            }
        }
    } else {
        // there is not enough residual capacity
        // saturate the reverse arc
        Node& x = nodes[sister.head];
        Node& y = nodes[a.head];
        Cap push = -(deltaCap + a.rCap);
        sister.rCap -= push;
        a.rCap = 0;
        flow -= push;
        incNode(a.head, 0, push);
        incNode(sister.head, push, 0);
    }

    sister.isRevResidual = a.rCap != 0;
    a.isRevResidual = sister.rCap != 0;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::addEdge(
    NodeIdx from, NodeIdx to, Cap capacity, Cap revCapacity)
{
    tmpEdgeLast->tail = from;
    tmpEdgeLast->head = to;
    tmpEdgeLast->cap = capacity;
    tmpEdgeLast->revCap = revCapacity;
    tmpEdgeLast++;

    // use label as a temporary storage
    // to count the out degree of nodes
    nodes[from].label++;
    nodes[to].label++;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::incEdge(
    NodeIdx from, NodeIdx to, Cap capacity, Cap revCapacity)
{
    Node& x = nodes[from];
    Node& y = nodes[to];
    if (arcIter == NULL || arcIter->rev->head != x) {
        arcIter = x->firstArc;
    }
    Arc *end = (x + 1)->firstArc;
    Arc *initIter = arcIter;
    if (arcIter->head != y)
        for ((++arcIter) == end && (arcIter = x->firstArc); // TODO: Why is this a boolean!?
            arcIter != initIter;
            (++arcIter) == end && (arcIter = x->firstArc)) {
            if (arcIter->head == y) {
                break;
            }
        }
    if (arcIter->head != y) {
        fprintf(stdout, "Cannot increment arc (%d, %d)!\n", (int)(x - nodes), (int)(y - nodes));
        exit(1);
    }
    incArc(arcIter, capacity);
    incArc(arcIter->rev, revCapacity);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline int IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::isNodeOnSrcSide(NodeIdx node, int freeNodeValue)
{
    if (nodes[node].label == 0) {
        return freeNodeValue;
    }
    return (nodes[node].label > 0 ? 1 : 0);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::initGraph()
{
    initGraphFast();
    topLevelS = topLevelT = 1;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::initSize(int64_t numNodes, int64_t numEdges)
{
    init_n_nodes = numNodes;
    init_n_edges = numEdges;
    // compute allocation size
    uint64_t arcTmpMemsize = (uint64_t)sizeof(TmpEdge) * (uint64_t)numEdges;
    uint64_t arcRealMemsize = (uint64_t)sizeof(Arc) * (uint64_t)(numEdges * 2);
    uint64_t nodeMemsize = (uint64_t)sizeof(NodeIdx) * (uint64_t)(numNodes * 5);
    uint64_t arcMemsize = 0;
    arcMemsize = arcRealMemsize + arcTmpMemsize;
    if (arcMemsize < (arcRealMemsize + nodeMemsize)) {
        arcMemsize = (arcRealMemsize + nodeMemsize);
    }

    // alocate arcs
    memArcs = new char[arcMemsize];
    tmpEdges = (TmpEdge*)(memArcs + arcRealMemsize);
    tmpEdgeLast = tmpEdges; // will advance as edges are added
    arcs = (Arc*)memArcs;
    arcEnd = arcs + numEdges * 2;

    // allocate nodes
    this->numNodes = numNodes;
    nodes = new Node[numNodes + 1];
    memset(nodes, 0, sizeof(Node) * (numNodes + 1));
    nodeEnd = nodes + numNodes;
    // TODO: This likely uses too much memory
    active0.init((NodeIdx*)(arcEnd));
    activeS1.init((NodeIdx*)(arcEnd)+numNodes);
    activeT1.init((NodeIdx*)(arcEnd)+(2 * numNodes));
    ptrs = (NodeIdx*)(arcEnd)+(3 * numNodes);
    excessBuckets.init(nodes, ptrs, numNodes);
    orphan3PassBuckets.init(nodes, numNodes);
    orphanBuckets.init(nodes, numNodes);

    // init members
    flow = 0;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline  void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::reset()
{
    // compute allocation size
    uint64_t arcTmpMemsize = (uint64_t)sizeof(TmpEdge) * (uint64_t)init_n_edges;
    uint64_t arcRealMemsize = (uint64_t)sizeof(Arc) * (uint64_t)(init_n_edges * 2);
    uint64_t nodeMemsize = (uint64_t)sizeof(NodeIdx) * (uint64_t)(init_n_nodes * 5);
    uint64_t arcMemsize = 0;
    arcMemsize = arcRealMemsize + arcTmpMemsize;
    if (arcMemsize < (arcRealMemsize + nodeMemsize)) {
        arcMemsize = (arcRealMemsize + nodeMemsize);
    }

    // alocate arcs
    tmpEdges = (TmpEdge*)(memArcs + arcRealMemsize);
    tmpEdgeLast = tmpEdges; // will advance as edges are added
    arcs = (Arc*)memArcs;
    arcEnd = arcs + init_n_edges * 2;

    // allocate nodes
    this->init_n_nodes = init_n_nodes;
    memset(nodes, 0, sizeof(Node) * (init_n_nodes + 1));
    nodeEnd = nodes + init_n_nodes;
    // TODO: This likely uses too much memory
    active0.init((NodeIdx*)(arcEnd));
    activeS1.init((NodeIdx*)(arcEnd)+init_n_nodes);
    activeT1.init((NodeIdx*)(arcEnd)+(2 * init_n_nodes));
    ptrs = (NodeIdx*)(arcEnd)+(3 * init_n_nodes);
    excessBuckets.init_NoAlloc(nodes, ptrs, init_n_nodes);
    orphan3PassBuckets.init_NoAlloc(nodes, init_n_nodes);
    orphanBuckets.init_NoAlloc(nodes, init_n_nodes);

    // init members
    flow = 0;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::initNodes()
{
    NodeIdx i = 0;
    for (Node *x = nodes; x <= nodeEnd; ++x, ++i) {
        x->firstArc = x->label;
        x->parent = INVALID_ARC;
        x->firstSon = INVALID_NODE;
        x->nextNode = INVALID_NODE;
        if (x->excess == 0) {
            x->label = 0;
            continue;
        }
        if (x->excess > 0) {
            x->label = 1;
            activeS1.add(i);
        } else {
            x->label = -1;
            activeT1.add(i);
        }
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::initGraphFast()
{
    Node *x;
    TmpEdge *te;

    // calculate start arc offsets and labels for every node
    nodes->firstArc = 0;
    for (x = nodes; x != nodeEnd; x++) {
        (x + 1)->firstArc = x->firstArc + x->label;
        x->label = x->firstArc;
    }
    nodeEnd->label = arcEnd - arcs;

    // copy arcs
    for (te = tmpEdges; te != tmpEdgeLast; te++) {
        Arc& a1 = arcs[nodes[te->tail].firstArc];
        a1.rev = nodes[te->head].firstArc;
        a1.head = te->head;
        a1.rCap = te->cap;
        a1.isRevResidual = te->revCap != 0;

        Arc& a2 = arcs[nodes[te->head].firstArc];
        a2.rev = nodes[te->tail].firstArc;
        a2.head = te->tail;
        a2.rCap = te->revCap;
        a2.isRevResidual = te->cap != 0;

        ++nodes[te->head].firstArc;
        ++nodes[te->tail].firstArc;
    }

    initNodes();
}

// @ret: minimum orphan level
template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool sTree>
inline  int64_t IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::augmentPath(NodeIdx i, Cap push)
{
    int64_t orphanMinLevel = (sTree ? topLevelS : topLevelT) + 1;

    augTimestamp++;
    while (nodes[i].excess == 0) {
        Arc& a = arcs[nodes[i].parent];
        Arc& sister = arcs[a.rev];

        if (sTree) {
            a.rCap += push;
            sister.isRevResidual = true;
            sister.rCap -= push;
        } else {
            sister.rCap += push;
            a.isRevResidual = true;
            a.rCap -= push;
        }

        // saturated?
        if ((sTree ? sister.rCap : a.rCap) == 0) {
            if (sTree) {
                a.isRevResidual = false;
            } else {
                sister.isRevResidual = false;
            }
            remove_sibling(i);
            orphanMinLevel = sTree ? nodes[i].label : -nodes[i].label;
            orphanBuckets.template add<sTree>(i);
        }

        // Advance
        i = a.head;
    }
    Node& x = nodes[i];
    x.excess += sTree ? -push : push;
    if (x.excess == 0) {
        orphanMinLevel = sTree ? x.label : -x.label;
        orphanBuckets.template add<sTree>(i);
    }
    flow += push;

    return orphanMinLevel;
}

// @ret: minimum level in which created an orphan
template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool sTree>
inline  int64_t IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::augmentExcess(NodeIdx i, Cap push)
{
    int64_t orphanMinLevel = (sTree ? topLevelS : topLevelT) + 1;
    augTimestamp++;

    // start of loop
    //----------------
    // x           the current node along the path
    // a          arc incoming into x
    // push       the amount of flow coming into x
    // a->resCap  updated with incoming flow already
    // x->excess  not updated with incoming flow yet
    //
    // end of loop
    //-----------------
    // x           the current node along the path
    // a          arc outgoing from x
    // push       the amount of flow coming out of x
    // a->resCap  updated with outgoing flow already
    // x->excess  updated with incoming flow already
    while (sTree ? (nodes[i].excess <= 0) : (nodes[i].excess >= 0)) {
        Node& x = nodes[i];
        Arc& a = arcs[x.parent];
        Arc& sister = arcs[a.rev];

        // update excess and find next flow
        const bool has_excess = sTree ? (sister.rCap < push - x.excess) : (a.rCap < x.excess + push);
        if (has_excess) {
            // some excess remains, node is an orphan
            x.excess += (sTree ? (sister.rCap - push) : (push - a.rCap));
            push = (sTree ? sister.rCap : a.rCap);
        } else {
            // all excess is pushed out, node may or may not be an orphan
            push += (sTree ? -(x.excess) : x.excess);
            x.excess = 0;
        }

        // push flow
        // note: push != 0
        if (sTree) {
            a.rCap += push;
            sister.isRevResidual = true;
            sister.rCap -= push;
        } else {
            sister.rCap += push;
            a.isRevResidual = true;
            a.rCap -= push;
        }

        // saturated?
        if ((sTree ? (sister.rCap) : (a.rCap)) == 0) {
            if (sTree) {
                a.isRevResidual = false;
            } else {
                sister.isRevResidual = false;
            }
            remove_sibling(i);
            orphanMinLevel = (sTree ? x.label : -x.label);
            orphanBuckets.template add<sTree>(i);
            if (x.excess) {
                excessBuckets.incMaxBucket(sTree ? x.label : -x.label);
            }
        }

        // advance
        // a precondition determines that the first node on the path is not in excess buckets
        // so only the next nodes may need to be removed from there
        i = a.head;
        if (sTree ? (nodes[i].excess < 0) : (nodes[i].excess > 0)) {
            excessBuckets.template remove<sTree>(i);
        }
    }

    // update the excess at the root
    Node& x = nodes[i];
    if (push <= (sTree ? (x.excess) : -(x.excess))) {
        flow += push;
    } else {
        flow += (sTree ? (x.excess) : -(x.excess));
    }
    x.excess += (sTree ? (-push) : push);
    if (sTree ? (x.excess <= 0) : (x.excess >= 0)) {
        orphanMinLevel = (sTree ? x.label : -x.label);
        orphanBuckets.template add<sTree>(i);
        if (x.excess) {
            excessBuckets.incMaxBucket(sTree ? x.label : -x.label);
        }
    }

    return orphanMinLevel;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool sTree>
inline  void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::augmentExcesses()
{
    NodeIdx i;
    int64_t minOrphanLevel;
    int64_t adoptedUpToLevel = excessBuckets.maxBucket;

    if (!excessBuckets.empty()) {
        for (; excessBuckets.maxBucket != (excessBuckets.minBucket - 1); excessBuckets.maxBucket--) {
            while ((i = excessBuckets.popFront(excessBuckets.maxBucket)) != INVALID_NODE) {
                minOrphanLevel = augmentExcess<sTree>(i, 0);
                // if we did not create new orphans
                if (adoptedUpToLevel < minOrphanLevel) {
                    minOrphanLevel = adoptedUpToLevel;
                }
                adoption<sTree>(minOrphanLevel, false);
                adoptedUpToLevel = excessBuckets.maxBucket;
            }
        }
    }
    excessBuckets.reset();
    if (orphanBuckets.maxBucket != 0) {
        adoption<sTree>(adoptedUpToLevel + 1, true);
    }
    // free 3pass orphans
    while ((i = excessBuckets.popFront(0)) != INVALID_NODE) {
        orphanFree<sTree>(i);
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline  void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::augment(ArcIdx bridge_idx)
{
    Arc& bridge = arcs[bridge_idx];
    Arc& sister_bridge = sister(bridge_idx);
    Cap bottleneck, bottleneckT, bottleneckS;
    bool forceBottleneck = false;

    // Find bottleneck capacity
    // TODO: This can probaly be rewritten or refactored to be nicer
    bottleneck = bridge.rCap;
    bottleneckS = bridge.rCap;
    if (bottleneck != 1) {
        NodeIdx i;
        for (i = sister_bridge.head; !nodes[i].excess; i = arcs[nodes[i].parent].head) {
            const Cap sister_cap = sister(nodes[i].parent).rCap;
            if (bottleneckS > sister_cap) {
                bottleneckS = sister_cap;
            }
        }
        const Node& x = nodes[i];
        if (bottleneckS > x.excess) {
            bottleneckS = x.excess;
        }
        if (x.label != 1) {
            forceBottleneck = true;
        }
        if (i == sister_bridge.head) {
            bottleneck = bottleneckS;
        }
    }

    if (bottleneck != 1) {
        bottleneckT = bridge.rCap;
        NodeIdx i;
        for (i = bridge.head; !nodes[i].excess; i = arcs[nodes[i].parent].head) {
            const Cap cap = arcs[nodes[i].parent].rCap;
            if (bottleneckT > cap) {
                bottleneckT = cap;
            }
        }
        const Node& x = nodes[i];
        if (bottleneckT > (-x.excess)) {
            bottleneckT = (-x.excess);
        }
        if (x.label != -1) {
            forceBottleneck = true;
        }
        if (i == bridge.head && bottleneck > bottleneckT) {
            bottleneck = bottleneckT;
        }

        if (forceBottleneck) {
            if (bottleneckS < bottleneckT) {
                bottleneck = bottleneckS;
            } else {
                bottleneck = bottleneckT;
            }
        }
    }

    // augment connecting arc
    sister_bridge.rCap += bottleneck;
    bridge.isRevResidual = true;
    bridge.rCap -= bottleneck;
    if (bridge.rCap == 0) {
        sister_bridge.isRevResidual = false;
    }
    flow -= bottleneck;

    // augment T
    if (bottleneck == 1 || forceBottleneck) {
        int64_t minOrphanLevel = augmentPath<false>(bridge.head, bottleneck);
        adoption<false>(minOrphanLevel, true);
    } else {
        int64_t minOrphanLevel = augmentExcess<false>(bridge.head, bottleneck);
        adoption<false>(minOrphanLevel, false);
        augmentExcesses<false>();
    }

    // augment S
    if (bottleneck == 1 || forceBottleneck) {
        int64_t minOrphanLevel = augmentPath<true>(sister_bridge.head, bottleneck);
        adoption<true>(minOrphanLevel, true);
    } else {
        int64_t minOrphanLevel = augmentExcess<true>(sister_bridge.head, bottleneck);
        adoption<true>(minOrphanLevel, false);
        augmentExcesses<true>();
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool sTree>
inline  void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::adoption(int64_t fromLevel, bool toTop)
{
    ArcIdx ai;
    int64_t threePassLevel;
    int64_t minLabel, numOrphans, numOrphansUniq;
    int64_t level;

    threePassLevel = 0;
    numOrphans = 0;
    numOrphansUniq = 0;
    for (level = fromLevel;
        level <= orphanBuckets.maxBucket && (toTop || threePassLevel || level <= excessBuckets.maxBucket);
        level++) {
        NodeIdx i;
        while ((i = orphanBuckets.popFront(level)) != INVALID_NODE) {
            Node& x = nodes[i];
            numOrphans++;
            if (x.lastAugTimestamp != augTimestamp) {
                x.lastAugTimestamp = augTimestamp;
                if (sTree) {
                    uniqOrphansS++;
                } else {
                    uniqOrphansT++;
                }
                numOrphansUniq++;
            }
            if (threePassLevel == 0 && numOrphans >= 3 * numOrphansUniq) {
                // switch to 3pass
                threePassLevel = 1;
            }

            //
            // check for same level connection
            //
            if (x.isParentCurr) {
                ai = x.parent;
            } else {
                ai = x.firstArc;
                x.isParentCurr = true;
            }
            x.parent = INVALID_ARC;
            const ArcIdx a_end = nodes[i + 1].firstArc;
            if (x.label != (sTree ? 1 : -1)) {
                minLabel = x.label - (sTree ? 1 : -1);
                for (; ai != a_end; ai++) {
                    const Arc& a = arcs[ai];
                    Node& y = nodes[a.head];
                    if ((sTree ? a.isRevResidual : a.rCap) != 0 && y.label == minLabel) {
                        x.parent = ai;
                        add_sibling(i, a.head);
                        break;
                    }
                }
            }
            if (x.parent != INVALID_ARC) {
                if (x.excess) {
                    excessBuckets.template add<sTree>(i);
                }
                continue;
            }

            //
            // on the top level there is no need to relabel
            //
            if (x.label == (sTree ? topLevelS : -topLevelT)) {
                orphanFree<sTree>(i);
                continue;
            }

            //
            // give up on same level - relabel it!
            // (1) create orphan sons
            //
            NodeIdx next;
            for (NodeIdx j = x.firstSon; j != INVALID_NODE; j = next) {
                next = nodes[j].nextNode; // Must store this now as it will be overwritten in add
                if (nodes[j].excess) {
                    excessBuckets.template remove<sTree>(j);
                }
                orphanBuckets.template add<sTree>(j);
            }
            x.firstSon = INVALID_NODE;

            //
            // (2) 3pass relabeling: move to buckets structure
            //
            if (threePassLevel) {
                x.label += (sTree ? 1 : -1);
                orphan3PassBuckets.template add<sTree>(i);
                if (threePassLevel == 1) {
                    threePassLevel = level + 1;
                }
                continue;
            }

            //
            // (2) relabel: find the lowest level parent
            //
            minLabel = (sTree ? topLevelS : -topLevelT);
            if (x.label != minLabel) {
                for (ai = x.firstArc; ai != a_end; ai++) {
                    const Arc& a = arcs[ai];
                    const Node& y = nodes[a.head];
                    if ((sTree ? a.isRevResidual : a.rCap) &&
                        // y->label != 0 ---> holds implicitly
                        (sTree ? (y.label > 0) : (y.label < 0)) &&
                        (sTree ? (y.label < minLabel) : (y.label > minLabel))) {
                        minLabel = y.label;
                        x.parent = ai;
                        if (minLabel == x.label) {
                            break;
                        }
                    }
                }
            }

            //
            // (3) relabel onto new parent
            //
            if (x.parent != INVALID_ARC) {
                x.label = minLabel + (sTree ? 1 : -1);
                add_sibling(i, arcs[x.parent].head);
                // add to active list of the next growth phase
                if (sTree) {
                    if (x.label == topLevelS) {
                        activeS1.add(i);
                    }
                } else {
                    if (x.label == -topLevelT) {
                        activeT1.add(i);
                    }
                }
                if (x.excess) {
                    excessBuckets.template add<sTree>(i);
                }
            } else {
                orphanFree<sTree>(i);
            }
        }
    }
    if (level > orphanBuckets.maxBucket) {
        orphanBuckets.maxBucket = 0;
    }

    if (threePassLevel) {
        adoption3Pass<sTree>(threePassLevel);
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool sTree>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::adoption3Pass(int64_t minBucket)
{
    int64_t minLabel, destLabel;

    for (int64_t level = minBucket; level <= orphan3PassBuckets.maxBucket; level++) {
        NodeIdx i;
        while ((i = orphan3PassBuckets.popFront(level)) != INVALID_NODE) {
            Node& x = nodes[i];
            ArcIdx a_end = nodes[i + 1].firstArc;

            // pass 2: find lowest level parent
            if (x.parent == INVALID_ARC) {
                minLabel = (sTree ? topLevelS : -topLevelT);
                destLabel = x.label - (sTree ? 1 : -1);
                for (ArcIdx ai = x.firstArc; ai != a_end; ai++) {
                    Arc& a = arcs[ai];
                    const Node& y = nodes[a.head];
                    if ((sTree ? a.isRevResidual : a.rCap) &&
                        ((sTree ? (y.excess > 0) : (y.excess < 0)) || y.parent != INVALID_ARC) &&
                        (sTree ? (y.label > 0) : (y.label < 0)) &&
                        (sTree ? (y.label < minLabel) : (y.label > minLabel))) {
                        x.parent = ai;
                        minLabel = y.label;
                        if (minLabel == destLabel) {
                            break;
                        }
                    }
                }
                if (x.parent == INVALID_ARC) {
                    x.label = 0;
                    if (x.excess) {
                        excessBuckets.template add<sTree>(i);
                    }
                    continue;
                }
                x.label = minLabel + (sTree ? 1 : -1);
                if (x.label != (sTree ? level : -level)) {
                    orphan3PassBuckets.template add<sTree>(i);
                    continue;
                }
            }

            // pass 3: lower potential sons and/or find first parent
            if (x.label != (sTree ? topLevelS : -topLevelT)) {
                minLabel = x.label + (sTree ? 1 : -1);
                for (ArcIdx ai = x.firstArc; ai != a_end; ai++) {
                    const Arc& a = arcs[ai];
                    Node& y = nodes[a.head];

                    // lower potential sons
                    if ((sTree ? a.rCap : a.isRevResidual) &&
                        (y.label == 0 ||
                            (sTree ? (minLabel < y.label) : (minLabel > y.label)))) {
                        if (y.label != 0) {
                            orphan3PassBuckets.template remove<sTree>(a.head);
                        } else if (y.excess) {
                            excessBuckets.template remove<sTree>(a.head);
                        }
                        y.label = minLabel;
                        y.parent = a.rev;
                        orphan3PassBuckets.template add<sTree>(a.head);
                    }
                }
            }

            // relabel onto new parent
            add_sibling(i, arcs[x.parent].head);
            x.isParentCurr = false;
            if (x.excess) {
                excessBuckets.template add<sTree>(i);
            }

            // add to active list of the next growth phase
            if (sTree) {
                if (x.label == topLevelS) {
                    activeS1.add(i);
                }
            } else {
                if (x.label == -topLevelT) {
                    activeT1.add(i);
                }
            }
        }
    }

    orphan3PassBuckets.maxBucket = 0;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool dirS>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::growth()
{
    // TODO: This loop could be written in a better way
    for (NodeIdx *active = active0.list; active != (active0.list + active0.len); active++) {
        // get active node
        NodeIdx i = *active;
        Node& x = nodes[i];

        // node no longer at level
        if (x.label != (dirS ? (topLevelS - 1) : -(topLevelT - 1))) {
            continue;
        }

        // grow or augment
        ArcIdx a_end = nodes[i + 1].firstArc;
        for (ArcIdx ai = x.firstArc; ai != a_end; ai++) {
            Arc& a = arcs[ai];
            if (!(dirS ? a.rCap : a.isRevResidual)) {
                continue;
            }
            NodeIdx j = a.head;
            Node& y = nodes[j];
            if (y.label == 0) {
                // grow node x (attach y)
                y.isParentCurr = 0;
                y.label = x.label + (dirS ? 1 : -1);
                y.parent = a.rev;
                add_sibling(j, i);
                if (dirS) {
                    activeS1.add(j);
                } else {
                    activeT1.add(j);
                }
            } else if (dirS ? (y.label < 0) : (y.label > 0)) {
                // augment
                augment(dirS ? ai : a.rev);
                if (x.label != (dirS ? (topLevelS - 1) : -(topLevelT - 1))) {
                    break;
                }
                if (dirS ? a.rCap : a.isRevResidual) {
                    ai--;
                }
            }
        }
    }
    active0.clear();
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool sTree>
inline void IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::augmentIncrements()
{
    NodeIdx *end = incList + incLen;
    int64_t minOrphanLevel = 1 << 30;

    for (NodeIdx *inc = incList; inc != end; inc++) {
        NodeIdx i = *inc;
        Node& x = nodes[i];
        if (!x.isIncremental || (sTree ? (x.label < 0) : (x.label > 0))) {
            continue;
        }
        x.isIncremental = false;
        if (x.label == 0) {
            //**** new root from outside the tree
            if (!x.excess) {
                continue;
            }
            x.isParentCurr = false;
            if (x.excess > 0) {
                x.label = topLevelS;
                activeS1.add(i);
            } else if (x.excess < 0) {
                x.label = -topLevelT;
                activeT1.add(i);
            }
        } else if ((sTree ? (x.excess <= 0) : (x.excess >= 0)) &&
            (x.parent == INVALID_ARC || !(sTree ? arcs[x.parent].isRevResidual : arcs[x.parent].rCap))) {
            //**** new orphan
            if (x.parent != INVALID_ARC) {
                remove_sibling(i);
            }
            if ((sTree ? x.label : -x.label) < minOrphanLevel) {
                minOrphanLevel = (sTree ? x.label : -x.label);
            }
            orphanBuckets.template add<sTree>(i);
            if (x.excess) {
                excessBuckets.incMaxBucket(sTree ? x.label : -x.label);
            }
        } else if (sTree ? (x.excess < 0) : (x.excess > 0)) {
            //**** new deficit/excess to empty
            excessBuckets.template add<sTree>(i);
        } else if (x.excess && x.parent != INVALID_ARC) {
            //**** new root
            remove_sibling(i);
            x.parent = INVALID_ARC;
            x.isParentCurr = false;
        }
    }
    if (orphanBuckets.maxBucket != 0) {
        adoption<sTree>(minOrphanLevel, false);
    } else {
        augmentExcesses<sTree>();
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline Flow IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::computeMaxFlow()
{
    return computeMaxFlow(true, false);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline Flow IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::computeMaxFlow(bool allowIncrements)
{
    return computeMaxFlow(true, allowIncrements);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline Flow IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx>::computeMaxFlow(
    bool initialDirS, bool allowIncrements)
{
    // incremental?
    if (incIteration >= 1 && incList != NULL) {
        augmentIncrements<true>();
        augmentIncrements<false>();
        incList = NULL;
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
        excessBuckets.allocate((topLevelS > topLevelT) ? topLevelS : topLevelT);
        if (dirS)
            growth<true>();
        else
            growth<false>(); //second iteration

        // switch to next level
        if (!allowIncrements && (activeS1.len == 0 || activeT1.len == 0)) {
            break;
        }
        if (activeS1.len == 0 && activeT1.len == 0) {
            break;
        }
        if (activeT1.len == 0) {
            dirS = true;
        } else if (activeS1.len == 0) {
            dirS = false;
        } else if (uniqOrphansT == uniqOrphansS && dirS) {
            dirS = false;
        } else if (uniqOrphansT < uniqOrphansS) {
            dirS = false;
        } else {
            dirS = true;
        }
    }

    incIteration++;
    return flow;
}

} // namespace reimpls

#endif // REIMPLS_IBFS_H__


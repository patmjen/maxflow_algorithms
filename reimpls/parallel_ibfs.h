#ifndef REIMPLS_PARALLEL_IBFS_H__
#define REIMPLS_PARALLEL_IBFS_H__

#include <cstdio>
#include <string>
#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <cassert>
#include <vector>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>

#include "util.h"

namespace reimpls {

template <class Cap, class Term, class Flow, class NodeIdx = uint32_t, class ArcIdx = uint32_t>
class ParallelIbfs {
    static constexpr size_t ALLOC_INIT_LEVELS = 4096;

    static constexpr NodeIdx INVALID_NODE = ~NodeIdx(0); // -1 for signed type, max. value for unsigned type
    static constexpr ArcIdx INVALID_ARC = ~ArcIdx(0); // -1 for signed type, max. value for unsigned type

    using BlockIdx = uint16_t; // We assume 65536 is enough blocks
    using BoundaryKey = uint32_t; // Must be 2 x sizeof(BlockIdx)
    using Dist = std::make_signed_t<NodeIdx>;

    static_assert(sizeof(BoundaryKey) == 2 * sizeof(BlockIdx),
        "BoundaryKey must be double the size of BlockIdx");
    static_assert(std::is_integral<ArcIdx>::value, "ArcIdx must be an integer type");
    static_assert(std::is_integral<NodeIdx>::value, "NodeIdx must be an integer type");
    static_assert(std::is_signed<Cap>::value, "Cap must be a signed type");

    // Forward decls.
    struct Node;
    struct Arc;
    struct BoundarySegment;
    struct IbfsBlock;
    struct TmpEdge;
    class ActiveList;
    class BucketsOneSided;
    class Buckets3Pass;
    class ExcessBuckets;

public:
    ParallelIbfs();
    ParallelIbfs(int64_t numNodes, int64_t numEdges);
    ~ParallelIbfs();
    void initSize(int64_t numNodes, int64_t numEdges);
    void reset();
    void registerNodes(NodeIdx begin, NodeIdx end, BlockIdx block);
    void addEdge(NodeIdx from, NodeIdx to, Cap capacity, Cap revCapacity);
    void addNode(NodeIdx node, Term capSource, Term capSink);
    // bool incShouldResetTrees();
    void initGraph();
    Flow computeMaxFlow();
    void resetTrees();

    inline Flow getFlow() const noexcept { return flow; }
    inline size_t getNumNodes() const noexcept { return nodes.size(); }
    inline size_t getNumArcs() const noexcept { return arcs.size(); }
    int isNodeOnSrcSide(NodeIdx node, int freeNodeValue = 0);
    unsigned int getNumThreads() const noexcept { return num_threads; }
    void setNumThreads(unsigned int threads) { num_threads = threads; }

private:
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

    int64_t init_n_nodes;
    int64_t init_n_edges;
    Arc* arcIter;

    std::vector<Node> nodes;
    std::vector<Arc> arcs;
    std::vector<NodeIdx> ptrs;
    //Node *nodes, *nodeEnd;
    //Arc *arcs, *arcEnd;
    //NodeIdx* ptrs;
    int64_t numNodes;
    int incIteration;
    //char *memArcs;
    std::vector<TmpEdge> tmpEdges;
    //TmpEdge *tmpEdges, 
    TmpEdge *tmpEdgeLast;
    Flow flow;

    std::vector<IbfsBlock> blocks;
    std::vector<BlockIdx> node_blocks;

    std::unordered_map<BoundaryKey, std::vector<std::pair<ArcIdx, Cap>>> boundary_arcs;
    std::list<BoundarySegment> boundary_segments;
    std::vector<BlockIdx> block_idxs;

    unsigned int num_threads;

    void print_graph(std::FILE* file = stdout) const;

    struct BoundarySegment {
        const std::vector<std::pair<ArcIdx, Cap>>& arcs;
        BlockIdx i;
        BlockIdx j;
        int32_t broken_invariants;
    };

    struct IbfsBlock {
        BlockIdx self;
        NodeIdx numNodes; // TODO: Remove when we change to std::vector
        ArcIdx numArcs; // TODO: Remove when we change to std::vector
        bool locked;

        std::vector<Node>& nodes;
        std::vector<Arc>& arcs;
        std::vector<NodeIdx>& ptrs;

        Flow flow;
        int64_t augTimestamp;
        int64_t topLevelS, topLevelT;
        ActiveList active0, activeS1, activeT1;
        Buckets3Pass orphan3PassBuckets;
        BucketsOneSided orphanBuckets;
        ExcessBuckets excessBuckets;
        int64_t uniqOrphansS, uniqOrphansT;

        IbfsBlock(NodeIdx numNodes, ArcIdx numArcs, std::vector<Node>& nodes, std::vector<Arc>& arcs, 
            std::vector<NodeIdx>& ptrs, BlockIdx block) :
            self(block),
            numNodes(numNodes),
            numArcs(numArcs),
            locked(false),
            nodes(nodes),
            arcs(arcs),
            ptrs(ptrs),
            flow(0),
            augTimestamp(0),
            topLevelS(1),
            topLevelT(1),
            active0(),
            activeS1(),
            activeT1(),
            orphan3PassBuckets(),
            orphanBuckets(),
            excessBuckets(),
            uniqOrphansS(1),
            uniqOrphansT(1)
        {
            NodeIdx *bufBegin = ptrs.data();
            active0.init(bufBegin);
            activeS1.init(bufBegin + numNodes);
            activeT1.init(bufBegin + 2 * numNodes);
            excessBuckets.init(nodes.data(), bufBegin + 3 * numNodes, numNodes);
            orphan3PassBuckets.init(nodes.data(), numNodes);
            orphanBuckets.init(nodes.data(), numNodes);
        }

        void incNode(NodeIdx node, Term deltaCapSource, Term deltaCapSink);
        void incArc(ArcIdx arc, Cap deltaCap);

        void augment(ArcIdx bridge);
        template <bool sTree> int64_t augmentPath(NodeIdx i, Cap push);
        template <bool sTree> int64_t augmentExcess(NodeIdx i, Cap push);
        template <bool sTree> void augmentExcesses();
        template <bool sTree> void augmentIncrements();
        template <bool sTree> void adoption(int64_t fromLevel, bool toTop);
        template <bool sTree> void adoption3Pass(int64_t minBucket);
        template <bool dirS> void growth();

        Flow computeMaxFlow(bool trackChanges, bool initialDirS);

        void remove_sibling(NodeIdx i);
        void add_sibling(NodeIdx i, NodeIdx parent);

        inline NodeIdx parent_idx(NodeIdx i) const { return arcs[nodes[i].parent].head; }
        inline Node& parent_node(NodeIdx i) { return nodes[parent_idx(i)]; }

        inline Arc& sister(ArcIdx a) { return arcs[arcs[a].rev]; }
        inline const Arc& sister(ArcIdx a) const { return arcs[arcs[a].rev]; }

        template <bool sTree>
        inline void orphanFree(NodeIdx i)
        {
            // TODO: Move definition outside class
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
    };

    struct TmpEdge {
        int64_t head;
        int64_t tail;
        Cap cap;
        Cap revCap;
    };

    class ActiveList {
    public:
        NodeIdx* list; // Holds index of next node in list, index of self if last, or INVALID_NODE
        NodeIdx first, last;

        inline ActiveList() :
            list(nullptr),
            first(INVALID_NODE),
            last(INVALID_NODE) {}

        inline void init(NodeIdx* mem)
        {
            list = mem;
            first = INVALID_NODE;
            last = INVALID_NODE;
        }

        inline void add(NodeIdx i)
        {
            if (list[i] == INVALID_NODE) {
                // Node is not currently in an active list
                if (empty()) {
                    first = i;
                } else {
                    assert(list[last] == last);
                    list[last] = i;
                }
                last = i;
                list[i] = i; // Mark as last
            }
        }

        inline NodeIdx pop()
        {
            NodeIdx out = first;
            if (out != INVALID_NODE) {
                if (list[out] == out) {
                    // This is the last node in the active list so "clear" the list
                    first = INVALID_NODE;
                    last = INVALID_NODE;
                } else {
                    first = list[first];
                }
                list[out] = INVALID_NODE;
            }
            return out;
        }

        inline NodeIdx empty()
        {
            return last == INVALID_NODE;
        }
    };

    // TODO: Merge these three classes into one (maybe with subclasses)
    class BucketsOneSided {
    public:
        Node* nodes;
        std::vector<NodeIdx> buckets;
        int64_t maxBucket;
        size_t allocLevels;

        BucketsOneSided() :
            nodes(nullptr),
            buckets(),
            maxBucket(0),
            allocLevels(0) {}

        inline void init(Node* a_nodes, int64_t numNodes)
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
            buckets.resize(allocLevels + 1, INVALID_NODE);
            maxBucket = 0;
        }

        inline void init_NoAlloc(Node* a_nodes, int64_t numNodes)
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
            std::fill(buckets.begin(), buckets.end(), INVALID_NODE);
            maxBucket = 0;
        }

        inline void allocate(int64_t numLevels)
        {
            if (numLevels > allocLevels) {
                // TODO: What if numLevels > 2 * allocLevels? Can this not happen?
                allocLevels *= 2;
                buckets.resize(allocLevels + 1, INVALID_NODE);
            }
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
        Node* nodes;
        std::vector<NodeIdx> buckets;
        int64_t maxBucket;
        size_t allocLevels;

        Buckets3Pass() :
            nodes(nullptr),
            buckets(),
            maxBucket(-1),
            allocLevels(-1) {}

        inline void init(Node* a_nodes, int64_t numNodes)
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
            buckets.resize(allocLevels + 1, INVALID_NODE);
            maxBucket = 0;
        }

        inline void init_NoAlloc(Node* a_nodes, int64_t numNodes)
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
            std::fill(buckets.begin(), buckets.end(), INVALID_NODE);
            maxBucket = 0;
        }

        inline void allocate(int64_t numLevels)
        {
            if (numLevels > allocLevels) {
                // TODO: What if numLevels > 2 * allocLevels? Can this not happen?
                allocLevels *= 2;
                buckets.resize(allocLevels + 1, INVALID_NODE);
            }
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
        Node* nodes;
        std::vector<NodeIdx> buckets;
        NodeIdx* ptrs;
        int64_t maxBucket;
        int64_t minBucket;
        size_t allocLevels;

        ExcessBuckets() :
            nodes(nullptr),
            buckets(),
            ptrs(nullptr),
            allocLevels(-1),
            maxBucket(-1),
            minBucket(-1) {}

        inline void init(Node* a_nodes, NodeIdx* a_ptrs, int64_t numNodes)
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
            buckets.resize(allocLevels + 1, INVALID_NODE);
            ptrs = a_ptrs;
            reset();
        }

        inline void init_NoAlloc(Node* a_nodes, NodeIdx* a_ptrs, int64_t numNodes)
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
            std::fill(buckets.begin(), buckets.end(), INVALID_NODE);
            ptrs = a_ptrs;
            reset();
        }

        inline void allocate(int64_t numLevels)
        {
            if (numLevels > allocLevels) {
                // TODO: What if numLevels > 2 * allocLevels? Can this not happen?
                allocLevels *= 2;
                buckets.resize(allocLevels + 1, INVALID_NODE);
            }
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
    
    static bool invalidates_invariants(const Node& x, const Node& y);

    inline BoundaryKey block_key(BlockIdx i, BlockIdx j) const noexcept;
    inline std::pair<BlockIdx, BlockIdx> blocks_from_key(BoundaryKey key) const noexcept;

    std::pair<std::list<BoundarySegment>, BlockIdx> next_boundary_segment_set();

    void unite_blocks(BlockIdx i, BlockIdx j);

    void resetTrees(BlockIdx block, int64_t newTopLevelS, int64_t newTopLevelT);

    inline bool isInitializedGraph() const noexcept { return arcs.size() > 0; }
    void initGraphFast();
    void initNodes();

    ArcIdx build_arc(NodeIdx from, NodeIdx to, Cap cap, Cap revCap);
};

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::ParallelIbfs() :
    init_n_nodes(0),
    init_n_edges(0),
    arcIter(nullptr),
    incIteration(0),
    numNodes(0),
    nodes(),
    //nodeEnd(nullptr),
    arcs(),
    //arcEnd(nullptr),
    ptrs(),
    //memArcs(nullptr),
    tmpEdges(),
    tmpEdgeLast(nullptr),
    flow(0),
    blocks(),
    node_blocks(),
    boundary_arcs(),
    boundary_segments(),
    block_idxs(),
    num_threads(std::thread::hardware_concurrency()) {}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::ParallelIbfs(int64_t numNodes, int64_t numEdges) :
    ParallelIbfs()
{
    initSize(numNodes, numEdges);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::~ParallelIbfs()
{
    /*delete[]nodes;
    delete[]memArcs;*/
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::addNode(
    NodeIdx node, Term capSource, Term capSink)
{
    assert(isInitializedGraph());
    
    Cap f = nodes[node].excess;
    if (f > 0) {
        capSource += f;
    } else {
        capSink -= f;
    }
    if (capSource < capSink) {
        blocks[node_blocks[node]].flow += capSource;
    } else {
        blocks[node_blocks[node]].flow += capSink;
    }
    nodes[node].excess = capSource - capSink;
}

// @pre: activeS1.empty() && activeT1.empty()
template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::resetTrees()
{
    resetTrees(1, 1);
}

// @pre: activeS1.empty() && activeT1.empty()
template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::resetTrees(
    BlockIdx block, int64_t newTopLevelS, int64_t newTopLevelT)
{
    auto& b = blocks[block];
    assert(b.activeS1.empty() && b.activeT1.empty());
    b.uniqOrphansS = 0;
    b.uniqOrphansT = 0;
    b.topLevelS = newTopLevelS;
    b.topLevelT = newTopLevelT;
    for (NodeIdx i = 0; i < nodes.size() - 1; ++i) {
        if (block_idxs[node_blocks[i]] != block) {
            continue;
        }
        Node& y = nodes[i];
        if (y.label < b.topLevelS && y.label > -b.topLevelT) {
            continue;
        }
        y.firstSon = INVALID_NODE;
        if (y.label == b.topLevelS) {
            b.activeS1.add(i);
        } else if (y.label == -b.topLevelT) {
            b.activeT1.add(i);
        } else {
            y.parent = INVALID_ARC;
            if (y.excess == 0) {
                y.label = 0;
            } else if (y.excess > 0) {
                y.label = b.topLevelS;
                b.activeS1.add(i);
            } else {
                y.label = -b.topLevelT;
                b.activeT1.add(i);
            }
        }
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::remove_sibling(NodeIdx i)
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
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::add_sibling(NodeIdx i, NodeIdx parent)
{
    nodes[i].nextNode = nodes[parent].firstSon;
    nodes[parent].firstSon = i;
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::print_graph(std::FILE* file) const
{
    const NodeIdx num_nodes = nodes.size();
    const NodeIdx num_arcs = arcs.size();
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

/*template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline bool ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::incShouldResetTrees()
{
    // TODO: Make sure uniqOrphansS + uniqOrphansT can be compared to int64_t
    return (uniqOrphansS + uniqOrphansT) >= 2 * numNodes;
}*/

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::incNode(
    NodeIdx node, Term deltaCapSource, Term deltaCapSink)
{
    Node& x = nodes[node];

    // Initialize Incremental Phase
    /*if (incList == nullptr) {
        // init list
        incList = active0.list;
        incLen = 0;

        // reset checks
        if (incShouldResetTrees()) {
            resetTrees(1, 1);
        }
    }*/

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
    Cap f = nodes[node].excess;
    if (f > 0) {
        deltaCapSource += f;
    } else {
        deltaCapSink -= f;
    }
    if (deltaCapSource < deltaCapSink) {
        flow += deltaCapSource;
    } else {
        flow += deltaCapSink;
    }
    nodes[node].excess = deltaCapSource - deltaCapSink;

    // add to incremental list
    if (!x.isIncremental) {
        active0.add(node);
        x.isIncremental = true;
    }
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::incArc(ArcIdx ai, Cap deltaCap)
{
    if (deltaCap == 0) {
        return;
    }
    Arc& a = arcs[ai];
    Arc& sister = arcs[a.rev];
    //assert(a.rCap + sister.rCap + deltaCap < 0);

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
            } else if (invalidates_invariants(x, y)) {
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
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::addEdge(
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

/*template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::incEdge(
    NodeIdx from, NodeIdx to, Cap capacity, Cap revCapacity)
{
    Node& x = nodes[from];
    Node& y = nodes[to];
    if (arcIter == nullptr || arcIter->rev->head != x) {
        arcIter = x->firstArc;
    }
    Arc* end = (x + 1)->firstArc;
    Arc* initIter = arcIter;
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
}*/

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline int ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::isNodeOnSrcSide(NodeIdx node, int freeNodeValue)
{
    if (nodes[node].label == 0) {
        return freeNodeValue;
    }
    return (nodes[node].label > 0 ? 1 : 0);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::initGraph()
{
    initGraphFast();
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::initSize(int64_t numNodes, int64_t numEdges)
{
    this->numNodes = numNodes;
    init_n_nodes = numNodes;
    init_n_edges = numEdges;
    // compute allocation size
    /*uint64_t arcTmpMemsize = (uint64_t)sizeof(TmpEdge) * (uint64_t)numEdges;
    uint64_t arcRealMemsize = (uint64_t)sizeof(Arc) * (uint64_t)(numEdges * 2);
    uint64_t nodeMemsize = (uint64_t)sizeof(NodeIdx) * (uint64_t)(numNodes * 5);
    uint64_t arcMemsize = 0;
    arcMemsize = arcRealMemsize + arcTmpMemsize;
    if (arcMemsize < (arcRealMemsize + nodeMemsize)) {
        arcMemsize = (arcRealMemsize + nodeMemsize);
    }*/

    arcs.resize(numEdges * 2);
    tmpEdges.resize(numEdges);
    nodes.resize(numNodes + 1);
    ptrs.resize(numNodes * 5);

    // alocate arcs
    //memArcs = new char[arcMemsize];
    //tmpEdges = (TmpEdge*)(memArcs + arcRealMemsize);
    tmpEdgeLast = tmpEdges.data(); // will advance as edges are added
    //arcs = (Arc*)memArcs;
    //arcEnd = arcs + numEdges * 2;

    // allocate nodes
    //this->numNodes = numNodes;
    //nodes = new Node[numNodes + 1];
    //memset(nodes, 0, sizeof(Node) * (numNodes + 1));
    //nodeEnd = nodes + numNodes;
    // TODO: This likely uses too much memory
    //ptrs = (NodeIdx*)(arcEnd)+(3 * numNodes);
    node_blocks.resize(numNodes);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline  void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::reset()
{
    // compute allocation size
    /*uint64_t arcTmpMemsize = (uint64_t)sizeof(TmpEdge) * (uint64_t)init_n_edges;
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
    activeS1.init((NodeIdx*)(arcEnd));
    activeT1.init((NodeIdx*)(arcEnd));
    ptrs = (NodeIdx*)(arcEnd)+(3 * init_n_nodes);
    excessBuckets.init_NoAlloc(nodes, ptrs, init_n_nodes);
    orphan3PassBuckets.init_NoAlloc(nodes, init_n_nodes);
    orphanBuckets.init_NoAlloc(nodes, init_n_nodes);

    // init members
    flow = 0;*/
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::registerNodes(
    NodeIdx begin, NodeIdx end, BlockIdx block)
{
    assert(isInitializedGraph());

    if (block >= blocks.size()) {
        // We assume that we always have blocks 0,1,2,...,N
        for (BlockIdx b = blocks.size(); b <= block; ++b) {
            blocks.emplace_back(init_n_nodes, 2 * init_n_edges, nodes, arcs, ptrs, b);
            block_idxs.push_back(b);
        }
    }

    std::fill(node_blocks.begin() + begin, node_blocks.begin() + end, block);
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::initNodes()
{
    // Init active lists first
    std::fill_n(ptrs.begin(), 3 * numNodes, INVALID_NODE);

    NodeIdx i = 0;
    //for (Node* x = nodes; x <= nodeEnd; ++x, ++i) {
    for (auto& x : nodes) {
        x.firstArc = x.label;
        x.parent = INVALID_ARC;
        x.firstSon = INVALID_NODE;
        x.nextNode = INVALID_NODE;
        if (x.excess == 0) {
            x.label = 0;
        } else if (x.excess > 0) {
            x.label = 1;
            blocks[node_blocks[i]].activeS1.add(i);
            //blocks[0].activeS1.add(i);
        } else {
            x.label = -1;
            blocks[node_blocks[i]].activeT1.add(i);
            //blocks[0].activeT1.add(i);
        }
        ++i;
    }
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline ArcIdx ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::build_arc(
    NodeIdx from, NodeIdx to, Cap cap, Cap revCap)
{
    ArcIdx ai = nodes[from].firstArc;
    arcs[ai].rev = nodes[to].firstArc;
    arcs[ai].head = to;
    arcs[ai].rCap = cap;
    arcs[ai].isRevResidual = revCap != 0;
    return ai;
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline bool ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::invalidates_invariants(const Node & x, const Node & y)
{
    return (x.label > 0 && y.label <= 0)
        || (x.label >= 0 && y.label < 0)
        || (x.label > 0 && y.label > (x.label + 1))
        || (x.label < (y.label - 1) && y.label < 0);
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline typename ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::BoundaryKey
ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::block_key(BlockIdx i, BlockIdx j) const noexcept
{
    constexpr BoundaryKey shift = sizeof(BlockIdx) * 8;
    if (i < j) {
        return static_cast<BoundaryKey>(i) | (static_cast<BoundaryKey>(j) << shift);
    } else {
        return static_cast<BoundaryKey>(j) | (static_cast<BoundaryKey>(i) << shift);
    }
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline std::pair<BlockIdx, BlockIdx> ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::blocks_from_key(
    BoundaryKey key) const noexcept
{
    static_assert(sizeof(std::pair<BlockIdx, BlockIdx>) == sizeof(BoundaryKey),
        "Pair of BlockIdx does not match size of BoundaryKey");
    // Since std::pair just stores two BlockIdx fields adjacent in memory, we can just reinterpret the key
    return *reinterpret_cast<std::pair<BlockIdx, BlockIdx> *>(&key);
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline std::pair<std::list<
    typename ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::BoundarySegment>, BlockIdx>
ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::next_boundary_segment_set()
{
    // NOTE: We assume the global lock is grabbed at this point so no other threads are scanning
    std::list<BoundarySegment> out;
    BlockIdx out_idx = 0;

    // Scan until we find a boundary segment with unlocked blocks
    auto iter = boundary_segments.begin();
    for (; iter != boundary_segments.end(); ++iter) {
        const auto& bs = *iter;
        if (!blocks[block_idxs[bs.i]].locked && !blocks[block_idxs[bs.j]].locked) {
            // Found boundary between two unlocked blocks
            out_idx = block_idxs[bs.i];
            BlockIdx j = block_idxs[bs.j];
            unite_blocks(out_idx, j);
            break;
        }
    }
    // If we are not at the end, move all relevant boundary segments to output
    // Note that iter starts at the found boundary segment or at the end
    while (iter != boundary_segments.end()) {
        const auto &bs = *iter;
        if (block_idxs[bs.i] == out_idx && block_idxs[bs.j] == out_idx) {
            out.push_back(bs);
            auto to_erase = iter;
            ++iter;
            boundary_segments.erase(to_erase);
        } else {
            ++iter;
        }
    }
    return std::make_pair(out, out_idx);
}

template<class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::unite_blocks(BlockIdx i, BlockIdx j)
{
    std::replace(block_idxs.begin(), block_idxs.end(), j, i);
    auto& bi = blocks[i];
    const auto& bj = blocks[j];
    bi.augTimestamp = std::max(bi.augTimestamp, bj.augTimestamp);
    bi.topLevelS = std::max(bi.topLevelS, bj.topLevelS);
    bi.topLevelT = std::max(bi.topLevelT, bj.topLevelT);
    bi.flow += bj.flow;
    bi.uniqOrphansS += bj.uniqOrphansS;
    bi.uniqOrphansT += bj.uniqOrphansT;
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::initGraphFast()
{
    Node* x;
    TmpEdge* te;

    // calculate start arc offsets and labels for every node
    nodes[0].firstArc = 0;
    //for (x = nodes; x != nodeEnd; x++) {
    for (NodeIdx i = 0; i < nodes.size() - 1; ++i) {
        nodes[i + 1].firstArc = nodes[i].firstArc + nodes[i].label;
        nodes[i].label = nodes[i].firstArc;
    }
    nodes.back().label = arcs.size();

    // copy arcs
    //for (te = tmpEdges; te != tmpEdgeLast; te++) {
    for (const auto& te : tmpEdges) {
        NodeIdx from = te.tail;
        NodeIdx to = te.head;
        Cap cap = te.cap;
        Cap revCap = te.revCap;
       
        if (node_blocks[from] != node_blocks[to]) {
            auto key = block_key(node_blocks[from], node_blocks[to]);
            ArcIdx a1 = build_arc(from, to, 0, 0);            
            ArcIdx a2 = build_arc(to, from, 0, 0);
            boundary_arcs[key].emplace_back(a1, cap);
            boundary_arcs[key].emplace_back(a2, revCap);
        } else {
            build_arc(from, to, cap, revCap);
            build_arc(to, from, revCap, cap);
        }
        ++nodes[to].firstArc;
        ++nodes[from].firstArc;
    }

    initNodes();
}

// @ret: minimum orphan level
template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool sTree>
inline  int64_t ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::augmentPath(NodeIdx i, Cap push)
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
inline  int64_t ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::augmentExcess(NodeIdx i, Cap push)
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
inline  void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::augmentExcesses()
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
inline  void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::augment(ArcIdx bridge_idx)
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
inline  void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::adoption(
    int64_t fromLevel, bool toTop)
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
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::adoption3Pass(int64_t minBucket)
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
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::growth()
{
    // TODO: This loop could be written in a better way
    NodeIdx i;
    while ((i = active0.pop()) != INVALID_NODE) {
        // get active node
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
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
template<bool sTree>
inline void ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::augmentIncrements()
{
    //NodeIdx* end = incList + incLen;
    int64_t minOrphanLevel = 1 << 30;

    NodeIdx end = INVALID_NODE;
    NodeIdx i;
    while ((i = active0.pop()) != end) {
        Node& x = nodes[i];
        if (!x.isIncremental || (sTree ? (x.label < 0) : (x.label > 0))) {
            if (end == INVALID_NODE) {
                end = i;
            }
            active0.add(i);
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
    if (i != INVALID_NODE) {
        active0.add(i);
    }
    if (orphanBuckets.maxBucket != 0) {
        adoption<sTree>(minOrphanLevel, false);
    }
    augmentExcesses<sTree>();
}

template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline Flow ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::computeMaxFlow()
{
    flow = 0;

    std::atomic<BlockIdx> processed_blocks = 0;
    std::vector<std::thread> threads;

    // Phase 1: solve all base blocks
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            const BlockIdx num_blocks = blocks.size();
            BlockIdx crnt = processed_blocks.fetch_add(1);
            while (crnt < num_blocks) {
                blocks[crnt].computeMaxFlow(true, true);
                crnt = processed_blocks.fetch_add(1);
            }
        });
    }
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    // Build list of boundary segments
    for (const auto& ba : boundary_arcs) {
        BlockIdx i, j;
        std::tie(i, j) = blocks_from_key(ba.first);
        boundary_segments.push_back({ ba.second, i, j, 0 });
    }

    // Compute merge order heuristic
    for (auto& bs : boundary_segments) {
        int32_t broken_invariants = 0;
        for (const auto& a : bs.arcs) {
            Node& y = nodes[arcs[a.first].head];
            Node& x = nodes[arcs[arcs[a.first].rev].head];
            if (invalidates_invariants(x, y)) {
                broken_invariants++;
            }
        }
        bs.broken_invariants = broken_invariants;
    }

    boundary_segments.sort([](const auto& bs1, const auto& bs2) {
        return bs1.broken_invariants > bs2.broken_invariants;
    });

    // Phase 2: merge blocks
    threads.clear();
    std::mutex lock;


    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {

            BlockIdx crnt;
            std::list<BoundarySegment> boundary_set;
            while (true) {
                lock.lock();

                std::tie(boundary_set, crnt) = next_boundary_segment_set();
                if (boundary_set.empty()) {
                    lock.unlock();
                    break;
                }

                // Lock block
                auto& block = blocks[crnt];
                block.locked = true;

                lock.unlock();

                // Re-add boundary arcs
                for (const auto& segment : boundary_set) {
                    for (const auto& a : segment.arcs) {
                        block.incArc(a.first, a.second);
                    }
                }

                // Compute maxflow for merged block
                block.template augmentIncrements<true>();
                block.template augmentIncrements<false>();
                assert(block.active0.empty());
                block.computeMaxFlow(true, true);

                lock.lock();
                block.locked = false;
                lock.unlock();
            }
        });
    }
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    // Sum up all subgraph flows
    std::sort(block_idxs.begin(), block_idxs.end());
    auto last = std::unique(block_idxs.begin(), block_idxs.end());
    for (auto iter = block_idxs.begin(); iter != last; ++iter) {
        flow += blocks[*iter].flow;
    }
    return flow;
}


template <class Cap, class Term, class Flow, class NodeIdx, class ArcIdx>
inline Flow ParallelIbfs<Cap, Term, Flow, NodeIdx, ArcIdx>::IbfsBlock::computeMaxFlow(
    bool initialDirS, bool allowIncrements)
{
    // incremental?
    /*if (incIteration >= 1 && incList != nullptr) {
        augmentIncrements<true>();
        augmentIncrements<false>();
        incList = nullptr;
    }*/

    assert(active0.empty());

    //
    // IBFS
    //
    bool dirS = initialDirS;
    while (true) {
        // BFS level
        if (dirS) {
            std::swap(active0, activeS1);
            topLevelS++;
        } else {
            std::swap(active0, activeT1);
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
        if (!allowIncrements && (activeS1.empty() || activeT1.empty())) {
            break;
        }
        if (activeS1.empty() && activeT1.empty()) {
            break;
        }
        if (activeT1.empty()) {
            dirS = true;
        } else if (activeS1.empty()) {
            dirS = false;
        } else if (uniqOrphansT == uniqOrphansS && dirS) {
            dirS = false;
        } else if (uniqOrphansT < uniqOrphansS) {
            dirS = false;
        } else {
            dirS = true;
        }
    }

    //incIteration++;
    return flow;
}

} // namespace reimpls

#endif // REIMPLS_PARALLEL_IBFS_H__

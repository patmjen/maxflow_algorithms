#ifndef REIMPLS_PARALLEL_PR_H__
#define REIMPLS_PARALLEL_PR_H__

#include <vector>
#include <thread>
#include <atomic>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <cinttypes>
#include <cassert>

#include "tbb/concurrent_vector.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#include "util.h"

namespace reimpls {

/**
 * Parallel Push-Relabel algorithm for min-cut/max-flow based on:
 *     Efficient Implementation of a Synchronous Parallel Push-Relabel Algorithm
 *     Baumstark, N., Blelloch, G., Shun, J., 2015, ESA
 *
 * NOTE: This implementation relies on OpenMP for parallelism.
 */
template <class Cap, class Flow, class ArcIdx = uint32_t, class NodeIdx = uint32_t>
class ParallelPushRelabel {
    static_assert(std::is_integral<ArcIdx>::value, "ArcIdx must be an integer type");
    static_assert(std::is_integral<NodeIdx>::value, "NodeIdx must be an integer type");
    static_assert(std::is_signed<Cap>::value, "Cap must be a signed type");

    using Label = NodeIdx;

    // Forward decls.
    struct Node;
    struct Arc;

    static constexpr NodeIdx INVALID_NODE = ~NodeIdx(0); // -1 for signed type, max. value for unsigned type
    static constexpr ArcIdx INVALID_ARC = ~ArcIdx(0); // -1 for signed type, max. value for unsigned type

public:
    enum TermType : int32_t {
        SOURCE = 0,
        SINK = 1
    };

    ParallelPushRelabel(size_t expected_nodes, size_t expected_arcs);

    inline void set_source(NodeIdx s) noexcept { source = s; }
    inline void set_sink(NodeIdx t) noexcept { sink = t; };

    void add_edge(NodeIdx i, NodeIdx j, Cap cap, Cap rev_cap);

    TermType what_segment(NodeIdx i);

    void mincut();
    void global_relabel();

    Flow get_flow();

    inline unsigned int get_num_threads() const noexcept { return num_threads; }
    inline void set_num_threads(unsigned int num) noexcept { num_threads = num; }

private:
    std::vector<Node> nodes;
    std::vector<Arc> arcs;

    // TODO: Replace this with an inline linked list similar to active nodes in BK
    tbb::concurrent_vector<NodeIdx> working_set;
    tbb::concurrent_vector<NodeIdx> new_working_set;
    tbb::concurrent_vector<NodeIdx> q;
    tbb::concurrent_vector<NodeIdx> new_q;

    NodeIdx source;
    NodeIdx sink;

    unsigned int num_threads;

    void init_mincut();

    void add_half_edge(NodeIdx from, NodeIdx to, Cap cap);

    bool owns_arc(NodeIdx n, NodeIdx other) const;

    inline ArcIdx sister_idx(ArcIdx a) const noexcept { return a ^ 1; }
    inline Arc& sister(ArcIdx a) { return arcs[sister_idx(a)]; }
    inline const Arc& sister(ArcIdx a) const { return arcs[sister_idx(a)]; }

#pragma pack (1)
    struct REIMPLS_PACKED Node {
        ArcIdx first; // First out-going arc
        ArcIdx degree; // Number of out-going arcs
        
        Cap excess;
        std::atomic<Cap> added_excess;
        std::atomic<Label> label;
        Label label_copy;

        Label work;

        std::atomic_flag is_discovered;
        std::vector<NodeIdx> discovered_nodes; // TODO: Do this without a vector

        Node() :
            first(INVALID_ARC),
            degree(0),
            excess(0),
            added_excess(),
            label(0),
            label_copy(0),
            work(0),
            is_discovered(),
            discovered_nodes() {}
    };

    struct REIMPLS_PACKED Arc {
        ArcIdx next; // Next arc with the same originating node
        NodeIdx head; // Node this arc points to.

        Cap cap; // Capacity
        Cap flow; // Current flow

        Arc(NodeIdx head, ArcIdx next, Cap cap) :
            next(next),
            head(head),
            cap(cap),
            flow(0) {}

        inline Cap r_cap() const noexcept { return cap - flow; }
    };
#pragma pack ()
};

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::ParallelPushRelabel(
    size_t num_nodes, size_t expected_arcs) :
    nodes(num_nodes),
    arcs(),
    working_set(),
    source(INVALID_NODE),
    sink(INVALID_NODE),
    num_threads(std::thread::hardware_concurrency())
{
    arcs.reserve(2 * expected_arcs);
    working_set.reserve(num_nodes);
    new_working_set.reserve(num_nodes);
    q.reserve(num_nodes);
    new_q.reserve(num_nodes);
}

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline void ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::add_edge(NodeIdx i, NodeIdx j, Cap cap, Cap rev_cap)
{
    assert(i >= 0 && i < nodes.size());
    assert(j >= 0 && j < nodes.size());
    assert(i != j);
    assert(cap >= 0);
    assert(rev_cap >= 0);

#ifndef REIMPLS_NO_OVERFLOW_CHECKS
    if (arcs.size() > std::numeric_limits<ArcIdx>::max() - 2) {
        throw std::overflow_error("Arc count exceeds capacity of index type. "
            "Please increase capacity of ArcIdx type.");
    }
#endif

    add_half_edge(i, j, cap);
    add_half_edge(j, i, rev_cap);
}

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline typename ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::TermType 
ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::what_segment(NodeIdx i)
{
    assert(0 <= i && i < nodes.size());
    return nodes[i].label >= nodes.size() - 2 ? SOURCE : SINK;
}

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline void ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::mincut()
{
    if (source == INVALID_NODE || sink == INVALID_NODE) {
        throw std::runtime_error("Must set source and sink before calling mincut");
    }

    init_mincut();

    size_t work_since_last_global_relabel = std::numeric_limits<size_t>::max();

    while (true) {
        // Check if it's time for a global relabel
        constexpr size_t div_freq = 2; // From hi_pr: freq = 0.5
        constexpr size_t alpha = 6; // From hi_pr: alpha = 6
        if (work_since_last_global_relabel / div_freq > alpha * nodes.size() + arcs.size() / 2) {
            work_since_last_global_relabel = 0;
            global_relabel();
            // Update working set
            #pragma omp parallel for
            for (int i = 0; i < working_set.size(); ++i) {
                NodeIdx ni = working_set[i];
                if (nodes[ni].label < nodes.size()) {
                    new_working_set.push_back(ni);
                }
            }
            std::swap(new_working_set, working_set);
            new_working_set.clear();
        }

        if (working_set.empty()) {
            // We're done!
            break;
        }

        #pragma omp parallel for
        for (int i = 0; i < working_set.size(); ++i) {
            NodeIdx ni = working_set[i];
            Node& n = nodes[ni];
            n.discovered_nodes.clear();
            n.label_copy = n.label;
            Cap excess = n.excess;
            n.work = 0;
            
            while (excess > 0) {
                Label new_label = nodes.size();
                bool skipped = false;
                int scanned_arcs = 0;

                // Loop over outgoing residual arcs
                for (ArcIdx ai = n.first; ai != INVALID_ARC && excess != 0; ai = arcs[ai].next) {
                    Arc& a = arcs[ai];
                    if (!a.r_cap()) {
                        continue;
                    }
                    scanned_arcs++;
                    NodeIdx mi = a.head;
                    Node& m = nodes[mi];
                    bool admissible = n.label_copy == m.label + 1;

                    // Is this an arc between two active nodes?
                    if (m.excess != 0) {
                        if (admissible && !owns_arc(ni, mi)) {
                            skipped = true;
                            continue; // Skip to next redidual arc
                        }
                    }

                    if (admissible && a.r_cap()) {
                        Cap delta = std::min<Cap>(a.r_cap(), excess);
                        a.flow += delta;
                        sister(ai).flow -= delta;
                        excess -= delta;

                        m.added_excess.fetch_add(delta);

                        if (mi != sink && !m.is_discovered.test_and_set()) {
                            n.discovered_nodes.push_back(mi);
                        }
                    }

                    if (a.r_cap() && m.label >= n.label_copy) {
                        new_label = std::min<Cap>(new_label, m.label + 1);
                    }
                }

                if (excess == 0 || skipped) {
                    break;
                }
                n.label_copy = new_label;
                constexpr size_t beta = 12; // from hi_pr: beta = 12
                n.work += scanned_arcs + beta;
                
                if (n.label_copy == nodes.size()) {
                    break;
                }
            }

            n.added_excess += excess - n.excess;
            if (excess && !n.is_discovered.test_and_set()) {
                n.discovered_nodes.push_back(ni);
            }
        }

        #pragma omp parallel for reduction(+: work_since_last_global_relabel)
        for (int i = 0; i < working_set.size(); ++i) {
            Node& n = nodes[working_set[i]];
            n.label = n.label_copy;
            n.excess += n.added_excess;
            n.added_excess = 0;
            n.is_discovered.clear();

            work_since_last_global_relabel += n.work;
            
            for (auto ni : n.discovered_nodes) {
                if (nodes[ni].label < nodes.size()) {
                    new_working_set.push_back(ni);
                }
            }
        }
        std::swap(working_set, new_working_set);
        new_working_set.clear();


        #pragma omp parallel for
        for (int i = 0; i < working_set.size(); ++i) {
            Node& n = nodes[working_set[i]];
            n.excess += n.added_excess;
            n.added_excess = 0;
            n.is_discovered.clear();
        }
    }
}

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline void ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::global_relabel()
{
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].label = nodes.size();
    }

    nodes[sink].label = 0;
    q.push_back(sink);
    while (!q.empty()) {
        #pragma omp parallel for
        for (int i = 0; i < q.size(); ++i) {
            Node& n = nodes[q[i]];
            n.discovered_nodes.clear();
            for (ArcIdx ai = n.first; ai != INVALID_ARC; ai = arcs[ai].next) {
                NodeIdx mi = arcs[ai].head;
                if (mi != sink && mi != source && sister(ai).r_cap() > 0) {
                    Node& m = nodes[mi];
                    Label num_nodes = nodes.size();
                    if (m.label.compare_exchange_strong(num_nodes, n.label + 1)) {
                        new_q.push_back(mi);
                    }
                }
            }
        }
        std::swap(q, new_q);
        new_q.clear();
    }
}

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline Flow ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::get_flow()
{
    Flow flow = 0;
    for (ArcIdx ai = nodes[sink].first; ai != INVALID_ARC; ai = arcs[ai].next) {
        flow += sister(ai).flow;
    }
    return flow;
}

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline void ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::init_mincut()
{
#ifdef _OPENMP
    omp_set_num_threads(num_threads);
#endif

    #pragma omp parallel for
    for (int i = 0; i < nodes.size(); ++i) {
        Node& n = nodes[i];
        n.excess = 0;
        n.added_excess = 0;
        n.label = 0;
        n.work = 0;
        n.is_discovered.clear();
        n.discovered_nodes.clear();
        n.discovered_nodes.reserve(n.degree);
    }
    nodes[source].label = nodes.size();

    // Saturate all source adjacent arcs
    for (ArcIdx ai = nodes[source].first; ai != INVALID_ARC; ai = arcs[ai].next) {
        Arc& a = arcs[ai];
        a.flow = a.cap;
        sister(ai).flow = -a.cap;
        nodes[a.head].excess = a.cap;
    }

    // Add all vertices with excess to working set except the source
    #pragma omp parallel for
    for (int i = 0; i < nodes.size(); ++i) {
        if (i != source && nodes[i].excess) {
            working_set.push_back(i);
        }
    }
}

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline void ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::add_half_edge(NodeIdx from, NodeIdx to, Cap cap)
{
    ArcIdx ai = arcs.size();
    arcs.emplace_back(to, nodes[from].first, cap);
    nodes[from].first = ai;
    nodes[from].degree++;
}

template<class Cap, class Flow, class ArcIdx, class NodeIdx>
inline bool ParallelPushRelabel<Cap, Flow, ArcIdx, NodeIdx>::owns_arc(NodeIdx n, NodeIdx other) const
{
    Label nl = nodes[n].label;
    Label ol = nodes[other].label;

    return nl == ol + 1 || nl < ol - 1 || (nl == ol && n < other);
}

} // namespace reimpls

#endif // REIMPLS_PARALLEL_PR_H__

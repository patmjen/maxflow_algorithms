#ifndef REIMPLS_GRAPH_H__
#define REIMPLS_GRAPH_H__

#include <vector>
#include <deque>
#include <cinttypes>
#include <cassert>
#include <algorithm>
#include <type_traits>

#include "util.h"

namespace reimpls {

using Time = uint32_t;
using Dist = uint16_t;

template <class Cap, class Term, class Flow, class ArcIdx = int32_t, class NodeIdx = int32_t>
class Graph {
    static_assert(std::is_integral<ArcIdx>::value, "ArcIdx must be an integer type");
    static_assert(std::is_integral<NodeIdx>::value, "NodeIdx must be an integer type");
    static_assert(std::is_signed<Term>::value, "Term must be a signed type");

    // Forward decls.
    struct Node;
    struct Arc;

public:
    static const NodeIdx INVALID_NODE = ~NodeIdx(0); // -1 for signed type, max. value for unsigned type
    static const ArcIdx INVALID_ARC = ~ArcIdx(0); // -1 for signed type, max. value for unsigned type
    static const ArcIdx TERMINAL_ARC = INVALID_ARC - 1;
    static const ArcIdx ORPHAN_ARC = INVALID_ARC - 2;

    enum TermType : int32_t {
        SOURCE = 0,
        SINK = 1
    };

    explicit Graph();
    explicit Graph(size_t expected_nodes, size_t expected_arcs);

    void reserve_nodes(size_t num);
    void reserve_edges(size_t num);

    NodeIdx add_node(size_t num = 1);

    void add_tweights(NodeIdx i, Term cap_source, Term cap_sink);

    void add_edge(NodeIdx i, NodeIdx j, Cap cap, Cap rev_cap, bool merge_duplicates = true);

    Flow maxflow(bool reuse_trees = false);

    Flow get_maxflow() const noexcept { return flow; }

    TermType what_segment(NodeIdx i, TermType default_segment = SOURCE) const;

    inline size_t get_node_num() const noexcept { return nodes.size(); }
    inline size_t get_arc_num() const noexcept { return arcs.size(); }

    void mark_node(NodeIdx i);

private:
    std::vector<Node> nodes;
    std::vector<Arc> arcs;

    Flow flow;
    int32_t maxflow_iteration;

    NodeIdx first_active, last_active;
    std::deque<NodeIdx> orphan_nodes;

    Time time;

#pragma pack (1)
    struct REIMPLS_PACKED Node {
        ArcIdx first; // First out-going arc.
        ArcIdx parent; // Arc to parent node in search tree
        NodeIdx next_active; // Index of next active node (or itself if this is the last one)

        Time timestamp; // Timestamp showing when dist was computed.
        Dist dist; // Distance to terminal.

        Term tr_cap; // If tr_cap > 0 then tr_cap is residual capacity of the arc SOURCE->node.
                     // Otherwise         -tr_cap is residual capacity of the arc node->SINK.

        bool is_sink : 1;	// flag showing if the node is in the source or sink tree (if parent!=NULL)
        bool is_marked : 1; // flag showing if the node has been marked by mark_node

        Node() :
            first(INVALID_ARC),
            parent(INVALID_ARC),
            next_active(INVALID_NODE),
            timestamp(0),
            dist(0),
            tr_cap(0),
            is_sink(false) {}
    };

    struct REIMPLS_PACKED Arc {
        ArcIdx next; // Next arc with the same originating node
        NodeIdx head; // Node this arc points to.

        Cap r_cap; // Residual capacity

        Arc() :
            next(INVALID_ARC),
            head(INVALID_NODE),
            r_cap(0) {}

        Arc(NodeIdx _head, ArcIdx _next, Cap _r_cap) :
            next(_next),
            head(_head),
            r_cap(_r_cap) {}
    };
#pragma pack ()

    void add_half_edge(NodeIdx from, NodeIdx to, Cap cap, bool merge_duplicates = true);

    void init_maxflow();
    void init_maxflow_reuse_trees();

    void make_active(NodeIdx i);
    void make_front_orphan(NodeIdx i);
    void make_back_orphan(NodeIdx i);

    NodeIdx next_active();

    void augment(ArcIdx middle);
    Term tree_bottleneck(NodeIdx start, bool source_tree) const;
    void augment_tree(NodeIdx start, Term bottleneck, bool source_tree);

    ArcIdx grow_search_tree(NodeIdx start);
    template <bool source> ArcIdx grow_search_tree_impl(NodeIdx start);

    void process_orphan(NodeIdx i);
    template <bool source> void process_orphan_impl(NodeIdx i);

    inline ArcIdx sister_idx(ArcIdx a) const noexcept { return a ^ 1; }
    inline Arc& sister(ArcIdx a) { return arcs[sister_idx(a)]; }
    inline const Arc& sister(ArcIdx a) const { return arcs[sister_idx(a)]; }

    inline ArcIdx sister_or_arc_idx(ArcIdx a, bool sis) const noexcept { return a ^ (ArcIdx)sis; }
    inline Arc& sister_or_arc(ArcIdx a, bool sis) { return arcs[sister_or_arc_idx(a, sis)]; }
    inline const Arc& sister_or_arc(ArcIdx a, bool sis) const { return arcs[sister_or_arc_idx(a, sis)]; }

    inline Node& head_node(const Arc& a) { return nodes[a.head]; }
    inline Node& head_node(ArcIdx a) { return head_node(arcs[a]); }
    inline const Node& head_node(const Arc& a) const { return nodes[a.head]; }
    inline const Node& head_node(ArcIdx a) const { return head_node(arcs[a]); }
};


template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::Graph() :
    nodes(),
    arcs(),
    flow(0),
    maxflow_iteration(0),
    first_active(INVALID_NODE),
    last_active(INVALID_NODE),
    orphan_nodes(),
    time(0)
{}

template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::Graph(size_t expected_nodes, size_t expected_arcs) :
    Graph()
{
    reserve_nodes(expected_nodes);
    reserve_edges(expected_arcs);
}

template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::reserve_nodes(size_t num)
{
    nodes.reserve(num);
}

template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::reserve_edges(size_t num)
{
    arcs.reserve(2 * num);
}

template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline NodeIdx Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::add_node(size_t num)
{
    NodeIdx crnt = nodes.size();

#ifndef REIMPLS_NO_OVERFLOW_CHECKS
    if (crnt > std::numeric_limits<NodeIdx>::max() - num) {
        throw std::overflow_error("Node count exceeds capacity of index type. "
            "Please increase capacity of NodeIdx type.");
    }
#endif

    nodes.resize(crnt + num);
    return crnt;
}

template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::add_tweights(NodeIdx i, Term cap_source, Term cap_sink)
{
    assert(i >= 0 && i < nodes.size());
    Term delta = nodes[i].tr_cap;
    if (delta > 0) {
        cap_source += delta;
    } else {
        cap_sink -= delta;
    }
    flow += std::min(cap_source, cap_sink);
    nodes[i].tr_cap = cap_source - cap_sink;
}

template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::add_edge(
    NodeIdx i, NodeIdx j, Cap cap, Cap rev_cap, bool merge_duplicates)
{
    assert(i >= 0 && i < nodes.size());
    assert(j >= 0 && j < nodes.size());
    assert(i != j);
    assert(cap >= 0);
    assert(rev_cap >= 0);

    if (merge_duplicates && cap == 0 && rev_cap == 0) {
        return;
    }

#ifndef REIMPLS_NO_OVERFLOW_CHECKS
    if (arcs.size() > std::numeric_limits<ArcIdx>::max() - 2) {
        throw std::overflow_error("Arc count exceeds capacity of index type. "
            "Please increase capacity of ArcIdx type.");
    }
#endif

    add_half_edge(i, j, cap, merge_duplicates);
    add_half_edge(j, i, rev_cap, merge_duplicates);
}

template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::add_half_edge(
    NodeIdx from, NodeIdx to, Cap cap, bool merge_duplicates)
{
    ArcIdx ai;

    if (merge_duplicates) {
        // Search for existing arcs between nodes.
        ai = nodes[from].first;
        while (ai != INVALID_ARC) {
            Arc& arc = arcs[ai];
            if (arc.head == to) {
                // Existing arc found, adding capacity.
                arc.r_cap += cap;
                return;
            }
            ai = arc.next;
        }
    }

    // Create new arc.
    ai = arcs.size();
    arcs.emplace_back(to, nodes[from].first, cap);
    nodes[from].first = ai;
}

template <class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline typename Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::TermType
Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::what_segment(NodeIdx i, TermType default_segment) const
{
    if (nodes[i].parent != INVALID_ARC) {
        return (nodes[i].is_sink) ? SINK : SOURCE;
    } else {
        return default_segment;
    }
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::mark_node(NodeIdx i)
{
    make_active(i);
    nodes[i].is_marked = true;
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline Flow Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::maxflow(bool reuse_trees)
{
    if (reuse_trees) {
        init_maxflow_reuse_trees();
    } else {
        init_maxflow();
    }

    NodeIdx crnt_node = INVALID_NODE;

    // main loop
    while (true) {
        NodeIdx i = crnt_node;

        // Check if we are already exploring a valid active node
        if (i != INVALID_NODE) {
            Node& n = nodes[i];
            n.next_active = INVALID_NODE;
            if (n.parent == INVALID_ARC) {
                // Active node was not valid so don't explore after all
                i = INVALID_NODE;
            }
        }

        // If we are not already exploring a node try to get a new one
        if (i == INVALID_NODE) {
            i = next_active();
            if (i == INVALID_NODE) {
                // No more nodes to explore so we are done
                break;
            }
        }

        // At this point i must point to a valid active node
        ArcIdx source_sink_connector = grow_search_tree(i);

#ifndef REIMPLS_NO_OVERFLOW_CHECKS
        // Check for overflow in time variable.
        if (time == std::numeric_limits<Time>::max()) {
            throw std::overflow_error("Overflow in 'time' variable. Please increase capacity of Time type.");
        }
#endif
        time++;

        if (source_sink_connector != INVALID_ARC) {
            // Growth was aborted because we found a node from the other tree
            nodes[i].next_active = i; // Mark as active
            crnt_node = i;

            augment(source_sink_connector);

            std::deque<NodeIdx> crnt_orphan_nodes = std::move(orphan_nodes); // Snapshot of current ophans
            for (NodeIdx orphan : crnt_orphan_nodes) {
                process_orphan(orphan);
                // If any additional orphans were added during processing, we process them immediately
                // This leads to a significant decrease of the overall runtime
                while (!orphan_nodes.empty()) {
                    NodeIdx o = orphan_nodes.front();
                    orphan_nodes.pop_front();
                    process_orphan(o);
                }
            }
        } else {
            crnt_node = INVALID_NODE;
        }

    }

    maxflow_iteration++;
    return flow;
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::init_maxflow()
{
    first_active = INVALID_NODE;
    last_active = INVALID_NODE;
    orphan_nodes.clear();
    time = 0;

    for (size_t i = 0; i < nodes.size(); ++i) {
        Node& n = nodes[i];
        n.next_active = INVALID_NODE;
        n.is_marked = false;
        n.timestamp = time;
        if (n.tr_cap != 0) {
            // n is connected to the source or sink
            n.is_sink = n.tr_cap < 0; // negative capacity goes to sink
            n.parent = TERMINAL_ARC;
            n.dist = 1;
            make_active(i);
        } else {
            n.parent = INVALID_ARC;
        }
    }
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::init_maxflow_reuse_trees()
{
    NodeIdx i = first_active;

    // Reset queues as we are going to re-add to them during initialization
    first_active = INVALID_NODE;
    last_active = INVALID_NODE;
    orphan_nodes.clear();

    time++;

    NodeIdx next = i;
    while (next != INVALID_NODE) {
        i = next;
        Node& n = nodes[i];
        next = n.next_active;
        if (next == i) {
            // Node i is the last one so prepare to terminate
            next = INVALID_NODE;
        }
        n.next_active = INVALID_NODE;
        n.is_marked = false;
        make_active(i);

        if (n.tr_cap == 0) {
            if (n.parent != INVALID_ARC) {
                make_back_orphan(i);
            }
            continue;
        }
        if (n.tr_cap > 0) {
            if (n.parent == INVALID_ARC || n.is_sink) {
                n.is_sink = false;
                for (ArcIdx ai = n.first; ai != INVALID_ARC; ai = arcs[ai].next) {
                    NodeIdx j = arcs[ai].head;
                    const Node& m = nodes[j];
                    if (!m.is_marked) {
                        if (m.parent == sister_idx(ai)) {
                            make_back_orphan(j);
                        }
                        if (m.parent != INVALID_ARC && m.is_sink && arcs[ai].r_cap > 0) {
                            make_active(j);
                        }
                    }
                }
            }
        } else {
            // TODO: Unify with above code block as it's basically the same
            if (n.parent == INVALID_ARC || !n.is_sink) {
                n.is_sink = true;
                for (ArcIdx ai = n.first; ai != INVALID_ARC; ai = arcs[ai].next) {
                    NodeIdx j = arcs[ai].head;
                    const Node& m = nodes[j];
                    if (!m.is_marked) {
                        if (m.parent == sister_idx(ai)) {
                            make_back_orphan(j);
                        }
                        if (m.parent != INVALID_ARC && !m.is_sink && sister(ai).r_cap > 0) {
                            make_active(j);
                        }
                    }
                }
            }
        }
        n.parent = TERMINAL_ARC;
        n.timestamp = time;
        n.dist = 1;
    }

    // Adoption
    while (!orphan_nodes.empty()) {
        NodeIdx o = orphan_nodes.front();
        orphan_nodes.pop_front();
        process_orphan(o);
    }
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::make_active(NodeIdx i)
{
    if (nodes[i].next_active == INVALID_NODE) {
        // It's not in the active list yet
        if (last_active != INVALID_NODE) {
            nodes[last_active].next_active = i;
        }
        else {
            first_active = i;
        }
        last_active = i;
        nodes[i].next_active = i;
    }
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::make_front_orphan(NodeIdx i)
{
    nodes[i].parent = ORPHAN_ARC;
    orphan_nodes.push_front(i);
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::make_back_orphan(NodeIdx i)
{
    nodes[i].parent = ORPHAN_ARC;
    orphan_nodes.push_back(i);
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline NodeIdx Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::next_active()
{
    NodeIdx i;
    // Pop nodes from the active list until we find a valid one or run out of nodes
    for (i = first_active; i != INVALID_NODE; i = first_active) {
        // Pop node from active list
        Node& n = nodes[i];
        if (n.next_active == i) {
            // This is the last node in the active list so "clear" it
            first_active = INVALID_NODE;
            last_active = INVALID_NODE;
        } else {
            first_active = n.next_active;
        }
        n.next_active = INVALID_NODE; // Mark as not active

        if (n.parent != INVALID_ARC) {
            // Valid active node found
            break;
        }
    }
    return i;
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::augment(ArcIdx middle_idx)
{
    Arc& middle = arcs[middle_idx];
    Arc& middle_sister = sister(middle_idx);
    // Step 1: Find bottleneck capacity
    Term bottleneck = middle.r_cap;
    bottleneck = std::min(bottleneck, tree_bottleneck(middle_sister.head, true));
    bottleneck = std::min(bottleneck, tree_bottleneck(middle.head, false));

    // Step  2: Augment along source and sink tree
    middle_sister.r_cap += bottleneck;
    middle.r_cap -= bottleneck;
    augment_tree(middle_sister.head, bottleneck, true);
    augment_tree(middle.head, bottleneck, false);

    // Step 3: Add bottleneck to overall flow
    flow += bottleneck;
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline Term Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::tree_bottleneck(NodeIdx start, bool source_tree) const
{
    NodeIdx i = start;
    Term bottleneck = std::numeric_limits<Term>::max();
    while (true) {
        ArcIdx a = nodes[i].parent;
        if (a == TERMINAL_ARC) {
            break;
        }
        Cap r_cap = source_tree ? sister(a).r_cap : arcs[a].r_cap;
        bottleneck = std::min<Term>(bottleneck, r_cap);
        i = arcs[a].head;
    }
    const Term tr_cap = nodes[i].tr_cap;
    return std::min<Term>(bottleneck, source_tree ? tr_cap : -tr_cap);
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::augment_tree(
    NodeIdx start, Term bottleneck, bool source_tree)
{
    NodeIdx i = start;
    while (true) {
        ArcIdx ai = nodes[i].parent;
        if (ai == TERMINAL_ARC) {
            break;
        }
        Arc& a = source_tree ? arcs[ai] : sister(ai);
        Arc& b = source_tree ? sister(ai) : arcs[ai];
        a.r_cap += bottleneck;
        b.r_cap -= bottleneck;
        if (b.r_cap == 0) {
            make_front_orphan(i);
        }
        i = arcs[ai].head;
    }
    nodes[i].tr_cap += source_tree ? -bottleneck : bottleneck;
    if (nodes[i].tr_cap == 0) {
        make_front_orphan(i);
    }
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline ArcIdx Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::grow_search_tree(NodeIdx start)
{
    return nodes[start].is_sink ? grow_search_tree_impl<false>(start) : grow_search_tree_impl<true>(start);
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
template<bool source>
inline ArcIdx Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::grow_search_tree_impl(NodeIdx start_idx)
{
    const Node& start = nodes[start_idx];
    ArcIdx ai;
    // Add neighbor nodes search tree until we find a node from the other search tree or run out of neighbors
    for (ai = start.first; ai != INVALID_ARC; ai = arcs[ai].next) {
        if (sister_or_arc(ai, !source).r_cap > 0) {
            Node& n = head_node(ai);
            if (n.parent == INVALID_ARC) {
                // This node is not yet in a tree so claim it for this one
                n.is_sink = !source;
                n.parent = sister_idx(ai);
                n.timestamp = start.timestamp;
                n.dist = start.dist + 1;
                make_active(arcs[ai].head);
            } else if (n.is_sink == source) {
                // Found a node from the other search tree so abort
                if (!source) {
                    // If we are growing the sink tree we instead return the sister arc
                    ai = sister_idx(ai);
                }
                break;
            } else if (n.timestamp <= start.timestamp && n.dist > start.dist) {
                // Heuristic: trying to make the distance from j to the source/sink shorter
                n.parent = sister_idx(ai);
                n.timestamp = n.timestamp;
                n.dist = start.dist + 1;
            }
        }
    }
    return ai;
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::process_orphan(NodeIdx i)
{
    if (nodes[i].is_sink) {
        process_orphan_impl<false>(i);
    } else {
        process_orphan_impl<true>(i);
    }
}

template<class Cap, class Term, class Flow, class ArcIdx, class NodeIdx>
template<bool source>
inline void Graph<Cap, Term, Flow, ArcIdx, NodeIdx>::process_orphan_impl(NodeIdx i)
{
    Node &n = nodes[i];
    static const int32_t INF_DIST = std::numeric_limits<int32_t>::max();
    int32_t min_d = INF_DIST;
    ArcIdx min_a0 = INVALID_ARC;
    // Try to find a new parent
    for (ArcIdx a0 = n.first; a0 != INVALID_ARC; a0 = arcs[a0].next) {
        if (sister_or_arc(a0, source).r_cap > 0) {
            NodeIdx j = arcs[a0].head;
            ArcIdx a = nodes[j].parent;
            if (nodes[j].is_sink != source && a != INVALID_ARC) {
                // Check origin of m
                int32_t d = 0;
                while (true) {
                    Node &m = nodes[j];
                    if (m.timestamp == time) {
                        d += m.dist;
                        break;
                    }
                    a = m.parent;
                    d++;
                    if (a == TERMINAL_ARC) {
                        m.timestamp = time;
                        m.dist = 1;
                        break;
                    }
                    if (a == ORPHAN_ARC) {
                        d = INF_DIST; // infinite distance
                        break;
                    }
                    j = arcs[a].head;
                }
                if (d < INF_DIST) {
                    // m originates from the source
                    if (d < min_d) {
                        min_a0 = a0;
                        min_d = d;
                    }
                    // Set marks along the path
                    j = arcs[a0].head;
                    while (nodes[j].timestamp != time) {
                        Node &m = nodes[j];
                        m.timestamp = time;
                        m.dist = d--;
                        j = arcs[m.parent].head;
                    }
                }
            }
        }
    }
    n.parent = min_a0;
    if (min_a0 != INVALID_ARC) {
        n.timestamp = time;
        n.dist = min_d + 1;
    } else {
        // No parent was found so process neighbors
        for (ArcIdx a0 = n.first; a0 != INVALID_ARC; a0 = arcs[a0].next) {
            NodeIdx j = arcs[a0].head;
            Node &m = nodes[j];
            if (m.is_sink != source && m.parent != INVALID_ARC) {
                if (sister_or_arc(a0, source).r_cap > 0) {
                    make_active(j);
                }
                ArcIdx pa = m.parent;
                if (pa != TERMINAL_ARC && pa != ORPHAN_ARC && arcs[pa].head == i) {
                    make_back_orphan(j);
                }
            }
        }
    }
}

} // namespace reimpls

#endif // REIMPLS_GRAPH_H__

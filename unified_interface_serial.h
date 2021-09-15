#ifndef UNIFIED_INTERFACE_SERIAL_H__
#define UNIFIED_INTERFACE_SERIAL_H__

#include <cinttypes>

#include "reimpls/graph.h"
#include "reimpls/ibfs.h"
#include "reimpls/hpf.h"

namespace unified {

enum TermType : int32_t {
    SOURCE = 0,
    SINK = 1
};

template <class Cap, class Term, class Flow, class ArcIdx = uint32_t, class NodeIdx = uint32_t>
class SerialMaxflow {
public:
    virtual void allocate_size(size_t num_nodes, size_t num_term_arcs, size_t num_nbor_arcs) = 0;
    virtual void add_tweights(NodeIdx i, Term cap_source, Term cap_sink) = 0;
    virtual void add_edge(NodeIdx from, NodeIdx to, Cap cap, Cap rev_cap) = 0;
    virtual void init_maxflow() = 0;
    virtual void maxflow() = 0;
    virtual Flow get_maxflow() const = 0;
    virtual TermType what_segment(NodeIdx i, TermType default_segment = SOURCE) const = 0;
};

template <class Cap, class Term, class Flow, class ArcIdx = uint32_t, class NodeIdx = uint32_t>
class SerialBk : public SerialMaxflow<Cap, Term, Flow, ArcIdx, NodeIdx> {
public:
    explicit SerialBk() :
        graph() {}

    explicit SerialBk(size_t num_nodes, size_t num_term_arcs, size_t num_nbor_arcs) :
        graph(num_nodes, num_nbor_arcs) {}

    void allocate_size(size_t num_nodes, size_t num_term_arcs, size_t num_nbor_arcs) override
    {
        graph.add_node(num_nodes);
        graph.reserve_edges(num_nbor_arcs);
    }

    void add_tweights(NodeIdx i, Term cap_source, Term cap_sink) override
    {
        graph.add_tweights(i, cap_source, cap_sink);
    }

    void add_edge(NodeIdx from, NodeIdx to, Cap cap, Cap rev_cap) override
    {
        graph.add_edge(from, to, cap, rev_cap, false);
    }

    void init_maxflow() override { /* Do nothing */ }

    void maxflow() override
    {
        graph.maxflow();
    }

    Flow get_maxflow() const override
    {
        return graph.get_maxflow();
    }

    TermType what_segment(NodeIdx i, TermType default_segment = SOURCE) const override
    {
        using T = decltype(graph)::TermType;
        return static_cast<TermType>(graph.what_segment(i, static_cast<T>(default_segment)));
    }

private:
    reimpls::Graph<Cap, Term, Flow, ArcIdx, NodeIdx> graph;
};

template <class Cap, class Term, class Flow, class ArcIdx = uint32_t, class NodeIdx = uint32_t>
class SerialEibfs : public SerialMaxflow<Cap, Term, Flow, ArcIdx, NodeIdx> {
public:
    explicit SerialEibfs() :
        graph() {}

    explicit SerialEibfs(size_t num_nodes, size_t num_term_arcs, size_t num_nbor_arcs) :
        graph(num_nodes, num_nbor_arcs) {}

    void allocate_size(size_t num_nodes, size_t num_term_arcs, size_t num_nbor_arcs) override
    {
        graph.initSize(num_nodes, num_nbor_arcs);
    }

    void add_tweights(NodeIdx i, Term cap_source, Term cap_sink) override
    {
        graph.addNode(i, cap_source, cap_sink);
    }

    void add_edge(NodeIdx from, NodeIdx to, Cap cap, Cap rev_cap) override
    {
        graph.addEdge(from, to, cap, rev_cap);
    }

    void init_maxflow() override
    {
        graph.initGraph();
    }

    void maxflow() override
    {
        graph.computeMaxFlow();
    }

    Flow get_maxflow() const override
    {
        return graph.getFlow();
    }

    TermType what_segment(NodeIdx i, TermType default_segment = SOURCE) const override
    {
        int free_node_value = (default_segment == SOURCE) ? 1 : 0;
        return (graph.isNodeOnSrcSide(i, free_node_value) == 1) ? SOURCE : SINK;
    }

private:
    reimpls::IBFSGraph<Cap, Term, Flow, NodeIdx, ArcIdx> graph;
};

template <class Cap>
class SerialHpf : public SerialMaxflow<Cap, Cap, Cap, uint32_t, uint32_t> {
    // TODO: Make these configurable in HPF class too
    using Term = Cap;
    using Flow = Cap;
    using ArcIdx = uint32_t;
    using NodeIdx = uint32_t;

    static const NodeIdx SOURCE_NODE = 0;
    static const NodeIdx SINK_NODE = 1;
public:
    explicit SerialHpf() :
        graph()
    {
        graph.set_source(SOURCE_NODE);
        graph.set_sink(SINK_NODE);
    }

    explicit SerialHpf(size_t num_nodes, size_t num_term_arcs, size_t num_nbor_arcs) :
        graph(num_nodes + 2, num_term_arcs + num_nbor_arcs)
    {
        graph.set_source(SOURCE_NODE);
        graph.set_sink(SINK_NODE);
    }

    void allocate_size(size_t num_nodes, size_t num_term_arcs, size_t num_nbor_arcs)
    {
        graph.add_node(num_nodes + 2);
        graph.reserve_edges(num_term_arcs + num_nbor_arcs);
    }

    void add_tweights(NodeIdx i, Term cap_source, Term cap_sink) override
    {
        if (cap_source != 0) {
            graph.add_edge(SOURCE_NODE, i + 2, cap_source);
        }
        if (cap_sink != 0) {
            graph.add_edge(i + 2, SINK_NODE, cap_sink);
        }
    }

    void add_edge(NodeIdx from, NodeIdx to, Cap cap, Cap rev_cap) override
    {
        if (cap != 0) {
            graph.add_edge(from + 2, to + 2, cap);
        }
        if (rev_cap != 0) {
            graph.add_edge(to + 2, from + 2, rev_cap);
        }
    }

    void init_maxflow() override { /* Do nothing */ }

    void maxflow() override
    {
        graph.mincut();
    }

    Flow get_maxflow() const override
    {
        return graph.compute_maxflow();
    }

    TermType what_segment(NodeIdx i, TermType default_segment = SOURCE) const override
    {
        constexpr auto GRAPH_SOURCE = decltype(graph)::TermType::SOURCE;
        return (graph.what_label(i) == GRAPH_SOURCE) ? SOURCE : SINK;
    }

private:
    reimpls::Hpf<Cap, reimpls::LabelOrder::HIGHEST_FIRST, reimpls::RootOrder::LIFO> graph;
};


} // namespace unified

#endif // UNIFIED_INTERFACE_H__
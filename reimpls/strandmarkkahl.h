#ifndef REIMPLS_PARALLEL_SK_H__
#define REIMPLS_PARALLEL_SK_H__

#include <vector>
#include <memory>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <numeric>
#include <tuple>
#include <thread>
#include <iostream> // DEBUG
#include <cassert>
#include <cstdlib>
#include <cinttypes>

#include "graph.h"
#include "util.h"

namespace reimpls {

/**
 * Implementation of the parallel BK algorithm from:
 *     Parallel and Distributed Graph Cuts by Dual Decomposition
 *     Strandmark, P., Kahl, F., 2010, CVPR
 */
template <class Cap, class Term, class Flow, class NodeIdx = int32_t>
class ParallelSkGraph {
    using BlockIdx = int16_t;

    static_assert(std::is_signed<NodeIdx>::value, "NodeIdx must a signed type");
    static_assert(std::is_signed<BlockIdx>::value, "BlockIdx must be a signed type");

    static constexpr BlockIdx INVALID_BLOCK = std::numeric_limits<BlockIdx>::max();
    static constexpr NodeIdx INVALID_NODE = std::numeric_limits<NodeIdx>::max();

    static constexpr bool INTEGER_CAPACITIES = std::is_integral<Cap>::value || std::is_integral<Term>::value;

public:
    explicit ParallelSkGraph(size_t expected_nodes, size_t expected_edges_per_block);

    void add_node(NodeIdx num);
    void add_nodes_to_block(NodeIdx begin, NodeIdx end, BlockIdx b);

    void add_edge(NodeIdx i, NodeIdx j, Cap cap, Cap rev_cap);
    void add_tweights(NodeIdx i, Term source_cap, Term sink_cap);

    int what_segment(NodeIdx i, int default_segment = SOURCE); // TODO: Use an enum

    Flow maxflow();

    inline unsigned int get_iter() const noexcept { return iter; }

    inline unsigned int get_max_iter() const noexcept { return max_iter; }
    inline void set_max_iter(unsigned int mi) const noexcept { max_iter = mi; }

private:
    Flow flow;

    unsigned int iter;
    unsigned int max_iter;

    std::vector<std::shared_ptr<reimpls::Graph<Cap, Term, Flow>>> blocks;

    size_t expected_edges_per_block;
    std::vector<BlockIdx> node_blocks;
    std::vector<NodeIdx> node_offsets; // i >= 0: block index, i < 0: -i-1 is index in shared_node_*
    std::vector<std::vector<BlockIdx>> shared_node_blocks;
    std::vector<std::vector<NodeIdx>> shared_node_offsets;
    std::vector<NodeIdx> shared_nodes;

    void update_graph(NodeIdx i, int diff, int prev_diff, Cap& step, uint8_t& has_flipped,
        BlockIdx b1, BlockIdx b2, NodeIdx offset1, NodeIdx offset2);

    NodeIdx shared_index(NodeIdx i) const;
    inline bool is_shared(NodeIdx i) const { return node_offsets[i] < 0; }

    NodeIdx get_block_offset(NodeIdx node, BlockIdx b) const;
    std::vector<std::tuple<BlockIdx, NodeIdx, NodeIdx>> get_shared_block_offsets(NodeIdx i, NodeIdx j) const;

};

template<class Cap, class Term, class Flow, class NodeIdx>
inline ParallelSkGraph<Cap, Term, Flow, NodeIdx>::ParallelSkGraph(
    size_t expected_nodes, size_t expected_edges_per_block) :
    flow(0),
    iter(0),
    max_iter(1000), // Same default as Kahl-Strandmark reference 
    blocks(),
    expected_edges_per_block(expected_edges_per_block),
    node_blocks(),
    node_offsets(),
    shared_node_blocks(),
    shared_node_offsets(),
    shared_nodes()
{
    node_blocks.reserve(expected_nodes);
    node_offsets.reserve(expected_nodes);
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline void ParallelSkGraph<Cap, Term, Flow, NodeIdx>::add_node(NodeIdx num)
{
    node_blocks.resize(num);
    node_offsets.resize(num, INVALID_NODE); // Need to init. so we can tell if node is shared
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline void ParallelSkGraph<Cap, Term, Flow, NodeIdx>::add_nodes_to_block(
    NodeIdx begin, NodeIdx end, BlockIdx b)
{
    if (b >= blocks.size()) {
        blocks.resize(b + 1);
    }
    if (blocks[b] == nullptr) {
        //blocks[b] = std::make_shared<bk::Graph<Cap, Term, Flow>>(end - begin, 0, bk_error_handler);
        blocks[b] = std::make_shared<reimpls::Graph<Cap, Term, Flow>>(end - begin, expected_edges_per_block);
    }

    NodeIdx offset = begin - blocks[b]->add_node(end - begin);
    
    for (NodeIdx i = begin; i < end; ++i) {
        if (node_offsets[i] == INVALID_NODE) {
            // First time we're adding this node so just update the index
            node_blocks[i] = b;
            node_offsets[i] = offset;
        } else if (!is_shared(i)) {
            // Second time we're adding this node so make it shared
            assert(shared_node_blocks.size() == shared_node_offsets.size());
            assert(shared_node_blocks.size() == shared_nodes.size());
            NodeIdx new_shared_index = shared_nodes.size();
            shared_node_blocks.push_back({ node_blocks[i], b });
            shared_node_offsets.push_back({ node_offsets[i], offset });
            shared_nodes.push_back(i);
            node_offsets[i] = -new_shared_index - 1;
        } else {
            // Third time or later
            NodeIdx si = shared_index(i);
            shared_node_blocks[si].push_back(b);
            shared_node_offsets[si].push_back(offset);
        }
    }
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline void ParallelSkGraph<Cap, Term, Flow, NodeIdx>::add_edge(NodeIdx i, NodeIdx j, Cap cap, Cap rev_cap)
{
    assert(i != j);
    assert(cap >= 0);
    assert(rev_cap >= 0);

    bool i_shared = is_shared(i);
    bool j_shared = is_shared(j);
    if (!i_shared || !j_shared) {
        // Node i and/or j is not shared
        BlockIdx b = i_shared ? node_blocks[j] : node_blocks[i];
        NodeIdx offset_i = i_shared ? get_block_offset(i, b) : node_offsets[i];
        NodeIdx offset_j = j_shared ? get_block_offset(j, b) : node_offsets[j];
        blocks[b]->add_edge(i - offset_i, j - offset_j, cap, rev_cap);
    } else {
        // Both node i and j are shared
        BlockIdx b;
        NodeIdx offset_i, offset_j;
        auto shared_block_offsets = get_shared_block_offsets(i, j);
        Cap num_blocks = shared_block_offsets.size();
        Cap sum_cap = 0;
        Cap sum_rev_cap = 0;
        for (int ii = 0; ii < shared_block_offsets.size(); ++ii) {
            const auto& shared_offset = shared_block_offsets[ii];
            std::tie(b, offset_i, offset_j) = shared_offset;
            if (ii < shared_block_offsets.size() - 1) {
                blocks[b]->add_edge(i - offset_i, j - offset_j, cap / num_blocks, rev_cap / num_blocks, false);
                sum_cap += cap / num_blocks;
                sum_rev_cap += rev_cap / num_blocks;
            } else {
                // Last block so add remaining capcaity
                blocks[b]->add_edge(i - offset_i, j - offset_j, cap - sum_cap, rev_cap - sum_rev_cap, false);
            }
        }
    }
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline void ParallelSkGraph<Cap, Term, Flow, NodeIdx>::add_tweights(
    NodeIdx i, Term source_cap, Term sink_cap)
{
    if (node_offsets[i] >= 0) {
        // Node is not shared so just add the weights
        auto& block = blocks[node_blocks[i]];
        block->add_tweights(i - node_offsets[i], source_cap, sink_cap);
    } else {
        // Node is shared to distribute the weights to all blocks
        NodeIdx si = shared_index(i);
        Cap num_blocks = shared_node_blocks[si].size();
        Term sum_source = 0;
        Term sum_sink = 0;
        for (int ii = 0; ii < shared_node_blocks[si].size(); ++ii) {
            auto& block = blocks[shared_node_blocks[si][ii]];
            if (ii < shared_node_blocks[si].size() - 1) {
                block->add_tweights(i - shared_node_offsets[si][ii], 
                    source_cap / num_blocks, sink_cap / num_blocks);
                sum_source += source_cap / num_blocks;
                sum_sink += sink_cap / num_blocks;
            } else {
                // Last block so add remaining capacity
                block->add_tweights(i - shared_node_offsets[si][ii], 
                    source_cap - sum_source, sink_cap - sum_sink);
            }
        }
    }
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline int ParallelSkGraph<Cap, Term, Flow, NodeIdx>::what_segment(NodeIdx i, int default_segment)
{
    return blocks[node_blocks[i][0]]->what_segment(i - node_offsets[i][0], default_segment);
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline Flow ParallelSkGraph<Cap, Term, Flow, NodeIdx>::maxflow()
{
    Cap stepsize = 10;

    Barrier barr(blocks.size() + 1); // Workers + master thread

    bool running = true;
    std::vector<Flow> block_flows(blocks.size(), 0);
    std::vector<std::thread> threads;

    // Bookkeeping for steps
    std::vector<Cap> steps(shared_nodes.size(), stepsize);
    std::vector<int> prev_diffs(shared_nodes.size(), 0);
    std::vector<uint8_t> has_flipped(shared_nodes.size(), false); // uint8_t to avoid "vector of bool" optim.

    // Launch worker threads
    for (unsigned int t = 0; t < blocks.size(); ++t) {
        threads.emplace_back([&](unsigned int t)
        {
            bool reuse_trees = false;
            while (running) {
                // First maxflow without reusing trees, then enable it
                // This is essential for good performance
                block_flows[t] = blocks[t]->maxflow(reuse_trees);
                reuse_trees = true;

                // Wait for all workers to finish
                barr.wait(); // Stop 1

                // Wait for the main thread to process the graphs
                barr.wait(); // Stop 2
            }
        }, t);
    }

    // Start supergradient ascent
    iter = 0;
    while (running) {
        iter++;
        // Wait for all blocks to finish
        barr.wait(); // Stop 1

        // Go through shared nodes and look at assigments
        NodeIdx num_diff = 0;
        for (NodeIdx si = 0; si < shared_nodes.size(); ++si) {
            NodeIdx node = shared_nodes[si];
            for (int i = 0; i < shared_node_blocks[si].size() - 1; ++i) {
                BlockIdx b1 = shared_node_blocks[si][i];
                BlockIdx b2 = shared_node_blocks[si][i + 1];
                NodeIdx offset1 = shared_node_offsets[si][i];
                NodeIdx offset2 = shared_node_offsets[si][i + 1];

                int l1 = blocks[b1]->what_segment(node - offset1);
                int l2 = blocks[b2]->what_segment(node - offset2);
                
                if (l1 != l2) {
                    num_diff++;
                    int diff = l1 - l2;
                    update_graph(node, diff, prev_diffs[si], steps[si], has_flipped[si], 
                        b1, b2, offset1, offset2);
                    prev_diffs[si] = diff;
                }
            }
        }

        if (num_diff == 0 || iter >= max_iter) {
            // All blocks agreed or we ran out of iterations
            running = false;
        }

        // We're done processing the graph and ready for the next iteration
        barr.wait(); // Stop 2
    }

    // Sum up all subgraph flows
    flow = std::accumulate(block_flows.begin(), block_flows.end(), 0);

    // Wait for all theads to completely finish
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return flow;
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline void ParallelSkGraph<Cap, Term, Flow, NodeIdx>::update_graph(
    NodeIdx i, int diff, int prev_diff, Cap& step, uint8_t& has_flipped,
    BlockIdx b1, BlockIdx b2, NodeIdx offset1, NodeIdx offset2)
{
    // For integer capacities we sometimes do nothing
    // This helps with potential convergence issues
    if (INTEGER_CAPACITIES && iter > 15 && std::rand() % 2 == 0) {
        return;
    }

    if (prev_diff * diff == -1) {
        // Both labels flipped during last maxflow computation
        // This indicates the step is too large so decrease and don't increase again
        step = std::max<Cap>(1, step / 2);
        has_flipped = true;
    } else if (prev_diff * diff == 1 && !has_flipped) {
        // Same difference as last time, and we haven't flipped yet
        // This indicates we can increase the step size
        step *= 2;
    }

    // Change graphs
    Cap change = diff * step;
    blocks[b1]->add_tweights(i - offset1, change, 0);
    blocks[b2]->add_tweights(i - offset2, -change, 0);

    blocks[b1]->mark_node(i - offset1);
    blocks[b2]->mark_node(i - offset2);
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline NodeIdx ParallelSkGraph<Cap, Term, Flow, NodeIdx>::shared_index(NodeIdx i) const
{
    assert(is_shared(i));
    return -node_offsets[i] - 1;
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline NodeIdx ParallelSkGraph<Cap, Term, Flow, NodeIdx>::get_block_offset(NodeIdx node, BlockIdx b) const
{
    NodeIdx si = shared_index(node);
    for (int i = 0; i < shared_node_blocks[si].size(); ++i) {
        if (shared_node_blocks[si][i] == b) {
            return shared_node_offsets[si][i];
        }
    }
    assert(false);
    throw std::runtime_error("Node is not in given block");
}

template<class Cap, class Term, class Flow, class NodeIdx>
inline std::vector<
    std::tuple<typename ParallelSkGraph<Cap, Term, Flow, NodeIdx>::BlockIdx, NodeIdx, NodeIdx>>
ParallelSkGraph<Cap, Term, Flow, NodeIdx>::get_shared_block_offsets(NodeIdx i, NodeIdx j) const
{
    std::vector<std::tuple<BlockIdx, NodeIdx, NodeIdx>> out;
    // We just use a naive search since all vectors are very short
    NodeIdx si = shared_index(i);
    NodeIdx sj = shared_index(j);
    const auto& blocks_i = shared_node_blocks[si];
    const auto& blocks_j = shared_node_blocks[sj];
    for (int ii = 0; ii < blocks_i.size(); ++ii) {
        for (int ij = 0; ij < blocks_j.size(); ++ij) {
            BlockIdx bi = blocks_i[ii];
            BlockIdx bj = blocks_j[ij];
            if (bi == bj) {
                out.emplace_back(bi, shared_node_offsets[si][ii], shared_node_offsets[sj][ij]);
            }
        }
    }
    return out;
}

} // namespace reimpls

#endif // REIMPLS_PARALLEL_SK_H__

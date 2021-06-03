#include <iostream>
#include <chrono>
#include <stdexcept>
#include <typeinfo>
#include <string>
#include <fstream>
#include <tuple>

#include "json.hpp"

#include "graph_io.h"
#include "bk/graph.h"
#include "nbk/graph.h"
#include "reimpls/graph.h"
#include "reimpls/graph2.h"
#include "reimpls/parallel_graph.h"
#include "reimpls/parallel_ibfs.h"
#include "reimpls/ibfs.h"
#include "reimpls/ibfs2.h"
#include "reimpls/parallel_pr.h"
#include "reimpls/parallel_sk.h"
#include "ibfs/ibfs.h"
#include "reimpls/hpf.h"
#include "hi_pr/hi_pr.h"
#include "sppr/maxFlow.h"

#include <cstdlib>
#include "d_maxflow/parallel_ARD1.h"
#include "d_maxflow/dimacs_parser.h"
#include "d_maxflow/region_graph.h"
#include "d_maxflow/region_splitter2.h"

#include "reimpls/robin_hood.h"

using Duration = std::chrono::duration<double>;
static const auto now = std::chrono::steady_clock::now;

using json = nlohmann::json;

enum Algorithm {
    // Serial algorithms
    ALGO_BK,
    ALGO_NBK,
    ALGO_MBK,
    ALGO_MBK2,
    ALGO_EIBFS,
    ALGO_EIBFS2,
    ALGO_EIBFS_OLD,
    ALGO_HPF,
    ALGO_HPF_HF,
    ALGO_HPF_HL,
    ALGO_HPF_LF,
    ALGO_HPF_LL,
    ALGO_HI_PR,

    // Parallel algorithms
    ALGO_PMBK,
    ALGO_PPR,
    ALGO_PSK,
    ALGO_PARD,
    ALGO_PEIBFS,

    // Dummy just to check loading and output
    ALGO_DUMMY
};

enum FileType {
    FTYPE_DIMACS,
    FTYPE_BBK,
    FTYPE_BQ
};

struct BenchConfig {
    std::string bench_name;

    TypeCode cap_type;
    TypeCode term_type;
    TypeCode flow_type;
    TypeCode index_type;

    Algorithm algo;

    int num_run;
    int num_threads;
};

struct DataConfig {
    std::string bench_name;

    TypeCode nbor_cap_type;
    TypeCode term_cap_type;

    std::string file_name;
    FileType file_type;
};

#define SWITCH_ON_SIGNED_TYPE(type, name, ...) switch (type) { \
    /*case TYPE_INT8: { using name = int8_t; __VA_ARGS__ } break;*/ \
    /*case TYPE_INT16: { using name = int16_t; __VA_ARGS__ } break;*/ \
    case TYPE_INT32: { using name = int32_t; __VA_ARGS__ } break; \
    case TYPE_INT64: { using name = int64_t; __VA_ARGS__ } break; \
    /*case TYPE_FLOAT: { using name = float; __VA_ARGS__ } break;*/ \
    /*case TYPE_DOUBLE: { using name = double; __VA_ARGS__ } break;*/ \
    default: throw std::runtime_error("Invalid type code for signed type."); \
}

#define SWITCH_ON_INDEX_TYPE(type, name, ...) switch (type) { \
    /*case TYPE_UINT16: { using name = uint16_t; __VA_ARGS__ } break;*/ \
    /*case TYPE_INT16: { using name = int16_t; __VA_ARGS__ } break;*/ \
    case TYPE_UINT32: { using name = uint32_t; __VA_ARGS__ } break; \
    case TYPE_INT32: { using name = int32_t; __VA_ARGS__ } break; \
    case TYPE_INT64: { using name = int64_t; __VA_ARGS__ } break; \
    case TYPE_UINT64: { using name = int64_t; __VA_ARGS__ } break; \
    default: throw std::runtime_error("Invalid type code for index."); \
}

#define SWITCH_ON_FLOW_TYPE(type, name, ...) switch (type) { \
    case TYPE_INT64: { using name = int64_t; __VA_ARGS__ } break; \
    /*case TYPE_DOUBLE: { using name = double; __VA_ARGS__ } break;*/ \
    default: throw std::runtime_error("Invalid type code for flow."); \
}

#define RUN_BENCH_FUNC(data_config, bench_config, data, func) do { \
    SWITCH_ON_SIGNED_TYPE(bench_config.cap_type, CapType, \
        SWITCH_ON_FLOW_TYPE(bench_config.flow_type, FlowType, \
            SWITCH_ON_INDEX_TYPE(bench_config.index_type, IndexType, \
                func<CapType, CapType, FlowType, IndexType, decltype(data)>(data_config, bench_config, data);))) \
} while (false)

TypeCode code_from_string(const std::string& str);
Algorithm algo_from_string(const std::string& str);
const char* algo_to_string(Algorithm algo);
FileType ftype_from_string(const std::string& str);

bool algo_is_parallel(Algorithm algo);

std::vector<BenchConfig> gen_bench_configs(json config);
std::vector<DataConfig> gen_data_configs(json config);

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double> bench_bk(BenchConfig config, const Data& data)
{
    // Build graph.
    auto build_begin = now();
    bk::Graph<Cap, Term, Flow> graph(data.num_nodes, data.neighbor_arcs.size());
    graph.add_node(data.num_nodes);
    for (const auto& tarc : data.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    auto flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double> bench_nbk(BenchConfig config, const Data& data)
{
	// Build graph.
	auto build_begin = now();
	nbk::Graph<Cap, Term, Flow> graph(data.num_nodes, data.neighbor_arcs.size());
	graph.add_node(data.num_nodes);
	for (const auto& tarc : data.terminal_arcs) {
		graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
	}
	for (const auto& narc : data.neighbor_arcs) {
		graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap);
	}
	Duration build_dur = now() - build_begin;

	// Solve graph.
	auto solve_begin = now();
	auto flow = graph.maxflow();
	Duration solve_dur = now() - solve_begin;

	return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double> bench_mbk(BenchConfig config, const Data& data)
{
    // Build graph.
    auto build_begin = now();
    reimpls::Graph<Cap, Term, Flow, Index, Index> graph(data.num_nodes, data.neighbor_arcs.size());
    graph.add_node(data.num_nodes);
    for (const auto& tarc : data.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap, false);
    }
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    auto flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double> bench_mbk2(BenchConfig config, const Data& data)
{
    // Build graph.
    auto build_begin = now();
    reimpls::Graph2<Cap, Term, Flow, Index, Index> graph(data.num_nodes, data.neighbor_arcs.size());
    graph.add_node(data.num_nodes);
    for (const auto& tarc : data.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap, false);
    }
    graph.init_maxflow();
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    auto flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double> bench_eibfs(BenchConfig config, const Data& data)
{
    using Ibfs = reimpls::IBFSGraph<Cap, Term, Flow, uint32_t, Index>;

    // Build graph.
    auto build_begin = now();
    Ibfs graph(data.num_nodes, data.neighbor_arcs.size());
    for (const auto& tarc : data.terminal_arcs) {
        graph.addNode(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.addEdge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    graph.initGraph();
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    auto flow = graph.computeMaxFlow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double> bench_eibfs2(BenchConfig config, const Data& data)
{
    using Ibfs = reimpls::IBFSGraph2<Cap, Term, Flow>;

    // Build graph.
    auto build_begin = now();
    Ibfs graph(data.num_nodes, data.neighbor_arcs.size());
    for (const auto& tarc : data.terminal_arcs) {
        graph.addNode(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.addEdge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    graph.initGraph();
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    auto flow = graph.computeMaxFlow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double> bench_eibfs_old(BenchConfig config, const Data& data)
{
    using Ibfs = ibfs::IBFSGraph<Cap, Term, Flow>;

    // Build graph.
    auto build_begin = now();
    Ibfs graph(Ibfs::IB_INIT_FAST);
    graph.initSize(data.num_nodes, data.neighbor_arcs.size());
    for (const auto& tarc : data.terminal_arcs) {
        graph.addNode(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.addEdge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    graph.initGraph();
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    auto flow = graph.computeMaxFlow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data, reimpls::LabelOrder LO, reimpls::RootOrder RO>
std::tuple<Flow, double, double> bench_hpf(BenchConfig config, const Data& data)
{
    // Build graph.
    auto build_begin = now();
    // reimpls::Hpf<Cap, mbk::LabelOrder::HIGHEST_FIRST, reimpls::RootOrder::FIFO> graph(
    reimpls::Hpf<Cap, LO, RO> graph(
        data.num_nodes + 2, data.neighbor_arcs.size() + data.terminal_arcs.size());;
    graph.set_source(0);
    graph.set_sink(1);
    graph.add_node(data.num_nodes + 2);
    for (const auto& tarc : data.terminal_arcs) {
        graph.add_edge(0, tarc.node + 2, tarc.source_cap);
        graph.add_edge(tarc.node + 2, 1, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        if (narc.cap) {
            graph.add_edge(narc.i + 2, narc.j + 2, narc.cap);
        }
        if (narc.rev_cap) {
            graph.add_edge(narc.j + 2, narc.i + 2, narc.rev_cap);
        }
    }
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    graph.mincut();
    Duration solve_dur = now() - solve_begin;

    auto flow = graph.compute_maxflow();
    return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double> bench_hi_pr(BenchConfig config, const Data& data)
{
    hi_pr::HiPr graph;

    // Reorder data to comply with HI_PR implementation
    std::vector<int> endpoints; // [from, to, from, to, ...]
    std::vector<int> capacities; // [cap, rev_cap, cap, rev_cap, ...]
    std::vector<int> excesses; // source_cap - sink_cap for all nodes
    endpoints.reserve(2 * data.neighbor_arcs.size());
    capacities.reserve(2 * data.neighbor_arcs.size());
    excesses.resize(data.num_nodes, 0);

    for (const auto& narc : data.neighbor_arcs) {
        endpoints.push_back(narc.i);
        endpoints.push_back(narc.j);
        capacities.push_back(narc.cap);
        capacities.push_back(narc.rev_cap);
    }
    for (const auto& tarc : data.terminal_arcs) {
        excesses[tarc.node] = tarc.source_cap - tarc.sink_cap;
    }

    // Build graph.
    auto build_begin = now();
    graph.construct(data.num_nodes, data.neighbor_arcs.size(),
        endpoints.data(), capacities.data(), excesses.data());
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    graph.stageOne();
    Duration solve_dur = now() - solve_begin;

    auto flow = graph.flow - graph.flow0;
    return std::make_tuple(flow, build_dur.count(), solve_dur.count());
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double, uint16_t> bench_parallel_mbk(
    BenchConfig config, const Data& data, std::vector<uint16_t> node_blocks, uint16_t num_blocks)
{
    auto block_intervals = split_block_intervals(node_blocks);

    // Build graph.
    auto build_begin = now();
    reimpls::ParallelGraph<Cap, Term, Flow> graph(data.num_nodes, data.neighbor_arcs.size(), num_blocks);
    graph.set_num_threads(config.num_threads);

    Index added_nodes = 0;
    for (const auto& itv : block_intervals) {
        // itv = { interval_length, block_index }
        graph.add_node(itv.first, itv.second);
        added_nodes += itv.first;
    }
    if (added_nodes < data.num_nodes) {
        // Data was likely a .bq file so need to repeat blocks   
        for (const auto& itv : block_intervals) {
            // itv = { interval_length, block_index }
            graph.add_node(itv.first, itv.second);
        }
    }

    for (const auto& tarc : data.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap, false);
    }
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    auto flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count(), num_blocks);
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double, uint16_t> bench_parallel_pr(
    BenchConfig config, const Data& data, std::vector<uint16_t> node_blocks, uint16_t num_blocks)
{
    if (!std::is_same<Cap, int32_t>::value) {
        throw std::runtime_error("Only int32_t cap is supported for ppr");
    }
    /*auto build_begin = now();   
    reimpls::ParallelPushRelabel<int32_t, int64_t, Index, Index> graph(
        data.num_nodes + 2, data.neighbor_arcs.size() + data.terminal_arcs.size());
    graph.set_source(0);
    graph.set_sink(1);    

    for (const auto& tarc : data.terminal_arcs) {
        graph.add_edge(0, tarc.node + 2, tarc.source_cap, 0);
        graph.add_edge(tarc.node + 2, 1, tarc.sink_cap, 0);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.add_edge(narc.i + 2, narc.j + 2, narc.cap, narc.rev_cap);
    }
    Duration build_dur = now() - build_begin;

    graph.set_num_threads(config.num_threads);

    auto solve_begin = now();
    graph.mincut();
    Duration solve_dur = now() - solve_begin;
    
    return std::make_tuple(graph.get_flow(), build_dur.count(), solve_dur.count(), data.num_nodes);*/

    auto build_begin = now();

    size_t num_nodes = data.num_nodes + 2;
    size_t num_arcs = 2 * (data.terminal_arcs.size() + data.neighbor_arcs.size());
    std::vector<wghVertex<int>> verts(num_arcs);
    std::vector<int> neighbors(num_arcs, -1);
    std::vector<int> arc_weights(num_arcs, 0);

    int source = 0;
    int sink = 1;

    wghGraph<int> wg(verts.data(), num_nodes, num_arcs);

    size_t total_arcs = 0;
    for (const auto& tarc : data.terminal_arcs) {
        if (tarc.source_cap > tarc.sink_cap) {
            wg.V[source].degree++;
        } else {
            wg.V[sink].degree++;
        }
        wg.V[tarc.node].degree++;
    }
    for (const auto& narc : data.neighbor_arcs) {
        wg.V[narc.i + 2].degree++;
        wg.V[narc.j + 2].degree++;
    }

    std::vector<int> offsets(num_nodes + 2, 0);
    for (size_t i = 1; i < num_nodes; ++i) {
        offsets[i] = offsets[i - 1] + wg.V[i - 1].degree;
    }

    int64_t flow = 0;
    for (const auto& tarc : data.terminal_arcs) {
        if (tarc.source_cap > tarc.sink_cap) {
            neighbors[offsets[source]] = tarc.node + 2;
            neighbors[offsets[tarc.node + 2]] = source;

            arc_weights[offsets[source]] = tarc.source_cap - tarc.sink_cap;
            arc_weights[offsets[tarc.node + 2]] = 0;

            flow += tarc.sink_cap;
            offsets[source]++;
        } else {
            neighbors[offsets[sink]] = tarc.node + 2;
            neighbors[offsets[tarc.node + 2]] = sink;

            arc_weights[offsets[sink]] = 0;
            arc_weights[offsets[tarc.node + 2]] = tarc.sink_cap - tarc.source_cap;

            flow += tarc.source_cap;
            offsets[sink]++;
        }
        offsets[tarc.node + 2]++;
    }
    for (const auto& narc : data.neighbor_arcs) {
        neighbors[offsets[narc.i + 2]] = narc.j + 2;
        neighbors[offsets[narc.j + 2]] = narc.i + 2;

        arc_weights[offsets[narc.i + 2]] = narc.cap;
        arc_weights[offsets[narc.j + 2]] = narc.rev_cap;
        
        offsets[narc.i + 2]++;
        offsets[narc.j + 2]++;
    }

    wg.V[0].Neighbors = neighbors.data();
    wg.V[0].nghWeights = arc_weights.data();
    for (size_t i = 1; i < num_nodes; ++i) {
        wg.V[i].Neighbors = wg.V[i - 1].Neighbors + wg.V[i - 1].degree;
        wg.V[i].nghWeights = wg.V[i - 1].nghWeights + wg.V[i - 1].degree;
    }

    FlowGraph<int> g(wg, source, sink);
    setWorkers(config.num_threads);
    prepareMaxFlow(g);

    Duration build_dur = now() - build_begin;

    auto solve_begin = now();
    flow += maxFlow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count(), data.num_nodes); 
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double, uint16_t> bench_parallel_sk(
    BenchConfig config, const Data& data, std::vector<uint16_t> node_blocks, uint16_t num_blocks)
{
    if (node_blocks.size() < data.num_nodes) {
        // Data was likely as .bq file so need to repeat blocks
        auto old_size = node_blocks.size();
        node_blocks.resize(2 * old_size);
        std::copy_n(node_blocks.begin(), old_size, node_blocks.begin() + old_size);
    }

    unsigned int num_threads = config.num_threads;
    uint16_t blocks_per_thread = num_blocks / num_threads;
    if (blocks_per_thread == 0) {
        // Have more threads than blocks, just use one thread per block
        blocks_per_thread = 1;
        num_threads = num_blocks;
    }

    // Update node_blocks so blocks have their correct block indices
    for (auto& block : node_blocks) {
        block = std::min<uint16_t>(block / blocks_per_thread, num_threads - 1);
    }
   
    // Find nodes we need to make shared across multiple blocks
    std::vector<robin_hood::unordered_set<uint64_t>> extra_block_nodes(num_threads);
    for (const auto& narc : data.neighbor_arcs) {
        uint16_t bi = node_blocks[narc.i];
        uint16_t bj = node_blocks[narc.j];
        if (bi != bj && (narc.cap != 0 || narc.rev_cap != 0)) {
            //if (bi < bj) {
                extra_block_nodes[bi].insert(narc.j);
            //} else {
                extra_block_nodes[bj].insert(narc.i);
            //}
        }
    }

    auto block_intervals = split_block_intervals(node_blocks);

    auto build_begin = now();
    size_t edges_per_block = data.neighbor_arcs.size() / config.num_threads;
    reimpls::ParallelSkGraph<Cap, Term, Flow, typename std::make_signed<Index>::type> graph(
        data.num_nodes, edges_per_block + edges_per_block / 5);
    graph.add_node(data.num_nodes);

    Index added_nodes = 0;
    for (const auto& itv : block_intervals) {
        // itv = { interval_length, block_index }
        graph.add_nodes_to_block(added_nodes, added_nodes + itv.first, itv.second);
        added_nodes += itv.first;
    }
    for (uint16_t block = 0; block < num_threads; ++block) {
        for (uint64_t i : extra_block_nodes[block]) {
            graph.add_nodes_to_block(i, i+1, block);
        }
    }

    for (const auto& tarc : data.terminal_arcs) {
        if (tarc.source_cap != 0 || tarc.sink_cap != 0) {
            graph.add_tweights(tarc.node, tarc.source_cap * 2, tarc.sink_cap * 2);
        }
    }
    for (const auto& narc : data.neighbor_arcs) {
        if (narc.cap != 0 || narc.rev_cap != 0) {
            graph.add_edge(narc.i, narc.j, narc.cap * 2, narc.rev_cap * 2);
        }
    }
   
    Duration build_dur = now() - build_begin;

    auto solve_begin = now();
    auto flow = graph.maxflow() / 2;
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count(), config.num_threads);
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double, uint16_t> bench_parallel_rd(
    BenchConfig config, const Data& data, std::vector<uint16_t> node_blocks, uint16_t num_blocks)
{
    constexpr int source = 0;
    constexpr int sink = 1;
    const char *splitter_file = "pard_tmp_splitter";
    const int d = 1;
    const int size[] = { data.num_nodes };

    // Remove previous output directory
    // TODO: Use proper C++ function so this works across on Windows too.
    std::string rm_command = "rm -rf ";
    rm_command += splitter_file;
    rm_command += "_reg";
    system(rm_command.c_str());
    
    if (node_blocks.size() < data.num_nodes) {
        // Data was likely as .bq file so need to repeat blocks
        auto old_size = node_blocks.size();
        node_blocks.resize(2 * old_size);
        std::copy_n(node_blocks.begin(), old_size, node_blocks.begin() + old_size);
    }

    unsigned int num_threads = config.num_threads;
    uint16_t blocks_per_thread = num_blocks / num_threads;
    if (blocks_per_thread == 0) {
        // Have more threads than blocks, just use one thread per block
        blocks_per_thread = 1;
        num_threads = num_blocks;
    }

    // Update node_blocks so blocks have their correct block indices
    for (auto& block : node_blocks) {
        block = std::min<uint16_t>(block / blocks_per_thread, num_threads - 1);
    }
    
    auto build_begin = now();
    
    region_graph G;
    G.pth = std::string(splitter_file) + "_reg";
    region_splitter2 splitter(splitter_file, &G, num_threads, node_blocks);

    parallel_ARD1 pard;
    pard.params.n_threads = config.num_threads;
    pard.construct(&G);
    splitter.allocate1(data.num_nodes + 2, 0, source, sink, d, size);

    // Add edges
    for (int loop = 0; loop < 2; ++loop) {
        for (const auto& tarc : data.terminal_arcs) {
            splitter.read_arc(loop, source, tarc.node + 2, tarc.source_cap, 0);
            splitter.read_arc(loop, tarc.node + 2, sink, tarc.sink_cap, 0);
        }
        for (const auto& narc : data.neighbor_arcs) {
            splitter.read_arc(loop, narc.i + 2, narc.j + 2, narc.cap, narc.rev_cap);
        }
        splitter.allocate2(loop);
    }
    splitter.allocate3();

    Duration build_dur = now() - build_begin;
    
    //auto solve_begin = now();
    int flow = pard.maxflow();
    //Duration solve_dur = now() - solve_begin;
    
    // Remove output directory since we're done
    system(rm_command.c_str());

    // To avoid including the time to read in the problem from disk in the solve time, we rely on
    // the internal timer of the implementation here. For the build time, our external timer seems
    // to give a more fair assessment (although it's longer than needed since it does include some
    // disk I/O).
    return std::make_tuple(flow, build_dur.count(), pard.info.solve_t.time(), config.num_threads);
}

template <class Cap, class Term, class Flow, class Index, class Data>
std::tuple<Flow, double, double, uint16_t> bench_parallel_eibfs(
    BenchConfig config, const Data& data, std::vector<uint16_t> node_blocks, uint16_t num_blocks)
{
    using Ibfs = reimpls::ParallelIbfs<Cap, Term, Flow>;
    auto block_intervals = split_block_intervals(node_blocks);

    // Build graph.
    auto build_begin = now();
    Ibfs graph(data.num_nodes, data.neighbor_arcs.size());
    graph.setNumThreads(config.num_threads);

    Index added_nodes = 0;
    for (const auto& itv : block_intervals) {
        // itv = { interval_length, block_index }
        graph.registerNodes(added_nodes, added_nodes + itv.first, itv.second);
        added_nodes += itv.first;
    }
    if (added_nodes < data.num_nodes) {
        // Data was likely a .bq file so need to repeat blocks   
        for (const auto& itv : block_intervals) {
            // itv = { interval_length, block_index }
            graph.registerNodes(added_nodes, added_nodes + itv.first, itv.second);
        }
    }

    for (const auto& tarc : data.terminal_arcs) {
        graph.addNode(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : data.neighbor_arcs) {
        graph.addEdge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    graph.initGraph();
    Duration build_dur = now() - build_begin;

    // Solve graph.
    auto solve_begin = now();
    auto flow = graph.computeMaxFlow();
    Duration solve_dur = now() - solve_begin;

    return std::make_tuple(flow, build_dur.count(), solve_dur.count(), num_blocks);
}


void print_config_header()
{
    std::cout << "bench_name,";
    std::cout << "file_name,";
    std::cout << "num_nodes,";
    std::cout << "num_term_arcs,";
    std::cout << "num_nbor_arcs,";
    std::cout << "cap_type,";
    std::cout << "term_type,";
    std::cout << "flow_type,";
    std::cout << "index_type,";
    std::cout << "algorithm,";
    std::cout << "number_of_runs,";
    std::cout << "num_threads,";
    std::cout << "num_blocks,";
    std::cout << "build_time,";
    std::cout << "solve_time,";
    std::cout << "maxflow" << std::endl;
}

void print_data_config_values(DataConfig config)
{
    std::cout << config.bench_name << ",";
    std::cout << config.file_name << ",";
    std::cout << std::flush;
}

template <class Data>
void print_data_sizes(const Data& data)
{
    std::cout << data.num_nodes << ",";
    std::cout << data.terminal_arcs.size() << ",";
    std::cout << data.neighbor_arcs.size() << ",";
    std::cout << std::flush;
}

template <class Cap, class Term, class Flow, class Index>
void print_bench_config_values(BenchConfig config)
{
    std::cout << typeid(Cap).name() << ",";
    std::cout << typeid(Term).name() << ",";
    std::cout << typeid(Flow).name() << ",";
    std::cout << typeid(Index).name() << ",";
    std::cout << algo_to_string(config.algo) << ",";
    std::cout << config.num_run << ",";
    std::cout << config.num_threads << ",";
    std::cout << std::flush;
}

template <class Cap, class Term, class Flow>
void print_results(double build_time, double solve_time, Flow maxflow)
{
    std::cout << build_time << ",";
    std::cout << solve_time << ",";
    std::cout << maxflow << std::endl;
}

template <class Cap, class Term, class Flow, class Index, class Data>
void bench_data(DataConfig data_config, BenchConfig bench_config, const Data& data)
{
    Flow flow;
    double build_time, solve_time;

    uint16_t num_blocks = 1;
    std::vector<uint16_t> node_blocks;
    if (algo_is_parallel(bench_config.algo)) {
        // Algorithms is parallel so try to load a block file
        std::tie(node_blocks, num_blocks) = read_blocks(data_config.file_name + ".blk");
    }

    for (size_t i = 0; i < bench_config.num_run; i++) {
        print_data_config_values(data_config);
        print_data_sizes(data);               
        print_bench_config_values<Cap, Term, Flow, Index>(bench_config);

        uint16_t used_blocks;
        switch (bench_config.algo) {
        // Serial algorithms
        case ALGO_BK:
            std::tie(flow, build_time, solve_time) = 
                bench_bk<Cap, Term, Flow, Index, Data>(bench_config, data);
            break;
	case ALGO_NBK:
	    std::tie(flow, build_time, solve_time) = bench_nbk<Cap, Term, Flow, Index, Data>(bench_config, data);
	    break;
        case ALGO_MBK:
            std::tie(flow, build_time, solve_time) = bench_mbk<Cap, Term, Flow, Index, Data>(bench_config, data);
            break;
        case ALGO_MBK2:
            std::tie(flow, build_time, solve_time) = bench_mbk2<Cap, Term, Flow, Index, Data>(bench_config, data);
            break;
        case ALGO_EIBFS:
            std::tie(flow, build_time, solve_time) = bench_eibfs<Cap, Term, Flow, Index, Data>(bench_config, data);
            break;
        case ALGO_EIBFS2:
            std::tie(flow, build_time, solve_time) = bench_eibfs2<Cap, Term, Flow, Index, Data>(bench_config, data);
            break;
        case ALGO_EIBFS_OLD:
            std::tie(flow, build_time, solve_time) = bench_eibfs_old<Cap, Term, Flow, Index, Data>(bench_config, data);
            break;
        case ALGO_HPF: // Fall through to default HPF config
        case ALGO_HPF_HF:
            std::tie(flow, build_time, solve_time) = bench_hpf<Cap, Term, Flow, Index, Data, reimpls::LabelOrder::HIGHEST_FIRST, reimpls::RootOrder::FIFO>(bench_config, data);
            break;
        case ALGO_HPF_HL:
            std::tie(flow, build_time, solve_time) = bench_hpf<Cap, Term, Flow, Index, Data, reimpls::LabelOrder::HIGHEST_FIRST, reimpls::RootOrder::LIFO>(bench_config, data);
            break;
        case ALGO_HPF_LF:
            std::tie(flow, build_time, solve_time) = bench_hpf<Cap, Term, Flow, Index, Data, reimpls::LabelOrder::LOWEST_FIRST, reimpls::RootOrder::FIFO>(bench_config, data);
            break;
        case ALGO_HPF_LL:
            std::tie(flow, build_time, solve_time) = bench_hpf<Cap, Term, Flow, Index, Data, reimpls::LabelOrder::LOWEST_FIRST, reimpls::RootOrder::LIFO>(bench_config, data);
            break;
        case ALGO_HI_PR:
            std::tie(flow, build_time, solve_time) = bench_hi_pr<Cap, Term, Flow, Index, Data>(bench_config, data);
            break;
        // Parallel algorithms
        case ALGO_PMBK:
            std::tie(flow, build_time, solve_time, used_blocks) = bench_parallel_mbk<Cap, Term, Flow, Index, Data>(bench_config, data, node_blocks, num_blocks);
            break;
        case ALGO_PPR:
            std::tie(flow, build_time, solve_time, used_blocks) = bench_parallel_pr<Cap, Term, Flow, Index, Data>(bench_config, data, node_blocks, num_blocks);
            break;
        case ALGO_PSK:
            std::tie(flow, build_time, solve_time, used_blocks) = bench_parallel_sk<Cap, Term, Flow, Index, Data>(bench_config, data, node_blocks, num_blocks);
            break;
        case ALGO_PARD:
            std::tie(flow, build_time, solve_time, used_blocks) = bench_parallel_rd<Cap, Term, Flow, Index, Data>(bench_config, data, node_blocks, num_blocks);
            break;
        case ALGO_PEIBFS:
            std::tie(flow, build_time, solve_time, used_blocks) = bench_parallel_eibfs<Cap, Term, Flow, Index, Data>(bench_config, data, node_blocks, num_blocks);
            break;
        // Dummy and default
        case ALGO_DUMMY:
            flow = 0;
            build_time = 0;
            solve_time = 0;
            break;
        default:
            throw std::runtime_error("Unsupported algorithm.");
        }

        std::cout << used_blocks << "," << std::flush;
	print_results<Cap, Term, Flow>(build_time, solve_time, flow);
    }
}

template <class DataCap, class DataTerm>
void bench(DataConfig config, std::vector<BenchConfig> bench_configs)
{
    BkGraph<DataCap, DataTerm> data;
    if (config.file_type == FTYPE_DIMACS) {
        data = read_dimacs_to_bk<DataCap, DataTerm>(config.file_name);
    } else if (config.file_type == FTYPE_BBK) {
        data = read_bbk_to_bk<DataCap, DataTerm>(config.file_name);
    } else if (config.file_type == FTYPE_BQ) {
        data = qpbo_to_graph(read_bq_to_qpbo<DataCap>(config.file_name));
    }

    std::cerr << "Benching " << config.file_name << std::endl;
    for (const auto& bc : bench_configs) {
        std::cerr << "... " << algo_to_string(bc.algo) << std::endl;
        RUN_BENCH_FUNC(config, bc, data, bench_data);
    }
}

int main(int argc, const char* argv[])
{
    std::string fname;
    if (argc < 2) {
        /*std::cout << "usage: demo <config>\n";
        return -1;*/
        fname = "C:/Users/patmjen/Documents/HCP Anywhere/projects/parallel-qpbo/bench_config.json";
    } else {
        fname = argv[1];
    }

    json config;
    try {
        std::fstream file(fname);
        file >> config;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<BenchConfig> bench_configs = gen_bench_configs(config);
    std::vector<DataConfig> data_configs = gen_data_configs(config);

	print_config_header();

    try {
        for (const auto& dc : data_configs) {
            if (dc.nbor_cap_type != TYPE_INT32 || dc.term_cap_type != TYPE_INT32) {
                throw std::runtime_error("Only int32 weights are supported for data files.");
            }
            bench<int, int>(dc, bench_configs);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

TypeCode code_from_string(const std::string& str)
{
    if (str == "uint8") return TYPE_UINT8;
    if (str == "int8") return TYPE_INT8;
    if (str == "uint16") return TYPE_UINT16;
    if (str == "int16") return TYPE_INT16;
    if (str == "uint32") return TYPE_UINT32;
    if (str == "int32") return TYPE_INT32;
    if (str == "uint64") return TYPE_UINT64;
    if (str == "int64") return TYPE_INT64;
    if (str == "float") return TYPE_FLOAT;
    if (str == "double") return TYPE_DOUBLE;
    throw std::invalid_argument("Invalid type.");
}

const char* algo_to_string(Algorithm algo)
{
    switch (algo) {
    case ALGO_BK:
        return "bk";
    case ALGO_NBK:
        return "nbk";
    case ALGO_MBK:
        return "mbk";
    case ALGO_MBK2:
        return "mbk2";
    case ALGO_EIBFS:
        return "eibfs";
    case ALGO_EIBFS2:
        return "eibfs2";
    case ALGO_EIBFS_OLD:
        return "eibfs_old";
    case ALGO_HPF:
        return "hpf";
    case ALGO_HPF_HF:
        return "hpf_hf";
    case ALGO_HPF_HL:
        return "hpf_hl";
    case ALGO_HPF_LF:
        return "hpf_lf";
    case ALGO_HPF_LL:
        return "hpf_ll";
    case ALGO_HI_PR:
        return "hi_pr";
    case ALGO_PMBK:
        return "pmbk";
    case ALGO_PPR:
        return "ppr";
    case ALGO_PSK:
        return "psk";
    case ALGO_PARD:
        return "pard";
    case ALGO_PEIBFS:
        return "peibfs";
    case ALGO_DUMMY:
        return "dummy";
    default:
        throw std::invalid_argument("Invalid algorithm.");
    }
}

Algorithm algo_from_string(const std::string& str)
{
    if (str == algo_to_string(ALGO_BK)) return ALGO_BK;
    if (str == algo_to_string(ALGO_NBK)) return ALGO_NBK;
    if (str == algo_to_string(ALGO_MBK)) return ALGO_MBK;
    if (str == algo_to_string(ALGO_MBK2)) return ALGO_MBK2;
    if (str == algo_to_string(ALGO_EIBFS)) return ALGO_EIBFS;
    if (str == algo_to_string(ALGO_EIBFS2)) return ALGO_EIBFS2;
    if (str == algo_to_string(ALGO_EIBFS_OLD)) return ALGO_EIBFS_OLD;
    if (str == algo_to_string(ALGO_HPF)) return ALGO_HPF;
    if (str == algo_to_string(ALGO_HPF_HF)) return ALGO_HPF_HF;
    if (str == algo_to_string(ALGO_HPF_HL)) return ALGO_HPF_HL;
    if (str == algo_to_string(ALGO_HPF_LF)) return ALGO_HPF_LF;
    if (str == algo_to_string(ALGO_HPF_LL)) return ALGO_HPF_LL;
    if (str == algo_to_string(ALGO_HI_PR)) return ALGO_HI_PR;
    if (str == algo_to_string(ALGO_PMBK)) return ALGO_PMBK;
    if (str == algo_to_string(ALGO_PPR)) return ALGO_PPR;
    if (str == algo_to_string(ALGO_PSK)) return ALGO_PSK;
    if (str == algo_to_string(ALGO_PARD)) return ALGO_PARD;
    if (str == algo_to_string(ALGO_PEIBFS)) return ALGO_PEIBFS;
    if (str == algo_to_string(ALGO_DUMMY)) return ALGO_DUMMY;
    throw std::invalid_argument("Invalid algorithm.");
}

FileType ftype_from_string(const std::string& str)
{
    if (str == "dimacs") return FTYPE_DIMACS;
    if (str == "bbk") return FTYPE_BBK;
    if (str == "bq") return FTYPE_BQ;
    throw std::invalid_argument("Invalid file type.");
}

bool algo_is_parallel(Algorithm algo)
{
    return algo == ALGO_PMBK || algo == ALGO_PPR || algo == ALGO_PSK || algo == ALGO_PARD || algo == ALGO_PEIBFS;
}

std::vector<BenchConfig> gen_bench_configs(json config)
{
    std::vector<BenchConfig> out;
    for (auto& type_config : config["types"]) {
        for (auto& algo : config["algorithms"]) {
            auto algorithm = algo_from_string(algo);
            if (algo_is_parallel(algorithm) && config.contains("parallel")) {
                auto& parallel = config["parallel"];
                for (auto& threads : parallel["threads"]) {
                    out.push_back({
                        config["name"],
                        code_from_string(type_config["cap"]),
                        code_from_string(type_config["term"]),
                        code_from_string(type_config["flow"]),
                        code_from_string(type_config["index"]),
                        algorithm,
                        config["num_run"],
                        threads.get<int>()
                    });
                }
            } else {
                out.push_back({
                    config["name"],
                    code_from_string(type_config["cap"]),
                    code_from_string(type_config["term"]),
                    code_from_string(type_config["flow"]),
                    code_from_string(type_config["index"]),
                    algorithm,
                    config["num_run"],
                    1
                });
            }
        }
    }
    return out;
}

std::vector<DataConfig> gen_data_configs(json config)
{
    std::vector<DataConfig> out;
    for (auto& data : config["data_sets"]) {
        out.push_back({
            config["name"],
            code_from_string(data["nbor_cap_type"]),
            code_from_string(data["term_cap_type"]),
            data["file_name"],
            ftype_from_string(data["file_type"])
        });
    }
    return out;
}

#include <iostream>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <inttypes.h>
#include <tuple>
#include <vector>

#include "graph_io.h"

#include "bk/graph.h"
#include "reimpls/parallel_graph.h"
#include "reimpls/graph.h"
#include "reimpls/graph2.h"
#include "reimpls/ibfs.h"
#include "reimpls/ibfs2.h"
#include "reimpls/parallel_ibfs.h"
#include "ibfs/ibfs.h"
#include "reimpls/hpf.h"
#include "hi_pr/hi_pr.h"
#include "reimpls/parallel_sk.h"
#ifdef GRIDCUT_IS_AVAILABLE
#include "grid_cut/GridGraph_3D_6C.h"
#include "grid_cut/GridGraph_3D_6C_MT.h"
#endif

using Duration = std::chrono::duration<double>;
static const auto now = std::chrono::steady_clock::now;

struct coord {
    int x;
    int y;
    int z;

    coord() = default;
    coord(int x, int y, int z) : x(x), y(y), z(z) {}
};

coord id2coord(int i, int width, int height)
{
    return coord(
        (i % (width * height)) % width,
        (i % (width * height)) / width,
        i / (width * height)
    );
};

template <class capty, class tcapty>
void bench_bk(const BkGraph<capty, tcapty>& bkg)
{
    std::cout << "building... ";
    auto build_begin = now();
    bk::Graph<int, int, int> graph(bkg.num_nodes, bkg.neighbor_arcs.size());
    graph.add_node(bkg.num_nodes);
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "solving... ";
    auto solve_begin = now();
    auto flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + solve_dur).count() << " seconds, maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_mbk(const BkGraph<capty, tcapty>& bkg)
{
    std::cout << "building... ";
    auto build_begin = now();
    reimpls::Graph<int, int, int, uint32_t, uint32_t> graph(bkg.num_nodes, bkg.neighbor_arcs.size());
    graph.add_node(bkg.num_nodes);
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap, false);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "solving... ";
    auto solve_begin = now();
    auto flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + solve_dur).count() << " seconds, maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_mbk2(const BkGraph<capty, tcapty>& bkg)
{
    std::cout << "building... ";
    auto build_begin = now();
    reimpls::Graph2<int, int, int, uint32_t, uint32_t> graph(bkg.num_nodes, bkg.neighbor_arcs.size());
    graph.add_node(bkg.num_nodes);
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap, false);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "init... ";
    auto init_begin = now();
    graph.init_maxflow();
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    std::cout << "solving... ";
    auto solve_begin = now();
    auto flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + init_dur + solve_dur).count() << " seconds, maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_pmbk(const BkGraph<capty, tcapty>& bkg, const std::string& fname)
{
    uint16_t num_blocks;
    std::vector<uint16_t> node_blocks;
    std::tie(node_blocks, num_blocks) = read_blocks(fname + ".blk");
    auto block_intervals = split_block_intervals(node_blocks);
    bool was_qpbo_file = fname.back() == 'q';

    std::cout << "building... ";
    auto build_begin = now();
    reimpls::ParallelGraph<int, int, int, uint32_t, uint32_t> graph(bkg.num_nodes, bkg.neighbor_arcs.size(), num_blocks);

    for (const auto& itv : block_intervals) {
        graph.add_node(itv.first, itv.second);
    }
    if (was_qpbo_file) {
        for (const auto& itv : block_intervals) {
            graph.add_node(itv.first, itv.second);
        }
    }

    /*for (size_t i = 0; i < num_blocks; i++) {
        if (i == num_blocks - 1) {
            graph.add_node(bkg.num_nodes / num_blocks + bkg.num_nodes % num_blocks, i);
        } else {
            graph.add_node(bkg.num_nodes / num_blocks, i);
        }
    }*/
    /*uint16_t num_blocks;
    std::vector<std::pair<size_t, uint16_t>> block_intervals;
    std::tie(block_intervals, num_blocks) = grid_block_intervals(256, 256, 119, 256, 256, 8);
    for (auto interval : block_intervals) {
        graph.add_node(interval.first, interval.second);
    }*/

    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap, false);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "solving (threads: " << graph.get_num_threads() << ")... ";
    auto solve_begin = now();
    auto flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;
    std::cout << "solved in " << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + solve_dur).count() << " seconds, maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_ibfs(const BkGraph<capty, tcapty>& bkg)
{
    using Ibfs = reimpls::IBFSGraph<int, int, int, uint32_t, uint64_t>;
    std::cout << "building...";
    auto build_begin = now();
    Ibfs graph(bkg.num_nodes, bkg.neighbor_arcs.size());
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.addNode(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.addEdge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "init... ";
    auto init_begin = now();
    graph.initGraph();
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    std::cout << "solving... ";
    auto solve_begin = now();
    auto flow = graph.computeMaxFlow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + init_dur + solve_dur).count();
    std::cout << " seconds, maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_ibfs2(const BkGraph<capty, tcapty>& bkg)
{
    using Ibfs = reimpls::IBFSGraph2<int, int, int>;
    std::cout << "building...";
    auto build_begin = now();
    Ibfs graph(bkg.num_nodes, bkg.neighbor_arcs.size());
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.addNode(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.addEdge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "init... ";
    auto init_begin = now();
    graph.initGraph();
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    std::cout << "solving... ";
    auto solve_begin = now();
    auto flow = graph.computeMaxFlow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + init_dur + solve_dur).count();
    std::cout << " seconds, maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_ibfs_old(const BkGraph<capty, tcapty>& bkg)
{
    using Ibfs = ibfs::IBFSGraph<int, int, int>;
    std::cout << "building...";
    auto build_begin = now();
    Ibfs graph(Ibfs::IB_INIT_FAST);
    graph.initSize(bkg.num_nodes, bkg.neighbor_arcs.size());
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.addNode(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.addEdge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "init... ";
    auto init_begin = now();
    graph.initGraph();
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    std::cout << "solving... ";
    auto solve_begin = now();
    auto flow = graph.computeMaxFlow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + init_dur + solve_dur).count();
    std::cout << " seconds, maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_pibfs(const BkGraph<capty, tcapty>& bkg)
{
    using Ibfs = reimpls::ParallelIbfs<int, int, int>;
    std::cout << "building...";
    auto build_begin = now();
    /*int num_blocks = 8;
    size_t begin = 0;
    for (size_t i = 0; i < num_blocks; i++) {
        size_t num_nodes;
        if (i == num_blocks - 1) {
            num_nodes =  bkg.num_nodes / num_blocks + bkg.num_nodes % num_blocks;
        } else {
            num_nodes = bkg.num_nodes / num_blocks;
        }
        graph.registerNodes(begin, begin + num_nodes, i);
        begin += num_nodes;
    }*/
    uint16_t num_blocks;
    std::vector<std::pair<size_t, uint16_t>> block_intervals;
    std::tie(block_intervals, num_blocks) = grid_block_intervals(256, 256, 119, 128, 128, 64);
    Ibfs graph(bkg.num_nodes, bkg.neighbor_arcs.size());
    size_t begin = 0;
    for (auto interval : block_intervals) {
        graph.registerNodes(begin, begin + interval.first, interval.second);
        begin += interval.first;
    }

    for (const auto& tarc : bkg.terminal_arcs) {
        graph.addNode(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.addEdge(narc.i, narc.j, narc.cap, narc.rev_cap);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "init... ";
    auto init_begin = now();
    graph.initGraph();
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    std::cout << "solving (blocks: " << num_blocks << ")... ";
    auto solve_begin = now();
    graph.setNumThreads(16);
    auto flow = graph.computeMaxFlow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + init_dur + solve_dur).count();
    std::cout << " seconds, maxflow: " << flow << std::endl;
}

/*void bench_ppr(const BkGraph<int, int>& bkg)
{
    std::cout << "init... ";
    auto init_begin = now();
    reimpls::ParallelPushRelabel<int, int> graph(
        bkg.num_nodes + 2, bkg.neighbor_arcs.size() + bkg.terminal_arcs.size());

    graph.set_source(0);
    graph.set_sink(1);
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds\n";

    std::cout << "building... ";
    auto build_begin = now();
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_edge(0, tarc.node + 2, tarc.source_cap, 0);
        graph.add_edge(tarc.node + 2, 1, tarc.sink_cap, 0);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.add_edge(narc.i + 2, narc.j + 2, narc.cap, narc.rev_cap);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds\n";

    graph.set_num_threads(16);
    std::cout << "solving... ";
    auto solve_begin = now();
    graph.mincut();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds\n";

    std::cout << "total: " << (init_dur + build_dur + solve_dur).count();
    std::cout << " seconds, flow: " << graph.get_flow() << "\n";
}*/

template <class capty>
void bench_hpf(const BkGraph<capty, capty> bkg)
{
    auto init_begin = now();
    reimpls::Hpf<int, reimpls::LabelOrder::LOWEST_FIRST, reimpls::RootOrder::FIFO> graph(
        bkg.num_nodes + 2, bkg.neighbor_arcs.size() + bkg.terminal_arcs.size());
    graph.set_source(0);
    graph.set_sink(1);

    std::cout << "building... ";
    auto build_begin = now();
    graph.add_node(bkg.num_nodes + 2);
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_edge(0, tarc.node + 2, tarc.source_cap);
        graph.add_edge(tarc.node + 2, 1, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        if (narc.cap) {
            graph.add_edge(narc.i + 2, narc.j + 2, narc.cap);
        }
        if (narc.rev_cap) {
            graph.add_edge(narc.j + 2, narc.i + 2, narc.rev_cap);
        }
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds\n";

    std::cout << "solving... ";
    auto solve_begin = now();
    graph.mincut();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds\n";

    std::cout << "total: " << (build_dur + solve_dur).count();
    std::cout << " seconds, flow: " << graph.compute_maxflow() << "\n";
}

void bench_hi_pr(const BkGraph<int, int> bkg)
{
    std::cout << "init...\n";
    hi_pr::HiPr graph;

    std::vector<int> endpoints; // [from, to, from, to, ...]
    std::vector<int> capacities; // [cap, rev_cap, cap, rev_cap, ...]
    std::vector<int> excesses; // source_cap - sink_cap for all nodes
    endpoints.reserve(2 * bkg.neighbor_arcs.size());
    capacities.reserve(2 * bkg.neighbor_arcs.size());
    excesses.resize(bkg.num_nodes, 0);

    for (const auto& narc : bkg.neighbor_arcs) {
        endpoints.push_back(narc.i);
        endpoints.push_back(narc.j);
        capacities.push_back(narc.cap);
        capacities.push_back(narc.rev_cap);
    }
    for (const auto& tarc : bkg.terminal_arcs) {
        excesses[tarc.node] = tarc.source_cap - tarc.sink_cap;
    }

    std::cout << "building... ";
    auto build_begin = now();
    graph.construct(bkg.num_nodes, bkg.neighbor_arcs.size(),
        endpoints.data(), capacities.data(), excesses.data());
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds\n";

    std::cout << "solving... ";
    auto solve_begin = now();
    graph.stageOne();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds\n";

    std::cout << "total: " << (build_dur + solve_dur).count() << " seconds, ";
    std::cout << "maxflow: " << graph.flow - graph.flow0 << "\n";
}

/*void bench_sk_old(const BkGraph<int, int> bkg)
{
    const int num_threads = 2;// std::thread::hardware_concurrency();
    std::vector<std::pair<size_t, uint16_t>> block_intervals;
    size_t h = 256, w = 256, d = 119; // bone.n6
    //size_t h = 128, w = 256, d = 119; // bone_subx
    //size_t h = 170, w = 170, d = 144; // liver
    size_t block_d = d / num_threads;

    std::tie(block_intervals, std::ignore) = grid_block_intervals(h, w, d, h, w, block_d);
    std::vector<int> split;
    int cumsum = 0;
    for (int i = 0; i < num_threads; ++i) {
        cumsum += block_intervals[i].first;
        split.push_back(cumsum);
    }

    std::cout << "init... ";
    auto init_begin = now();
    sk::ParallelGraph<int, int, int> graph(num_threads, bkg.num_nodes, bkg.neighbor_arcs.size(),
        split.data(), w*h);
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    std::cout << "building... ";
    auto build_begin = now();
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap * 2, tarc.sink_cap * 2);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap * 2, narc.rev_cap * 2);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "solving (threads: " << num_threads << ")... " << std::endl;
    auto solve_begin = now();
    graph.maxflow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (init_dur + build_dur + solve_dur).count() << " seconds, ";
    std::cout << "maxflow: " << graph.flow / 2 << ", iters: " << graph.iter << std::endl;
}*/

template <class capty, class tcapty>
void bench_sk(const BkGraph<capty, tcapty> bkg)
{
    const int num_threads = 2;// std::thread::hardware_concurrency();
    std::vector<std::pair<size_t, uint16_t>> block_intervals;
    size_t h = 256, w = 256, d = 119; // bone.n6
    //size_t h = 128, w = 256, d = 119; // bone_subx
    //size_t h = 170, w = 170, d = 144; // liver
    size_t block_d = d / num_threads;

    std::tie(block_intervals, std::ignore) = grid_block_intervals(h, w, d, h, w, block_d);

    std::cout << "init... ";
    auto init_begin = now();
    size_t edges_per_block = bkg.neighbor_arcs.size() / num_threads;
    reimpls::ParallelSkGraph<int, int, int> graph(bkg.num_nodes, edges_per_block + edges_per_block / 5);
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    std::cout << "building... ";
    auto build_begin = now();
    graph.add_node(bkg.num_nodes);

    int cumsum = 0;
    int i;
    for (i = 0; i < num_threads - 1; ++i) {
        auto interval = block_intervals[i];
        int end = std::min<int>(cumsum + interval.first + w * h, bkg.num_nodes);
        graph.add_nodes_to_block(cumsum, end, interval.second);
        cumsum += interval.first;
    }
    graph.add_nodes_to_block(0, w * h, block_intervals[i].second);
    graph.add_nodes_to_block(cumsum, bkg.num_nodes, block_intervals[i].second);
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap * 2, tarc.sink_cap * 2);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap * 2, narc.rev_cap * 2);
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "solving (threads: " << num_threads << ")... " << std::endl;
    auto solve_begin = now();
    int flow = graph.maxflow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (init_dur + build_dur + solve_dur).count() << " seconds, ";
    std::cout << "maxflow: " << flow / 2 << ", iters: " << graph.get_iter() << std::endl;
}

#ifdef GRIDCUT_IS_AVAILABLE
template <class capty, class tcapty>
void bench_grid_cut(const BkGraph<capty, tcapty> bkg)
{
    // bone.n6c10
    //const int width = 256, height = 256, depth = 119;

    // liver.n6c10
    const int width = 170, height = 170, depth = 144;

    std::cout << "init... ";
    auto init_begin = now();
    GridGraph_3D_6C<tcapty, capty, int> graph(width, height, depth);
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    // Precompute x,y,z coordinates. This make the time comparisons more fair
    std::cout << "precomputing data (not counted)... ";
    auto precomp_begin = now();
    std::vector<std::tuple<coord, tcapty, tcapty>> term_arcs; // xyz, sink, source
    term_arcs.reserve(bkg.terminal_arcs.size());
    for (const auto& tarc : bkg.terminal_arcs) {
        term_arcs.emplace_back(id2coord(tarc.node, width, height), tarc.source_cap, tarc.sink_cap);
    }

    std::vector<std::tuple<coord, coord, capty>> nbor_arcs; // xyz, offset, cap
    nbor_arcs.reserve(bkg.neighbor_arcs.size());
    for (const auto& narc : bkg.neighbor_arcs) {
        coord ci = id2coord(narc.i, width, height);
        coord cj = id2coord(narc.j, width, height);

        coord offset_i(cj.x - ci.x, cj.y - ci.y, cj.z - ci.z);
        coord offset_j(ci.x - cj.x, ci.y - cj.y, ci.z - cj.z);
        
        nbor_arcs.emplace_back(ci, offset_i, narc.cap);
        nbor_arcs.emplace_back(cj, offset_j, narc.rev_cap);
    }
    Duration precomp_dur = now() - precomp_begin;
    std::cout << precomp_dur.count() << " seconds" << std::endl;

    std::cout << "building... ";
    auto build_begin = now();
    for (const auto& tarc : term_arcs) {
        coord c;
        tcapty source_cap, sink_cap;
        std::tie(c, source_cap, sink_cap) = tarc;
        graph.set_terminal_cap(graph.node_id(c.x, c.y, c.z), source_cap, sink_cap);
    }
    for (const auto& narc : nbor_arcs) {
        coord c, offset;
        capty cap;
        std::tie(c, offset, cap) = narc;
        if (abs(offset.x) <= 1 && abs(offset.y) <= 1 && abs(offset.z) <= 1) {
            graph.set_neighbor_cap(graph.node_id(c.x, c.y, c.z), offset.x, offset.y, offset.z, cap);
        }
    }
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "solving... ";
    auto solve_begin = now();
    graph.compute_maxflow();
    auto flow = graph.get_flow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (init_dur + build_dur + solve_dur).count() << " seconds, ";
    std::cout << "maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_grid_cut_fastbuild(const BkGraph<capty, tcapty> bkg)
{
    // bone.n6c10
    //const int width = 256, height = 256, depth = 119;

    // liver.n6c10
    const int width = 170, height = 170, depth = 144;

    std::cout << "init... ";
    auto init_begin = now();
    GridGraph_3D_6C<tcapty, capty, int> graph(width, height, depth);
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    // Precompute capacity arrays for fast build.
    std::cout << "precomputing data (not counted)... ";
    auto precomp_begin = now();
    std::vector<tcapty> source_caps(width * height * depth, 0);
    std::vector<tcapty> sink_caps(width * height * depth, 0);
    for (const auto& tarc : bkg.terminal_arcs) {
        source_caps[tarc.node] += tarc.source_cap;
        sink_caps[tarc.node] += tarc.sink_cap;
    }

    std::vector<capty> lee_caps(width * height * depth, 0);
    std::vector<capty> gee_caps(width * height * depth, 0);
    std::vector<capty> ele_caps(width * height * depth, 0);
    std::vector<capty> ege_caps(width * height * depth, 0);
    std::vector<capty> eel_caps(width * height * depth, 0);
    std::vector<capty> eeg_caps(width * height * depth, 0);
    for (const auto& narc : bkg.neighbor_arcs) {
        coord ci = id2coord(narc.i, width, height);
        coord cj = id2coord(narc.j, width, height);

        coord offset_i(cj.x - ci.x, cj.y - ci.y, cj.z - ci.z);
        coord offset_j(ci.x - cj.x, ci.y - cj.y, ci.z - cj.z);
        if (offset_i.x == -1) {
            lee_caps[narc.i] += narc.cap;
        } else if (offset_i.x == 1) {
            gee_caps[narc.i] += narc.cap;
        } else if (offset_i.y == -1) {
            ele_caps[narc.i] += narc.cap;
        } else if (offset_i.y == 1) {
            ege_caps[narc.i] += narc.cap;
        } else if (offset_i.z == -1) {
            eel_caps[narc.i] += narc.cap;
        } else if (offset_i.z == 1) {
            eeg_caps[narc.i] += narc.cap;
        }

        if (offset_j.x == -1) {
            lee_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.x == 1) {
            gee_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.y == -1) {
            ele_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.y == 1) {
            ege_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.z == -1) {
            eel_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.z == 1) {
            eeg_caps[narc.j] += narc.rev_cap;
        }
    }
    Duration precomp_dur = now() - precomp_begin;
    std::cout << precomp_dur.count() << " seconds" << std::endl;

    std::cout << "building... ";
    auto build_begin = now();
    graph.set_caps(
        source_caps.data(),
        sink_caps.data(),
        lee_caps.data(),
        gee_caps.data(),
        ele_caps.data(),
        ege_caps.data(),
        eel_caps.data(),
        eeg_caps.data()
    );
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "solving... ";
    auto solve_begin = now();
    graph.compute_maxflow();
    auto flow = graph.get_flow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (init_dur + build_dur + solve_dur).count() << " seconds, ";
    std::cout << "maxflow: " << flow << std::endl;
}

template <class capty, class tcapty>
void bench_grid_cut_mt(const BkGraph<capty, tcapty> bkg)
{
    // bone.n6c10
    //const int width = 256, height = 256, depth = 119;

    // liver.n6c10
    const int width = 170, height = 170, depth = 144;

    const int block_size = 32;
    const int num_threads = std::thread::hardware_concurrency();

    std::cout << "init... ";
    auto init_begin = now();
    GridGraph_3D_6C_MT<tcapty, capty, int> graph(width, height, depth, num_threads, block_size);
    Duration init_dur = now() - init_begin;
    std::cout << init_dur.count() << " seconds" << std::endl;

    // Precompute capacity arrays for fast build.
    std::cout << "precomputing data (not counted)... ";
    auto precomp_begin = now();
    std::vector<tcapty> source_caps(width * height * depth, 0);
    std::vector<tcapty> sink_caps(width * height * depth, 0);
    for (const auto& tarc : bkg.terminal_arcs) {
        source_caps[tarc.node] += tarc.source_cap;
        sink_caps[tarc.node] += tarc.sink_cap;
    }

    std::vector<capty> lee_caps(width * height * depth, 0);
    std::vector<capty> gee_caps(width * height * depth, 0);
    std::vector<capty> ele_caps(width * height * depth, 0);
    std::vector<capty> ege_caps(width * height * depth, 0);
    std::vector<capty> eel_caps(width * height * depth, 0);
    std::vector<capty> eeg_caps(width * height * depth, 0);
    for (const auto& narc : bkg.neighbor_arcs) {
        coord ci = id2coord(narc.i, width, height);
        coord cj = id2coord(narc.j, width, height);

        coord offset_i(cj.x - ci.x, cj.y - ci.y, cj.z - ci.z);
        coord offset_j(ci.x - cj.x, ci.y - cj.y, ci.z - cj.z);
        if (offset_i.x == -1) {
            lee_caps[narc.i] += narc.cap;
        } else if (offset_i.x == 1) {
            gee_caps[narc.i] += narc.cap;
        } else if (offset_i.y == -1) {
            ele_caps[narc.i] += narc.cap;
        } else if (offset_i.y == 1) {
            ege_caps[narc.i] += narc.cap;
        } else if (offset_i.z == -1) {
            eel_caps[narc.i] += narc.cap;
        } else if (offset_i.z == 1) {
            eeg_caps[narc.i] += narc.cap;
        }

        if (offset_j.x == -1) {
            lee_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.x == 1) {
            gee_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.y == -1) {
            ele_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.y == 1) {
            ege_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.z == -1) {
            eel_caps[narc.j] += narc.rev_cap;
        } else if (offset_j.z == 1) {
            eeg_caps[narc.j] += narc.rev_cap;
        }
    }
    Duration precomp_dur = now() - precomp_begin;
    std::cout << precomp_dur.count() << " seconds" << std::endl;

    std::cout << "building... ";
    auto build_begin = now();
    graph.set_caps(
        source_caps.data(),
        sink_caps.data(),
        lee_caps.data(),
        gee_caps.data(),
        ele_caps.data(),
        ege_caps.data(),
        eel_caps.data(),
        eeg_caps.data()
    );
    Duration build_dur = now() - build_begin;
    std::cout << build_dur.count() << " seconds" << std::endl;

    std::cout << "solving (" << num_threads << " threads, " << block_size << "^3 blocks)... ";
    auto solve_begin = now();
    graph.compute_maxflow();
    auto flow = graph.get_flow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (init_dur + build_dur + solve_dur).count() << " seconds, ";
    std::cout << "maxflow: " << flow << std::endl;
}
#endif

template <class capty, class tcapty>
BkGraph<capty, tcapty> read_graph(const std::string& fname)
{
    static_assert(std::is_convertible<capty, tcapty>::value, "Must be able to convert capty to tcapty");
    auto bk_reader = [](const std::string& fname) { return read_bbk_to_bk<capty, tcapty>(fname); };
    auto bq_reader = [](const std::string& fname)
    {
        return qpbo_to_graph(read_bq_to_qpbo<capty>(fname));
    };

    bool likely_bbk = (fname.back() != 'q'); // QPBO files usually end in .bq

    try {
        return likely_bbk ? bk_reader(fname) : bq_reader(fname);
    } catch (const std::runtime_error&) {
        // Primary reader didn't work, so let's try the other one
        return likely_bbk ? bq_reader(fname) : bk_reader(fname);
    }
}

int main(int argc, const char* argv[])
{
    std::string fname;
    if (argc < 2) {
        /*std::cout << "ERROR: must provide problem instance file\n";
        std::cout << "Usage: demo <file> [<algo>...]\n";
        std::cout << "  Benchmark FILE with ALGOs. ALGO must be one of:\n";
        std::cout << "    bk mbk pmbk eilbfs_old eibfs eibfs2 peibfs ppr hpf hi_pr sk\n";
        return -1;*/
        fname = "C:/Users/patmjen/Documents/HCP Anywhere/projects/parallel-qpbo/data/liver.n6c10.max.bbk";
    } else {
        fname = argv[1];
    }
    std::cout << "reading " << fname << "..." << std::endl;

    try {
        auto bkg = read_graph<int, int>(fname);

        for (int i = 0; i < argc - 2; ++i) {
            std::string algo = argv[i + 2];
            if (algo == "bk") {
                std::cerr << "BK:" << std::endl;
                bench_bk(bkg);
            }
            if (algo == "mbk") {
                std::cerr << "MBK:" << std::endl;
                bench_mbk(bkg);
            }
            if (algo == "mbk2") {
                std::cerr << "MBK2:" << std::endl;
                bench_mbk2(bkg);
            }
            if (algo == "pmbk") {
                std::cerr << "Parallel MBK:" << std::endl;
                bench_pmbk(bkg, fname);
            }
            if (algo == "eibfs_old") {
                std::cerr << "EIBFS old:" << std::endl;
                bench_ibfs_old(bkg);
            }
            if (algo =="eibfs") {
                std::cerr << "EIBFS new:" << std::endl;
                bench_ibfs(bkg);
            }
            if (algo =="eibfs2") {
                std::cerr << "EIBFS new2:" << std::endl;
                bench_ibfs2(bkg);
            }
            if (algo =="peibfs") {
                std::cerr << "Parallel EIBFS:" << std::endl;
                bench_pibfs(bkg);
            }
            /*if (algo =="ppr") {
                std::cerr << "Parallel PR:" << std::endl;
                bench_ppr(bkg);
            }*/
            if (algo =="hpf") {
                std::cerr << "HPF:" << std::endl;
                bench_hpf(bkg);
            }
            if (algo =="hi_pr") {
                std::cerr << "HI_PR:" << std::endl;
                bench_hi_pr(bkg);
            }
            /*if (algo =="sk_old") {
                std::cerr << "Strandmark-Kahl old:" << std::endl;
                bench_sk_old(bkg);
            }*/
            if (algo =="sk") {
                std::cerr << "Strandmark-Kahl new:" << std::endl;
                bench_sk(bkg);
            }
#ifdef GRIDCUT_IS_AVAILABLE
            if (algo == "grid_cut") {
                std::cerr << "GridCut:" << std::endl;
                bench_grid_cut(bkg);
            }
            if (algo == "grid_cut_fastbuild") {
                std::cerr << "GridCut (fastbuild):" << std::endl;
                bench_grid_cut_fastbuild(bkg);
            }
            if (algo == "grid_cut_mt") {
                std::cerr << "Parallel GridCut:" << std::endl;
                bench_grid_cut_mt(bkg);
            }
#endif
        }
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
        return -1;
    }

    return 0;
}

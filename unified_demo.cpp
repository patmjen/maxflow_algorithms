#include <iostream>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <inttypes.h>
#include <type_traits>

#include "graph_io.h"

#include "unified/serial.h"

using Duration = std::chrono::duration<double>;
static const auto now = std::chrono::steady_clock::now;

template <class Graph, class Cap, class Term>
void run_benchmark(const BkGraph<Cap, Term>& bkg)
{
    std::cout << "building... ";
    auto build_begin = now();
    Graph graph;
    graph.allocate_size(bkg.num_nodes, bkg.terminal_arcs.size(), bkg.neighbor_arcs.size());
    for (const auto& tarc : bkg.terminal_arcs) {
        graph.add_tweights(tarc.node, tarc.source_cap, tarc.sink_cap);
    }
    for (const auto& narc : bkg.neighbor_arcs) {
        graph.add_edge(narc.i, narc.j, narc.cap, narc.rev_cap);
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
    graph.maxflow();
    Duration solve_dur = now() - solve_begin;
    std::cout << solve_dur.count() << " seconds" << std::endl;

    std::cout << "get maxflow...";
    auto maxflow_begin = now();
    auto flow = graph.get_maxflow();
    Duration maxflow_dur = now() - maxflow_begin;
    std::cout << maxflow_dur.count() << " seconds" << std::endl;

    std::cout << "total: " << (build_dur + solve_dur).count() << " seconds, maxflow: " << flow << std::endl;
}

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
        std::cout << "ERROR: must provide problem instance file\n";
        std::cout << "Usage: udemo <file> [<algo>...]\n";
        std::cout << "  Benchmark FILE with ALGOs. ALGO must be one of:\n";
        std::cout << "    bk mbk pmbk eilbfs_old eibfs eibfs2 peibfs ppr hpf hi_pr sk\n";
        return -1;
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
                run_benchmark<unified::SerialBk<int, int, int>>(bkg);
            } else if (algo == "eibfs") {
                std::cerr << "EIBFS:" << std::endl;
                run_benchmark<unified::SerialEibfs<int, int, int>>(bkg);
            } else if (algo =="hpf") {
                std::cerr << "HPF:" << std::endl;
                run_benchmark<unified::SerialHpf<int>>(bkg);
            }
        }
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
        return -1;
    }

    return 0;
}

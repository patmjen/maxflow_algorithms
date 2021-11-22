#include <iostream>
#include <chrono>
#include <string>
#include <fstream>

#include "graph_io.h"

template <class Ty>
bool operator==(const BkTermArc<Ty>& a1, const BkTermArc<Ty>& a2)
{
    return a1.node == a2.node && a1.source_cap == a2.source_cap && a1.sink_cap == a2.sink_cap;
}

template <class Ty>
bool operator==(const BkNborArc<Ty>& a1, const BkNborArc<Ty>& a2)
{
    return (a1.i == a2.i && a1.j == a2.j && a1.cap == a2.cap && a1.rev_cap == a2.rev_cap)
           || (a1.i == a2.j && a1.j == a2.i && a1.cap == a2.rev_cap && a1.rev_cap == a2.cap);
}

template <class Ty>
bool operator==(const BkUnaryTerm<Ty>& a1, const BkUnaryTerm<Ty>& a2)
{
    return a1.node == a2.node && a1.e0 == a2.e0 && a1.e1 == a2.e1;
}

template <class Ty>
bool operator==(const BkBinaryTerm<Ty>& a1, const BkBinaryTerm<Ty>& a2)
{
    return a1.i == a2.i && a1.j == a2.j && a1.e00 == a2.e00 && a1.e01 == a2.e01 && 
        a1.e10 == a2.e10 && a1.e11 == a2.e11;
}

template <class Ty>
bool operator!=(const BkTermArc<Ty>& a1, const BkTermArc<Ty>& a2)
{
    return !(a1 == a2);
}

template <class Ty>
bool operator!=(const BkNborArc<Ty>& a1, const BkNborArc<Ty>& a2)
{
    return !(a1 == a2);
}

template <class Ty>
bool operator!=(const BkUnaryTerm<Ty>& a1, const BkUnaryTerm<Ty>& a2)
{
    return !(a1 == a2);
}

template <class Ty>
bool operator!=(const BkBinaryTerm<Ty>& a1, const BkBinaryTerm<Ty>& a2)
{
    return !(a1 == a2);
}

template <class C, class T>
void check_graphs_equal(const BkGraph<C, T>& a, const BkGraph<C, T>& b)
{
    bool num_nodes_eq = a.num_nodes == b.num_nodes;
    bool nbor_arcs_eq = a.neighbor_arcs == b.neighbor_arcs;
    bool term_arcs_eq = a.terminal_arcs == b.terminal_arcs;
    if (num_nodes_eq && nbor_arcs_eq && term_arcs_eq) {
        std::cout << "SUCCESS: graphs are equal\n";
    } else {
        std::cout << "ERROR: graphs are NOT equal\n";
        if (!num_nodes_eq) {
            std::cout << "  Nodes differ. " << a.num_nodes << " != " << b.num_nodes;
        }
        if (!nbor_arcs_eq) {
            std::cout << "  Nbor. arcs differ. ";
            std::cout << a.neighbor_arcs.size() << " != " << b.neighbor_arcs.size() << '\n';
            auto size = std::min(a.neighbor_arcs.size(), b.neighbor_arcs.size());
            for (size_t i = 0; i < size; ++i) {
                const auto& aa = a.neighbor_arcs[i];
                const auto& ba = b.neighbor_arcs[i];
                if (aa != ba) {
                    std::cout << "  First different arcs:\n";
                    std::cout << "  a: " << aa.i << ' ' << aa.j << ' ';
                    std::cout << aa.cap << ' ' << aa.rev_cap << '\n';
                    std::cout << "  b: " << ba.i << ' ' << ba.j << ' ';
                    std::cout << ba.cap << ' ' << ba.rev_cap << '\n';
                    break;
                }
            }
        }
        if (!term_arcs_eq) {
            std::cout << "  Term. arcs differ. ";
            std::cout << a.terminal_arcs.size() << " vs. " << b.terminal_arcs.size() << '\n';
            auto size = std::min(a.terminal_arcs.size(), b.terminal_arcs.size());
            for (size_t i = 0; i < size; ++i) {
                const auto& aa = a.terminal_arcs[i];
                const auto& ba = b.terminal_arcs[i];
                if (aa != ba) {
                    std::cout << "  First different arcs:\n";
                    std::cout << "  a: " << aa.node << ' ';
                    std::cout << aa.source_cap << ' ' << aa.sink_cap << '\n';
                    std::cout << "  b: " << ba.node << ' ';
                    std::cout << ba.source_cap << ' ' << ba.sink_cap << '\n';
                    break;
                }
            }
        }
    }
}


void dimacs_to_bbk(const std::string& fname, bool compress)
{
    std::string bfname = fname + ".bbk";

    std::cout << "reading dimacs... ";
    auto start = std::chrono::system_clock::now();
    auto bkg_dimacs = read_dimacs_to_bk<int, int>(fname);
    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "writing " << std::string(compress ? "compressed" : "") << " bbk... ";
    start = std::chrono::system_clock::now();
    write_bk_to_bbk<int, int>(bfname, bkg_dimacs, compress);
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "reading binary bk... ";
    start = std::chrono::system_clock::now();
    auto bkg_binary = read_bbk_to_bk<int, int>(bfname);
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    check_graphs_equal(bkg_dimacs, bkg_binary);
}

template <class C, class T>
void write_dimacs(const std::string& out_fname, const BkGraph<C, T>& bkg)
{
    // We just use the slow iostream since it's easy
    std::cout << "writing dimacs (" << out_fname << ")... ";
    auto start = std::chrono::system_clock::now();
    auto dimacs = std::fstream(out_fname, std::ios::out);
    auto num_arcs = bkg.neighbor_arcs.size() + bkg.terminal_arcs.size();

    // Problem line
    dimacs << "p max " << bkg.num_nodes + 2 << ' ' << num_arcs << '\n';
    dimacs << "c terminal arcs: " << bkg.terminal_arcs.size();
    dimacs << ", neighbor_arcs: " << bkg.neighbor_arcs.size() << '\n';

    // Node descriptor lines
    dimacs << "n 1 s\nn 2 t\n";

    // Arc descriptor lines
    for (const auto& a : bkg.terminal_arcs) {
        if (a.source_cap != 0) {
            dimacs << "a 1 " << a.node + 3 << ' ' << a.source_cap << '\n';
        }
        if (a.sink_cap != 0) {
            dimacs << "a " << a.node + 3 << " 2 " << a.sink_cap << '\n';
        }
        if (a.source_cap == 0 && a.sink_cap == 0) {
            // To keep compatibility we also print empty arcs
            dimacs << "a 1 " << a.node + 3 << " 0\n";
        }
    }
    for (const auto& a : bkg.neighbor_arcs) {
        if (a.cap != 0) {
            dimacs << "a " << a.i + 3 << ' ' << a.j + 3 << ' ' << a.cap << '\n';
        }
        if (a.rev_cap != 0) {
            dimacs << "a " << a.j + 3 << ' ' << a.i + 3 << ' ' << a.rev_cap << '\n';
        }
        if (a.cap == 0 && a.rev_cap == 0) {
            // To keep compatibility we also print empty arcs
            dimacs << "a " << a.i + 3 << ' ' << a.j + 3 << " 0 0\n";
        }
    }
    
    dimacs << "c End of file\n";
    dimacs.close();
    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "reading dimacs file... ";
    start = std::chrono::system_clock::now();
    auto bkg_dimacs = read_dimacs_to_bk<int, int>(out_fname);
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";
    
    check_graphs_equal(bkg, bkg_dimacs);
}

void bbk_to_dimacs(const std::string& fname)
{
    std::string out_fname = fname + ".max";

    std::cout << "reading binary bbk... ";
    auto start = std::chrono::system_clock::now();
    auto bkg = read_bbk_to_bk<int, int>(fname);
    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    write_dimacs(out_fname, bkg);
}

void bq_to_dimacs(const std::string& fname)
{
    std::string out_fname = fname + ".max";

    std::cout << "reading bq... ";
    auto start = std::chrono::system_clock::now();
    auto bkg = qpbo_to_graph(read_bq_to_qpbo<int>(fname));
    std::chrono::duration<double> dur = std::chrono::system_clock::now()  - start;
    std::cout << dur.count() << " seconds\n";

    write_dimacs(out_fname, bkg);
}

void blk_to_txt(const std::string& fname)
{
    std::string out_fname = fname + ".txt";

    std::cout << "reading blk... ";
    auto start = std::chrono::system_clock::now();
    std::vector<uint16_t> blocks;
    uint16_t num_blocks;
    std::tie(blocks, num_blocks) = read_blocks(fname);
    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";
    
    std::cout << "writing txt... ";
    start = std::chrono::system_clock::now();
    auto txt = std::fstream(out_fname, std::ios::out);
    txt << num_blocks << '\n';
    for (auto b : blocks) {
        txt << b << ' ';
    }
    txt << '\n';
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";
}

void bbk_to_compressed_bbk(const std::string fname)
{
    std::string bfname = fname + ".bbk";
    std::string cfname = fname + ".bbkc";

    std::cout << "reading bbk... ";
    auto start = std::chrono::system_clock::now();
    auto bkg = read_bbk_to_bk<int, int>(bfname);
    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "writing compressed bbk... ";
    start = std::chrono::system_clock::now();
    write_bk_to_bbk<int, int>(cfname, bkg);
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "reading compressed bbk... ";
    start = std::chrono::system_clock::now();
    auto bkg_compressed = read_bbk_to_bk<int, int>(cfname);
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    check_graphs_equal(bkg, bkg_compressed);
}

void bq_to_compressed_bq(const std::string& fname)
{
    std::string bfname = fname + ".bq";
    std::string cfname = fname + ".bqc";

    std::cout << "reading bq... ";
    auto start = std::chrono::system_clock::now();
    auto bq = read_bq_to_qpbo<int>(bfname);
    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "writing compressed bq... ";
    start = std::chrono::system_clock::now();
    write_bq_to_qpbo<int>(cfname, bq, true);
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "reading compressed bq... ";
    start = std::chrono::system_clock::now();
    auto bq_compressed = read_bq_to_qpbo<int>(cfname);
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    if (bq.num_nodes == bq_compressed.num_nodes
        && bq.unary_terms == bq_compressed.unary_terms
        && bq.binary_terms == bq_compressed.binary_terms) {
        std::cout << "SUCCESS: QPB functions are equal\n";
    } else {
        std::cout << "ERROR: QPB functions are NOT equal\n";
    }
}

void bq_to_bbk(const std::string& fname, bool compress)
{
    std::string ofname = fname + ".bbk";

    std::cout << "reading bq... ";
    auto start = std::chrono::system_clock::now();
    auto bq = read_bq_to_qpbo<int>(fname);
    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "qpbo to bbk... ";
    start = std::chrono::system_clock::now();
    auto bkg = qpbo_to_graph(bq); 
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";

    std::cout << "writing " << std::string(compress ? "compressed" : "") << " bbk... ";
    start = std::chrono::system_clock::now();
    write_bk_to_bbk<int, int>(ofname, bkg, compress);
    dur = std::chrono::system_clock::now() - start;
    std::cout << dur.count() << " seconds\n";
}

int main(int argc, const char *argv[])
{
    if (argc < 3) {
        std::cout << "usage: bench_io <command> <fname> [--no-compress]\n";
        std::cout << "  commands:\n";
        std::cout << "  * dimacs_to_bbk\n";
        std::cout << "  * bbk_to_dimacs\n";
        std::cout << "  * bq_to_dimacs\n";      
        std::cout << "  * blk_to_txt\n";
        std::cout << "  * bbk_to_compressed_bbk\n";
        std::cout << "  * bq_to_compressed_bq\n";
        std::cout << "  * bq_to_bbk\n";
        return 0;
    }
    std::string cmd = argv[1];
    std::string fname = argv[2];
    bool compress = argc > 3 ? std::string(argv[3]) != "--no-compress" : true;
    
    try {
        if (cmd == "dimacs_to_bbk") {
            dimacs_to_bbk(fname, compress);
        } else if (cmd == "bbk_to_dimacs") {
            bbk_to_dimacs(fname);
        } else if (cmd == "bq_to_dimacs") {
            bq_to_dimacs(fname);
        } else if (cmd == "blk_to_txt") {
            blk_to_txt(fname);
        } else if (cmd == "bbk_to_compressed_bbk") {
            bbk_to_compressed_bbk(fname);
        } else if (cmd == "bq_to_compressed_bq") {
            bq_to_compressed_bq(fname);
        } else if (cmd == "bq_to_bbk") {
            bq_to_bbk(fname, compress);
        } else {
            std::cout << "ERROR: Invalid command\n";
        }
    } catch (const std::exception& e) {
        std::cout << "\nERROR: " << e.what() << "\n";
    }

    return 0;
}

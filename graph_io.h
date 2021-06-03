#ifndef GRAPH_IO_H__
#define GRAPH_IO_H__

#include <stdexcept>
#include <vector>
#include <stdio.h>
#include <inttypes.h>
#include <string>
#include <cstring>
#include <fstream>
#include <utility>
#include <tuple>
#include <type_traits>
#include <assert.h>

#include "robin_hood.h"
#include "snappy.h"

/** Enum for switching over POD types. */
enum TypeCode : uint8_t {
    TYPE_UINT8,
    TYPE_INT8,
    TYPE_UINT16,
    TYPE_INT16,
    TYPE_UINT32,
    TYPE_INT32,
    TYPE_UINT64,
    TYPE_INT64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_INVALID = 0xFF
};

/** Return the type code for a given type. */
template <class Ty>
constexpr TypeCode type_code()
{
    // We cannot use switch so have to revert to this
    if (std::is_same<Ty, uint8_t>::value) {
        return TYPE_UINT8;
    } else if (std::is_same<Ty, int8_t>::value) {
        return TYPE_INT8;
    } else if (std::is_same<Ty, uint16_t>::value) {
        return TYPE_UINT16;
    } else if (std::is_same<Ty, int16_t>::value) {
        return TYPE_INT16;
    } else if (std::is_same<Ty, uint32_t>::value) {
        return TYPE_UINT32;
    } else if (std::is_same<Ty, int32_t>::value) {
        return TYPE_INT32;
    } else if (std::is_same<Ty, uint64_t>::value) {
        return TYPE_UINT64;
    } else if (std::is_same<Ty, int64_t>::value) {
        return TYPE_INT64;
    } else if (std::is_same<Ty, float>::value) {
        return TYPE_FLOAT;
    } else if (std::is_same<Ty, double>::value) {
        return TYPE_DOUBLE;
    } else {
        return TYPE_INVALID;
    }
}

/** Terminal arc with source and sink capacity for given node. */
template <class Ty>
struct BkTermArc {
    uint64_t node;
    Ty source_cap;
    Ty sink_cap;
};

/** Neighbor arc with forward and reverse capacity. */
template <class Ty>
struct BkNborArc {
    uint64_t i;
    uint64_t j;
    Ty cap;
    Ty rev_cap;
};

/** BK-like graph with number of nodes, a list of neighbor arcs, and list of terminal arcs. */
template <class capty, class tcapty>
struct BkGraph {
    uint64_t num_nodes;

    std::vector<BkNborArc<capty>> neighbor_arcs;
    std::vector<BkTermArc<tcapty>> terminal_arcs;
};

/** Unary term */
template <class Ty>
struct BkUnaryTerm {
    uint64_t node;
    Ty e0;
    Ty e1;
};

/** Binary term */
template <class Ty>
struct BkBinaryTerm {
    uint64_t i;
    uint64_t j;
    Ty e00;
    Ty e01;
    Ty e10;
    Ty e11;
};

/** BK-like QPB energy function. Specifies number of nodes, unary terms, and binary terms */
template <class Ty>
struct BkQpbo {
    uint64_t num_nodes;
    
    std::vector<BkUnaryTerm<Ty>> unary_terms;
    std::vector<BkBinaryTerm<Ty>> binary_terms;
};

template <class Ty>
void compress_and_write(const std::vector<Ty>& vec, std::fstream& file, std::string& buffer)
{
    size_t compressed_bytes = snappy::Compress((char *)vec.data(), vec.size() * sizeof(Ty), &buffer);
    file.write((char *)&compressed_bytes, sizeof(compressed_bytes));
    file.write(buffer.data(), compressed_bytes);
}

template <class Ty>
void read_and_decompress(Ty *sink, std::fstream& file, std::string& buffer)
{
    size_t compressed_bytes;
    file.read((char *)&compressed_bytes, sizeof(compressed_bytes));
    buffer.resize(compressed_bytes);
    file.read(&buffer[0], compressed_bytes);
    snappy::RawUncompress(buffer.data(), compressed_bytes, (char *)sink);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Blocks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Blk files have the following format:
 *
 * Nodes: uint64_t num_nodes
 * Blocks: uint16_t num_blocks
 * Data: (num_nodes x uint16_t) node_blocks
 */

/**
 * Read block file describing which block each node in a corresponding graph belongs to.
 * Returns vector with block index for each node and number of blocks.
 */
std::pair<std::vector<uint16_t>, uint16_t> read_blocks(const std::string& fname);

/** Write blocks to block file. */
void write_blocks(const std::string& fname, const std::vector<uint16_t>& node_blocks, uint16_t num_blocks);

/**
 * Split vector of block indices into intervals containing runs of the same block.
 * Returns vector of (interval length, block index).
 */
std::vector<std::pair<size_t, uint16_t>> split_block_intervals(const std::vector<uint16_t>& node_blocks);

/**
 * Make intervals of block indices corresponding to box-shaped blocks in a grid graph.
 * Returns vector of (interval length, block index) and max block index.
 */
std::pair<std::vector<std::pair<size_t, uint16_t>>, uint16_t> grid_block_intervals(
    size_t grid_h, size_t grid_w, size_t grid_d, size_t block_h, size_t block_w, size_t block_d);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DIMACS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
 * Read DIMACS file describing a directed arc weighted graph with a source and sink node.
 * Implmented according to the spec. at: http://lpsolve.sourceforge.net/5.5/DIMACS_maxf.htm
 */
template <class captype, class tcaptype>
BkGraph<captype, tcaptype> read_dimacs_to_bk(const std::string fname);

/**
 * Read lines in fs until a non-comment line is read into line buffer. 
 * Returns true if more lines can be read.
 */
bool next_code_line(std::fstream& fs, std::string& line);

/**
 * Parse problem line and returns (number of node, number of arcs).
 */
std::pair<uint64_t, uint64_t> parse_problem_line(const std::string& line);

/**
 * Parse node descriptor line giving index of source or sink node.
 * Returns (i, 's') for source or (i, 't') for sink, where i is the index.
 */
std::pair<uint64_t, char> parse_node_line(const std::string& line);

/**
 * Parse arc descriptor line giving 'from' and 'to' node indices and capacity for arc.
 */
std::tuple<uint64_t, uint64_t, int64_t> parse_arc_line_int(const std::string& line);
std::tuple<uint64_t, uint64_t, double> parse_arc_line_float(const std::string& line);
template <class captype>
inline std::tuple<uint64_t, uint64_t, captype> parse_arc_line(const std::string& line)
{
    uint64_t from, to;
    captype cap;
    if (std::is_floating_point<captype>::value) {
        double dcap;
        std::tie(from, to, dcap) = parse_arc_line_float(line);
        cap = static_cast<captype>(dcap);
    } else {
        int64_t icap;
        std::tie(from, to, icap) = parse_arc_line_int(line);
        cap = static_cast<captype>(icap);
    }
    return std::make_tuple(from, to, cap);
}

/**
 * Convert DIMACS node index to BK node index.
 */
inline uint64_t convert_node_id(uint64_t n_id, uint64_t s_id, uint64_t t_id)
{
    uint64_t out = n_id - 1;
    if (n_id > s_id) out--;
    if (n_id > t_id) out--;
    return out;
}

template <class captype, class tcaptype>
BkGraph<captype, tcaptype> read_dimacs_to_bk(const std::string fname)
{
    int64_t num_nodes, s_id, t_id;

    std::fstream file(fname);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fname);
    }

    std::string line;

    // Read problem line
    next_code_line(file, line);
    std::tie(num_nodes, std::ignore) = parse_problem_line(line);

    // Subtract two as we don't add source and sink
    BkGraph<captype, tcaptype> bkg;
    bkg.num_nodes = num_nodes - 2;

    // Read node descriptors
    next_code_line(file, line);
    auto node1 = parse_node_line(line);
    next_code_line(file, line);
    auto node2 = parse_node_line(line);
    if (node1.second == 's') {
        s_id = node1.first;
        t_id = node2.first;
    } else {
        s_id = node2.first;
        t_id = node1.first;
    }

    uint64_t from, to;
    captype cap;
    while (next_code_line(file, line)) {
        std::tie(from, to, cap) = parse_arc_line<captype>(line);
        // We assume no i->source or sink->i edges exist
        if (from == s_id) {
            bkg.terminal_arcs.push_back({ convert_node_id(to, s_id, t_id), cap, 0 });
        } else if (to == t_id) {
            bkg.terminal_arcs.push_back({ convert_node_id(from, s_id, t_id), 0, cap });
        } else {
            assert(to != s_id && from != t_id);
            bkg.neighbor_arcs.push_back(
                { convert_node_id(from, s_id, t_id), convert_node_id(to, s_id, t_id), cap, 0 });
        }
    }

    return bkg;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary BK
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Binary BK files have the following format:
 *
 * Uncompressed
 * ============
 * Header: (3 x uint8) 'BBQ'
 * Types codes: (2 x uint8) captype, tcaptype
 * Sizes: (3 x uint64) num_nodes, num_terminal_arcs, num_neighbor_arcs
 * Terminal arcs: (num_terminal_arcs x BkTermArc)
 * Neighbor arcs: (num_neighbor_arcs x BkNborArc)
 *
 * Compressed
 * ==========
 * Header: (3 x uint8) 'bbq'
 * Types codes: (2 x uint8) captype, tcaptype
 * Sizes: (3 x uint64) num_nodes, num_terminal_arcs, num_neighbor_arcs
 * Terminal arcs: (1 x uint64) compressed_bytes_1
 *                (compressed_bytes_1 x uint8) compressed num_terminal_arcs x BkTermArc
 * Neighbor arcs: (1 x uint64) compressed_bytes_2
 *                (compressed_bytes_2 x uint8) compressed num_neighbor_arcs x BkNborArc
 */

/** Read binary BK file */
template <class captype, class tcaptype>
BkGraph<captype, tcaptype> read_bbk_to_bk(const std::string fname)
{
    std::fstream file(fname, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fname);
    }

    // Read file header
    uint8_t header[3] = { 0 };
    file.read((char *)header, sizeof(header));
    if (std::strncmp((char *)header, "BBQ", 3) != 0 && std::strncmp((char *)header, "bbq", 3) != 0) {
        throw std::runtime_error("Invalid file header for binary BK file.");
    }

    const bool compressed = std::islower(header[0]);

    // Read graph types and ensure they are correct
    uint8_t types[2] = { TYPE_INVALID };
    file.read((char *)types, sizeof(types));
    if (types[0] != type_code<captype>() || types[1] != type_code<tcaptype>()) {
        throw std::runtime_error("Types for binary BK file do not match requested.");
    }

    // Read graph sizes
    uint64_t sizes[3] = { 0 };
    file.read((char *)sizes, sizeof(sizes));
    const uint64_t num_nodes = sizes[0];
    const uint64_t num_term_arcs = sizes[1];
    const uint64_t num_nbor_arcs = sizes[2];

    // Read graph
    BkGraph<captype, tcaptype> bkg;
    bkg.num_nodes = num_nodes;
    bkg.terminal_arcs = std::vector<BkTermArc<tcaptype>>(num_term_arcs);
    bkg.neighbor_arcs = std::vector<BkNborArc<captype>>(num_nbor_arcs);

    if (compressed) {
        std::string buffer;
        read_and_decompress(bkg.terminal_arcs.data(), file, buffer);
        read_and_decompress(bkg.neighbor_arcs.data(), file, buffer);
    } else {
        file.read((char *)bkg.terminal_arcs.data(), num_term_arcs * sizeof(BkTermArc<tcaptype>));
        file.read((char *)bkg.neighbor_arcs.data(), num_nbor_arcs * sizeof(BkNborArc<captype>));
    }

    return bkg;
}

/** Write binary BK file */
template <class captype, class tcaptype>
void write_bk_to_bbk(const std::string fname, const BkGraph<captype, tcaptype>& bkg, bool compress = true)
{
    std::fstream file(fname, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fname);
    }

    // Write file header
    if (compress) {
        file.write("bbq", 3);
    } else {
        file.write("BBQ", 3);
    }

    // Write graph types and sizes
    static const uint8_t types[2] = { type_code<captype>(), type_code<tcaptype>() };
    const uint64_t sizes[3] = { bkg.num_nodes, bkg.terminal_arcs.size(), bkg.neighbor_arcs.size() };
    file.write((char *)types, sizeof(types));
    file.write((char *)sizes, sizeof(sizes));

    // Write graph data
    if (compress) {
        std::string buffer;
        compress_and_write(bkg.terminal_arcs, file, buffer);
        compress_and_write(bkg.neighbor_arcs, file, buffer);
    } else {
        file.write((char *)bkg.terminal_arcs.data(), bkg.terminal_arcs.size() * sizeof(BkTermArc<tcaptype>));
        file.write((char *)bkg.neighbor_arcs.data(), bkg.neighbor_arcs.size() * sizeof(BkNborArc<captype>));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary QPBO
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Binary QPBO files have the following format:
 *
 * Uncompressed
 * ============
 * Header: (5 x uint8) 'BQPBO'
 * Types codes: (1 x uint8) captype
 * Sizes: (3 x uint64) num_nodes, num_unary_terms, num_binary_terms
 * Unary arcs: (num_unary_terms x BkUnaryTerm)
 * Binary arcs: (num_binary_terms x BkBinaryTerm)
 *
 * Compressed
 * ==========
 * Header: (5 x uint8) 'bqpbo'
 * Types codes: (1 x uint8) captype
 * Sizes: (3 x uint64) num_nodes, num_unary_terms, num_binary_terms
 * Unary terms: (1 x uint64) compressed_bytes_1
 *              (compressed_bytes_1 x uint8) compressed num_unary_terms x BkUnaryTerm
 * Binary terms: (1 x uint64) compressed_bytes_2
 *               (compressed_bytes_2 x uint8) compressed num_binary_terms x BkBinaryTerm
 */

/** Read binary QPBO file */
template <class captype>
BkQpbo<captype> read_bq_to_qpbo(const std::string fname)
{
    std::fstream file(fname, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fname);
    }

    // Read file header
    uint8_t header[5] = { 0 };
    file.read((char *)header, sizeof(header));
    if (std::strncmp((char *)header, "BQPBO", sizeof(header)) != 0 &&
        std::strncmp((char *)header, "bqpbo", sizeof(header)) != 0) {
        throw std::runtime_error("Invalid file header for binary QPBO file.");
    }

    const bool compressed = std::islower(header[0]);

    // Read QPBO type and ensure it is correct
    uint8_t type = TYPE_INVALID;
    file.read((char *)&type, sizeof(type));
    if (type != type_code<captype>()) {
        throw std::runtime_error("Type for binary QPBO file do not match requested.");
    }

    // Read QPBO sizes
    uint64_t sizes[3] = { 0 };
    file.read((char *)sizes, sizeof(sizes));
    const uint64_t num_nodes = sizes[0];
    const uint64_t num_unary = sizes[1];
    const uint64_t num_binary = sizes[2];

    // Read QPBO
    BkQpbo<captype> bq;
    bq.num_nodes = num_nodes;
    bq.unary_terms = std::vector<BkUnaryTerm<captype>>(num_unary);
    bq.binary_terms = std::vector<BkBinaryTerm<captype>>(num_binary);

    if (compressed) {
        std::string buffer;
        read_and_decompress(bq.unary_terms.data(), file, buffer);
        read_and_decompress(bq.binary_terms.data(), file, buffer);
    } else {
        file.read((char *)bq.unary_terms.data(), num_unary * sizeof(BkUnaryTerm<captype>));
        file.read((char *)bq.binary_terms.data(), num_binary * sizeof(BkBinaryTerm<captype>));
    }

    return bq;
}

/** Write binary QPBO file */
template <class captype>
void write_bq_to_qpbo(const std::string fname, const BkQpbo<captype>& bq, bool compress = true)
{
    std::fstream file(fname, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fname);
    }

    // Write file header
    if (compress) {
        file.write("bqpbo", 5);
    } else {
        file.write("BQPBO", 5);
    }

    // Write QPBO type and sizes
    static const uint8_t type = type_code<captype>();
    const uint64_t sizes[3] = { bq.num_nodes, bq.unary_terms.size(), bq.binary_terms.size() };
    file.write((char *)&type, sizeof(type));
    file.write((char *)sizes, sizeof(sizes));

    // Write QPBO data
    if (compress) {
        std::string buffer;
        compress_and_write(bq.unary_terms, file, buffer);
        compress_and_write(bq.binary_terms, file, buffer);
    } else {
        file.write((char *)bq.unary_terms.data(), bq.unary_terms.size() * sizeof(BkUnaryTerm<captype>));
        file.write((char *)bq.binary_terms.data(), bq.binary_terms.size() * sizeof(BkBinaryTerm<captype>));
    }
}

/** Convert binary QPB energy term to normal form graph arc weights */
template <class Cap>
inline std::tuple<Cap, Cap, Cap, Cap> compute_normal_form_weights(Cap e00, Cap e01, Cap e10, Cap e11)
{
    // TODO: Refactor QPBO classes use this function (possibly moved to other file) instead of own method

    // The goal is to rewrite the energy terms e00, e10, e10, e11 so:
    //  1. They are in normal form
    //  2. Only e10 and e01 are non-zero
    Cap ci, cj, cij, cji;
    ci = e11 - e00;
    cij = e01 - e00;
    cji = e10 - e11;

    if (cij < 0) {
        ci -= cij;
        cj = cij;
        cji += cij;
        cij = 0;
    } else if (cji < 0) {
        ci += cji;
        cj = -cji;
        cij += cji;
        cji = 0;
    } else {
        cj = 0;
    }

    return std::make_tuple(ci, cj, cij, cji);
}

/** Convert QPB energy function to graph 
 *
 * This function is essentially equivalent to adding terms to a QPBO instance,
 * calling tranform_to_second_stage, and then reading out the resulting graph.
 */
template <class Cap>
BkGraph<Cap, Cap> qpbo_to_graph(const BkQpbo<Cap>& bq)
{
    BkGraph<Cap, Cap> bk;
    bk.num_nodes = bq.num_nodes * 2;
    bk.terminal_arcs.resize(bq.unary_terms.size() * 2);
    bk.neighbor_arcs.resize(bq.binary_terms.size() * 2);

    size_t node_offset = bq.unary_terms.size();
    size_t arc_offset = bq.binary_terms.size();
    size_t dual_offset = bq.num_nodes;

    robin_hood::unordered_map<uint64_t, Cap> tr_caps;

    // Add primal and dual arcs for each binary term
    size_t i = 0;
    for (const auto& b : bq.binary_terms) {
        Cap ci, cj, cij, cji;
        if (b.e00 + b.e11 <= b.e01 + b.e10) {
            // Term is submodular
            std::tie(ci, cj, cij, cji) = compute_normal_form_weights(b.e00, b.e01, b.e10, b.e11);
            bk.neighbor_arcs[i] = { b.i, b.j, cij, cji };
            bk.neighbor_arcs[i + arc_offset] = { b.j + dual_offset, b.i + dual_offset, cij, cji };
        } else {
            // Term is not submodular
            // Note that energy coefs. are switched!
            std::tie(ci, cj, cij, cji) = compute_normal_form_weights(b.e01, b.e00, b.e11, b.e10);
            bk.neighbor_arcs[i] = { b.i, b.j + dual_offset, cij, cji };
            bk.neighbor_arcs[i + arc_offset] = { b.j, b.i + dual_offset, cij, cji };
        }

        tr_caps[b.i] += ci;
        tr_caps[b.j] += cj;

        i++;
    }

    // Add primal and dual nodes for each unary term
    i = 0;
    for (const auto& u : bq.unary_terms) {
        Cap e0 = u.e0;
        Cap e1 = u.e1;

        auto iter = tr_caps.find(u.node);
        if (iter != tr_caps.end()) {
            Cap cap = iter->second;
            if (cap < 0) {
                e1 += -cap;
            } else {
                e0 += cap;
            }
            tr_caps.erase(iter); // Erase so it doesn't get added a second time
        }

        bk.terminal_arcs[i] = { u.node, e1, e0 };
        bk.terminal_arcs[i + node_offset] = { u.node + dual_offset, e0, e1 };
        i++;
    }

    // Add terminal arcs for remaining tr_caps entries
    for (const auto& keyval : tr_caps) {
        uint64_t node = keyval.first;
        Cap cap = keyval.second;
        // Add extra terminal arc if cap is non-zero
        if (cap > 0) {
            bk.terminal_arcs.push_back({ node, cap, 0 });
            bk.terminal_arcs.push_back({ node + dual_offset, 0, cap });
        } else if (cap < 0) {
            bk.terminal_arcs.push_back({ node, 0, -cap });
            bk.terminal_arcs.push_back({ node + dual_offset, -cap, 0 });
        }
    }

    return bk;
}

#endif // GRAPH_IO_H__

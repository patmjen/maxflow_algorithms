#include "graph_io.h"
#include <algorithm>
#include <cctype>

std::pair<std::vector<uint16_t>, uint16_t> read_blocks(const std::string & fname)
{
    std::fstream file(fname, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fname);
    }

    uint64_t num_nodes;
    file.read((char *)&num_nodes, sizeof(num_nodes));

    uint16_t num_blocks;
    file.read((char *)&num_blocks, sizeof(num_blocks));

    std::vector<uint16_t> blocks(num_nodes);
    file.read((char *)blocks.data(), num_nodes * sizeof(uint16_t));

    return std::make_pair(blocks, num_blocks);
}

void write_blocks(const std::string & fname, const std::vector<uint16_t>& node_blocks, uint16_t num_blocks)
{
    std::fstream file(fname, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fname);
    }

    uint64_t num_nodes = node_blocks.size();
    file.write((char *)&num_nodes, sizeof(num_nodes));

    file.write((char *)&num_blocks, sizeof(num_blocks));

    file.write((char *)node_blocks.data(), num_nodes * sizeof(uint16_t));
}

std::vector<std::pair<size_t, uint16_t>> split_block_intervals(const std::vector<uint16_t>& node_blocks)
{
    size_t count = 0;
    uint16_t crnt = 0;
    std::vector<std::pair<size_t, uint16_t>> intervals;
    for (uint16_t block : node_blocks) {
        if (block != crnt) {
            if (count > 0) {
                intervals.push_back(std::make_pair(count, crnt));
            }
            crnt = block;
            count = 0;
        }
        count++;
    }
    if (count > 0) {
        intervals.push_back(std::make_pair(count, crnt));
    }
    return intervals;
}

std::pair<std::vector<std::pair<size_t, uint16_t>>, uint16_t> grid_block_intervals(
    size_t grid_h, size_t grid_w, size_t grid_d, size_t block_h, size_t block_w, size_t block_d)
{
    std::vector<std::pair<size_t, uint16_t>> intervals;
    uint16_t max_block = 0;

    // Compute number of blocks in each dimension
    size_t nblock_h = grid_h / block_h + (grid_h % block_h != 0);
    size_t nblock_w = grid_w / block_w + (grid_w % block_w != 0);
    size_t nblock_d = grid_d / block_d + (grid_d % block_d != 0);

    for (size_t k = 0; k < grid_d; ++k) {
        for (size_t j = 0; j < grid_w; ++j) {
            for (size_t bi = 0; bi < nblock_h; ++bi) {
                size_t bk = k / block_d;
                size_t bj = j / block_w;

                uint16_t block = bi + bj * nblock_h + bk * nblock_h * nblock_w;
                size_t block_length = std::min(grid_h, (bi + 1) * block_h) - bi * block_h;
                if (intervals.size() && intervals.back().second == block) {
                    // If the previous interval had the same block, just extend it
                    intervals.back().first += block_length;
                } else {
                    intervals.emplace_back(block_length, block);
                }
                max_block = std::max(max_block, block);
            }
        }
    }

    return std::make_pair(intervals, max_block);
}

bool next_code_line(std::fstream& fs, std::string& line)
{
    bool should_skip = true;
    bool can_read = fs && !fs.eof();
    while (can_read && should_skip) {
        std::getline(fs, line); // TODO: Reading to a std::string maybe takes a lot of time...
        can_read = fs && !fs.eof();
        // We skip the line if it's a comment or all whitespace
        should_skip = line[0] == 'c' || std::all_of(line.begin(), line.end(), isspace);
    }
    return can_read;
}

std::pair<uint64_t, uint64_t> parse_problem_line(const std::string& line)
{
    char ltype;
    char buf[32] = { '\0' };
    uint64_t num_nodes, num_arcs;

    assert(line[0] == 'p');
    sscanf(line.c_str(), "%c %s %llu %llu", &ltype, buf, &num_nodes, &num_arcs);
    assert(ltype == 'p' && std::strncmp(buf, "max", 3) == 0);
    return std::make_pair(num_nodes, num_arcs);
}

std::pair<uint64_t, char> parse_node_line(const std::string& line)
{
    char ltype, ntype;
    uint64_t nid;
    assert(line[0] == 'n');
    sscanf(line.c_str(), "%c %llu %c", &ltype, &nid, &ntype);
    assert(ltype == 'n');
    return std::make_pair(nid, ntype);
}

std::tuple<uint64_t, uint64_t, int64_t> parse_arc_line_int(const std::string& line)
{
    char ltype;
    uint64_t from, to;
    int64_t cap;
    assert(line[0] == 'a');
    sscanf(line.c_str(), "%c %llu %llu %lld", &ltype, &from, &to, &cap);
    assert(ltype == 'a');
    return std::make_tuple(from, to, cap);
}

std::tuple<uint64_t, uint64_t, double> parse_arc_line_float(const std::string& line)
{
    char ltype;
    uint64_t from, to;
    double cap;
    assert(line[0] == 'a');
    sscanf(line.c_str(), "%c %llu %llu %lf", &ltype, &from, &to, &cap);
    assert(ltype == 'a');
    return std::make_tuple(from, to, cap);
}

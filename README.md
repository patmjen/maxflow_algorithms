# Max-Flow/Min-Cut Algorithms

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.4903945.svg)](https://doi.org/10.5281/zenodo.4903945)


This is a collection of min-cut/max-flow algorithms which can used for benchmarking min-cut/max-flow algoriths. The collection is released in companionship with the paper:

<ul><b>Review of Serial and Parallel Min-Cut/Max-Flow Algorithms for Computer Vision</b>,<br>
    Patrick M. Jensen, Niels Jeppesen, Anders B. Dahl, Vedrana A. Dahl, 2022 (Under review).<br>
  [ <a href="https://arxiv.org/abs/2202.00418">arXiv preprint</a> ]
</ul>

If you make use of these, remember to also cite the relevant papers.

## Contents

* [Implemented Algorithms](#Implemented-Algorithms)
* [Programs](#Programs)
* [How to Build](#How-to-Build)
* [Licences](#Licences)
* [Binary File Formats](#Binary-File-Formats)

## Implemented Algorithms

This collection includes the following implementations:

* `bk` - Author reference implementation of the Boykov-Kolmogorov (BK) algorithm from Boykov & Kolmogorov, "An ExperimentalComparison of Min-Cut/Max-Flow Algorithms for EnergyMinimization in Vision", 2004, PAMI.
* `mbk` - Our re-implementation of the BK algorithm containing several low-level optimizations. Generally, this implementation performs faster than the author reference.
* `eibfs_old` - Author reference implementation of the Excesses Incremental Breadth-First Search (EIBFS) algorithm from Goldberg et al., "Faster  and  More  DynamicMaximum Flow by Incremental Breadth-First Search", 2015, ESA.
* `eibfs` - Our re-implementation of the EIBFS algorithm using indices instead of pointers. Generally, this implementation performs better than the author implementation.
* `eibfs2` - Our re-implementation of the EIBFS algorithm using indices instead of pointers and with no arc reordering before solving. This uses less memory than the other version, but is generally slower.
* `hpf` - Author reference implementation of the Hochbaum pseudoflow (HPF) from Hochbaum, "The  pseudoflow algorithm: A new algorithm for the maximum-flow problem", 2008, Operations Research. There are four possible configurations of this algorithm:
  1. Highest label with FIFO buckets: `hpf_hf`.
  2. Highest label with LIFO buckets: `hpf_hl`.
  3. Lowest label with FIFO buckets: `hpf_lf`.
  4. Lowest label with LIFO buckets: `hpf_ll`.
* `pmbk` - Our re-implementation of the parallel bottom-up merging approach from Liu & Sun, "Parallel Graph-cuts by Adaptive Bottom-up Merging", 2010, CVPR. The original author implementation only allowed for grid graphs while our implementation can handle any graph and any divison into blocks. Note, that this comes with a small performance penalty while building the graph.
* `pard` - Author reference implementation of the parallel region discharge algorithm from Shekhovtsov & Hlaváč, "A Distributed Mincut/Maxflow Algorithm Combining Path Augmentation and Push-Relabel", 2013, IJCV. Note that the implementation writes the graph to disk as part of initialization which adds extra overhead.
* `ppr` - Author reference implementation of a synchronous parallel push-relabel method from Baumstark et al., "Efficient implementation of a synchronous parallel push-relabel algorithm", 2015, ESA.
* `sk` - Our re-implementation of the parallel dual decomposition approach from Strandmark & Kahl, "Parallel and Distributed Graph Cuts by Dual Decomposition", 2010, CVPR.
* `peibfs` - Our implementation of the parallel bottom-up mering approach by Liu and Sun but using EIBFS instead of BK for the max-flow/min-cut computations. Due to high initialization costs, this implementation generally performs worse than the BK version.

## Programs

We provide three programs:

* **`demo`**: Allows for quick benchmarks by running a specified set of algorithms on a single problem instance. The problem instance must be saved in our binary file format (`.bkk` or `.bq`, see below). Usage:

  ```txt
  usage: demo <file> [<algo>...]\n";
    Benchmark FILE with ALGOs. ALGO must be one of:
    bk mbk pmbk eilbfs_old eibfs eibfs2 peibfs ppr hpf hi_pr sk
  ```

  Example for benchmarking the `bone.n6c10.max.bkk` problem instance with the
  `MBK` and `EIBFS` algorithms:

  ```txt
  demo bone.n6c10.max.bbk mbk eibfs
  ```

* **`bench`**: Allows for more systematic benchmarking by specifying the setup via a JSON configuration file. This is the program which produced the results in the above paper. Usage:

  ```txt
  usage: bench <json_config>
  ```

  The json config file should contain the following fields:

  * `name`: The benchmark name.
  * `num_run`: How many times to run each algorithm on each dataset.
  * `types`: List of types that the algorithms are to use. Each entry must have the form

    ```json
    {
      "cap": "<Type for arc capacities. Can be int32 or int64>",
      "term": "<Type for terminal arc capacities. Can be int32 or int64>",
      "flow": "<Type for the final maxflow. Can be int64>",
      "index": "<Type for indices. Can be uint32, int32, uint64, or int64>"
    }
    ```

  * `algorithms`: List of algorithms to use. Entries follow the abbreviations in [Implemented Algorithms](#Implemented-Algorithms).
  * `data_sets`: List of entries specifying the problem instances to run on. Each entry must have the form

    ```json
    {
      "file_name": "<Path to file name>",
      "file_type": "<Type of file. Can be 'bbk', 'bq', or 'dimacs'>",
      "nbor_cap_type": "<Type of neighbor arc capacities>",
      "term_cap_type": "<Type of terminal arc capacities>"
    }
    ```

    If parallel algorithms are being run, each file must also have a corresponding block file (see [Binary File Formats](#Binary-File-Formats)), which specifies a partition of the graph nodes into blocks. The name of this file must be equal to the "file_name" field with ".blk" appended - e.g. for 'example.max' the block file is 'example.max.blk'.
  * `parallel`: If parallel algorithms are run, this field configures properties specific for those. It must include a `threads` field giving a list of the number of threads to run with for each problem instance and each parallel algorithm.

  Two examples of json config files are included: `bench_config_serial.json` and `bench_config_parallel.json`.

* **`bench_io`**: Allows for converting between the different file formats. Usage:

  ```txt
  usage: bench_io <command> <fname>
    commands:
    * dimacs_to_bbk
    * bbk_to_dimacs
    * bq_to_dimacs
    * blk_to_txt
    * bbk_to_compressed_bbk
    * bq_to_compressed_bq
  ```

## How to Build

The programs are written in C++ and and we use CMake version 3.13 to build the programs. Below, we provide build instructions for the major operating systems. Mac OS was not tested, but hopefully the linux instructions will suffice. Some author reference implementations make use of the Intel Threading Build Blocks library, so this must be installed and you must modify the `TBB_PATH` variable in `CMakeLists.txt`, line 28.

### Windows

For Windows we used Visual Studio Community 2019 (2) Version 16.7.3 with the Microsoft (R) C/C++ Optimizing Compiler Version 19.27.29111).

First, compile the parallel region discharge code in the `prd` directory. Following the author instructions (https://cmp.felk.cvut.cz/~shekhovt/d_maxflow/) this is done by opening `prd/projects/d_maxflow.sln` with Visual Studion, configuring as Release-Win64 and building the solution.

Then, compile our programs with CMake by running the following from the project directory:

```txt
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The programs should then be in the `build/Release` directory.

### Linux / Mac OS

For linux (and Mac OS) we used GCC version 9.2.0. First, compile the parallel region discharge code in the `prd` directory. Following the author instructions (https://cmp.felk.cvut.cz/~shekhovt/d_maxflow/) this is done by navigating to the `prd` directory and running `make -C projects/d_maxflow`.

Then, compile our programs with CMake by running the following from the project directory:

```txt
mkdir build
cd build
cmake ..
cmake --build .
```

The programs should then be in the `build` directory.

## Licences

We provide our re-implementations under the MIT license (see license file in `reimpls`), except for `hpf.h`, `ibfs.h`, and `ibfs2.h` which have their own licenses. Note that the `robin_hood` hashtable (in `robin_hood.h`) is not developed by us but is also released under its own MIT license (see the file).

The original author implementations are released under their own licenses. Where available, licenses are provided in the respective directories and we list them below for convenience. Note that this is not legal advice - double check yourself before use. Where a license was available, they allow for research purposes but most disallow commercial use.

* `bk`: GPLv3 license.
* `hi_pr`: Own license.
* `hpf`: Own license.
* `eibfs`: Own license.
* `nbk`: GPLv3 license.
* `prd`: Own license.
* `reimpls`: MIT license.
* `sk`: Unknown. Contact orignal author for more information.
* `sppr`: Unknown. Contact original author for more information.

## Binary File Formats

We have two formats: one for graphs and one for quadratic pseudo-boolean optimization (QPBO) problems. We have also provded a program to convert between the formats - see above.

**Binary BK** (`.bbk`) files are for storing normal graphs for min-cut/max-flow. They closely follow the internal storage format used in the original implementation of the Boykov-Kolmogorov algorithm, meaning that terminal arcs are stored in a separate list from normal neighbor arcs. The format is:

Uncompressed:

```txt
Header: (3 x uint8) 'BBQ'
Types codes: (2 x uint8) captype, tcaptype
Sizes: (3 x uint64) num_nodes, num_terminal_arcs, num_neighbor_arcs
Terminal arcs: (num_terminal_arcs x BkTermArc)
Neighbor arcs: (num_neighbor_arcs x BkNborArc)
```

Compressed (using Google's snappy: https://github.com/google/snappy):

```txt
Header: (3 x uint8) 'bbq'
Types codes: (2 x uint8) captype, tcaptype
Sizes: (3 x uint64) num_nodes, num_terminal_arcs, num_neighbor_arcs
Terminal arcs: (1 x uint64) compressed_bytes_1
               (compressed_bytes_1 x uint8) compressed num_terminal_arcs x BkTermArc
Neighbor arcs: (1 x uint64) compressed_bytes_2
               (compressed_bytes_2 x uint8) compressed num_neighbor_arcs x BkNborArc
```

Where:

```c++
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
```

**Binary QPBO** (`.bq`) files are for storing QPBO problems. Unary and binary
terms are stored in separate lists. The format is:

Uncompressed:

```txt
Header: (5 x uint8) 'BQPBO'
Types codes: (1 x uint8) captype
Sizes: (3 x uint64) num_nodes, num_unary_terms, num_binary_terms
Unary arcs: (num_unary_terms x BkUnaryTerm)
Binary arcs: (num_binary_terms x BkBinaryTerm)
```

Compressed (using Google's snappy: https://github.com/google/snappy):

```txt
Header: (5 x uint8) 'bqpbo'
Types codes: (1 x uint8) captype
Sizes: (3 x uint64) num_nodes, num_unary_terms, num_binary_terms
Unary terms: (1 x uint64) compressed_bytes_1
             (compressed_bytes_1 x uint8) compressed num_unary_terms x BkUnaryTerm
Binary terms: (1 x uint64) compressed_bytes_2
              (compressed_bytes_2 x uint8) compressed num_binary_terms x BkBinaryTerm
```

Where:

```c++
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
```

**Block** (`.blk`) files are for storing a partition of the graph nodes into
disjoint blocks. The format is:

```txt
Nodes: uint64_t num_nodes
Blocks: uint16_t num_blocks
Data: (num_nodes x uint16_t) node_blocks
```

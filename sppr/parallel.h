#ifndef _PARALLEL_H
#define _PARALLEL_H

// Different compilers have different pragma macros
#ifdef _MSC_VER
#define SPPR_PRAGMA(x) __pragma(x)
#else
#define SPPR_PRAGMA(x) _Pragma(x)
#endif

// cilkarts cilk++
#if defined(CILK)
#include <cilk.h>
#include <cassert>
#define parallel_main cilk_main
#define sppr_parallel_for cilk_for
#define sppr_parallel_for_1 SPPR_PRAGMA("cilk_grainsize = 1") cilk_for
#define sppr_parallel_for_256 SPPR_PRAGMA("cilk_grainsize = 256") cilk_for

static int getWorkers() { return -1; }
static void setWorkers(int n) { }

// intel cilk+
#elif defined(CILKP)
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#define sppr_parallel_for cilk_for
#define parallel_main main
#define sppr_parallel_for_1 SPPR_PRAGMA("cilk grainsize = 1") sppr_parallel_for
#define sppr_parallel_for_256 SPPR_PRAGMA("cilk grainsize = 256") sppr_parallel_for

static int getWorkers() {
  return __cilkrts_get_nworkers();
}
static void setWorkers(int n) {
  __cilkrts_end_cilk();
  //__cilkrts_init();
  std::stringstream ss; ss << n;
  if (0 != __cilkrts_set_param("nworkers", ss.str().c_str())) {
    std::cerr << "failed to set worker count!" << std::endl;
    std::abort();
  }
}

// openmp
#elif defined(_OPENMP)
#include <omp.h>
#define cilk_spawn
#define cilk_sync
#define parallel_main main
#define sppr_parallel_for SPPR_PRAGMA("omp parallel for") for
#define sppr_parallel_for_1 SPPR_PRAGMA("omp parallel for schedule (static,1)") for
#define sppr_parallel_for_256 SPPR_PRAGMA("omp parallel for schedule (static,256)") for

static int getWorkers() { return omp_get_max_threads(); }
static void setWorkers(int n) { omp_set_num_threads(n); }

// c++
#else
#define cilk_spawn
#define cilk_sync
#define parallel_main main
#define sppr_parallel_for for
#define sppr_parallel_for_1 for
#define sppr_parallel_for_256 for
#define cilk_for for

static int getWorkers() { return 1; }
static void setWorkers(int n) { }

#endif

#include <limits.h>

#if defined(LONG)
typedef long intT;
typedef unsigned long uintT;
#define INT_T_MAX LONG_MAX
#define UINT_T_MAX ULONG_MAX
#else
typedef int intT;
typedef unsigned int uintT;
#define INT_T_MAX INT_MAX
#define UINT_T_MAX UINT_MAX
#endif

#endif // _PARALLEL_H

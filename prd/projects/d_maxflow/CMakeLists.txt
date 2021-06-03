cmake_minimum_required (VERSION 2.6)
project (d_maxflow)
set (VERSION_MAJOR 1)
set (VERSION_MINOR 1)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../../lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin)

set (CMAKE_BUILD_TYPE Release)
set (BUILD_SHARED_LIBS OFF)

configure_file (
  "cfg.h.in"
  "${PROJECT_BINARY_DIR}/cfg.h"
  )
include_directories("${PROJECT_BINARY_DIR}")
include_directories(../../code)

ADD_SUBDIRECTORY(../../code ../../build/exttype)
ADD_SUBDIRECTORY(../../code/d_maxflow ../../build/d_maxflow)

add_executable(cut_cost ../../code/d_maxflow/cut_cost.cpp)
add_executable(solve_dimacs ../../code/d_maxflow/solve_dimacs.cpp)
add_executable(solve_random ../../code/d_maxflow/solve_random.cpp)

target_link_libraries (solve_dimacs exttype d_maxflow maxflow_BK maxflow_GT maxflow_DD)
target_link_libraries (solve_random exttype d_maxflow maxflow_BK maxflow_GT maxflow_DD)

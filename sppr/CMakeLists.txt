cmake_minimum_required(VERSION 3.13)

add_library(sppr STATIC "maxFlow.cpp")
target_include_directories(sppr PUBLIC .)

# TODO: Don't hardcode this path
if (EXISTS ${TBB_PATH})
    message("SPPR TBB path: " "${TBB_PATH}")
    if (WIN32)
        target_link_directories(sppr PRIVATE "${TBB_PATH}/lib/intel64_win/vc14")
    else()
        target_link_directories(sppr PRIVATE "${TBB_PATH}/lib/intel64/gcc4.8")
    endif()
    target_include_directories(sppr PUBLIC "${TBB_PATH}/include")
    target_link_libraries(sppr tbb)
    target_compile_definitions(sppr PRIVATE SPPR_HAS_TBB)
else()
    message(WARNING "Cannot find Intel Threading Building Blocks library. Some algorithms may not compile.")
endif()

# TODO: Use better method to set OpenMP flags
if (WIN32)
    target_compile_options(sppr PRIVATE /openmp)
else()
    target_compile_options(sppr PRIVATE -fopenmp)
    target_link_options(sppr PRIVATE -g -O3 -lpthread -fopenmp)
endif()

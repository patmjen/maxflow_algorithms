cmake_minimum_required(VERSION 3.13)

add_library(sppr STATIC "maxFlow.cpp")
target_include_directories(sppr PUBLIC .)

# TODO: Don't hardcode this path
file(TO_CMAKE_PATH "~/oneapi-tbb-2021.1.1" SPPR_TBB_PATH)
if (EXISTS ${SPPR_TBB_PATH})
    message("SPPR TBB path: " "${SPPR_TBB_PATH}")
    foreach (TGT sppr)
        target_link_directories(${TGT} PRIVATE "${SPPR_TBB_PATH}/lib/intel64/gcc4.8")
        target_include_directories(${TGT} PUBLIC "${SPPR_TBB_PATH}/include")
        target_link_libraries(${TGT} tbb)
        target_compile_definitions(${TGT} PRIVATE SPPR_HAS_TBB)
    endforeach()
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

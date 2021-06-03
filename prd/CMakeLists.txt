cmake_minimum_required(VERSION 3.13)

add_library(d_maxflow_prd INTERFACE)

if (WIN32)
    # For Windows we assume d_maxflow was built using the provided Visual Studio Solution
    target_link_directories(d_maxflow_prd INTERFACE lib)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        target_link_libraries(d_maxflow_prd INTERFACE d_maxflow-x64-D exttype-x64-D)
    else()
        target_link_libraries(d_maxflow_prd INTERFACE d_maxflow-x64 exttype-x64)
    endif()
else()
    # For unix/linux we assume d_maxflow was build using the provided makefile
    target_link_directories(d_maxflow_prd INTERFACE bin/lib)
    target_link_libraries(d_maxflow_prd INTERFACE d_maxflow exttype)
endif()
target_include_directories(d_maxflow_prd INTERFACE code)

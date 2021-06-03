cmake_minimum_required(VERSION 3.13)

# Find location of boost libraries
if (NOT DEFINED ENV{BOOST_ROOT})
    message(FATAL_ERROR "Environment variable for boost location not found")
endif()
file(TO_CMAKE_PATH "$ENV{BOOST_ROOT}" SK_BOOST_PATH)
message("Boost path for SK: " "${SK_BOOST_PATH}")

add_library(sk STATIC parallelmaxflow.cpp)
target_include_directories(sk PUBLIC ${SK_BOOST_PATH} .)
target_link_directories(sk PUBLIC ${SK_BOOST_PATH}/stage/lib)
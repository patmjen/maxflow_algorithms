cmake_minimum_required(VERSION 3.13)

add_library(hi_pr STATIC hi_pr.cpp construct.cpp timer.cpp parser.cpp)
target_include_directories(hi_pr PUBLIC .)
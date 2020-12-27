cmake_minimum_required(VERSION 3.15)
project(lab)

set(CMAKE_CXX_STANDARD 20)

add_executable(lab src/main.cpp src/DSMgr.cpp src/DSMgr.h src/BMgr.cpp src/BMgr.h src/LRU.cpp src/LRU.h)
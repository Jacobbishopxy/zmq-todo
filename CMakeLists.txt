# @file:	CMakeLists.txt
# @author:	Jacob Xie
# @date:	2024/12/06 14:56:49 Friday
# @brief:


cmake_minimum_required(VERSION 3.7)
project(zmq-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================================================================================
# Deps
# ================================================================================================

# spdlog
set(SPDLOG_VERSION 1.15.0)
set(SPDLOG_ROOT "/opt/qi-deps/spdlog-${SPDLOG_VERSION}")
message("Using ${SPDLOG_ROOT}")
include_directories("${SPDLOG_ROOT}/include")
link_directories("${SPDLOG_ROOT}/lib")

# zmq
set(ZMQ_VERSION 4.10.0)
find_package(cppzmq PATHS "/opt/qi-deps/cppzmq-${ZMQ_VERSION}")

# ================================================================================================
# Dirs
# ================================================================================================

add_subdirectory("zmq-cpp")

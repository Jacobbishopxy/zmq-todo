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

if (UNIX)
    # json
    set(JSON_VERSION 3.11.3)
    set(JSON_ROOT "/opt/qi-deps/json-${JSON_VERSION}")
    message("Using ${JSON_ROOT}")
    include_directories("${JSON_ROOT}/include")

    # spdlog
    set(SPDLOG_VERSION 1.15.0)
    set(SPDLOG_ROOT "/opt/qi-deps/spdlog-${SPDLOG_VERSION}")
    message("Using ${SPDLOG_ROOT}")
    include_directories("${SPDLOG_ROOT}/include")
    link_directories("${SPDLOG_ROOT}/lib")

    # zmq
    set(ZMQ_VERSION 4.10.0)
    find_package(cppzmq PATHS "/opt/qi-deps/cppzmq-${ZMQ_VERSION}")

    # uws
    set(UWS_VERSION 20.71.0)
    set(UWS_DIR "/opt/qi-deps/uWebSockets-${UWS_VERSION}")
    include_directories("${UWS_DIR}/include")
    link_directories("${UWS_DIR}/lib")
    set(LIB_UWEBSOCKETS uSockets z)
else ()
    #spdlog
    set(SPDLOG_VERSION 1.11.0)
    set(SPDLOG_ROOT "D:/Root/opt/qi-deps/spdlog-${SPDLOG_VERSION}")
    message("Using ${SPDLOG_ROOT}")
    include_directories("${SPDLOG_ROOT}/include")
    link_directories("${SPDLOG_ROOT}/lib")

    #zeromq
    include_directories(D:/Root/opt/qi-deps/zeromq-4.3.6/include)
    link_directories(D:/Root/opt/qi-deps/zeromq-4.3.6/lib)

    #cppzmq
    include_directories(D:/Root/opt/qi-deps/cppzmq-4.10.0/include)

endif ()
# ================================================================================================
# Dirs
# ================================================================================================

add_subdirectory("zmq-cpp")

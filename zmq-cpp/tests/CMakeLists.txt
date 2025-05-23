# @file:	CMakeLists.txt
# @author:	Jacob Xie
# @date:	2024/12/08 00:06:31 Sunday
# @brief:

cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(zmq-cpp LANGUAGES CXX VERSION 0.0.1)


# ================================================================================================
# deps
# ================================================================================================


find_package(cppzmq)

include_directories("include")

# ================================================================================================
# exec
# ================================================================================================
# combinations

add_executable(simple_rep simple_rep.cc)
target_link_libraries(simple_rep cppzmq)

add_executable(simple_req simple_req.cc)
target_link_libraries(simple_req cppzmq)

add_executable(simple_router_server simple_router_server.cc)
target_link_libraries(simple_router_server cppzmq)

add_executable(simple_router_client simple_router_client.cc)
target_link_libraries(simple_router_client cppzmq)

add_executable(simple_dealer_client simple_dealer_client.cc)
target_link_libraries(simple_dealer_client cppzmq)

add_executable(simple_dealer_server simple_dealer_server.cc)
target_link_libraries(simple_dealer_server cppzmq)

###################################################################################################
# pub_server -> [sub_client]

add_executable(simple_pub_server simple_pub_server.cc)
target_link_libraries(simple_pub_server cppzmq)

add_executable(simple_sub_client simple_sub_client.cc)
target_link_libraries(simple_sub_client cppzmq)

###################################################################################################
# [pub_client] -> sub_server

add_executable(simple_pub_client simple_pub_client.cc)
target_link_libraries(simple_pub_client cppzmq)

add_executable(simple_sub_server simple_sub_server.cc)
target_link_libraries(simple_sub_server cppzmq)


###################################################################################################
# proxy: req/dealer -> (router -> dealer) -> rep/router

add_executable(proxy_dealer_router proxy_dealer_router.cc)
target_link_libraries(proxy_dealer_router cppzmq)

add_executable(proxy_req_client proxy_req_client.cc)
target_link_libraries(proxy_req_client cppzmq)

add_executable(proxy_dealer_client proxy_dealer_client.cc)
target_link_libraries(proxy_dealer_client cppzmq)

add_executable(proxy_rep_worker proxy_rep_worker.cc)
target_link_libraries(proxy_rep_worker cppzmq)

add_executable(proxy_router_worker proxy_router_worker.cc)
target_link_libraries(proxy_router_worker cppzmq)

###################################################################################################
# proxy: pub -> (xsub -> xpub) -> sub

add_executable(proxy_pub_sub proxy_pub_sub.cc)
target_link_libraries(proxy_pub_sub cppzmq)

add_executable(proxy_pub proxy_pub.cc)
target_link_libraries(proxy_pub cppzmq)

add_executable(proxy_sub proxy_sub.cc)
target_link_libraries(proxy_sub cppzmq)

# ================================================================================================
# misc
# ================================================================================================

add_executable(simple_pair simple_pair.cc)
target_link_libraries(simple_pair cppzmq)

add_executable(xt_dealer xt_dealer.cc)
target_link_libraries(xt_dealer cppzmq)

add_executable(xt_dealer2 xt_dealer.cc)
target_link_libraries(xt_dealer2 cppzmq)

add_executable(xt_pub xt_pub.cc)
target_link_libraries(xt_pub cppzmq)

add_executable(xt_sub xt_sub.cc)
target_link_libraries(xt_sub cppzmq)

# socket monitor, recognize routing_id
add_executable(socket_monitor socket_monitor.cc)
target_link_libraries(socket_monitor cppzmq)


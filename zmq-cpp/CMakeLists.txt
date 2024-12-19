# @file:	CMakeLists.txt
# @author:	Jacob Xie
# @date:	2024/12/06 14:58:09 Friday
# @brief:

include_directories("include")

add_subdirectory("tests")
add_subdirectory("proxy_req_router")
add_subdirectory("majordomo")
add_subdirectory("protocol")

# @file:	CMakeLists.txt
# @author:	Jacob Xie
# @date:	2024/12/10 14:12:30 Tuesday
# @brief:


add_executable(todo_client adt.cc client.cc)
target_link_libraries(todo_client cppzmq)

add_executable(todo_middleware adt.cc middleware.cc)
target_link_libraries(todo_middleware cppzmq)

add_executable(todo_worker adt.cc worker.cc)
target_link_libraries(todo_worker cppzmq)



add_executable(x_client adt.cc client.cc client_main.cc)
target_link_libraries(x_client cppzmq)

add_executable(x_client_http adt.cc client_http.cc client_http_main.cc)
target_link_libraries(x_client_http cppzmq ${LIB_UWEBSOCKETS})

add_executable(x_broker adt.cc broker.cc)
target_link_libraries(x_broker cppzmq)

add_executable(x_worker adt.cc worker.cc)
target_link_libraries(x_worker cppzmq)


add_executable(m_client adt.cc client.cc)
target_link_libraries(m_client cppzmq)

add_executable(m_broker adt.cc broker.cc)
target_link_libraries(m_broker cppzmq)

add_executable(m_worker adt.cc worker.cc)
target_link_libraries(m_worker cppzmq)

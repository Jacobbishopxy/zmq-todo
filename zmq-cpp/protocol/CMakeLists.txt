
add_executable(xt_dealer2 xt_dealer.cc)
target_link_libraries(xt_dealer2 cppzmq)

add_executable(xt_pub xt_pub.cc)
target_link_libraries(xt_pub cppzmq)

add_executable(xt_sub xt_sub.cc)
target_link_libraries(xt_sub cppzmq)


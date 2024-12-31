if(UNIX)
	add_executable(test_mul_msg adt.cc test_mul_msg.cpp)
    target_link_libraries(test_mul_msg cppzmq)
else()
    add_executable(test_mul_msg adt.cc test_mul_msg.cpp)
    target_link_libraries(test_mul_msg PRIVATE unity libzmq-v143-mt-gd-4_3_6 libzmq-v143-mt-sgd-4_3_6)
    add_custom_command(TARGET test_mul_msg POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/Root/opt/qi-deps/zeromq-4.3.6/bin/libzmq-v143-mt-gd-4_3_6.dll"
        $<TARGET_FILE_DIR:test_mul_msg>
    )
endif()

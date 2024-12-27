if (UNIX)
    add_executable(x_client adt.cc client.cc client_main.cc)
    target_link_libraries(x_client cppzmq)

    add_executable(x_client_http adt.cc client_http.cc client_http_main.cc)
    target_link_libraries(x_client_http cppzmq ${LIB_UWEBSOCKETS})

    add_executable(x_broker adt.cc broker.cc)
    target_link_libraries(x_broker cppzmq)

    add_executable(x_worker adt.cc worker.cc)
    target_link_libraries(x_worker cppzmq)
else ()
    add_executable(x_worker adt.cc worker.cc)
    target_link_libraries(x_worker PRIVATE unity libzmq-v143-mt-gd-4_3_6 libzmq-v143-mt-sgd-4_3_6)
    add_custom_command(TARGET x_worker POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Root/opt/qi-deps/zeromq-4.3.6/bin/libzmq-v143-mt-gd-4_3_6.dll"
            $<TARGET_FILE_DIR:x_worker>
    )
endif ()
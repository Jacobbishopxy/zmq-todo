/**
 * @file:	proxy_pub_sub.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 21:16:32 Sunday
 * @brief:
 **/

#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    // zmq context
    zmq::context_t context(1);

    // Frontend SUB
    zmq::socket_t frontend(context, zmq::socket_type::xsub);
    frontend.bind(FrontendEP);

    // Backend PUB
    zmq::socket_t backend(context, zmq::socket_type::xpub);
    backend.bind(BackendEP);

    std::cout << "PUB-SUB proxy started: frontend on " << FrontendEP
              << ", backend on " << BackendEP << "..." << std::endl;

    // Start the proxy
    zmq::proxy(frontend, backend);

    return 0;
}

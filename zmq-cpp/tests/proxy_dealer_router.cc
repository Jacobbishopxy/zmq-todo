/**
 * @file:	proxy_dealer_router.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 20:22:57 Sunday
 * @brief:
 **/

#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    zmq::context_t context(1);

    // Frontend ROUTER socket for clients
    zmq::socket_t frontend(context, zmq::socket_type::router);
    frontend.bind(FrontendEP);

    // Backend DEALER socket for workers
    zmq::socket_t backend(context, zmq::socket_type::dealer);
    backend.bind(BackendEP);

    std::cout << "Proxy started: frontend on " << FrontendEP << ", backend on "
              << BackendEP << std::endl;

    // Start the proxy
    zmq::proxy(frontend, backend);

    return 0;
}

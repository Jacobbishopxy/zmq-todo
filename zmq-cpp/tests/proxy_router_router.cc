/**
 * @file:	proxy_router_router.cc
 * @author:	Jacob Xie
 * @date:	2025/07/23 18:58:33 Wednesday
 * @brief:
 **/

#include <iostream>

#include "common.hpp"

/*

dealer -> router:
[dealer_id][msg]...

*/

int main(int argc, char** argv)
{
    zmq::context_t context(1);

    // Frontend ROUTER socket for clients
    zmq::socket_t frontend(context, zmq::socket_type::router);
    frontend.bind(FrontendEP);

    // Backend ROUTER socket for workers
    zmq::socket_t backend(context, zmq::socket_type::router);
    backend.bind(BackendEP);

    // Monitor
    CustomMonitor frontend_monitor;
    frontend_monitor.start(frontend, InprocMonitorF);
    CustomMonitor backend_monitor;
    backend_monitor.start(backend, InprocMonitorB);

    std::cout << "Proxy started: frontend on " << FrontendEP << ", backend on "
              << BackendEP << std::endl;

    // Start the proxy
    // zmq::proxy(frontend, backend);

    zmq::pollitem_t items[] = {
        {frontend.handle(), 0, ZMQ_POLLIN | ZMQ_POLLERR, 0},
        {backend.handle(), 0, ZMQ_POLLIN | ZMQ_POLLERR, 0},
    };

    while (true)
    {
        zmq::poll(items, 2);

        std::vector<zmq::message_t> mm;

        // frontend
        if (items[0].revents)
        {
            auto recv_r = zmq::recv_multipart(frontend, std::back_inserter(mm));
            std::swap(mm[0], mm[1]);
            zmq::send_multipart(backend, mm);
        }

        // backend
        if (items[1].revents)
        {
            auto recv_r = zmq::recv_multipart(backend, std::back_inserter(mm));
            std::swap(mm[0], mm[1]);
            zmq::send_multipart(frontend, mm);
        }
    }

    frontend_monitor.abort();
    backend_monitor.abort();

    return 0;
}

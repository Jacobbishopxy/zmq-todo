/**
 * @file:	proxy_dealer_router.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 20:22:57 Sunday
 * @brief:
 **/

#include <iostream>
#include <thread>

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

    // Monitor
    CustomMonitor frontend_monitor;
    frontend_monitor.init(frontend, InprocMonitorF, ZMQ_EVENT_ALL);
    auto fml = [&frontend_monitor]
    {
        while (frontend_monitor.check_event(-1))
        {
        }
    };
    auto frontend_monitor_t = std::thread(fml);
    frontend_monitor_t.detach();
    CustomMonitor backend_monitor;
    backend_monitor.init(backend, InprocMonitorB, ZMQ_EVENT_ALL);
    auto bml = [&backend_monitor]
    {
        while (backend_monitor.check_event(-1))
        {
        }
    };
    auto backend_monitor_t = std::thread(bml);
    backend_monitor_t.detach();

    std::cout << "Proxy started: frontend on " << FrontendEP << ", backend on "
              << BackendEP << std::endl;

    // Start the proxy
    zmq::proxy(frontend, backend);

    frontend_monitor.abort();
    backend_monitor.abort();

    return 0;
}

/**
 * @file:	middleware.cc
 * @author:	Jacob Xie
 * @date:	2024/12/11 13:54:59 Wednesday
 * @brief:
 **/

#include "middleware.h"

#include <sstream>
#include <zmq.hpp>

#include "common.hpp"

TodoMiddleware::TodoMiddleware(const std::string& frontendAddress, const std::string& backendAddress)
    : m_context(std::make_shared<zmq::context_t>(1)),
      m_frontend(std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::router)),
      m_backend(std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::dealer))
{
    m_frontend->bind(frontendAddress);
    m_backend->bind(backendAddress);

    // Setup monitors for both sockets
    this->setupMonitor(m_frontend, "Frontend");
    this->setupMonitor(m_backend, "Backend");
}

TodoMiddleware::~TodoMiddleware()
{
    if (m_frontend_monitor_t.joinable())
    {
        m_frontend_monitor_t.join();
    }
    if (m_backend_monitor_t.joinable())
    {
        m_backend_monitor_t.join();
    }
}

void TodoMiddleware::run()
{
    zmq::proxy(*m_frontend, *m_backend);
}

void TodoMiddleware::setupMonitor(const std::shared_ptr<zmq::socket_t> socket, const std::string& socketName)
{
    // Generate a unique monitor endpoint
    std::ostringstream monitorAddress;
    monitorAddress << "inproc://" << socketName << "_monitor";

    // Launch a thread to handle monitor events
    if (socketName == "Frontend")
    {
        this->m_frontend_monitor = std::make_shared<CustomMonitor>();
        m_frontend_monitor->init(*this->m_frontend, monitorAddress.str());
        auto l = [this]
        {
            while (this->m_frontend_monitor->check_event(-1))
            {
            }
        };
        this->m_frontend_monitor_t = std::thread(l);
        this->m_frontend_monitor_t.detach();
    }
    else if (socketName == "Backend")
    {
        this->m_backend_monitor = std::make_shared<CustomMonitor>();
        m_backend_monitor->init(*this->m_backend, monitorAddress.str());
        auto l = [this]
        {
            while (this->m_backend_monitor->check_event(-1))
            {
            }
        };
        this->m_backend_monitor_t = std::thread(l);
        this->m_backend_monitor_t.detach();
    }
}

int main(int argc, char** argv)
{
    auto m = TodoMiddleware(FrontendEP, BackendEP);

    std::cout << "Todo middleware: frontend on "
              << FrontendEP << ", backend on "
              << BackendEP << std::endl;

    m.run();

    return 0;
}

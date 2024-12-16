/**
 * @file:	middleware.h
 * @author:	Jacob Xie
 * @date:	2024/12/10 16:17:31 Tuesday
 * @brief:
 **/

#ifndef __MIDDLEWARE__H__
#define __MIDDLEWARE__H__

#include <string>
#include <thread>
#include <zmq.hpp>

#include "common.hpp"

class TodoMiddleware
{
public:
    TodoMiddleware(const std::string& frontendAddress, const std::string& backendAddress);
    ~TodoMiddleware();

    void run();

private:
    void setupMonitor(const std::shared_ptr<zmq::socket_t> socket, const std::string& socketName);

    std::shared_ptr<zmq::context_t> m_context;
    std::shared_ptr<zmq::socket_t> m_frontend;
    std::shared_ptr<zmq::socket_t> m_backend;

    std::shared_ptr<CustomMonitor> m_frontend_monitor;
    std::shared_ptr<CustomMonitor> m_backend_monitor;

    std::thread m_frontend_monitor_t;
    std::thread m_backend_monitor_t;
};

#endif  //!__MIDDLEWARE__H__

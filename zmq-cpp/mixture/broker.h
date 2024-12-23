/**
 * @file:	broker.h
 * @author:	Jacob Xie
 * @date:	2024/12/16 18:52:48 Monday
 * @brief:
 **/

#ifndef __BROKER__H__
#define __BROKER__H__

#include <thread>
#include <zmq.hpp>

// #include "common.hpp"

class TodoBroker
{
public:
    TodoBroker(const std::string& router_address, const std::string& sub_address, const std::string& pub_address);
    ~TodoBroker();

    void run();

private:
    std::string m_router_address;
    std::string m_pub_address;
    std::string m_sub_address;
    zmq::context_t m_context;
    // CustomMonitor m_router_monitor;
    // CustomMonitor m_pub_monitor;
    // CustomMonitor m_sub_monitor;
    std::thread m_router_t;
    std::thread m_proxy_t;
    // std::thread m_monitor_t;

    // void setupMonitor();
};

#endif  //!__BROKER__H__

/**
 * @file:	broker.h
 * @author:	Jacob Xie
 * @date:	2024/12/16 18:52:48 Monday
 * @brief:
 **/

#ifndef __BROKER__H__
#define __BROKER__H__

#include <zmq.hpp>

#include "common.hpp"

class TodoBroker
{
public:
    TodoBroker(const std::string& bind_address);
    ~TodoBroker();

    void run();

private:
    void setupMonitor();

    std::shared_ptr<zmq::context_t> m_context;
    std::shared_ptr<zmq::socket_t> m_router;
    std::shared_ptr<CustomMonitor> m_monitor;
    std::thread m_monitor_t;
};

#endif  //!__BROKER__H__

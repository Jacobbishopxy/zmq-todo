/**
 * @file:	common.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/08 00:19:29 Sunday
 * @brief:
 **/

#ifndef __COMMON__H__
#define __COMMON__H__

#include <iostream>
#include <string>
#include <zmq.hpp>

const std::string EP = "tcp://127.0.0.1:5555";

const std::string FrontendEP = "tcp://127.0.0.1:5565";
const std::string BackendEP = "tcp://127.0.0.1:5566";

const std::string PubSubTopic = "TA";

const std::string InprocMonitor = "inproc://monitor";
const std::string InprocMonitorF = "inproc://frontend_monitor";
const std::string InprocMonitorB = "inproc://backend_monitor";

class CustomMonitor : public zmq::monitor_t
{
public:
    void on_event_connected(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[CONNECTED] Event on address: " << addr << std::endl;
    }

    void on_event_disconnected(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[DISCONNECTED] Event on address: " << addr << std::endl;
    }

    void on_event_bind_failed(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[BIND FAILED] Address: " << addr << std::endl;
    }

    void on_event_listening(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[LISTENING] Address: " << addr << std::endl;
    }

    void on_event_accepted(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[EVENT ACCEPTED] Address: " << addr << std::endl;
    }
};

#endif  //!__COMMON__H__

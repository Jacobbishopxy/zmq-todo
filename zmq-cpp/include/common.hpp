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
#include <thread>
#include <zmq.hpp>

const std::string EP = "tcp://127.0.0.1:5555";
const std::string EP2 = "tcp://127.0.0.1:5556";

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

    void start(zmq::socket_t& socket, std::string const& addr)
    {
        this->init(socket, addr);
        auto l = [this]
        {
            while (this->check_event(-1))
            {
            }
        };
        auto t = std::thread(l);
        t.detach();
    }
};

inline void recv_multipart(zmq::socket_t& socket)
{
    zmq::message_t message;

    // Receive the first part
    while (true)
    {
        auto recv_result = socket.recv(message, zmq::recv_flags::none);
        std::cout << "> " << message.to_string() << std::endl;

        // Check if this is the last part
        if (!message.more())
        {
            std::cout << std::endl;
            break;  // Exit the loop if no more parts
        }
    }
}

#endif  //!__COMMON__H__

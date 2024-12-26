/**
 * @file:	broker.cc
 * @author:	Jacob Xie
 * @date:	2024/12/17 10:53:01 Tuesday
 * @brief:
 **/

#include "broker.h"

#include <iostream>

#include "common.hpp"

TodoBroker::TodoBroker(const std::string& router_address, const std::string& sub_address, const std::string& pub_address)
{
    m_context = zmq::context_t(1);
    m_router_address = router_address;
    m_sub_address = sub_address;
    m_pub_address = pub_address;
}

TodoBroker::~TodoBroker()
{
    if (m_router_t.joinable())
        m_router_t.join();
    if (m_proxy_t.joinable())
        m_proxy_t.join();
}

void TodoBroker::run()
{
    // router thread
    auto router_t = [this]()
    {
        zmq::socket_t router(this->m_context, zmq::socket_type::router);
        router.bind(this->m_router_address);

        while (true)
        {
            zmq::message_t from_id;
            zmq::message_t to_id;
            zmq::message_t action;
            zmq::message_t payload;

            auto f = router.recv(from_id, zmq::recv_flags::none);
            auto t = router.recv(to_id, zmq::recv_flags::none);
            auto a = router.recv(action, zmq::recv_flags::none);
            auto p = router.recv(payload, zmq::recv_flags::none);
            std::cout << "TodoBroker recv: [from " << from_id.to_string() << "] [to " << to_id.to_string() << "]" << std::endl;

            router.send(to_id, zmq::send_flags::sndmore);
            router.send(from_id, zmq::send_flags::sndmore);
            router.send(action, zmq::send_flags::sndmore);
            router.send(payload, zmq::send_flags::none);
        }
    };
    this->m_router_t = std::thread(router_t);

    // proxy thread
    auto proxy_t = [this]()
    {
        zmq::socket_t sub(this->m_context, zmq::socket_type::xsub);
        sub.bind(this->m_sub_address);

        zmq::socket_t pub(this->m_context, zmq::socket_type::xpub);
        pub.bind(this->m_pub_address);

        zmq::proxy(sub, pub);
    };
    this->m_proxy_t = std::thread(proxy_t);
}

int main(int argc, char** argv)
{
    auto b = TodoBroker(EP, FrontendEP, BackendEP);

    std::cout << "Todo broker: " << std::endl;
    std::cout << "Router bound on: " << EP << "..." << std::endl;
    std::cout << "Proxy frontend sub bound on: " << FrontendEP << "..." << std::endl;
    std::cout << "Proxy backend pub bound on: " << BackendEP << "..." << std::endl;

    b.run();

    return EXIT_SUCCESS;
}

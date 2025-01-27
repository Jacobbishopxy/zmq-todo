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
    // monitor
    m_router_monitor = std::make_shared<CustomMonitor>();
    m_pub_monitor = std::make_shared<CustomMonitor>();
    m_sub_monitor = std::make_shared<CustomMonitor>();

    std::atomic_bool router_ready = false;
    std::atomic_bool proxy_ready = false;

    // router thread
    auto router_t = [&, this]() mutable
    {
        std::cout << "Starting router_t..." << std::endl;

        zmq::socket_t router(this->m_context, zmq::socket_type::router);
        router.bind(this->m_router_address);
        // bind router to monitor
        m_router_monitor->init(router, "inproc://mixture_router_monitor");

        // notify router_monitor ready
        router_ready.store(true);

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
    auto proxy_t = [&, this]()
    {
        std::cout << "Starting proxy_t..." << std::endl;

        zmq::socket_t sub(this->m_context, zmq::socket_type::xsub);
        sub.bind(this->m_sub_address);
        m_sub_monitor->init(sub, "inproc://mixture_sub_monitor");

        zmq::socket_t pub(this->m_context, zmq::socket_type::xpub);
        pub.bind(this->m_pub_address);
        m_pub_monitor->init(pub, "inproc://mixture_pub_monitor");

        // notify pub/sub monitor ready
        proxy_ready.store(true);

        // start pub/sub proxy
        zmq::proxy(sub, pub);
    };
    this->m_proxy_t = std::thread(proxy_t);

    auto monitor_t = [&, this]() mutable
    {
        while (!proxy_ready.load() || !router_ready.load())
            std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "Starting monitor_t..." << std::endl;

        zmq::pollitem_t items[] = {
            {m_router_monitor->handle(), 0, ZMQ_POLLIN | ZMQ_POLLERR, 0},
            {m_pub_monitor->handle(), 0, ZMQ_POLLIN | ZMQ_POLLERR, 0},
            {m_sub_monitor->handle(), 0, ZMQ_POLLIN | ZMQ_POLLERR, 0},
        };

        while (true)
        {
            zmq::poll(items, 3);

            if (items[0].revents)
                m_router_monitor->process(items[0].revents);

            if (items[1].revents)
                m_pub_monitor->process(items[1].revents);

            if (items[2].revents)
                m_sub_monitor->process(items[2].revents);
        }
    };
    this->m_monitor_t = std::thread(monitor_t);
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

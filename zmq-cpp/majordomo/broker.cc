/**
 * @file:	broker.cc
 * @author:	Jacob Xie
 * @date:	2024/12/17 10:53:01 Tuesday
 * @brief:
 **/

#include "broker.h"

TodoBroker::TodoBroker(const std::string& bind_address)
    : m_context(std::make_shared<zmq::context_t>(1)),
      m_router(std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::router))
{
    m_router->bind(bind_address);

    this->setupMonitor();
}

TodoBroker::~TodoBroker()
{
    this->m_router->close();
}

void TodoBroker::run()
{
    while (true)
    {
        zmq::message_t from_id;
        zmq::message_t to_id;
        zmq::message_t action;
        zmq::message_t payload;

        auto f = m_router->recv(from_id, zmq::recv_flags::none);
        auto t = m_router->recv(to_id, zmq::recv_flags::none);
        auto a = m_router->recv(action, zmq::recv_flags::none);
        auto p = m_router->recv(payload, zmq::recv_flags::none);
        std::cout << "TodoBroker recv: [from " << from_id.to_string() << "] [to " << to_id.to_string() << "]" << std::endl;

        // std::cout << "- " << from_id.to_string() << std::endl;
        // std::cout << "- " << to_id.to_string() << std::endl;
        // std::cout << "- " << action.to_string() << std::endl;
        // std::cout << "- " << payload.to_string() << std::endl;

        m_router->send(to_id, zmq::send_flags::sndmore);
        m_router->send(from_id, zmq::send_flags::sndmore);
        m_router->send(action, zmq::send_flags::sndmore);
        m_router->send(payload, zmq::send_flags::none);
        std::cout << "TodoBroker redirect: [from " << to_id.to_string() << "] [to " << from_id.to_string() << "]" << std::endl;
    }
}

void TodoBroker::setupMonitor()
{
    const std::string monitor_addr = "inproc://majordomo_broker_monitor";

    this->m_monitor = std::make_shared<CustomMonitor>();
    this->m_monitor->init(*this->m_router, monitor_addr);

    auto lbd = [this]
    {
        while (this->m_monitor->check_event(-1))
        {
        }
    };
    this->m_monitor_t = std::thread(lbd);
    this->m_monitor_t.detach();
}

int main(int argc, char** argv)
{
    auto m = TodoBroker(EP);

    std::cout << "Todo broker bound on: " << EP << "..." << std::endl;

    m.run();

    return EXIT_SUCCESS;
}

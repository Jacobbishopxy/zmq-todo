/**
 * @file:	PubManager.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/19 14:14:06 Thursday
 * @brief:
 **/

#ifndef __PUBMANAGER__H__
#define __PUBMANAGER__H__

#include <thread>
#include <zmq_addon.hpp>

#include "ProtoMsg.h"

template <IsProtoMsgO O>
class PubManager
{
public:
    PubManager(
        zmq::context_t& context,
        const std::string& connect_addr)
        : m_address(connect_addr),
          m_context(context),
          m_main_pair(m_context, zmq::socket_type::pair),
          m_running(false)
    {
        m_inproc_addr = "inproc://dealer_manager";
        m_is_bind = false;
    }

    ~PubManager()
    {
        this->stop();
    }

    void set_inproc_address(const std::string& address)
    {
        m_inproc_addr = address;
    }

    void make_bind()
    {
        m_is_bind = true;
    }

    void start()
    {
        if (m_running.load())
            return;

        m_running.store(true);
        m_main_pair.bind(m_inproc_addr);
        m_pub_thread = std::thread(&PubManager<O>::pubLoop, this);
    }

    void stop()
    {
        if (!m_running.load())
            return;

        m_running.store(false);
        m_main_pair.send(zmq::str_buffer("STOP!"), zmq::send_flags::none);

        if (m_pub_thread.joinable())
            m_pub_thread.join();
    }

    void setTopic(const std::string& topic)
    {
        m_topic = topic;
    }

    void publishMessage(const O& message)
    {
        auto zmq_messages = message.toZmq();
        zmq::message_t t(m_topic.size());
        memcpy(t.data(), m_topic.data(), m_topic.size());
        m_main_pair.send(t, zmq::send_flags::sndmore);
        zmq::send_multipart(m_main_pair, zmq_messages);
    }

private:
    std::string m_inproc_addr;
    std::string m_address;
    bool m_is_bind;
    zmq::context_t& m_context;
    zmq::socket_t m_main_pair;
    std::atomic<bool> m_running;
    std::string m_topic;
    std::thread m_pub_thread;

    void pubLoop()
    {
        zmq::socket_t pub(m_context, zmq::socket_type::pub);
        if (m_is_bind)
            pub.bind(m_address);
        else
            pub.connect(m_address);

        zmq::socket_t pair(m_context, zmq::socket_type::pair);
        pair.connect(m_inproc_addr);

        zmq::pollitem_t items[] = {
            {pair.handle(), 0, ZMQ_POLLIN, 0},
        };

        while (m_running.load())
        {
            zmq::poll(items, 1);

            if (items[0].revents & ZMQ_POLLIN)
            {
                // PAIR receives a message from m_main_pair to forward to PUB
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(pair, std::back_inserter(mm));

                // Quit this event loop
                if (mm.size() == 1)
                {
                    std::string sig(static_cast<char*>(mm[0].data()), mm[0].size());
                    if (sig == "STOP!\0")
                        break;
                }

                zmq::send_multipart(pub, mm);
            }
        }
    }
};

#endif  //!__PUBMANAGER__H__

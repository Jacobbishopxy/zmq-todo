/**
 * @file:	SubManager.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/19 14:35:50 Thursday
 * @brief:
 **/

#ifndef __SUBMANAGER__H__
#define __SUBMANAGER__H__

#include <thread>
#include <unordered_set>
#include <zmq_addon.hpp>

#include "ProtoMsg.h"

template <IsProtoMsgI I>
class SubManager
{
public:
    SubManager(
        zmq::context_t& context,
        const std::string& address)
        : m_connect_addr(address),
          m_context(context),
          m_main_pair(m_context, zmq::socket_type::pair),
          m_running(false)
    {
        m_inproc_addr = "inproc://sub_manager";
        m_is_bind = false;
    }

    ~SubManager()
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
        m_sub_thread = std::thread(&SubManager<I>::subLoop, this);
    }

    void stop()
    {
        if (!m_running.load())
            return;

        m_running.store(false);
        m_main_pair.send(zmq::str_buffer("STOP!"), zmq::send_flags::none);

        if (m_sub_thread.joinable())
            m_sub_thread.join();
    }

    void subscribe(const std::string& topic)
    {
        m_subscribed_topics.insert(topic);

        if (m_running.load())
        {
            m_main_pair.send(zmq::str_buffer("S!"), zmq::send_flags::sndmore);
            zmq::message_t topicFrame;
            memcpy(topicFrame.data(), topic.data(), topic.size());
            m_main_pair.send(topicFrame, zmq::send_flags::none);
        }
    }

    void unsubscribe(const std::string& topic)
    {
        m_subscribed_topics.erase(topic);

        if (m_running.load())
        {
            m_main_pair.send(zmq::str_buffer("U!"), zmq::send_flags::sndmore);
            zmq::message_t topicFrame;
            memcpy(topicFrame.data(), topic.data(), topic.size());
            m_main_pair.send(topicFrame, zmq::send_flags::none);
        }
    }

    std::tuple<std::string, I> recvMessage()
    {
        std::vector<zmq::message_t> mm;
        auto recv_r = zmq::recv_multipart(m_main_pair, std::back_inserter(mm));

        auto& fst = mm.front();
        std::string topic(static_cast<char*>(fst.data()), fst.size());

        mm.erase(mm.begin());

        return std::make_tuple(topic, I(mm));
    }

private:
    std::string m_inproc_addr;
    std::string m_connect_addr;
    bool m_is_bind;
    zmq::context_t& m_context;
    zmq::socket_t m_main_pair;
    std::atomic<bool> m_running;
    std::unordered_set<std::string> m_subscribed_topics;
    std::thread m_sub_thread;

    void subLoop()
    {
        zmq::socket_t sub(m_context, zmq::socket_type::sub);
        if (m_is_bind)
            sub.bind(m_connect_addr);
        else
            sub.connect(m_connect_addr);

        // Set subscription filter
        for (const auto& t : m_subscribed_topics)
        {
            sub.set(zmq::sockopt::subscribe, t);
        }

        zmq::socket_t pair(m_context, zmq::socket_type::pair);
        pair.connect(m_inproc_addr);

        zmq::pollitem_t items[] = {
            {pair.handle(), 0, ZMQ_POLLIN, 0},
            {sub.handle(), 0, ZMQ_POLLIN, 0},
        };

        while (m_running.load())
        {
            zmq::poll(items, 2);

            if (items[0].revents & ZMQ_POLLIN)
            {
                // PAIR receives termination signal
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(pair, std::back_inserter(mm));

                if (mm.size() == 1)
                {
                    std::string sig(static_cast<char*>(mm[0].data()), mm[0].size());
                    if (sig == "STOP!\0")
                        break;
                }

                if (mm.size() == 2)
                {
                    std::string sig(static_cast<char*>(mm[0].data()), mm[0].size());
                    if (sig == "S!\0")
                    {
                        std::string t(static_cast<char*>(mm[1].data()), mm[1].size());
                        sub.set(zmq::sockopt::subscribe, t);
                    }

                    if (sig == "U!\0")
                    {
                        std::string t(static_cast<char*>(mm[1].data()), mm[1].size());
                        sub.set(zmq::sockopt::unsubscribe, t);
                    }
                }
            }

            if (items[1].revents & ZMQ_POLLIN)
            {
                // SUB receives a message, forward it to PAIR
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(sub, std::back_inserter(mm));
                zmq::send_multipart(pair, mm);
            }
        }
    }
};

#endif  //!__SUBMANAGER__H__

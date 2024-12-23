/**
 * @file:	DealerManager.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/18 15:29:41 Wednesday
 * @brief:
 **/

#ifndef __DEALERMANAGER__H__
#define __DEALERMANAGER__H__

#include <thread>
#include <zmq_addon.hpp>

#include "ProtoMsg.h"

template <IsProtoMsgI I, IsProtoMsgO O>
class DealerManager
{
public:
    DealerManager(
        zmq::context_t& context,
        const std::string& address)
        : m_address(address),
          m_context(context),
          m_main_pair(m_context, zmq::socket_type::pair),
          m_running(false)
    {
        m_inproc_addr = "inproc://dealer_manager";
        m_is_bind = false;
    }

    ~DealerManager()
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
        m_dealer_thread = std::thread(&DealerManager<I, O>::dealerLoop, this);
    }

    void stop()
    {
        if (!m_running.load())
            return;

        m_running.store(false);
        m_main_pair.send(zmq::str_buffer("STOP!"), zmq::send_flags::none);

        if (m_dealer_thread.joinable())
            m_dealer_thread.join();
    }

    void sendMessage(const O& message)
    {
        auto zmq_messages = message.toZmq();
        zmq::send_multipart(m_main_pair, zmq_messages);
    }

    I recvMessage()
    {
        std::vector<zmq::message_t> mm;
        auto recv_r = zmq::recv_multipart(m_main_pair, std::back_inserter(mm));

        return I(mm);
    };

private:
    std::string m_inproc_addr;
    std::string m_address;
    bool m_is_bind;
    zmq::context_t& m_context;
    zmq::socket_t m_main_pair;
    std::atomic<bool> m_running;
    std::thread m_dealer_thread;

    void dealerLoop()
    {
        zmq::socket_t dealer(m_context, zmq::socket_type::dealer);
        if (m_is_bind)
            dealer.bind(m_address);
        else
            dealer.connect(m_address);

        zmq::socket_t pair(m_context, zmq::socket_type::pair);
        pair.connect(m_inproc_addr);

        zmq::pollitem_t items[] = {
            {pair.handle(), 0, ZMQ_POLLIN, 0},
            {dealer.handle(), 0, ZMQ_POLLIN, 0},
        };

        while (m_running.load())
        {
            // Poll indefinitely
            zmq::poll(items, 2);

            if (items[0].revents & ZMQ_POLLIN)
            {
                // PAIR receives msg from m_main_pair, forward to DEALER
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(pair, std::back_inserter(mm));

                // quit this event loop
                if (mm.size() == 1)
                {
                    std::string sig(static_cast<char*>(mm[0].data()), mm[0].size());
                    if (sig == "STOP!\0")
                        break;
                }

                zmq::send_multipart(dealer, mm);
            }

            if (items[1].revents & ZMQ_POLLIN)
            {
                // DEADER receives msg from external, forword to PAIR
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(dealer, std::back_inserter(mm));
                zmq::send_multipart(pair, mm);
            }
        }
    }
};

#endif  //!__DEALERMANAGER__H__

/**
 * @file:	ClientService.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/20 14:24:34 Friday
 * @brief:
 **/

#ifndef __DEALERSERVICE__H__
#define __DEALERSERVICE__H__

#include <stdexcept>
#include <thread>
#include <zmq_addon.hpp>

#include "adt.h"
#include "common.hpp"

struct IReceiver
{
    virtual void recvSubMessage(const TodoStreamResponse& message) = 0;
};

template <typename T>
concept IsReceiver = std::derived_from<T, IReceiver>;

template <class ReceiverType, typename BuilderPatternReturnType>
struct ClientBuilder
{
    ClientBuilder() = default;
    ClientBuilder(ClientBuilder&&) noexcept = default;
    ClientBuilder& operator=(ClientBuilder&&) noexcept = default;

    // disallow copy
    ClientBuilder(const ClientBuilder&) = delete;
    ClientBuilder& operator=(const ClientBuilder&) = delete;

    // register ReceiverType instance
    BuilderPatternReturnType&& registerReceiver(ReceiverType& spi)
    {
        m_receiver = std::move(spi);
        return std::move(static_cast<BuilderPatternReturnType&&>(*this));
    }

protected:
    std::optional<ReceiverType> m_receiver;  // Holds the ReceiverType instance
};

template <IsReceiver T>
class ClientService : public ClientBuilder<T, ClientService<T>>
{
public:
    ClientService(
        const std::string& client_id,
        const std::string& router_address,
        const std::string& sub_topic,
        const std::string& sub_address)

    {
        m_router_address = router_address;
        m_client_id = client_id;
        m_sub_topic = sub_topic;
        m_sub_address = sub_address;
        m_running = false;
        m_context = zmq::context_t(1);
        m_main_pair_i = zmq::socket_t(m_context, zmq::socket_type::pair);
        m_main_pair_o = zmq::socket_t(m_context, zmq::socket_type::pair);
        m_inproc_addr_i = "inproc://client_service_i";
        m_inproc_addr_o = "inproc://client_service_o";
    }

    ~ClientService()
    {
        this->stop();
    }

    // ================================================================================================
    // public API
    // ================================================================================================

    void start()
    {
        if (m_running.load())
            return;

        m_running.store(true);
        // send timeout
        m_main_pair_i.set(zmq::sockopt::sndtimeo, 2000);
        m_main_pair_o.set(zmq::sockopt::sndtimeo, 2000);
        // recv timeout
        m_main_pair_i.set(zmq::sockopt::rcvtimeo, 2000);
        m_main_pair_o.set(zmq::sockopt::rcvtimeo, 2000);
        // bind inproc address
        m_main_pair_i.bind(m_inproc_addr_i);
        m_main_pair_o.bind(m_inproc_addr_o);

        if (this->m_receiver.has_value())
        {
            // move m_receiver into eventLoop thread
            T receiver = std::move(this->m_receiver.value());
            this->m_receiver.reset();

            // move m_receiver into lambda
            auto t = [this, receiver = std::move(receiver)]() mutable
            {
                // move m_receiver into eventLoop
                this->eventLoop(std::move(receiver));
            };
            m_event_thread = std::thread(t);
        }
        else
        {
            throw std::runtime_error("Receiver is not initialized!");
        }
    }

    void stop()
    {
        if (!m_running.load())
            return;

        m_running.store(false);
        m_main_pair_o.send(zmq::str_buffer("STOP!"), zmq::send_flags::none);

        if (m_event_thread.joinable())
            m_event_thread.join();
    }

    void sendMessage(const TodoRequest& message)
    {
        // send message via m_main_pair_o to dealer (cross threads)
        auto zmq_messages = message.toZmq();
        zmq::send_multipart(m_main_pair_o, zmq_messages);
    }

    std::optional<TodoResponse> recvMessage()
    {
        // recv message via m_main_pair_i from dealer (cross threads)
        std::vector<zmq::message_t> mm;
        auto recv_r = zmq::recv_multipart(m_main_pair_i, std::back_inserter(mm));
        // check message
        if (mm.size() <= 1)
        {
            std::cerr << "Invalid message from ROUTER" << std::endl;
            return std::nullopt;
        }
        TodoResponse rsp(std::move(mm));
        return rsp;
    }

    // ================================================================================================
    // private
    // ================================================================================================

private:
    std::string m_inproc_addr_i;
    std::string m_inproc_addr_o;
    std::string m_client_id;
    std::string m_router_address;
    std::string m_sub_topic;
    std::string m_sub_address;
    zmq::context_t m_context;
    std::atomic<bool> m_running;
    zmq::socket_t m_main_pair_i;
    zmq::socket_t m_main_pair_o;
    std::thread m_event_thread;

    void eventLoop(T receiver)
    {
        // handle: DEALER -> m_main_pair
        zmq::socket_t pair_i(m_context, zmq::socket_type::pair);
        pair_i.connect(m_inproc_addr_i);
        // handle: m_main_pair -> DEALER
        zmq::socket_t pair_o(m_context, zmq::socket_type::pair);
        pair_o.connect(m_inproc_addr_o);

        zmq::socket_t dealer(m_context, zmq::socket_type::dealer);
        // dealer_id
        dealer.set(zmq::sockopt::routing_id, m_client_id);
        // send timeout
        dealer.set(zmq::sockopt::sndtimeo, 2000);
        // recv timeout
        dealer.set(zmq::sockopt::rcvtimeo, 2000);
        // connect
        dealer.connect(m_router_address);

        zmq::socket_t sub(m_context, zmq::socket_type::sub);
        // topic
        sub.set(zmq::sockopt::subscribe, m_sub_topic);
        // connect
        sub.connect(m_sub_address);

        // register events
        zmq::pollitem_t items[] = {
            {pair_o.handle(), 0, ZMQ_POLLIN, 0},
            {dealer.handle(), 0, ZMQ_POLLIN, 0},
            {sub.handle(), 0, ZMQ_POLLIN, 0},
        };

        // start the event loop
        while (m_running.load())
        {
            zmq::poll(items, 3);

            // PAIR(o) receives msg from m_main_pair_o
            if (items[0].revents && ZMQ_POLLIN)
            {
                // PAIR forward to external ROUTER
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(pair_o, std::back_inserter(mm));

                // quit this event loop
                if (mm.size() == 1)
                {
                    std::string sig = messageToString(mm[0]);
                    if (sig == "STOP!\0")
                        break;
                }

                zmq::send_multipart(dealer, mm);
            }

            // DEALER receives msg from external ROUTER
            if (items[1].revents && ZMQ_POLLIN)
            {
                // DEALER recv msg
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(dealer, std::back_inserter(mm));
                // forward to m_main_pair_i
                zmq::send_multipart(pair_i, mm);
            }

            // SUB receives msg from external PUB
            if (items[2].revents && ZMQ_POLLIN)
            {
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(sub, std::back_inserter(mm));
                // check message
                if (mm.size() <= 1)
                {
                    std::cerr << "Invalid message from PUB" << std::endl;
                    continue;
                }
                // pop out the first element, which is the topic name
                mm.erase(mm.begin());
                // zmq -> adt
                TodoStreamResponse rsp(std::move(mm));
                // receiver SPI
                receiver.recvSubMessage(rsp);
            }
        }
    }
};

#endif  //!__DEALERSERVICE__H__

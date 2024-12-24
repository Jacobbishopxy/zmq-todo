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

    // register DealerType instance
    BuilderPatternReturnType&& registerApp(ReceiverType& spi)
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
        const std::string& dealer_address,
        const std::string& sub_topic,
        const std::string& sub_address)
        : m_dealer_address(dealer_address),
          m_dealer_id(client_id),
          m_context(zmq::context_t(1)),
          m_main_pair(m_context, zmq::socket_type::pair),
          m_sub_topic(sub_topic),
          m_sub_address(sub_address),
          m_running(false)
    {
        m_inproc_addr = "inproc://client_service";
    }

    ~ClientService()
    {
        this->stop();
    }

    // ================================================================================================
    // API
    // ================================================================================================

    void start()
    {
        if (m_running.load())
            return;

        m_running.store(true);
        m_main_pair.bind(m_inproc_addr);

        if (this->m_receiver.has_value())
        {
            // move m_receiver into eventLoop thread
            T client = std::move(this->m_receiver.value());
            this->m_receiver.reset();

            auto t = [this, client = std::move(client)]() mutable
            {
                this->eventLoop(std::move(client));
            };
            m_dealer_thread = std::thread(t);
        }
        else
        {
            throw std::runtime_error("Client is not initialized!");
        }
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

    void sendMessage(const TodoRequest& message)
    {
        // send message via m_main_pair to dealer (cross threads)
        auto zmq_messages = message.toZmq();
        zmq::send_multipart(m_main_pair, zmq_messages);
    }

    TodoResponse recvMessage()
    {
        // recv message via m_main_pair from dealer (cross threads)
        std::vector<zmq::message_t> mm;
        auto recv_r = zmq::recv_multipart(m_main_pair, std::back_inserter(mm));

        TodoResponse rsp(std::move(mm));
        return rsp;
    }

private:
    std::string m_inproc_addr;
    std::string m_dealer_id;
    std::string m_dealer_address;
    std::string m_sub_topic;
    std::string m_sub_address;
    zmq::context_t m_context;
    std::atomic<bool> m_running;
    zmq::socket_t m_main_pair;
    std::thread m_dealer_thread;

    void eventLoop(T client)
    {
        zmq::socket_t pair(m_context, zmq::socket_type::pair);
        pair.connect(m_inproc_addr);

        zmq::socket_t dealer(m_context, zmq::socket_type::dealer);
        // dealer_id
        dealer.set(zmq::sockopt::routing_id, m_dealer_id);
        // send timeout
        dealer.set(zmq::sockopt::sndtimeo, 2000);
        // recv timeout
        dealer.set(zmq::sockopt::rcvtimeo, 2000);
        // connect
        dealer.connect(m_dealer_address);

        zmq::socket_t sub(m_context, zmq::socket_type::sub);
        // topic
        sub.set(zmq::sockopt::subscribe, m_sub_topic);
        // connect
        sub.connect(m_sub_address);

        // register events
        zmq::pollitem_t items[] = {
            {pair.handle(), 0, ZMQ_POLLIN, 0},
            {dealer.handle(), 0, ZMQ_POLLIN, 0},
            {sub.handle(), 0, ZMQ_POLLIN, 0},
        };

        // start event loop
        while (m_running.load())
        {
            zmq::poll(items, 3);

            // pair receives msg from m_main_pair
            if (items[0].revents && ZMQ_POLLIN)
            {
                // PAIR forward to external ROUTER
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

            // dealer receives msg from external router
            if (items[1].revents && ZMQ_POLLIN)
            {
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(dealer, std::back_inserter(mm));

                // send back to m_main_pair
                zmq::send_multipart(pair, mm);
            }

            // sub receives msg from external pub
            if (items[2].revents && ZMQ_POLLIN)
            {
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(dealer, std::back_inserter(mm));
                // zmq -> adt
                TodoStreamResponse rsp(std::move(mm));
                // client SPI
                client.recvSubMessage(rsp);
            }
        }
    }
};

#endif  //!__DEALERSERVICE__H__

/**
 * @file:	ClientService.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/20 14:24:34 Friday
 * @brief:
 **/

#ifndef __DEALERSERVICE__H__
#define __DEALERSERVICE__H__

#include <thread>
#include <zmq_addon.hpp>

#include "adt.h"

struct IClient
{
    virtual void recvDealerMessage(const TodoResponse& message) = 0;
    virtual void recvSubMessage(const TodoStreamResponse& message) = 0;
};

template <typename T>
concept IsClient = std::derived_from<T, IClient>;

template <class ClientType, typename BuilderPatternReturnType>
struct ClientBuilder
{
    ClientBuilder() = default;
    ClientBuilder(ClientBuilder&&) noexcept = default;
    ClientBuilder& operator=(ClientBuilder&&) noexcept = default;

    // disallow copy
    ClientBuilder(const ClientBuilder&) = delete;
    ClientBuilder& operator=(const ClientBuilder&) = delete;

    // register DealerType instance
    BuilderPatternReturnType&& registerApp(ClientType& spi)
    {
        m_client_ptr = &spi;
        return std::move(static_cast<BuilderPatternReturnType&&>(*this));
    }

    // SPI point get method
    [[nodiscard]] ClientType* getClientPtr() const noexcept
    {
        return m_client_ptr;
    }

private:
    ClientType* m_client_ptr = nullptr;  // SPI interface pointer
};

template <IsClient T>
class ClientService : public ClientBuilder<T, ClientService<T>>
{
public:
    ClientService(
        const std::string& dealer_address,
        const std::string& sub_address)
        : m_dealer_address(dealer_address),
          m_sub_address(sub_address),
          m_running(false)
    {
        m_context = zmq::context_t(1);

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
        m_dealer_thread = std::thread(&ClientService<T>::clientLoop, this);
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
        auto zmq_messages = message.toZmq();
        zmq::send_multipart(m_main_pair, zmq_messages);
    }

private:
    std::string m_inproc_addr;
    std::string m_dealer_address;
    std::string m_sub_address;
    zmq::context_t m_context;
    std::atomic<bool> m_running;
    zmq::socket_t m_main_pair;
    std::thread m_dealer_thread;

    void clientLoop()
    {
        zmq::socket_t pair(m_context, zmq::socket_type::pair);
        pair.connect(m_inproc_addr);

        zmq::socket_t dealer(m_context, zmq::socket_type::dealer);
        dealer.connect(m_dealer_address);

        zmq::socket_t sub(m_context, zmq::socket_type::sub);
        sub.connect(m_sub_address);

        T client;

        zmq::pollitem_t items[] = {
            {pair.handle(), 0, ZMQ_POLLIN, 0},
            {dealer.handle(), 0, ZMQ_POLLIN, 0},
            {sub.handle(), 0, ZMQ_POLLIN, 0},
        };

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
                // zmq -> adt
                TodoResponse rsp(mm);
                // client SPI
                client.recvDealerMessage(rsp);
            }

            // sub receives msg from external pub
            if (items[2].revents && ZMQ_POLLIN)
            {
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(dealer, std::back_inserter(mm));
                // zmq -> adt
                TodoStreamResponse rsp(mm);
                // client SPI
                client.recvSubMessage(rsp);
            }
        }
    }
};

#endif  //!__DEALERSERVICE__H__

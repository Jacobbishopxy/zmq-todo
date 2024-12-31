/**
 * @file:	WorkerService.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/24 14:33:24 Tuesday
 * @brief:
 **/

#ifndef __WORKERSERVICE__H__
#define __WORKERSERVICE__H__

#include <stdexcept>
#include <thread>
#include <zmq_addon.hpp>

#include "adt.h"
#include "common.hpp"

struct IProcessor
{
    virtual void recvDealerMessage(const TodoRequest& request) = 0;
};

template <typename T>
concept IsProcessor = std::derived_from<T, IProcessor>;

template <class ProcessorType, typename BuilderPatternReturnType>
struct WorkerBuilder
{
    WorkerBuilder() = default;
    WorkerBuilder(WorkerBuilder&&) noexcept = default;
    WorkerBuilder& operator=(WorkerBuilder&&) noexcept = default;

    // disallow copy
    WorkerBuilder(const WorkerBuilder&) = delete;
    WorkerBuilder& operator=(const WorkerBuilder&) = delete;

    // register ProcessorType instance
    BuilderPatternReturnType&& registerProcessor(ProcessorType& spi)
    {
        m_processor = std::move(spi);
        return std::move(static_cast<BuilderPatternReturnType&&>(*this));
    }

protected:
    std::optional<ProcessorType> m_processor;
};

template <IsProcessor T>
class WorkerService : public WorkerBuilder<T, WorkerService<T>>
{
public:
    WorkerService(
        const std::string& worker_id,
        const std::string& router_address,
        const std::string& pub_topic,
        const std::string& pub_address)
    {
        m_router_address = router_address;
        m_worker_id = worker_id;
        m_pub_topic = pub_topic;
        m_pub_address = pub_address;
        m_running = false;
        m_context = zmq::context_t(1);
        m_main_pair = zmq::socket_t(m_context, zmq::socket_type::pair);
        m_inproc_addr = "inproc://worker_service";
    }

    ~WorkerService()
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
        m_main_pair.set(zmq::sockopt::sndtimeo, 2000);
        // recv timeout
        m_main_pair.set(zmq::sockopt::rcvtimeo, 2000);
        // bind inproc address
        m_main_pair.bind(m_inproc_addr);

        if (this->m_processor.has_value())
        {
            // move m_processor into lambda
            auto t = [this, processor = std::move(this->m_processor.value())]() mutable
            {
                // move m_processor into eventLoop
                this->eventLoop(std::move(processor));
            };
            this->m_processor.reset();
            m_event_thread = std::thread(t);
        }
        else
        {
            throw std::runtime_error("Processor is not initialized!");
        }
    }

    void stop()
    {
        if (!m_running.load())
            return;

        m_running.store(false);
        m_main_pair.send(zmq::str_buffer("STOP!"), zmq::send_flags::none);

        if (m_event_thread.joinable())
            m_event_thread.join();
    }

    void sendMessage(const TodoResponse& message)
    {
        // send message via m_main_pair to dealer (cross threads)
        auto zmq_messages = message.toZmq();
        zmq::send_multipart(m_main_pair, zmq_messages);
    }

    void pubMessage(const TodoStreamResponse& message)
    {
        // publish message via m_main_pair to pub (cross threads)
        auto zmq_messages = message.toZmq();
        zmq::send_multipart(m_main_pair, zmq_messages);
    }

    // ================================================================================================
    // private
    // ================================================================================================

private:
    std::string m_inproc_addr;
    std::string m_worker_id;
    std::string m_router_address;
    std::string m_pub_topic;
    std::string m_pub_address;
    zmq::context_t m_context;
    std::atomic<bool> m_running;
    zmq::socket_t m_main_pair;
    std::thread m_event_thread;

    void eventLoop(T processor)
    {
        zmq::socket_t pair(m_context, zmq::socket_type::pair);
        pair.connect(m_inproc_addr);

        zmq::socket_t dealer(m_context, zmq::socket_type::dealer);
        // dealer_id
        dealer.set(zmq::sockopt::routing_id, m_worker_id);
        // send timeout
        dealer.set(zmq::sockopt::sndtimeo, 2000);
        // recv timeout
        dealer.set(zmq::sockopt::rcvtimeo, 2000);
        // connect
        dealer.connect(m_router_address);

        zmq::socket_t pub(m_context, zmq::socket_type::pub);
        pub.connect(m_pub_address);

        // register events
        zmq::pollitem_t items[] = {
            {pair.handle(), 0, ZMQ_POLLIN, 0},
            {dealer.handle(), 0, ZMQ_POLLIN, 0},
        };

        // start the event loop
        while (m_running.load())
        {
            zmq::poll(items, 2);

            // PAIR receives msg from m_main_pair
            if (items[0].revents && ZMQ_POLLIN)
            {
                // std::cout << "eventLoop.pair.recv_multipart" << std::endl;
                // PAIR forward to external ROUTER/PUB
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(pair, std::back_inserter(mm));

                // quit this event loop
                if (mm.size() == 1)
                {
                    std::string sig = messageToString(mm[0]);
                    if (sig == "STOP!\0")
                        break;
                }

                // size == 2 --> TodoStreamResponse, pub message
                if (mm.size() == 2)
                {
                    // std::cout << "eventLoop.pub.send_multipart" << std::endl;
                    // add zmq topic
                    zmq::message_t topic(m_pub_topic.size());
                    memcpy(topic.data(), m_pub_topic.data(), m_pub_topic.size());
                    pub.send(topic, zmq::send_flags::sndmore);
                    zmq::send_multipart(pub, mm);
                }

                // size == 3 --> TodoResponse, dealer message
                if (mm.size() == 3)
                {
                    // std::cout << "eventLoop.dealer.send_multipart" << std::endl;
                    zmq::send_multipart(dealer, mm);
                }
            }

            // DEALER receives msg from external ROUTER
            if (items[1].revents && ZMQ_POLLIN)
            {
                // std::cout << "eventLoop.dealer.recv_multipart" << std::endl;
                std::vector<zmq::message_t> mm;
                auto recv_r = zmq::recv_multipart(dealer, std::back_inserter(mm));
                // zmq -> adt
                TodoRequest req(std::move(mm));
                // processor SPI
                processor.recvDealerMessage(req);
            }
        }
    }
};

#endif  //!__WORKERSERVICE__H__

/**
 * @file:	simple_sub_server.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 18:30:21 Sunday
 * @brief:
 **/

#include <iostream>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    // Create a zmq context and SUB socket
    zmq::context_t context(1);
    zmq::socket_t subscriber(context, zmq::socket_type::sub);

    // Bind
    subscriber.bind(EP);
    std::cout << "Subscriber bound to " << EP << "..." << std::endl;

    // Subscribe to a topic
    std::string topic_filter = PubSubTopic;
    subscriber.set(zmq::sockopt::subscribe, topic_filter);
    std::cout << "Subscribed to topic: " << topic_filter << std::endl;

    while (true)
    {
        std::vector<zmq::message_t> mm;
        auto recv_r = zmq::recv_multipart(subscriber, std::back_inserter(mm));

        for (const auto& m : mm)
            std::cout << "- " << m.to_string() << std::endl;
        std::cout << std::endl;

        // subscriber.send(mm[0], zmq::send_flags::sndmore);

        // subscriber.send(zmq::buffer("hah"), zmq::send_flags::none);
    }

#if 0
    while (true)
    {
        // Receive the topic
        zmq::message_t topic;
        auto t = subscriber.recv(topic, zmq::recv_flags::none);
        if (t)
            std::cout << "recv_result t: " << t.value() << std::endl;
        std::string topic_text(static_cast<char*>(topic.data()), topic.size());

        // Receive the message payload
        zmq::message_t message;
        auto m = subscriber.recv(message, zmq::recv_flags::none);
        if (m)
            std::cout << "recv_result m: " << m.value() << std::endl;
        std::string message_text(static_cast<char*>(message.data()), message.size());

        // Log the received message
        std::cout << "Received: [" << topic_text << "] " << message_text
                  << std::endl;
    }
#endif

    return 0;
}

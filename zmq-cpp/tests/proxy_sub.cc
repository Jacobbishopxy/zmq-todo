/**
 * @file:	proxy_sub.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 21:29:02 Sunday
 * @brief:
 **/

#include <iostream>
#include <string>
#include <zmq.hpp>

#include "common.hpp"

int main()
{
    // Create a ZeroMQ context and SUB socket
    zmq::context_t context(1);
    zmq::socket_t subscriber(context, zmq::socket_type::sub);

    // Connect to the PUB-SUB proxy backend
    subscriber.connect(BackendEP);

    // Subscribe to a specific topic
    std::string topic_filter = PubSubTopic;
    subscriber.set(zmq::sockopt::subscribe, topic_filter);
    std::cout << "Subscribed to topic: " << topic_filter << std::endl;

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

    return 0;
}

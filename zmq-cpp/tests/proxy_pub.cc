/**
 * @file:	proxy_pub.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 21:26:40 Sunday
 * @brief:
 **/

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <zmq.hpp>

#include "common.hpp"

int main()
{
    // Create a ZeroMQ context and PUB socket
    zmq::context_t context(1);
    zmq::socket_t publisher(context, zmq::socket_type::pub);

    // Connect to the PUB-SUB proxy frontend
    publisher.connect(FrontendEP);

    int message_count = 0;
    while (true)
    {
        // Create a topic and message payload
        std::string topic = PubSubTopic;
        std::string message = "Message #" + std::to_string(message_count++);

        // Send a multipart message: [topic][payload]
        publisher.send(zmq::message_t(topic.data(), topic.size()), zmq::send_flags::sndmore);
        publisher.send(zmq::message_t(message.data(), message.size()), zmq::send_flags::none);

        std::cout << "Published: [" << topic << "] " << message << std::endl;

        // Sleep for a while before sending the next message
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

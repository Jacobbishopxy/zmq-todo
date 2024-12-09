/**
 * @file:	simple_pub_client.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 18:30:05 Sunday
 * @brief:
 **/

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    // Create a zmq context and PUB socket
    zmq::context_t context(1);
    zmq::socket_t publisher(context, zmq::socket_type::pub);

    // Connect
    publisher.connect(EP);
    std::cout << "Publisher connected to " << EP << "..." << std::endl;

    int message_count = 0;
    while (true)
    {
        // Create a topic and message payload
        std::string topic = PubSubTopic;
        std::string message = "Message #" + std::to_string(message_count++);

        // Send a multipart message: [topic][payload]
        zmq::message_t t(topic.size());
        memcpy(t.data(), topic.data(), topic.size());
        zmq::message_t m(message.size());
        memcpy(m.data(), message.data(), message.size());
        publisher.send(t, zmq::send_flags::sndmore);
        publisher.send(m, zmq::send_flags::none);
        std::cout << "Published: [" << topic << "] " << message << std::endl;

        // Sleep
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}

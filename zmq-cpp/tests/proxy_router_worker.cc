/**
 * @file:	proxy_router_worker.cc
 * @author:	Jacob Xie
 * @date:	2024/12/12 13:05:56 Thursday
 * @brief:
 **/

#include <cstring>
#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    zmq::context_t context(1);
    zmq::socket_t worker(context, zmq::socket_type::router);

    worker.connect(BackendEP);
    std::cout << "Worker connected to backend: " << BackendEP << "..."
              << std::endl;

    while (true)
    {
        zmq::message_t proxyIdentity;
        zmq::message_t reqIdentity;
        zmq::message_t delimiter;
        zmq::message_t message;

        int frameN(0);

        // Receive the first part
        while (true)
        {
            if (frameN == 0)
            {
                auto recv_result = worker.recv(proxyIdentity, zmq::recv_flags::none);
                std::cout << "<" << proxyIdentity.to_string() << "|";
                frameN++;
                continue;
            }
            else if (frameN == 1)
            {
                auto recv_result = worker.recv(reqIdentity, zmq::recv_flags::none);
                std::cout << reqIdentity.to_string() << "|";
                frameN++;
                continue;
            }
            else if (frameN == 2)
            {
                auto recv_result = worker.recv(delimiter, zmq::recv_flags::none);
                std::cout << delimiter.to_string() << "|";
                frameN++;
                continue;
            }
            else
            {
                auto recv_result = worker.recv(message, zmq::recv_flags::none);
                std::cout << message.to_string() << ",";
            }

            // Check if this is the last part
            if (!message.more())
            {
                std::cout << ">" << std::endl;
                break;  // Exit the loop if no more parts
            }
        }

        // Send a reply back
        std::string reply_text = "Reply: " + message.to_string();
        zmq::message_t reply(reply_text.size());
        memcpy(reply.data(), reply_text.data(), reply_text.size());

        worker.send(proxyIdentity, zmq::send_flags::sndmore);
        worker.send(reqIdentity, zmq::send_flags::sndmore);
        worker.send(delimiter, zmq::send_flags::sndmore);
        worker.send(reply, zmq::send_flags::none);
    }

    return 0;
}

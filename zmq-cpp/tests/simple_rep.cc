/**
 * @file:	simple_rep.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 00:12:38 Sunday
 * @brief:
 **/

#include <iostream>
#include <string>
#include <thread>
#include <zmq.hpp>

#include "common.hpp"

int main()
{
    // Prepare the context and REP socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::rep);

    // Bind to a TCP address
    socket.bind(EP);
    std::cout << "Server listening on " << EP << "..." << std::endl;

    while (true)
    {
        // Receive a request from the client
        zmq::message_t request;
        auto r = socket.recv(request, zmq::recv_flags::none);

        if (r)
            std::cout << "recv_result: " << r.value() << std::endl;
        std::string received_message(static_cast<char*>(request.data()), request.size());
        std::cout << "Received request: " << received_message << std::endl;

        // Simulate some work
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Send a reply back to the client
        std::string reply_message = "Hello, Client!";
        zmq::message_t reply(reply_message.size());
        memcpy(reply.data(), reply_message.data(), reply_message.size());
        socket.send(reply, zmq::send_flags::none);
    }

    return 0;
}

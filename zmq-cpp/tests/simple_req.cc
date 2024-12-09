/**
 * @file:	simple_req.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 00:16:56 Sunday
 * @brief:
 **/

#include <iostream>
#include <string>
#include <zmq.hpp>

#include "common.hpp"

int main()
{
    // Prepare the context and REQ socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::req);

    // Connect to the server
    socket.connect(EP);
    std::cout << "Client connected to " << EP << "..." << std::endl;

    // Send a request to the server
    std::string request_message = "Hello, Server!";
    zmq::message_t request(request_message.size());
    memcpy(request.data(), request_message.data(), request_message.size());
    socket.send(request, zmq::send_flags::none);
    std::cout << "Sent request: " << request_message << std::endl;

    // Wait for a reply from the server
    zmq::message_t reply;
    auto r = socket.recv(reply, zmq::recv_flags::none);
    if (r)
        std::cout << "recv_result: " << r.value() << std::endl;
    std::string reply_message(static_cast<char*>(reply.data()), reply.size());
    std::cout << "Received reply: " << reply_message << std::endl;

    return 0;
}

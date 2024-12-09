/**
 * @file:	proxy_req_client.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 21:05:47 Sunday
 * @brief:
 **/

#include <iostream>
#include <string>
#include <zmq.hpp>

#include "common.hpp"

int main()
{
    zmq::context_t context(1);
    zmq::socket_t client(context, zmq::socket_type::req);

    // Connect to the frontend ROUTER socket
    client.connect(FrontendEP);
    std::cout << "Client connected to frontend: " << FrontendEP << "..."
              << std::endl;

    for (int i = 0; i < 5; ++i)
    {
        // Send a request to the server
        std::string request_text = "Hello " + std::to_string(i);
        client.send(zmq::message_t(request_text.data(), request_text.size()), zmq::send_flags::none);
        std::cout << "Client sent: " << request_text << std::endl;

        // Wait for a reply
        zmq::message_t reply;
        auto r = client.recv(reply, zmq::recv_flags::none);
        if (r)
            std::cout << "recv_result r: " << r.value() << std::endl;
        std::string reply_text(static_cast<char*>(reply.data()), reply.size());
        std::cout << "Client received: " << reply_text << std::endl;
    }

    return 0;
}

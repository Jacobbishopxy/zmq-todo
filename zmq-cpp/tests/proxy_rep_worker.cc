/**
 * @file:	proxy_rep_worker.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 20:50:56 Sunday
 * @brief:
 **/

#include <cstring>
#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    zmq::context_t context(1);
    zmq::socket_t worker(context, zmq::socket_type::rep);

    worker.connect(BackendEP);
    std::cout << "Worker connected to backend: " << BackendEP << "..."
              << std::endl;

    while (true)
    {
        // Receive a request from the client
        zmq::message_t request;
        auto r = worker.recv(request, zmq::recv_flags::none);
        if (r)
            std::cout << "recv_result r: " << r.value() << std::endl;
        std::string request_text(static_cast<char*>(request.data()), request.size());
        std::cout << "Worker received: " << request_text << std::endl;

        // Send a reply back
        std::string reply_text = "Reply: " + request_text;
        zmq::message_t reply(reply_text.size());
        memcpy(reply.data(), reply_text.data(), reply_text.size());
        worker.send(reply, zmq::send_flags::none);
    }

    return 0;
}

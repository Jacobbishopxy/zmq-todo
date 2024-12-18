/**
 * @file:	simple_router_server.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 15:16:49 Sunday
 * @brief:
 **/

#include <cstring>
#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    // Create a zmq context and router socket
    zmq::context_t context(1);
    zmq::socket_t router(context, zmq::socket_type::router);

    // Router Id
    std::string router_id = "router_server";
    router.set(zmq::sockopt::routing_id, router_id);

    // Bind
    router.bind(EP);
    std::cout << "Router listening on " << EP << "..." << std::endl;

#if 0
    while (true)
    {
        recv_multipart(router);
    }
#endif

#if 1
    while (true)
    {
        zmq::message_t identity;
        zmq::message_t delimiter_or_request;
        zmq::message_t request;
        bool is_msg_len3 = true;

        // Receive the first frame (identity)
        auto id = router.recv(identity, zmq::recv_flags::none);
        if (id)
            std::cout << "recv_result -> id: " << id.value() << std::endl;

        // Receive the second frame
        auto dr = router.recv(delimiter_or_request, zmq::recv_flags::none);
        if (dr)
            std::cout << "recv_result -> delimiter_or_request: " << dr.value() << std::endl;

        std::string client_id(static_cast<char*>(identity.data()), identity.size());

        // Check if the second frame is a delimiter (empty frame)
        if (delimiter_or_request.size() == 0)
        {
            // REQ client: Receive the third frame (actual request)
            auto rq = router.recv(request, zmq::recv_flags::none);
            if (rq)
                std::cout << "recv_result -> req request: " << rq.value() << std::endl;
            std::string request_text(static_cast<char*>(request.data()), request.size());
            std::cout << "Received from REQ/DEALER [" << client_id
                      << "]: " << request_text << std::endl;
        }
        else
        {
            is_msg_len3 = false;
            // DEALER client: Treat the second frame as the request
            std::string request_text(static_cast<char*>(delimiter_or_request.data()), delimiter_or_request.size());
            std::cout << "Received from DEALER [" << client_id
                      << "]: " << request_text << std::endl;
        }

        // Prepare a reply
        std::string reply_text = "Reply to " + client_id;
        zmq::message_t reply(reply_text.size());
        memcpy(reply.data(), reply_text.data(), reply_text.size());

        if (is_msg_len3)
        {
            // Send multipart reply: [identity][delimiter][reply]
            router.send(identity, zmq::send_flags::sndmore);  // Identity frame
            router.send(zmq::message_t(0),
                        zmq::send_flags::sndmore);      // Empty delimiter frame
            router.send(reply, zmq::send_flags::none);  // Reply frame
        }
        else
        {
            // Send multipart reply: [identity][reply]
            router.send(identity, zmq::send_flags::sndmore);  // Identity frame
            router.send(reply, zmq::send_flags::none);        // Reply frame
        }
    }
#endif

    return 0;
}

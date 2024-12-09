/**
 * @file:	simple_router_client.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 15:17:32 Sunday
 * @brief:
 **/

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    // Create a zmq context and router socket
    zmq::context_t context(1);
    zmq::socket_t router(context, zmq::socket_type::router);

    // Router Id
    std::string router_id = "router_client";
    router.set(zmq::sockopt::routing_id, router_id);

    router.connect(EP);
    std::cout << router_id << " connected to " << EP << "..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Send a request
    // identity
    std::string dealer_id = "router_server";
    zmq::message_t identity(dealer_id.size());
    memcpy(identity.data(), dealer_id.data(), dealer_id.size());
    std::string request_text = "Hello, Router!";
    zmq::message_t request(request_text.size());
    memcpy(request.data(), request_text.data(), request_text.size());

    // [identity][delimiter][message]
    router.send(identity, zmq::send_flags::sndmore);  // Identity frame
    router.send(zmq::message_t(0), zmq::send_flags::sndmore);
    router.send(request, zmq::send_flags::none);
    std::cout << "Sent request: " << request_text << std::endl;

    // Receive a reply
    zmq::message_t reply;
    auto r1 = router.recv(reply, zmq::recv_flags::none);
    if (r1)
        std::cout << "recv_result r1: " << r1.value() << std::endl;
    auto r2 = router.recv(reply, zmq::recv_flags::none);
    if (r2)
        std::cout << "recv_result r2: " << r2.value() << std::endl;

    std::string reply_text(static_cast<char*>(reply.data()), reply.size());
    std::cout << "Received reply: " << reply_text << std::endl;

    return 0;
}

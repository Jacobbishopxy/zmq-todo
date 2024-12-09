/**
 * @file:	simple_dealer.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 10:45:26 Sunday
 * @brief:
 **/

#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    // Create a zmq context and router socket
    zmq::context_t context(1);
    zmq::socket_t dealer(context, zmq::socket_type::dealer);

    // Dealer Id
    std::string dealer_id = "dealer#1";
    dealer.set(zmq::sockopt::routing_id, dealer_id);

    dealer.connect(EP);
    std::cout << dealer_id << " connected to " << EP << "..." << std::endl;

    // Send a request
    std::string request_text = "Hello, Router!";
    zmq::message_t request(request_text.size());
    memcpy(request.data(), request_text.data(), request_text.size());
    // when this line has been commented out, router would recv 2 frame
    dealer.send(zmq::message_t(0), zmq::send_flags::sndmore);
    dealer.send(request, zmq::send_flags::none);
    std::cout << "Sent request: " << request_text << std::endl;

    // Receive a reply
    zmq::message_t reply;
    auto r1 = dealer.recv(reply, zmq::recv_flags::none);
    if (r1)
        std::cout << "recv_result r1: " << r1.value() << std::endl;
    auto r2 = dealer.recv(reply, zmq::recv_flags::none);
    if (r2)
        std::cout << "recv_result r2: " << r2.value() << std::endl;

    std::string reply_text(static_cast<char*>(reply.data()), reply.size());
    std::cout << "Received reply: " << reply_text << std::endl;

    return 0;
}

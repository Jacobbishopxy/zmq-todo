/**
 * @file:	simple_dealer_server.cc
 * @author:	Jacob Xie
 * @date:	2024/12/16 12:27:04 Monday
 * @brief:
 **/
#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

void printUsage(const char* programName)
{
    std::cerr << "Usage: " << programName << " --router_id=<router_id>\n";
    std::cerr << "Options:\n";
    std::cerr << "  --router_id=<router_id>  Specify the router ID to send the message to (required).\n";
}

// Function to parse command-line arguments
std::string parseRouterId(int argc, char** argv)
{
    std::string routerId;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.rfind("--router_id=", 0) == 0)
        {
            routerId = arg.substr(12);  // Extract value after "--router_id="
        }
    }

    return routerId;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse the --router_id argument
    std::string router_id = parseRouterId(argc, argv);

    // Create a zmq context and router socket
    zmq::context_t context(1);
    zmq::socket_t dealer(context, zmq::socket_type::dealer);

    dealer.bind(EP);
    std::cout << "Dealer bound to " << EP << "..." << std::endl;

    // send to a router
    zmq::message_t send_to(router_id.size());
    memcpy(send_to.data(), router_id.data(), router_id.size());

    // Send a request
    std::string request_text = "Hello, Router!";
    zmq::message_t request(request_text.size());
    memcpy(request.data(), request_text.data(), request_text.size());

    // dealer.send(send_to, zmq::send_flags::sndmore);            // router_id
    dealer.send(zmq::message_t(0), zmq::send_flags::sndmore);  // delimiter
    dealer.send(request, zmq::send_flags::none);
    std::cout << "Sent request: " << request_text << std::endl;

    recv_multipart(dealer);

    return 0;
}

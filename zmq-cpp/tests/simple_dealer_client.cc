/**
 * @file:	simple_dealer_client.cc
 * @author:	Jacob Xie
 * @date:	2024/12/08 10:45:26 Sunday
 * @brief:
 **/

#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

void printUsage(const char* programName)
{
    std::cerr << "Usage: " << programName << " --router_id=<router_id>\n";
    std::cerr << "Options:\n";
    std::cerr << "  --send_from=<self_id>  Specify the self ID to send the message to (required).\n";
    std::cerr << "  --send_to=<target_id>  Specify the target ID to send the message to (required).\n";
}

// Function to parse command-line arguments
std::string parseRouterId(int argc, char** argv)
{
    std::string routerId;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.rfind("--send_from=", 0) == 0)
        {
            routerId = arg.substr(12);  // Extract value after "--send_from="
        }
        else if (arg.rfind("--send_to=", 0) == 0)
        {
            routerId = arg.substr(10);  // Extract value after "--send_to="
        }
    }

    return routerId;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse the --router_id argument
    std::string send_from = "dealer#1";
    std::string send_to = parseRouterId(argc, argv);

    // Create a zmq context and router socket
    zmq::context_t context(1);
    zmq::socket_t dealer(context, zmq::socket_type::dealer);

    dealer.set(zmq::sockopt::routing_id, send_from);

    dealer.connect(EP);
    std::cout << send_from << " connected to " << EP << "..." << std::endl;

    // send to a router
    zmq::message_t target(send_to.size());
    memcpy(target.data(), send_to.data(), send_to.size());

    // Send a request
    std::string request_text = "Hello, Router!";
    zmq::message_t request(request_text.size());
    memcpy(request.data(), request_text.data(), request_text.size());

    // dealer.send(send_to, zmq::send_flags::sndmore);            // router_id
    dealer.send(zmq::message_t(0), zmq::send_flags::sndmore);  // delimiter
    dealer.send(request, zmq::send_flags::none);
    std::cout << "Sent request: " << request_text << std::endl;

    recv_multipart_print(dealer);

    return 0;
}

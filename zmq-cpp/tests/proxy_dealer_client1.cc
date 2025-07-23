/**
 * @file:	proxy_dealer_client1.cc
 * @author:	Jacob Xie
 * @date:	2025/07/23 19:00:08 Wednesday
 * @brief:
 **/

#include <iostream>
#include <string>
#include <vector>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "common.hpp"

int main()
{
    zmq::context_t context(1);
    zmq::socket_t client(context, zmq::socket_type::dealer);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.set(zmq::sockopt::routing_id, "my_dealer_client");

    std::string target_id = "my_dealer_worker";

    // Connect to the frontend ROUTER socket
    client.connect(FrontendEP);
    std::cout << "Client connected to frontend: " << FrontendEP << "..."
              << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int i = 0; i < 5; ++i)
    {
        auto m1 = stringToMessage(target_id);
        auto m2 = stringToMessage("hello! [" + std::to_string(i) + "]");

        std::vector<zmq::message_t> msg;
        msg.emplace_back(std::move(m1));
        msg.emplace_back(std::move(m2));

        zmq::send_multipart(client, msg);

        std::cout << "Sent " << std::to_string(i) << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

/**
 * @file:	proxy_dealer_worker.cc
 * @author:	Jacob Xie
 * @date:	2025/07/23 18:56:16 Wednesday
 * @brief:
 **/

#include <cstring>
#include <iostream>
#include <zmq.hpp>

#include "common.hpp"

int main(int argc, char** argv)
{
    zmq::context_t context(1);
    zmq::socket_t worker(context, zmq::socket_type::dealer);
    worker.set(zmq::sockopt::routing_id, "my_dealer_worker");

    worker.connect(BackendEP);
    std::cout << "Worker connected to backend: " << BackendEP << "..."
              << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    while (true)
    {
        std::vector<zmq::message_t> mm;
        auto recv_r = zmq::recv_multipart(worker, std::back_inserter(mm));

        std::cout << "Recv: ";
        for (const auto& m : mm)
        {
            std::cout << "[" << messageToString(m) << "]";
        }
        std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

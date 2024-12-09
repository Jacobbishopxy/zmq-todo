/**
 * @file:	simple_pair.cc
 * @author:	Jacob Xie
 * @date:	2024/12/09 17:20:55 Monday
 * @brief:
 **/

#include <thread>

#include "common.hpp"

int main(int argc, char** argv)
{
    // Create a ZeroMQ context and PUB socket
    zmq::context_t context(1);
    zmq::socket_t empty(context, zmq::socket_type::pair);

    empty.bind(EP);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}

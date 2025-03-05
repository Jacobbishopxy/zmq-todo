/**
 * @file:	socket_monitor.cc
 * @author:	Jacob Xie
 * @date:	2025/03/05 13:26:13 Wednesday
 * @brief:
 **/

#include <iostream>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "common.hpp"

void monitor_thread(zmq::socket_t* monitor_socket)
{
    while (true)
    {
        zmq::message_t event_msg;
        auto rr = monitor_socket->recv(event_msg);

        // Parse the event message
        const uint8_t* data = static_cast<uint8_t*>(event_msg.data());
        uint16_t event = *reinterpret_cast<const uint16_t*>(data);
        uint32_t routing_id = *reinterpret_cast<const uint32_t*>(data + 2);

        std::cout << "Event: " << event << ", Routing ID: " << routing_id << std::endl;
    }
}

int main()
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_ROUTER);

    // Bind the socket
    socket.bind("tcp://*:5555");

    // Create a monitor socket
    std::string monitor_endpoint = "inproc://monitor";
    zmq_socket_monitor(socket, monitor_endpoint.c_str(), ZMQ_EVENT_ALL);

    zmq::socket_t monitor_socket(context, ZMQ_PAIR);
    monitor_socket.connect(monitor_endpoint);

    // Start the monitor thread
    std::thread monitor(monitor_thread, &monitor_socket);
    monitor.detach();

    // Main application logic
    while (true)
    {
        std::vector<zmq::message_t> request;
        auto rr = zmq::recv_multipart(socket, std::back_inserter(request));

        // Get the routing ID from the message
        auto routing_id = messageToString(request[0]);
        std::cout << "Received message with routing ID: " << routing_id << std::endl;
    }

    return 0;
}

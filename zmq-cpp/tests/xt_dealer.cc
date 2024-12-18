/**
 * @file:	xt_dealer.cc
 * @author:	Jacob Xie
 * @date:	2024/12/18 10:15:50 Wednesday
 * @brief:	cross-thread dealer
 **/

#include <zmq.hpp>

#include "common.hpp"

const std::string INPROC_ADDR = "inproc://xt_dealer";

void dealerThread(zmq::context_t& context)
{
    // DEALER socket for external communication
    zmq::socket_t dealer(context, zmq::socket_type::dealer);
    dealer.connect(EP);

    // PAIR socket for communication with the main thread
    zmq::socket_t pair(context, zmq::socket_type::pair);
    pair.connect(INPROC_ADDR);

    while (true)
    {
        zmq::pollitem_t items[] = {
            {pair.handle(), 0, ZMQ_POLLIN, 0},
            {dealer.handle(), 0, ZMQ_POLLIN, 0},
        };

        // Poll indefinitely
        zmq::poll(items, 2, std::chrono::milliseconds{-1});

        // Handle messages from the main thread
        if (items[0].revents & ZMQ_POLLIN)
        {
            zmq::message_t msg;
            auto recv_r = pair.recv(msg, zmq::recv_flags::none);
            std::string content(static_cast<char*>(msg.data()), msg.size());
            std::cout << "[Dealer Thread] Received from main: " << content << std::endl;

            // Forward the message to the external DEALER socket
            dealer.send(zmq::buffer(content), zmq::send_flags::none);
        }

        // Handle messages from the external endpoint
        if (items[1].revents & ZMQ_POLLIN)
        {
            zmq::message_t msg;
            auto recv_r = dealer.recv(msg, zmq::recv_flags::none);
            std::string content(static_cast<char*>(msg.data()), msg.size());
            std::cout << "[Dealer Thread] Received from external: " << content << std::endl;

            // Forward the message back to the main thread
            pair.send(zmq::buffer(content), zmq::send_flags::none);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <message_to_send>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string message = argv[1];

    zmq::context_t context(1);

    // PAIR socket for communication with the dealer thread
    zmq::socket_t main_pair(context, zmq::socket_type::pair);
    main_pair.bind(INPROC_ADDR);

    // Start the dealer thread
    std::thread dealer_t(dealerThread, std::ref(context));

    // Allow the thread to initialize
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Send the message from the command-line argument to the dealer thread
    main_pair.send(zmq::buffer(message), zmq::send_flags::none);

    zmq::message_t reply;
    auto recv_r = main_pair.recv(reply, zmq::recv_flags::none);
    std::string content(static_cast<char*>(reply.data()), reply.size());
    std::cout << "[Main Thread] Received from dealer thread: " << content << std::endl;

    // Join the dealer thread before exiting
    dealer_t.join();

    return 0;
}

/**
 * @file:	common.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/08 00:19:29 Sunday
 * @brief:
 **/

#ifndef __COMMON__H__
#define __COMMON__H__

#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <zmq.hpp>

#if 0
// test on localhost
const std::string EP = "tcp://127.0.0.1:5555";
const std::string EP2 = "tcp://127.0.0.1:5556";
const std::string FrontendEP = "tcp://127.0.0.1:5565";
const std::string BackendEP = "tcp://127.0.0.1:5566";
#endif

#if 1
const std::string EP = "tcp://0.0.0.0:5555";
const std::string EP2 = "tcp://0.0.0.0:5556";
const std::string FrontendEP = "tcp://0.0.0.0:5565";
const std::string BackendEP = "tcp://0.0.0.0:5566";
#endif

const std::string PubSubTopic = "TA";

const std::string InprocMonitor = "inproc://monitor";
const std::string InprocMonitorF = "inproc://frontend_monitor";
const std::string InprocMonitorB = "inproc://backend_monitor";

class CustomMonitor : public zmq::monitor_t
{
public:
    void on_event_connected(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[CONNECTED] Event on address: " << addr << std::endl;
    }

    void on_event_disconnected(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[DISCONNECTED] Event on address: " << addr << std::endl;
    }

    void on_event_bind_failed(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[BIND FAILED] Address: " << addr << std::endl;
    }

    void on_event_listening(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[LISTENING] Address: " << addr << std::endl;
    }

    void on_event_accepted(const zmq_event_t& event, const char* addr) override
    {
        std::cout << "[EVENT ACCEPTED] Address: " << addr << std::endl;
    }

    void start(zmq::socket_t& socket, std::string const& addr)
    {
        this->init(socket, addr);
        auto l = [this]
        {
            while (this->check_event(-1))
            {
            }
        };
        auto t = std::thread(l);
        t.detach();
    }
};

inline std::vector<zmq::message_t> recvMultipart(zmq::socket_t& socket)
{
    std::vector<zmq::message_t> ans;

    while (true)
    {
        zmq::message_t message;
        auto recv_result = socket.recv(message, zmq::recv_flags::none);
        ans.push_back(std::move(message));

        if (!message.more())
            break;
    }

    return ans;
}

inline void sendMultipart(zmq::socket_t& socket, std::vector<zmq::message_t>& messages)
{
    for (size_t i = 0; i < messages.size(); ++i)
    {
        // Use sndmore for all but the last message
        auto flags = (i < messages.size() - 1) ? zmq::send_flags::sndmore : zmq::send_flags::none;
        socket.send(messages[i], flags);
    }
}

inline void recvMultipartPrint(zmq::socket_t& socket)
{
    zmq::message_t message;

    // Receive the first part
    while (true)
    {
        auto recv_result = socket.recv(message, zmq::recv_flags::none);
        std::cout << "> " << message.to_string() << std::endl;

        // Check if this is the last part
        if (!message.more())
        {
            std::cout << std::endl;
            break;  // Exit the loop if no more parts
        }
    }
}

inline std::string messageToString(const zmq::message_t& msg)
{
    return std::string(static_cast<const char*>(msg.data()), msg.size());
}

inline std::string messageToString(zmq::message_t& msg)
{
    return std::string(static_cast<char*>(msg.data()), msg.size());
}

inline zmq::message_t stringToMessage(const std::string& str)
{
    zmq::message_t res(str.size());
    memcpy(res.data(), str.data(), str.size());
    return res;
}

inline zmq::message_t stringToMessage(std::string& str)
{
    zmq::message_t res(str.size());
    memcpy(res.data(), str.data(), str.size());
    return res;
}

// Function to generate random string
inline std::string generateRandomString(size_t length)
{
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.resize(length);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.size() - 1);

    for (size_t i = 0; i < length; ++i)
    {
        result[i] = charset[dis(gen)];
    }

    return result;
}

inline void randomSleep(int max_seconds)
{
    // Randomly generate sleep time between 0 and 5 seconds
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, max_seconds);  // Sleep time between 0 and 5 seconds

    int sleep_duration = dis(gen);

    // Sleep for the generated duration
    std::this_thread::sleep_for(std::chrono::seconds(sleep_duration));
}

inline std::string getCurrentDateTime()
{
    // Get the current time
    std::time_t now = std::time(nullptr);

    // Convert to local time
    std::tm localTime;
#ifdef _WIN32
    // Windows-specific
    localtime_s(&localTime, &now);
#else
    // POSIX-compliant
    localtime_r(&now, &localTime);
#endif

    // Format the time as a string
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

inline std::string getTid()
{
    std::stringstream ss;
    auto tid = std::this_thread::get_id();
    ss << tid;
    return ss.str();
}

#endif  //!__COMMON__H__

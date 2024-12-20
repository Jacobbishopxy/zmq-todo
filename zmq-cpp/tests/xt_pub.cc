/**
 * @file:	xt_pub.cc
 * @author:	Jacob Xie
 * @date:	2024/12/19 14:50:18 Thursday
 * @brief:
 **/

#include <random>

#include "PubManager.hpp"
#include "common.hpp"

struct MockOut : ProtoMsgO
{
    uint num;
    std::string desc;

    MockOut(uint num, const std::string& desc)
        : num(num), desc(desc) {}

    std::vector<zmq::message_t> toZmq() const override
    {
        std::vector<zmq::message_t> messages;

        zmq::message_t numFrame(sizeof(num));
        memcpy(numFrame.data(), &num, sizeof(num));
        messages.emplace_back(std::move(numFrame));

        zmq::message_t descFrame(desc.size());
        memcpy(descFrame.data(), desc.data(), desc.size());
        messages.emplace_back(std::move(descFrame));

        return messages;
    }
};

// Function to generate random string
std::string generateRandomString(size_t length)
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

// Function to generate random MockOut
MockOut generateRandomMockOut()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    // Generate random num (unsigned integer)
    std::uniform_int_distribution<uint> num_dis(1, 1000);  // Random number between 1 and 1000
    uint num = num_dis(gen);

    // Generate random desc (random string of length between 5 and 20)
    std::uniform_int_distribution<size_t> desc_length_dis(5, 20);  // Random length between 5 and 20
    size_t desc_length = desc_length_dis(gen);
    std::string desc = generateRandomString(desc_length);

    return MockOut{num, desc};
}

void randomSleep()
{
    // Randomly generate sleep time between 0 and 5 seconds
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 5);  // Sleep time between 0 and 5 seconds

    int sleep_duration = dis(gen);

    std::cout << "Sleeping for " << sleep_duration << " seconds...\n";

    // Sleep for the generated duration
    std::this_thread::sleep_for(std::chrono::seconds(sleep_duration));
}

int main(int argc, char** argv)
{
    zmq::context_t context(1);

    auto myPub = PubManager<MockOut>(context, EP);
    myPub.make_bind();
    myPub.setTopic("TA");
    myPub.start();

    while (true)
    {
        randomSleep();
        MockOut mock = generateRandomMockOut();
        std::cout << "num: " << mock.num << ", desc: " << mock.desc << std::endl;
        myPub.publishMessage(mock);
    }

    return 0;
}

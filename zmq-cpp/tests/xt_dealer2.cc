/**
 * @file:	xt_dealer.cc
 * @author:	Jacob Xie
 * @date:	2024/12/19 09:11:20 Thursday
 * @brief:
 **/

#include "DealerManager.hpp"
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

struct MockIn : ProtoMsgI
{
    std::string info;

    MockIn(const std::vector<zmq::message_t>& messages)
        : ProtoMsgI(messages)
    {
        if (!messages.empty())
            info = std::move(messageToString(messages[0]));
    }
};

int main(int argc, char** argv)
{
    zmq::context_t context(1);

    auto myDealer = DealerManager<MockIn, MockOut>(context, EP);

    myDealer.start();
    std::cout << "Connected to " << EP << "..." << std::endl;

    MockOut mock_out{42, "mock out"};
    myDealer.sendMessage(mock_out);

    auto r = myDealer.recvMessage();
    std::cout << "recv >>> info: " << r.info << std::endl;

    return 0;
}

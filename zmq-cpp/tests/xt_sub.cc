/**
 * @file:	xt_sub.cc
 * @author:	Jacob Xie
 * @date:	2024/12/19 16:59:58 Thursday
 * @brief:
 **/

#include "SubManager.hpp"
#include "common.hpp"

uint messageToUint(const zmq::message_t& msg)
{
    return *static_cast<const uint*>(msg.data());
}

struct MockIn : ProtoMsgI
{
    uint num;
    std::string desc;

    MockIn(const std::vector<zmq::message_t>& messages)
        : ProtoMsgI(messages)
    {
        if (!messages.empty())
        {
            num = messageToUint(messages[0]);
            desc = messageToString(messages[1]);
        }
    }
};

int main(int argc, char** argv)
{
    zmq::context_t context(1);

    auto mySub = SubManager<MockIn>(context, EP);
    mySub.subscribe("TA");
    mySub.start();

    while (true)
    {
        auto [t, r] = mySub.recvMessage();
        std::cout << "topic: " << t << ", num: " << r.num << ", desc: " << r.desc << std::endl;
    }

    return 0;
}

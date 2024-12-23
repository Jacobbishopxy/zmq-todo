/**
 * @file:	ProtoMsg.h
 * @author:	Jacob Xie
 * @date:	2024/12/18 15:29:57 Wednesday
 * @brief:
 **/

#ifndef __PROTOMSG__H__
#define __PROTOMSG__H__

#include <zmq.hpp>

// [zmq::message] -> T
struct ProtoMsgI
{
    ProtoMsgI(const std::vector<zmq::message_t>& messages){};
    virtual ~ProtoMsgI() = default;
};

template <typename T>
concept IsProtoMsgI = std::derived_from<T, ProtoMsgI>;

// T -> [zmq::message]
struct ProtoMsgO
{
    virtual ~ProtoMsgO() = default;
    virtual std::vector<zmq::message_t> toZmq() const = 0;
};

template <typename T>
concept IsProtoMsgO = std::derived_from<T, ProtoMsgO>;

#endif  //!__PROTOMSG__H__

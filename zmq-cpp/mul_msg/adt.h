/**
 * @file:	adt.h
 * @author:	Jacob Xie
 * @date:	2024/12/16 15:21:32 Monday
 * @brief:
 **/

#ifndef __ADT__H__
#define __ADT__H__

#include <variant>
#include <zmq.hpp>

#include "ProtoMsg.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
#if WIN32
using uint = unsigned int;
#endif

enum class TodoAction
{
    GET_ALL,
    GET,
    CREATE,
    MODIFY,
    DELETE,
};

std::string todoActionToString(TodoAction action);

// Convert TodoAction to zmq::message_t
zmq::message_t todoActionTomessage(TodoAction action);

// Convert zmq::message_t back to TodoAction
TodoAction messageToTodoAction(zmq::message_t& message);

struct Todo
{
    uint32_t id;
    char description[256];
    bool completed;
};

using ResponsePayload = std::variant<bool, std::vector<Todo>>;

struct TodoResponse : public ProtoMsgI, ProtoMsgO
{
    std::string client_id;
    TodoAction action;
    ResponsePayload payload;

    TodoResponse(
        const std::string& client_id,
        const TodoAction& action,
        const ResponsePayload& payload);
    explicit TodoResponse(std::vector<zmq::message_t>&& messages);

    [[nodiscard]] std::vector<zmq::message_t> toZmq() const override;
};

#endif  //!__ADT__H__

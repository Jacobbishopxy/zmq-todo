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
#include "json.hpp"

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
    uint id;
    std::string description;
    bool completed;

    // Converts a Todo object to JSON
    void to_json(json& j) const;

    // Converts JSON to a Todo object
    static Todo from_json(const json& j);
};

void from_json(const nlohmann::json& j, Todo& d);

// Serializes a vector of Todo objects to JSON
json to_json(const std::vector<Todo>& todos);

// Deserializes JSON to a vector of Todo objects
std::vector<Todo> from_json(const json& j);

struct EmptyPayload
{
};

using RequestPayload = std::variant<Todo, uint, EmptyPayload>;

using ResponsePayload = std::variant<Todo, bool, std::vector<Todo>>;

struct TodoRequest : public ProtoMsgI, ProtoMsgO
{
    std::string worker_id;
    TodoAction action;
    RequestPayload payload;

    TodoRequest(
        const std::string& worker_id,
        const TodoAction& action,
        const RequestPayload& payload);
    explicit TodoRequest(std::vector<zmq::message_t>&& messages);

    std::vector<zmq::message_t> toZmq() const override;
};

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

    std::vector<zmq::message_t> toZmq() const override;
};

struct TodoStreamResponse : public ProtoMsgI, ProtoMsgO
{
    std::string info;
    std::string time;

    TodoStreamResponse(
        const std::string& info,
        const std::string& time);
    explicit TodoStreamResponse(std::vector<zmq::message_t>&& messages);

    std::vector<zmq::message_t> toZmq() const override;
};

void to_json(nlohmann::json& j, const TodoStreamResponse& d);

#endif  //!__ADT__H__

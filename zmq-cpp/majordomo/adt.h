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

#include "json.hpp"

using json = nlohmann::json;

enum class TodoAction
{
    GET_ALL,
    GET,
    CREATE,
    MODIFY,
    DELETE,
};

// Convert TodoAction to zmq::message_t
zmq::message_t todo_action_to_message(TodoAction action);

// Convert zmq::message_t back to TodoAction
TodoAction message_to_todo_action(zmq::message_t& message);

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

// Serializes a vector of Todo objects to JSON
json to_json(const std::vector<Todo>& todos);

// Deserializes JSON to a vector of Todo objects
std::vector<Todo> from_json(const json& j);

struct EmptyPayload
{
};

using RequestPayload = std::variant<Todo, uint, EmptyPayload>;

using ResponsePayload = std::variant<Todo, bool, std::vector<Todo>>;

struct TodoRequest
{
    std::string worker_id;
    TodoAction action;
    RequestPayload payload;
};

struct TodoResponse
{
    std::string client_id;
    TodoAction action;
    ResponsePayload payload;
};

#endif  //!__ADT__H__

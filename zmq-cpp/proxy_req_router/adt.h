/**
 * @file:	adt.h
 * @author:	Jacob Xie
 * @date:	2024/12/10 16:23:16 Tuesday
 * @brief:
 **/

#ifndef __ADT__H__
#define __ADT__H__

#include <sys/types.h>

#include <string>
#include <variant>
#include <vector>
#include <zmq.hpp>

#include <nlohmann/json.hpp>

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

// Serializes a vector of Todo objects to JSON
json to_json(const std::vector<Todo>& todos);

// Deserializes JSON to a vector of Todo objects
std::vector<Todo> from_json(const json& j);

struct EmptyPayload
{
};

using TodoRequest = std::variant<Todo, uint, EmptyPayload>;

using TodoResponse = std::variant<Todo, bool, std::vector<Todo>>;

#endif  //!__ADT__H__

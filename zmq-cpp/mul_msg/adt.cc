/**
 * @file:	adt.cc
 * @author:	Jacob Xie
 * @date:	2024/12/16 18:41:20 Monday
 * @brief:
 **/

#include "adt.h"

#include <unordered_map>

#include "common.hpp"

static const std::unordered_map<TodoAction, std::string> action_to_string = {
    {TodoAction::GET_ALL, "GET_ALL"},
    {TodoAction::GET, "GET"},
    {TodoAction::CREATE, "CREATE"},
    {TodoAction::MODIFY, "MODIFY"},
    {TodoAction::DELETE, "DELETE"}};

static const std::unordered_map<std::string, TodoAction> string_to_action = {
    {"GET_ALL", TodoAction::GET_ALL},
    {"GET", TodoAction::GET},
    {"CREATE", TodoAction::CREATE},
    {"MODIFY", TodoAction::MODIFY},
    {"DELETE", TodoAction::DELETE}};

// Convert TodoAction to string
std::string todoActionToString(TodoAction action)
{
    auto it = action_to_string.find(action);
    if (it != action_to_string.end())
    {
        const std::string& action_str = it->second;
        return action_str;
    }
    else
    {
        throw std::invalid_argument("Invalid TodoAction");
    }
}

// Convert TodoAction to zmq::message_t
zmq::message_t todoActionTomessage(TodoAction action)
{
    auto it = action_to_string.find(action);
    if (it != action_to_string.end())
    {
        const std::string& action_str = it->second;

        zmq::message_t message(action_str.size() + 1);
        memcpy(message.data(), action_str.c_str(), action_str.size());

        return message;
    }
    else
    {
        throw std::invalid_argument("Invalid TodoAction");
    }
}

// Convert zmq::message_t back to TodoAction
TodoAction messageToTodoAction(zmq::message_t& message)
{
    std::string action_str = messageToString(message);

    // Remove the last character if the string is not empty
    if (!action_str.empty())
        action_str.pop_back();

    auto it = string_to_action.find(action_str);
    if (it != string_to_action.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Invalid action string" + action_str);
    }
}

// ================================================================================================

TodoResponse::TodoResponse(
    const std::string& client_id,
    const TodoAction& action,
    const ResponsePayload& payload)
    : client_id(client_id), action(action), payload(payload) {}

TodoResponse::TodoResponse(std::vector<zmq::message_t>&& messages)
{
    // 1. client_id
    client_id = messageToString(messages[0]);

    // 2. action
    action = messageToTodoAction(messages[1]);

    // 3. response payload
    if (action == TodoAction::GET_ALL || action == TodoAction::GET)
    {
        std::vector<Todo> todos;
        for (size_t i = 2; i < messages.size(); ++i)
        {
            // 拆箱子的过程，把message_t拆成Todo
            Todo todo {};
            auto& cur = messages[i];
            memcpy(&todo, cur.data(), sizeof(todo));
            todos.emplace_back(todo);
        }
        payload = todos;
    }
    else
    {
        payload = static_cast<bool>(messages[2].data());
    }
}

std::vector<zmq::message_t> TodoResponse::toZmq() const
{
    std::vector<zmq::message_t> messages;

    // 1. client_id
    zmq::message_t clientIdFrame = stringToMessage(client_id);
    messages.emplace_back(std::move(clientIdFrame));

    // 2. action
    zmq::message_t actionFrame = todoActionTomessage(action);
    messages.emplace_back(std::move(actionFrame));

    // 3. response payload
    // 装箱子的过程，把payload打包成message_t
    if (std::holds_alternative<bool>(payload))
    {
        zmq::message_t responseMsg(sizeof(bool));
        std::memcpy(responseMsg.data(), &std::get<bool>(payload), sizeof(bool));
        messages.emplace_back((std::move(responseMsg)));
    }
    else if (std::holds_alternative<std::vector<Todo>>(payload))
    {
        auto vec = std::get<std::vector<Todo>>(payload);
        for (auto it = vec.begin(); it != vec.end(); ++it)
        {
            zmq::message_t responseMsg(sizeof(*it));
            std::memcpy(responseMsg.data(), &*it, sizeof(*it));
            messages.emplace_back(std::move(responseMsg));
        }
    }
    else
    {
        // blank payload
    }

    return messages;
}

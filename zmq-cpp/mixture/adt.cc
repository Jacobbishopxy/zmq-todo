/**
 * @file:	adt.cc
 * @author:	Jacob Xie
 * @date:	2024/12/16 18:41:20 Monday
 * @brief:
 **/

#include "adt.h"

#include <iostream>
#include <unordered_map>

#include "common.hpp"

// Convert TodoAction to zmq::message_t
zmq::message_t todo_action_to_message(TodoAction action)
{
    static const std::unordered_map<TodoAction, std::string> action_to_string = {
        {TodoAction::GET_ALL, "GET_ALL"},
        {TodoAction::GET, "GET"},
        {TodoAction::CREATE, "CREATE"},
        {TodoAction::MODIFY, "MODIFY"},
        {TodoAction::DELETE, "DELETE"}};

    auto it = action_to_string.find(action);
    if (it != action_to_string.end())
    {
        const std::string& action_str = it->second;

        // Create a message with the size of the string + 1 for null terminator
        zmq::message_t message(action_str.size() + 1);

        // Copy the string into the message
        memcpy(message.data(), action_str.c_str(), action_str.size() + 1);

        return message;
    }
    else
    {
        throw std::invalid_argument("Invalid TodoAction");
    }
}

// Convert zmq::message_t back to TodoAction
TodoAction message_to_todo_action(zmq::message_t& message)
{
    std::string action_str(static_cast<char*>(message.data()), message.size());

    // Remove the last character if the string is not empty
    if (!action_str.empty())
        action_str.pop_back();

    std::cout << "Received action: " << action_str << std::endl;

    static const std::unordered_map<std::string, TodoAction> string_to_action = {
        {"GET_ALL", TodoAction::GET_ALL},
        {"GET", TodoAction::GET},
        {"CREATE", TodoAction::CREATE},
        {"MODIFY", TodoAction::MODIFY},
        {"DELETE", TodoAction::DELETE}};

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

// Converts a Todo object to JSON
void Todo::to_json(json& j) const
{
    j = json{{"id", id}, {"description", description}, {"completed", completed}};
}

// Converts JSON to a Todo object
Todo Todo::from_json(const json& j)
{
    return {
        j.at("id").get<uint>(),
        j.at("description").get<std::string>(),
        j.at("completed").get<bool>()};
}

// Serializes a vector of Todo objects to JSON
json to_json(const std::vector<Todo>& todos)
{
    json j = json::array();
    for (const auto& todo : todos)
    {
        json todoJson;
        todo.to_json(todoJson);
        j.push_back(todoJson);
    }
    return j;
}

// Deserializes JSON to a vector of Todo objects
std::vector<Todo> from_json(const json& j)
{
    std::vector<Todo> todos;
    for (const auto& todoJson : j)
    {
        todos.push_back(Todo::from_json(todoJson));
    }
    return todos;
}

// ================================================================================================
// ADT <-> zmq msg
// 1. TodoRequest
// 2. TodoResponse
// 3. TodoStreamResponse
// ================================================================================================

TodoRequest::TodoRequest(
    const std::string& worker_id,
    const TodoAction& action,
    const RequestPayload& payload)
    : worker_id(worker_id), action(action), payload(payload) {}

TodoRequest::TodoRequest(std::vector<zmq::message_t>&& messages)
{
    // 1. worker_id
    worker_id = messageToString(messages[0]);

    // 2. action
    action = message_to_todo_action(messages[1]);

    // 3. payload
    if (messages[2].size() > 0)
    {
        if (action == TodoAction::CREATE || action == TodoAction::MODIFY)
        {
            std::string payloadStr = messageToString(messages[2]);
            json payloadJson = json::parse(payloadStr);
            payload = Todo::from_json(payloadJson);
        }
        else if (action == TodoAction::GET || action == TodoAction::DELETE)
        {
            std::string payloadStr = messageToString(messages[2]);
            json payloadJson = json::parse(payloadStr);
            payload = payloadJson.get<uint>();
        }
        else
        {
            payload = EmptyPayload{};
        }
    }
    else
    {
        payload = EmptyPayload{};
    }

    // return *this;
}

std::vector<zmq::message_t> TodoRequest::toZmq() const
{
    std::vector<zmq::message_t> messages;

    // 1. worker_id
    zmq::message_t workerIdFrame = stringToMessage(worker_id);
    messages.emplace_back(std::move(workerIdFrame));

    // 2. action
    zmq::message_t actionFrame = todo_action_to_message(action);
    messages.emplace_back(std::move(actionFrame));

    // 3. request payload
    json payloadJson;
    if (std::holds_alternative<Todo>(payload))
    {
        Todo todo = std::get<Todo>(payload);
        todo.to_json(payloadJson);
    }
    else if (std::holds_alternative<uint>(payload))
    {
        payloadJson = std::get<uint>(payload);
    }
    else
    {
        payloadJson = json::object();  // Empty JSON object
    }
    std::string serializedPayload = payloadJson.dump();
    zmq::message_t payloadMsg = stringToMessage(serializedPayload);
    messages.emplace_back(std::move(payloadMsg));

    return messages;
}

TodoResponse::TodoResponse(std::vector<zmq::message_t>&& messages)
{
    // 1. client_id
    client_id = messageToString(messages[0]);

    // 2. action
    action = message_to_todo_action(messages[1]);

    // 3. response payload
    std::string responseStr = messageToString(messages[2]);
    json responseJson = json::parse(responseStr);
    // Determine response type
    if (responseJson.is_array())
        payload = from_json(responseJson);
    else if (responseJson.is_boolean())
        payload = responseJson.get<bool>();
    else
        payload = Todo::from_json(responseJson);

    // return *this;
}

std::vector<zmq::message_t> TodoResponse::toZmq() const
{
    std::vector<zmq::message_t> messages;

    // 1. client_id
    zmq::message_t clientIdFrame = stringToMessage(client_id);
    messages.emplace_back(std::move(clientIdFrame));

    // 2. action
    zmq::message_t actionFrame = todo_action_to_message(action);
    messages.emplace_back(std::move(actionFrame));

    // 3. response payload
    json responseJson;
    if (std::holds_alternative<Todo>(payload))
    {
        Todo todo = std::get<Todo>(payload);
        todo.to_json(responseJson);
    }
    else if (std::holds_alternative<bool>(payload))
    {
        responseJson = std::get<bool>(payload);
    }
    else if (std::holds_alternative<std::vector<Todo>>(payload))
    {
        responseJson = to_json(std::get<std::vector<Todo>>(payload));
    }
    else
    {
        responseJson = json::object();  // Empty JSON object
    }
    std::string serializedResponse = responseJson.dump();
    zmq::message_t responseMsg = stringToMessage(serializedResponse);
    messages.emplace_back(std::move(responseMsg));

    return messages;
}

TodoStreamResponse::TodoStreamResponse(std::vector<zmq::message_t>&& messages)
{
    // 1. topic
    topic = messageToString(messages[0]);

    // 2. info
    info = messageToString(messages[1]);

    // return *this;
}

std::vector<zmq::message_t> TodoStreamResponse::toZmq() const
{
    std::vector<zmq::message_t> messages;

    // 1. topic
    zmq::message_t topicFrame = stringToMessage(topic);
    messages.emplace_back(std::move(topicFrame));

    // 2. info
    zmq::message_t infoFrame = stringToMessage(info);
    messages.emplace_back(std::move(infoFrame));

    return messages;
}

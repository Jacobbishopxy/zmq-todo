/**
 * @file:	adt.cc
 * @author:	Jacob Xie
 * @date:	2024/12/10 16:30:02 Tuesday
 * @brief:
 **/

#include "adt.h"
#include <iostream>
#include <unordered_map>

// Convert TodoAction to zmq::message_t
zmq::message_t todoActionTomessage(TodoAction action)
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
TodoAction messageToTodoAction(zmq::message_t& message)
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

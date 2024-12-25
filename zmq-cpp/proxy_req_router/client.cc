/**
 * @file:	client.cc
 * @author:	Jacob Xie
 * @date:	2024/12/11 08:58:47 Wednesday
 * @brief:
 **/

#include "client.h"

#include <iostream>

#include "common.hpp"

using json = nlohmann::json;

void printTodos(const std::vector<Todo>& todos);

void printUsage(const char* programName);

TodoClient::TodoClient(const std::string& address)
    : m_context(std::make_shared<zmq::context_t>(1)),
      m_client_socket(std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::req))
{
    this->m_client_socket->connect(address);
}

TodoClient::~TodoClient()
{
}

std::shared_ptr<zmq::socket_t> TodoClient::getSocket()
{
    return this->m_client_socket;
}

std::vector<Todo> TodoClient::getAllTodo()
{
    TodoRequest payload = EmptyPayload{};
    sendRequest(TodoAction::GET_ALL, payload);

    TodoResponse response = receiveResponse();
    if (std::holds_alternative<std::vector<Todo>>(response))
    {
        return std::get<std::vector<Todo>>(response);
    }

    throw std::runtime_error("Unexpected response type for GET_ALL");
}

std::optional<Todo> TodoClient::getTodo(int id)
{
    TodoRequest payload = static_cast<uint>(id);
    sendRequest(TodoAction::GET, payload);

    TodoResponse response = receiveResponse();
    if (std::holds_alternative<Todo>(response))
    {
        return std::get<Todo>(response);
    }

    return std::nullopt;
}

bool TodoClient::createTodo(const Todo& todo)
{
    TodoRequest payload = todo;
    sendRequest(TodoAction::CREATE, payload);

    TodoResponse response = receiveResponse();
    if (std::holds_alternative<bool>(response))
    {
        return std::get<bool>(response);
    }

    throw std::runtime_error("Unexpected response type for MODIFY");
}

bool TodoClient::modifyTodo(const Todo& todo)
{
    TodoRequest payload = todo;
    sendRequest(TodoAction::MODIFY, payload);

    TodoResponse response = receiveResponse();
    if (std::holds_alternative<bool>(response))
    {
        return std::get<bool>(response);
    }

    throw std::runtime_error("Unexpected response type for MODIFY");
}

bool TodoClient::deleteTodo(int id)
{
    TodoRequest payload = static_cast<uint>(id);
    sendRequest(TodoAction::DELETE, payload);

    TodoResponse response = receiveResponse();
    if (std::holds_alternative<bool>(response))
    {
        return std::get<bool>(response);
    }

    throw std::runtime_error("Unexpected response type for DELETE");
}

void TodoClient::sendRequest(TodoAction action, TodoRequest& payload)
{
    // Send action
    zmq::message_t actionMsg = todoActionTomessage(action);
    m_client_socket->send(actionMsg, zmq::send_flags::sndmore);

    // Send payload
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
    zmq::message_t payloadMsg(serializedPayload.data(), serializedPayload.size());
    m_client_socket->send(payloadMsg, zmq::send_flags::none);
}

TodoResponse TodoClient::receiveResponse()
{
    // action
    zmq::message_t actionMsg;
    auto a = m_client_socket->recv(actionMsg, zmq::recv_flags::none);
    auto action = messageToTodoAction(actionMsg);

    // payload
    zmq::message_t responseMsg;
    auto r = m_client_socket->recv(responseMsg, zmq::recv_flags::none);
    std::string responseStr(static_cast<char*>(responseMsg.data()), responseMsg.size());
    json responseJson = json::parse(responseStr);

    // Determine response type
    if (responseJson.is_array())
        return from_json(responseJson);
    else if (responseJson.is_boolean())
        return responseJson.get<bool>();
    else
        return Todo::from_json(responseJson);

    throw std::runtime_error("Unexpected response format");
}

void printTodos(const std::vector<Todo>& todos)
{
    for (const auto& todo : todos)
    {
        std::cout << "ID: " << todo.id
                  << ", Description: " << todo.description
                  << ", Completed: " << (todo.completed ? "Yes" : "No")
                  << "\n";
    }
}

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " --action=<ACTION> [--payload=<PAYLOAD>]\n";
    std::cout << "Available Actions:\n";
    std::cout << "  GET_ALL\n";
    std::cout << "  GET --payload=<Todo ID>\n";
    std::cout << "  CREATE --payload=<JSON Todo>\n";
    std::cout << "  MODIFY --payload=<JSON Todo>\n";
    std::cout << "  DELETE --payload=<Todo ID>\n";
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command-line arguments
    std::string actionArg;
    std::string payloadArg;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.rfind("--action=", 0) == 0)
        {
            actionArg = arg.substr(9);
        }
        else if (arg.rfind("--payload=", 0) == 0)
        {
            payloadArg = arg.substr(10);
        }
    }

    if (actionArg.empty())
    {
        std::cerr << "Error: --action is required.\n";
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    try
    {
        TodoClient client(FrontendEP);

        if (actionArg == "GET_ALL")
        {
            auto todos = client.getAllTodo();
            for (const auto& todo : todos)
            {
                std::cout << "ID: " << todo.id
                          << ", Description: " << todo.description
                          << ", Completed: " << (todo.completed ? "Yes" : "No") << "\n";
            }
        }
        else if (actionArg == "GET")
        {
            if (payloadArg.empty())
            {
                throw std::invalid_argument("GET requires a payload (Todo ID).");
            }
            int id = std::stoi(payloadArg);
            auto todo = client.getTodo(id);
            if (todo)
            {
                std::cout << "ID: " << todo->id
                          << ", Description: " << todo->description
                          << ", Completed: " << (todo->completed ? "Yes" : "No") << "\n";
            }
            else
            {
                std::cout << "Todo with ID " << id << " not found.\n";
            }
        }
        else if (actionArg == "CREATE")
        {
            if (payloadArg.empty())
            {
                throw std::invalid_argument("CREATE requires a payload (JSON Todo).");
            }
            json todoJson = json::parse(payloadArg);
            Todo todo = Todo::from_json(todoJson);
            if (client.createTodo(todo))
            {
                std::cout << "Todo created successfully.\n";
            }
            else
            {
                std::cout << "Failed to create Todo.\n";
            }
        }
        else if (actionArg == "MODIFY")
        {
            if (payloadArg.empty())
            {
                throw std::invalid_argument("MODIFY requires a payload (JSON Todo).");
            }
            json todoJson = json::parse(payloadArg);
            Todo todo = Todo::from_json(todoJson);
            if (client.modifyTodo(todo))
            {
                std::cout << "Todo modified successfully.\n";
            }
            else
            {
                std::cout << "Failed to modify Todo.\n";
            }
        }
        else if (actionArg == "DELETE")
        {
            if (payloadArg.empty())
            {
                throw std::invalid_argument("DELETE requires a payload (Todo ID).");
            }
            int id = std::stoi(payloadArg);
            if (client.deleteTodo(id))
            {
                std::cout << "Todo deleted successfully.\n";
            }
            else
            {
                std::cout << "Failed to delete Todo.\n";
            }
        }
        else
        {
            throw std::invalid_argument("Unknown action: " + actionArg);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

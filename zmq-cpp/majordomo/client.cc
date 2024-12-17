/**
 * @file:	client.cc
 * @author:	Jacob Xie
 * @date:	2024/12/16 19:13:17 Monday
 * @brief:
 **/

#include "client.h"

#include <iostream>

#include "common.hpp"

TodoClient::TodoClient(const std::string& client_id, const std::string& connect_address)
    : m_context(std::make_shared<zmq::context_t>(1)),
      m_client_socket(std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::dealer))
{
    this->m_client_id = client_id;
    this->m_client_socket->set(zmq::sockopt::routing_id, this->m_client_id);
    // timeout for `send`
    this->m_client_socket->set(zmq::sockopt::sndtimeo, 2000);
    // timeout for `recv`
    this->m_client_socket->set(zmq::sockopt::rcvtimeo, 2000);
    this->m_client_socket->connect(connect_address);

    std::cout << client_id << " connected to " << connect_address << std::endl;
}

TodoClient::~TodoClient()
{
    this->m_client_socket->close();
}

std::vector<Todo> TodoClient::getAllTodo(const std::string& worker_id)
{
    TodoRequest req{
        .worker_id = worker_id,
        .action = TodoAction::GET_ALL,
        .payload = EmptyPayload(),
    };
    sendRequest(req);

    TodoResponse rsp = receiveResponse();
    if (std::holds_alternative<std::vector<Todo>>(rsp.payload))
    {
        return std::get<std::vector<Todo>>(rsp.payload);
    }

    throw std::runtime_error("Unexpected response type for GET_ALL");
}

std::optional<Todo> TodoClient::getTodo(const std::string& worker_id, int id)
{
    TodoRequest req{
        .worker_id = worker_id,
        .action = TodoAction::GET,
        .payload = static_cast<uint>(id),
    };
    sendRequest(req);

    TodoResponse rsp = receiveResponse();
    if (std::holds_alternative<Todo>(rsp.payload))
    {
        return std::get<Todo>(rsp.payload);
    }

    return std::nullopt;
}

bool TodoClient::createTodo(const std::string& worker_id, const Todo& todo)
{
    TodoRequest req{
        .worker_id = worker_id,
        .action = TodoAction::CREATE,
        .payload = todo,
    };
    sendRequest(req);

    TodoResponse rsp = receiveResponse();
    if (std::holds_alternative<bool>(rsp.payload))
    {
        return std::get<bool>(rsp.payload);
    }

    throw std::runtime_error("Unexpected response type for MODIFY");
}

bool TodoClient::modifyTodo(const std::string& worker_id, const Todo& todo)
{
    TodoRequest req{
        .worker_id = worker_id,
        .action = TodoAction::MODIFY,
        .payload = todo,
    };
    sendRequest(req);

    TodoResponse rsp = receiveResponse();
    if (std::holds_alternative<bool>(rsp.payload))
    {
        return std::get<bool>(rsp.payload);
    }

    throw std::runtime_error("Unexpected response type for MODIFY");
}

bool TodoClient::deleteTodo(const std::string& worker_id, int id)
{
    TodoRequest req{
        .worker_id = worker_id,
        .action = TodoAction::DELETE,
        .payload = static_cast<uint>(id),
    };
    sendRequest(req);

    TodoResponse rsp = receiveResponse();
    if (std::holds_alternative<bool>(rsp.payload))
    {
        return std::get<bool>(rsp.payload);
    }

    throw std::runtime_error("Unexpected response type for DELETE");
}

void TodoClient::sendRequest(TodoRequest& request)
{
    std::cout << "TodoClient::sendRequest " << request.worker_id << std::endl;
    // [worker_id][action][request_payload]

    // send worker_id
    zmq::message_t worker_id_msg(request.worker_id.size());
    memcpy(worker_id_msg.data(), request.worker_id.data(), request.worker_id.size());
    m_client_socket->send(worker_id_msg, zmq::send_flags::sndmore);

    // send action
    zmq::message_t action_msg = todo_action_to_message(request.action);
    m_client_socket->send(action_msg, zmq::send_flags::sndmore);

    // send payload
    json payloadJson;
    if (std::holds_alternative<Todo>(request.payload))
    {
        Todo todo = std::get<Todo>(request.payload);
        todo.to_json(payloadJson);
    }
    else if (std::holds_alternative<uint>(request.payload))
    {
        payloadJson = std::get<uint>(request.payload);
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
    // [client_id][action][response_payload]

    std::cout << "TodoClient::receiveResponse start: " << std::endl;

    // client_id
    zmq::message_t client_id_msg;
    auto c = m_client_socket->recv(client_id_msg, zmq::recv_flags::none);
    std::string client_id(static_cast<char*>(client_id_msg.data()), client_id_msg.size());

    // action
    zmq::message_t action_msg;
    auto a = m_client_socket->recv(action_msg, zmq::recv_flags::none);
    auto action = message_to_todo_action(action_msg);

    // payload
    zmq::message_t payload_msg;
    auto p = m_client_socket->recv(payload_msg, zmq::recv_flags::none);
    std::string responseStr(static_cast<char*>(payload_msg.data()), payload_msg.size());
    json responseJson = json::parse(responseStr);
    ResponsePayload payload;
    // Determine response type
    if (responseJson.is_array())
        payload = from_json(responseJson);
    else if (responseJson.is_boolean())
        payload = responseJson.get<bool>();
    else
        payload = Todo::from_json(responseJson);

    std::cout << "TodoClient::receiveResponse done: " << std::endl;

    return TodoResponse{
        .client_id = client_id,
        .action = action,
        .payload = payload,
    };
}

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName
              << " --client_id=<CLIENT_ID> --worker_id=<WORKER_ID> --action=<ACTION> [--payload=<PAYLOAD>]\n";
    std::cout << "Available Actions:\n";
    std::cout << "  GET_ALL\n";
    std::cout << "  GET --payload=<Todo ID>\n";
    std::cout << "  CREATE --payload=<JSON Todo>\n";
    std::cout << "  MODIFY --payload=<JSON Todo>\n";
    std::cout << "  DELETE --payload=<Todo ID>\n";
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command-line arguments
    std::string clientArg;
    std::string workerArg;
    std::string actionArg;
    std::string payloadArg;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.rfind("--client_id=", 0) == 0)
        {
            clientArg = arg.substr(12);
        }
        else if (arg.rfind("--worker_id=", 0) == 0)
        {
            workerArg = arg.substr(12);
        }
        else if (arg.rfind("--action=", 0) == 0)
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
        TodoClient client(clientArg, EP);

        if (actionArg == "GET_ALL")
        {
            auto todos = client.getAllTodo(workerArg);
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
            auto todo = client.getTodo(workerArg, id);
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
            if (client.createTodo(workerArg, todo))
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
            if (client.modifyTodo(workerArg, todo))
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
            if (client.deleteTodo(workerArg, id))
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

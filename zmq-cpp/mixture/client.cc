/**
 * @file:   client.cc
 * @author: Jacob Xie
 * @date:   2024/12/16 19:13:17 Monday
 * @brief:
 **/

#include "client.h"

#include <iostream>

#include "adt.h"
#include "common.hpp"

class TodoClient;

void Receiver::recvSubMessage(const TodoStreamResponse& message)
{
    this->m_client_ptr->increaseMsgCount();
    this->m_client_ptr->printMsgCount();
}

void Receiver::bindClient(TodoClient* client_ptr)
{
    this->m_client_ptr = client_ptr;
}

// ================================================================================================

TodoClient::TodoClient(
    const std::string& client_id,
    const std::string& connect_address,
    const std::string& sub_topic,
    const std::string& sub_address)
{
    // initialize ClientService
    this->m_service = std::make_shared<ClientService<Receiver>>(client_id, connect_address, sub_topic, sub_address);
    // receiver bind ClientService
    Receiver r;
    r.bindClient(this);
    // register Receiver
    this->m_service->registerReceiver(r);
    this->m_service->start();
}

TodoClient::~TodoClient()
{
    this->m_service->stop();
}

std::vector<Todo> TodoClient::getAllTodo(const std::string& worker_id)
{
    TodoRequest req(worker_id, TodoAction::GET_ALL, EmptyPayload());
    // send request by DEALER
    this->m_service->sendMessage(req);
    // recv response from DEALER
    auto rsp = this->m_service->recvMessage();
    if (std::holds_alternative<std::vector<Todo>>(rsp.payload))
    {
        return std::get<std::vector<Todo>>(rsp.payload);
    }
    throw std::runtime_error("Unexpected response type for GET_ALL");
}

std::optional<Todo> TodoClient::getTodo(const std::string& worker_id, uint id)
{
    TodoRequest req(worker_id, TodoAction::GET, id);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (std::holds_alternative<Todo>(rsp.payload))
    {
        return std::get<Todo>(rsp.payload);
    }

    return std::nullopt;
}

bool TodoClient::createTodo(const std::string& worker_id, const Todo& todo)
{
    TodoRequest req(worker_id, TodoAction::CREATE, todo);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (std::holds_alternative<bool>(rsp.payload))
    {
        return std::get<bool>(rsp.payload);
    }

    throw std::runtime_error("Unexpected response type for CREATE");
}

bool TodoClient::modifyTodo(const std::string& worker_id, const Todo& todo)
{
    TodoRequest req(worker_id, TodoAction::MODIFY, todo);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (std::holds_alternative<bool>(rsp.payload))
    {
        return std::get<bool>(rsp.payload);
    }

    throw std::runtime_error("Unexpected response type for MODIFY");
}

bool TodoClient::deleteTodo(const std::string& worker_id, uint id)
{
    TodoRequest req(worker_id, TodoAction::DELETE, id);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (std::holds_alternative<bool>(rsp.payload))
    {
        return std::get<bool>(rsp.payload);
    }

    throw std::runtime_error("Unexpected response type for DELETE");
}

void TodoClient::increaseMsgCount()
{
    this->m_msg_count += 1;
}

void TodoClient::printMsgCount()
{
    std::cout << "TodoClient::printMsgCount: " << this->m_msg_count << std::endl;
}

// ================================================================================================

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

    TodoClient client(
        clientArg,
        EP,
        workerArg,
        BackendEP);

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

    // wait for sub messages
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return EXIT_SUCCESS;
}

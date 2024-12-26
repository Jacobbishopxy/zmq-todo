/**
 * @file:   client.cc
 * @author: Jacob Xie
 * @date:   2024/12/16 19:13:17 Monday
 * @brief:
 **/

#include "client.h"

#include <iostream>

#include "adt.h"

class TodoClient;

void Receiver::recvSubMessage(const TodoStreamResponse& message)
{
    std::cout << "Receiver::recvSubMessage: [info]: "
              << message.info << ", [time]: "
              << message.time << std::endl;
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
    const std::string& broker_address,
    const std::string& sub_topic,
    const std::string& sub_address)
{
    // initialize ClientService
    this->m_service = std::make_shared<ClientService<Receiver>>(client_id, broker_address, sub_topic, sub_address);
    // receiver bind TodoClient
    Receiver r;
    r.bindClient(this);
    // register Receiver
    this->m_service->registerReceiver(r);
    this->m_service->start();

    m_msg_count = 0;
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
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<std::vector<Todo>>(rsp->payload))
    {
        return std::get<std::vector<Todo>>(rsp->payload);
    }
    throw std::runtime_error("Unexpected response type for GET_ALL");
}

std::optional<Todo> TodoClient::getTodo(const std::string& worker_id, uint id)
{
    TodoRequest req(worker_id, TodoAction::GET, id);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<Todo>(rsp->payload))
    {
        return std::get<Todo>(rsp->payload);
    }

    return std::nullopt;
}

bool TodoClient::createTodo(const std::string& worker_id, const Todo& todo)
{
    TodoRequest req(worker_id, TodoAction::CREATE, todo);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<bool>(rsp->payload))
    {
        return std::get<bool>(rsp->payload);
    }

    throw std::runtime_error("Unexpected response type for CREATE");
}

bool TodoClient::modifyTodo(const std::string& worker_id, const Todo& todo)
{
    TodoRequest req(worker_id, TodoAction::MODIFY, todo);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<bool>(rsp->payload))
    {
        return std::get<bool>(rsp->payload);
    }

    throw std::runtime_error("Unexpected response type for MODIFY");
}

bool TodoClient::deleteTodo(const std::string& worker_id, uint id)
{
    TodoRequest req(worker_id, TodoAction::DELETE, id);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<bool>(rsp->payload))
    {
        return std::get<bool>(rsp->payload);
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


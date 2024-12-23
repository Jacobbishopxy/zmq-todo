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

void Receiver::recvDealerMessage(const TodoResponse& message)
{
    //
}

void Receiver::recvSubMessage(const TodoStreamResponse& message)
{
    //
}

// ================================================================================================

TodoClient::TodoClient(const std::string& client_id, const std::string& connect_address, const std::string& sub_topic, const std::string& sub_address)
{
    //
    this->m_service = std::make_shared<ClientService<Receiver>>("my_client", "my_worker", EP, BackendEP);
    Receiver r;
    r.bindClient(*this);
    this->m_service->registerApp(r);
    this->m_service->start();
}

TodoClient::~TodoClient()
{
    this->m_service->stop();
}

std::vector<Todo> TodoClient::getAllTodo(const std::string& worker_id)
{
    TodoRequest req(worker_id, TodoAction::GET_ALL, EmptyPayload());

    this->m_service->sendMessage(req);
}

std::optional<Todo> TodoClient::getTodo(const std::string& worker_id, int id)
{
    //
}

bool TodoClient::createTodo(const std::string& worker_id, const Todo& todo)
{
    //
}

bool TodoClient::modifyTodo(const std::string& worker_id, const Todo& todo)
{
    //
}

bool TodoClient::deleteTodo(const std::string& worker_id, int id)
{
    //
}

int main(int argc, char** argv)
{
    return EXIT_SUCCESS;
}

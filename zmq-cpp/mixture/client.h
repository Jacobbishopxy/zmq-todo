/**
 * @file:	client.h
 * @author:	Jacob Xie
 * @date:	2024/12/16 18:45:45 Monday
 * @brief:
 **/

#ifndef __CLIENT__H__
#define __CLIENT__H__

#include <memory>
#include <zmq.hpp>

#include "ClientService.hpp"
#include "adt.h"

class TodoClient;

class Receiver : public IReceiver
{
public:
    void recvSubMessage(const TodoStreamResponse& message) override;

    void bindClient(TodoClient* client_ptr);

private:
    TodoClient* m_client_ptr;
};

class TodoClient
{
public:
    TodoClient(
        const std::string& client_id,
        const std::string& connect_address,
        const std::string& sub_topic,
        const std::string& sub_address);
    ~TodoClient();

    std::vector<Todo> getAllTodo(const std::string& worker_id);
    std::optional<Todo> getTodo(const std::string& worker_id, uint id);
    bool createTodo(const std::string& worker_id, const Todo& todo);
    bool modifyTodo(const std::string& worker_id, const Todo& todo);
    bool deleteTodo(const std::string& worker_id, uint id);

    void increaseMsgCount();
    void printMsgCount();

private:
    std::shared_ptr<ClientService<Receiver>> m_service;
    uint m_msg_count;
};

#endif  //!__CLIENT__H__

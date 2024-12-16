/**
 * @file:	client.h
 * @author:	Jacob Xie
 * @date:	2024/12/16 18:45:45 Monday
 * @brief:
 **/

#ifndef __CLIENT__H__
#define __CLIENT__H__

#include <zmq.hpp>

#include "adt.h"

class TodoClient
{
public:
    TodoClient(const std::string& connect_address);
    ~TodoClient();

    std::vector<Todo> getAllTodo();
    std::optional<Todo> getTodo(int id);
    bool createTodo(const Todo& todo);
    bool modifyTodo(const Todo& todo);
    bool deleteTodo(int id);

protected:
    std::shared_ptr<zmq::socket_t> getSocket();

private:
    void sendRequest(TodoRequest& request);
    TodoResponse receiveResponse();

    std::shared_ptr<zmq::context_t> m_context;
    std::shared_ptr<zmq::socket_t> m_client_socket;
};

#endif  //!__CLIENT__H__

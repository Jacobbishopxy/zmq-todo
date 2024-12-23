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
    TodoClient(const std::string& client_id, const std::string& connect_address);
    ~TodoClient();

    std::vector<Todo> getAllTodo(const std::string& worker_id);
    std::optional<Todo> getTodo(const std::string& worker_id, int id);
    bool createTodo(const std::string& worker_id, const Todo& todo);
    bool modifyTodo(const std::string& worker_id, const Todo& todo);
    bool deleteTodo(const std::string& worker_id, int id);

protected:
    std::shared_ptr<zmq::socket_t> getSocket();

private:
    void sendRequest(TodoRequest& request);
    TodoResponse receiveResponse();

};

#endif  //!__CLIENT__H__

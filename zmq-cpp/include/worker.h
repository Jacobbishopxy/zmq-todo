/**
 * @file:	worker.h
 * @author:	Jacob Xie
 * @date:	2024/12/10 16:17:38 Tuesday
 * @brief:
 **/

#ifndef __WORKER__H__
#define __WORKER__H__

#include <unordered_map>
#include <zmq.hpp>

#include "adt.h"

class TodoWorker
{
public:
    TodoWorker(const std::string& address);
    ~TodoWorker();

    void run();

protected:
    std::shared_ptr<zmq::socket_t> getSocket();

private:
    void handleRequest(
        TodoAction action,
        const TodoRequest& payload,
        const zmq::message_t& reqId,
        const zmq::message_t& proxyId);
    void sendResponse(
        TodoAction action,
        const TodoResponse& response,
        const zmq::message_t& reqId,
        const zmq::message_t& proxyId);

    std::shared_ptr<zmq::context_t> m_context;
    std::shared_ptr<zmq::socket_t> m_worker_socket;

    std::unordered_map<uint, Todo> m_todos;
    uint m_next_id;
};

#endif  //!__WORKER__H__

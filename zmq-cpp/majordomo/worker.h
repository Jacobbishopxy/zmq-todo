/**
 * @file:	worker.h
 * @author:	Jacob Xie
 * @date:	2024/12/16 18:47:44 Monday
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
    void handleRequest(const TodoRequest& request);
    const TodoResponse& sendResponse();

    std::shared_ptr<zmq::context_t> m_context;
    std::shared_ptr<zmq::socket_t> m_worker_socket;

    std::unordered_map<uint, Todo> m_todos;
    uint m_next_id;
};

#endif  //!__WORKER__H__

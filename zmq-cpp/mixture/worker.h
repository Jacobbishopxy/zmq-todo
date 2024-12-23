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
    TodoWorker(const std::string& worker_id, const std::string& address);
    ~TodoWorker();

    void run();

protected:
    std::shared_ptr<zmq::socket_t> getSocket();

private:
    void handleRequest(const TodoRequest& request);
    void sendResponse(const TodoResponse& response);

    std::string m_worker_id;
    std::unordered_map<uint, Todo> m_todos;
    uint m_next_id;
};

#endif  //!__WORKER__H__

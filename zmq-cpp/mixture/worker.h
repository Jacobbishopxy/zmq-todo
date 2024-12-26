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

#include "WorkerService.hpp"
#include "adt.h"

class TodoWorker;

class Processor : public IProcessor
{
public:
    void recvDealerMessage(const TodoRequest& request) override;

    void bindWorker(TodoWorker* worker_ptr);

private:
    TodoWorker* m_worker_ptr;
};

class TodoWorker
{
public:
    TodoWorker(
        const std::string& worker_id,
        const std::string& broker_address,
        const std::string& pub_topic,
        const std::string& pub_address);
    ~TodoWorker();

    void run();
    TodoResponse handleRequest(const TodoRequest& request);
    void sendResponse(const TodoResponse& response);
    void pubInfo(const TodoStreamResponse& response);

private:
    std::shared_ptr<WorkerService<Processor>> m_service;
    std::unordered_map<uint, Todo> m_todos;
    uint m_next_id;
    std::thread m_pub_thread;
};

#endif  //!__WORKER__H__

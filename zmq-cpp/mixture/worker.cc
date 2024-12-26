/**
 * @file:	worker.cc
 * @author:	Jacob Xie
 * @date:	2024/12/17 08:37:27 Tuesday
 * @brief:
 **/

#include "worker.h"

#include <iostream>

#include "common.hpp"

void Processor::recvDealerMessage(const TodoRequest& request)
{
    std::cout << "Processor::recvDealerMessage: "
        << request.worker_id << " . "
        << todoActionToString(request.action)
        << std::endl;

    // receive a TodoRequest, handle biz logic, send a TodoResponse
    auto rsp = this->m_worker_ptr->handleRequest(request);
    this->m_worker_ptr->sendResponse(rsp);

    // publish info
    auto c = rsp.client_id;
    auto act = todoActionToString(rsp.action);
    auto info = c + "." + act;
    auto time = getCurrentDateTime();
    TodoStreamResponse sr(info, time);
    this->m_worker_ptr->pubInfo(sr);
}

void Processor::bindWorker(TodoWorker* worker_ptr)
{
    this->m_worker_ptr = worker_ptr;
}

// ================================================================================================

TodoWorker::TodoWorker(
    const std::string& worker_id,
    const std::string& broker_address,
    const std::string& pub_topic,
    const std::string& pub_address)
{
    // initialize WorkerService
    this->m_service = std::make_shared<WorkerService<Processor>>(worker_id, broker_address, pub_topic, pub_address);
    // processor bind TodoWorker
    Processor p;
    p.bindWorker(this);
    // register Processor
    this->m_service->registerProcessor(p);
    this->m_service->start();
}

TodoWorker::~TodoWorker()
{
    this->m_service->stop();

    if (m_pub_thread.joinable())
        m_pub_thread.join();
}

void TodoWorker::run()
{
    this->m_service->start();

#if 1
    auto p = [this]()
    {
        while (true)
        {
            randomSleep(30);

            auto randInfo = "randInfo." + generateRandomString(18);
            auto time = getCurrentDateTime();
            TodoStreamResponse rsp(randInfo, time);

            this->pubInfo(rsp);
        }
    };
    m_pub_thread = std::thread(p);
#endif
}

TodoResponse TodoWorker::handleRequest(const TodoRequest& request)
{
    std::cout << "TodoWorker::handleRequest: [client_id]: "
              << request.worker_id << ", [action]: "
              << todoActionToString(request.action) << std::endl;

    switch (request.action)
    {
        case TodoAction::GET_ALL:
        {
            std::vector<Todo> todos;
            for (const auto& [id, todo] : m_todos)
                todos.push_back(todo);

            return TodoResponse(request.worker_id, TodoAction::GET_ALL, todos);
        }
        case TodoAction::GET:
        {
            uint id = std::get<uint>(request.payload);
            if (m_todos.find(id) != m_todos.end())
                return TodoResponse(request.worker_id, TodoAction::GET, m_todos[id]);
            else
                return TodoResponse(request.worker_id, TodoAction::GET, false);
        }
        case TodoAction::CREATE:
        {
            Todo todo = std::get<Todo>(request.payload);
            todo.id = m_next_id++;
            m_todos[todo.id] = todo;
            return TodoResponse(request.worker_id, TodoAction::CREATE, true);
        }
        case TodoAction::MODIFY:
        {
            Todo todo = std::get<Todo>(request.payload);
            if (m_todos.find(todo.id) != m_todos.end())
            {
                m_todos[todo.id] = todo;
                return TodoResponse(request.worker_id, TodoAction::MODIFY, true);
            }
            else
            {
                return TodoResponse(request.worker_id, TodoAction::MODIFY, false);
            }
        }
        case TodoAction::DELETE:
        {
            uint id = std::get<uint>(request.payload);
            auto payload = m_todos.erase(id) ? true : false;
            return TodoResponse(request.worker_id, TodoAction::DELETE, payload);
        }
        default:
        {
            std::cerr << "Unknown action received!" << std::endl;
            return TodoResponse(request.worker_id, TodoAction::DELETE, false);
        }
    }
}

void TodoWorker::sendResponse(const TodoResponse& response)
{
    std::cout << "TodoWorker::sendResponse: [client_id]: "
              << response.client_id << ", [action]: "
              << todoActionToString(response.action) << std::endl;

    this->m_service->sendMessage(response);
}

void TodoWorker::pubInfo(const TodoStreamResponse& response)
{
    std::cout << "TodoWorker::pubInfo: [info]: "
              << response.info << ", [time]: "
              << response.time << std::endl;
    this->m_service->pubMessage(response);
}

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " --worker_id=<WORKER_ID>" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    std::string workerArg;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.rfind("--worker_id=", 0) == 0)
        {
            workerArg = arg.substr(12);
        }
    }

    TodoWorker worker(
        workerArg,
        EP,
        workerArg,
        FrontendEP);

    worker.run();

    std::cout << "TodoWorker start" << std::endl;
    std::cout << "Broker address: " << EP << std::endl;
    std::cout << "Pub address: " << FrontendEP << std::endl;

    // wait for client messages
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return EXIT_SUCCESS;
}

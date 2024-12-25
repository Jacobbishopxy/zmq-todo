/**
 * @file:	worker.cc
 * @author:	Jacob Xie
 * @date:	2024/12/17 08:37:27 Tuesday
 * @brief:
 **/

#include "worker.h"

#include <iostream>

#include "common.hpp"

TodoWorker::TodoWorker(const std::string& worker_id, const std::string& address)
    : m_context(std::make_shared<zmq::context_t>()),
      m_worker_socket(std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::dealer))
{
    this->m_worker_id = worker_id;
    this->m_worker_socket->set(zmq::sockopt::routing_id, this->m_worker_id);
    this->m_worker_socket->connect(address);
}

TodoWorker::~TodoWorker()
{
    this->m_worker_socket->close();
}

void TodoWorker::run()
{
    while (true)
    {
        std::vector<zmq::message_t> frames;

        // Receive up to five frames
        for (int i = 0; i < 3; ++i)
        {
            zmq::message_t frame;
            if (!m_worker_socket->recv(frame, zmq::recv_flags::none))
            {
                break;
            }
            frames.push_back(std::move(frame));
        }

        // Validate frame count
        if (frames.size() < 3)
        {
            std::cerr << "Error: Received incomplete message with " << frames.size() << " frame(s).\n";
            zmq::message_t errorFrame("Error: Incomplete message", 24);
            for (size_t i = 0; i < frames.size(); ++i)
            {
                zmq::send_flags flags = (i == frames.size() - 1) ? zmq::send_flags::none : zmq::send_flags::sndmore;
                m_worker_socket->send(frames[i], flags);
            }
            continue;
        }

        // Extract frames
        zmq::message_t& clientFrame = frames[0];
        zmq::message_t& actionFrame = frames[1];
        zmq::message_t& payloadFrame = frames[2];

        std::string client_id(static_cast<char*>(clientFrame.data()), clientFrame.size());

        // Parse the action
        TodoAction action = messageToTodoAction(actionFrame);

        // Parse the payload
        RequestPayload payload;
        if (payloadFrame.size() > 0)
        {
            if (action == TodoAction::CREATE || action == TodoAction::MODIFY)
            {
                std::string payloadStr(static_cast<char*>(payloadFrame.data()), payloadFrame.size());
                json payloadJson = json::parse(payloadStr);

                payload = Todo::from_json(payloadJson);
            }
            else if (action == TodoAction::GET || action == TodoAction::DELETE)
            {
                std::string payloadStr(static_cast<char*>(payloadFrame.data()), payloadFrame.size());
                json payloadJson = json::parse(payloadStr);
                payload = payloadJson.get<uint>();
            }
            else
            {
                payload = EmptyPayload{};
            }
        }
        else
        {
            payload = EmptyPayload{};
        }

        TodoRequest req{
            .worker_id = client_id,
            .action = action,
            .payload = payload,
        };

        // Handle the request
        handleRequest(req);
    }
}

std::shared_ptr<zmq::socket_t> TodoWorker::getSocket()
{
    return m_worker_socket;
}

void TodoWorker::handleRequest(const TodoRequest& request)
{
    std::cout << "TodoWorker::handleRequest: " << request.worker_id << std::endl;

    switch (request.action)
    {
        case TodoAction::GET_ALL:
        {
            std::vector<Todo> todos;
            for (const auto& [id, todo] : m_todos)
                todos.push_back(todo);
            TodoResponse rsp{
                .client_id = request.worker_id,
                .action = TodoAction::GET_ALL,
                .payload = todos,
            };

            sendResponse(rsp);
            break;
        }
        case TodoAction::GET:
        {
            uint id = std::get<uint>(request.payload);
            TodoResponse rsp;
            if (m_todos.find(id) != m_todos.end())
                rsp = {
                    .client_id = request.worker_id,
                    .action = TodoAction::GET,
                    .payload = m_todos[id],
                };
            else
                rsp = {
                    .client_id = request.worker_id,
                    .action = TodoAction::GET,
                    .payload = false,
                };
            sendResponse(rsp);
            break;
        }
        case TodoAction::CREATE:
        {
            Todo todo = std::get<Todo>(request.payload);
            todo.id = m_next_id++;
            m_todos[todo.id] = todo;
            TodoResponse rsp{
                .client_id = request.worker_id,
                .action = TodoAction::CREATE,
                .payload = true};

            sendResponse(rsp);
            break;
        }
        case TodoAction::MODIFY:
        {
            Todo todo = std::get<Todo>(request.payload);
            TodoResponse rsp;
            if (m_todos.find(todo.id) != m_todos.end())
            {
                m_todos[todo.id] = todo;
                rsp = {
                    .client_id = request.worker_id,
                    .action = TodoAction::MODIFY,
                    .payload = true,
                };
            }
            else
            {
                rsp = {
                    .client_id = request.worker_id,
                    .action = TodoAction::MODIFY,
                    .payload = false,
                };
            }
            sendResponse(rsp);
            break;
        }
        case TodoAction::DELETE:
        {
            uint id = std::get<uint>(request.payload);
            TodoResponse rsp{
                .client_id = request.worker_id,
                .action = TodoAction::DELETE,
                .payload = m_todos.erase(id) ? true : false,
            };
            sendResponse(rsp);
            break;
        }
        default:
            std::cerr << "Unknown action received!" << std::endl;
            TodoResponse rsp{
                .client_id = request.worker_id,
                .action = TodoAction::DELETE,
                .payload = false};

            sendResponse(rsp);
            break;
    }
}

void TodoWorker::sendResponse(const TodoResponse& response)
{
    // [client_id][action][response_payload]

    std::cout << "TodoWorker::sendResponse: " << response.client_id << std::endl;

    zmq::message_t client_id(response.client_id);
    memcpy(client_id.data(), response.client_id.data(), response.client_id.size());
    m_worker_socket->send(client_id, zmq::send_flags::sndmore);

    zmq::message_t action = todoActionTomessage(response.action);
    m_worker_socket->send(action, zmq::send_flags::sndmore);

    json responseJson;
    // Send response payload
    if (std::holds_alternative<Todo>(response.payload))
    {
        Todo todo = std::get<Todo>(response.payload);
        todo.to_json(responseJson);
    }
    else if (std::holds_alternative<bool>(response.payload))
    {
        responseJson = std::get<bool>(response.payload);
    }
    else if (std::holds_alternative<std::vector<Todo>>(response.payload))
    {
        responseJson = to_json(std::get<std::vector<Todo>>(response.payload));
    }
    else
    {
        responseJson = json::object();  // Empty JSON object
    }

    std::string serializedResponse = responseJson.dump();
    zmq::message_t responseMsg(serializedResponse.data(), serializedResponse.size());
    m_worker_socket->send(responseMsg, zmq::send_flags::none);
};

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " --worker_id=<WORKER_ID>";
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

    try
    {
        // Create and run the worker
        TodoWorker worker(workerArg, EP);
        std::cout << "Worker started and connected to " << BackendEP << "\n";
        worker.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

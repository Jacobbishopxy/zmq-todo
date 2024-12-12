/**
 * @file:	worker.cc
 * @author:	Jacob Xie
 * @date:	2024/12/11 16:29:20 Wednesday
 * @brief:
 **/

#include "worker.h"

#include <iostream>

#include "common.hpp"

TodoWorker::TodoWorker(const std::string& backendAddress)
    : m_context(std::make_shared<zmq::context_t>(1)),
      m_worker_socket(std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::router))
{
    m_worker_socket->connect(backendAddress);
}

TodoWorker::~TodoWorker()
{
    // Resources managed by shared_ptr
}

void TodoWorker::run()
{
    // while (true)
    //     recv_multipart(*m_worker_socket);

    // #if 0
    while (true)
    {
        std::vector<zmq::message_t> frames;

        // Receive up to five frames
        for (int i = 0; i < 5; ++i)
        {
            zmq::message_t frame;
            if (!m_worker_socket->recv(frame, zmq::recv_flags::none))
            {
                break;
            }
            frames.push_back(std::move(frame));
        }

        // Validate frame count
        if (frames.size() < 5)
        {
            std::cerr << "Error: Received incomplete message with " << frames.size() << " frame(s).\n";
            zmq::message_t errorFrame("Error: Incomplete message", 24);
            for (size_t i = 0; i < frames.size(); ++i)
            {
                zmq::send_flags flags = (i == frames.size() - 1) ? zmq::send_flags::none : zmq::send_flags::sndmore;
                m_worker_socket->send(frames[i], flags);
            }
            if (frames.size() == 4)
            {
                m_worker_socket->send(errorFrame, zmq::send_flags::none);
            }
            continue;
        }

        // Extract frames
        zmq::message_t& proxyId = frames[0];
        zmq::message_t& reqId = frames[1];
        zmq::message_t& delimiter = frames[2];  // Can be used for validation if necessary
        zmq::message_t& actionFrame = frames[3];
        zmq::message_t& payloadFrame = frames[4];

        // Parse the action
        TodoAction action = message_to_todo_action(actionFrame);

        // Parse the payload
        TodoRequest payload;
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

        // Handle the request
        handleRequest(action, payload, reqId, proxyId);
    }
    // #endif
}

std::shared_ptr<zmq::socket_t> TodoWorker::getSocket()
{
    return m_worker_socket;
}

void TodoWorker::handleRequest(TodoAction action, const TodoRequest& payload, const zmq::message_t& reqId, const zmq::message_t& proxyId)
{
    switch (action)
    {
        case TodoAction::GET_ALL:
        {
            std::vector<Todo> todos;
            for (const auto& [id, todo] : m_todos)
            {
                todos.push_back(todo);
            }
            sendResponse(action, todos, reqId, proxyId);
            break;
        }
        case TodoAction::GET:
        {
            uint id = std::get<uint>(payload);
            if (m_todos.find(id) != m_todos.end())
                sendResponse(action, m_todos[id], reqId, proxyId);
            else
                sendResponse(action, false, reqId, proxyId);
            break;
        }
        case TodoAction::CREATE:
        {
            Todo todo = std::get<Todo>(payload);
            todo.id = m_next_id++;
            m_todos[todo.id] = todo;
            sendResponse(action, true, reqId, proxyId);
            break;
        }
        case TodoAction::MODIFY:
        {
            Todo todo = std::get<Todo>(payload);
            if (m_todos.find(todo.id) != m_todos.end())
            {
                m_todos[todo.id] = todo;
                sendResponse(action, true, reqId, proxyId);
            }
            else
            {
                sendResponse(action, false, reqId, proxyId);
            }
            break;
        }
        case TodoAction::DELETE:
        {
            uint id = std::get<uint>(payload);
            if (m_todos.erase(id))
            {
                sendResponse(action, true, reqId, proxyId);
            }
            else
            {
                sendResponse(action, false, reqId, proxyId);
            }
            break;
        }
        default:
            std::cerr << "Unknown action received\n";
            sendResponse(action, false, reqId, proxyId);
            break;
    }
}

void TodoWorker::sendResponse(TodoAction action, const TodoResponse& response, const zmq::message_t& reqId, const zmq::message_t& proxyId)
{
    // Send Proxy ID
    zmq::message_t pid(proxyId.size());
    memcpy(pid.data(), proxyId.data(), proxyId.size());
    m_worker_socket->send(pid, zmq::send_flags::sndmore);

    // Send REQ ID
    zmq::message_t rid(reqId.size());
    memcpy(rid.data(), reqId.data(), reqId.size());
    m_worker_socket->send(rid, zmq::send_flags::sndmore);

    // Send delimiter
    zmq::message_t delimiter;
    m_worker_socket->send(delimiter, zmq::send_flags::sndmore);

    // Send action
    zmq::message_t actionFrame = todo_action_to_message(action);
    m_worker_socket->send(actionFrame, zmq::send_flags::sndmore);

    // Send response payload as JSON
    json responseJson;
    // Send response payload
    if (std::holds_alternative<Todo>(response))
    {
        Todo todo = std::get<Todo>(response);
        todo.to_json(responseJson);
    }
    else if (std::holds_alternative<bool>(response))
    {
        responseJson = std::get<bool>(response);
    }
    else if (std::holds_alternative<std::vector<Todo>>(response))
    {
        responseJson = to_json(std::get<std::vector<Todo>>(response));
    }
    else
    {
        responseJson = json::object();  // Empty JSON object
    }

    std::string serializedResponse = responseJson.dump();
    zmq::message_t responseMsg(serializedResponse.data(), serializedResponse.size());
    m_worker_socket->send(responseMsg, zmq::send_flags::none);
}

int main(int argc, char** argv)
{
    try
    {
        // Create and run the worker
        TodoWorker worker(BackendEP);
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

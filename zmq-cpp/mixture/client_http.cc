/**
 * @file:	client_http.cc
 * @author:	Jacob Xie
 * @date:	2024/12/26 13:48:39 Thursday
 * @brief:
 **/

#include "client_http.h"

#include "json.hpp"

void Receiver::recvSubMessage(const std::string& topic, const TodoStreamResponse& message)
{
    this->m_client_ptr->triggerMsgCount();

    nlohmann::json j;
    to_json(j, message);
    this->m_client_ptr->broadcastMessage(topic, j.dump());
}

void Receiver::bindClient(TodoClientHttp* client_ptr)
{
    this->m_client_ptr = client_ptr;
}

// ================================================================================================

TodoClientHttp::TodoClientHttp(
    const std::string& client_id,
    const std::string& broker_address,
    const std::string& sub_topic,
    const std::string& sub_address)
{
    m_sub_topic = sub_topic;
    // initialize ClientService
    this->m_service = std::make_shared<ClientService<Receiver>>(client_id, broker_address, sub_topic, sub_address);
    // receiver bind TodoClient
    Receiver r;
    r.bindClient(this);
    // register Receiver
    this->m_service->registerReceiver(r);

    m_msg_count = 0;
}

TodoClientHttp::~TodoClientHttp()
{
    this->m_service->stop();
}

std::vector<Todo> TodoClientHttp::getAllTodo(const std::string& worker_id)
{
    TodoRequest req(worker_id, TodoAction::GET_ALL, EmptyPayload());
    // send request by DEALER
    this->m_service->sendMessage(req);
    // recv response from DEALER
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<std::vector<Todo>>(rsp->payload))
    {
        return std::get<std::vector<Todo>>(rsp->payload);
    }
    throw std::runtime_error("Unexpected response type for GET_ALL");
}

std::optional<Todo> TodoClientHttp::getTodo(const std::string& worker_id, uint id)
{
    TodoRequest req(worker_id, TodoAction::GET, id);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<Todo>(rsp->payload))
    {
        return std::get<Todo>(rsp->payload);
    }

    return std::nullopt;
}

bool TodoClientHttp::createTodo(const std::string& worker_id, const Todo& todo)
{
    TodoRequest req(worker_id, TodoAction::CREATE, todo);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<bool>(rsp->payload))
    {
        return std::get<bool>(rsp->payload);
    }

    throw std::runtime_error("Unexpected response type for CREATE");
}

bool TodoClientHttp::modifyTodo(const std::string& worker_id, const Todo& todo)
{
    TodoRequest req(worker_id, TodoAction::MODIFY, todo);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<bool>(rsp->payload))
    {
        return std::get<bool>(rsp->payload);
    }

    throw std::runtime_error("Unexpected response type for MODIFY");
}

bool TodoClientHttp::deleteTodo(const std::string& worker_id, uint id)
{
    TodoRequest req(worker_id, TodoAction::DELETE, id);
    this->m_service->sendMessage(req);
    auto rsp = this->m_service->recvMessage();
    if (!rsp)
    {
        std::cerr << "recvMessage timeout" << std::endl;
        return {};
    }
    if (std::holds_alternative<bool>(rsp->payload))
    {
        return std::get<bool>(rsp->payload);
    }

    throw std::runtime_error("Unexpected response type for DELETE");
}

void TodoClientHttp::triggerMsgCount()
{
    this->m_msg_count += 1;
    // std::cout << "TodoClientHttp::triggerMsgCount msg_count: " << m_msg_count << std::endl;
}

void TodoClientHttp::start(int port)
{
    // uWS::App
    std::cout << "Starting TodoClientHttp server on port " << port << "..." << std::endl;

    auto t = [this, port]()
    {
        this->m_uws_app = std::make_shared<uWS::App>();

        // ================================================================================================
        // get all todos
        // ================================================================================================
        auto get_all = [this](auto* res, auto* req)
        {
            // ?worker_id=
            std::string_view worker_id = req->getQuery("worker_id");
            if (worker_id.empty())
            {
                res->writeStatus("400 Bad Request")->end("Missing required query parameter: worker_id");
                return;
            }
            auto all_todos = this->getAllTodo(std::string(worker_id));
            try
            {
                nlohmann::json j = to_json(all_todos);
                res->end(j.dump());
            }
            catch (...)
            {
                res->writeStatus("500 Internal Server Error")->end("500 Internal Server Error: An unexpected condition was encountered.");
            }
        };
        this->m_uws_app->get("/todos", get_all);

        // ================================================================================================
        // get todo
        // ================================================================================================
        auto get_todo = [this](auto* res, auto* req)
        {
            // ?worker_id=
            std::string_view worker_id = req->getQuery("worker_id");
            if (worker_id.empty())
            {
                res->writeStatus("400 Bad Request")->end("Missing required query parameter: worker_id");
                return;
            }
            auto todo_id = std::stoi(std::string(req->getParameter(0)));
            auto todo = this->getTodo(std::string(worker_id), todo_id);
            if (todo)
            {
                nlohmann::json j;
                todo.value().to_json(j);
                res->end(j.dump());
            }
            else
            {
                res->writeStatus("400 Bad Request")->end("todo_id not found: " + std::to_string(todo_id));
            }
        };
        this->m_uws_app->get("/todos/:id", get_todo);

        // ================================================================================================
        // create todo
        // ================================================================================================
        auto new_todo = [this](auto* res, auto* req)
        {
            // ?worker_id=
            std::string_view worker_id = req->getQuery("worker_id");
            if (worker_id.empty())
            {
                res->writeStatus("400 Bad Request")->end("Missing required query parameter: worker_id");
                return;
            }
            auto isAborted = std::make_shared<bool>(false);
            std::string buffer;
            auto onData = [this,
                           res,
                           isAborted,
                           worker_id,
                           buffer = std::move(buffer)](std::string_view data, bool last) mutable
            {
                buffer.append(data.data(), data.length());
                if (last)
                {
                    try
                    {
                        Todo todo = nlohmann::json::parse(buffer);
                        auto success = this->createTodo(std::string(worker_id), todo);
                        if (success)
                            res->end("success!");
                        else
                            res->end("failed!");
                    }
                    catch (nlohmann::json::exception& e)
                    {
                        res->writeStatus("400 Bad Request")->end("Invalid JSON payload");
                    }
                    catch (...)
                    {
                        res->writeStatus("500 Internal Server Error")->end("500 Internal Server Error: An unexpected condition was encountered.");
                    }
                }
            };
            res->onData(onData);
            res->onAborted([isAborted]()
                           { *isAborted = true; });
        };
        this->m_uws_app->post("/todos", new_todo);

        // ================================================================================================
        // modify todo
        // ================================================================================================
        auto modify_todo = [this](auto* res, auto* req)
        {
            // ?worker_id=
            std::string_view worker_id = req->getQuery("worker_id");
            if (worker_id.empty())
            {
                res->writeStatus("400 Bad Request")->end("Missing required query parameter: worker_id");
                return;
            }
            auto isAborted = std::make_shared<bool>(false);
            std::string buffer;
            auto onData = [this,
                           res,
                           isAborted,
                           worker_id,
                           buffer = std::move(buffer)](std::string_view data, bool last) mutable
            {
                buffer.append(data.data(), data.length());
                if (last)
                {
                    try
                    {
                        Todo todo = nlohmann::json::parse(buffer);
                        auto success = this->modifyTodo(std::string(worker_id), todo);
                        if (success)
                            res->end("success!");
                        else
                            res->end("failed!");
                    }
                    catch (nlohmann::json::exception& e)
                    {
                        res->writeStatus("400 Bad Request")->end("Invalid JSON payload");
                    }
                    catch (...)
                    {
                        res->writeStatus("500 Internal Server Error")->end("500 Internal Server Error: An unexpected condition was encountered.");
                    }
                }
            };

            res->onData(onData);
            res->onAborted([isAborted]()
                           { *isAborted = true; });
        };
        this->m_uws_app->put("/todos/:id", modify_todo);

        // ================================================================================================
        // delete todo
        // ================================================================================================
        auto delete_todo = [this](auto* res, auto* req)
        {
            // ?worker_id=
            std::string_view worker_id = req->getQuery("worker_id");
            if (worker_id.empty())
            {
                res->writeStatus("400 Bad Request")->end("Missing required query parameter: worker_id");
                return;
            }
            auto todoId = std::stoi(std::string(req->getParameter(0)));
            auto success = this->deleteTodo(std::string(worker_id), todoId);
            if (success)
                res->end("success!");
            else
                res->end("failed!");
        };
        this->m_uws_app->del("/todos/:id", delete_todo);

        // ================================================================================================
        // websockets
        // ================================================================================================
        this->m_uws_app->ws<WsData>("/*", {
                                              .open = [this](auto* ws)
                                              { this->handleWebSocketConnection(ws); },
                                              .message = [this](auto* ws, std::string_view message, uWS::OpCode)
                                              { this->handleWebSocketMessage(ws, message); },
                                              .close = [this](auto* ws, int, std::string_view)
                                              { this->handleWebSocketClose(ws); },
                                          });

        // ================================================================================================
        // run
        // ================================================================================================
        auto listen = [port](auto* token)
        {
            if (token)
            {
                std::cout << "Server listening on port " << port << "!" << std::endl;
            }
            else
            {
                std::cerr << "Failed to start server on port " << port << std::endl;
                exit(EXIT_FAILURE);
            }
        };

        this->m_uws_app->listen(port, listen).run();
    };

    // start HTTP + WebSocket server
    m_uws_t = std::thread(t);
    // start zmq DEALER + SUB
    this->m_service->start();
}

void TodoClientHttp::broadcastMessage(const std::string& topic, const std::string& message)
{
    std::string_view t = topic;
    auto loop = this->m_uws_app->getLoop();
    auto defer = [this, t, message]()
    {
        this->m_uws_app->publish(t, message, uWS::OpCode::TEXT);
    };
    loop->defer(defer);
}

// ================================================================================================

void TodoClientHttp::handleWebSocketConnection(uWS::WebSocket<false, true, WsData>* ws)
{
    auto msg = "open tid: " + getTid();
    ws->send(msg, uWS::OpCode::TEXT);
}

void TodoClientHttp::handleWebSocketMessage(uWS::WebSocket<false, true, WsData>* ws, std::string_view message)
{
    nlohmann::json request;
    try
    {
        request = nlohmann::json::parse(message);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        std::cerr << "TodoClientHttp::handleWebSocketMessage parse_error: " << e.what() << std::endl;
        return;
    }
    if (request.contains("action") && request["action"] == "subscribe" && request.contains("topic"))
    {
        std::string topic = request["topic"];
        ws->subscribe(topic);
    }
    else if (request.contains("action") && request["action"] == "unsubscribe")
    {
        ws->unsubscribe(m_sub_topic);
    }
    else if (request.contains("action") && request["action"] == "disconnect")
    {
        ws->close();
    }
}

void TodoClientHttp::handleWebSocketClose(uWS::WebSocket<false, true, WsData>* ws)
{
    auto msg = "close tid: " + getTid();
    ws->send(msg, uWS::OpCode::TEXT);
    ws->close();
}

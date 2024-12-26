/**
 * @file:	client_http.h
 * @author:	Jacob Xie
 * @date:	2024/12/26 13:48:46 Thursday
 * @brief:
 **/

#ifndef __CLIENT_HTTP__H__
#define __CLIENT_HTTP__H__

#include <uWebSockets/App.h>

#include <zmq.hpp>

#include "ClientService.hpp"
#include "adt.h"

struct WsData
{
    std::string_view user_secure_token;
};

class TodoClientHttp;

class Receiver : public IReceiver
{
public:
    void recvSubMessage(const TodoStreamResponse& message) override;

    void bindClient(TodoClientHttp* client_ptr);

private:
    TodoClientHttp* m_client_ptr;
};

class TodoClientHttp
{
public:
    TodoClientHttp(
        const std::string& client_id,
        const std::string& broker_address,
        const std::string& sub_topic,
        const std::string& sub_address);

    ~TodoClientHttp();

    std::vector<Todo> getAllTodo(const std::string& worker_id);
    std::optional<Todo> getTodo(const std::string& worker_id, uint id);
    bool createTodo(const std::string& worker_id, const Todo& todo);
    bool modifyTodo(const std::string& worker_id, const Todo& todo);
    bool deleteTodo(const std::string& worker_id, uint id);

    void triggerMsgCount();
    void broadcastMessage(const std::string& message);
    void start(int port, const std::string& worker_id);

private:
    std::string m_sub_topic;
    std::shared_ptr<ClientService<Receiver>> m_service;
    std::shared_ptr<uWS::App> m_uws_app;
    std::thread m_uws_t;
    uint m_msg_count;

    void handleWebSocketConnection(uWS::WebSocket<false, true, WsData>* ws);
    void handleWebSocketMessage(uWS::WebSocket<false, true, WsData>* ws, std::string_view message);
    void handleWebSocketClose(uWS::WebSocket<false, true, WsData>* ws);
};

#endif  //!__CLIENT_HTTP__H__

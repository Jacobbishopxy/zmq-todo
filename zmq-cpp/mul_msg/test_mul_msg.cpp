//
// Created by huaisy on 2024/12/30.
//
#include <iostream>

#include "adt.h"

int main()
{
    constexpr Todo t1 = {1, "Open the door of a refrigerator", false};
    constexpr Todo t2 = {2, "Put the elephant in the refrigerator", false};
    constexpr Todo t3 = {3, "Close the door of the refrigerator", false};
    const ResponsePayload payload = std::vector<Todo>{t1, t2, t3};

    const TodoResponse rsp = {"Jack", TodoAction::GET_ALL, payload};

    const auto zmqPackage = rsp.toZmq();

    // 打印 zmqPackage 的大小，预期为5
    std::cout << "Number of elements in zmqPackage: " << zmqPackage.size() << "\n";

    // 打印每个消息体
    for (size_t i = 0; i < zmqPackage.size(); ++i)
    {
        const auto& msg = zmqPackage[i];
        std::cout << "Message " << i << ": ";
        if (i == 0 || i == 1)
        {
            // 第一个元素为 client_id
            std::cout << std::string(static_cast<const char*>(msg.data()), msg.size()) << "\n";
        }
        else
        {
            // 其他元素为 Todos
            Todo todo {};
            std::memcpy(&todo, msg.data(), sizeof(todo));
            std::cout << "Todo{id: " << todo.id
                      << ", description: " << todo.description
                      << ", completed: " << todo.completed << "}\n";
        }
    }
}
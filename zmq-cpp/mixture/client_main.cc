/**
 * @file:	client_main.cc
 * @author:	Jacob Xie
 * @date:	2024/12/26 15:26:28 Thursday
 * @brief:
 **/

#include "client.h"


void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName
              << " --client_id=<CLIENT_ID> --worker_id=<WORKER_ID> --action=<ACTION> [--payload=<PAYLOAD>]\n";
    std::cout << "Available Actions:\n";
    std::cout << "  GET_ALL\n";
    std::cout << "  GET --payload=<Todo ID>\n";
    std::cout << "  CREATE --payload=<JSON Todo>\n";
    std::cout << "  MODIFY --payload=<JSON Todo>\n";
    std::cout << "  DELETE --payload=<Todo ID>\n";
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command-line arguments
    std::string clientArg;
    std::string workerArg;
    std::string actionArg;
    std::string payloadArg;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.rfind("--client_id=", 0) == 0)
        {
            clientArg = arg.substr(12);
        }
        else if (arg.rfind("--worker_id=", 0) == 0)
        {
            workerArg = arg.substr(12);
        }
        else if (arg.rfind("--action=", 0) == 0)
        {
            actionArg = arg.substr(9);
        }
        else if (arg.rfind("--payload=", 0) == 0)
        {
            payloadArg = arg.substr(10);
        }
    }

    if (actionArg.empty())
    {
        std::cerr << "Error: --action is required.\n";
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    TodoClient client(
        clientArg,
        EP,
        workerArg,
        BackendEP);

    std::cout << "TodoClient start" << std::endl;
    std::cout << "Broker address: " << EP << std::endl;
    std::cout << "Sub address: " << BackendEP << std::endl;

    if (actionArg == "GET_ALL")
    {
        auto todos = client.getAllTodo(workerArg);
        std::cout << "GET_ALL:" << std::endl;
        for (const auto& todo : todos)
        {
            std::cout << "ID: " << todo.id
                      << ", Description: " << todo.description
                      << ", Completed: " << (todo.completed ? "Yes" : "No") << "\n";
        }
    }
    else if (actionArg == "GET")
    {
        if (payloadArg.empty())
        {
            throw std::invalid_argument("GET requires a payload (Todo ID).");
        }
        uint id = std::stoi(payloadArg);
        std::cout << "id is: " << id << std::endl;
        auto todo = client.getTodo(workerArg, id);
        std::cout << "GET:" << std::endl;
        if (todo)
        {
            std::cout << "ID: " << todo->id
                      << ", Description: " << todo->description
                      << ", Completed: " << (todo->completed ? "Yes" : "No") << "\n";
        }
        else
        {
            std::cout << "Todo with ID " << id << " not found.\n";
        }
    }
    else if (actionArg == "CREATE")
    {
        if (payloadArg.empty())
        {
            throw std::invalid_argument("CREATE requires a payload (JSON Todo).");
        }
        std::cout << "todoJson: " << payloadArg << std::endl;
        json todoJson = json::parse(payloadArg);
        std::cout << "todoJson: " << todoJson << std::endl;
        Todo todo = Todo::from_json(todoJson);
        if (client.createTodo(workerArg, todo))
        {
            std::cout << "Todo created successfully.\n";
        }
        else
        {
            std::cout << "Failed to create Todo.\n";
        }
    }
    else if (actionArg == "MODIFY")
    {
        if (payloadArg.empty())
        {
            throw std::invalid_argument("MODIFY requires a payload (JSON Todo).");
        }
        json todoJson = json::parse(payloadArg);
        Todo todo = Todo::from_json(todoJson);
        if (client.modifyTodo(workerArg, todo))
        {
            std::cout << "Todo modified successfully.\n";
        }
        else
        {
            std::cout << "Failed to modify Todo.\n";
        }
    }
    else if (actionArg == "DELETE")
    {
        if (payloadArg.empty())
        {
            throw std::invalid_argument("DELETE requires a payload (Todo ID).");
        }
        int id = std::stoi(payloadArg);
        if (client.deleteTodo(workerArg, id))
        {
            std::cout << "Todo deleted successfully.\n";
        }
        else
        {
            std::cout << "Failed to delete Todo.\n";
        }
    }
    else
    {
        throw std::invalid_argument("Unknown action: " + actionArg);
    }

    // wait for sub messages
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return EXIT_SUCCESS;
}

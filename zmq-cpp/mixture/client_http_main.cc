/**
 * @file:	client_http_main.cc
 * @author:	Jacob Xie
 * @date:	2024/12/26 15:29:17 Thursday
 * @brief:
 **/

#include "client_http.h"

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName
              << " --client_id=<CLIENT_ID> --worker_id=<WORKER_ID>" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command-line arguments
    std::string clientArg;
    std::string workerArg;
    int port = 5556;

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
    }

    TodoClientHttp client(clientArg, EP, workerArg, BackendEP);
    client.start(port);

    std::cout << "TodoClient start" << std::endl;
    std::cout << "Broker address: " << EP << std::endl;
    std::cout << "Sub address: " << BackendEP << std::endl;
    std::cout << "Http on port: " << port << std::endl;

    // wait for sub messages
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return 0;
}

/**
 * @file:	main.cc
 * @author:	Jacob Xie
 * @date:	2025/01/10 13:53:28 Friday
 * @brief:
 **/

#include <memory>

#include "C.hpp"

// ================================================================================================
// Biz layer
// ================================================================================================

class Processor
{
public:
    virtual ~Processor() = default;

    virtual void onMessage(const std::string& source_id, const std::string& message)
    {
        throw std::logic_error("Function not implemented yet.");
    }

    void recvMsg(const std::string& source_id, const std::string& message)
    {
        // TODO: BUG!
        this->onMessage(source_id, message);
    }
};

class OmsWorker
{
public:
    OmsWorker()
    {
        service = std::make_shared<WorkerService<Processor>>();
    }

    void registerProcessor(Processor& p)
    {
        std::cout << "OmsWorker::registerProcessor p: " << typeid(p).name() << std::endl;
        service->registerProcessor(p);
    }

    void processMessage(const std::string& source_id, const std::string& message)
    {
        service->recvMsg(source_id, message);
    }

private:
    std::shared_ptr<WorkerService<Processor>> service;
};

// ================================================================================================
// App layer
// ================================================================================================

class MyProcessor : public Processor
{
public:
    void onMessage(const std::string& source_id, const std::string& message) override
    {
        std::cout << "MyProcessor handling custom message from "
                  << source_id << ": "
                  << message << std::endl;
    }
};

// ================================================================================================

int main(int argc, char** argv)
{

    OmsWorker worker;
    MyProcessor m;

    worker.registerProcessor(m);
    worker.processMessage("s", "xy");

    return 0;
}

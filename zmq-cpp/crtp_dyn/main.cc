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
        std::cout << "Processor handling message from " << source_id
                  << ": " << message << std::endl;
    }

    void recvMsg(const std::string& source_id, const std::string& message)
    {
        this->onMessage(source_id, message);
    }
};

class OmsWorker
{
public:
    OmsWorker()
        : service(std::make_unique<WorkerService<Processor>>()) {}

    void registerProcessor(Processor& p)
    {
        std::cout << "OmsWorker::registerProcessor p: " << typeid(p).name() << std::endl;
        service->registerProcessor(p);
    }

    void processMessage(const std::string& source_id, const std::string& message)
    {
        service->onMessage(source_id, message);
    }

private:
    std::unique_ptr<WorkerService<Processor>> service;
};

// ================================================================================================
// App layer
// ================================================================================================

class MyProcessor : public Processor
{
public:
    void onMessage(const std::string& source_id, const std::string& message) override
    {
        std::cout << "MyProcessor handling custom message from " << source_id
                  << ": " << message << std::endl;
    }
};

// ================================================================================================

int main(int argc, char** argv)
{
    OmsWorker worker1;
    Processor m1;

    worker1.registerProcessor(m1);
    worker1.processMessage("s1", "msg");

    // ================================================================================================
    std::cout << std::endl;

    OmsWorker worker2;
    MyProcessor m2;

    worker2.registerProcessor(m2);
    worker2.processMessage("s2", "xy");

    return 0;
}

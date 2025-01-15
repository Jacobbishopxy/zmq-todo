/**
 * @file:	C.hpp
 * @author:	Jacob Xie
 * @date:	2025/01/10 13:52:30 Friday
 * @brief:
 **/

#ifndef __C__H__
#define __C__H__

#include <iostream>
#include <memory>
#include <string>

// ================================================================================================
// template layer
// ================================================================================================

template <class ProcessorType, typename BuilderPatternReturnType>
class WorkerBuilder
{
public:
    WorkerBuilder() = default;
    WorkerBuilder(WorkerBuilder&&) noexcept = default;
    WorkerBuilder& operator=(WorkerBuilder&&) noexcept = default;

    // 禁止拷贝
    WorkerBuilder(const WorkerBuilder&) = delete;
    WorkerBuilder& operator=(const WorkerBuilder&) = delete;

    // 注册 Processor 实例
    BuilderPatternReturnType&& registerProcessor(std::shared_ptr<ProcessorType> spi)
    {
        std::cout << "WorkerBuilder::registerProcessor s: " << typeid(spi).name() << std::endl;

        m_processor = spi;
        return std::move(static_cast<BuilderPatternReturnType&&>(*this));
    }

protected:
    std::shared_ptr<ProcessorType> m_processor;
};

template <typename T>
class WorkerService : public WorkerBuilder<T, WorkerService<T>>
{
public:
    void recvMsg(const std::string& source_id, const std::string& message)
    {
        if (this->m_processor)
        {
            this->m_processor->recvMsg(source_id, message);
        }
        else
        {
            std::cout << "No processor registered." << std::endl;
        }
    }
};

#endif  //!__C__H__

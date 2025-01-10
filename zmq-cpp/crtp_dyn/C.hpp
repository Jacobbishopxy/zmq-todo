/**
 * @file:	C.hpp
 * @author:	Jacob Xie
 * @date:	2025/01/10 13:52:30 Friday
 * @brief:
 **/

#ifndef __C__H__
#define __C__H__

#include <iostream>
#include <optional>
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
    BuilderPatternReturnType&& registerProcessor(ProcessorType& spi)
    {
        std::cout << "WorkerBuilder::registerProcessor s: " << typeid(spi).name() << std::endl;

        m_processor = std::move(spi);
        return std::move(static_cast<BuilderPatternReturnType&&>(*this));
    }

protected:
    std::optional<ProcessorType> m_processor;
};

template <typename T>
class WorkerService : public WorkerBuilder<T, WorkerService<T>>
{
public:
    void recvMsg(const std::string& source_id, const std::string& message)
    {
        if (this->m_processor.has_value())
        {
            this->m_processor.value().recvMsg(source_id, message);
        }
        else
        {
            std::cout << "No processor registered." << std::endl;
        }
    }
};

#endif  //!__C__H__

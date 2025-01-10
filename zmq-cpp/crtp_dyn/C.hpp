/**
 * @file:	C.hpp
 * @author:	Jacob Xie
 * @date:	2025/01/10 13:52:30 Friday
 * @brief:
 **/

#ifndef __C__H__
#define __C__H__

#include <iostream>
#include <string>

// ================================================================================================
// template layer
// ================================================================================================

template <typename T>
class WorkerService
{
public:
    void registerProcessor(T& p)
    {
        std::cout << "WorkerService<T>::registerProcessor T: " << typeid(p).name() << std::endl;
        processor_ = &p;
    }

    void onMessage(const std::string& source_id, const std::string& message)
    {
        if (processor_)
        {
            processor_->recvMsg(source_id, message);
        }
        else
        {
            std::cout << "No processor registered." << std::endl;
        }
    }

private:
    T* processor_ = nullptr;  // Pointer to the registered processor
};

#endif  //!__C__H__

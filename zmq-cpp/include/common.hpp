/**
 * @file:	common.hpp
 * @author:	Jacob Xie
 * @date:	2024/12/08 00:19:29 Sunday
 * @brief:
 **/

#ifndef __COMMON__H__
#define __COMMON__H__

#include <string>

const std::string EP = "tcp://127.0.0.1:5555";

const std::string FrontendEP = "tcp://127.0.0.1:5565";
const std::string BackendEP = "tcp://127.0.0.1:5566";

const std::string PubSubTopic = "TA";

#endif  //!__COMMON__H__

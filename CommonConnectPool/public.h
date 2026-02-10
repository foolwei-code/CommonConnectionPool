#pragma once
#include <iostream> 
// 日志宏定义
#define LOG(str) \
std::cout << __FILE__ << ":" << __LINE__ << ":"<< __TIMESTAMP__ << ": "<< str << std::endl;
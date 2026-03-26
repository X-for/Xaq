#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include "core/value.h"

class Evaluator; // 前向声明执行器

// 实现用于 return 的异常类
struct ReturnException : public std::runtime_error
{
    Value value;
    bool has_value; 
    explicit ReturnException(Value value, bool has_value) : std::runtime_error("Function returned"), value(std::move(value)), has_value(has_value) {}
};

// 所有可调用对象的积累
class BaseCallable {
    public:
    virtual ~BaseCallable() = default;
    // 核心动作：接收执行器和参数列表，返回一个运算结果
    virtual Value call(Evaluator* evaluator, const std::vector<Value>& arguments) = 0;
    
    virtual std::string to_string() const = 0;
};

// value.h

#pragma once
#include "value_type.h"
#include <variant>
#include <string>
#include <iostream>


class BaseCallable; // 前向声明可调用对象

class Value
{
private:
    ValueType type_;
    std::variant<std::monostate, int64_t, double, bool, std::string, std::shared_ptr<BaseCallable>> value_;

public:
    Value();
    explicit Value(int64_t v);            // int
    explicit Value(double v);             // float
    explicit Value(bool v);               // bool
    explicit Value(const std::string &v); // string
    
    
    Value(std::shared_ptr<BaseCallable> callable) : type_(ValueType::Callable), value_(std::move(callable)) {} // callable
    
    ValueType get_type() const { return type_; }; // get type of value
    friend std::ostream &operator<<(std::ostream &os, const Value &v);
    int64_t as_int() const
    {
        if (type_ == ValueType::Int)
            return std::get<int64_t>(value_);
        throw std::runtime_error("Value is not an int");
    }
    double as_float() const
    {
        if (type_ == ValueType::Float)
            return std::get<double>(value_);
        // 允许整数隐式转浮点数
        if (type_ == ValueType::Int)
            return static_cast<double>(std::get<int64_t>(value_));
        throw std::runtime_error("Value is not a Float.");
    }

    bool is_truthy() const
    {
        if (type_ == ValueType::Null)
            return false;
        if (type_ == ValueType::Bool)
            return std::get<bool>(value_);
        return true; // 其他情况默认当做 true 处理
    }

    std::shared_ptr<BaseCallable> as_callable() const {
        if (type_ == ValueType::Callable) return std::get<std::shared_ptr<BaseCallable>>(value_);
        throw std::runtime_error("Value is not callable.");
    }
};
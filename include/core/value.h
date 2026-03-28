// value.h

#pragma once
#include <variant>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

class BaseCallable; // 前向声明可调用对象

enum class ValueType
{
    Null,
    Int,
    Float,
    Bool,
    String,
    Callable,
    Array,
    Object
};

class Value
{
private:
    ValueType type_;
    std::variant<std::monostate, int64_t, double, bool, std::string, std::shared_ptr<BaseCallable>, std::shared_ptr<std::vector<Value>>> value_;

public:
    Value();
    explicit Value(int64_t v);            // int
    explicit Value(double v);             // float
    explicit Value(bool v);               // bool
    explicit Value(const std::string &v); // string

    Value(std::shared_ptr<BaseCallable> callable) : type_(ValueType::Callable), value_(std::move(callable)) {} // callable

    Value(std::shared_ptr<std::vector<Value>> array) : type_(ValueType::Array), value_(std::move(array)) {} // array

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
    std::string as_string() const
    {
        if (type_ == ValueType::String)
            return std::get<std::string>(value_);
        throw std::runtime_error("Value is not a String.");
    }
    bool is_truthy() const
    {
        if (type_ == ValueType::Null)
            return false;
        if (type_ == ValueType::Bool)
            return std::get<bool>(value_);
        return true; // 其他情况默认当做 true 处理
    }

    std::shared_ptr<BaseCallable> as_callable() const
    {
        if (type_ == ValueType::Callable)
            return std::get<std::shared_ptr<BaseCallable>>(value_);
        throw std::runtime_error("Value is not callable.");
    }

    std::shared_ptr<std::vector<Value>> as_array() const
    {
        if (type_ == ValueType::Array)
            return std::get<std::shared_ptr<std::vector<Value>>>(value_);
        throw std::runtime_error("Value is not an Array.");
    }
};
#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "core/value.h"

class Environment {
private:
    std::unordered_map<std::string, Value> values_;
    std::shared_ptr<Environment> enclosing_; // 指向外层作用域的指针

public:
    // 构造函数：如果是全局环境，enclosing 就是 nullptr
    explicit Environment(std::shared_ptr<Environment> enclosing = nullptr);

    // 声明新变量 (例如: auto x = 1)
    void define(const std::string& name, const Value& value);

    // 获取变量的值 (例如: print(x))
    Value get(const std::string& name) const;

    // 给已存在的变量重新赋值 (例如: x = 2)
    void assign(const std::string& name, const Value& value);
};
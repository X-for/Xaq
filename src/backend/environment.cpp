#include "backend/environment.h"

Environment::Environment(std::shared_ptr<Environment> enclosing)
    : enclosing_(std::move(enclosing)) {}

// define 永远只在当前局部作用域新建变量
void Environment::define(const std::string& name, const Value& value) {
    values_[name] = value;
}

// get 会沿着作用域链一直往外找
Value Environment::get(const std::string& name) const {
    // 1. 先在当前作用域找
    auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }

    // 2. 如果没找到，且存在外层作用域，就去外层找
    if (enclosing_ != nullptr) {
        return enclosing_->get(name);
    }

    // 3. 彻底找不到，抛出运行时异常
    throw std::runtime_error("Undefined variable '" + name + "'.");
}

// assign 也需要沿着作用域链找，确保修改的是正确的那个变量
void Environment::assign(const std::string& name, const Value& value) {
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = value;
        return;
    }

    if (enclosing_ != nullptr) {
        enclosing_->assign(name, value);
        return;
    }

    throw std::runtime_error("Undefined variable '" + name + "'.");
}
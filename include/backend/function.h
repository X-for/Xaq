#pragma once
#include <functional>
#include <memory>

#include "backend/environment.h"
#include "core/callable.h"
#include "frontend/ast.h"

class Function : public BaseCallable {
private:
    FunctionStmt* declaration_; // 存储函数声明的 AST 节点
    std::shared_ptr<Environment> closure_; // 存储定义时的环境 (闭包)
public:
    Function(FunctionStmt* declaration, std::shared_ptr<Environment> closure)
        : declaration_(declaration)
        , closure_(std::move(closure)) { }

    Value call(Evaluator* evaluator, const std::vector<Value>& arguments) override;

    std::string to_string() const override { return "<func " + declaration_->name.lexeme + ">"; }
};

class NativeFunction : public BaseCallable {
private:
    int arity_; // 参数个数
    std::function<Value(const std::vector<Value>&)> body_;

public:
    NativeFunction(int arity, std::function<Value(const std::vector<Value>&)> body)
        : arity_(std::move(arity))
        , body_(std::move(body)) { }
    int arity() const { return arity_; }
    Value call(Evaluator* evaluator, const std::vector<Value>& arguments) override {
        return body_(arguments); // 直接执行绑定的 C++ 逻辑
    }

    std::string to_string() const override { return "<native fn>"; }
};
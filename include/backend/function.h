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

class BaseClass : public BaseCallable {
public:
    std::string name;
    ClassStmt* declaration; // 保存类的 AST 节点，后面实例化时需要从中提取属性和方法

    BaseClass(std::string name, ClassStmt* declaration)
        : name(std::move(name)), declaration(declaration) {}

    int arity() const override { 
        return 0; // 暂时写死为 0，等我们做构造函数时再改
    }

    Value call(Evaluator* evaluator, const std::vector<Value>& arguments) override {
        // TODO: 这里将来要负责 new 一个 XaqInstance 对象出来并返回
        return Value(); 
    }

    std::string to_string() const override { 
        return "<class " + name + ">"; 
    }
};
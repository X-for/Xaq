#pragma once
#include "core/callable.h"
#include "frontend/ast.h"
#include "backend/environment.h"
#include <memory>

class Function : public BaseCallable
{
private:
    FunctionStmt *declaration_;            // 存储函数声明的 AST 节点
    std::shared_ptr<Environment> closure_; // 存储定义时的环境 (闭包)
public:
    Function(FunctionStmt *declaration, std::shared_ptr<Environment> closure)
        : declaration_(declaration), closure_(std::move(closure)) {}

    Value call(Evaluator *evaluator, const std::vector<Value> &arguments) override;

    std::string to_string() const override
    {
        return "<func " + declaration_->name.lexeme + ">";
    }

};
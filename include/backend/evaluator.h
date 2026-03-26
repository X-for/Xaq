#pragma once
#include "frontend/ast.h"
#include "core/value.h"
#include "backend/environment.h"
#include <memory>
#include <vector>
#include <stdexcept>

class Evaluator {
private:
    std::shared_ptr<Environment> environment_; // 执行器的作用域
public:
    // 初始化全局环境
    Evaluator() {
        environment_ = std::make_shared<Environment>();
    };

    // 核心求值入口
    void evaluate(const std::vector<std::unique_ptr<Stmt>>& statements);

    // 执行单条语句
    void execute(Stmt *stmt);

    Value evaluate(Expr* expr);
    void execute_block(const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> block_env); // 处理 block 语句

private:
    // 语句执行
    void execute_var_decl(VarDeclStmt* stmt);
    void execute_expression_stmt(ExpressionStmt* stmt);
    void execute_block(BlockStmt* stmt, std::shared_ptr<Environment> block_env);
    void execute_function(FunctionStmt* stmt); // 把函数存入内存
    void execute_return(ReturnStmt* stmt); // 处理 return 语句

    // 针对每种 AST 节点的具体执行逻辑
    Value evaluate_variable(VariableExpr* expr);
    Value evaluate_literal(LiteralExpr* expr);
    Value evaluate_binary(BinaryExpr* expr);
    Value evaluate_logical(LogicalExpr* expr);
    Value evaluate_assign(AssignExpr* expr);
    Value evaluate_call(CallExpr* expr); // 执行函数调用

};
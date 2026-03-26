#pragma once
#include "frontend/ast.h"
#include "core/value.h"
#include <memory>
#include <stdexcept>

class Evaluator {
public:
    Evaluator() = default;

    // 核心求值入口
    Value evaluate(Expr* expr);

private:
    // 针对每种 AST 节点的具体执行逻辑
    Value evaluate_literal(LiteralExpr* expr);
    Value evaluate_binary(BinaryExpr* expr);
};
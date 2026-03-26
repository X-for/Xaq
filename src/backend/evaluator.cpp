#include "backend/evaluator.h"

Value Evaluator::evaluate(Expr *expr)
{
    if (!expr)
        throw std::runtime_error("Cannot evaluate null expression.");

    // 使用 RTTI 动态类型转换来路由节点
    if (auto lit = dynamic_cast<LiteralExpr *>(expr))
    {
        return evaluate_literal(lit);
    }
    else if (auto bin = dynamic_cast<BinaryExpr *>(expr))
    {
        return evaluate_binary(bin);
    } else if (auto log = dynamic_cast<LogicalExpr *>(expr)) {
        return evaluate_logical(log);
    }

    throw std::runtime_error("Unknown AST node type in evaluator.");
}

// 1. 执行字面量：最底层的节点，直接把包在里面的 Value 丢出去即可
Value Evaluator::evaluate_literal(LiteralExpr *expr)
{
    return expr->value;
}

// 2. 执行二元运算：先算左边，再算右边，最后结合
Value Evaluator::evaluate_binary(BinaryExpr *expr)
{
    // 递归求值左子树和右子树
    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());

    // 算术运算符
    if (expr->op.type == TokenType::PLUS)
    { // +
        return Value(left.as_int() + right.as_int());
    }
    else if (expr->op.type == TokenType::MINUS)
    { // -
        return Value(left.as_int() - right.as_int());
    }
    else if (expr->op.type == TokenType::STAR)
    { // *
        return Value(left.as_int() * right.as_int());
    }
    else if (expr->op.type == TokenType::SLASH)
    { // /
        int64_t r_val = right.as_int();
        if (r_val == 0)
        {
            throw std::runtime_error("Division by zero.");
        }
        return Value(left.as_int() / right.as_int());
    }
    else if (expr->op.type == TokenType::PERCENT)
    { // %
        int64_t r_val = right.as_int();
        if (r_val == 0)
            throw std::runtime_error("Modulo by zero.");
        return Value(left.as_int() % r_val); // 取模运算
    }

    // 比较运算符
    else if (expr->op.type == TokenType::GREATER)
    {
        return Value(left.as_int() > right.as_int());
    }
    else if (expr->op.type == TokenType::GREATER_EQUAL)
    {
        return Value(left.as_int() >= right.as_int());
    }
    else if (expr->op.type == TokenType::LESS)
    {
        return Value(left.as_int() < right.as_int());
    }
    else if (expr->op.type == TokenType::LESS_EQUAL)
    {
        return Value(left.as_int() <= right.as_int());
    }
    else if (expr->op.type == TokenType::EQUAL_EQUAL)
    {
        // 如果两边都是布尔值，按布尔值比较
        if (left.get_type() == ValueType::Bool && right.get_type() == ValueType::Bool)
        {
            return Value(left.is_truthy() == right.is_truthy());
        }
        // 否则按整数比较 (后续可以继续扩展 Float 的比较)
        return Value(left.as_int() == right.as_int());
    }
    else if (expr->op.type == TokenType::BANG_EQUAL)
    {
        if (left.get_type() == ValueType::Bool && right.get_type() == ValueType::Bool)
        {
            return Value(left.is_truthy() != right.is_truthy());
        }
        return Value(left.as_int() != right.as_int());
    }

    // 位运算符
    else if (expr->op.type == TokenType::AMPERSAND)
    {
        return Value(left.as_int() & right.as_int());
    }
    else if (expr->op.type == TokenType::PIPE)
    {
        return Value(left.as_int() | right.as_int());
    }
    else if (expr->op.type == TokenType::LESS_LESS)
    {
        return Value(left.as_int() << right.as_int());
    }
    else if (expr->op.type == TokenType::GREATER_GREATER)
    {
        return Value(left.as_int() >> right.as_int());
    }

    throw std::runtime_error("Unsupported binary operator:" + expr->op.lexeme);
}

Value Evaluator::evaluate_logical(LogicalExpr* expr) {
    // 关键点：只先求值左边！
    Value left = evaluate(expr->left.get());

    if (expr->op.type == TokenType::PIPE_PIPE) {
        // 对于 ||，如果左边是 true，直接返回，右边连看都不看
        if (left.is_truthy()) return left;
    } 
    else if (expr->op.type == TokenType::AMPERSAND_AMPERSAND) {
        // 对于 &&，如果左边是 false，直接返回，右边连看都不看
        if (!left.is_truthy()) return left;
    }

    // 只有当左边无法决定结果时，才去求值右边
    return evaluate(expr->right.get());
}
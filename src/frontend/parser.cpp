#include "frontend/parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

// --- 辅助方法 ---
Token Parser::peek() const { return tokens_[current_]; }
Token Parser::previous() const { return tokens_[current_ - 1]; }
bool Parser::is_at_end() const { return peek().type == TokenType::END_OF_FILE; }
Token Parser::advance()
{
    if (!is_at_end())
        current_++;
    return previous();
}
bool Parser::check(TokenType type) const
{
    if (is_at_end())
        return false;
    return peek().type == type;
}
bool Parser::match(std::initializer_list<TokenType> types)
{
    for (TokenType type : types)
    {
        if (check(type))
        {
            advance();
            return true;
        }
    }
    return false;
}
Token Parser::consume(TokenType type, const std::string &message)
{
    if (check(type))
        return advance();
    throw std::runtime_error(message); // 简单的错误处理
}

// ==========================================
// 递归下降解析：从低优先级到高优先级
// ==========================================

// 入口点：解析一个完整的表达式
std::unique_ptr<Expr> Parser::expression()
{
    return logical_or();
}

std::unique_ptr<Expr> Parser::logical_or() {
    auto expr = logical_and();

    while (match({TokenType::PIPE_PIPE})) {
        Token op = previous();
        auto right = logical_and();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logical_and() {
    auto expr = equality(); // 交接给比较运算符

    while (match({TokenType::AMPERSAND_AMPERSAND})) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

// 解析 ==, !=
std::unique_ptr<Expr> Parser::equality()
{
    auto expr = comparison();

    while (match({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL}))
    {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

// 解析 >, <, >=, <=
std::unique_ptr<Expr> Parser::comparison()
{
    auto expr = term(); // term 对应加减法

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL}))
    {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

// 解析 +, -
std::unique_ptr<Expr> Parser::term()
{
    auto expr = factor(); // factor 对应乘除法

    while (match({TokenType::MINUS, TokenType::PLUS}))
    {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

// 解析 *, /, %
std::unique_ptr<Expr> Parser::factor()
{
    auto expr = call(); // 目前直接跳到字面量

    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT}))
    {
        Token op = previous();
        auto right = primary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::call() {
    auto expr = primary(); // 先获取最左边的基础元素（比如 math, arr）

    while (true) { 
        if (match({TokenType::LEFT_PAREN})) {
            // 解析函数调用: func_name(arg1, arg2)
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    args.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::unique_ptr<Expr>(new CallExpr(std::move(expr), std::move(args)));
        } 
        else if (match({TokenType::DOT})) {
            // 解析属性访问: object.name
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            expr = std::unique_ptr<Expr>(new GetExpr(std::move(expr), std::move(name)));
        }
        else if (match({TokenType::LEFT_BRACKET})) {
            // 解析数组索引: arr[0]
            auto index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
            expr = std::unique_ptr<Expr>(new IndexExpr(std::move(expr), std::move(index)));
        }
        else {
            break; // 如果都不是，就退出循环
        }
    }

    return expr;
}

// 解析最底层的字面量和括号
std::unique_ptr<Expr> Parser::primary()
{
    if (match({TokenType::FALSE_LITERAL}))
        return std::make_unique<LiteralExpr>(Value(false));
    if (match({TokenType::TRUE_LITERAL}))
        return std::make_unique<LiteralExpr>(Value(true));
    if (match({TokenType::NULL_LITERAL}))
        return std::make_unique<LiteralExpr>(Value());

    if (match({TokenType::NUMBER})) {
        std::string text = previous().lexeme;
        // 如果包含小数点，解析为 Float
        if (text.find('.') != std::string::npos) {
            return std::make_unique<LiteralExpr>(Value(std::stod(text)));
        } 
        // 否则解析为 Int
        else {
            return std::make_unique<LiteralExpr>(Value(static_cast<int64_t>(std::stoll(text))));
        }
    }

    if (match({TokenType::STRING}))
    {
        return std::make_unique<LiteralExpr>(Value(previous().lexeme));
    }
    if (match({TokenType::IDENTIFIER}))
    {
        return std::make_unique<VariableExpr>(previous());
    }
    // 解析数组 [1, 2, a + b]
    if (match({TokenType::LEFT_BRACKET}))
    {
        std::vector<std::unique_ptr<Expr>> elements;

        // 如果不是直接遇到了右括号 `]`，就说明数组里有内容
        if (!check(TokenType::RIGHT_BRACKET))
        {
            do
            {
                elements.push_back(expression()); // 递归解析每一个元素（允许复杂的表达式）
            } while (match({TokenType::COMMA})); // 只要遇到逗号，就继续解析下一个
        }

        consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

    if (match({TokenType::LEFT_PAREN}))
    {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        // 注意：这里需要一个 GroupingExpr 节点来包裹括号里的表达式，为了简单，我们可以先直接返回 expr
        return expr;
    }

    throw std::runtime_error("Expect expression.");
}

// 公开的调用接口
std::unique_ptr<Expr> Parser::parse()
{
    try
    {
        return expression();
    }
    catch (const std::runtime_error &error)
    {
        std::cerr << "Parse Error: " << error.what() << "\n";
        return nullptr;
    }
}
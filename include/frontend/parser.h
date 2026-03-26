#pragma once
#include <vector>
#include <memory>
#include "frontend/token.h"
#include "frontend/ast.h"

class Parser {
private:
    std::vector<Token> tokens_;
    int current_ = 0; // 当前正在查看的 Token 索引

    // --- 辅助方法（和 Lexer 很像） ---
    Token peek() const;
    Token previous() const;
    bool is_at_end() const;
    bool check(TokenType type) const;
    Token advance();
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);

    // --- 递归下降的核心：解析各种语法结构 ---
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> primary(); // 解析最基础的字面量或括号
    std::unique_ptr<Expr> logical_or();
    std::unique_ptr<Expr> logical_and();

public:
    explicit Parser(std::vector<Token> tokens);

    // 启动解析，返回一个表示整个表达式或语句的 AST 节点
    std::unique_ptr<Expr> parse(); 
};
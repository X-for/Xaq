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
    Token peek_next() const;
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

    // 语法解析的路由方法
    std::unique_ptr<Stmt> declaration();         // 判断是声明变量还是普通语句
    std::unique_ptr<Stmt> var_declaration();     // 处理 auto x = 42
    std::unique_ptr<Stmt> statement();           // 处理普通语句
    std::unique_ptr<Stmt> expression_statement();// 处理独立的表达式 (如 1 + 2)
    std::unique_ptr<Expr> assignment();          // 处理赋值表达式 (如 x = 1 + 2)

    std::vector<std::unique_ptr<Stmt>> block(); // 处理块级作用域 { ... }
    std::unique_ptr<Stmt> function(const std::string& kind); // 处理函数和方法声明 

public:
    explicit Parser(std::vector<Token> tokens);

    // 启动解析，返回一个表示整个表达式或语句的 AST 节点
    std::vector<std::unique_ptr<Stmt>> parse(); 
};
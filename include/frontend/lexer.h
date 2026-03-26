#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "frontend/token.h"

class Lexer {
private:
    std::string source_;                // 存放整个 Xaq 脚本的源码文本
    std::vector<Token> tokens_;         // 解析后生成的 Token 列表
    
    int start_ = 0;                     // 当前正在解析的 Token 的起始字符位置
    int current_ = 0;                   // 当前扫描器所在的字符位置
    int line_ = 1;                      // 当前行号（用于报错追踪）

    // Xaq 语言的关键字映射表
    static const std::unordered_map<std::string, TokenType> keywords_;

    // --- 内部辅助方法 ---
    bool is_at_end() const;
    char advance();
    char peek() const;
    char peek_next() const;
    bool match(char expected);

    // --- 各种类型 Token 的解析逻辑 ---
    void scan_token();
    void string_literal();              // 处理常规字符串和 """ 多行字符串
    void number_literal();              // 处理 Int 和 Float
    void identifier();                  // 处理变量名、类名、关键字等
    void block_comment();               // 处理 /* */ 多行注释
    void add_token(TokenType type);     // <--- 补上这一行声明                                              
    void doc_string();                  // 处理 """ 文档字符串
public:
    explicit Lexer(std::string source);
    
    // 核心入口：开始扫描并返回所有 Token
    std::vector<Token> scan_tokens();
};
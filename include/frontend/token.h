#pragma once
#include <string>
#include <iostream>

enum class TokenType {
    // 1. 单字符符号
    LEFT_PAREN, RIGHT_PAREN,     // ( )
    LEFT_BRACE, RIGHT_BRACE,     // { }
    LEFT_BRACKET, RIGHT_BRACKET, // [ ]
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, COLON, // , . - + ; / * :
    EQUAL, NOT, LESS, GREATER,     // = ! < >

    // 2. 双字符符号
    BANG_EQUAL, EQUAL_EQUAL,     // != ==
    GREATER_EQUAL, LESS_EQUAL,   // >= <=
    PLUS_EQUAL, MINUS_EQUAL,     // += -=
    ARROW,                       // ->

        // 补充的符号
    PERCENT, PERCENT_EQUAL,      // % %=
    STAR_STAR, STAR_STAR_EQUAL,  // ** **=
    COLON_EQUAL,                 // := (海象运算符)
    AMPERSAND, PIPE, CARET, TILDE, // & | ^ ~ (位运算)
    LESS_LESS, GREATER_GREATER,  // << >> (位移)
    AMPERSAND_AMPERSAND, PIPE_PIPE, // && || (逻辑运算)
    LESS_LESS_EQUAL, GREATER_GREATER_EQUAL, // <<= >>= (位移赋值)
    
    // 补充的关键字
    TRY, CATCH, FINALLY, THROW,  // 异常处理

    // 3. 字面量
    IDENTIFIER, STRING, NUMBER, 

    // 4. Xaq 关键字
    AUTO, FUNC, RETURN, IF, ELSE, FOR, IN, 
    CLASS, METHOD, OPERATOR, THIS, PRIVATE, 
    IMPORT, AS, TRUE_LITERAL, FALSE_LITERAL, NULL_LITERAL,

    // 5. 特殊标记
    END_OF_FILE, 


};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    Token(TokenType type, std::string lexeme, int line)
        : type(type), lexeme(std::move(lexeme)), line(line) {}
};
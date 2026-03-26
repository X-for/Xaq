#include "frontend/parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

// --- 辅助方法 ---
Token Parser::peek() const { return tokens_[current_]; }
Token Parser::peek_next() const
{
    if (current_ + 1 >= tokens_.size())
        return tokens_.back();
    return tokens_[current_ + 1];
}
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
    return assignment();
}

std::unique_ptr<Expr> Parser::logical_or()
{
    auto expr = logical_and();

    while (match({TokenType::PIPE_PIPE}))
    {
        Token op = previous();
        auto right = logical_and();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logical_and()
{
    auto expr = equality(); // 交接给比较运算符

    while (match({TokenType::AMPERSAND_AMPERSAND}))
    {
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

std::unique_ptr<Expr> Parser::call()
{
    auto expr = primary(); // 先获取最左边的基础元素（比如 math, arr）

    while (true)
    {
        if (match({TokenType::LEFT_PAREN}))
        {
            // 解析函数调用: func_name(arg1, arg2)
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RIGHT_PAREN))
            {
                do
                {
                    args.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::unique_ptr<Expr>(new CallExpr(std::move(expr), std::move(args)));
        }
        else if (match({TokenType::DOT}))
        {
            // 解析属性访问: object.name
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            expr = std::unique_ptr<Expr>(new GetExpr(std::move(expr), std::move(name)));
        }
        else if (match({TokenType::LEFT_BRACKET}))
        {
            // 解析数组索引: arr[0]
            auto index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
            expr = std::unique_ptr<Expr>(new IndexExpr(std::move(expr), std::move(index)));
        }
        else
        {
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

    if (match({TokenType::NUMBER}))
    {
        std::string text = previous().lexeme;
        // 如果包含小数点，解析为 Float
        if (text.find('.') != std::string::npos)
        {
            return std::make_unique<LiteralExpr>(Value(std::stod(text)));
        }
        // 否则解析为 Int
        else
        {
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

// ==========================================
// 语句解析 (Statements)
// ==========================================

// 路由分支：遇到 auto 关键字就去声明变量，否则当做普通语句
std::unique_ptr<Stmt> Parser::declaration()
{
    try
    {
        if (match({TokenType::AUTO}))
        {
            return var_declaration();
        }
        if (check(TokenType::IDENTIFIER) && peek_next().type == TokenType::COLON)
        {
            return var_declaration();
        }
        return statement();
    }
    catch (const std::runtime_error &error)
    {
        std::cerr << "Parse Error: " << error.what() << "\n";
        // 遇到错误时，为了防止死循环，强制让指针前进一步 (实际工程中这里需要更复杂的错误恢复机制)
        advance();
        return nullptr;
    }
}

// 解析变量声明: auto x1 = 42
std::unique_ptr<Stmt> Parser::var_declaration()
{
    // 1. 匹配变量名
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name after 'auto'.");

    // --- 新增：处理类型标注 (如 : Int) ---
    if (match({TokenType::COLON}))
    {
        // 由于我们的 Evaluator 目前是动态类型的，这里我们暂时只“吞掉”类型名称
        consume(TokenType::IDENTIFIER, "Expect type name after ':'.");
    }

    std::unique_ptr<Expr> initializer = nullptr;
    // 2. 如果有 '='，则解析后面的初始化表达式
    if (match({TokenType::EQUAL}))
    {
        initializer = expression();
    }

    // 注意：Xaq 语法中没有强制要求分号结尾，所以我们这里暂不强制校验分号

    return std::make_unique<VarDeclStmt>(std::move(name), std::move(initializer));
}

// 解析普通语句
std::unique_ptr<Stmt> Parser::statement()
{
    if (match({TokenType::LEFT_BRACE}))
    {
        return std::make_unique<BlockStmt>(block());
    }
    return expression_statement();
}

// 将普通的数学运算/函数调用包装成一个“语句节点”
std::unique_ptr<Stmt> Parser::expression_statement()
{
    auto expr = expression();
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

// 实现赋值表达式解析
std::unique_ptr<Expr> Parser::assignment()
{
    auto expr = logical_or(); // 先解析左边的表达式

    if (match({
            TokenType::EQUAL,
            TokenType::PLUS_EQUAL,
            TokenType::MINUS_EQUAL,
            TokenType::STAR_STAR_EQUAL,
            TokenType::SLASH_EQUAL,
            TokenType::PERCENT_EQUAL,
        }))
    {
        Token op = previous();
        auto value = assignment();

        if (auto var_expr = dynamic_cast<VariableExpr *>(expr.get()))
        {
            Token name = var_expr->name;
            return std::make_unique<AssignExpr>(std::move(name), std::move(op), std::move(value));
        }
        throw std::runtime_error("Invalid assignment target.");
    }
    return expr;
}

// block() 方法的实现
std::vector<std::unique_ptr<Stmt>> Parser::block()
{
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !is_at_end())
    {
        statements.push_back(declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}


// function() 方法的实现
std::unique_ptr<Stmt> Parser::function(const std::string& kind) {
    Token keyword = previous(); // 记录是 func 还是 method
    Token name = consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");
    
    // ==========================================
    // 1. 解析参数列表 (形参)
    // ==========================================
    consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");
    std::vector<ParamDecl> parameters;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            // 1.1 获取参数名
            Token param_name = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            
            // 1.2 解析可选的类型标注 (如 : Int)
            std::string type_name = "";
            if (match({TokenType::COLON})) {
                type_name = consume(TokenType::IDENTIFIER, "Expect type name.").lexeme;
            }
            
            // 1.3 解析可选的默认值表达式 (如 = 0)
            std::unique_ptr<Expr> default_value = nullptr;
            if (match({TokenType::EQUAL})) {
                default_value = expression();
            }
            
            // 将完整的参数信息打包推入数组
            parameters.push_back({param_name, type_name, std::move(default_value)});
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    // ==========================================
    // 2. 解析返回签名 (->)
    // ==========================================
    std::vector<ParamDecl> return_vars;
    
    // 解析返回签名的两种形式: -> Int 或 -> (ret: Int, err: String)
    if (match({TokenType::ARROW})) { 
        if (match({TokenType::LEFT_PAREN})) {
            // 解析多变量具名返回: -> (ret: Int = 0, err: String)
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    Token ret_name = consume(TokenType::IDENTIFIER, "Expect return variable name.");
                    
                    std::string type_name = "";
                    if (match({TokenType::COLON})) {
                        type_name = consume(TokenType::IDENTIFIER, "Expect type name.").lexeme;
                    }
                    
                    std::unique_ptr<Expr> default_value = nullptr;
                    if (match({TokenType::EQUAL})) {
                        default_value = expression();
                    }
                    
                    return_vars.push_back({ret_name, type_name, std::move(default_value)});
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after return signature.");
        } else {
            // 解析单返回类型: -> Int
            Token type_token = consume(TokenType::IDENTIFIER, "Expect return type after '->'.");
            // 对于匿名单返回类型，我们克隆一个 Token 做占位，但清空它的名字
            Token dummy_name = type_token;
            dummy_name.lexeme = ""; 
            return_vars.push_back({dummy_name, type_token.lexeme, nullptr});
        }
    }

    // ==========================================
    // 3. 解析函数体 (Block)
    // ==========================================
    consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
    std::vector<std::unique_ptr<Stmt>> body = block(); 

    return std::make_unique<FunctionStmt>(std::move(keyword), std::move(name), 
                                          std::move(parameters), std::move(return_vars), std::move(body));
}

// 公开的调用接口
std::vector<std::unique_ptr<Stmt>> Parser::parse()
{
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!is_at_end())
    {
        statements.push_back(declaration());
    }
    return statements;
}
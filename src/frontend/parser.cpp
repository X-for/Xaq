#include "frontend/parser.h"
#include <iostream>
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) { }

// --- 辅助方法 ---
Token Parser::peek() const { return tokens_[current_]; }
Token Parser::peek_next() const {
    if (current_ + 1 >= tokens_.size())
        return tokens_.back();
    return tokens_[current_ + 1];
}
Token Parser::previous() const { return tokens_[current_ - 1]; }
bool Parser::is_at_end() const { return peek().type == TokenType::END_OF_FILE; }
Token Parser::advance() {
    if (!is_at_end())
        current_++;
    return previous();
}
bool Parser::check(TokenType type) const {
    if (is_at_end())
        return false;
    return peek().type == type;
}
bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type))
        return advance();
    throw std::runtime_error(message); // 简单的错误处理
}

bool Parser::is_inline_func_condition() const {
    if (peek().type != TokenType::IDENTIFIER)
        return false;
    if (peek_next().type != TokenType::LEFT_PAREN)
        return false;

    int temp = current_ + 2;
    int parens = 1;
    while (temp < tokens_.size() && parens > 0) {
        if (tokens_[temp].type == TokenType::LEFT_PAREN)
            parens++;
        if (tokens_[temp].type == TokenType::RIGHT_PAREN)
            parens--;
        temp++;
    }
    if (temp < tokens_.size()) {
        if (tokens_[temp].type == TokenType::ARROW || tokens_[temp].type == TokenType::LEFT_BRACE)
            return true;
        // 兼容 Lexer 可能把 -> 拆解成了 MINUS 和 GREATER 的情况
        if (tokens_[temp].type == TokenType::MINUS && temp + 1 < tokens_.size()
            && tokens_[temp + 1].type == TokenType::GREATER)
            return true;
    }
    return false;
}

// 探路：判断是否是 for i in ... 或者 for i, v in ...
bool Parser::is_for_in_loop() const {
    if (peek().type != TokenType::IDENTIFIER)
        return false;
    int temp = current_ + 1;
    if (temp < tokens_.size() && tokens_[temp].type == TokenType::COMMA) {
        temp += 2; // 跳过 COMMA 和 第二个变量
    }
    if (temp < tokens_.size() && tokens_[temp].type == TokenType::IN) {
        return true;
    }
    return false;
}

// ==========================================
// 递归下降解析：从低优先级到高优先级
// ==========================================

// 入口点：解析一个完整的表达式
std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::logical_or() {
    auto expr = logical_and();

    while (match({ TokenType::PIPE_PIPE })) {
        Token op = previous();
        auto right = logical_and();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logical_and() {
    auto expr = equality(); // 交接给比较运算符

    while (match({ TokenType::AMPERSAND_AMPERSAND })) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

// 解析 ==, !=
std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();

    while (match({ TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL })) {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

// 解析 >, <, >=, <=
std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term(); // term 对应加减法

    while (match({ TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL })) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

// 解析 +, -
std::unique_ptr<Expr> Parser::term() {
    auto expr = factor(); // factor 对应乘除法

    while (match({ TokenType::MINUS, TokenType::PLUS })) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

// 解析 *, /, %
std::unique_ptr<Expr> Parser::factor() {
    auto expr = unary(); // 目前直接跳到字面量

    while (match({ TokenType::SLASH, TokenType::STAR, TokenType::PERCENT })) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    // 匹配一元前缀: 负号(-)、逻辑非(!)、位非(~)、正号(+)
    if (match({ TokenType::MINUS, TokenType::NOT, TokenType::TILDE, TokenType::PLUS })) {
        Token op = previous();
        auto right = unary(); // 递归调用，支持 !!true 这种写法
        return std::make_unique<UnaryExpr>(std::move(op), std::move(right));
    }
    return call(); // 如果不是前缀符号，移交给下一级 call()
}

std::unique_ptr<Expr> Parser::call() {
    auto expr = primary(); // 先获取最左边的基础元素（比如 math, arr）

    while (true) {
        if (match({ TokenType::LEFT_PAREN })) {
            // 解析函数调用: func_name(arg1, arg2)
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    args.push_back(expression());
                } while (match({ TokenType::COMMA }));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::unique_ptr<Expr>(new CallExpr(std::move(expr), std::move(args)));
        } else if (match({ TokenType::DOT })) {
            // 解析属性访问: object.name
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            expr = std::unique_ptr<Expr>(new GetExpr(std::move(expr), std::move(name)));
        } else if (match({ TokenType::LEFT_BRACKET })) {
            // 解析数组索引: arr[0]
            auto index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
            expr = std::unique_ptr<Expr>(new IndexExpr(std::move(expr), std::move(index)));
        } else {
            break; // 如果都不是，就退出循环
        }
    }

    return expr;
}

// 解析最底层的字面量和括号
std::unique_ptr<Expr> Parser::primary() {
    if (match({ TokenType::FALSE_LITERAL }))
        return std::make_unique<LiteralExpr>(Value(false));
    if (match({ TokenType::TRUE_LITERAL }))
        return std::make_unique<LiteralExpr>(Value(true));
    if (match({ TokenType::NULL_LITERAL }))
        return std::make_unique<LiteralExpr>(Value());

    if (match({ TokenType::NUMBER })) {
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

    if (match({ TokenType::STRING })) {
        return std::make_unique<LiteralExpr>(Value(previous().lexeme));
    }
    if (match({ TokenType::IDENTIFIER })) {
        return std::make_unique<VariableExpr>(previous());
    }
    // 解析数组 [1, 2, a + b]
    if (match({ TokenType::LEFT_BRACKET })) {
        std::vector<std::unique_ptr<Expr>> elements;

        // 如果不是直接遇到了右括号 `]`，就说明数组里有内容
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(expression()); // 递归解析每一个元素（允许复杂的表达式）
            } while (match({ TokenType::COMMA })); // 只要遇到逗号，就继续解析下一个
        }

        consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

    if (match({ TokenType::LEFT_PAREN })) {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        // 注意：这里需要一个 GroupingExpr 节点来包裹括号里的表达式，为了简单，我们可以先直接返回 expr
        return expr;
    }

    // 处理this
    if (match({ TokenType::THIS })) {
        return std::make_unique<VariableExpr>(previous());
    }

    throw std::runtime_error("Expect expression.");
}

// ==========================================
// 语句解析 (Statements)
// ==========================================

// 路由分支：遇到 auto 关键字就去声明变量，否则当做普通语句
std::unique_ptr<Stmt> Parser::declaration() {
    try {
        if (match({ TokenType::FUNC })) {
            return function("function");
        }
        if (match({ TokenType::RETURN })) {
            return return_statement();
        }
        if (match({ TokenType::AUTO })) {
            return var_declaration();
        }
        if (check(TokenType::IDENTIFIER) && peek_next().type == TokenType::COLON) {
            return var_declaration();
        }

        return statement();
    } catch (const std::runtime_error& error) {
        std::cerr << "Parse Error: " << error.what() << "\n";
        // 遇到错误时，为了防止死循环，强制让指针前进一步 (实际工程中这里需要更复杂的错误恢复机制)
        advance();
        return nullptr;
    }
}

// 解析变量声明: auto x1 = 42
std::unique_ptr<Stmt> Parser::var_declaration() {
    std::vector<Token> names;

    // 1. 解析以逗号分隔的变量名（以及可选的类型）
    do {
        names.push_back(consume(TokenType::IDENTIFIER, "Expect variable name."));
        if (match({ TokenType::COLON })) {
            consume(TokenType::IDENTIFIER, "Expect type name.");
            if (match({ TokenType::LEFT_BRACKET })) {
                while (!check(TokenType::RIGHT_BRACKET) && !is_at_end())
                    advance();
                consume(TokenType::RIGHT_BRACKET, "Expect ']' after generic type.");
            }
        }
    } while (match({ TokenType::COMMA }));

    // 2. 解析可选的等号和以逗号分隔的初始化表达式
    std::vector<std::unique_ptr<Expr>> initializers;
    if (match({ TokenType::EQUAL })) {
        do {
            initializers.push_back(expression());
        } while (match({ TokenType::COMMA }));
    }

    // 3. 组装 AST (将多变量声明打包成一个 BlockStmt)
    std::vector<std::unique_ptr<Stmt>> decls;
    for (size_t i = 0; i < names.size(); ++i) {
        std::unique_ptr<Expr> init = nullptr;
        // 如果初始化表达式较少，后面的变量就保持默认 Null
        if (i < initializers.size() && initializers[i] != nullptr) {
            init = std::move(initializers[i]);
        }
        decls.push_back(std::make_unique<VarDeclStmt>(names[i], std::move(init)));
    }

    if (decls.size() == 1) {
        return std::move(decls[0]);
    }
    return std::make_unique<MultiVarDeclStmt>(std::move(decls));
}

// 解析普通语句
std::unique_ptr<Stmt> Parser::statement() {
    // return
    if (match({ TokenType::RETURN })) {
        return return_statement();
    }
    // if
    if (match({ TokenType::IF })) {
        return if_statement();
    }
    // for
    if (match({ TokenType::FOR })) {
        return for_statement();
    }

    if (match({ TokenType::LEFT_BRACE })) {
        return std::make_unique<BlockStmt>(block());
    }
    if (match({ TokenType::CLASS })) {
        return class_declaration();
    }
    return expression_statement();
}

// 将普通的数学运算/函数调用包装成一个“语句节点”
std::unique_ptr<Stmt> Parser::expression_statement() {
    auto expr = expression();
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

// 实现赋值表达式解析
std::unique_ptr<Expr> Parser::assignment() {
    auto expr = logical_or(); // 先解析左边的表达式

    if (match({
            TokenType::EQUAL, // =
            TokenType::PLUS_EQUAL, // +=
            TokenType::MINUS_EQUAL, // -=
            TokenType::STAR_EQUAL, // *=
            TokenType::SLASH_EQUAL, // /=
            TokenType::PERCENT_EQUAL, // %=
            TokenType::STAR_STAR_EQUAL, // **=
            TokenType::COLON_EQUAL, // := (海象运算符)
        })) {
        Token op = previous();
        auto value = assignment();

        if (auto var_expr = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = var_expr->name;
            return std::make_unique<AssignExpr>(std::move(name), std::move(op), std::move(value));
        }
        throw std::runtime_error("Invalid assignment target.");
    }
    return expr;
}

// block() 方法的实现
std::vector<std::unique_ptr<Stmt>> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
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
            if (match({ TokenType::COLON })) {
                type_name = consume(TokenType::IDENTIFIER, "Expect type name.").lexeme;
            }

            // 1.3 解析可选的默认值表达式 (如 = 0)
            std::unique_ptr<Expr> default_value = nullptr;
            if (match({ TokenType::EQUAL })) {
                default_value = expression();
            }

            // 将完整的参数信息打包推入数组
            parameters.push_back({ param_name, type_name, std::move(default_value) });
        } while (match({ TokenType::COMMA }));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    // ==========================================
    // 2. 解析返回签名 (->)
    // ==========================================
    std::vector<ParamDecl> return_vars;

    // 解析返回签名的两种形式: -> Int 或 -> (ret: Int, err: String)
    if (match({ TokenType::ARROW })) {
        if (match({ TokenType::LEFT_PAREN })) {
            // 解析多变量具名返回: -> (ret: Int = 0, err: String)
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    Token ret_name = consume(TokenType::IDENTIFIER, "Expect return variable name.");

                    std::string type_name = "";
                    if (match({ TokenType::COLON })) {
                        type_name = consume(TokenType::IDENTIFIER, "Expect type name.").lexeme;
                    }

                    std::unique_ptr<Expr> default_value = nullptr;
                    if (match({ TokenType::EQUAL })) {
                        default_value = expression();
                    }

                    return_vars.push_back({ ret_name, type_name, std::move(default_value) });
                } while (match({ TokenType::COMMA }));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after return signature.");
        } else {
            // 解析单返回类型: -> Int
            Token type_token = consume(TokenType::IDENTIFIER, "Expect return type after '->'.");
            // 对于匿名单返回类型，我们克隆一个 Token 做占位，但清空它的名字
            Token dummy_name = type_token;
            dummy_name.lexeme = "";
            return_vars.push_back({ dummy_name, type_token.lexeme, nullptr });
        }
    }

    // ==========================================
    // 3. 解析函数体 (Block)
    // ==========================================
    consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
    std::vector<std::unique_ptr<Stmt>> body = block();

    return std::make_unique<FunctionStmt>(
        std::move(keyword), std::move(name), std::move(parameters), std::move(return_vars), std::move(body));
}

std::unique_ptr<Stmt> Parser::return_statement() {
    Token keyword = previous(); // 记录下return
    std::vector<std::unique_ptr<Expr>> values;
    if (!check(TokenType::RIGHT_BRACE)) { // 如果 return 后面不是直接跟着 `}`, 就说明有返回值
        do {
            values.push_back(expression());
        } while (match({ TokenType::COMMA })); // 支持 return a, b, c
    }
    return std::make_unique<ReturnStmt>(std::move(keyword), std::move(values));
}

// 核心的 for 解析路由
std::unique_ptr<Stmt> Parser::for_statement() {
    // 1. 判断并解析 For-In 循环
    if (is_for_in_loop()) {
        std::vector<Token> vars;
        vars.push_back(consume(TokenType::IDENTIFIER, "Expect variable name."));
        if (match({ TokenType::COMMA })) {
            vars.push_back(consume(TokenType::IDENTIFIER, "Expect second variable name."));
        }
        consume(TokenType::IN, "Expect 'in' after for variables.");

        std::vector<std::unique_ptr<Expr>> iterables;
        do {
            iterables.push_back(expression());
        } while (match({ TokenType::COMMA }));

        consume(TokenType::LEFT_BRACE, "Expect '{' before for body.");
        auto body = std::make_unique<BlockStmt>(block());
        return std::make_unique<ForInStmt>(std::move(vars), std::move(iterables), std::move(body));
    }

    // 2. 否则，先吃掉第一个表达式 (可能是条件，也可能是 C 风格的初始化赋值)
    auto first_clause = expression();

    // 3. 判断是否是 C 风格的 for 循环 (带有分号)
    if (match({ TokenType::SEMICOLON })) {
        auto condition = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
        auto increment = expression();

        consume(TokenType::LEFT_BRACE, "Expect '{' after for clauses.");
        auto body = std::make_unique<BlockStmt>(block());

        // 【魔法解包】把 C 风格的 for 解包成 { 初始化; while (条件) { 循环体; 步进; } }
        std::vector<std::unique_ptr<Stmt>> new_body_stmts;
        new_body_stmts.push_back(std::move(body));
        new_body_stmts.push_back(std::make_unique<ExpressionStmt>(std::move(increment)));
        auto new_body = std::make_unique<BlockStmt>(std::move(new_body_stmts));

        auto while_loop = std::make_unique<ForStmt>(std::move(condition), std::move(new_body));

        std::vector<std::unique_ptr<Stmt>> outer_stmts;
        outer_stmts.push_back(std::make_unique<ExpressionStmt>(std::move(first_clause)));
        outer_stmts.push_back(std::move(while_loop));

        // 外部包裹一个匿名的 BlockStmt 可以确保 i=0 这类变量不会泄露到 for 外部 (完美还原了作用域!)
        return std::make_unique<BlockStmt>(std::move(outer_stmts));
    } else {
        // 4. 退回最基础的条件循环 (类似 while)
        consume(TokenType::LEFT_BRACE, "Expect '{' after for condition.");
        auto body = std::make_unique<BlockStmt>(block());
        return std::make_unique<ForStmt>(std::move(first_clause), std::move(body));
    }
}

// 核心的 if 解析逻辑
std::unique_ptr<Stmt> Parser::if_statement() {
    std::unique_ptr<Stmt> init_stmt = nullptr;
    std::unique_ptr<Expr> condition = nullptr;

    // 1. 处理内联函数条件 (语法糖解包)
    if (is_inline_func_condition()) {
        Token func_name = advance(); // 获取函数名
        consume(TokenType::LEFT_PAREN, "Expect '('");

        std::vector<ParamDecl> params;
        std::vector<std::unique_ptr<Expr>> call_args; // 同步构建调用参数

        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                Token param_name = consume(TokenType::IDENTIFIER, "Expect parameter name.");
                params.push_back({ param_name, "", nullptr });
                // 将形参名作为变量表达式传入，以便后续生成 CallExpr
                call_args.push_back(std::make_unique<VariableExpr>(param_name));
                if (match({ TokenType::COLON }))
                    consume(TokenType::IDENTIFIER, "Expect type name.");
            } while (match({ TokenType::COMMA }));
        }
        consume(TokenType::RIGHT_PAREN, "Expect ')'");

        // 简化处理：无视返回签名长什么样，直接快进到函数体的 {
        while (!check(TokenType::LEFT_BRACE) && !is_at_end()) {
            advance();
        }

        consume(TokenType::LEFT_BRACE, "Expect '{' before inline function body.");
        auto func_body = block(); // 解析函数的 body

        // 构造 FunctionStmt 存入 init_stmt
        Token fake_kw(TokenType::FUNC, "func", func_name.line);
        std::vector<ParamDecl> empty_returns; // 省略返回变量
        init_stmt = std::make_unique<FunctionStmt>(
            fake_kw, func_name, std::move(params), std::move(empty_returns), std::move(func_body));
        // 将条件设置为对刚刚定义的函数的调用
        condition = std::make_unique<CallExpr>(std::make_unique<VariableExpr>(func_name), std::move(call_args));
    } else {
        // 普通的条件表达式
        condition = expression();
    }

    // 2. 解析 if 的主分支 block (这里会消耗掉第二个 '{')
    consume(TokenType::LEFT_BRACE, "Expect '{' after if condition.");
    auto then_branch = std::make_unique<BlockStmt>(block());

    // 3. 处理 else 分支 (支持省略 if)
    std::unique_ptr<Stmt> else_branch = nullptr;
    if (match({ TokenType::ELSE })) {
        if (check(TokenType::LEFT_BRACE)) {
            // 纯 else: else { ... }
            consume(TokenType::LEFT_BRACE, "Expect '{' after else.");
            else_branch = std::make_unique<BlockStmt>(block());
        } else {
            // 支持 else x == y {...} 和 else if x == y {...}
            if (match({ TokenType::IF })) { } // 如果有 if，主动吃掉
            else_branch = if_statement(); // 递归解析
        }
    }

    return std::make_unique<IfStmt>(
        std::move(init_stmt), std::move(condition), std::move(then_branch), std::move(else_branch));
}

std::unique_ptr<Stmt> Parser::class_declaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect class name.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

    std::vector<std::unique_ptr<Stmt>> properties;
    std::vector<std::unique_ptr<Stmt>> methods;

    while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
        if (match({ TokenType::METHOD })) {
            // 如果遇到 method 关键字，解析为方法
            methods.push_back(method_declaration());
        } else {
            // 否则复用你已经完善的多变量/单变量声明逻辑
            properties.push_back(var_declaration());
        }
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");

    return std::make_unique<ClassStmt>(std::move(name), std::move(properties), std::move(methods));
}

std::unique_ptr<Stmt> Parser::method_declaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect method name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after method name.");

    std::vector<ParamDecl> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token param_name = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.push_back({ param_name, "", nullptr });
            // 兼容可能存在的参数类型标注 (例如 x: Int)
            if (match({ TokenType::COLON }))
                consume(TokenType::IDENTIFIER, "Expect type name.");
        } while (match({ TokenType::COMMA }));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    // 简化处理：无视返回签名，直接快进到方法体的 {
    while (!check(TokenType::LEFT_BRACE) && !is_at_end()) {
        advance();
    }

    consume(TokenType::LEFT_BRACE, "Expect '{' before method body.");
    auto body = block();

    // 伪造一个 FUNC token 用于构建函数节点
    Token fake_kw(TokenType::FUNC, "func", name.line);
    std::vector<ParamDecl> empty_returns;

    return std::make_unique<FunctionStmt>(
        fake_kw, std::move(name), std::move(params), std::move(empty_returns), std::move(body));
}

// 公开的调用接口
std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!is_at_end()) {
        statements.push_back(declaration());
    }
    return statements;
}

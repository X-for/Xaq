#include <iostream>
#include <string>
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "backend/evaluator.h" // 新增：引入执行器

void run_code(const std::string& code) {
    std::cout << "--- 源码 ---\n" << code << "\n\n";

    // 1. 词法分析 (Lexing)
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.scan_tokens();

    // 2. 语法分析 (Parsing)
    Parser parser(tokens);
    std::unique_ptr<Expr> ast = parser.parse();

    // 3. 打印抽象语法树 (AST)
    std::cout << "--- AST 结构 ---\n";
    if (ast) {
        std::cout << ast->to_string() << "\n";
        
        // ==========================================
        // 4. 执行引擎 (Evaluation) - 新增部分
        // ==========================================
        std::cout << "\n--- 执行结果 ---\n";
        try {
            Evaluator evaluator;
            Value result = evaluator.evaluate(ast.get());
            std::cout << "=> " << result << "\n";
        } catch (const std::runtime_error& e) {
            std::cerr << "运行时错误: " << e.what() << "\n";
        }
        
    } else {
        std::cout << "解析失败，AST 为空。\n";
    }
    std::cout << "----------------\n\n";
}

int main() {
    // 测试 1: 基础加减乘除与优先级
    run_code("1 + 2 * 3 - 4 / 2");

    // 测试 2: 比较运算符
    run_code("10 >= 5 == true");

    // 测试 3: 逻辑运算符
    run_code("false && 1 / 0"); // 这里应该短路，不会抛出除零错误
    run_code("true || false && true");  

    return 0;
}
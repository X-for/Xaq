#include <iostream>
#include <string>
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "backend/evaluator.h"

void run_code(const std::string& code) {
    std::cout << "--- 源码 ---\n" << code << "\n\n";

    Lexer lexer(code);
    std::vector<Token> tokens = lexer.scan_tokens();

    Parser parser(tokens);
    // 接收完整的语句列表
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

    std::cout << "--- AST 结构 ---\n";
    for (const auto& stmt : statements) {
        if (stmt) std::cout << stmt->to_string() << "\n";
    }
    
    std::cout << "\n--- 执行结果 ---\n";
    try {
        Evaluator evaluator;
        // 把所有的语句交给执行器
        evaluator.evaluate(statements); 
    } catch (const std::runtime_error& e) {
        std::cerr << "运行时错误: " << e.what() << "\n";
    }
    std::cout << "----------------\n\n";
}

int main() {
    // 测试：声明变量，并使用变量进行计算
    run_code(
        "x: Int = 10 \n"
        "auto y = x * 2 \n"
        "x=5\n"
        "x"
    );

    run_code(
        "a: Int = 5 \n"
        "b: Int = 3 \n"
        "a = 10"
        "a > b && b < 10 || a == 0"
    );

    return 0;
}
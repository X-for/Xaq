#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "backend/evaluator.h"

void run(const std::string& code) {
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.scan_tokens();

    Parser parser(tokens);
    // 接收完整的语句列表
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

    for (const auto& stmt : statements) {
        if (stmt) std::cout << stmt->to_string() << "\n";
    }
    
    try {
        Evaluator evaluator;
        // 把所有的语句交给执行器
        evaluator.evaluate(statements); 
    } catch (const std::runtime_error& e) {
        std::cerr << "runtime error: " << e.what() << "\n";
    }
}

void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        exit(74); // 74 是标准 IO 错误码
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    // 将整个文件内容传递给核心逻辑
    run(buffer.str());
    
    // 注意：如果 run() 内部发生了语法或运行时错误，通常在这里需要检查错误标志并 exit()。
}

void runPrompt() {
    std::string line;
    for (;;) {
        std::cout << "> ";
        // 遇到 EOF (Windows 下 Ctrl+Z, Linux/Mac 下 Ctrl+D) 时 getline 会返回 false
        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break; 
        }
        
        // 将单行输入传递给核心逻辑
        run(line);
        
        // 注意：在 REPL 模式下，单行错误不应该导致程序退出，
        // 这里通常需要重置你的全局或实例错误标志位。
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Usage: my_program [script_path]" << std::endl;
        return 64; 
    } else if (argc == 2) {
        // 如果带了一个参数，当作文件路径执行
        runFile(argv[1]);
    } else {
        // 如果没有带参数，进入 REPL 交互模式
        runPrompt();
    }
    return 0;
}
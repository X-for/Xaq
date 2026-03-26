#pragma once
#include "frontend/token.h"
#include "core/value.h"
#include <memory>
#include <vector>
#include <sstream>

// ==========================================
// 1. 基类定义
// ==========================================

// 表达式基类 (会产生 Value)
struct Expr
{
    virtual ~Expr() = default;
    virtual std::string to_string() const = 0; // 方便调试的字符串表示
};

// 语句基类 (执行动作，不产生 Value)
struct Stmt
{
    virtual ~Stmt() = default;
};

// ==========================================
// 2. 核心表达式节点 (Expressions)
// ==========================================

// 字面量表达式 (例如: 42, "Hello", true)
struct LiteralExpr : public Expr
{
    Value value; // 直接复用我们之前写的核心 Value 类型

    explicit LiteralExpr(Value value) : value(std::move(value)) {}

    std::string to_string() const override {
        std::ostringstream oss;
        
        // 如果是字符串，我们在 AST 打印时给它套上双引号以便区分
        if (value.get_type() == ValueType::String) {
            oss << "\"" << value << "\""; 
        } else {
            // 其他类型（数字、布尔、Null）直接复用 Value 的 << 重载
            oss << value;
        }
        
        return oss.str();
    }
};

// 变量访问表达式 (例如: x, arr)
struct VariableExpr : public Expr {
    Token name;

    explicit VariableExpr(Token name) : name(std::move(name)) {}

    std::string to_string() const override {
        return name.lexeme; // 直接打印变量名
    }
};

// 数组字面量表达式 (例如: [1, 2, 3])
struct ArrayExpr : public Expr {
    std::vector<std::unique_ptr<Expr>> elements; // 存放数组里的每一个元素表达式

    explicit ArrayExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {}

    std::string to_string() const override {
        std::string res = "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            res += elements[i]->to_string();
            if (i < elements.size() - 1) res += " "; // AST 打印时用空格隔开
        }
        res += "]";
        return res;
    }
};

// 二元运算表达式 (例如: a + b, x == y)
struct BinaryExpr : public Expr
{
    std::unique_ptr<Expr> left;  // 左操作数
    Token op;                    // 操作符 (如 PLUS, EQUAL_EQUAL)
    std::unique_ptr<Expr> right; // 右操作数

    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    std::string to_string() const override {
        return "(" + op.lexeme + " " + left->to_string() + " " + right->to_string() + ")";
    }
};

// ==========================================
// 3. 核心语句节点 (Statements)
// ==========================================

// 变量声明语句 (例如: auto x1 = 42)
struct VarDeclStmt : public Stmt
{
    Token name;                        // 变量名
    std::unique_ptr<Expr> initializer; // 初始化表达式 (可能为空，如 x3: Bool)
    // 提示：未来这里可以加一个 Token type_annotation 用于处理显式类型标注

    VarDeclStmt(Token name, std::unique_ptr<Expr> initializer)
        : name(std::move(name)), initializer(std::move(initializer)) {}
};

// 表达式语句 (例如: 单独的一行 print("Hello"); 或是单独的方法调用 s1.age())
struct ExpressionStmt : public Stmt
{
    std::unique_ptr<Expr> expression;

    explicit ExpressionStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}
};

//  CallExpr
struct CallExpr : public Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}

    std::string to_string() const override {
        std::string res = "(call " + callee->to_string();
        for (const auto& arg : arguments) res += " " + arg->to_string();
        return res + ")";
    }
};

// 模块导入语句 (例如: import c "func1.c" as func1_c)
struct ImportStmt : public Stmt {
    Token lang_type;   // 外部语言类型 (例如 'c', 'cpp', 'py'。如果是原生 Xaq 模块，这个 Token 可以标记为为空或特殊类型)
    Token module_path; // 模块路径或名称 (例如 "func1.c" 或 math)
    Token alias;       // 别名 (例如 func1_c。如果没有别名，则与 module_path 核心名一致)

    ImportStmt(Token lang_type, Token module_path, Token alias)
        : lang_type(std::move(lang_type)), module_path(std::move(module_path)), alias(std::move(alias)) {}
};

// 属性/方法访问表达式 (例如: math.add 或 s1.age)
struct GetExpr : public Expr {
    std::unique_ptr<Expr> object; // 左边的对象 (如 math)
    Token name;                   // 右边的属性名 (如 add)

    GetExpr(std::unique_ptr<Expr> object, Token name)
        : object(std::move(object)), name(std::move(name)) {}

    std::string to_string() const override {
        return "(get " + object->to_string() + " " + name.lexeme + ")";
    }
};

// 索引访问表达式 (例如: arr[0])
struct IndexExpr : public Expr {
    std::unique_ptr<Expr> object; // 左边的对象 (如 arr)
    std::unique_ptr<Expr> index;  // 括号里的索引表达式 (如 0)

    IndexExpr(std::unique_ptr<Expr> object, std::unique_ptr<Expr> index)
        : object(std::move(object)), index(std::move(index)) {}

    std::string to_string() const override {
        return "(index " + object->to_string() + " " + index->to_string() + ")";
    }
};


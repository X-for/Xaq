#include "backend/evaluator.h"
#include "backend/function.h"
#include "fstream"

// 假设这是你的 Evaluator 构造函数
Evaluator::Evaluator() {
    environment_ = std::make_shared<Environment>();

    // 在 Evaluator 构造函数的最开头，解绑 C/C++ IO 流同步，极大提升读写速度
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // 1. 挂载底层打印函数 __print__ (限定只能输出字符串)
    environment_->define("__print__", Value(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& args) {
        // 调用 as_string() 提取原生 C++ 字符串，避免 operator<< 自带的引号
        // 如果传入的 args[0] 不是 String 类型，as_string() 会自动抛出类型错误
        std::cout << args[0].as_string() << '\n';
        return Value();
    })));

    // 2. 挂载底层输入函数 __input__ (限定只能读取并返回字符串)
    environment_->define("__input__", Value(std::make_shared<NativeFunction>(0, [](const std::vector<Value>& args) {
        std::string line;
        std::getline(std::cin, line);
        // 直接封装为 String 类型的 Value 返回
        return Value(line);
    })));

    // 3. 注入文件 IO 流：read_file 函数
    environment_->define("read_file", Value(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& args) {
        std::string path = args[0].as_string();
        std::ifstream file(path);
        if (!file.is_open())
            throw std::runtime_error("Could not open file.");
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return Value(content); // 把文件内容作为字符串返回
    })));
}

// ==========================================
// 1. 语句执行入口与路由
// ==========================================

void Evaluator::evaluate(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        if (stmt)
            execute(stmt.get());
    }
}

void Evaluator::execute(Stmt* stmt) {
    if (auto var_decl = dynamic_cast<VarDeclStmt*>(stmt)) {
        execute_var_decl(var_decl);
    } else if (auto expr_stmt = dynamic_cast<ExpressionStmt*>(stmt)) {
        execute_expression_stmt(expr_stmt);
    } else if (auto block_stmt = dynamic_cast<BlockStmt*>(stmt)) {
        execute_block(block_stmt, std::make_shared<Environment>(environment_));
    } else if (auto func_stmt = dynamic_cast<FunctionStmt*>(stmt)) {
        execute_function(func_stmt);
    } else if (auto return_stmt = dynamic_cast<ReturnStmt*>(stmt)) {
        execute_return(return_stmt);
    } else if (auto if_stmt = dynamic_cast<IfStmt*>(stmt)) {
        execute_if(if_stmt);
    } else if (auto for_stmt = dynamic_cast<ForStmt*>(stmt)) {
        execute_for(for_stmt);
    } else if (auto multi_var_decl = dynamic_cast<MultiVarDeclStmt*>(stmt)) {
        execute_multi_var_decl(multi_var_decl);
    } else if (auto for_in_stmt = dynamic_cast<ForInStmt*>(stmt)) {
        execute_for_in(for_in_stmt);
    } else if (auto class_stmt = dynamic_cast<ClassStmt*>(stmt)) {
        execute_class(class_stmt);
    }

    else {
        throw std::runtime_error("Unknown statement type in evaluator.");
    }
}

// ==========================================
// 2. 具体的语句执行逻辑
// ==========================================

// 执行变量声明：auto x = 42
void Evaluator::execute_var_decl(VarDeclStmt* stmt) {
    Value value = Value(); // 默认赋值为 Null

    // 如果等号右边有初始化表达式，就先把它算出来
    if (stmt->initializer != nullptr) {
        value = evaluate(stmt->initializer.get());
    }

    // 将变量名和算出的值存入当前环境 (内存)
    environment_->define(stmt->name.lexeme, value);
}

// 执行普通表达式语句
void Evaluator::execute_expression_stmt(ExpressionStmt* stmt) { Value result = evaluate(stmt->expression.get()); }

// ==========================================
// 3. 表达式求值路由补充 (读取变量)
// ==========================================

Value Evaluator::evaluate(Expr* expr) {
    if (!expr)
        throw std::runtime_error("Cannot evaluate null expression.");

    // 使用 RTTI 动态类型转换来路由节点
    if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
        return evaluate_literal(lit);
    } else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        return evaluate_binary(bin);
    } else if (auto log = dynamic_cast<LogicalExpr*>(expr)) {
        return evaluate_logical(log);
    } else if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        return evaluate_variable(var);
    } else if (auto assign = dynamic_cast<AssignExpr*>(expr)) {
        return evaluate_assign(assign);
    } else if (auto call_expr = dynamic_cast<CallExpr*>(expr)) {
        return evaluate_call(call_expr);
    } else if (auto unary = dynamic_cast<UnaryExpr*>(expr)) {
        return evaluate_unary(unary);
    } else if (auto index_expr = dynamic_cast<IndexExpr*>(expr)) {
        return evaluate_index(index_expr);
    } else if (auto arr_expr = dynamic_cast<ArrayExpr*>(expr)) {
        return evaluate_array(arr_expr);
    }
    throw std::runtime_error("Unknown AST node type in evaluator.");
}

// 1. 执行字面量：最底层的节点，直接把包在里面的 Value 丢出去即可
Value Evaluator::evaluate_literal(LiteralExpr* expr) { return expr->value; }

// 实现一元运算
Value Evaluator::evaluate_unary(UnaryExpr* expr) {
    Value right = evaluate(expr->right.get());
    if (expr->op.type == TokenType::MINUS) {
        if (right.get_type() == ValueType::Float)
            return Value(-right.as_float());
        return Value(-right.as_int());
    } else if (expr->op.type == TokenType::NOT) {
        return Value(!right.is_truthy());
    }
    throw std::runtime_error("Unsupported unary operator.");
}

Value Evaluator::evaluate_array(ArrayExpr* expr) {
    auto elements = std::make_shared<std::vector<Value>>();
    for (const auto& el : expr->elements) {
        // 递归求值数组里的每一个元素
        elements->push_back(evaluate(el.get()));
    }
    return Value(elements); // 包装成带 std::shared_ptr 的 Value 并返回
}

// 实现数组索引访问
Value Evaluator::evaluate_index(IndexExpr* expr) {
    Value obj = evaluate(expr->object.get()); // 算出左边是个啥 (比如 arr)
    Value index = evaluate(expr->index.get()); // 算出中括号里是个啥 (比如 i)

    if (obj.get_type() == ValueType::Array) {
        auto arr = obj.as_array();
        int64_t idx = index.as_int();

        // 支持类似 Python 的倒数索引 arr[-1]
        if (idx < 0)
            idx += arr->size();

        if (idx >= 0 && idx < arr->size()) {
            return (*arr)[idx];
        }
        throw std::runtime_error("Array index out of bounds.");
    }
    // TODO: 后续可以在这里继续加上 Dict 类型和 String 类型的索引访问支持
    throw std::runtime_error("Only arrays support indexing right now.");
}

// 2. 执行二元运算：先算左边，再算右边，最后结合
Value Evaluator::evaluate_binary(BinaryExpr* expr) {
    // 递归求值左子树和右子树
    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());

    // 算术运算符
    if (expr->op.type == TokenType::PLUS) { // +
        return Value(left.as_int() + right.as_int());
    } else if (expr->op.type == TokenType::MINUS) { // -
        return Value(left.as_int() - right.as_int());
    } else if (expr->op.type == TokenType::STAR) { // *
        return Value(left.as_int() * right.as_int());
    } else if (expr->op.type == TokenType::SLASH) { // /
        int64_t r_val = right.as_int();
        if (r_val == 0) {
            throw std::runtime_error("Division by zero.");
        }
        return Value(left.as_int() / right.as_int());
    } else if (expr->op.type == TokenType::PERCENT) { // %
        int64_t r_val = right.as_int();
        if (r_val == 0)
            throw std::runtime_error("Modulo by zero.");
        return Value(left.as_int() % r_val); // 取模运算
    }

    // 比较运算符
    else if (expr->op.type == TokenType::GREATER) {
        return Value(left.as_int() > right.as_int());
    } else if (expr->op.type == TokenType::GREATER_EQUAL) {
        return Value(left.as_int() >= right.as_int());
    } else if (expr->op.type == TokenType::LESS) {
        return Value(left.as_int() < right.as_int());
    } else if (expr->op.type == TokenType::LESS_EQUAL) {
        return Value(left.as_int() <= right.as_int());
    } else if (expr->op.type == TokenType::EQUAL_EQUAL) {
        // 如果两边都是布尔值，按布尔值比较
        if (left.get_type() == ValueType::Bool && right.get_type() == ValueType::Bool) {
            return Value(left.is_truthy() == right.is_truthy());
        }
        // 否则按整数比较 (后续可以继续扩展 Float 的比较)
        return Value(left.as_int() == right.as_int());
    } else if (expr->op.type == TokenType::BANG_EQUAL) {
        if (left.get_type() == ValueType::Bool && right.get_type() == ValueType::Bool) {
            return Value(left.is_truthy() != right.is_truthy());
        }
        return Value(left.as_int() != right.as_int());
    }

    // 位运算符
    else if (expr->op.type == TokenType::AMPERSAND) {
        return Value(left.as_int() & right.as_int());
    } else if (expr->op.type == TokenType::PIPE) {
        return Value(left.as_int() | right.as_int());
    } else if (expr->op.type == TokenType::LESS_LESS) {
        return Value(left.as_int() << right.as_int());
    } else if (expr->op.type == TokenType::GREATER_GREATER) {
        return Value(left.as_int() >> right.as_int());
    }

    throw std::runtime_error("Unsupported binary operator:" + expr->op.lexeme);
}

Value Evaluator::evaluate_logical(LogicalExpr* expr) {
    // 关键点：只先求值左边！
    Value left = evaluate(expr->left.get());

    if (expr->op.type == TokenType::PIPE_PIPE) {
        // 对于 ||，如果左边是 true，直接返回，右边连看都不看
        if (left.is_truthy())
            return left;
    } else if (expr->op.type == TokenType::AMPERSAND_AMPERSAND) {
        // 对于 &&，如果左边是 false，直接返回，右边连看都不看
        if (!left.is_truthy())
            return left;
    }

    // 只有当左边无法决定结果时，才去求值右边
    return evaluate(expr->right.get());
}

Value Evaluator::evaluate_assign(AssignExpr* expr) {
    // 1. 先把等号右边的值算出来
    Value right_val = evaluate(expr->value.get());

    // 2. 如果是普通的 =，直接存入 Environment
    if (expr->op.type == TokenType::EQUAL) {
        try {
            // 尝试赋值给已存在的变量（会在当前及外层作用域链中查找）
            environment_->assign(expr->name.lexeme, right_val);
        } catch (const std::runtime_error& e) {
            // 如果报错（找不到该变量），则作为隐式声明存入当前作用域
            environment_->define(expr->name.lexeme, right_val);
        }
        return right_val;
    }

    // 3. 处理复合赋值 (+=, -=, *= 等)
    // 必须先从 Environment 里把老的值拿出来
    Value current_val = environment_->get(expr->name.lexeme);
    Value new_val = Value();

    if (expr->op.type == TokenType::PLUS_EQUAL) {
        new_val = Value(current_val.as_int() + right_val.as_int());
    } else if (expr->op.type == TokenType::MINUS_EQUAL) {
        new_val = Value(current_val.as_int() - right_val.as_int());
    } else if (expr->op.type == TokenType::STAR_EQUAL) {
        new_val = Value(current_val.as_int() * right_val.as_int());
    }
    // ... 还可以继续补充 /= 和 %=

    // 把算出的新值塞回 Environment
    environment_->assign(expr->name.lexeme, new_val);
    return new_val; // 赋值表达式本身也返回这个新值
}

// 执行代码块
void Evaluator::execute_block(BlockStmt* stmt, std::shared_ptr<Environment> block_env) {
    std::shared_ptr<Environment> previous = environment_; // 先保存当前环境
    try {
        environment_ = block_env; // 将执行器的环境切换到新的局部环境
        for (const auto& statement : stmt->statements) {
            if (statement)
                execute(statement.get());
        }
    } catch (...) {
        environment_ = previous; // 报错也切换回原环境
        throw; // 继续往上抛异常
    }
    environment_ = previous; // 执行完毕后切换回原环境
}

// 读取变量：直接向 Environment 要数据
Value Evaluator::evaluate_variable(VariableExpr* expr) { return environment_->get(expr->name.lexeme); }

void Evaluator::execute_multi_var_decl(MultiVarDeclStmt* stmt) {
    // 关键：不创建新的 Environment，直接在当前环境中依次执行声明
    for (const auto& decl : stmt->decls) {
        if (decl) {
            execute(decl.get());
        }
    }
}

// ============ 函数/方法 ===============

// 把函数存入内存
void Evaluator::execute_function(FunctionStmt* stmt) {
    auto func = std::make_shared<Function>(stmt, environment_);
    environment_->define(stmt->name.lexeme, Value(func));
}

// 2. 执行 Return，直接抛出异常打断 C++ 调用栈
void Evaluator::execute_return(ReturnStmt* stmt) {
    // 无返回值
    if (stmt->values.empty()) {
        throw ReturnException(Value(), false); // 没有返回值，触发隐式返回规则
    }
    // 单返回值处理
    if (stmt->values.size() == 1) {
        Value value = evaluate(stmt->values[0].get());
        throw ReturnException(value, true); // 将算出的值包裹在异常中抛出
    }
    // 多返回值处理
    auto results = std::make_shared<std::vector<Value>>();
    for (const auto& expr : stmt->values) {
        results->push_back(evaluate(expr.get()));
    }

    throw ReturnException(Value(results), true);
}

// 3. 执行函数调用 (例如: add(1, 2))
Value Evaluator::evaluate_call(CallExpr* expr) {
    // 3.1 先算出被调用的对象 (比如从内存里拿出 add 函数)
    Value callee = evaluate(expr->callee.get());

    // 3.2 依次算出所有传入的实参
    std::vector<Value> arguments;
    for (const auto& arg : expr->arguments) {
        arguments.push_back(evaluate(arg.get()));
    }

    // 3.3 验证它是不是一个可调用对象
    std::shared_ptr<BaseCallable> function = callee.as_callable();

    // 3.4 触发它的 call 方法，并将自己 (Evaluator) 传进去以供执行函数体
    return function->call(this, arguments);
}

// 4. 补充一个 execute_block 的重载，专为函数体服务
void Evaluator::execute_block(
    const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> block_env) {
    std::shared_ptr<Environment> previous = environment_;
    try {
        environment_ = block_env;
        for (const auto& statement : statements) {
            if (statement)
                execute(statement.get());
        }
    } catch (...) {
        environment_ = previous;
        throw;
    }
    environment_ = previous;
}

// ============= 控制 ==============
// 执行 if 语句
void Evaluator::execute_if(IfStmt* stmt) {
    // 1. 如果有内联函数定义，先执行它 (将其注册到当前环境)
    if (stmt->init_stmt != nullptr) {
        execute(stmt->init_stmt.get());
    }

    // 2. 求值条件表达式 (如果是内联函数，此时就是执行刚注册的 CallExpr)
    Value condition = evaluate(stmt->condition.get());

    // 3. 进行分支判断
    if (condition.is_truthy()) {
        execute(stmt->then_branch.get());
    } else if (stmt->else_branch != nullptr) {
        execute(stmt->else_branch.get());
    }
}

// 执行基础 for 循环
void Evaluator::execute_for(ForStmt* stmt) {
    // 只要条件为真，就一直执行 body
    while (evaluate(stmt->condition.get()).is_truthy()) {
        execute(stmt->body.get());
    }
}

// for in 循环
void Evaluator::execute_for_in(ForInStmt* stmt) {
    // 1. 计算 in 后面的所有表达式的值
    std::vector<Value> iter_vals;
    for (const auto& expr : stmt->iterables) {
        iter_vals.push_back(evaluate(expr.get()));
    }

    // 2. 为循环准备一个隔离的作用域环境
    auto loop_env = std::make_shared<Environment>(environment_);
    std::shared_ptr<Environment> previous = environment_;

    try {
        environment_ = loop_env;

        // 情景 A: 纯数字构成的 Range 范围遍历
        if (iter_vals[0].get_type() == ValueType::Int) {
            int64_t start = 0, end = 0, step = 1;
            if (iter_vals.size() == 1) {
                end = iter_vals[0].as_int();
            } else if (iter_vals.size() == 2) {
                start = iter_vals[0].as_int();
                end = iter_vals[1].as_int();
            } else if (iter_vals.size() == 3) {
                start = iter_vals[0].as_int();
                end = iter_vals[1].as_int();
                step = iter_vals[2].as_int();
            }

            std::string var_name = stmt->vars[0].lexeme;
            auto cmp = [step](int64_t c, int64_t l) { return step > 0 ? c < l : c > l; };

            for (int64_t i = start; cmp(i, end); i += step) {
                environment_->define(var_name, Value(i)); // 将 i 注入环境
                execute(stmt->body.get());
            }
        }
        // 情景 B: 字符串/数组 遍历
        else if (iter_vals[0].get_type() == ValueType::String) {
            std::string str = iter_vals[0].as_string();
            // 自动判断是一个变量还是两个变量 (i 或者 i, v)
            std::string idx_var = stmt->vars.size() == 2 ? stmt->vars[0].lexeme : "";
            std::string val_var = stmt->vars.size() == 2 ? stmt->vars[1].lexeme : stmt->vars[0].lexeme;

            for (size_t i = 0; i < str.length(); ++i) {
                if (!idx_var.empty())
                    environment_->define(idx_var, Value(static_cast<int64_t>(i)));
                environment_->define(val_var, Value(std::string(1, str[i])));
                execute(stmt->body.get());
            }
        } else if (iter_vals[0].get_type() == ValueType::Array) {
            auto arr = iter_vals[0].as_array();

            // 如果只有一个变量 (for v in arr)，就把值赋给它
            // 如果有两个变量 (for i, v in arr)，就把索引赋给 i，值赋给 v
            std::string idx_var = stmt->vars.size() == 2 ? stmt->vars[0].lexeme : "";
            std::string val_var = stmt->vars.size() == 2 ? stmt->vars[1].lexeme : stmt->vars[0].lexeme;

            for (size_t i = 0; i < arr->size(); ++i) {
                if (!idx_var.empty())
                    environment_->define(idx_var, Value(static_cast<int64_t>(i)));
                environment_->define(val_var, (*arr)[i]);
                execute(stmt->body.get());
            }
        }

        else {
            // 这里预留给你后续完善了 Array 类型后再对接
            throw std::runtime_error("Iterable type not supported yet in for-in loop.");
        }
    } catch (...) {
        environment_ = previous;
        throw;
    }
    environment_ = previous;
}


void Evaluator::execute_class(ClassStmt* stmt) {
    // 1. 先在环境中占个坑（允许类的方法在内部引用类自己，比如链表节点）
    environment_->define(stmt->name.lexeme, Value());
    
    // 2. 实例化一个代表类蓝图的 C++ 对象 (我们待会儿要定义的 XaqClass)
    auto klass = std::make_shared<BaseClass>(stmt->name.lexeme, stmt);
    
    // 3. 把这个蓝图正式存入变量名中
    environment_->assign(stmt->name.lexeme, Value(klass));
}
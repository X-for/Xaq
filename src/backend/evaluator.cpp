#include "backend/evaluator.h"
#include "backend/function.h"

// ==========================================
// 1. 语句执行入口与路由
// ==========================================

void Evaluator::evaluate(const std::vector<std::unique_ptr<Stmt>> &statements)
{
    for (const auto &stmt : statements)
    {
        if (stmt)
            execute(stmt.get());
    }
}

void Evaluator::execute(Stmt *stmt)
{
    if (auto var_decl = dynamic_cast<VarDeclStmt *>(stmt))
    {
        execute_var_decl(var_decl);
    }
    else if (auto expr_stmt = dynamic_cast<ExpressionStmt *>(stmt))
    {
        execute_expression_stmt(expr_stmt);
    }
    else if (auto block_stmt = dynamic_cast<BlockStmt *>(stmt))
    {
        execute_block(block_stmt, std::make_shared<Environment>(environment_));
    }
    else if (auto func_stmt = dynamic_cast<FunctionStmt *>(stmt)) {
        execute_function(func_stmt);
    }
    else if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt)) {
        execute_return(return_stmt);
    }
    else
    {
        throw std::runtime_error("Unknown statement type in evaluator.");
    }
}

// ==========================================
// 2. 具体的语句执行逻辑
// ==========================================

// 执行变量声明：auto x = 42
void Evaluator::execute_var_decl(VarDeclStmt *stmt)
{
    Value value = Value(); // 默认赋值为 Null

    // 如果等号右边有初始化表达式，就先把它算出来
    if (stmt->initializer != nullptr)
    {
        value = evaluate(stmt->initializer.get());
    }

    // 将变量名和算出的值存入当前环境 (内存)
    environment_->define(stmt->name.lexeme, value);
}

// 执行普通表达式语句
void Evaluator::execute_expression_stmt(ExpressionStmt* stmt) {
    Value result = evaluate(stmt->expression.get()); 
    // 作为独立语句执行时，直接打印出来方便我们测试观察
    std::cout << result << "\n"; 
}

// ==========================================
// 3. 表达式求值路由补充 (读取变量)
// ==========================================

Value Evaluator::evaluate(Expr *expr)
{
    if (!expr)
        throw std::runtime_error("Cannot evaluate null expression.");

    // 使用 RTTI 动态类型转换来路由节点
    if (auto lit = dynamic_cast<LiteralExpr *>(expr))
    {
        return evaluate_literal(lit);
    }
    else if (auto bin = dynamic_cast<BinaryExpr *>(expr))
    {
        return evaluate_binary(bin);
    }
    else if (auto log = dynamic_cast<LogicalExpr *>(expr))
    {
        return evaluate_logical(log);
    }
    else if (auto var = dynamic_cast<VariableExpr *>(expr))
    {
        return evaluate_variable(var);
    } else if (auto assign = dynamic_cast<AssignExpr* >(expr)) {
        return evaluate_assign(assign);
    }
    else if (auto call_expr = dynamic_cast<CallExpr*>(expr)) {
        return evaluate_call(call_expr);
    }
    throw std::runtime_error("Unknown AST node type in evaluator.");
}

// 1. 执行字面量：最底层的节点，直接把包在里面的 Value 丢出去即可
Value Evaluator::evaluate_literal(LiteralExpr *expr)
{
    return expr->value;
}

// 2. 执行二元运算：先算左边，再算右边，最后结合
Value Evaluator::evaluate_binary(BinaryExpr *expr)
{
    // 递归求值左子树和右子树
    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());

    // 算术运算符
    if (expr->op.type == TokenType::PLUS)
    { // +
        return Value(left.as_int() + right.as_int());
    }
    else if (expr->op.type == TokenType::MINUS)
    { // -
        return Value(left.as_int() - right.as_int());
    }
    else if (expr->op.type == TokenType::STAR)
    { // *
        return Value(left.as_int() * right.as_int());
    }
    else if (expr->op.type == TokenType::SLASH)
    { // /
        int64_t r_val = right.as_int();
        if (r_val == 0)
        {
            throw std::runtime_error("Division by zero.");
        }
        return Value(left.as_int() / right.as_int());
    }
    else if (expr->op.type == TokenType::PERCENT)
    { // %
        int64_t r_val = right.as_int();
        if (r_val == 0)
            throw std::runtime_error("Modulo by zero.");
        return Value(left.as_int() % r_val); // 取模运算
    }

    // 比较运算符
    else if (expr->op.type == TokenType::GREATER)
    {
        return Value(left.as_int() > right.as_int());
    }
    else if (expr->op.type == TokenType::GREATER_EQUAL)
    {
        return Value(left.as_int() >= right.as_int());
    }
    else if (expr->op.type == TokenType::LESS)
    {
        return Value(left.as_int() < right.as_int());
    }
    else if (expr->op.type == TokenType::LESS_EQUAL)
    {
        return Value(left.as_int() <= right.as_int());
    }
    else if (expr->op.type == TokenType::EQUAL_EQUAL)
    {
        // 如果两边都是布尔值，按布尔值比较
        if (left.get_type() == ValueType::Bool && right.get_type() == ValueType::Bool)
        {
            return Value(left.is_truthy() == right.is_truthy());
        }
        // 否则按整数比较 (后续可以继续扩展 Float 的比较)
        return Value(left.as_int() == right.as_int());
    }
    else if (expr->op.type == TokenType::BANG_EQUAL)
    {
        if (left.get_type() == ValueType::Bool && right.get_type() == ValueType::Bool)
        {
            return Value(left.is_truthy() != right.is_truthy());
        }
        return Value(left.as_int() != right.as_int());
    }

    // 位运算符
    else if (expr->op.type == TokenType::AMPERSAND)
    {
        return Value(left.as_int() & right.as_int());
    }
    else if (expr->op.type == TokenType::PIPE)
    {
        return Value(left.as_int() | right.as_int());
    }
    else if (expr->op.type == TokenType::LESS_LESS)
    {
        return Value(left.as_int() << right.as_int());
    }
    else if (expr->op.type == TokenType::GREATER_GREATER)
    {
        return Value(left.as_int() >> right.as_int());
    }

    throw std::runtime_error("Unsupported binary operator:" + expr->op.lexeme);
}

Value Evaluator::evaluate_logical(LogicalExpr *expr)
{
    // 关键点：只先求值左边！
    Value left = evaluate(expr->left.get());

    if (expr->op.type == TokenType::PIPE_PIPE)
    {
        // 对于 ||，如果左边是 true，直接返回，右边连看都不看
        if (left.is_truthy())
            return left;
    }
    else if (expr->op.type == TokenType::AMPERSAND_AMPERSAND)
    {
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
        environment_->assign(expr->name.lexeme, right_val);
        return right_val;
    }

    // 3. 处理复合赋值 (+=, -=, *= 等)
    // 必须先从 Environment 里把老的值拿出来
    Value current_val = environment_->get(expr->name.lexeme);
    Value new_val = Value();

    if (expr->op.type == TokenType::PLUS_EQUAL) {
        new_val = Value(current_val.as_int() + right_val.as_int());
    } 
    else if (expr->op.type == TokenType::MINUS_EQUAL) {
        new_val = Value(current_val.as_int() - right_val.as_int());
    }
    else if (expr->op.type == TokenType::STAR_EQUAL) {
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
            if (statement) execute(statement.get());
        } 
    } catch (...) {
        environment_ = previous; // 报错也切换回原环境
        throw; // 继续往上抛异常
    }
    environment_ = previous; // 执行完毕后切换回原环境
}


// 读取变量：直接向 Environment 要数据
Value Evaluator::evaluate_variable(VariableExpr* expr) {
    return environment_->get(expr->name.lexeme);
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
void Evaluator::execute_block(const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> block_env) {
    std::shared_ptr<Environment> previous = environment_; 
    try {
        environment_ = block_env; 
        for (const auto& statement : statements) {
            if (statement) execute(statement.get());
        } 
    } catch (...) {
        environment_ = previous; 
        throw; 
    }
    environment_ = previous; 
}
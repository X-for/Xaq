#include "backend/function.h"
#include "backend/evaluator.h"

Value Function::call(Evaluator *evaluator, const std::vector<Value> &arguments) {
    // 1. 创建函数专属的局部环境，它的父级是闭包 (定义时的环境)
    auto environment = std::make_shared<Environment>(closure_);

    // 2. 将实参绑定到形参上
    for (size_t i = 0; i < declaration_->params.size(); ++i) {
        Value arg_val = Value();
        if (i < arguments.size()) {
            arg_val = arguments[i];
        } else if (declaration_->params[i].default_value) {
            // 如果没传实参，但有默认值 (如 b = 2)，则计算默认值
            arg_val = evaluator->evaluate(declaration_->params[i].default_value.get());
        }
        environment->define(declaration_->params[i].name.lexeme, arg_val);
    }

    // 3. 预先注入具名的返回变量 (如 -> (ret: Int = 0))
    for (const auto& ret_var : declaration_->return_vars) {
        Value ret_val = Value();
        if (ret_var.default_value) {
            ret_val = evaluator->evaluate(ret_var.default_value.get());
        }
        if (!ret_var.name.lexeme.empty()) {
            environment->define(ret_var.name.lexeme, ret_val);
        }
    }

    // 4. 开始执行函数体
    try {
        evaluator->execute_block(declaration_->body, environment);
    } catch (const ReturnException& ret) {
        // 成功拦截到 return 抛出的异常，将值正常返回！
        if (ret.has_value) {
            return ret.value;
        }
    }

    // 5. 如果函数体里没有写 return，触发 Xaq 的隐式返回规则
    // 如果有具名返回变量，默认返回第一个
    if (!declaration_->return_vars.empty() && !declaration_->return_vars[0].name.lexeme.empty()) {
        return environment->get(declaration_->return_vars[0].name.lexeme);
    }

    return Value(); // 否则返回 Null
}
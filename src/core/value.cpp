#include "core/value.h"
#include <iostream>

Value::Value() : type_(ValueType::Null), value_(std::monostate{}) {}

Value::Value(int64_t v) : type_(ValueType::Int), value_(v) {}
Value::Value(double v) : type_(ValueType::Float), value_(v) {}
Value::Value(bool v) : type_(ValueType::Bool), value_(v) {}
Value::Value(const std::string &v) : type_(ValueType::String), value_(v) {}

std::ostream &operator<<(std::ostream &os, const Value &val) {
    switch (val.get_type()) {
        case ValueType::Null:
            os << "null";
            break;
        case ValueType::Int:
            os << std::get<int64_t>(val.value_);
            break;
        case ValueType::Float:
            os << std::get<double>(val.value_);
            break;
        case ValueType::Bool:
            os << (std::get<bool>(val.value_) ? "true" : "false");
            break;
        case ValueType::String:
            os << "\"" << std::get<std::string>(val.value_) << "\"";
            break;
        case ValueType::Array: {
            auto arr = val.as_array();
            os << "[";
            for (size_t i = 0; i < arr->size(); ++i) {
                os << (*arr)[i];
                if (i < arr->size() - 1)
                    os << ", ";
            }
            os << "]";
            break;
        }
        default:
            os << "unknown";
    }
    return os;
}
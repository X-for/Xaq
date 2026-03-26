cmake_minimum_required(VERSION 3.10)

# 定义工程名称和使用的语言 (C++)
project(XaqLang CXX)

# 强制使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 告诉编译器去哪里找头文件
# 这样在 src 里的代码就可以直接写 #include "core/value.h"
include_directories(${CMAKE_SOURCE_DIR}/include)

# 递归查找 src 目录下所有的 .cpp 文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 将找到的源文件编译成可执行文件 xaq
add_executable(xaq ${SOURCES})
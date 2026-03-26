# Xaq

## 类型
Xaq 是一门动态类型的编程语言，支持以下基本类型：
| 类型    | 描述                             |
| ------- | -------------------------------- |
| Null    | 表示空值                         |
| Int     | 整数类型                         |
| Float   | 浮点数类型                       |
| Bool    | 布尔类型                         |
| String  | 字符串类型                       |
| Foreign | 外部类型，用于表示外部资源或对象 |

## 基本语法

### 变量声明和赋值
```c
// 声明和赋值变量
auto x1 = 42 // 自动识别类型
x2 = 1.4 // 自动识别类型
x3: Bool = true // 显式指定类型
x4 = String("Hello, World!")  // 显示指定类型
x1 = 1.2 // 错误: 类型不匹配
x1 = Int(1.2) // 正确: 显式转换类型 x1 = 1
x2 = 42 // 正确: 自动类型转换 x2 = 42.0
x4 = 12 // 正确: 自动类型转换为字符串 x4 = "12"
```
### 运算符
#### 算术运算符
```c
a: Int, b: Int = 10, 20
```
| 运算符 | 描述   | 实例                          |
| ------ | ------ | ----------------------------- |
| +      | 加法   | a + b 结果为30                |
| -      | 减法   | a - b 结果为-10               |
| *      | 乘法   | a * b 结果为200               |
| /      | 除法   | a / b 结果为0 , b / a 结果为2 |
| %      | 取模   | a % b 结果为10                |
| **     | 幂运算 | a ** 2 结果为100              |

#### 比较运算符
```c
a: Int, b: Int = 10, 20
```
| 运算符 | 描述     | 实例               |
| ------ | -------- | ------------------ |
| ==     | 等于     | a == b 结果为false |
| !=     | 不等于   | a != b 结果为true  |
| >      | 大于     | a > b 结果为false  |
| <      | 小于     | a < b 结果为true   |
| >=     | 大于等于 | a >= b 结果为false |
| <=     | 小于等于 | a <= b 结果为true  |

#### 赋值运算符
```c
a: Int = 10
```
| 运算符 | 描述       | 实例                                           |
| ------ | ---------- | ---------------------------------------------- |
| =      | 赋值       | a = 20 结果为20                                |
| +=     | 加法赋值   | a += 5 结果为15                                |
| -=     | 减法赋值   | a -= 5 结果为5                                 |
| *=     | 乘法赋值   | a *= 2 结果为20                                |
| /=     | 除法赋值   | a /= 2 结果为3                                 |
| %=     | 取模赋值   | a %= 3 结果为1                                 |
| **=    | 幂运算赋值 | a **= 2 结果为100                              |
| :=     | 海象运算符 | a := 10 结果为10, 可以在表达式中进行赋值和返回 |

#### 位运算符
```c
a: Int = 10
b: Int = 20
```
| 运算符 | 函数/方法实现          | 描述   | 实例                                                  |
| ------ | ---------------------- | ------ | ----------------------------------------------------- |
| &      | Int.and()              | 位与   | a & b 结果为0                                         |
| \|     | Int.or()               | 位或   | a  \| b 结果为30                                      |
| ^      | Int.xor()              | 位异或 | a ^ b 结果为30                                        |
|        | Int.xnor(Int, bits=32) | 位同或 | a.xnor(b) 结果为0, bits调整位宽, 位宽不一样结果不一样 |
| ~      | Int.not()              | 位非   | ~a 结果为-11                                          |
| <<     | Int.shl()              | 左移   | a << 2 结果为40                                       |
| >>     | Int.shr()              | 右移   | a >> 2 结果为2                                        |

##### 逻辑运算符
```c
a, b : Bool = true, false
```
| 运算符 | 函数/方法实现   | 描述   | 实例                |
| ------ | --------------- | ------ | ------------------- |
| &&     | Bool.and(), and | 逻辑与 | a && b 结果为false  |
| \|\|   | Bool.or(), or   | 逻辑或 | a \|\| b 结果为true |
| !      | Bool.not(), not | 逻辑非 | !a 结果             |

##### 运算符优先级
| 运算符                                   | 描述   |
| ---------------------------------------- | ------ |
| (expression), [expression], {expression} | 圆括号 |
| **                                       | 幂运算 |
| +x, -x, ~x                                  | | 正负号, 位非 |
| *, /, %                                | 乘除模 |
| +, -                                  | 加减   |
| <<, >>                               | 位移   |
| &, Int.and()                                    | 位与   |
| ^, Int.xor()                                    | 位异或 |
| \|, Int.or()                                   | 位或   |
| ==, !=, >, <, >=, <=                 | 比较   |
| !, Bool.not(), not                                   | 逻辑非 |
| &&, Bool.and(), and                                   | 逻辑与 |
| \|\|, Bool.or(), or                                    | 逻辑或   |
| if, else if, else                    | 条件语句 |
| lambda                               | 匿名函数 |
| :=                                  | 海象运算符 |


### 函数定义和调用

#### 函数定义

```c
// 定义函数1
func add(a: Int, b: Int) -> (ret: Int=0) {
    ret = a + b
}

// 定义函数2
func add(a, b) -> Int {
    return a + b
}

// 定义函数3
func add(a: Int, b: Int) -> ret {
    ret = a + b
}

// 定义函数4
func add(a, b) {
    return a + b // 如果没有显示指明返回类型, 默认返回类型return值
}

// 定义函数5
func add(a, b=2) {
    return a + b // 参数b有默认值, 可以省略调用时的参数
}
```
#### 函数调用
```c
// 调用函数1
ret1 = add(1, 2)
// 调用函数2
ret2 = add(1) // 如果存在默认值, 可以省略参数
// 调用函数3
ret3 = add a, b // 
```


### 条件语句
```c
x: Int, y: Int = 10, 20
// if 语句1
if x > y {
    statement1
} else if x == y {
    statement2
} else {
    statement3
}

// if 语句2
if cmp1(x, y) -> () {return x > y} {
    statement1
} else if cmp2(x, y) -> (ret: Bool) {return ret = x == y} {
    statement2
} else {
    statement3
} 
// 这里的cmp1和cmp2可以当作函数直接调用, 也可以当作条件表达式直接使用

// if 语句3
if x == y {
    statement1
} else cmp3(x, y) -> Bool {return x > y} {
    statement2
} else {
    statement3
}
// else后的if可以省略, 知道匹配到最后一个else为止

```

### 循环语句
```c
// 循环1
i = 10
for i > 0 {
    statement
    i = i - 1
}
// 循环2
arr: Array[Int] = [1, 2, 3, 4, 5]
for i in arr {
    statement
} 
for i, v in arr {
    statement
} // i 是索引, v是值
// 遍历数组中的每个元素
// 循环3
for i in 10 {
    statement
} // 默认从0到9循环

// 循环4
for i in 5, 10 {
    statement
} // 从5到9循环

// 循环5 
for i in 10, 5 {
    statement
} // 从9到5循环

for i in 10, 5, -1 {
    statement
} // 从10到5循环, 步长为-1

for i in "string" {
    statement
} // 遍历字符串中的每个字符

for i=0; i<10; i+=1 {
    statement
} // C风格的for循环

```

### 类


#### 类的定义
```c
// 定义类
class Person {
    height: Float
    age: Int
    name: String="Alex" // 成员变量可以有默认值

    // 方法定义 
    method age() -> Int {
        return age
    } 
    // 方法定义2
    method name() {} // 方法与成员变量同名时直接返回成员变量的值

    method set_height(h: Float) {
        this.height = h
    }
}

// 定义类2 
class BigInt {
    value: String // 使用字符串来存储大整数

    operator +(other: BigInt) -> this.BigInt {
        // 实现大整数加法的逻辑
    }
}

// 继承类
Person Student {
    student_id: String
    Person.height // 继承父类的成员变量, 不可修改类型
    Person.method age(){} // 继承父类的方法
    Person.method set_height(h: Float){ // 重写父类的方法
        this.height = h
    }
}
// 多继承
Person Staff {
    staff_id: String
    Person.height
    Person.age
    Person.method age(){}
    Person.method set_height(h: Float){
        this.height = h
    }
}
[Staff, Student] Employee {
    employee_id: String
    Staff.staff_id
    Student.student_id
    age: Float, Private // 直接定义成员变量, 不继承父类的成员变量
    Person.height
    Student.method age(){}
    method Employee(employee_id: String, height: Float) {
        this.employee_id = employee_id
        this.height = height
    } // 构造函数
    Person.method set_height(h: Float){
        this.height = h
    }
    Private method age_up() {
        this.age = this.age + 1
    } // 私有方法, 只能在类的内部调用
}
```
#### 调用类和方法
```c
s1: Student = Student(student_id="S123", height=180.5)
s1.age() // 调用方法
s1.name // 访问成员变量
s1.set_height(185.0) // 调用方法修改成员变量
s1.name = "Bob" // 直接修改成员变量

s2: Employee = "E123", 175.0 // 调用构造函数创建对象
s2.age // 错误: 私有成员变量不能访问
s2.age() // 正确: 通过方法访问私有成员变量
```

### 导入模块
```c
import math // 导入内置模块
import "code.func1" // 导入用户的模块/代码, 这里的code.func1是一个文件路径
import math as m // 导入模块并起别名
import c "func1.c" as func1_c // 导入C语言编写的模块并起别名
import cpp "func2.cpp" as func2_cpp // 导入C++语言编写的模块并起别名
import cpp "func3.h" as func3_cpp // 导入C++头文件编写的模块并起别名
import py "code.func4.py" as func4_py // 导入Python语言编写的模块并起别名
```

### 注释
```c
// 这是单行注释
/*
这是多行注释
*/
"""
这是文档字符串，可以用于注释或字符串，支持多行
"""
```
## 复合数据结构
### 迭代类型
Xaq的迭代类型是一种可以被迭代的集合类型，支持索引访问、切片、迭代等操作。迭代类型包括字符串、数组、字典、集合等。
#### 迭代器定义
```c
iter1: Iterator[Int] = [1, 2, 3, 4]
iter2: Iterator[] = ["a", "b34", "c213"]
iter3: Iterator[] = [1, "a", true]
iter4: Iterator[Int] = [] // 定义一个空的整数迭代器
iter5: Iterator[Any] = [1, "a", true] // 定义一个任意类型的迭代器
```
#### 迭代器方法
| 方法       | 描述                                |
| ---------- | ----------------------------------- |
| Iterator() | 创建一个迭代器实例                  |
| next()     | 获取下一个元素                      |
| has_next() | 检查是否还有下一个元素              |
| length()   | 获取迭代器的长度                    |
| reset()    | 重置迭代器到初始状态                |
| []         | 索引访问                            |
| index()    | 索引访问(默认访问 第一个元素/下标0) |
| [::]       | 切片操作 start, end, step           |
| slice()    | 切片操作 start, end, step           |
| add()      | 添加元素                            |

### 数组
Xaq的数组继承自迭代类型，是一种动态大小的集合，可以存储同一类型的元素。数组支持索引访问、切片、迭代等操作。
#### 数组定义
```c
arr1: Array[Int] = [1, 2, 3, 4] // 定义一个整数数组
arr2: Array[] = ["a", "b34", "c213"] // 定义一个字符串数组
arr3: Array[] = [1, "a", true] // 定义一个混合类型的数组
arr4: Array[Int] = [] // 定义一个空的整数数组
arr5: Array[Any] = [1, "a", true] // 定义一个任意类型的数组
arr6 = [1, 2, 3] // 自动推断为 Array[Int]
arr7 = [1, "a", true] // 自动推断为 Array[Any]
arr8 = Array[Int](5) // 定义一个长度为5的整数数组, 元素默认值为0
arr9 = Array[]("a", "b", "c") // 定义一个字符串数组, 元素类型自动推断为String
```
#### 数组操作
##### 数组索引访问
```c
arr: Array[Int] = [10, 20, 30, 40]
a1 = arr[0] // 访问第一个元素, a1 = 10
a2 = arr[2] // 访问第三个元素, a2 = 30
a3 = arr[-1] // 访问最后一个元素, a3 = 40
a4 = arr.end // 访问最后一个元素, a4 = 40
a5 = arr.end(-1) // 访问最后一个元素, a5 = 40
a6 = arr.end(-2) // 访问倒数第二个元素, a6 = 30
```
##### 数组切片
```c
arr: Array[Int] = [10, 20, 30, 40, 50]
s1 = arr[1:4] // 切片从索引1到索引4(不包含4), s1 = [20, 30, 40]
s1 = arr.slice(1, 4) // 等价
s2 = arr[:3] // 切片从开始到索引3(不包含3), s2 = [10, 20, 30]
s2 = arr.slice(0, 3) // 等价
s3 = arr[2:] // 切片从索引2到结束, s3 = [30, 40, 50]
s3 = arr.slice(2, arr.length) // 等价
s4 = arr[:] // 切片整个数组, s4 = [10, 20, 30, 40, 50]
s4 = arr.slice() // 等价
s5 = arr[::2] // 切片步长为2, s5 = [10, 30, 50]
s5 = arr.slice(step=2) // 等价
s6 = arr[1:4:2] // 切片从索引1到索引4(不包含4), 步长为2, s6 = [20, 40]
s6 = arr.slice(1, 4, 2) // 等价
```

##### 数组方法
| 方法       | 描述                                    |
| ---------- | --------------------------------------- |
| Array()    | 创建一个数组实例                        |
| length()   | 获取数组的长度                          |
| add()      | 添加元素 add(element, index)            |
| remove()   | 删除元素 remove(index=-1, element=None) |
| clear()    | 清空数组                                |
| index()    | 索引访问                                |
| slice()    | 切片操作 start, end, step               |
| sort()     | 排序数组                                |
| reverse()  | 反转数组                                |
| extend()   | 扩展数组, 添加另一个数组的元素          |
| +          | 扩展数组,添加另一个数组的元素           |
| copy()     | 创建一个新的数组实例                    |
| contains() | 检查数组是否包含某个元素                |

### 字典
Xaq的字典是一种键值对集合，支持索引访问、迭代等操作。字典的键必须是不可变类型，值可以是任意类型。
#### 字典定义
```c
dict1: Dict[String, Int] = {"a": 1, "b": 2} // 定义一个字符串键和整数值的字典
dict2: Dict[] = {"a": 1, "b": "two", "c": true} // 定义一个混合类型的字典
dict3: Dict[String, Any] = {"a": 1, "b": "two", "c": true} // 定义一个字符串键和任意值的字典
dict4: Dict[String, Int] = {} // 定义一个空的字符串键和整数值的字典
dict5: Dict[Any, Any] = {"a": 1, 2: "two", true: "three"} // 定义一个任意键和值的字典
dict6 = {"a": 1, "b": 2} // 自动推断为 Dict[String, Int]
dict7 = {"a": 1, "b": "two", "c": true} // 自动推断为 Dict[String, Any]
```
#### 字典方法
| 方法     | 描述                                                    |
| -------- | ------------------------------------------------------- |
| Dict()   | 创建一个字典实例                                        |
| length() | 获取字典键个数                                          |
| add()    | 添加键值对 add(key, value)                              |
| remove() | 删除键值对 remove(key)                                  |
| clear()  | 清空字典                                                |
| keys()   | 获取字典的所有键                                        |
| values() | 获取字典的所有值                                        |
| items()  | 获取字典的所有键值对                                    |
| index()  | 索引访问, 索引必须是键                                  |
| []       | 索引访问, 索引必须是键                                  |
| get()    | 获取键对应的值, 没有则返回默认值 get(key, default=Null) |

### 集合
Xaq的集合是一种无序不重复元素的集合，支持迭代等操作。集合的元素必须是不可变类型。
#### 集合定义
```c
set1: Set[Int] = (1, 2, 3, 4) // 定义一个整数集合
set2: Set[] = ("a", "b34", "c213") // 定义一个字符串集合
set3: Set[] = (1, "a", true) // 定义一个混合类型的集合
set4: Set[Int] = () // 定义一个空的整数集合
set5: Set[Any] = (1, "a", true) // 定义一个任意类型的集合
set6 = (1, 2, 3) // 自动推断为 Set[Int]
set7 = (1, "a", true) // 自动推断为 Set[Any]
```
#### 集合方法
| 方法         | 描述                     |
| ------------ | ------------------------ |
| Set()        | 创建一个集合实例         |
| length()     | 获取集合的元素个数       |
| add()        | 添加元素 add(element)    |
| remove()     | 删除元素 remove(element) |
| clear()      | 清空集合                 |
| contains()   | 检查集合是否包含某个元素 |
| union()      | 返回两个集合的并集       |
| intersect()  | 返回两个集合的交集       |
| difference() | 返回两个集合的差集       |

## 异常处理
### 语法错误
```c
// 语法错误示例
for 1 in 10: // 符号错误
    statement
```

### 异常

#### 异常处理
```c
try {

} catch (ExceptionType1 e) {
    // 处理异常类型1
} catch (ExceptionType2 e) {
    // 处理异常类型2
} finally {
    // 无论是否发生异常都会执行的代码
}
```

#### 抛出异常
```c
throw ExceptionType("Error message") // 抛出一个异常实例
```

#### 异常基类
```c
class BaseException {
    message: String

    method __init__(message: String) {
        this.message = message
    }

    method __str__() -> String {
        return this.message
    }

    method __repr__() -> String {
        return this.message
    }
}
```


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

p1 : Person = Person(height=172.2, age=12, name="aa")

__print__(p1.name)
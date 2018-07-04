# Write a simple library for cpp



## 该项目用到的C++11新特性

* enum class

使用方法

```c++
enum class JsonType {
    T_NULL = 1,
    T_NUMBER = 2,
    T_BOOL = 4,
    T_STRING = 8,
    T_ARRAY = 16,
    T_OBJECT = 32
};
```

要想理解enum class，先来看看enum，打个比方：

```c++
enum Color {black, white, red};
//实际上它是#define的打包
#define black 0
#define white 1
#define red 2
```

所以即便是两个不一样的enum，即便是不同的枚举名，里面的内容也不能重名。另外，对于enum class来说，其是更为类型安全的做法。

在C语言中，会有这样的用法：

```c
enum Direction {TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT};

int main()
{
  enum Direction d = 1;
}
```

但C++是不允许的，C++只允许Direction d 由枚举体的TOP_RIGHT等值或者另外一个枚举体变量赋值。比如这样：

```c++
enum Color { red, green, blue };                    // plain enum 
enum Card { red_card, green_card, yellow_card };    // another plain enum 

void fun() {

    // examples of bad use of plain enums:
    Color color = Color::red;
    Card card = Card::green_card;

    int num = color;    // no problem

    if (color == Card::red_card) // no problem (bad)
        cout << "bad" << endl;

    if (card == Color::green)   // no problem (bad)
        cout << "bad" << endl;
}
```

总结一下：

* enum class：枚举类型名是局部的，它们的值不会被隐式地转换成其它类型，比如其它的enum或者是整数；
* enum：枚举类型与enum的作用域相同，并且会被隐式地转换成其它类型；

因此：

> Prefer scoped enums to unscoped enums



* alias declarations, using xxx = yyy;

使用方法：

```c++
using array = std::vector<Json>;
using object = std::map<std::string, Json>;
```

alias declarations与typedef声明做的是同样的事情，但alias declarations往往更容易被接收，其中一点就是，alias declarations看起来更加清晰：

```c++
typedef void (*FP)(int, const std::string&);    //typedef

using FP = void(*)(int, const std::string&):    //别名声明
```

另一个好处就是alias template，打个比方定义一个链表的别名，该链表使用自定义的分配器，使用alias template：

```c++
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;

MyAllocList<Widget> lw;
//MyAllocList<T> == std::list<T, MyAlloc<T>>
```

但如果只使用typedef：

```c++
template<typename T>
struct MyAllocList{
    typedef std::list<T, MyAlloc<T>> type;
};

MyAllocList<Widget>::type lw;
```

因此如果在另外一个template里面使用这个别名，就需要加上typename，因为这是一个依赖别名，而alias template就不需要：

```c++
template<typename T>
class Widget{
private:
    typename MyAllocList<T>::type list;
    MyAllocList<T> list;    //没有"typename"，没有"::type"
    ...
};
```



* explict

```c++
explicit Json() noexcept;
explicit Json(std::nullptr_t) noexcept;//nullptr is a C++ keyword literal of type std::nullptr_t
explicit Json(double);
explicit Json(int);
explicit Json(bool);
explicit Json(const std::string&);
explicit Json(std::string&&);
explicit Json(const char*);
explicit Json(const array&);
explicit Json(array&&);
explicit Json(const object &);
explicit Json(object &&);
```

在不加explicit的时候，编译器允许做隐式转换，这样就可以使用一个简单的参数来调用构造函数。打个比方：

```c++
class Foo
{
    public:
      // single parameter constructor, can be used as an implicit conversion
      Foo (int foo) : m_foo (foo) 
      {
      }

      int GetFoo () { return m_foo; }

    private:
      int m_foo;
};

void DoBar (Foo foo)
{
	int i = foo.GetFoo ();
}

int main ()
{
  	DoBar (42);
}
```

虽然参数不是Foo对象，但一个int也能被传递进去。

因此添加一个explicit的关键字在构造函数前，可以有效地避免隐式转换，在编译时把类似这样的操作DoBar (42)，提前报告。

 

* std::move

[参考](http://www.lucienxian.top/2017/09/09/C-11%E6%96%B0%E6%A0%87%E5%87%86-%E5%AF%B9%E8%B1%A1%E7%A7%BB%E5%8A%A8%E5%92%8C%E5%8F%B3%E5%80%BC%E5%BC%95%E7%94%A8/)

* nullptr_t

```c++
explicit Json(std::nullptr_t) noexcept;

static void serialize(std::nullptr_t, string &out){
    out+="null";
}
```

nullptr是一个常量，而nullptr_t是该常量的类型。

```c++
std::nullptr_t p;
p = nullptr;
```



* runtime_error

使用：

```c++
double JsonValue::double_value() const               {throw std::runtime_error("not a number!");}
int JsonValue::int_value() const                     {throw std::runtime_error("not a number!");}
const string &JsonValue::string_value() const        {throw std::runtime_error("not a string!");}
bool JsonValue::bool_value() const                   {throw std::runtime_error("not a boolean!");}
const Json::array &JsonValue::array_value() const    {throw std::runtime_error("not a array!");}
const Json::object &JsonValue::object_value() const  {throw std::runtime_error("not a object!");}
```

runtime_error继承自exception，它报告运行时的异常。

* noexcept

使用：

```c++
Json::Json() noexcept               : jv_ptr(obj_null){}
Json::Json(std::nullptr_t) noexcept : jv_ptr(obj_null){}
```

对于用户以及编译器来说，预先知道某个函数不会抛出异常，有利于简化该函数的代码，并且编译器也能执行某些特殊的优化操作。

需要注意的是编译器并不会在编译时监察noexcept，也就是即便函数声明了noexcept，又同时有throw语句，或者调用了可能抛出异常的操作，编译同样顺利通过，比如这样：

```c++
void f() noexcept{
    throw exception();
}
```

一旦一个noexcept函数抛出了异常，程序就会调用terminate以确保遵守不在运行时抛出异常的承诺。

> noexcept最有用的地方是用在move constructor和move assignment上，你的move操作如果不是noexcept的，很多情况下即使逻辑上可以move，编译器也会执行copy。



* std::numeric_limits<T>

```c++
if (std::fabs(val) == std::numeric_limits<double>::max())
            throw std::runtime_error("DOUBLE OVERFLOW");
```

在c++11中，std::numeric_limits模板类提供了各种基础算术类型的极值等属性信息，以取代预处理常数。

* raw string

我们主要在测试中使用了raw string：

```c++
std::string test = R"(["a",123,[true,false,null]])";
```

raw string的一大作用就是，它不会对反斜杠'\'进行特殊的转义处理。



## Usage

要使用该库，首先要包含头文件**include "json.hpp"**

* 构造一个json对象

```c++
Json j_arr = Json(Json::array{Json(1), Json(2), Json(3)});

Json j_obj(Json::object {
    { "key1",    Json("value1") },
    { "key2", Json(false) },
    { "key3", j_arr },
});
```

* 序列化

```c++
j_obj.serialize(out_obj);

std::cout << out_obj << std::endl;
//{"key1": "value1", "key2": false, "key3": [1, 2, 3]}
```

* 反序列化

```c++
std::string err_com;
Json ret = Json::parse(out_obj, err_com);
if (err_com.size()){
    std::cout << err_com << std::endl;
    return 0;
}
JSON11_TEST_ASSERT(ret.is_object());
```


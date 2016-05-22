#include <iostream>

void test_func1()
{
    std::cout << "called the test function1" << std::endl;
}

void test_func2()
{
    std::cout << "called the test function2" << std::endl;
}

void test_func3(int a)
{
    std::cout << "called the test function3, passed value: " << a << std::endl;
}

struct MyClass
{
    void some_method()
    {
        std::cout << "This is a method of class MyClass" << std::endl;
    }
};

int main(int argc, char **argv)
{
    MyClass mc;
    Delegate::Delegate<void()> d;

    d += Delegate::Function(test_func1);
    d += Delegate::Function(test_func2);
    d += Delegate::Function(&mc, &MyClass::some_method);

    d();

    return 0;
}

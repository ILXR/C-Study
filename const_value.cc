#include <bits/stdc++.h>

using namespace std;

// static无论是全局变量还是局部变量都存储在全局/静态区域，在编译期就为其分配内存，在程序结束时释放
// const全局变量存储在只读数据段，编译期最初将其保存在符号表中，第一次使用时为其分配内存，在程序结束时释放
// const局部变量存储在栈中，代码块结束时释放
// 全局变量存储在全局/静态区域，在编译期为其分配内存，在程序结束时释放
// 局部变量存储在栈中，代码块结束时释放

const int a = 1;        // 全局常量无法修改
static const int b = 2; // 全局静态常量也无法修改

void fun()
{
    static const int c = 3; // 局部静态常量无法修改，存储在 rodata
    // *((int *)&c) = -3;      // segment fault

    const int d = 4; // 局部常量可以修改
    *((int *)&d) = -4;
}

class A
{
public:
    static const int e; // 存储在 rodata
};
const int A::e = 5;

int main(void)
{
    // *((int *)&a) = -1; // segmentation fault
    // *((int *)&b) = -2; // segmentation fault
    fun();
    // *((int *)&(A::e)) = -5; // segmentation fault
}
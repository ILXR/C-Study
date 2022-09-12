#include <bits/stdc++.h>

using namespace std;


// 总结：局部静态常量 和 全局常量 都会存储在 rodata 区域，无法修改
// 除此之外均可以修改
// 注意：局部常量（非静态）是存在栈上的，可以修改

const int a = 1;        // 全局常量无法修改
static const int b = 2; // 全局静态常量也无法修改

void fun()
{
    static const int c = 3; // 局部静态常量无法修改，存储在 rodata
    // *((int *)&c) = -3;      // segment fault

    const int d = 4; // 局部常量可以修改
    *((int *)&d) = -4;
}

int main(void)
{
    // *((int *)&a) = -1; // segmentation fault
    // *((int *)&b) = -2; // segmentation fault
    fun();
}
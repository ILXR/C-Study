#include <iostream>

using namespace std;

// alignas 只能控制结构体整体的对齐，并且必须是 2 的幂，sizeof(struct node) % align == 0，整数倍的关系
// 结构体内部 分别按照不同数据类型对齐，每个成员的起始地址一定是它的 size 的整数倍
struct alignas(8) node
{
    char a;     // 0
    uint16_t b; // 2 , sizeof uint16_t = 2
    uint32_t c; // 4 , sizeof uint32_t = 4
    char d;     // 8
    uint16_t e; // 10
};

int main(int argc, char const *argv[])
{
    // offset 可以返回某个成员相对于结构体起始地址的偏移量（字节）
    cout << sizeof(node) << " " << alignof(node) << endl
         << offsetof(node, a) << " " << offsetof(node, b) << " "
         << offsetof(node, c) << " " << offsetof(node, d) << " " << offsetof(node, e) << endl;
    return 0;
}